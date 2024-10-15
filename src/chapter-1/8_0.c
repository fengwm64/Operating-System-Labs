#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SHMKEY 75

int shmid, i;
int *addr;

void client()
{
    int i;
    shmid = shmget(SHMKEY, 1024, 0777); /*打开共享存储区*/
    addr = shmat(shmid, 0, 0);          /*获得共享存储区首地址*/
    for (i = 9; i >= 0; i--)
    {
        // 同步机制
        // -1 代表获得共享存储区首地址失败 或者 server 正在写入
        while (*addr != -1);
        printf("(client) sent\n");
        // 往首地址里写入一个 int 型数据
        *addr = i;
    }
    exit(0);
}

void server()
{
    shmid = shmget(SHMKEY, 1024, 0777 | IPC_CREAT);/*创建共享存储区*/
    addr = shmat(shmid, 0, 0);                      /*获取首地址*/
    do
    {
        // 同步机制
        // 当 client 更改首地址数据时代表写入完毕可读
        *addr = -1;
        while (*addr == -1);
        printf("(server) received\n");
    } while (*addr);
    shmctl(shmid, IPC_RMID, 0); /*撤消共享存储区，归还资源*/
    exit(0);
}

void main()
{
    printf("\nFWM 32116160100\n\n");
    while ((i = fork()) == -1);
    if (!i)
        server();
    system("ipcs -m");
    while ((i = fork()) == -1);
    if (!i)
        client();
    wait(0);
    wait(0);
}
