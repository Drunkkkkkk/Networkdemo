#include <stdio.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>


typedef struct{
    u_int32_t type;
    char data[1024];
} messageObject;

#define MSG_PING    1
#define MSG_PONG    2
#define MSG_TYPE1   11
#define MSG_TYPE2   21


#define MAXLINE 4096
#define KEEP_ALIVE_TIME 10
#define KEEP_ALIVE_INTERVAL 3
#define KEEP_ALIVE_PROBTIMES 3

int main(int argc,char **argv)
{
    if(argc != 2) {
        error(1,0,"usage:tcpclient<IPaddress>");
    }

    int socket_fd;
    socket_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    socklen_t server_len = sizeof (server_addr);
    int connect_rt = connect(socket_fd,(struct sockaddr*)&server_addr,server_len);
    if(connect_rt < 0) {
        error(1,errno,"connect failed");
    }

    char recv_line[MAXLINE+1];
    int n;

    fd_set readmask;
    fd_set allreads;

    struct timeval tv;
    int heatbeats = 0;

    messageObject messageObject;

    FD_ZERO(&allreads);
    FD_SET(0,&allreads);
    FD_SET(socket_fd,&allreads);
    for(;;) {
        readmask = allreads;
        int rc = select(socket_fd+1,&readmask,NULL,NULL,&tv);
        //等待
        if (rc == 0) {
            if (++heatbeats > KEEP_ALIVE_PROBTIMES) {
                error(1,0,"connection dead\n");
            }
            printf("sending heartbeat #%d\n",heatbeats);
            messageObject.type = htonl(MSG_PING);
            rc = send(socket_fd,(char *)& messageObject,
                      sizeof (messageObject),0);
            if(rc < 0) {
                error(1,errno,"send failure");
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }
        //测试sock是否可读，即是否网络上有数据
        if (FD_ISSET(socket_fd,&readmask)) {
            n = read(socket_fd,recv_line,MAXLINE);
            if (n < 0) {
                error(1,errno,"read error");
            } else if (n == 0) {
                error(1,0,"server terminated \n");
            }
            printf("received hearbeat,make heartbeats to 0 \n");
            heatbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }
}
