
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#ifdef WIFI_ENABLE
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico_hal.h"

enum ParseStep {
    PARSE_VERB,
    PARSE_PATH,
    PARSE_PROTOCOL,
    PARSE_PROTOCOL_POST,
    PARSE_HEADER_NAME,
    PARSE_HEADER_VALUE_PRE,
    PARSE_HEADER_VALUE,
    PARSE_HEADER_VALUE_POST,
    PARSE_BODY_PRE,
    PARSE_BODY,
    PARSE_DONE,
    PARSE_ERROR
};


enum ParseHeader {
    CONTENT_LENGTH,
    HEADER_UNKNOWN,
};

enum HttpVerb {
    VERB_GET, VERB_POST, VERB_DELETE, VERB_UNKNOWN
};


struct HttpHandler {
    enum ParseStep currentStep;
    enum ParseHeader currentHeader;
    enum HttpVerb httpVerb;
    uint32_t parsePosition;
    char parseBuffer[128];
    char path[128];
    uint32_t contentLength;
    uint32_t contentLengthProcessed;
    int filehandle;
    int dirhandle;
    bool moretosend;
};


struct HttpHandler handler;


int hexDigitToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    else
        return -1;
}


void copyPath(char* dest, const char* src) {
    while (*src != '\0') {
        if (*src == '%') {
            int h1 = hexDigitToInt(src[1]);
            if (h1 >= 0) {
                int h2 = hexDigitToInt(src[2]);
                if (h2 >= 0) {
                    *dest = h1 * 16 + h2;
                    dest++;
                    src += 3;
                    continue;
                }
            }
        }
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

void sendHttpResponse(struct tcp_pcb *pcb, uint status, const char* msg) {
    char buffer[128];
    int len = sprintf(buffer, "HTTP/1.1 %d OK\r\n", status);
    tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);

    if (msg != NULL && msg[0] != '\0') {
        size_t msglen = strlen(msg);
        len = sprintf(buffer, "Content-Length: %d\r\nContent-Type: text/plain\r\n\r\n", msglen + 2);
        tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);
        tcp_write(pcb, msg, msglen, TCP_WRITE_FLAG_COPY);
        tcp_write(pcb, "\r\n", 2, TCP_WRITE_FLAG_COPY);
    }
    else {
        tcp_write(pcb, "\r\n", 2, TCP_WRITE_FLAG_COPY);
    }

    tcp_output(pcb);
}


void handleDeleteOperation(struct tcp_pcb* pcb) {
    if (pico_remove(handler.path) >= 0) {
        sendHttpResponse(pcb, 200, NULL);
    } else {
        sendHttpResponse(pcb, 400, "Failed to delete file.");
    }
}


void handleCreateDirectory(struct tcp_pcb* pcb) {
    if (pico_mkdir(handler.path) >= 0) {
        sendHttpResponse(pcb, 200, NULL);
    } else {
        sendHttpResponse(pcb, 400, "Failed to create directory.");
    }
}


const static char CHUNKED_RESPONSE[] = 
    "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: application/octet-stream\r\n\r\n";


void startSendingFileContents(struct tcp_pcb *pcb) {

    struct lfs_info file_info;
    if (pico_stat(handler.path, &file_info) < 0) {
        sendHttpResponse(pcb, 404, "Can't obtain info.");
        return;
    }

    if (file_info.type == LFS_TYPE_DIR) {
        printf("Need to handle directory read\n");
        handler.dirhandle = pico_dir_open(handler.path);
        if (handler.dirhandle < 0) {
            sendHttpResponse(pcb, 400, "Can't open directory.");
            handler.dirhandle = 0;
            return;
        }        
        handler.moretosend = true;
        tcp_write(pcb, CHUNKED_RESPONSE, sizeof(CHUNKED_RESPONSE)- 1, TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);        
    }
    else {
        handler.filehandle = pico_open(handler.path, LFS_O_RDONLY);
        if (handler.filehandle < 0) {
            sendHttpResponse(pcb, 400, "Can't open file.");
            handler.filehandle = 0;
            return;
        }

        handler.contentLength = file_info.size;

        if (handler.contentLength < 1024) {
            char buffer[1024];
            int len = sprintf(buffer, 
                "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: application/octet-stream\r\n\r\n", 
                handler.contentLength);
            tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);
            pico_read(handler.filehandle, buffer, handler.contentLength);
            tcp_write(pcb, buffer, handler.contentLength, TCP_WRITE_FLAG_COPY);
            tcp_output(pcb);
        }
        else {
            handler.moretosend = true;
            tcp_write(pcb, CHUNKED_RESPONSE, sizeof(CHUNKED_RESPONSE)- 1, TCP_WRITE_FLAG_COPY);
            tcp_output(pcb);        
        }
    }
}

