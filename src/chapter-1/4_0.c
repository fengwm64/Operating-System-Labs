#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

void main()
{
    time_t t;
    FILE *fp;                                 // 建立文件
    fp = fopen("/var/log/Mydaemon.log", "a"); // 打开文件
    // setlinebuf(fp);                           // 将文件输出流设置为行缓冲
    pid_t pid;    // 守护神
    pid = fork(); // 创建子进程
    if (pid > 0)
    {
        printf("Daemon on duty!\n");
        exit(0);
    }
    else if (pid < 0)
    {
        printf("Can't fork!\n");
        exit(-1);
    }
    // 父进程退出，保留子进程作为守护进程运行
    while (1)
    {
        if (fp >= 0)
        {
            sleep(30);
            printf("Daemon on duty!\n");
            t = time(0);                                                                           // 记录当前时间戳
            fprintf(fp, "Now On the FWM's Computer current time is %s\n", asctime(localtime(&t))); // 将时间戳写入日志文件
        }
    }
    fclose(fp); // 关闭文件指针
}
