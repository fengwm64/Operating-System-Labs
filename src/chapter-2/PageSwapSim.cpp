/*
 * @Author: 
 * @Date: 2023-04-24 11:49:16
 * @LastEditTime: 2023-05-03 10:28:45
 * @FilePath: /源码/PageSwapSim.cpp
 * @Description: 页面置换算法模拟
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <limits.h>

using namespace std;

// 输出文字颜色相关的宏
#define NONE "\033[m"
#define YELLOW "\033[1;33m"
#define LIGHT_BLUE "\033[1;34m"
#define LIGHT_CYAN "\033[1;36m"

#define MaxSize 10   // 队列最大长度
#define ElemType int // 队列元素的数据类型

// ------------ 循环队列 ------------
typedef struct
{
    ElemType *base; // 动态数组指针
    int front;      // 头指针，指向队头
    int rear;       // 尾指针，指向队尾
} SqQueue;
bool InitQueue(SqQueue &Q);            // 初始化队列
bool DestroyQueue(SqQueue &Q);         // 销毁队列
int QueueLength(SqQueue Q);            // 队列求长
bool EnQueue(SqQueue &Q, ElemType e);  // 入队
bool DeQueue(SqQueue &Q, ElemType &e); // 出队

// ------------ 测试数据 ------------
class PageData
{
public:
    PageData();
    ~PageData();
    void InitData(const string &fname); // 从文件中读取数据初始化数据成员
    void PrintData();                   // 打印初始化后的数据

    int blockNum;  // 内存块数量
    int pageNum;   // 页面数量
    int *pageList; // 页面号序列
} pd;

// ------------ 记录过程 ------------
class PageTrack
{
private:
    int **optTrack;   // 记录opt过程
    int **fifoTrack;  // 记录fifo过程
    int **lruTrack;   // 记录lru过程
    bool **faults;    // 记录缺页情况
    int pos[3];       // 数组指针，[0]opt,[1]fifo...
    double result[3]; // 命中率结果

public:
    PageTrack();
    ~PageTrack();
    void InitTrack();                                        // 初始化结构体的数据成员
    void UpdateTrack(int type, int num, int page, int flag); // 每读取到一个新的页面记录一次
    void PrintTrack(int type);                               // 打印过程
    void HitRate(int type, int miss);                        // 计算命中率
} track;

// ------------------ 公共操作 --------------------
bool CheckFault(int *block, int key); // 用于检查是否缺页

// ----------------- OPT算法部分 -------------------
void OPT_FRead(int *block, int &pagePos); // 内存为空时读入页
int NextVis(int pagePos, int num);        // 计算页面的下一次访问的时间
int OPT_Replace(int pagePos, int *block); // 选择内存块进行替换
void OPT();                               // OPT算法

// ----------------- FIFO算法部分 -------------------
void FIFO_FRead(int *block, int &pagePos, SqQueue &sq); // 内存为空时读入页
int FIFO_Replace(int *block, SqQueue &sq);              // 选择内存块进行替换
void FIFO();                                            // FIFO算法

// ----------------- LRU算法部分 -------------------
void LRU_FRead(int *block, int &pagePos, SqQueue &sq);    // 内存为空时读入页
int LRU_Replace(int *block, SqQueue &sq);                 // 选择内存块进行替换
void LRU_Flash(int pagePos, SqQueue &sq, SqQueue &temSq); // 更新访问时间
void LRU();                                               // LRU算法

int main(int argc, char *argv[])
{
    // 读取参数
    if (argc != 2)
    {
        cout << "ERROR：参数输入错误！请检查后重试。" << endl;
        exit(-1);
    }
    string fname = argv[1];
    cout << "输入参数为：" << fname << endl;

    // 初始化
    pd.InitData(fname);
    pd.PrintData();
    track.InitTrack();

    // 运行算法
    OPT();
    FIFO();
    LRU();

    // 分别打印每个算法的运行过程以及结果
    for (int i = 0; i < 3; i++)
        track.PrintTrack(i);

    exit(0);
}

// ------------ 循环队列 ------------
// 初始化队列
bool InitQueue(SqQueue &Q)
{
    Q.base = new ElemType[MaxSize]; // 分配动态空间
    // 如果base指针为NULL，代表空间分配失败
    if (!Q.base)
    {
        cout << "队列初始化失败！" << endl;
        return false;
    }
    Q.front = Q.rear = 0; // 空间分配成功，初始化队头与队尾指针
    return true;
}
// 销毁队列
bool DestroyQueue(SqQueue &Q)
{
    delete Q.base;        // 释放动态分配的空间
    Q.base = NULL;        // 数组指针置空
    Q.front = Q.rear = 0; // 头尾置0，队列空
    return true;
}
// 队列求长
int QueueLength(SqQueue Q)
{
    return (Q.rear - Q.front + MaxSize) % MaxSize;
}
// 入队
bool EnQueue(SqQueue &Q, ElemType e)
{
    // 判断队是否已满，满则返回错误
    if ((Q.rear + 1) % MaxSize == Q.front)
    {
        cout << "队满！！！" << endl;
        return false;
    }
    Q.base[Q.rear] = e;              // 元素e入队尾
    Q.rear = (Q.rear + 1) % MaxSize; // 根据循环队列逻辑，使尾指针逻辑后移
    return true;
}
// 出队
bool DeQueue(SqQueue &Q, ElemType &e)
{
    // 判断队是否已空，空则返回错误
    if (Q.front == Q.rear)
    {
        cout << "队空！！！" << endl;
        return false;
    }
    e = Q.base[Q.front];
    Q.front = (Q.front + 1) % MaxSize; // 根据循环队列逻辑，使头指针逻辑后移
    return true;
}

// ------------ 测试数据 ------------
PageData::PageData()
{
    int blockNum = -1;
    int pageNum = -1;
    int *pageList = NULL;
}
PageData::~PageData()
{
    blockNum = -1;
    pageNum = -1;
    delete[] pageList;
}
// 从文件中读取数据初始化
void PageData::InitData(const string &fname)
{
    string line;
    ifstream ifs;
    stringstream ss;

    cout << "\nCopyright (c) 2023 by FWM- , All Rights Reserved.\n"
         << endl;

    // 打开文件
    ifs.open(fname);
    if (!ifs.is_open())
    {
        cout << "文件打开失败！" << endl;
        return;
    }

    // 读取文件
    // i 为行号
    for (int i = 1; getline(ifs, line); i++)
    {
        switch (i)
        {
        case 1:
            // 读取块号
            blockNum = stoi(line);
            break;

        case 2:
            // 读取页面序列数量
            pageNum = stoi(line);
            if (pageNum <= 0)
            {
                cout << "文件数据有误！" << endl;
                return;
            }
            // 创建页面序列的存储空间
            pageList = new int[pageNum];
            break;

        case 3:
            int k = 0;    // pageList指针
            int num = -1; // 存储每次从ss里读取的数
            // 初始化ss
            ss.str(line);
            while (ss >> num)
            {
                // 存储到pageList
                pageList[k] = num;
                // 指针后移
                k++;
            }
            break;
        }
    }
    // 关闭文件
    ifs.close();
}
// 打印初始化后的数据
void PageData::PrintData()
{
    cout << "文件内容：\n";
    // 块数量
    cout << blockNum << endl;
    // 读取页面序列数量
    cout << pageNum << endl;
    for (int i = 0; i < pd.pageNum; i++)
        cout << pageList[i] << " ";
    cout << endl;
}

// ------------ 记录过程 ------------
PageTrack::PageTrack()
{
    int **optTrack = NULL;
    int **fifoTrack = NULL;
    int **lruTrack = NULL;
    int **faults = NULL;
}
PageTrack::~PageTrack()
{
    for (int i = 0; i < pd.blockNum; i++)
    {
        if (i < 3)
            delete[] faults[i];
        delete[] optTrack[i];
        delete[] fifoTrack[i];
        delete[] lruTrack[i];
    }
    delete[] optTrack;
    delete[] fifoTrack;
    delete[] lruTrack;
    delete[] faults;
}
// 初始化
void PageTrack::InitTrack()
{
    // 创建三个[blockNum][pageNum]的数组
    optTrack = new int *[pd.blockNum];
    fifoTrack = new int *[pd.blockNum];
    lruTrack = new int *[pd.blockNum];
    faults = new bool *[3];

    for (int i = 0; i < pd.blockNum; i++)
    {
        if (i < 3)
        {
            faults[i] = new bool[pd.pageNum];
            pos[i] = 0;
            result[i] = -1;
        }
        optTrack[i] = new int[pd.pageNum];
        fifoTrack[i] = new int[pd.pageNum];
        lruTrack[i] = new int[pd.pageNum];
        for (int k = 0; k < pd.pageNum; k++)
        {
            if (i < 3)
                faults[i][k] = 0;
            optTrack[i][k] = -1;
            fifoTrack[i][k] = -1;
            lruTrack[i][k] = -1;
        }
    }
}
// 每读取到一个新的页面记录一次
void PageTrack::UpdateTrack(int type, int num, int page, int flag)
{
    // flag:1更改 0复制（没有发生页面置换）
    switch (type)
    {
    case 0:
        // 拷贝*Track[type][pos-1]
        if (pos[type] - 1 >= 0)
            for (int i = 0; i < pd.blockNum; i++)
                optTrack[i][pos[type]] = optTrack[i][pos[type] - 1];
        // 更改
        if (flag)
            optTrack[num][pos[type]] = page;
        break;
    case 1:
        // 拷贝*Track[type][pos-1]
        for (int i = 0; i < pd.blockNum; i++)
            if (pos[type] - 1 >= 0)
                fifoTrack[i][pos[type]] = fifoTrack[i][pos[type] - 1];
        if (flag)
            fifoTrack[num][pos[type]] = page;
        break;
    case 2:
        // 拷贝*Track[type][pos-1]
        for (int i = 0; i < pd.blockNum; i++)
            if (pos[type] - 1 >= 0)
                lruTrack[i][pos[type]] = lruTrack[i][pos[type] - 1];
        if (flag)
            lruTrack[num][pos[type]] = page;
        break;
    }
    if (flag)
        faults[type][pos[type]] = 1;
    pos[type]++;
}
// 打印过程
void PageTrack::PrintTrack(int type)
{
    switch (type)
    {
    case 0:
        cout << LIGHT_CYAN << "\nOPT\n";
        cout << NONE << "序列   |";
        for (int k = 0; k < pd.pageNum; k++)
            cout << setw(2) << pd.pageList[k] << " ";
        cout << endl;
        for (int i = 0; i < pd.blockNum; i++)
        {
            cout << "内存块" << i << "|";
            for (int j = 0; j < pd.pageNum; j++)
            {
                if (optTrack[i][j] == -1)
                {
                    cout << "   ";
                    continue;
                }
                cout << setw(2) << optTrack[i][j] << " ";
            }
            cout << endl;
        }
        break;
    case 1:
        cout << LIGHT_CYAN << "\nFIFO\n";
        cout << NONE << "序列   |";
        for (int k = 0; k < pd.pageNum; k++)
            cout << setw(2) << pd.pageList[k] << " ";
        cout << endl;
        for (int i = 0; i < pd.blockNum; i++)
        {
            cout << "内存块" << i << "|";
            for (int j = 0; j < pd.pageNum; j++)
            {
                if (fifoTrack[i][j] == -1)
                {
                    cout << "   ";
                    continue;
                }
                cout << setw(2) << fifoTrack[i][j] << " ";
            }
            cout << endl;
        }
        break;
    case 2:
        cout << LIGHT_CYAN << "\nLRU\n";
        cout << NONE << "序列   |";
        for (int k = 0; k < pd.pageNum; k++)
            cout << setw(2) << pd.pageList[k] << " ";
        cout << endl;
        for (int i = 0; i < pd.blockNum; i++)
        {
            cout << "内存块" << i << "|";
            for (int j = 0; j < pd.pageNum; j++)
            {
                if (lruTrack[i][j] == -1)
                {
                    cout << "   ";
                    continue;
                }
                cout << setw(2) << lruTrack[i][j] << " ";
            }
            cout << endl;
        }
        break;
    }
    cout << "缺页   |";
    string sig = "√";
    for (int i = 0; i < pd.pageNum; i++)
    {
        if (faults[type][i])
            cout << LIGHT_BLUE << setw(4) << sig << " ";
        else
            cout << "   ";
    }
    cout << "\n"
         << YELLOW << "命中率 :" << setw(2) << result[type] << "%" << endl;
}
// 计算命中率
void PageTrack::HitRate(int type, int miss)
{
    double lackRate = (double)miss / (double)pd.pageNum;
    result[type] = (1.0 - lackRate) * 100.0;
}

// ------------ 公共操作 ------------
// 用于检查是否缺页(key是否位于block[]内)
bool CheckFault(int *block, int key)
{
    for (int i = 0; i < pd.blockNum; i++)
    {
        // 需要使用的页面在内存中存在
        // 不缺页返回false
        if (block[i] == key)
            return false;
    }
    // 缺页返回true
    return true;
}

// ------------ OPT算法部分 ------------
// 内存为空时读入页
void OPT_FRead(int *block, int &pagePos)
{
    // 初始化
    int i = 0;
    pagePos = 0;
    for (int i = 0; i < pd.blockNum; i++)
        block[i] = -1;

    while (i < pd.blockNum)
    {
        // 避免重复读入
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            block[i] = pd.pageList[pagePos];
            // 有更改
            track.UpdateTrack(0, i, block[i], 1);
            i++;
            pagePos++;
        }
        else
        {
            // 没有更改，直接复制
            track.UpdateTrack(0, 0, 0, 0);
            pagePos++;
        }
    }
}
// 计算页面的下一次访问的时间（用页表中相距的距离表示）
int NextVis(int pagePos, int num)
{
    // pagePos  page序列当前位置
    // num 需要计算的页号
    int count = 0;
    for (int i = pagePos; i < pd.pageNum; i++)
    {
        if (pd.pageList[i] == num)
            return count;
        count++;
    }
    return INT_MAX;
}
// 用于选择内存中哪一个内存块进行替换
int OPT_Replace(int pagePos, int *block)
{
    // max temp 为该页未来下一次访问的时间
    // num 下一次访问时间最久的页面号对应的内存块号
    int max, temp, num;
    max = temp = num = INT_MIN;
    // 选择未来最久不会被访问的页淘汰
    for (int i = 0; i < pd.blockNum; i++)
    {
        temp = NextVis(pagePos, block[i]);
        if (temp > max)
        {
            max = temp;
            num = i;
        }
    }
    return num;
}
// OPT算法
void OPT()
{
    int *block = new int[pd.blockNum]; // 内存块
    int miss = 0;                      // 缺页次数
    int pagePos = 0;                   // page序列当前位置
    double lackRate;                   // 缺页中断率

    // 一开始内存为空，读入页
    OPT_FRead(block, pagePos);
    // 更新缺页次数
    miss = pd.blockNum;

    // 不断读入
    while (pagePos < pd.pageNum)
    {
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            // 缺页
            miss++;
            // 找到要淘汰的内存块号
            int num = OPT_Replace(pagePos, block);
            if (num == -1 || num > pd.blockNum)
            {
                cout << "OPT_Replace()出错！程序中止" << endl;
                exit(-1);
            }
            block[num] = pd.pageList[pagePos];
            // 有更改
            track.UpdateTrack(0, num, block[num], 1);
        }
        else
        {
            // 没有更改，直接复制
            track.UpdateTrack(0, 0, 0, 0);
        }
        pagePos++;
    }
    // 计算缺页率，并存储到数组result中
    track.HitRate(0, miss);
    delete[] block;
}

// ------------ FIFO算法部分 ------------
// 内存为空时读入页
void FIFO_FRead(int *block, int &pagePos, SqQueue &sq)
{
    // 初始化
    int i = 0;
    pagePos = 0;
    for (int i = 0; i < pd.blockNum; i++)
        block[i] = -1;
    while (i < pd.blockNum)
    {
        // 避免重复读入
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            // 读入
            block[i] = pd.pageList[pagePos];
            // 记录，有更改
            track.UpdateTrack(1, i, block[i], 1);
            // ！！入队列
            EnQueue(sq, block[i]);
            i++;
            pagePos++;
        }
        else
        {
            // 记录，无更改
            track.UpdateTrack(1, 0, 0, 0);
            pagePos++;
        }
    }
}
// 用于选择内存中哪一个内存块进行替换
int FIFO_Replace(int *block, SqQueue &sq)
{
    int e; // 被淘汰的页号
    if (!DeQueue(sq, e))
    {
        printf("出队失败！队空！！！\n");
        return -1;
    }
    for (int i = 0; i < pd.blockNum; i++)
    {
        if (block[i] == e)
            return i;
    }
    return -1;
}
// FIFO算法
void FIFO()
{
    SqQueue sq;
    InitQueue(sq);
    int *block = new int[pd.blockNum]; // 内存块
    int miss = 0;                      // 缺页次数
    int pagePos = 0;                   // page序列当前位置
    double lackRate;                   // 缺页中断率

    // 一开始内存为空，读入页
    FIFO_FRead(block, pagePos, sq);
    // 更新缺页次数
    miss = pd.blockNum;

    while (pagePos < pd.pageNum)
    {
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            // 缺页
            miss++;
            // 找到要淘汰的页框号
            int num = FIFO_Replace(block, sq);
            block[num] = pd.pageList[pagePos];
            // 入队
            EnQueue(sq, pd.pageList[pagePos]);
            // 记录，有更改
            track.UpdateTrack(1, num, block[num], 1);
        }
        else
        {
            // 记录，无更改
            track.UpdateTrack(1, 0, 0, 0);
        }
        pagePos++;
    }
    track.HitRate(1, miss);
    delete[] block;
    DestroyQueue(sq);
}

// ----------------- LRU算法部分 -------------------
// 内存为空时读入页
void LRU_FRead(int *block, int &pagePos, SqQueue &sq)
{
    // 初始化
    int i = 0;
    pagePos = 0;
    for (int i = 0; i < pd.blockNum; i++)
        block[i] = -1;

    while (i < pd.blockNum)
    {
        // 避免重复读入
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            block[i] = pd.pageList[pagePos];
            EnQueue(sq, block[i]);
            track.UpdateTrack(2, i, block[i], 1);
            i++;
            pagePos++;
        }
        else
        {
            track.UpdateTrack(2, 0, 0, 0);
            pagePos++;
        }
    }
}
// 用于选择内存中哪一个内存块进行替换
int LRU_Replace(int *block, SqQueue &sq)
{
    int e; // 被淘汰的页号
    if (!DeQueue(sq, e))
        return -1;
    for (int i = 0; i < pd.blockNum; i++)
    {
        if (block[i] == e)
            return i;
    }
    return -1;
}
// 更新访问时间
void LRU_Flash(int pagePos, SqQueue &sq, SqQueue &temSq)
{
    int len;     // 队列长度
    int pageNum; // 页号
    len = QueueLength(sq);

    for (int i = 0; i < len; i++)
    {
        // 出队
        if (!DeQueue(sq, pageNum))
            exit(-1);
        // 避免复制到需要更新访问时间的页号
        if (pageNum == pd.pageList[pagePos])
            continue;
        // 复制
        EnQueue(temSq, pageNum);
    }
    // 更新访问时间
    EnQueue(temSq, pd.pageList[pagePos]);
    // 将更新完成的队列与原来的队列交换
    SqQueue temp = temSq;
    temSq = sq;
    sq = temp;
    // 记录，无更改
    track.UpdateTrack(2, 0, 0, 0);
}
// LRU算法
void LRU()
{
    int miss = 0;                      // 缺页次数
    int *block = new int[pd.blockNum]; // 内存块
    int pagePos = 0;                   // page序列当前位置
    double lackRate;                   // 缺页中断率
    SqQueue sq;                        // 队列，记录访问时间
    SqQueue temSq;                     // 用于更新访问时间的临时队列

    InitQueue(sq);
    InitQueue(temSq);

    // 一开始内存为空，读入页
    LRU_FRead(block, pagePos, sq);
    // 更新缺页次数
    miss = pd.blockNum;

    while (pagePos < pd.pageNum)
    {
        if (CheckFault(block, pd.pageList[pagePos]))
        {
            // 缺页
            miss++;
            // 找到要淘汰的页
            int num = LRU_Replace(block, sq);
            block[num] = pd.pageList[pagePos];
            EnQueue(sq, pd.pageList[pagePos]);
            track.UpdateTrack(2, num, block[num], 1);
        }
        else
        {
            // 不缺页
            // 需要更改访问时间
            LRU_Flash(pagePos, sq, temSq);
        }
        pagePos++;
    }
    // 计算缺页中断率
    track.HitRate(2, miss);
    delete[] block;

    // 销毁队列
    DestroyQueue(sq);
    DestroyQueue(temSq);
}
