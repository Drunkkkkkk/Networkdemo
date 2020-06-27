#include <stdio.h>
#include <error.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv)
{
    if(argc != 2) {
        //errno 0表示无错误
        error(1,0,"usage: unixstreamserver<local_path>");
    }

    int listenfd,connfd;
    socklen_t clilen;
    struct sockaddr_un cliaddr,servaddr;

    listenfd = socket(AF_LOCAL,SOCK_STREAM,0);
    if (listenfd < 0) {
        error(1,errno,"socket create failed");
    }

    char *local_path = argv[1];
    //??
    unlink(local_path);
    bzero(&servaddr,sizeof (servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path,local_path);

    if(bind(listenfd,(struct sockaddr*)&servaddr,
            sizeof (servaddr))<0) {
        error(1,errno,"bind failed");
    }

    if(listen(listenfd,1024) < 0) {
        error(1,errno,"Listen failed");
    }

    clilen = sizeof (cliaddr);
    if((connfd = accept(listenfd,(struct sockaddr *)&cliaddr,
                        &clilen))<0) {
        if(errno == EINTR)
            error(1,errno,"accept failed");
        else {
            error(1,errno,"accept failed");
        }
    }

    char buf[4096];

    while (1) {
        bzero(buf,sizeof (buf));
        if(read(connfd,buf,4096) == 0) {
            printf("client quit");
            break;
        }
        printf("Receive: %s", buf);

        char send_line[4096];
        bzero(send_line,4096);
        sprintf(send_line,"Hi %s",buf);

        int nbytes = sizeof (send_line);

        if(write(connfd,send_line,nbytes) != nbytes)
            error(1,errno,"write error");
    }

    close(listenfd);
    close(connfd);

    _exit(0);
}
