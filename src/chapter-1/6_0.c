#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int pid1, pid2;

void main()
{
    int fd[2];
    char outpipe[100], inpipe[100];
    // 创建一个管道
    pipe(fd);
    // 创建子进程1
    while ((pid1 = fork()) == -1);
    // 子进程1
    if (pid1 == 0)
    {
        // 对管道加锁，实现管道操作互斥
        lockf(fd[1], 1, 0);
        // 把串放入数组outpipe中
        sprintf(outpipe, "child 1 process is sending message!");
        // 向管道写长为50字节的串
        write(fd[1], outpipe, 50);
        // 自我阻塞5秒
        sleep(5);
        // 解锁
        lockf(fd[1], 0, 0);
        exit(0);
    }
    // 父进程
    else
    {
        // 创建子进程2
        while ((pid2 = fork()) == -1);
        // 子进程2
        if (pid2 == 0)
        {
            // 子进程2操作同子1
            lockf(fd[1], 1, 0);
            sprintf(outpipe, "child 2 process is sending message!");
            write(fd[1], outpipe, 50);
            sleep(5);
            lockf(fd[1], 0, 0);
            exit(0);
        }
        // 父进程
        else
        { 
            // 等待子1
            wait(0);      
            // 从管道中读长为50字节的串          
            read(fd[0], inpipe, 50);
            // 输出
            printf("%s\n", inpipe);
            // 等待子2
            wait(0);
            read(fd[0], inpipe, 50);
            printf("%s\n", inpipe);
            exit(0);
        }
    }
}
