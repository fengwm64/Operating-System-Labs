/*
 * @Author: 
 * @Date: 2023-05-04 16:31:46
 * @LastEditTime: 2023-05-07 17:01:37
 * @FilePath: /源码/DiskMgmt.cpp
 * @Description: 磁盘管理实验
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <limits.h>

using namespace std;

// 终端文字颜色相关的宏
#define NONE "\033[m"
#define YELLOW "\033[1;33m"
#define LIGHT_BLUE "\033[1;34m"
#define LIGHT_CYAN "\033[1;36m"

// ------------ 快速排序 ------------
int Partition(vector<int> &A, int low, int high);  // 划分
void QuickSort(vector<int> &A, int low, int high); // 快排

// ------------ 测试数据 ------------
class DiskData
{
public:
    void InitData(const string &fname); // 从文件中读取数据初始化数据成员
    void PrintData();                   // 打印初始化后的数据

    int start;            // 起始磁道号
    vector<int> disklist; // 磁道号序列
    vector<int> sortlist; // 排序后的磁道号序列
} dd;

void FCFS(); // 先来先服务算法 FCFS

int SSTF_FindFirst(const vector<int> &sortlist, int start);                   // 找到第一个服务的磁道号下标
int SSTF_FindNext(const vector<int> &sortlist, vector<bool> &flag, int head); // 用于找到下一个服务的磁道号下标
void SSTF();                                                                  // 最短寻道时间优先算法 SSTF

int LOOK_FindFirst(const vector<int> &sortlist, int start, int dir);       // 找到第一个服务的磁道号下标
void LOOK_Move(const vector<int> &sortlist, int head, int start, int dir); // 电梯移动
void LOOK();                                                               // 电梯算法 LOOK

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

    dd.InitData(fname);
    dd.PrintData();

    // 开始模拟各种磁道算法
    FCFS();
    SSTF();
    LOOK();
    return 0;
}

// ------------ 快速排序 ------------
// 划分
int Partition(vector<int> &A, int low, int high)
{
    // 选取下标为low的元素作为划分枢纽
    int pivot = A[low];
    // 寻找枢纽的位置下标
    while (low < high)
    {
        // 在high部分找到一个不属于high的元素
        while (low < high && A[high] >= pivot)
            high--;
        // 然后将其替换到low部分去
        A[low] = A[high];
        // 同理，在low部分找到不属于的元素
        while (low < high && A[low] <= pivot)
            low++;
        // 替换到high部分去
        A[high] = A[low];
    }
    // 枢纽放入分界位置
    A[low] = pivot;
    // 返回
    return low;
}
// 快排
void QuickSort(vector<int> &A, int low, int high)
{
    // 递归终止条件
    if (low < high)
    {
        // 找到一个分界位置
        int pivot = Partition(A, low, high);
        // 递归分界左边
        QuickSort(A, low, pivot - 1);
        // 递归分界右边
        QuickSort(A, pivot + 1, high);
    }
}

// ------------ 测试数据 ------------
void DiskData::InitData(const string &fname)
{
    string line;
    ifstream ifs;
    stringstream ss;

    cout << "\nCopyright (c) 2023 by FWM , All Rights Reserved.\n"
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
            // 读取起始磁道号
            ss.str(line);
            ss >> start;
            ss.clear();
            break;

        case 2:
            int num = -1; // 存储每次从ss里读取的数
            // 初始化ss
            ss.str(line);
            while (ss >> num)
                disklist.push_back(num);
            break;
        }
    }
    // 关闭文件
    ifs.close();
    // 排序
    sortlist = disklist;
    QuickSort(sortlist, 0, sortlist.size() - 1);
}
void DiskData::PrintData()
{
    cout << "文件内容：\n";
    // 起始磁道号
    cout << start << endl;
    // 磁道号序列
    for (int i = 0; i < disklist.size(); i++)
        cout << disklist[i] << " ";
    cout << endl
         << endl;
}

// -------- 先来先服务算法 FCFS --------
void FCFS()
{
    int head = dd.start; // head模拟的磁头,head是磁道号
    int count = 0;       // 磁头移动的总距离

    cout << YELLOW << "先来先服务法(FCFS)" << endl;
    cout << NONE << "磁头服务顺序：";
    cout << head << " ";
    // FCFS，直接遍历即可
    for (int i = 0; i < dd.disklist.size(); i++)
    {
        cout << dd.disklist[i] << ' ';
        count += abs(dd.disklist[i] - head);
        head = dd.disklist[i];
    }
    cout << "\n磁头移动的总磁道数为：" << LIGHT_CYAN << count << endl;
    cout << NONE << "平均寻找长度为：" << (double)count / (double)dd.disklist.size() << endl;
}

// ----- 最短寻道时间优先算法 SSTF -----
// 找到第一个服务的磁道号下标
int SSTF_FindFirst(const vector<int> &sortlist, int start)
{
    int i = 0;
    int low = 0, high = sortlist.size() - 1, mid = -1;
    // 当start小于等于A[0]时特殊处理
    if (start <= sortlist[0])
        return 0;
    // 当start大于等于A[high]时特殊处理
    if (start >= sortlist[high])
        return high;
    // 二分查找
    while (low < high)
    {
        mid = (low + high) / 2;
        if (start >= sortlist[mid])
            low = mid + 1;
        else
            high = mid;
    }

    // 比较位置high与high-1位置哪个更靠近start（high位置是恰好>start；high-1位置恰好<start）
    int dist1 = abs(sortlist[high] - start);
    int dist2 = abs(sortlist[high - 1] - start);
    if (dist1 <= dist2)
        return high;
    else
        return high - 1;
    return -1;
}
// 用于找到下一个服务的磁道号下标
int SSTF_FindNext(const vector<int> &sortlist, vector<bool> &flag, int head)
{
    // 往左看
    int frontPos = head - 1;
    int frontRec = INT_MAX;
    // 找到第一个没被访问过的
    while (frontPos >= 0)
    {
        if (!flag[frontPos])
            break;
        frontPos--;
    }
    // 计算与sortlist[head]的差
    if (frontPos != -1)
        frontRec = abs(sortlist[frontPos] - sortlist[head]);

    // 往右看
    int rearPos = head + 1;
    int rearRec = INT_MAX;
    // 找到第一个没被访问过的
    while (rearPos < sortlist.size())
    {
        if (!flag[rearPos])
            break;
        rearPos++;
    }
    // 计算与sortlist[head]的差
    if (rearPos != sortlist.size())
        rearRec = abs(sortlist[rearPos] - sortlist[head]);

    // 比较左右，返回距离data[head]最近的
    // 左右距离相等
    if (frontRec == rearRec && frontRec != INT_MAX)
        return frontPos;
    // 右边更近
    else if (frontRec > rearRec)
        return rearPos;
    // 左边更近
    else if (frontRec < rearRec)
        return frontPos;
    else
        return -1;
}
// 最短寻道时间优先算法 SSTF
void SSTF()
{
    int count = 0;                                // 磁头移动的总距离
    int head = 0;                                 // head模拟的磁头，head是下标！！
    int qhead = 0;                                // 记录head的上一个位置
    vector<bool> flag(dd.disklist.size(), false); // 标记数组，是否已经访问

    cout << YELLOW << "\n最短寻道时间优先算法(SSTF)\n";
    cout << NONE << "磁头服务顺序：";
    cout << dd.start << " ";

    // 找到磁头第一个服务的磁道号下标
    head = SSTF_FindFirst(dd.sortlist, dd.start);
    flag[head] = true;
    // 记录磁头移动距离
    count += abs(dd.start - dd.sortlist[head]);
    cout << dd.sortlist[head] << " ";
    // 记录磁头上一个位置（便于计算移动距离）
    qhead = head;
    while ((head = SSTF_FindNext(dd.sortlist, flag, head)) != -1)
    {
        flag[head] = true;
        count += abs(dd.sortlist[head] - dd.sortlist[qhead]);
        cout << dd.sortlist[head] << " ";
        qhead = head;
    }
    cout << endl;
    cout << "磁头移动的总磁道数为：" << LIGHT_CYAN << count << endl;
    cout << NONE << "平均寻找长度为：" << (double)count / (double)dd.sortlist.size() << endl;
}

// --------- 电梯算法 LOOK ---------
// 找到第一个服务的磁道号下标
int LOOK_FindFirst(const vector<int> &sortlist, int start, int dir)
{
    int i = 0;
    int low = 0, high = sortlist.size() - 1, mid = -1;
    // 当start小于等于A[0]时特殊处理
    if (start <= sortlist[0])
        return 0;
    // 当start大于等于A[high]时特殊处理
    if (start >= sortlist[high])
        return high;
    // 二分查找
    while (low < high)
    {
        mid = (low + high) / 2;
        if (start >= sortlist[mid])
            low = mid + 1;
        else
            high = mid;
    }
    // high是恰好大于start的下标，high-1是恰好小于start的下标
    // 根据方向返回下标
    // 向磁道号减小的方向移动
    if (dir == 0)
        return high - 1;
    // 向磁道号增大的方向移动
    else
        return high;
}
// 电梯移动
void LOOK_Move(const vector<int> &sortlist, int head, int start, int dir)
{
    // 电梯扫描，磁道号增大、减小方向各一次
    for (int i = 0; i < 2; i++)
    {
        // 减小方向移动
        if (dir == 0)
        {
            // 不断左移
            while (head >= 0)
            {
                cout << sortlist[head] << " ";
                head--;
            }
            // 指针左移到最小磁道号，更新指针到开始位置右侧
            head = start + 1;
            // 设置移动的方向(0左1右)
            dir = 1;
        }
        // 增大方向移动
        else
        {
            // 同理
            while (head < sortlist.size())
            {
                cout << sortlist[head] << " ";
                head++;
            }
            head = start - 1;
            dir = 0;
        }
    }
}
// 电梯算法 LOOK
void LOOK()
{
    int head = 0;  // head模拟的磁头，head是下标！！
    int qhead = 0; // 记录head的上一个位置
    int count = 0; // 磁头移动的总距离

    cout << YELLOW << "\n电梯算法(LOOK)\n";

    // ------- 磁头向增大方向移动 -------
    // 找到距离起点最近的磁道
    head = LOOK_FindFirst(dd.sortlist, dd.start, 1);
    // 记录head此刻的位置
    qhead = head;
    cout << LIGHT_BLUE << "1.磁头方向：磁道号增加\n";
    cout << NONE << "磁头服务顺序：";
    cout << dd.start << " ";
    LOOK_Move(dd.sortlist, head, head, 1);
    cout << endl;
    count = (dd.sortlist.back() - dd.start) + (dd.sortlist.back() - dd.sortlist.front());
    cout << "磁头移动的总磁道数为：" << LIGHT_CYAN << count << endl;
    cout << NONE << "平均寻找长度为：" << (double)count / (double)dd.sortlist.size() << endl;

    // ------- 磁头向减小方向移动 -------
    // 找到距离起点最近的点
    head = LOOK_FindFirst(dd.sortlist, dd.start, 0);
    cout << LIGHT_BLUE << "2.磁头方向：磁道号减少\n";
    cout << NONE << "磁头服务顺序：";
    cout << dd.start << " ";
    LOOK_Move(dd.sortlist, head, head, 0);
    cout << endl;
    count = (dd.start - dd.sortlist.front()) + (dd.sortlist.back() - dd.sortlist.front());
    cout << "磁头移动的总磁道数为：" << LIGHT_CYAN << count << endl;
    cout << NONE << "平均寻找长度为：" << (double)count / (double)dd.sortlist.size() << endl;
}
