#!/bin/bash
###
 # @Author: 
 # @Date: 2023-05-12 16:01:36
 # @LastEditTime: 2023-06-13 19:28:56
 # @FilePath: OSLab/scripts/insatll_require.sh
 # @Description: 安装所需软件包
 # Copyright (c) 2023 by GZHU-FWM, All Rights Reserved. 
### 

sudo add-apt-repository ppa:git-core/ppa

# 执行 apt 更新和升级
sudo apt update
sudo apt upgrade

# 安装软件包
sudo apt-get install -y neofetch gcc vim build-essential g++ libgtk2.0-dev nasm gdb git
