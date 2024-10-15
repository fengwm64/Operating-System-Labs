#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void main()
{
    int pid;
    // 创建子进程
    pid = fork(); 
    switch (pid)
    {
    // 创建失败
    case -1: 
        printf("fork fail!\n");
        exit(1);
    // 子进程
    case 0: 
        execl("/bin/ls", "ls", "-1", "-color", NULL);
        printf("exec fail!\n");
        exit(1);
    // 父进程
    default:        
        // 同步
        wait(NULL); 
        printf("ls completed !\n");
        exit(0);
    }
}
