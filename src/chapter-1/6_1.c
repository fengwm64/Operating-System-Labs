// 2023.4.23 ON Ubuntu LTS 18

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFSIZE 100

int RDY = 0;
int filedes[2];
char outpipe[BUFSIZE], inpipe[BUFSIZE];

void ParentHandler()
{
    // 互斥锁，避免在处理信号时候被打断
    lockf(1, 1, 0);
    // 子进程READY信号
    RDY++;
    lockf(1, 0, 0);
}
void Child1Handler()
{
    sprintf(inpipe, "Child 1 is sending a message!\n");
    write(filedes[1], inpipe, sizeof(inpipe));
}
void Child2Handler()
{
    sprintf(inpipe, "Child 2 is sending a message!\n");
    write(filedes[1], inpipe, sizeof(inpipe));
}

void main()
{
    int pid_1, pid_2;

    // 创建管道
    if (pipe(filedes))
    {
        printf("Failed to create pipe!\n");
        exit(1);
    }

    // 设置文件描述符为非阻塞模式
    // read就不会在读取完数据后阻塞
    int flags = fcntl(filedes[0], F_GETFL, 0);
    fcntl(filedes[0], F_SETFL, flags | O_NONBLOCK);

    /*
	父进程注册信号处理函数
    SIGUSR1 子进程1准备完成
	SIGUSR2 子进程2准备完成

	使用两个不同的信号
	避免在连续接受相同信号时忽略处理
    */
    signal(SIGUSR1, ParentHandler);
    signal(SIGUSR2,ParentHandler);

    // 创建子进程
    while ((pid_1 = fork()) == -1);
    if (pid_1 != 0)
        while ((pid_2 = fork()) == -1);

    switch (pid_1)
    {
    case 0:
        // ---------- 子进程1 ----------
        signal(SIGUSR2, Child1Handler);
	    // 互斥锁，避免两个子进程同时发生信号
        lockf(1, 1, 0);
        // 告诉父亲自己已准备完毕
        kill(getppid(), SIGUSR1);
        lockf(1, 0, 0);
        // 等待父亲信号
        pause();
        exit(0);

    default:
        // ---------- 父进程 ----------
        if (pid_2 != 0)
        {
            // 等待子进程就绪
            while (RDY != 2);
            // 子进程就绪，发写信号
            kill(pid_1, SIGUSR2);
            // 等待子进程1完成信号
            wait(0);
            // 向子进程2发写信号
            kill(pid_2, SIGUSR2);
            // 等待子进程2完成信号
            wait(0);
            // 读取管道
            close(filedes[1]); 			// 关闭管道写端
            while (read(filedes[0], outpipe, sizeof(inpipe)) > 0)
                printf("%s", outpipe);
            close(filedes[0]);
            exit(0);
        }

        // --------- 子进程2 ----------
        signal(SIGUSR2, Child2Handler);
	    // 互斥锁，避免两个子进程同时发生信号
        lockf(1, 1, 0);
        kill(getppid(), SIGUSR2);
        lockf(1, 0, 0);
        pause();
        exit(0);
    }
}
