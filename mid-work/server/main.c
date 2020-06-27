﻿#include <stdio.h>
//#include <server.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

static int count;

char *run_cmd(char *cmd) {
    char *data = malloc(16384);
    bzero(data, sizeof(data));
    FILE *fdp;
    const int max_buffer = 256;
    char buffer[max_buffer];
    fdp = popen(cmd, "r");
    char *data_index = data;
    if (fdp) {
        while (!feof(fdp)) {
            if (fgets(buffer, max_buffer, fdp) != NULL) {
                int len = strlen(buffer);
                memcpy(data_index, buffer, len);
                data_index += len;
            }
        }
        pclose(fdp);
    }
    return data;
}

int main(int argc, char *argv[])
{
//    if(argc != 2) {
//        error(1,errno,"usage:tcpserver<Port>");
//    }

//    int port = atoi(argv[1]);
//    int listen_fd = tcp_server(port);

//    struct sockaddr_in client_addr;
//    socklen_t client_len = sizeof (client_addr);
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listenfd, 1024);
    if (rt2 < 0) {
        error(1, errno, "listen failed ");
    }

    signal(SIGPIPE, SIG_IGN);


    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buf[256];
    count = 0;

    while (1) {
        if((connfd = accept(listenfd,(struct sockaddr *)&client_addr,&client_len)) < 0) {
            error(1,errno,"connect failed");
        }
        while (1) {
            memset(buf,0,sizeof (buf));
            int n = read(connfd,buf,sizeof (buf));
            if (n < 0) {
                error(1,errno,"error read message");
            } else if (n == 0) {
                printf("client closed \n");
                close(connfd);
                break;
            }
            count++;
            buf[n] = 0;
            if (strncmp(buf,"ls",n) == 0) {
                return 1;
            } else if (strncmp(buf,"pwd",n) == 0){
                char buf[256];
                char *result = getcwd(buf,256);
                if (send(connfd,result,strlen(result),0) < 0){
                    return 1;
                }
            } else if (strncmp(buf,"cd ",n) == 0){
                char target[256];
                memset(target,0,sizeof (target));
                memcpy(target,buf + 3,strlen(buf) - 3);
                if(chdir(target) == -1) {
                    printf("change dir failed, %s\n",target);
                }
            } else {
                char *error = "error: unknow input type";
                if(send(connfd,error,strlen(error),0) < 0)
                    return 1;
            }
        }
    }
    exit(0);
}
