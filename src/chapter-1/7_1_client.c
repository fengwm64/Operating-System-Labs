// Client.c
// 2023.4.23 ON Ubuntu LTS 18

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define MSGKEY 75

struct msgform
{
    long mtype;
    pid_t PID;
    char mtext[1000];
} msg;

int msgqid;
bool flag = 1;
pid_t ser_pid;

void Snd() { flag = 0; }

void client()
{
    // 打开75#消息队列
    msgqid = msgget(MSGKEY, 0777);

    // 发送自己的PID
    msg.mtype = 9999;
    msg.PID = getpid();
    msgsnd(msgqid, &msg, 1024, 0);

    // 接收
    msgrcv(msgqid, &msg, 1024, 9999, 0);
    ser_pid = msg.PID;
    printf("server_pid 为：%d\n\n", ser_pid);

    for (int i = 10; i >= 1; i--)
    {
        msg.mtype = i;
        printf("(client)sent\n");
        // 发送消息
        msgsnd(msgqid, &msg, 1024, 0);
        kill(ser_pid, SIGUSR1);
        // 等待server读取完毕
        while (flag);
        // 复位flag
        flag = 1;
    }
    exit(0);
}

void main()
{
    signal(SIGUSR1, Snd);
    client();
}
