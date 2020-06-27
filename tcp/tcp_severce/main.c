#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

ssize_t readn(int fd,void *buffer,size_t size) {

    int length = size;
    char *buffer_pointer = buffer;

    while (length > 0) {
        int result = read(fd,buffer_pointer,length);

        /*read函数  -1 出错 0 EOF */
        if (result < 0) {
            //涉及到阻塞，现在不能理解
            if(errno == EINTR)
                continue; //再次调用read
            else
                return -1;
        } else if (result == 0)
            break;   //EOF 表示套接字关闭

        length -= result;
        buffer_pointer += result; // 将结果存入，有什么用？？？ 从下一个地址开始保持？？
    }
    return (size - length);  //返回的是实际读取的字节数
}
void read_data(int sockfd) {
    ssize_t n;
    char buf[1024];

    int time = 0;
    for(;;) {
        /* 格式化输出到流中 */
        fprintf(stdout,"block in read\n");
        if((n = readn(sockfd,buf,1024)) == 0)
            return;

        time++;
        fprintf(stdout,"1K read for %d \n",time);
        usleep(1000); //挂起进程
    }

}



int main()
{
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr,servaddr; //套接字地址结构

    listenfd = socket(AF_INET,SOCK_STREAM,0); // ipv4 tcp
    bzero(&servaddr,sizeof (servaddr)); //清0 ？？

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //internet adress
    servaddr.sin_port = htons(12345);

    //转化为通用套接字 为什么？？？ 难道是因为端口设置了通配地址，在这里要进行转化？？
    bind(listenfd,(struct sockaddr*) &servaddr,sizeof (servaddr));
    // 后面这个参数暂时不理解
    listen(listenfd,1024);

    for(;;) {
        clilen = sizeof (cliaddr);
        connfd = accept(listenfd,(struct sockaddr*) &cliaddr, &clilen);
        read_data(connfd);
        close(connfd); //关闭套接字，监听没有关闭
    }
}
