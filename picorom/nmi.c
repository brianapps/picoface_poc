#include "pico/stdlib.h"

const uint8_t NMI_ROM[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0xed, 0x4d, 0x06, 0x02, 0xed, 0x4d, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xed, 0x73, 0x9b, 0x01, 0x22, 0x8d, 0x01, 0xed, 0x53, 
    0x8f, 0x01, 0xed, 0x43, 0x91, 0x01, 0xdd, 0x22, 0x95, 0x01, 0xfd, 0x22, 0x93, 0x01, 0x31, 0x56, 
    0x0a, 0xf5, 0xc5, 0xd5, 0xe5, 0xdd, 0xe5, 0xf5, 0xe1, 0x22, 0x99, 0x01, 0xd9, 0x22, 0x85, 0x01, 
    0xed, 0x53, 0x87, 0x01, 0xed, 0x43, 0x89, 0x01, 0xd9, 0x08, 0xf5, 0xe1, 0x22, 0x8b, 0x01, 0x08, 
    0x3a, 0x48, 0x5c, 0x1f, 0x1f, 0x1f, 0xe6, 0x07, 0x32, 0x9e, 0x01, 0xdd, 0x22, 0x95, 0x01, 0xfd, 
    0x22, 0x93, 0x01, 0xaf, 0x32, 0x1e, 0x01, 0x32, 0x97, 0x01, 0xed, 0x57, 0x32, 0x84, 0x01, 0xe2, 
    0xd2, 0x00, 0x3e, 0x01, 0x32, 0x1e, 0x01, 0x3e, 0x04, 0x32, 0x97, 0x01, 0xcd, 0x81, 0x04, 0x32, 
    0x1d, 0x01, 0x21, 0x00, 0x40, 0x11, 0x58, 0x0a, 0x01, 0x00, 0x1b, 0xed, 0xb0, 0x21, 0x1f, 0x01, 
    0xcd, 0xaa, 0x01, 0x21, 0x1f, 0x01, 0xc3, 0xec, 0x01, 0xcd, 0x6c, 0x03, 0x18, 0x09, 0x18, 0x07, 
    0xcd, 0x2e, 0x02, 0x18, 0xe8, 0x18, 0xe6, 0x21, 0x58, 0x0a, 0x11, 0x00, 0x40, 0x01, 0x00, 0x1b, 
    0xed, 0xb0, 0xdd, 0xe1, 0xe1, 0xd1, 0xc1, 0x3a, 0x1e, 0x01, 0xfe, 0x00, 0x20, 0x07, 0xf1, 0xed, 
    0x7b, 0x9b, 0x01, 0x18, 0x06, 0xf1, 0xed, 0x7b, 0x9b, 0x01, 0xfb, 0xc9, 0x00, 0x00, 0x00, 0x00, 
    0x01, 0x01, 0x4a, 0x01, 0x00, 0x00, 0xf3, 0x03, 0x02, 0x53, 0x01, 0xe9, 0x00, 0x6c, 0x04, 0x02, 
    0x61, 0x01, 0xf0, 0x00, 0x72, 0x05, 0x02, 0x6f, 0x01, 0xf5, 0x00, 0x70, 0x06, 0x02, 0x7f, 0x01, 
    0x00, 0x00, 0x78, 0x07, 0x02, 0x7a, 0x01, 0xee, 0x00, 0xff, 0x70, 0x69, 0x63, 0x6f, 0x46, 0x61, 
    0x63, 0x65, 0x00, 0x53, 0x61, 0x76, 0x65, 0x20, 0x73, 0x6e, 0x61, 0x70, 0x73, 0x68, 0x6f, 0x74, 
    0x00, 0x4c, 0x6f, 0x61, 0x64, 0x20, 0x73, 0x6e, 0x61, 0x70, 0x73, 0x68, 0x6f, 0x74, 0x00, 0x43, 
    0x68, 0x61, 0x6e, 0x67, 0x65, 0x20, 0x52, 0x4f, 0x4d, 0x00, 0x45, 0x78, 0x69, 0x74, 0x00, 0x50, 
    0x6f, 0x6b, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 
    0xb9, 0xc0, 0xcd, 0xeb, 0x04, 0xcd, 0x43, 0x05, 0xbf, 0xc9, 0xe5, 0xcd, 0x1b, 0x05, 0xe1, 0x7e, 
    0xfe, 0xff, 0xc8, 0xcb, 0x7f, 0x28, 0x06, 0x11, 0x07, 0x00, 0x19, 0x18, 0xf2, 0x23, 0x56, 0x23, 
    0x5e, 0x23, 0xfe, 0x00, 0x28, 0x13, 0xfe, 0x61, 0x38, 0x06, 0xfe, 0x7b, 0x30, 0x02, 0xc6, 0xe0, 
    0x4f, 0xcd, 0xc1, 0x04, 0x0e, 0x2d, 0xcd, 0xc1, 0x04, 0x4e, 0x23, 0x46, 0x23, 0x23, 0x23, 0x78, 
    0xb1, 0x28, 0xcc, 0xe5, 0x60, 0x69, 0xcd, 0xb7, 0x04, 0xe1, 0x18, 0xc3, 0xe5, 0xcd, 0x37, 0x05, 
    0xe1, 0xe5, 0x4f, 0x7e, 0xfe, 0xff, 0x28, 0x0d, 0xcb, 0x7f, 0x20, 0x03, 0xb9, 0x28, 0x0c, 0x11, 
    0x07, 0x00, 0x19, 0x18, 0xee, 0xcd, 0x43, 0x05, 0xe1, 0x18, 0xe1, 0xc5, 0x23, 0x56, 0x23, 0x5e, 
    0x23, 0x23, 0x23, 0x4e, 0x23, 0x46, 0xc5, 0xd5, 0xcd, 0xeb, 0x04, 0xcd, 0x43, 0x05, 0xd1, 0xcd, 
    0xeb, 0x04, 0xe1, 0xc1, 0x7c, 0xb5, 0x28, 0x03, 0xd1, 0x79, 0xe9, 0xe1, 0x18, 0xbe, 0x11, 0x00, 
    0x00, 0xdd, 0x21, 0x00, 0x00, 0xdd, 0x36, 0x01, 0x05, 0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0x21, 
    0x17, 0x03, 0x7b, 0xb2, 0x28, 0x04, 0xcb, 0xbe, 0x18, 0x02, 0xcb, 0xfe, 0xed, 0x53, 0xe5, 0x02, 
    0x21, 0x73, 0x25, 0xdd, 0x75, 0x04, 0xdd, 0x74, 0x05, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 
    0xfe, 0xff, 0x28, 0xf9, 0x21, 0x1e, 0x03, 0x3a, 0x74, 0x25, 0xfe, 0x01, 0x28, 0x04, 0xcb, 0xfe, 
    0x18, 0x02, 0xcb, 0xbe, 0x3a, 0x73, 0x25, 0x47, 0xaf, 0x11, 0x07, 0x00, 0x21, 0x25, 0x03, 0xb8, 
    0x38, 0x04, 0xcb, 0xfe, 0x18, 0x02, 0xcb, 0xbe, 0x19, 0x3c, 0xfe, 0x0a, 0x20, 0xf1, 0x3a, 0x73, 
    0x25, 0xfe, 0x00, 0x28, 0x16, 0x47, 0x21, 0x75, 0x25, 0x11, 0x28, 0x03, 0xc5, 0x01, 0x02, 0x00, 
    0xed, 0xb0, 0x01, 0x05, 0x00, 0xeb, 0x09, 0xeb, 0xc1, 0x10, 0xf1, 0x21, 0x09, 0x03, 0xcd, 0xaa, 
    0x01, 0x21, 0x09, 0x03, 0xc3, 0xec, 0x01, 0x3e, 0x09, 0x18, 0x02, 0xd6, 0x31, 0x06, 0x00, 0x4f, 
    0x21, 0x75, 0x25, 0x09, 0x09, 0x5e, 0x23, 0x56, 0xeb, 0xc3, 0xda, 0x03, 0x2a, 0xe5, 0x02, 0x11, 
    0x0a, 0x00, 0x19, 0xeb, 0xc3, 0x31, 0x02, 0x2a, 0xe5, 0x02, 0x11, 0x0a, 0x00, 0xb7, 0xed, 0x52, 
    0xeb, 0xc3, 0x31, 0x02, 0xc9, 0x00, 0x00, 0x4c, 0x6f, 0x61, 0x64, 0x20, 0x73, 0x6e, 0x61, 0x70, 
    0x73, 0x68, 0x6f, 0x74, 0x00, 0x00, 0x45, 0x78, 0x69, 0x74, 0x00, 0x4e, 0x65, 0x78, 0x74, 0x00, 
    0x50, 0x72, 0x65, 0x76, 0x69, 0x6f, 0x75, 0x73, 0x00, 0x00, 0x01, 0x01, 0xe7, 0x02, 0x00, 0x00, 
    0x78, 0x0e, 0x02, 0xf6, 0x02, 0xe4, 0x02, 0x70, 0x0e, 0x09, 0x00, 0x03, 0xd7, 0x02, 0x6e, 0x0e, 
    0x14, 0xfb, 0x02, 0xcc, 0x02, 0x31, 0x03, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x32, 0x04, 0x02, 0xf5, 
    0x02, 0xbb, 0x02, 0x33, 0x05, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x34, 0x06, 0x02, 0xf5, 0x02, 0xbb, 
    0x02, 0x35, 0x07, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x36, 0x08, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x37, 
    0x09, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x38, 0x0a, 0x02, 0xf5, 0x02, 0xbb, 0x02, 0x39, 0x0b, 0x02, 
    0xf5, 0x02, 0xbb, 0x02, 0x30, 0x0c, 0x02, 0xf5, 0x02, 0xb7, 0x02, 0xff, 0xdd, 0xe5, 0xdd, 0x21, 
    0x00, 0x00, 0x21, 0x84, 0x01, 0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0xdd, 0x36, 0x01, 0x03, 0xdd, 
    0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x11, 0x58, 0x0a, 0x01, 0x00, 0x1b, 
    0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0xdd, 0x71, 0x04, 0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 0x04, 
    0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x06, 0x0c, 0x21, 0x00, 0x5b, 
    0xc5, 0x11, 0x73, 0x25, 0x01, 0xc0, 0x0d, 0xdd, 0x73, 0x02, 0xdd, 0x72, 0x03, 0xdd, 0x71, 0x04, 
    0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 0x04, 0xed, 0xb0, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 
    0xfe, 0xff, 0x28, 0xf9, 0xc1, 0x10, 0xd9, 0xdd, 0xe1, 0xc9, 0xdd, 0xe5, 0xdd, 0x21, 0x00, 0x00, 
    0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 0x21, 0x58, 0x25, 0xdd, 0x75, 0x04, 0xdd, 0x74, 0x05, 0xdd, 
    0x36, 0x01, 0x01, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 0x00, 0xfe, 0xff, 0x28, 0xf9, 0x06, 0x08, 
    0x11, 0x00, 0x40, 0xc5, 0x21, 0x58, 0x0a, 0x01, 0x00, 0x18, 0xdd, 0x75, 0x02, 0xdd, 0x74, 0x03, 
    0xdd, 0x71, 0x04, 0xdd, 0x70, 0x05, 0xdd, 0x36, 0x01, 0x02, 0xdd, 0x36, 0x00, 0xff, 0xdd, 0x7e, 
    0x00, 0xfe, 0xff, 0x28, 0xf9, 0xed, 0xb0, 0xc1, 0x10, 0xd9, 0xdd, 0xe1, 0x3a, 0x58, 0x25, 0xed, 
    0x47, 0x3a, 0x71, 0x25, 0xfe, 0x02, 0x28, 0x04, 0xed, 0x56, 0x18, 0x02, 0xed, 0x5e, 0xd9, 0x08, 
    0x2a, 0x5f, 0x25, 0xe5, 0xf1, 0x2a, 0x59, 0x25, 0xed, 0x5b, 0x5b, 0x25, 0xed, 0x4b, 0x5d, 0x25, 
    0xd9, 0x08, 0x2a, 0x6d, 0x25, 0xe5, 0x2a, 0x61, 0x25, 0xed, 0x5b, 0x63, 0x25, 0xed, 0x4b, 0x65, 
    0x25, 0xfd, 0x2a, 0x67, 0x25, 0xdd, 0x2a, 0x69, 0x25, 0x3a, 0x6b, 0x25, 0xcb, 0x57, 0x28, 0x08, 
    0xf1, 0xed, 0x7b, 0x6f, 0x25, 0xc3, 0x1a, 0x01, 0xf1, 0xed, 0x7b, 0x6f, 0x25, 0xf3, 0xc3, 0x1b, 
    0x01, 0x11, 0x3e, 0x3e, 0x21, 0x3c, 0x00, 0x01, 0x04, 0x00, 0xed, 0xb0, 0x21, 0x00, 0x3d, 0x3e, 
    0x3e, 0x77, 0x11, 0x01, 0x3d, 0x01, 0x00, 0x01, 0xed, 0xb0, 0xed, 0x57, 0x4f, 0x3e, 0x3d, 0xed, 
    0x47, 0xfb, 0x76, 0xf3, 0x79, 0xed, 0x47, 0x78, 0xc9, 0xf5, 0xc5, 0x06, 0x1a, 0x10, 0xfe, 0x1b, 
    0x7a, 0xb3, 0x20, 0xf7, 0xc1, 0xf1, 0xc9, 0x7e, 0xb7, 0xc8, 0x4f, 0xcd, 0xc1, 0x04, 0x23, 0x18, 
    0xf6, 0xf5, 0xd5, 0xc5, 0xe5, 0x26, 0x00, 0x69, 0x29, 0x29, 0x29, 0x01, 0x28, 0x05, 0x09, 0x7a, 
    0xe6, 0x07, 0x0f, 0x0f, 0x0f, 0xb3, 0x5f, 0x7a, 0xe6, 0x18, 0xf6, 0x40, 0x57, 0x06, 0x08, 0x7e, 
    0x12, 0x23, 0x14, 0x10, 0xfa, 0xe1, 0xc1, 0xd1, 0xf1, 0x1c, 0xc9, 0x21, 0x00, 0x58, 0x6b, 0x1e, 
    0x00, 0xcb, 0x3a, 0xcb, 0x1b, 0xcb, 0x3a, 0xcb, 0x1b, 0xcb, 0x3a, 0xcb, 0x1b, 0x19, 0x7e, 0xa7, 
    0xe6, 0x07, 0xcb, 0x27, 0xcb, 0x27, 0xcb, 0x27, 0x5f, 0x7e, 0xa7, 0xe6, 0x38, 0xcb, 0x3f, 0xcb, 
    0x3f, 0xcb, 0x3f, 0xb3, 0x5f, 0x7e, 0xe6, 0xc0, 0xb3, 0x77, 0xc9, 0x21, 0x00, 0x58, 0x3e, 0x07, 
    0x77, 0x11, 0x01, 0x58, 0x01, 0xff, 0x01, 0xed, 0xb0, 0xaf, 0x21, 0x00, 0x40, 0x77, 0x11, 0x01, 
    0x40, 0x01, 0xff, 0x0f, 0xed, 0xb0, 0xc9, 0xcd, 0x57, 0x05, 0xfe, 0x00, 0x28, 0xf9, 0xfe, 0xff, 
    0x28, 0xf5, 0xc9, 0x11, 0xe8, 0x03, 0xcd, 0xa9, 0x04, 0xcd, 0x57, 0x05, 0xfe, 0x00, 0x20, 0xf3, 
    0x11, 0x64, 0x00, 0xcd, 0xa9, 0x04, 0xc9, 0x26, 0x00, 0x2e, 0x00, 0x1e, 0x00, 0x06, 0xfe, 0x0e, 
    0xfe, 0xed, 0x78, 0xcb, 0x40, 0x20, 0x07, 0x0f, 0x38, 0x0f, 0x26, 0x01, 0x18, 0x0b, 0xcb, 0x78, 
    0x20, 0x07, 0x1f, 0x1f, 0x38, 0x02, 0xcb, 0xcc, 0x07, 0xc5, 0x06, 0x05, 0x1c, 0x1f, 0x38, 0x0b, 
    0x4f, 0xaf, 0xbd, 0x28, 0x04, 0xc1, 0x3e, 0xff, 0xc9, 0x79, 0x6b, 0x10, 0xef, 0xc1, 0x37, 0xcb, 
    0x10, 0x38, 0xcc, 0xaf, 0xbd, 0xc8, 0xcb, 0x44, 0x28, 0x05, 0x11, 0xd7, 0x05, 0x18, 0x0c, 0xcb, 
    0x4c, 0x28, 0x05, 0x11, 0xff, 0x05, 0x18, 0x03, 0x11, 0xaf, 0x05, 0x26, 0x00, 0x19, 0x7e, 0xc9, 
    0x7a, 0x78, 0x63, 0x76, 0x7e, 0x61, 0x73, 0x64, 0x66, 0x67, 0x71, 0x77, 0x65, 0x72, 0x74, 0x31, 
    0x32, 0x33, 0x34, 0x35, 0x30, 0x39, 0x38, 0x37, 0x36, 0x70, 0x6f, 0x69, 0x75, 0x79, 0x13, 0x6c, 
    0x6b, 0x6a, 0x68, 0x20, 0x6d, 0x6e, 0x62, 0x7e, 0x5a, 0x58, 0x43, 0x56, 0x7e, 0x41, 0x53, 0x44, 
    0x46, 0x47, 0x51, 0x57, 0x45, 0x52, 0x54, 0x31, 0x32, 0x33, 0x34, 0x35, 0x30, 0x39, 0x38, 0x37, 
    0x36, 0x50, 0x4f, 0x49, 0x55, 0x59, 0x13, 0x4c, 0x4b, 0x4a, 0x48, 0x10, 0x4d, 0x4e, 0x42, 0x7e, 
    0x3a, 0x60, 0x3f, 0x2f, 0x7e, 0x7e, 0x7c, 0x5c, 0x7b, 0x7d, 0x7e, 0x7e, 0x7e, 0x3c, 0x3e, 0x21, 
    0x40, 0x23, 0x24, 0x25, 0x5f, 0x29, 0x28, 0x27, 0x26, 0x22, 0x3b, 0x7e, 0x5d, 0x5b, 0x13, 0x3d, 
    0x2b, 0x2d, 0x5e, 0x20, 0x2e, 0x2c, 0x2a, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x36, 0x36, 0x7f, 0x36, 0x7f, 0x36, 0x36, 0x00, 0x0c, 0x3f, 0x68, 0x3e, 0x0b, 0x7e, 0x18, 0x00, 
    0x60, 0x66, 0x0c, 0x18, 0x30, 0x66, 0x06, 0x00, 0x38, 0x6c, 0x6c, 0x38, 0x6d, 0x66, 0x3b, 0x00, 
    0x0c, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 
    0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 0x00, 0x18, 0x7e, 0x3c, 0x7e, 0x18, 0x00, 0x00, 
    0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x30, 
    0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 
    0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00, 0x3c, 0x66, 0x6e, 0x7e, 0x76, 0x66, 0x3c, 0x00, 
    0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x3c, 0x66, 0x06, 0x0c, 0x18, 0x30, 0x7e, 0x00, 
    0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00, 0x0c, 0x1c, 0x3c, 0x6c, 0x7e, 0x0c, 0x0c, 0x00, 
    0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00, 0x1c, 0x30, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00, 
    0x7e, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00, 0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00, 
    0x3c, 0x66, 0x66, 0x3e, 0x06, 0x0c, 0x38, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 
    0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00, 
    0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00, 
    0x3c, 0x66, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00, 0x3c, 0x66, 0x6e, 0x6a, 0x6e, 0x60, 0x3c, 0x00, 
    0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00, 
    0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00, 0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00, 
    0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00, 0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00, 
    0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00, 0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00, 
    0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x3e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00, 
    0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 
    0x63, 0x77, 0x7f, 0x6b, 0x6b, 0x63, 0x63, 0x00, 0x66, 0x66, 0x76, 0x7e, 0x6e, 0x66, 0x66, 0x00, 
    0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00, 
    0x3c, 0x66, 0x66, 0x66, 0x6a, 0x6c, 0x36, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0x66, 0x00, 
    0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
    0x63, 0x63, 0x6b, 0x6b, 0x7f, 0x77, 0x63, 0x00, 0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00, 
    0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00, 0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00, 
    0x7c, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7c, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x00, 
    0x3e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3e, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x1c, 0x36, 0x30, 0x7c, 0x30, 0x30, 0x7e, 0x00, 
    0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00, 
    0x00, 0x00, 0x3c, 0x66, 0x60, 0x66, 0x3c, 0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00, 
    0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 0x1c, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00, 
    0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 
    0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x70, 
    0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 
    0x00, 0x00, 0x36, 0x7f, 0x6b, 0x6b, 0x63, 0x00, 0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00, 
    0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 
    0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x07, 0x00, 0x00, 0x6c, 0x76, 0x60, 0x60, 0x60, 0x00, 
    0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00, 
    0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00, 
    0x00, 0x00, 0x63, 0x6b, 0x6b, 0x7f, 0x36, 0x00, 0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00, 
    0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c, 0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00, 
    0x0c, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0c, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 
    0x30, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x30, 0x00, 0x31, 0x6b, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x3c, 0x42, 0x99, 0xa1, 0xa1, 0x99, 0x42, 0x3c, 
};

const uint32_t NMI_ROM_SIZE = sizeof(NMI_ROM);
const uint32_t EXITNMI = 0x11b;
