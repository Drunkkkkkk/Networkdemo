#include <stdio.h>
#include <tcp_client.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
        error(1,0,"usage:tcpclient<IPaddress> <Port>");

    int port = atoi(argv[2]);
    int socket_fd = tcp_client(argv[1],port);


    fd_set readmask,allreads;
    FD_ZERO(&allreads);
    FD_SET(0,&allreads);
    FD_SET(socket_fd,&allreads);


    int n;
    char recv_line[MAXLINE],send_line[MAXLINE];

    for(;;) {
        readmask = allreads;

        int s = select(socket_fd+1,&readmask,NULL,NULL,NULL);

        if (s <= 0) {
            error(1,errno,"slect failed");
        }

        if (FD_ISSET(socket_fd,&readmask)) {
            n = read(socket_fd,recv_line,MAXLINE);
            if (n < 0) {
                error(1,errno,"read error");
            } else if (n == 0){
                printf("server!! closed");
                break;
            }
            recv_line[n] = 0;
            fputs(recv_line,stdout);
            fputs("\n",stdout);
        }

        if (FD_ISSET(STDIN_FILENO,&readmask)){
            if (fgets(send_line,MAXLINE,stdin) != NULL) {
                int i = strlen(send_line);
                if (send_line[i - 1] == '\n') {
                    send_line[i - 1] = 0; // 空 = NULL？
                }
                if (strncmp(send_line,"quit",strlen(send_line)) == 0) {
                    //shutdown??
                    if(shutdown(socket_fd,1)) {
                        error(1,errno,"shutdown failed");
                    }
                }
                size_t rt = write(socket_fd,send_line,strlen(send_line));
                if(rt < 0) {
                    error(1,errno,"write failed");
                }
            }
        }
    }
    exit(0);
}

