// Client.c
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGKEY 75

struct msgform
{
    long mtype;
    char mtext[1000];
} msg;

int msgqid;

void client()
{
    int i;
    // 打开75#消息队列
    msgqid = msgget(MSGKEY, 0777); 
    for (i = 10; i >= 1; i--)
    {
        msg.mtype = i;
        printf("(client)sent\n");
        // 发送消息
        msgsnd(msgqid, &msg, 1024, 0); 
    }
    exit(0);
}

void main()
{
    client();
}
