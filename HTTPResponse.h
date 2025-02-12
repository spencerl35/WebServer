#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include <stdint.h>
#include <stdlib.h>

struct HTTPResponseHeader {
    int status;
    char date[50];
    char server[100];
    char contentLength[25];
    char contentType[30];
};

struct HTTPResponse {
    struct HTTPResponseHeader header;
    uint8_t *headerRawData;
    size_t headerRawDataSize;
    uint8_t *body;
    size_t bodySize;
};

#endif