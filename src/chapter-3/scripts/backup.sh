#!/bin/bash

###
 # @Author: 
 # @Date: 2023-05-09 16:41:44
 # @LastEditTime: 2023-05-10 00:24:44
 # @FilePath: /bochs/scripts/backup.sh
 # @Description: 备份
 # Copyright (c) 2023 by GZHU-FWM, All Rights Reserved. 
### 

# 常数 k，用于构造备份文件名
k=0

# 判断备份文件是否存在，如果存在则更新 k 值
while [ -f "../archive/${1}_${k}.bak" ]
do
    k=$(expr $k + 1)
done

# 构造备份文件名，并执行备份操作
backup_file="../archive/${1}_${k}.bak"
cp ../boot/${1}.* $backup_file

# 输出备份成功的消息
echo "备份成功: $backup_file"
