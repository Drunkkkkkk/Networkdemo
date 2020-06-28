#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SERV_PORT   12345

int main(int argc,char **argv)
{
    if (argc != 2) {
        error(1,0,"usage: nonblockingclient <IPaddress>");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, server_len);
    if (connect_rt < 0) {
        error(1, errno, "connect failed ");
    }
    // 调用close发送RST 强制关闭，跳过TCP四次握手
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    setsockopt(socket_fd,SOL_SOCKET,SO_LINGER,&ling,sizeof (ling));
    close(socket_fd);

    exit(0);
}
