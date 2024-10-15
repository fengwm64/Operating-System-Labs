// Server.c
// 2023.4.23 ON Ubuntu LTS 18

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define MSGKEY 75 // 信息队列描述符

// 消息结构
struct msgform
{
    long mtype;       // 消息类型，必须为long类型
    pid_t PID;        // client的PID，同步需要
    char mtext[1000]; // 消息正文
} msg;

int msgqid;
pid_t cli_pid;
bool flag = 1;

void Rcv() { flag = 0; }

void server()
{
    // 创建75#消息队列，权限0777
    msgqid = msgget(MSGKEY, 0777 | IPC_CREAT);

    // 接收 client 的 PID 消息
    msgrcv(msgqid, &msg, 1024, 9999, 0);
    cli_pid = msg.PID;
    printf("client_pid 为：%d\n", cli_pid);

    // 发送PID
    msg.mtype = 9999;
    msg.PID = getpid();
    msgsnd(msgqid, &msg, 1024, 0);

    do
    {
        while (flag);
        flag = 1;
        // 接收消息
        msgrcv(msgqid, &msg, 1024, 0, 0);
        // 输出
        printf("(server)received\n");
        // 告知client可以发送
        kill(cli_pid, SIGUSR1);
    } while (msg.mtype != 1);

    // 删除消息队列，归还资源
    msgctl(msgqid, IPC_RMID, 0);
    exit(0);
}

void main()
{
    signal(SIGUSR1, Rcv);
    server();
}
