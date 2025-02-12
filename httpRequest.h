#ifndef HTTPREQUEST
#define HTTPREQUEST

struct HTTPRequest {
    char requestType[10];
    char filePath[85];
    char protocol[10];
};

#endif