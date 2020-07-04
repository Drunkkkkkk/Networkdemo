#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <error.h>
#include <server.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_NUMBER   4
#define BLOCK_QUEUE_SIZE 100
#define MAX_LINE 16384

typedef struct {
    pthread_t thread_tid; // id
    long thread_count;
} Thread;

Thread *thread_array;

typedef struct {
    int number; // 队列里的描述字最大个数
    int *fd; // 这是一个数组指针
    int front; //头
    int rear; //尾
    pthread_mutex_t mutex; //锁
    pthread_cond_t cond; //条件变量
} block_queue;

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

// 接受数据，转换后发送数据
void loop_echo(int fd) {
    char outbuf[MAX_LINE + 1];
    size_t outbuf_used = 0;
    ssize_t result;
    while (1) {
        char ch;
        result = recv(fd, &ch, 1, 0);
        printf("recv %d bytes\n",result);

        if (result == 0) {
            break;
        } else if (result == -1) {
            error(1, errno, "read error");
            break;
        }

        if (outbuf_used < sizeof (outbuf)) {
            outbuf[outbuf_used++] = rot13_char(ch);
        }

        if (ch == '\n') {
            send(fd, outbuf, outbuf_used, 0);
            printf("send %d bytes \n",outbuf_used);
            outbuf_used = 0;
            continue;
        }


    }
}

//初始化队列
void block_queue_init(block_queue *blockQueue, int number) {
    blockQueue->number = number;
    blockQueue->fd = calloc(number, sizeof (int));
    blockQueue->front = blockQueue->rear = 0;
    pthread_mutex_init(&blockQueue->mutex, NULL);
    pthread_cond_init(&blockQueue->cond, NULL);
}

// 往队列里放置一个描述字 fd
void block_queue_push(block_queue *blockQueue, int fd) {
    // 加锁
    pthread_mutex_lock(&blockQueue->mutex);
    // 加到队尾
    blockQueue->fd[blockQueue->rear] = fd;
    if (++blockQueue->rear == blockQueue->number) {
        // 循环使用队列
        blockQueue->rear = 0;
    }
    printf("push fd %d \n", fd);
    // 通知其他等待读的线程,有新的连接字需要处理 加入一个就发送一个
    pthread_cond_signal(&blockQueue->cond);
    printf("thr cond is %d",blockQueue->cond);
    // 解锁
    pthread_mutex_unlock(&blockQueue->mutex);
}

// 从队列中读取描述字进行处理
int block_queue_pop(block_queue *blockQueue) {
    pthread_mutex_lock(&blockQueue->mutex);
    // 如果队列里面没有新的套接字可以处理，一直等待，直到有新的连接字入队列
    while (blockQueue->front == blockQueue->rear) {
        //无条件等待 会先 解锁 ，休眠等待
        pthread_cond_wait(&blockQueue->cond,&blockQueue->mutex);
    }
    int fd = blockQueue->fd[blockQueue->front];
    // 重置
    if (++blockQueue->front == blockQueue->number) {
        blockQueue->front = 0;
    }
    printf("pop fd %d \n",fd);
    pthread_mutex_unlock(&blockQueue->mutex);
    return fd;
}

// 入口函数
void thread_run(void *arg) {
    pthread_t tid = pthread_self(); // 获取 线程的 id
    pthread_detach(tid); // 将线程设置为分离的

    block_queue *blockQueue = (block_queue *) arg;
    // 循环处理
    while (1) {
        int fd = block_queue_pop(blockQueue); // 弹出套接字
        printf("get fd in thread, fd==%d, tid == %d \n", fd, tid);
        loop_echo(fd);
    }
}

int main(int c, char **v)
{
    int listener_fd = tcp_server_listen(SERV_PORT);

    block_queue blockQueue;
    // 初始化 最大数量为100
    block_queue_init(&blockQueue,BLOCK_QUEUE_SIZE);

    // 创建大小为 4 线程数组 工作线程？？
    thread_array = calloc(THREAD_NUMBER,sizeof (Thread));
    int i;
    // 创建四个线程
    for (i = 0; i < THREAD_NUMBER; i++) {
        pthread_create(&(thread_array[i].thread_tid), NULL, &thread_run, (void *)&blockQueue);

        printf("creat thread %d is %d \n", i, thread_array[i].thread_tid);
    }

    // 阻塞
    while (1) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof (ss);
        int fd = accept(listener_fd, (struct sockaddr*)&ss, &slen);
        if (fd < 0) {
            error(1,errno,"accept failed");
        } else {
            // 加入套接字
            block_queue_push(&blockQueue,fd);
            printf("add socket %d \n",fd);
        }
    }
    return 0;
}

