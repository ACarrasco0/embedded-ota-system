/*
* name: tcp_server.c
* Description: Small tcp server for simple transmissions.
*/ 
#include "core/config.h"
#include "core/scheduler.h"
#include "logger/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PORT "4050"  
#define BACKLOG 5   // pending connections to be hold

static int socket_rf = -1;
struct sockaddr_storage their_addr;

STATUS_E tcp_server_init(void){
    /*
    * Initializes the TCP server by creating a socket, binding it to a port, and
    * enabling it to listen for incoming connections. It uses getaddrinfo() to
    * set up the address information and handles any errors that may occur during
    * the initialization process.
    */
    int status = 0;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        log_error("gai error: %s", gai_strerror(status));
        return STATUS_ERROR;
    }

    socket_rf = socket(servinfo->ai_family, servinfo->ai_socktype | SOCK_NONBLOCK, 
        servinfo->ai_protocol);
    if (socket_rf == -1) { 
        log_error("Error creating socket: %s", strerror(errno));
        return STATUS_ERROR;
    }

    status = bind(socket_rf, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status == -1) { 
        log_error("Error binding to the port: %s", strerror(errno));
        return STATUS_ERROR;
    } 

    status = listen(socket_rf, BACKLOG);
    if (status == -1) {
        log_error("Error ennabling listening: %s", strerror(errno));
        return STATUS_ERROR;
    }

    freeaddrinfo(servinfo); // free the linked-list

    log_info("TCP server listening on port %s with fd=%d", PORT, socket_rf);
    return STATUS_OK;
}

STATUS_E tcp_server_deinit(void)
{
    tcp_server_task_running = 0; 
    shutdown(socket_rf, SHUT_RDWR);
    if (socket_rf >= 0) {
        close(socket_rf);
        socket_rf = -1;
    }
    return STATUS_OK;
}

int tcp_server_accept(struct sockaddr_storage *client_addr, socklen_t *addr_size)
{
    /*
    * Accepts incoming connection requests using accept().
    * Returns file descriptor on success, -1 on error.
    */
    int fd = accept(socket_rf, (struct sockaddr *)client_addr, addr_size);
    if (fd == -1) {
        if (!tcp_server_task_running) {
            return -1; // shutdown limpio
        }

        if (errno == EINTR || errno == EBADF) {
            return -1; // socket cerrado durante shutdown
        }

        //log_error("accept failed: %s", strerror(errno));
        return -1; 
    }
    log_info("client connected");

    // Set the client socket to non-blocking mode to avoid blocking on recv()
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    return fd;
}

int tcp_server_recv(int client_fd, char *buffer, int buffer_size)
{
    int bytes = recv(client_fd, buffer, buffer_size, 0);

    if (bytes > 0) {
#ifdef LOF_DEBUGGING
        log_info("received %d bytes", bytes);
        for (int i = 0; i < bytes; i++) {
            printf("%02X ", (unsigned char)buffer[i]);
        }
        printf("\n");
#endif
    }
    else if (bytes == 0) {
        log_info("client disconnected");
    }
    else {
        if (errno == EINTR) {
            log_info("recv interrupted by signal");
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now (normal in non-blocking sockets)
            return -1;
        }
        else {
            log_error("recv error: %s", strerror(errno));
        }
    }

    return bytes;
}

int tcp_server_send(int client_fd, const char *buffer, int bytes)
{
    /*
    * Sends data to client.
    * Returns bytes sent, -1 on error.
    */
    return send(client_fd, buffer, bytes, 0);
}