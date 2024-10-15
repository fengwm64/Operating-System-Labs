#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void main()
{
    int p1, p2;
    fflush(stdout);
    // 创建子进程p1，创建失败会一直尝试
    while ((p1 = fork()) == -1);
    // 子进程p1输出
    if (p1 == 0)
	{
		sleep(1);
        putchar('b');
    }
    else
    {
        // 创建子进程p2，创建失败会一直尝试
        while ((p2 = fork()) == -1);
        // 子进程p2输出
        if (p2 == 0)
        {
        	sleep(1);
            putchar('c');
        }
        // 父进程输出
        else
            putchar('a');
    }
    exit(0);
}
