/*
 * @Author: 
 * @Date: 2023-06-11 18:29:27
 * @LastEditTime: 2023-06-12 15:35:21
 * @FilePath: /OSLab/kernel/init.c
 * @Description:
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"

/*负责初始化所有模块 */
void init_all() {
   put_str("init_all\n");
   idt_init();    // 初始化中断
   timer_init();  // 初始化PIT
   mem_init();	  // 初始化内存管理系统
}