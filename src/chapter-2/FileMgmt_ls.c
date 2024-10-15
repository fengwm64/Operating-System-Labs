/*
 * @Author: 
 * @Date: 2023-05-08 15:08:11
 * @LastEditTime: 2023-05-09 14:11:21
 * @FilePath: /源码/FileMgmt_ls.c
 * @Description:
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define MAXLEN 256 // 路径最大长度
// 终端文字颜色相关的宏
#define NONE "\033[m"
#define YELLOW "\033[1;33m"
#define LIGHT_BLUE "\033[1;34m"
#define LIGHT_CYAN "\033[1;36m"

// 获取当前路径
void getCurPath(char *path)
{
    // 获取当前目录路径
    if (getcwd(path, MAXLEN * sizeof(char)) != NULL)
        printf("%s\n\n", path);
    else
        printf("获取当前目录路径失败！\n");
}
// ls
void simList(const char *path)
{
    DIR *dir = NULL;              // 目录指针
    struct dirent *dirent = NULL; // 目录项的结构体
    dir = opendir(path);

    if (dir == NULL)
    {
        printf("打开目录失败！检查目录是否存在\n");
        exit(-1);
    }
    while (dirent = readdir(dir))
    {
        if (strcmp(".", dirent->d_name) && strcmp("..", dirent->d_name))
            printf("%s\n", dirent->d_name);
    }
    printf("\n");
    closedir(dir);
}
// ls - l
void dtlList(const char *path)
{
    DIR *dir = NULL;              // 目录指针
    struct stat st;               // 文件信息结构图
    struct dirent *dirent = NULL; // 目录项的结构体
    char date[20];                // 日期字符串
    char temPath[MAXLEN];         // 用于拼接的相对路径
    int total = 0;                // 磁盘块数总和

    // 打开目录
    dir = opendir(path);

    // 开始逐项读取
    while (dirent = readdir(dir))
    {
        // 清空字符串
        memset(temPath, 0, sizeof(temPath));

        // 进行路径的拼接，采用绝对路径
        strcpy(temPath, path);
        strcat(temPath, "/");
        strcat(temPath, dirent->d_name);

        // 获取路径对于文件的信息
        lstat(temPath, &st);

        // --- 获取文件类型 ---
        if (S_ISREG(st.st_mode))
            printf("-");
        else if (S_ISDIR(st.st_mode))
            printf("d");
        else if (S_ISLNK(st.st_mode))
            printf("l");
        else if (S_ISCHR(st.st_mode))
            printf("c");
        else if (S_ISBLK(st.st_mode))
            printf("b");
        else if (S_ISFIFO(st.st_mode))
            printf("p");
        else if (S_ISSOCK(st.st_mode))
            printf("s");

        // --- 获取权限信息 ---
        // 所有者
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        // 所属组
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        // 其他用户
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");
        printf(" ");

        // 硬链接数
        printf("%-*ld", 4, (long)st.st_nlink);
        // 文件所有者
        printf("%-*s ", 8, getpwuid(st.st_uid)->pw_name);
        // 文件所属组
        printf("%-*s ", 8, getgrgid(st.st_gid)->gr_name);
        // 文件大小
        printf("%ld\t", (long)st.st_size);
        // 最后被修改时间
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
        printf("%s ", date);

        // --- 名称 ---
        // 可执行文件
        if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
            if (strcmp(".", dirent->d_name) && strcmp("..", dirent->d_name) && S_ISREG(st.st_mode))
                printf("%s%s%s*\n", LIGHT_CYAN, dirent->d_name, NONE);
            else
                printf("%s%s%s/\n", YELLOW, dirent->d_name, NONE);
        else
            printf("%s\n", dirent->d_name);

        // 记录文件磁盘块数
        total += st.st_blocks;
    }
    printf("%stotal: %d%s\n", LIGHT_BLUE, (total / 2), NONE);
    printf("\nCopyright (c) 2023 by FWM , All Rights Reserved.\n\n");

    closedir(dir);
}

int main(int argc, char *argv[])
{
    char path[MAXLEN]; // 路径

    switch (argc)
    {
    // 没有输入任何参数
    case 1:
        getCurPath(path);
        simList(path);
        break;
    // 工作模式
    case 2:
        getCurPath(path);
        // ls -l
        if (!strcmp("-l", argv[1]))
            dtlList(path);
        else
        {
            printf("选项错误！\n");
            exit(-1);
        }
        break;
    // 工作模式 + 工作路径
    case 3:
        // 拷贝路径
        strcpy(path, argv[2]);
        printf("%s\n\n", path);
        // ls -l
        if (!strcmp("-l", argv[1]))
            dtlList(path);
        else
        {
            printf("选项错误！\n");
            exit(-1);
        }
        break;
    default:
        printf("参数过多！\n");
        exit(-1);
        break;
    }
    return 0;
}