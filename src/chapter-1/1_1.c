#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void main()
{
    int p1, p2, i;
    // 创建子进程p1直到成功
    while ((p1 = fork()) == -1);
    // 子进程p1输出
    if (p1 == 0)
        for (i = 0; i < 10; i++)
            printf("daughter  %d\n", i);
    else
    {
        // 创建子进程p2直到成功
        while ((p2 = fork()) == -1);
        // 子进程p2输出
        if (p2 == 0)
            for (i = 0; i < 10; i++)
                printf("son  %d\n", i);
        // 父进程输出
        else
            for (i = 0; i < 10; i++)
                printf("parent  %d\n", i);
    }
}
