#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stddef.h>
#include <sys/socket.h>
#include <error.h>

#define MAXLINE 4096

int main(int argc, char **argv)
{
    if(argc != 2) {
        error(1,0,"usage: udpclient <IPaddress>");
    }
    int socket_fd;
    socket_fd = socket(AF_INET,SOCK_DGRAM,0);

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(43211);
    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    socklen_t server_len = sizeof (server_addr);

    struct sockaddr *reply_addr;
    reply_addr = malloc(server_len);

    char send_line[MAXLINE],recv_line[MAXLINE + 1];
    socklen_t len;
    int n;

    //从标准输入流中，向send_line输入MAXLINE大小的数据
    while (fgets(send_line,MAXLINE,stdin) != NULL) {
        int i = strlen(send_line);
        if(send_line[i - 1] == '\n') {
            send_line[i -1] = 0;
        }

        printf("now sending %s\n",send_line);
        size_t rt = sendto(socket_fd,send_line,strlen(send_line),
                           0,(struct sockaddr*)&server_addr,server_len);
        printf("send bytes: %zu \n",rt);

        len = 0;
        n = recvfrom(socket_fd,recv_line,MAXLINE,0,reply_addr,&len);
        if(n < 0)
            error(1,0,"recvfrom failed");
        recv_line[n] = 0;
        //fputs 把字符串输入到指定的流中
        fputs(recv_line,stdout);
        fputs("\n",stdout);
    }
}
