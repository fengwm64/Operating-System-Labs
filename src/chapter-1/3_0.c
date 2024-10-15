#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void main()
{
    int p1, p2, i;
    /*创建子进程p1*/
    while ((p1 = fork()) == -1); 
    if (p1 == 0)
    {
        /*加锁，
        这里第一个参数为stdout（标准输出设备的描述符）*/
        lockf(1, 1, 0); 
        for (i = 0; i < 10; i++)
            printf("daughter %d\n", i);
        /*解锁*/
        lockf(1, 0, 0); 
    }
    else
    {
        /*创建子进程p2*/
        while ((p2 = fork()) == -1); 
        if (p2 == 0)
        {
            /*加锁*/
            lockf(1, 1, 0); 
            for (i = 0; i < 10; i++)
                printf("son %d\n", i);
            /*解锁*/
            lockf(1, 0, 0); 
        }
        else
        {
            /*加锁*/
            lockf(1, 1, 0); 
            for (i = 0; i < 10; i++)
                printf(" parent %d\n", i);
            /*解锁*/
            lockf(1, 0, 0); 
        }
    }
}
