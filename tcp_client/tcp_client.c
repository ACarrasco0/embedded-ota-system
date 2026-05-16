/*
* name: tcp_client.c
* Description: Small tcp client for simple transmissions. This tool is not 
* executed in the raspberry, but in a different terminal of the computer.
* It is used to test the tcp server functionality.
*/

#include "tcp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MSG1 "Hello from client 1"
#define MSG2 "Hello from client 2"

static int sockfd = -1;

int tcp_client_connect(const char *ip, const char *port)
{
    /*
    * Connect to the TCP server at the specified IP and port by using 
    * getaddrinfo to resolve the address and then creating a socket based on 
    * the resolved information. Once the socket is connected, the resolved info
    * is freed and the function returns 0 on success.
    */
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);
    return 0;
}

int tcp_client_send(const char *buffer, int len)
{
    if (sockfd == -1) return -1;
    return send(sockfd, buffer, len, 0);
}

int tcp_client_recv(char *buffer, int len)
{
    if (sockfd == -1) return -1;
    return recv(sockfd, buffer, len, 0);
}

void tcp_client_disconnect(void)
{
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

int tcp_client_run(const char *ip, const char *port)
{
    /*
    * Main function to run the TCP client. It first connects to the server and 
    * then sends two messages to the server (It will be used to send OTA frames 
    * in the future). After sending the messages, it disconnects from the server.
    */
    if (tcp_client_connect(ip, port) != 0) {
        return -1;
    }

    // Send basic data to the server and wait for confirmation
    tcp_client_send(MSG1, strlen(MSG1));
    tcp_client_send(MSG2, strlen(MSG2));
    
    printf("Messages sent\n");

    tcp_client_disconnect();
    return 0;
}
/*
int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: ./tcp_client <ip> <port>\n");
        return 1;
    }

    return tcp_client_run(argv[1], argv[2]);
}*/