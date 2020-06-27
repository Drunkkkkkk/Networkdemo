#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <signal.h>

static int count;

//static void sig_int(int signo) {
//    printf("\nreceived %d datagrams\n",count);
//    exit(0);
//}

int main(int argc, char **argv)
{
    int listenfd;
    listenfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(43211);

    int rt1 = bind(listenfd,(struct sockaddr*) &server_addr,sizeof (server_addr));
    if(rt1 < 0) {
        error(1,errno,"bind failed");
    }

    int rt2 = listen(listenfd,1024);
    if (rt2 < 0) {
        error(1,errno,"listen failed");
    }

//    signal(SIGINT,sig_int);
    signal(SIGPIPE,SIG_DFL);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof (client_addr);

    if((connfd = accept(listenfd,(struct sockaddr *)&client_addr,&client_len)) < 0) {
        error(1,errno,"bind failed ");
    }

    char message[4096];
    count = 0;

    for(;;) {
        int n = read(connfd,message,4096);
        if (n < 0) {
            error(1,errno,"error read");
        } else if (n == 0) {
            error(1,0,"client closed \n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n",n,message);
        count++;

        char send_line[4096];
        sprintf(send_line,"Hi, %s\n",message);

        sleep(5);

        int write_nc = send(connfd,send_line,strlen(send_line),0);
        printf("send bytes: %zu \n",write_nc);
        if (write_nc < 0) {
            error(1,errno,"error write");
        }
    }
}
