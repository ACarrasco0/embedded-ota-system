/*
* name: tcp_server_task.h
* Description: This file contains the declarations for the tcp server task
* functionality.
*/

#ifndef TCP_SERVER_TASK_H
#define TCP_SERVER_TASK_H

#include "tcp_frame_server.h"

typedef void (*protocol_handler_t)(int client_fd, frame_t *frame);

void tcp_server_task(void);
void tcp_server_task_with_handler(protocol_handler_t protocol_handler);

#endif /* TCP_SERVER_TASK_H */