void continueSendingFileContents(struct tcp_pcb* pcb) {
    char buffer[1024];
    int toread = MIN(1024, handler.contentLength - handler.contentLengthProcessed);
    int len = sprintf(buffer, "%X\r\n", toread);
    tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);
    if (toread > 0) {
        pico_read(handler.filehandle, buffer, toread);
        tcp_write(pcb, buffer, toread, TCP_WRITE_FLAG_COPY);
    } else {
        handler.moretosend = false;
    }
    handler.contentLengthProcessed += toread;
    tcp_write(pcb, "\r\n", 2, TCP_WRITE_FLAG_COPY);    
    tcp_output(pcb);
}


void continueListingDirectory(struct tcp_pcb* pcb) {
    char bufferChunkSize[16];
    char buffer[1024];
    struct lfs_info file_info;
    if (pico_dir_read(handler.dirhandle, &file_info) > 0) {
        bool isdir = file_info.type == LFS_TYPE_DIR;
        int len = sprintf(buffer, "%6d %s%s%s\r\n\r\n", file_info.size, 
            isdir ? "[" : "", file_info.name, isdir ? "]" : "");
        int lenSize = sprintf(bufferChunkSize, "%X\r\n", len - 2);
        tcp_write(pcb, bufferChunkSize, lenSize, TCP_WRITE_FLAG_COPY);
        tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
    }
    else {
        struct pico_fsstat_t stat;
        pico_fsstat(&stat);
        int len = sprintf(buffer, "\r\nFS: blocks %d, block size %d, used %d\r\n\r\n", (int)stat.block_count, (int)stat.block_size,
           (int)stat.blocks_used);
        int lenSize = sprintf(bufferChunkSize, "%X\r\n", len - 2);

        tcp_write(pcb, bufferChunkSize, lenSize, TCP_WRITE_FLAG_COPY);
        tcp_write(pcb, buffer, len, TCP_WRITE_FLAG_COPY);
        tcp_write(pcb, "0\r\n\r\n", 5, TCP_WRITE_FLAG_COPY);    
        tcp_output(pcb);
        handler.moretosend = false;
    }
}



void continueSendingResponse(struct tcp_pcb* pcb) {
    if (handler.dirhandle != 0) {
        continueListingDirectory(pcb);
    } else {
        continueSendingFileContents(pcb);
    }
}


void init_http_parser() {
    handler.contentLengthProcessed = 0;
    handler.contentLength = 0;
    handler.parsePosition = 0;
    handler.currentHeader = HEADER_UNKNOWN;
    handler.httpVerb = VERB_UNKNOWN;
    handler.currentStep = PARSE_VERB;
    handler.filehandle = 0;
    handler.moretosend = false;
    handler.dirhandle = 0;
}

void end_http_parser() {
    if (handler.filehandle > 0) {
        pico_close(handler.filehandle);
        handler.filehandle = 0;
    }
    if (handler.dirhandle > 0) {
        pico_dir_close(handler.dirhandle);
        handler.dirhandle = 0;
    }
}


