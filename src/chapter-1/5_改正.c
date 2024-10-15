#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void waiting(), stop();
int wait_mark;

void main()
{
    int p1, p2, stdout;
    // 创建子进程p1
    while ((p1 = fork()) == -1); 
    // 父进程
    if (p1 > 0)
    {
        //创建子进程p2
        while ((p2 = fork()) == -1); 
        // 父进程
        if (p2 > 0)
        {
            wait_mark = 1;
            signal(SIGINT, stop);   /*接收到^c信号，转stop*/
            waiting();              /*等待^c信号*/
            kill(p1, 16);           /*向p1发软中断信号16*/
            kill(p2, 17);           /*向p2发软中断信号17*/
            wait(0);                /*同步*/
            wait(0);                /*同步*/
            printf("Parent process is killed!\n");
            exit(0);
        }
        // 子进程p2
        else
        {
            wait_mark = 1;
            signal(SIGINT, SIG_IGN);    /*忽略 CTRL+C 信号*/ 
            signal(17, stop);           /*接收到软中断信号17，转stop*/
            waiting();
            lockf(stdout, 1, 0);        /*对标准输出进行上锁*/
            printf("Child process 2 is killed by parent!\n");
            lockf(stdout, 0, 0);        /*对标准输出进行解锁*/
            exit(0);
        }
    }
    // 子进程p1
    else
    {
        wait_mark = 1;
        signal(SIGINT, SIG_IGN);    /* 忽略 CTRL+C 信号*/
        signal(16, stop);           /*接收到软中断信号16，转stop*/
        waiting();
        lockf(stdout, 1, 0);        /*对标准输出进行上锁*/
        printf("Child process 1 is killed by parent!\n");
        lockf(stdout, 0, 0);        /*对标准输出进行解锁*/
        exit(0);
    }
}

void waiting()
{
    while (wait_mark != 0);
}

void stop()
{
    wait_mark = 0;
}
