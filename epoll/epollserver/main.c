#include <stdio.h>
#include <server.h>
#include <sys/epoll.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXEVENTS 128

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

int main()
{
    int listen_fd, socket_fd;
    int n, i;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    efd = epoll_create1(0);
    if (efd == -1) {
        error(1,errno,"epoll create failed");
    }

    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET;
    //  注册文件描述符 listen_fd
    if (epoll_ctl(efd,EPOLL_CTL_ADD,listen_fd,&event) == -1) {
        error(1,errno,"epoll_ctl add listen fd failed");
    }

    //会初始化该内存空间 calloc 另该空间所有为0 分配内存块
    events = calloc(MAXEVENTS,sizeof (event));

    while (1) {
        // 正在监听套接字的个数 events 返回需要系统处理的i/o事件
        n = epoll_wait(efd,events,MAXEVENTS,-1);
        printf("epoll_wait wakeup\n");
        for (i = 0; i < n; i++) {
            // 出错，挂起， 不可读 关闭套接字
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events) & EPOLLIN)) {
                fprintf(stderr,"epoll error\n");
                close(events[i].data.fd);
                continue;
            } else if (listen_fd == events[i].data.fd) {
                // 返回的事件listen_fd 需要被处理
                // 标准地址结构
                struct sockaddr_storage ss;
                socklen_t slen = sizeof (ss);
                int fd = accept(listen_fd,(struct sockaddr *) &ss,&slen);
                if (fd < 0) {
                    error(1,errno,"accept failed");
                } else {
                    make_nonblocking(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET;

                    // 加入监听 套接字？？
                    if(epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        error(1,errno,"epoll_ctl_add connection fd failed");
                    }
                }
                continue;
            } else {
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d \n",socket_fd);
                while (1) {
                    char buf[512];
                    if ((n = read(socket_fd, buf, sizeof (buf))) < 0) {
                        if (errno != EAGAIN) {
                            error(1,errno,"read error");
                            close(socket_fd);
                        }
                        break;
                    } else if (n == 0) {
                        close(socket_fd);
                        printf("close the socket %d",socket_fd);
                        break;
                    } else {
                        for (i = 0; i < n; ++i) {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if (write(socket_fd, buf, n) < 0) {
                            error(1, errno, "write error");
                            printf("write successfully %d byte",sizeof (buf));
                        }
                    }
                }
            }
        }
    }

    free(events);
    close(listen_fd);
}
