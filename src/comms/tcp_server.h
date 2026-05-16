/*
* name: tcp_server.h
* Description: This file contains the declarations for the tcp server
* functionality.
*/

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "core/config.h"
#include <sys/socket.h>
#include <netinet/in.h>

STATUS_E tcp_server_init(void);
STATUS_E tcp_server_deinit(void);

int tcp_server_accept(struct sockaddr_storage *client_addr, socklen_t *addr_size);
int tcp_server_recv(int client_fd, char *buffer, int buffer_size);
int tcp_server_send(int client_fd, const char *buffer, int bytes);

#endif /* TCP_SERVER_H */
 