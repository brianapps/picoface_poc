#include "pico/stdlib.h"

const uint8_t NMI_ROM[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0xed, 0x4d, 0x06, 0x02, 0xed, 0x4d, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xed, 0x73, 0xa5, 0x01, 0x22, 0x97, 0x01, 0xed, 0x53, 
    0x99, 0x01, 0xed, 0x43, 0x9b, 0x01, 0xdd, 0x22, 0x9f, 0x01, 0xfd, 0x22, 0x9d, 0x01, 0x31, 0x25, 
    0x0f, 0xf5, 0xc5, 0xd5, 0xe5, 0xdd, 0xe5, 0xf5, 0xe1, 0x22, 0xa3, 0x01, 0xd9, 0x22, 0x8f, 0x01, 
    0xed, 0x53, 0x91, 0x01, 0xed, 0x43, 0x93, 0x01, 0xd9, 0x08, 0xf5, 0xe1, 0x22, 0x95, 0x01, 0x08, 
    0x3a, 0x48, 0x5c, 0x1f, 0x1f, 0x1f, 0xe6, 0x07, 0x32, 0xa8, 0x01, 0xdd, 0x22, 0x9f, 0x01, 0xfd, 
    0x22, 0x9d, 0x01, 0xaf, 0x32, 0x09, 0x01, 0x32, 0xa1, 0x01, 0xed, 0x57, 0x32, 0x8e, 0x01, 0xe2, 
    0xd2, 0x00, 0x3e, 0x01, 0x32, 0x09, 0x01, 0x3e, 0x04, 0x32, 0xa1, 0x01, 0xcd, 0x38, 0x09, 0x32, 
    0x08, 0x01, 0x21, 0x00, 0x40, 0x11, 0x27, 0x0f, 0x01, 0x00, 0x1b, 0xed, 0xb0, 0xcd, 0x0a, 0x01, 
    0x21, 0x27, 0x0f, 0x11, 0x00, 0x40, 0x01, 0x00, 0x1b, 0xed, 0xb0, 0xdd, 0xe1, 0xe1, 0xd1, 0xc1, 
    0x3a, 0x09, 0x01, 0xfe, 0x00, 0x20, 0x07, 0xf1, 0xed, 0x7b, 0xa5, 0x01, 0x18, 0x08, 0xf1, 0xed, 
    0x7b, 0xa5, 0x01, 0xfb, 0x00, 0x00, 0xc9, 0x00, 0x00, 0x00, 0x21, 0x29, 0x01, 0xcd, 0xa9, 0x01, 
    0x21, 0x29, 0x01, 0xc3, 0xeb, 0x01, 0xcd, 0x8e, 0x05, 0xfe, 0x00, 0x20, 0xed, 0xc9, 0xc9, 0xcd, 
    0x50, 0x04, 0x18, 0xe6, 0xcd, 0x09, 0x03, 0x18, 0xe1, 0x00, 0x01, 0x01, 0x54, 0x01, 0x00, 0x00, 
    0x73, 0x03, 0x02, 0x5d, 0x01, 0x16, 0x01, 0x6c, 0x04, 0x02, 0x6b, 0x01, 0x1f, 0x01, 0x72, 0x05, 
    0x02, 0x79, 0x01, 0x24, 0x01, 0x70, 0x06, 0x02, 0x89, 0x01, 0x00, 0x00, 0x78, 0x07, 0x02, 0x84, 
    0x01, 0x1e, 0x01, 0xff, 0x70, 0x69, 0x63, 0x6f, 0x46, 0x61, 0x63, 0x65, 0x00, 0x53, 0x61, 0x76, 
    0x65, 0x20, 0x73, 0x6e, 0x61, 0x70, 0x73, 0x68, 0x6f, 0x74, 0x00, 0x4c, 0x6f, 0x61, 0x64, 0x20, 
    0x73, 0x6e, 0x61, 0x70, 0x73, 0x68, 0x6f, 0x74, 0x00, 0x43, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x20, 
    0x52, 0x4f, 0x4d, 0x00, 0x45, 0x78, 0x69, 0x74, 0x00, 0x50, 0x6f, 0x6b, 0x65, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe5, 0xcd, 0xea, 0x09, 0xe1, 0x7e, 0xfe, 
    0xff, 0xc8, 0xcb, 0x7f, 0x28, 0x06, 0x11, 0x07, 0x00, 0x19, 0x18, 0xf2, 0x23, 0x56, 0x23, 0x5e, 
    0x23, 0xfe, 0x00, 0x28, 0x13, 0xfe, 0x61, 0x38, 0x06, 0xfe, 0x7b, 0x30, 0x02, 0xc6, 0xe0, 0x4f, 
    0xcd, 0x78, 0x09, 0x0e, 0x2d, 0xcd, 0x78, 0x09, 0x4e, 0x23, 0x46, 0x23, 0x23, 0x23, 0x78, 0xb1, 
    0x28, 0xcc, 0xe5, 0x60, 0x69, 0xcd, 0x6e, 0x09, 0xe1, 0x18, 0xc3, 0xe5, 0xcd, 0x06, 0x0a, 0xe1, 
    0xe5, 0x4f, 0x7e, 0xfe, 0xff, 0x28, 0x0d, 0xcb, 0x7f, 0x20, 0x03, 0xb9, 0x28, 0x0c, 0x11, 0x07, 
    0x00, 0x19, 0x18, 0xee, 0xcd, 0x12, 0x0a, 0xe1, 0x18, 0xe1, 0xc5, 0x23, 0x56, 0x23, 0x5e, 0x23, 
    0x23, 0x23, 0x4e, 0x23, 0x46, 0xc5, 0xd5, 0xcd, 0xba, 0x09, 0xcd, 0x12, 0x0a, 0xd1, 0xcd, 0xba, 
    0x09, 0xe1, 0xc1, 0x7c, 0xb5, 0x28, 0x03, 0xd1, 0x79, 0xe9, 0xe1, 0x18, 0xbe, 0x0a, 0x01, 0x43, 
    0x02, 0x51, 0x02, 0x5c, 0x02, 0x62, 0x02, 0x66, 0x02, 0x6c, 0x02, 0x70, 0x02, 0x74, 0x02, 0x78, 
    0x02, 0x7c, 0x02, 0x54, 0x68, 0x65, 0x20, 0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 0x72, 0x6f, 0x6d, 
    0x00, 0x54, 0x68, 0x65, 0x20, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64, 0x00, 0x54, 0x68, 0x69, 0x72, 
    0x64, 0x00, 0x41, 0x41, 0x44, 0x00, 0x45, 0x41, 0x44, 0x53, 0x45, 0x00, 0x34, 0x74, 0x68, 0x00, 
    0x34, 0x74, 0x68, 0x00, 0x34, 0x74, 0x68, 0x00, 0x34, 0x74, 0x68, 0x00, 0x54, 0x68, 0x65, 0x20, 
    0x73, 0x74, 0x75, 0x66, 0x66, 0x00, 0x03, 0x00, 0x9c, 0x02, 0xa5, 0x02, 0xb1, 0x02, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x61, 0x67, 0x65, 
    0x20, 0x54, 0x77, 0x6f, 0x00, 0x41, 0x6e, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20, 0x6f, 0x6e, 0x65, 
    0x00, 0x41, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x65, 0x20, 0x66, 0x69, 0x6e, 0x61, 0x6c, 0x00, 0x01, 
    0x07, 0x00, 0xeb, 0xdd, 0x7e, 0x02, 0xdd, 0xb6, 0x03, 0x28, 0x04, 0xcb, 0xbe, 0x18, 0x02, 0xcb, 
    0xfe, 0x09, 0x13, 0x1a, 0xfe, 0x00, 0x28, 0x04, 0xcb, 0xbe, 0x18, 0x02, 0xcb, 0xfe, 0x09, 0xeb, 
    0x2b, 0x0e, 0xff, 0x3e, 0x0a, 0x96, 0x23, 0x23, 0x06, 0x0a, 0xb8, 0xeb, 0x30, 0x0c, 0xcb, 0xbe, 
    0xeb, 0x13, 0x13, 0x13, 0xed, 0xa0, 0xed, 0xa0, 0x18, 0x0a, 0xcb, 0xfe, 0xeb, 0x23, 0x23, 0x13, 
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x10, 0xe2, 0xc9, 0xdd, 0x21, 0x00, 0x00, 0x11, 0x00, 0x00, 
    0xdd, 0x36, 0x01, 0x07, 0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0xed, 0x53, 0x91, 0x03, 0x21, 0x42, 
    0x2a, 0xdd, 0x75, 0x04, 0xdd, 0x74, 0x05, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 
    0x28, 0xf9, 0x11, 0xd4, 0x03, 0xcd, 0xbf, 0x02, 0x21, 0xbf, 0x03, 0xcd, 0xa9, 0x01, 0x3a, 0x90, 
    0x03, 0xfe, 0x00, 0x28, 0x02, 0x3e, 0x58, 0xc6, 0x20, 0x4f, 0x11, 0x0e, 0x0f, 0xcd, 0x78, 0x09, 
    0x21, 0xbf, 0x03, 0xc3, 0xeb, 0x01, 0xc9, 0x2a, 0x91, 0x03, 0x11, 0x0a, 0x00, 0x19, 0xeb, 0xc3, 
    0x10, 0x03, 0x2a, 0x91, 0x03, 0x11, 0x0a, 0x00, 0xb7, 0xed, 0x52, 0xeb, 0xc3, 0x10, 0x03, 0x3e, 
    0x09, 0x18, 0x02, 0xd6, 0x31, 0x06, 0x00, 0x4f, 0x21, 0x44, 0x2a, 0x09, 0x09, 0x5e, 0x23, 0x56, 
    0xeb, 0xcd, 0x29, 0x04, 0x18, 0xb8, 0x3a, 0x90, 0x03, 0xee, 0x01, 0x32, 0x90, 0x03, 0x18, 0xae, 
    0x00, 0x00, 0x00, 0x43, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x20, 0x52, 0x6f, 0x6d, 0x00, 0x00, 0x45, 
    0x78, 0x69, 0x74, 0x00, 0x4e, 0x65, 0x78, 0x74, 0x00, 0x50, 0x72, 0x65, 0x76, 0x69, 0x6f, 0x75, 
    0x73, 0x00, 0x57, 0x72, 0x69, 0x74, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x5b, 0x20, 0x5d, 0x00, 0x00, 
    0x01, 0x01, 0x93, 0x03, 0x00, 0x00, 0x77, 0x0f, 0x02, 0xb2, 0x03, 0x86, 0x03, 0x78, 0x0e, 0x02, 
    0x9f, 0x03, 0x56, 0x03, 0x70, 0x0e, 0x09, 0xa9, 0x03, 0x62, 0x03, 0x6e, 0x0e, 0x14, 0xa4, 0x03, 
    0x57, 0x03, 0x31, 0x03, 0x02, 0x9e, 0x03, 0x73, 0x03, 0x32, 0x04, 0x02, 0x9e, 0x03, 0x73, 0x03, 
    0x33, 0x05, 0x02, 0x9e, 0x03, 0x73, 0x03, 0x34, 0x06, 0x02, 0x9e, 0x03, 0x73, 0x03, 0x35, 0x07, 
    0x02, 0x9e, 0x03, 0x73, 0x03, 0x36, 0x08, 0x02, 0x9e, 0x03, 0x73, 0x03, 0x37, 0x09, 0x02, 0x9e, 
    0x03, 0x73, 0x03, 0x38, 0x0a, 0x02, 0x9e, 0x03, 0x73, 0x03, 0x39, 0x0b, 0x02, 0x9e, 0x03, 0x73, 
    0x03, 0x30, 0x0c, 0x02, 0x9e, 0x03, 0x6f, 0x03, 0xff, 0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0xdd, 
    0x36, 0x01, 0x08, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0xfe, 0x00, 
    0xc0, 0x3e, 0xc3, 0x32, 0x04, 0x01, 0xaf, 0x32, 0x05, 0x01, 0x32, 0x06, 0x01, 0xc3, 0x04, 0x01, 
    0x11, 0x00, 0x00, 0xdd, 0x21, 0x00, 0x00, 0xdd, 0x36, 0x01, 0x05, 0xdd, 0x73, 0x02, 0xdd, 0x72, 
    0x03, 0x21, 0x39, 0x05, 0x7b, 0xb2, 0x28, 0x04, 0xcb, 0xbe, 0x18, 0x02, 0xcb, 0xfe, 0xed, 0x53, 
    0x07, 0x05, 0x21, 0x42, 0x2a, 0xdd, 0x75, 0x04, 0xdd, 0x74, 0x05, 0xdd, 0x36, 0x00, 0xff, 0xdd, 
    0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x21, 0x40, 0x05, 0x3a, 0x43, 0x2a, 0xfe, 0x01, 0x28, 0x04, 
    0xcb, 0xfe, 0x18, 0x02, 0xcb, 0xbe, 0x3a, 0x42, 0x2a, 0x47, 0xaf, 0x11, 0x07, 0x00, 0x21, 0x47, 
    0x05, 0xb8, 0x38, 0x04, 0xcb, 0xfe, 0x18, 0x02, 0xcb, 0xbe, 0x19, 0x3c, 0xfe, 0x0a, 0x20, 0xf1, 
    0x3a, 0x42, 0x2a, 0xfe, 0x00, 0x28, 0x16, 0x47, 0x21, 0x44, 0x2a, 0x11, 0x4a, 0x05, 0xc5, 0x01, 
    0x02, 0x00, 0xed, 0xb0, 0x01, 0x05, 0x00, 0xeb, 0x09, 0xeb, 0xc1, 0x10, 0xf1, 0x21, 0x2b, 0x05, 
    0xcd, 0xa9, 0x01, 0x21, 0x2b, 0x05, 0xc3, 0xeb, 0x01, 0x3e, 0x09, 0x18, 0x02, 0xd6, 0x31, 0x06, 
    0x00, 0x4f, 0x21, 0x44, 0x2a, 0x09, 0x09, 0x5e, 0x23, 0x56, 0xeb, 0xc3, 0x8a, 0x08, 0x2a, 0x07, 
    0x05, 0x11, 0x0a, 0x00, 0x19, 0xeb, 0xc3, 0x53, 0x04, 0x2a, 0x07, 0x05, 0x11, 0x0a, 0x00, 0xb7, 
    0xed, 0x52, 0xeb, 0xc3, 0x53, 0x04, 0xc9, 0x00, 0x00, 0x4c, 0x6f, 0x61, 0x64, 0x20, 0x73, 0x6e, 
    0x61, 0x70, 0x73, 0x68, 0x6f, 0x74, 0x00, 0x00, 0x45, 0x78, 0x69, 0x74, 0x00, 0x4e, 0x65, 0x78, 
    0x74, 0x00, 0x50, 0x72, 0x65, 0x76, 0x69, 0x6f, 0x75, 0x73, 0x00, 0x00, 0x01, 0x01, 0x09, 0x05, 
    0x00, 0x00, 0x78, 0x0e, 0x02, 0x18, 0x05, 0x06, 0x05, 0x70, 0x0e, 0x09, 0x22, 0x05, 0xf9, 0x04, 
    0x6e, 0x0e, 0x14, 0x1d, 0x05, 0xee, 0x04, 0x31, 0x03, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x32, 0x04, 
    0x02, 0x17, 0x05, 0xdd, 0x04, 0x33, 0x05, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x34, 0x06, 0x02, 0x17, 
    0x05, 0xdd, 0x04, 0x35, 0x07, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x36, 0x08, 0x02, 0x17, 0x05, 0xdd, 
    0x04, 0x37, 0x09, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x38, 0x0a, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x39, 
    0x0b, 0x02, 0x17, 0x05, 0xdd, 0x04, 0x30, 0x0c, 0x02, 0x17, 0x05, 0xd9, 0x04, 0xff, 0x3a, 0xa8, 
    0x01, 0x32, 0xd4, 0x06, 0xaf, 0x32, 0xf5, 0x06, 0xcd, 0xea, 0x09, 0x11, 0x01, 0x01, 0x21, 0xf6, 
    0x06, 0xcd, 0x6e, 0x09, 0x11, 0x01, 0x03, 0x21, 0x49, 0x07, 0xcd, 0x6e, 0x09, 0x11, 0x01, 0x04, 
    0x06, 0x1e, 0x3e, 0x0f, 0xcd, 0xa2, 0x09, 0x11, 0x01, 0x06, 0x21, 0x4f, 0x07, 0xcd, 0x6e, 0x09, 
    0x11, 0x01, 0x09, 0x21, 0x04, 0x07, 0xcd, 0x6e, 0x09, 0x11, 0x01, 0x0a, 0x21, 0x21, 0x07, 0xcd, 
    0x6e, 0x09, 0x11, 0x01, 0x0b, 0x21, 0x3b, 0x07, 0xcd, 0x6e, 0x09, 0xcd, 0xaa, 0x06, 0x3a, 0xf5, 
    0x06, 0x47, 0x11, 0x01, 0x04, 0x21, 0xd5, 0x06, 0xfe, 0x00, 0x28, 0x08, 0x7e, 0x23, 0x4f, 0xcd, 
    0x78, 0x09, 0x10, 0xf8, 0x0e, 0x5f, 0xcd, 0x78, 0x09, 0xcd, 0x06, 0x0a, 0xfe, 0x18, 0x28, 0x3f, 
    0xfe, 0x13, 0x28, 0x5c, 0xfe, 0x0a, 0x28, 0x3d, 0xfe, 0x1f, 0x30, 0x0e, 0xfe, 0x09, 0x30, 0xe9, 
    0xe6, 0x07, 0x32, 0xd4, 0x06, 0xcd, 0xaa, 0x06, 0x18, 0x20, 0x4f, 0x3a, 0xf5, 0x06, 0xfe, 0x14, 
    0x30, 0x18, 0x21, 0xd5, 0x06, 0x16, 0x00, 0x5f, 0x19, 0x71, 0x16, 0x04, 0x5f, 0x1c, 0xcd, 0x78, 
    0x09, 0x0e, 0x5f, 0xcd, 0x78, 0x09, 0x3c, 0x32, 0xf5, 0x06, 0xcd, 0x12, 0x0a, 0x18, 0xba, 0xcd, 
    0x12, 0x0a, 0x3e, 0x01, 0xc9, 0x3a, 0xf5, 0x06, 0xfe, 0x00, 0x28, 0xee, 0x3d, 0x32, 0xf5, 0x06, 
    0x16, 0x04, 0x3c, 0x5f, 0x0e, 0x5f, 0xcd, 0x78, 0x09, 0x0e, 0x20, 0xcd, 0x78, 0x09, 0x18, 0xda, 
    0x3a, 0xd4, 0x06, 0x32, 0xa8, 0x01, 0xcd, 0x1c, 0x08, 0xaf, 0xdd, 0xe5, 0xdd, 0x21, 0x00, 0x00, 
    0xdd, 0x77, 0x04, 0xdd, 0x36, 0x01, 0x06, 0x3a, 0xf5, 0x06, 0x5f, 0x16, 0x00, 0x21, 0xd5, 0x06, 
    0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0x19, 0x36, 0x00, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 
    0xfe, 0xff, 0x28, 0xf9, 0xdd, 0xe1, 0xfe, 0x00, 0xc8, 0xfe, 0x01, 0x28, 0x03, 0x3e, 0x01, 0xc9, 
    0xcd, 0x98, 0x07, 0xd2, 0x98, 0x05, 0x3e, 0x01, 0x18, 0xc0, 0x3a, 0xd4, 0x06, 0x87, 0x87, 0x87, 
    0x16, 0x00, 0x5f, 0x21, 0x58, 0x07, 0x19, 0x11, 0x09, 0x06, 0xcd, 0x6e, 0x09, 0x3a, 0xd4, 0x06, 
    0x17, 0x17, 0x17, 0xe6, 0x38, 0xfe, 0x20, 0x30, 0x02, 0xf6, 0x07, 0x11, 0x09, 0x06, 0x06, 0x07, 
    0xcd, 0xa2, 0x09, 0xc9, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x53, 0x61, 0x76, 0x65, 0x20, 0x53, 0x6e, 0x61, 0x70, 0x73, 
    0x68, 0x6f, 0x74, 0x00, 0x45, 0x4e, 0x54, 0x45, 0x52, 0x20, 0x74, 0x6f, 0x20, 0x73, 0x61, 0x76, 
    0x65, 0x2c, 0x20, 0x42, 0x52, 0x45, 0x41, 0x4b, 0x20, 0x54, 0x6f, 0x20, 0x65, 0x78, 0x69, 0x74, 
    0x00, 0x53, 0x68, 0x69, 0x66, 0x74, 0x20, 0x31, 0x2d, 0x38, 0x20, 0x74, 0x6f, 0x20, 0x63, 0x68, 
    0x61, 0x6e, 0x67, 0x65, 0x20, 0x73, 0x61, 0x76, 0x65, 0x64, 0x00, 0x62, 0x6f, 0x72, 0x64, 0x65, 
    0x72, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x00, 0x4e, 0x61, 0x6d, 0x65, 0x3a, 0x00, 0x42, 
    0x6f, 0x72, 0x64, 0x65, 0x72, 0x3a, 0x20, 0x00, 0x42, 0x6c, 0x61, 0x63, 0x6b, 0x20, 0x20, 0x00, 
    0x42, 0x6c, 0x75, 0x65, 0x20, 0x20, 0x20, 0x00, 0x52, 0x65, 0x64, 0x20, 0x20, 0x20, 0x20, 0x00, 
    0x4d, 0x61, 0x67, 0x65, 0x6e, 0x74, 0x61, 0x00, 0x47, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x20, 0x00, 
    0x43, 0x79, 0x61, 0x6e, 0x20, 0x20, 0x20, 0x00, 0x59, 0x65, 0x6c, 0x6c, 0x6f, 0x77, 0x20, 0x00, 
    0x57, 0x68, 0x69, 0x74, 0x65, 0x20, 0x20, 0x00, 0x21, 0xa9, 0x07, 0xcd, 0xa9, 0x01, 0x21, 0xa9, 
    0x07, 0xc3, 0xeb, 0x01, 0x37, 0x3f, 0xc9, 0x37, 0xc9, 0x00, 0x01, 0x01, 0xc6, 0x07, 0x00, 0x00, 
    0x00, 0x02, 0x01, 0xe3, 0x07, 0x00, 0x00, 0x79, 0x04, 0x02, 0xf8, 0x07, 0xa7, 0x07, 0x6e, 0x05, 
    0x02, 0x11, 0x08, 0xa4, 0x07, 0xff, 0x41, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x77, 0x69, 0x74, 
    0x68, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x20, 0x65, 0x78, 0x69, 0x73, 
    0x74, 0x73, 0x00, 0x4f, 0x76, 0x65, 0x72, 0x77, 0x72, 0x69, 0x74, 0x65, 0x20, 0x74, 0x68, 0x69, 
    0x73, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x3f, 0x00, 0x59, 0x65, 0x73, 0x2c, 0x20, 0x6f, 0x76, 0x65, 
    0x72, 0x77, 0x72, 0x69, 0x74, 0x65, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x65, 
    0x00, 0x4e, 0x6f, 0x2c, 0x20, 0x63, 0x61, 0x6e, 0x63, 0x65, 0x6c, 0x00, 0xdd, 0xe5, 0xdd, 0x21, 
    0x00, 0x00, 0x21, 0x8e, 0x01, 0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0xdd, 0x36, 0x01, 0x03, 0xdd, 
    0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x11, 0x27, 0x0f, 0x01, 0x00, 0x1b, 
    0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0xdd, 0x71, 0x04, 0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 0x04, 
    0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x06, 0x0c, 0x21, 0x00, 0x5b, 
    0xc5, 0x11, 0x42, 0x2a, 0x01, 0xc0, 0x0d, 0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0xdd, 0x71, 0x04, 
    0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 0x04, 0xed, 0xb0, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 
    0xfe, 0xff, 0x28, 0xf9, 0xc1, 0x10, 0xd9, 0xdd, 0xe1, 0xc9, 0xdd, 0xe5, 0xdd, 0x21, 0x00, 0x00, 
    0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0x21, 0x27, 0x2a, 0xdd, 0x75, 0x04, 0xdd, 0x74, 0x05, 0xdd, 
    0x36, 0x01, 0x01, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x3a, 0x41, 
    0x2a, 0xe6, 0x07, 0xd3, 0xfe, 0x06, 0x08, 0x11, 0x00, 0x40, 0xc5, 0x21, 0x27, 0x0f, 0x01, 0x00, 
    0x18, 0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0xdd, 0x71, 0x04, 0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 
    0x02, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0xed, 0xb0, 0xc1, 0x10, 
    0xd9, 0xdd, 0xe1, 0x3a, 0x27, 0x2a, 0xed, 0x47, 0x3a, 0x40, 0x2a, 0xfe, 0x02, 0x28, 0x04, 0xed, 
    0x56, 0x18, 0x02, 0xed, 0x5e, 0xd9, 0x08, 0x2a, 0x2e, 0x2a, 0xe5, 0xf1, 0x2a, 0x28, 0x2a, 0xed, 
    0x5b, 0x2a, 0x2a, 0xed, 0x4b, 0x2c, 0x2a, 0xd9, 0x08, 0x2a, 0x3c, 0x2a, 0xe5, 0x2a, 0x30, 0x2a, 
    0xed, 0x5b, 0x32, 0x2a, 0xed, 0x4b, 0x34, 0x2a, 0xfd, 0x2a, 0x36, 0x2a, 0xdd, 0x2a, 0x38, 0x2a, 
    0x3a, 0x3a, 0x2a, 0xcb, 0x57, 0x28, 0x08, 0xf1, 0xed, 0x7b, 0x3e, 0x2a, 0xc3, 0x03, 0x01, 0xf1, 
    0xed, 0x7b, 0x3e, 0x2a, 0xf3, 0xc3, 0x06, 0x01, 0x11, 0x3e, 0x3e, 0x21, 0x3c, 0x00, 0x01, 0x04, 
    0x00, 0xed, 0xb0, 0x21, 0x00, 0x3d, 0x3e, 0x3e, 0x77, 0x11, 0x01, 0x3d, 0x01, 0x00, 0x01, 0xed, 
    0xb0, 0xed, 0x57, 0x4f, 0x3e, 0x3d, 0xed, 0x47, 0xfb, 0x76, 0xf3, 0x79, 0xed, 0x47, 0x78, 0xc9, 
    0xf5, 0xc5, 0x06, 0x1a, 0x10, 0xfe, 0x1b, 0x7a, 0xb3, 0x20, 0xf7, 0xc1, 0xf1, 0xc9, 0x7e, 0xb7, 
    0xc8, 0x4f, 0xcd, 0x78, 0x09, 0x23, 0x18, 0xf6, 0xf5, 0xd5, 0xc5, 0xe5, 0x26, 0x00, 0x69, 0x29, 
    0x29, 0x29, 0x01, 0xf7, 0x09, 0x09, 0x7a, 0xe6, 0x07, 0x0f, 0x0f, 0x0f, 0xb3, 0x5f, 0x7a, 0xe6, 
    0x18, 0xf6, 0x40, 0x57, 0x06, 0x08, 0x7e, 0x12, 0x23, 0x14, 0x10, 0xfa, 0xe1, 0xc1, 0xd1, 0xf1, 
    0x1c, 0xc9, 0x21, 0x00, 0x58, 0x6b, 0x1e, 0x00, 0xcb, 0x3a, 0xcb, 0x1b, 0xcb, 0x3a, 0xcb, 0x1b, 
    0xcb, 0x3a, 0xcb, 0x1b, 0x19, 0x77, 0x23, 0x10, 0xfc, 0xc9, 0x21, 0x00, 0x58, 0x6b, 0x1e, 0x00, 
    0xcb, 0x3a, 0xcb, 0x1b, 0xcb, 0x3a, 0xcb, 0x1b, 0xcb, 0x3a, 0xcb, 0x1b, 0x19, 0x7e, 0xa7, 0xe6, 
    0x07, 0xcb, 0x27, 0xcb, 0x27, 0xcb, 0x27, 0x5f, 0x7e, 0xa7, 0xe6, 0x38, 0xcb, 0x3f, 0xcb, 0x3f, 
    0xcb, 0x3f, 0xb3, 0x5f, 0x7e, 0xe6, 0xc0, 0xb3, 0x77, 0xc9, 0x21, 0x00, 0x58, 0x3e, 0x07, 0x77, 
    0x11, 0x01, 0x58, 0x01, 0x1f, 0x02, 0xed, 0xb0, 0xaf, 0x21, 0x00, 0x40, 0x77, 0x11, 0x01, 0x40, 
    0x01, 0xff, 0x0f, 0xed, 0xb0, 0xc9, 0xcd, 0x26, 0x0a, 0xfe, 0x00, 0x28, 0xf9, 0xfe, 0xff, 0x28, 
    0xf5, 0xc9, 0x11, 0xe8, 0x03, 0xcd, 0x60, 0x09, 0xcd, 0x26, 0x0a, 0xfe, 0x00, 0x20, 0xf3, 0x11, 
    0x64, 0x00, 0xcd, 0x60, 0x09, 0xc9, 0x26, 0x00, 0x2e, 0x00, 0x1e, 0x00, 0x06, 0xfe, 0x0e, 0xfe, 
    0xed, 0x78, 0xcb, 0x40, 0x20, 0x07, 0x0f, 0x38, 0x0f, 0x26, 0x01, 0x18, 0x0b, 0xcb, 0x78, 0x20, 
    0x07, 0x1f, 0x1f, 0x38, 0x02, 0xcb, 0xcc, 0x07, 0xc5, 0x06, 0x05, 0x1c, 0x1f, 0x38, 0x0b, 0x4f, 
    0xaf, 0xbd, 0x28, 0x04, 0xc1, 0x3e, 0xff, 0xc9, 0x79, 0x6b, 0x10, 0xef, 0xc1, 0x37, 0xcb, 0x10, 
    0x38, 0xcc, 0xaf, 0xbd, 0xc8, 0xcb, 0x44, 0x28, 0x05, 0x11, 0xa6, 0x0a, 0x18, 0x0c, 0xcb, 0x4c, 
    0x28, 0x05, 0x11, 0xce, 0x0a, 0x18, 0x03, 0x11, 0x7e, 0x0a, 0x26, 0x00, 0x19, 0x7e, 0xc9, 0x7a, 
    0x78, 0x63, 0x76, 0x7e, 0x61, 0x73, 0x64, 0x66, 0x67, 0x71, 0x77, 0x65, 0x72, 0x74, 0x31, 0x32, 
    0x33, 0x34, 0x35, 0x30, 0x39, 0x38, 0x37, 0x36, 0x70, 0x6f, 0x69, 0x75, 0x79, 0x13, 0x6c, 0x6b, 
    0x6a, 0x68, 0x20, 0x6d, 0x6e, 0x62, 0x7e, 0x5a, 0x58, 0x43, 0x56, 0x7e, 0x41, 0x53, 0x44, 0x46, 
    0x47, 0x51, 0x57, 0x45, 0x52, 0x54, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0a, 0x09, 0x08, 0x07, 0x06, 
    0x50, 0x4f, 0x49, 0x55, 0x59, 0x13, 0x4c, 0x4b, 0x4a, 0x48, 0x18, 0x4d, 0x4e, 0x42, 0x7e, 0x3a, 
    0x60, 0x3f, 0x2f, 0x7e, 0x7e, 0x7c, 0x5c, 0x7b, 0x7d, 0x7e, 0x7e, 0x7e, 0x3c, 0x3e, 0x21, 0x40, 
    0x23, 0x24, 0x25, 0x5f, 0x29, 0x28, 0x27, 0x26, 0x22, 0x3b, 0x7e, 0x5d, 0x5b, 0x13, 0x3d, 0x2b, 
    0x2d, 0x5e, 0x20, 0x2e, 0x2c, 0x2a, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 
    0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 
    0x36, 0x7f, 0x36, 0x7f, 0x36, 0x36, 0x00, 0x0c, 0x3f, 0x68, 0x3e, 0x0b, 0x7e, 0x18, 0x00, 0x60, 
    0x66, 0x0c, 0x18, 0x30, 0x66, 0x06, 0x00, 0x38, 0x6c, 0x6c, 0x38, 0x6d, 0x66, 0x3b, 0x00, 0x0c, 
    0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 0x30, 
    0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 0x00, 0x18, 0x7e, 0x3c, 0x7e, 0x18, 0x00, 0x00, 0x00, 
    0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x30, 0x00, 
    0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 
    0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00, 0x3c, 0x66, 0x6e, 0x7e, 0x76, 0x66, 0x3c, 0x00, 0x18, 
    0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x3c, 0x66, 0x06, 0x0c, 0x18, 0x30, 0x7e, 0x00, 0x3c, 
    0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00, 0x0c, 0x1c, 0x3c, 0x6c, 0x7e, 0x0c, 0x0c, 0x00, 0x7e, 
    0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00, 0x1c, 0x30, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00, 0x7e, 
    0x06, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00, 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00, 0x3c, 
    0x66, 0x66, 0x3e, 0x06, 0x0c, 0x38, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 
    0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00, 0x00, 
    0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00, 0x3c, 
    0x66, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00, 0x3c, 0x66, 0x6e, 0x6a, 0x6e, 0x60, 0x3c, 0x00, 0x3c, 
    0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00, 0x3c, 
    0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00, 0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00, 0x7e, 
    0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00, 0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00, 0x3c, 
    0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00, 0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 0x7e, 
    0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x3e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00, 0x66, 
    0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 0x63, 
    0x77, 0x7f, 0x6b, 0x6b, 0x63, 0x63, 0x00, 0x66, 0x66, 0x76, 0x7e, 0x6e, 0x66, 0x66, 0x00, 0x3c, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00, 0x3c, 
    0x66, 0x66, 0x66, 0x6a, 0x6c, 0x36, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0x66, 0x00, 0x3c, 
    0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x66, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 0x63, 
    0x63, 0x6b, 0x6b, 0x7f, 0x77, 0x63, 0x00, 0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00, 0x66, 
    0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00, 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00, 0x7c, 
    0x60, 0x60, 0x60, 0x60, 0x60, 0x7c, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x00, 0x3e, 
    0x06, 0x06, 0x06, 0x06, 0x06, 0x3e, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x1c, 0x36, 0x30, 0x7c, 0x30, 0x30, 0x7e, 0x00, 0x00, 
    0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00, 0x00, 
    0x00, 0x3c, 0x66, 0x60, 0x66, 0x3c, 0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00, 0x00, 
    0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 0x1c, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00, 0x00, 
    0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 0x18, 
    0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x70, 0x60, 
    0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 
    0x00, 0x36, 0x7f, 0x6b, 0x6b, 0x63, 0x00, 0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 
    0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x00, 
    0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x07, 0x00, 0x00, 0x6c, 0x76, 0x60, 0x60, 0x60, 0x00, 0x00, 
    0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00, 0x00, 
    0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 0x00, 
    0x00, 0x63, 0x6b, 0x6b, 0x7f, 0x36, 0x00, 0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00, 0x00, 
    0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c, 0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00, 0x0c, 
    0x18, 0x18, 0x70, 0x18, 0x18, 0x0c, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x30, 
    0x18, 0x18, 0x0e, 0x18, 0x18, 0x30, 0x00, 0x31, 0x6b, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 
    0x42, 0x99, 0xa1, 0xa1, 0x99, 0x42, 0x3c, 
};

const uint32_t NMI_ROM_SIZE = sizeof(NMI_ROM);
const uint32_t EXITNMI = 0x106;
