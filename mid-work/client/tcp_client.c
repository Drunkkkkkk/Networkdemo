#include "tcp_client.h"
#include <netinet/in.h>
#include <string.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>

int tcp_client(char *addr,int port) {
    int socket_fd;
    socket_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    //转化为二进制
    inet_pton(AF_INET,addr,&server_addr.sin_addr.s_addr);

    socklen_t server_len = sizeof (server_addr);
    int connect_rt = connect(socket_fd,(struct sockaddr*)&server_addr,server_len);
    if (connect_rt < 0) {
        error(1,errno,"connect failed");
    }

    return socket_fd;
}
