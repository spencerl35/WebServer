#include "HTTPResponse.h"
#include "httpRequest.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void sendResponse(struct HTTPResponse* response, int socket) {
    ssize_t bytesSentHeader = send(socket, response->headerRawData, response->headerRawDataSize, 0);
    if(bytesSentHeader == -1) {
        printf("Error sending header.");
        exit(1);
    }
    ssize_t bytesSentBody = send(socket, response->body, response->bodySize, 0);
    if(bytesSentBody == -1) {
        printf("Error sending body.");
        exit(1);
    }

    free(response->headerRawData);
    if(response->header.status == 200) { //response body is on the stack if its an error
        free(response->body);
    }

    if(close(socket) == -1) {
        printf("Error closing the socket.");
        exit(1);
    }
    exit(0);
}

void setRawHeader(struct HTTPResponse* response) {
    int size = 0;
    size += strlen(response->header.server);
    size += strlen(response->header.date);
    size += strlen(response->header.contentLength);
    size += strlen(response->header.contentType);
    size += 115; //size of all the static charcters in the header
    if(response->header.status == 200) {
        size += 5;
    } else if(response->header.status == 404) {
        size += 12;
    } else {
        size += 19;
    }
    response->headerRawData = malloc(size);
    if(response->headerRawData == NULL) {
        printf("Error allocating memory for header.");
        exit(1);
    }
    response->headerRawDataSize = size;

    sprintf((char*)response->headerRawData, "HTTP/1.1 %d", response->header.status);
    if(response->header.status == 200) {
        strcat((char*)response->headerRawData, " OK\r\n");
    } else if(response->header.status == 404) {
        strcat((char*)response->headerRawData, " Not Found\r\n");
    } else {
        strcat((char*)response->headerRawData, " Method Not Found\r\n");
    }
    strcat((char*)response->headerRawData, "Date: ");
    strcat((char*)response->headerRawData, response->header.date);
    strcat((char*)response->headerRawData, "\r\n");
    strcat((char*)response->headerRawData, "Server: ");
    strcat((char*)response->headerRawData, response->header.server);
    strcat((char*)response->headerRawData, "\r\n");
    strcat((char*)response->headerRawData, "Content-Length: ");
    strcat((char*)response->headerRawData, response->header.contentLength);
    strcat((char*)response->headerRawData, "\r\n");
    strcat((char*)response->headerRawData, "Content-Type: ");
    strcat((char*)response->headerRawData, response->header.contentType);
    strcat((char*)response->headerRawData, "\r\n");
    strcat((char*)response->headerRawData, "Content-Disposition: inline\r\n");
    strcat((char*)response->headerRawData, "Connection: Closed\r\n\r\n");
}

void processRequest(uint8_t* requestData, struct HTTPRequest* request) {
    char* token;
    const char delimiter[] = " \r";
    token = strtok((char*)requestData, delimiter);
    strcpy(request->requestType, token);
    token = strtok(NULL, delimiter);
    strcpy(request->filePath, token);
    token = strtok(NULL, delimiter);
    strcpy(request->protocol, token);
    token = strtok(NULL, delimiter);
}

void setTime(struct HTTPResponse* response) {
    time_t t = time(NULL);
    char* timePtr = strtok(ctime(&t), "\n");
    strcpy(response->header.date, timePtr);
}

void setServer(struct HTTPResponse* response) {
    gethostname(response->header.server, sizeof(response->header.server));
}

void setContentType(struct HTTPResponse* response, struct HTTPRequest* request) {
    char* type = strrchr(request->filePath, '.'); //points to the string after the last . in the filepath

    if(strcmp(type, ".html") == 0) {
        strcpy(response->header.contentType, "text/html");
    } else if(strcmp(type, ".jpeg") == 0) {
        strcpy(response->header.contentType, "image/jpeg");
    } else {
        strcpy(response->header.contentType, "text/plain");
    }
}

void requestError(struct HTTPResponse* response, int socket) {
    if(response->header.status == 404) {
        response->bodySize = 191;
        response->body = (uint8_t*)"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>404</title></head><body><p>404: File Not Found</p></body></html>";
    } else {
        response->bodySize = 204;
        response->body = (uint8_t*)"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>405</title></head><body><p>405: Method Not Allowed</p></body></html>";
    }
    strcpy(response->header.contentType, "text/html");
    sprintf((char*)response->header.contentLength, "%zu" ,strlen((char*)response->body));
    setRawHeader(response);
    sendResponse(response, socket);
}

void setStatusCode(struct HTTPResponse* response, struct HTTPRequest* request, int socket) {
    char fullpath[85] = ".";
    strcat(fullpath, request->filePath);
    if(access(fullpath, F_OK) == -1) {
        response->header.status = 404;
        requestError(response, socket);
    } else if(strcmp((request->requestType), "GET") != 0) {
        response->header.status = 405;
        requestError(response, socket);
    } else {
        response->header.status = 200;
    }
}

void setBodyContent(struct HTTPResponse* response, struct HTTPRequest* request) {
    char fullpath[85] = "./";
    strcat(fullpath, request->filePath);
    FILE* fp = fopen(fullpath, "rb");
    if(fp == NULL) {
        printf("Error opening requested file.");
        exit(1);
    }

    if(fseek(fp, 0L, SEEK_END) != 0) {
        printf("Error executing fseek().");
        exit(1);
    }
    long size = ftell(fp);
    response->bodySize = size;
    if(fseek(fp, 0L, SEEK_SET) != 0) {
        printf("Error executing fseek().");
        exit(1);
    }
    response->body = malloc(size);
    if(response->body == NULL) {
        printf("Error allocating memory for response body.");
        exit(1);
    }
    if(fread(response->body, size, 1, fp) != 1) {
        printf("Error reading file.");
        exit(1);
    }
    if(fclose(fp) == EOF) {
        printf("Error closing file.");
        exit(1);
    }
}

void setContentLength(struct HTTPResponse* response) {
    sprintf((char*)response->header.contentLength, "%zu", response->bodySize);
}

void handleRequest(int socket) {
    uint8_t requestData[1000];
    struct HTTPRequest request;
    if(recv(socket, requestData, sizeof(requestData), 0) == -1) {
        printf("Error receiving request.");
        exit(1);
    }
    processRequest(requestData, &request);

    struct HTTPResponse response;
    setTime(&response);
    setServer(&response);
    setStatusCode(&response, &request, socket);
    setContentType(&response, &request);
    setBodyContent(&response, &request);
    setContentLength(&response);
    setRawHeader(&response);
    sendResponse(&response , socket);
    exit(1); //should exit in sendResponse()
}