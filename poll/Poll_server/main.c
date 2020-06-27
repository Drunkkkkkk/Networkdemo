#include <stdio.h>
#include <server.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <error.h>
#include <errno.h>
#include <string.h>

#define INIT_SIZE 128
#define MAXLINE 1024

int main(int argc,char **argv)
{
    int listen_fd,connected_fd;
    int ready_number;
    ssize_t n;
    char buf[MAXLINE];
    struct sockaddr_in client_addr;

    listen_fd = tcp_server_listen(SERV_PORT);

    //初始化pollfd数组，这个数组的第一个元素是listen_fd，其余的用来记录将要连接的connect_fd

    // 用-1表示这个数组位置还没有被占用
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;
    event_set[0].events = POLLRDNORM; //有普通数据可读

    int i;
    for (i = i;i < INIT_SIZE; i++) {
        event_set[i].fd = -1;
    }

    printf("Poll_serverce is already\n");

    for(;;) {
        //poll 函数保证可以自动忽略 fd 为 -1 的 pollfd timeout: -1 表上在发生I/O 事件发生之前poll 调用一直阻塞
        if ((ready_number = poll(event_set,INIT_SIZE,-1)) < 0) {
            error(1,errno,"poll failed ");
        }

        //二进制用& 进行判断
        if(event_set[0].revents & POLLRDNORM) {
            socklen_t client_len = sizeof (client_len);
            connected_fd = accept(listen_fd, (struct sockaddr *)&client_addr,&client_len);

            for (i = 1; i < INIT_SIZE; i++) {
                if (event_set[i].fd < 0) {
                    event_set[i].fd = connected_fd;
                    //套接字有数据可读
                    event_set[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i == INIT_SIZE) {
                error(1,errno,"can not hold so many ");
            }

            //加速处理 已经完成了所有I/O 任务 ?
            if (--ready_number <= 0)
                continue;
        }

        for(i = 1; i < INIT_SIZE; i++) {
            int socket_fd;
            if((socket_fd = event_set[i].fd) < 0)
                continue;
            if (event_set[i].revents & (POLLRDNORM | POLLERR)) {
                memset(buf,0,MAXLINE);
                if ((n = read(socket_fd, buf, MAXLINE)) > 0 ) {
                    fputs(buf,stdout);
                    fputs("\n",stdout);
                    if (write(socket_fd, buf, n) < 0) {
                        error(1,errno,"write error");
                    }
                } else if (n == 0 || errno == ECONNRESET) {
                    close(socket_fd);
                    event_set[i].fd = -1;
                } else {
                    error(1,errno,"read error");
                }

                if (--ready_number <= 0)
                    break;
            }
        }
    }
}
