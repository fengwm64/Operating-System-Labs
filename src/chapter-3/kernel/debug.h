/*
 * @Author: 
 * @Date: 2023-06-11 22:47:47
 * @LastEditTime: 2023-06-12 15:44:05
 * @FilePath: /OSLab/kernel/debug.h
 * @Description: 断言头文件
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved. 
 */

#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

void panic_spin(char* filename, int line, const char* func, const char* condition);

/***************************  __VA_ARGS__  *******************************
 * __VA_ARGS__ 是预处理器所支持的专用标识符。
 * 代表所有与省略号相对应的参数. 
 * "..."表示定义的宏其参数可变.*/
#define PANIC(...) panic_spin (__FILE__, __LINE__, __func__, __VA_ARGS__)
 /***********************************************************************/

#ifdef NDEBUG
   #define ASSERT(CONDITION) ((void)0)
#else
   #define ASSERT(CONDITION)                                      \
      if (CONDITION) {}                                           \
      else { PANIC(#CONDITION); }/* 符号#让编译器将宏的参数转化为字符串字面量 */
#endif /*__NDEBUG */

#endif /*__KERNEL_DEBUG_H*/
