/*
* name: tcp_client.h
* Description: Header file for the tcp client. It contains the declaration of
* the function that runs the tcp client.
* It is used to test the tcp server functionality.
*/

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

int tcp_client_connect(const char *ip, const char *port);
int tcp_client_send(const char *buffer, int len);
int tcp_client_recv(char *buffer, int len);
void tcp_client_disconnect(void);

#endif