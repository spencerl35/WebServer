#ifndef HTTPWEBSERVER
#define HTTPWEBSERVER

#include "HTTPResponse.h"
#include "httpRequest.h"
#include <stdlib.h>

void processRequest(uint8_t requestData, struct HTTPRequest* request);

void setTime(struct HTTPResponse* response);

void setServer(struct HTTPResponse* response);

void setContentType(struct HTTPResponse* response, struct HTTPRequest* request);

void requestError(struct HTTPResponse* response, int socket);

void setStatusCode(struct HTTPResponse* response, struct HTTPRequest* request, int socket);

void setBodyContent(struct HTTPResponse* response, struct HTTPRequest* request);

void setContentLength(struct HTTPResponse* response);

void setRawHeader(struct HTTPResponse* response);

void sendResponse(struct HTTPResponse* response, int socket);

void handleRequest(int socket);

#endif