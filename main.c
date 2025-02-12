#include "httpWebServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

int main() {
    
    struct addrinfo hints, *servAddr;
    struct sockaddr_storage cleintAddr;
    socklen_t addrSize;
    int servSock_fd, cleintSock_fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //local host

    if(getaddrinfo(NULL, "80", &hints, &servAddr) != 0) { //localhost, port 80
        printf("Error performing getaddrinfo()");
        exit(1);
    }

    servSock_fd = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol); //create socket
    if(servSock_fd < 0) {
        printf("Invalid socket.");
        freeaddrinfo(servAddr);//free servAddr struct
        exit(1);
    }

    if(bind(servSock_fd, servAddr->ai_addr, servAddr->ai_addrlen) != 0) { //bind socket to address
        printf("Error binding.");
        freeaddrinfo(servAddr);//free servAddr struct
        exit(1);
    }

    if(listen(servSock_fd, 10) != 0) { //backlog is 10
        printf("Error listening.");
        freeaddrinfo(servAddr);//free servAddr struct
        exit(1);
    }
    freeaddrinfo(servAddr);//free servAddr struct
    

    while(1) {
        addrSize = sizeof(cleintAddr);
        cleintSock_fd = accept(servSock_fd, (struct sockaddr *)&cleintAddr, &addrSize);
        if(cleintSock_fd < 0) {
            printf("Invalid Cleint Socket");
            exit(1);
        }

        pid_t pid = fork();

        if(pid == -1) {
            close(cleintSock_fd); //error
        } else if (pid == 0) {
            close(servSock_fd);
            handleRequest(cleintSock_fd);

            exit(0);
        } else {
            close(cleintSock_fd);
            waitpid(pid, NULL, WNOHANG);
        }
    }

    return 0;
}