/*
 * @Author: 
 * @Date: 2023-06-10 09:48:11
 * @LastEditTime: 2023-06-12 15:44:48
 * @FilePath: /OSLab/kernel/main.c
 * @Description: 测试bitmap
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include "print.h"
#include "init.h"
#include "memory.h"

int main(void)
{
   put_str("\nI am kernel\n");
   init_all();

   void *addr = get_kernel_pages(3);
   put_str("\n get_kernel_page start vaddr is ");
   put_int((uint32_t)addr);
   put_str("\n");

   while (1);

   return 0;
}