bool appendToBufferUnlessStopChar(char c, char stopChar) {
    if (c == stopChar) {
        handler.parseBuffer[MIN(127, handler.parsePosition)] = '\0';
        return false;
    } else {
        if (handler.parsePosition < 128)
            handler.parseBuffer[handler.parsePosition] = c;
        handler.parsePosition++;
        return true;
    }
}

// return false if space
bool appendToBufferUnlessSpace(char c) {
    return appendToBufferUnlessStopChar(c, ' ');
}

bool parseNewline(char c) {
    if (c == '\r' && handler.parsePosition == 0) {
        handler.parsePosition++;
        return true;
    }
    else if (c == '\n' && handler.parsePosition == 0) {
        handler.parsePosition++;
        return true;
    }
    return false;
}



int parse_recv_buffer(struct tcp_pcb *tpcb, uint8_t* buffer, size_t buffer_len) {
    for (size_t i = 0; i < buffer_len; i++) {
        char c = buffer[i];
        switch (handler.currentStep) {
            case PARSE_VERB:
                if (!appendToBufferUnlessSpace(c)) {
                    
                    if (strcasecmp(handler.parseBuffer, "get") == 0) {
                        handler.httpVerb = VERB_GET;
                    }
                    else if (strcasecmp(handler.parseBuffer, "post") == 0) {
                        handler.httpVerb = VERB_POST;
                    }
                    else if (strcasecmp(handler.parseBuffer, "delete") == 0) {
                        handler.httpVerb = VERB_DELETE;
                    }

                    printf("Verb: %s (%d)\n", handler.parseBuffer, handler.httpVerb);
                    // check the verb
                    handler.parsePosition = 0;
                    handler.currentStep = PARSE_PATH;
                }
                break;

            case PARSE_PATH:
                if (!appendToBufferUnlessSpace(c)) {
                    copyPath(handler.path, handler.parseBuffer);
                    printf("Path: %s\n", handler.path);
                    // check the path
                    handler.parsePosition = 0;
                    handler.currentStep = PARSE_PROTOCOL;
                }
                break;

            case PARSE_PROTOCOL:
                if (!appendToBufferUnlessStopChar(c, '\r')) {
                    printf("Protocol: %s\n", handler.parseBuffer);
                    // check the protocol
                    handler.parsePosition = 0;
                    handler.currentStep = PARSE_PROTOCOL_POST;
                }
                break;

            case PARSE_PROTOCOL_POST:
                if (c != '\n')
                    return -1;
                handler.parsePosition = 0;
                handler.currentStep = PARSE_HEADER_NAME;
                break;


            case PARSE_HEADER_NAME:
                if (handler.parsePosition == 0 && c == '\r') {
                    handler.currentStep = PARSE_BODY_PRE;
                } else if (!appendToBufferUnlessStopChar(c, ':')) {
                    printf("Header name: %s\n", handler.parseBuffer);
                    if (strcasecmp(handler.parseBuffer, "CONTENT-LENGTH") == 0) {
                        handler.currentHeader = CONTENT_LENGTH;
                    }
                    else {
                        handler.currentHeader = HEADER_UNKNOWN;
                    }
                    // check the header name
                    handler.parsePosition = 0;
                    handler.currentStep = PARSE_HEADER_VALUE_PRE;
                }
                break;

            case PARSE_HEADER_VALUE_PRE:
                if (c != ' ' && c != '\t') {
                    if (c == '\r') 
                        return -1;
                    handler.parseBuffer[0] = c;
                    handler.parsePosition = 1;
                    handler.currentStep = PARSE_HEADER_VALUE;
                }
                break;


            case PARSE_HEADER_VALUE:
                if (!appendToBufferUnlessStopChar(c, '\r')) {
                    printf("Header value: %s\n", handler.parseBuffer);
                    // check the header value
                    if (handler.currentHeader == CONTENT_LENGTH) {
                        handler.contentLength = atoi(handler.parseBuffer);
                    }
                    handler.parsePosition = 0;
                    handler.currentStep = PARSE_HEADER_VALUE_POST;
                }
                break;                


            case PARSE_HEADER_VALUE_POST:
                if (c != '\n')
                    return -1;
                handler.parsePosition = 0;
                handler.currentStep = PARSE_HEADER_NAME;            
                break;

            
            case PARSE_BODY_PRE:
                if (c != '\n')
                    return -1;

                if (handler.contentLength == 0) {
                    printf("No content, request headers have been parsed\n");
                    if (handler.httpVerb == VERB_GET) {
                        startSendingFileContents(tpcb);
                    } else if (handler.httpVerb == VERB_DELETE) {
                        handleDeleteOperation(tpcb);
                    } else if (handler.httpVerb == VERB_POST) {
                        handleCreateDirectory(tpcb);
                    } else {
                        sendHttpResponse(tpcb, 400, "Alert!!\r\n");
                    }
                    handler.currentStep = PARSE_DONE;
                } else {
                    handler.currentStep = PARSE_BODY;
                }
                break;

    
            case PARSE_BODY:
                // consume rest of buffer;

                if (handler.httpVerb == VERB_POST )                

                //printf("Consume body from %d to %d\n", i, buffer_len);
                if (buffer_len - i + handler.contentLengthProcessed > handler.contentLength) {
                    printf("We have recieved too much data\n");
                    handler.currentStep = PARSE_DONE;
                    return -1;
                }
                else {
                    if (handler.httpVerb == VERB_POST) {
                        if (handler.contentLengthProcessed == 0) {
                            handler.filehandle = pico_open(handler.path, LFS_O_CREAT | LFS_O_WRONLY | LFS_O_TRUNC);
                        }

                        pico_write(handler.filehandle, buffer + i, buffer_len - i);
                    }
                    handler.contentLengthProcessed += buffer_len - i;
                    if (handler.contentLengthProcessed == handler.contentLength) {
                        printf("We have all the data now\n");
                        handler.currentStep = PARSE_DONE;
                        sendHttpResponse(tpcb, 200, "A");

                    }
                }
                i = buffer_len;
                return 0;

            case PARSE_DONE:
                return 0;
               
        }
    }
    return 0;
}



