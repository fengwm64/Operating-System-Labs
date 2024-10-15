/*
 * @Author:   
 * @Date: 2023-05-07 17:07:03
 * @LastEditTime: 2023-05-17 15:52:52
 * @FilePath: /源码/FileMgmt_fw.cpp
 * @Description: 文件管理实验 - fwrite/write
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include <iostream>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

#define COUNT 1000000

void fwriteTime(const char *str, const char *fname, int count)
{
    // 以追加方式打开文件
    FILE *fp = fopen(fname, "a");
    if (!fp)
    {
        cout << "文件打开失败!" << endl;
        exit(-1);
    }
    // 开始计时
    auto startTime = high_resolution_clock::now();
    // 写入文件
    for (int i = 0; i < count; i++)
        fwrite(str, sizeof(char), strlen(str), fp);
        
    // 结束计时
    auto endTime = high_resolution_clock::now();
    cout << "fwriteTime:\t" << duration_cast<milliseconds>(endTime - startTime).count() << " ms" << endl;

    // 关闭文件
    fclose(fp);
}
void writeTime(const char *str, const char *fname, int count)
{
    // 以追加方式打开文件
    int fd = open(fname, O_CREAT | O_RDWR | O_APPEND, 0666);
    if (fd == -1)
    {
        cout << "文件打开失败!" << endl;
        exit(-1);
    }
    // 开始计时
    auto startTime = high_resolution_clock::now();
    // 写入文件
    for (int i = 0; i < count; i++)
        write(fd, str, strlen(str));
    // 结束计时
    auto endTime = high_resolution_clock::now();
    cout << "writeTime:\t" << duration_cast<milliseconds>(endTime - startTime).count() << " ms" << endl;
    // 关闭文件
    close(fd);
}

int main()
{
    const char *str = "Good Moring,FWM\n";
    const char *fname = "file_data";
    cout << "追加写文件100万次时间对比" << endl;
    // fwrite
    fwriteTime(str, fname, COUNT);
    // 清空文件
    truncate(fname, 0);
    // write
    writeTime(str, fname, COUNT);
    // 删除文件
    remove(fname);
    cout << "\nCopyright (c) 2023 by FWM , All Rights Reserved.\n"
         << endl;
    return 0;
}
