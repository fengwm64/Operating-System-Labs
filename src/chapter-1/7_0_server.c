// Server.c
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGKEY 75    // 信息队列描述符

// 消息结构
struct msgform
{
    long mtype;         // 消息类型，必须为long类型
    char mtext[1000];   // 消息正文
} msg;

int msgqid;

void server()
{
    // 创建75#消息队列，权限0777
    msgqid = msgget(MSGKEY, 0777 | IPC_CREAT); 
    do
    {
        // 接收消息
        msgrcv(msgqid, &msg, 1030, 0, 0); 
        // 输出
        printf("(server)received\n");
    } while (msg.mtype != 1);

    // 删除消息队列，归还资源
    msgctl(msgqid, IPC_RMID, 0); 
    exit(0);
}

void main()
{
    server();
}
