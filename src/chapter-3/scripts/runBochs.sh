#!/bin/bash

###
# @Author: 
# @Date: 2023-05-10 00:25:36
 # @LastEditTime: 2023-06-12 11:31:26
 # @FilePath: /OSLab/scripts/runBochs.sh
# @Description: 一键运行bochs
# Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
###

# 路径
bochsPath=~/OS/bochs/bin
hdPath=$bochsPath/hd60M.img
incPath=../boot/include/
sourcePath=../boot
buildPath=../build
kernelPath=../kernel
libPath=../lib
devicePath=../device

# 启用错误处理
set -e

# 清空硬盘
echo "#### 清空硬盘 ####"
dd if=/dev/zero of=$hdPath bs=512 count=$(blockdev --getsz $hdPath) conv=notrunc

# 写入硬盘
echo "#### 写入硬盘 ####"
if [ "$1" == "-m" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    echo "写入mbr完成！"

elif [ "$1" == "-ml" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/.bin $sourcePath/loader.S
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=1 seek=2 conv=notrunc
    echo "写入mbr and loader完成！"

elif [ "$1" == "-pmode" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/loader.bin $sourcePath/loader.S
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=3 seek=2 conv=notrunc
    echo "写入mbr and loader完成! p_mode!"

elif [ "$1" == "-kernel" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/loader.bin $sourcePath/loader.S
    gcc -m32 -c -o $buildPath/main.o $kernelPath/main.c
    ld -m elf_i386 $buildPath/main.o -Ttext 0xc0001500 -e main -o $buildPath/kernel.bin
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=3 seek=2 conv=notrunc
    dd if=$buildPath/kernel.bin of=$hdPath bs=512 count=200 seek=9 conv=notrunc

elif [ "$1" == "-print" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/loader.bin $sourcePath/loader.S
    nasm -f elf -o $buildPath/print.o $libPath/kernel/print.S
    gcc -m32 -I $libPath/kernel/ -I $libPath/ -c -o $buildPath/main.o $kernelPath/main.c
    ld -m elf_i386 -Ttext 0xc0001500 -e main -o $buildPath/kernel.bin $buildPath/main.o $buildPath/print.o
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=4 seek=2 conv=notrunc
    dd if=$buildPath/kernel.bin of=$hdPath bs=512 count=200 seek=9 conv=notrunc

elif [ "$1" == "-interrupt" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/loader.bin $sourcePath/loader.S
    nasm -f elf -o $buildPath/print.o $libPath/kernel/print.S
    nasm -f elf -o $buildPath/kernel.o $kernelPath/kernel.S
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/main.o $kernelPath/main.c
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/interrupt.o $kernelPath/interrupt.c
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/init.o $kernelPath/init.c
    ld -m elf_i386 -Ttext 0xc0001500 -e main -o $buildPath/kernel.bin $buildPath/main.o $buildPath/init.o $buildPath/interrupt.o $buildPath/print.o $buildPath/kernel.o
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=4 seek=2 conv=notrunc
    dd if=$buildPath/kernel.bin of=$hdPath bs=512 count=200 seek=9 conv=notrunc

elif [ "$1" == "-runner" ]; then
    nasm -I $incPath -o $buildPath/mbr.bin $sourcePath/mbr.S
    nasm -I $incPath -o $buildPath/loader.bin $sourcePath/loader.S
    nasm -f elf -o $buildPath/print.o $libPath/kernel/print.S
    nasm -f elf -o $buildPath/kernel.o $kernelPath/kernel.S
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/main.o $kernelPath/main.c
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/interrupt.o $kernelPath/interrupt.c
    gcc -m32 -I $libPath/kernel/ -m32 -I $libPath/ -m32 -I $kernelPath/ -c -fno-builtin -o $buildPath/init.o $kernelPath/init.c
    gcc -m32 -I $libPath/kernel -m32 -I $libPath/ -c -o $buildPath/timer.o $devicePath/timer.c
    ld -m elf_i386 -Ttext 0xc0001500 -e main -o $buildPath/kernel.bin $buildPath/main.o $buildPath/init.o $buildPath/interrupt.o $buildPath/print.o $buildPath/kernel.o $buildPath/timer.o
    dd if=$buildPath/mbr.bin of=$hdPath bs=512 count=1 conv=notrunc
    dd if=$buildPath/loader.bin of=$hdPath bs=512 count=4 seek=2 conv=notrunc
    dd if=$buildPath/kernel.bin of=$hdPath bs=512 count=200 seek=9 conv=notrunc

elif [ "$1" == "-clean" ]; then
    rm -f $buildPath/*

else
    # 参数错误，打印帮助信息
    echo "用法： $0 [-m|-ml|-pmode|-kernel|-print|-runner|-clean]"
    echo "  -m      写入 MBR"
    echo "  -ml     写入 MBR 和 Loader"
    echo "  -pmode  写入 MBR 和 Loader，启动保护模式内核"
    echo "  -kernel 编译内核，写入 MBR、Loader 和内核"
    echo "  -print  编译内核，写入 MBR、Loader 和内核，包含打印相关文件"
    echo "  -interrupt  编译内核，写入 MBR、Loader 和内核，包含打印相关文件"
    echo "  -runner 编译内核，写入 MBR、Loader 和内核，包含打印相关文件，启动 Bochs"
    echo "  -clean  清空输出目录"
    exit 1
fi

# 运行bochs
$bochsPath/bochs -qf $bochsPath/fwmbochsrc.disk