#define TCP_PORT 80

err_t my_recv_function(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err) {
    //printf("In recv func\n");
    if (p == NULL) {
        printf("Connection has been closed\n");
        return ERR_OK;
    }

    struct pbuf* currBuf = p;
    int parseResult = 0;
    while (parseResult == 0 && currBuf != NULL) {
        parseResult = parse_recv_buffer(tpcb, reinterpret_cast<uint8_t*>(currBuf->payload), currBuf->len);
        currBuf = currBuf->next;
    }


    tcp_recved(tpcb, p->tot_len);

    pbuf_free(p);

    if (parseResult == -1) {
        printf("Parse error, aborting\n");
        tcp_close(tpcb);
        end_http_parser();
        return ERR_CLSD;
    }


    return ERR_OK;
}


err_t my_sent_function(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    if (handler.moretosend) {
        continueSendingResponse(tpcb);
        return ERR_OK;
    } else {
        tcp_close(tpcb);
        end_http_parser();
        return ERR_CLSD;
    }
}


void my_error_function(void *arg, err_t err) {
    //return ERR_OK;
}




err_t my_accept_function(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    if (err != ERR_OK) {
        printf("Failure in accept %d\n", err);
        return ERR_VAL;
    }
    printf("Client connected\n");
    init_http_parser();

    tcp_sent(client_pcb, my_sent_function);
    tcp_recv(client_pcb, my_recv_function);
    tcp_err(client_pcb, my_error_function);

    return ERR_OK;
}

bool start_http_server() {
    printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        printf("failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    pcb = tcp_listen_with_backlog(pcb, 1);
    if (!pcb) {
        printf("failed to listen\n");
        return false;
    }

    tcp_accept(pcb, my_accept_function);

    return true;
}



#endif