#include <server.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <signal.h>

int tcp_server(int port) {

    int listen_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof (on));

    int rt1 = bind(listen_fd,&server_addr,sizeof (server_addr));
    if (rt1 < 0) {
        error(1,errno,"bind failed");
    }

    int rt2 = listen(listen_fd,1024);
    if (rt2 < 0) {
        error(1,errno,"listen failed");
    }

    //当程序收到一个RST包时 不会无端退出。
    signal(SIGPIPE, SIG_IGN);

    return listen_fd;
}
