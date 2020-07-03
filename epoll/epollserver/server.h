#ifndef SERVER_H
#define SERVER_H

#define SERV_PORT 12345

void make_nonblocking(int fd);
int tcp_server_listen(int port);
int tcp_nonblocking_server_listen(int port);
#endif // SERVER_H
