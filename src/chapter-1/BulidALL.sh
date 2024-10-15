#!/bin/bash

# 删除所有生成的可执行文件
for file in *
do
    if [[ -x "$file" && ! -d "$file" && "$file" != *.sh && "$file" != *.c ]]; then
        rm "$file"
    fi
done

# 删除所有生成的目标文件
for file in *
do
    if [[ $file == *.o || $file == *.out ]]; then
        rm "$file"
    fi
done

# 编译所有.c文件
for file in *.c
do
    gcc $file -o ${file%.c}
done
