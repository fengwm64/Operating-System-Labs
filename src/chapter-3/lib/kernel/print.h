/*
 * @Author: 
 * @Date: 2023-06-11 13:55:07
 * @LastEditTime: 2023-06-12 15:38:05
 * @FilePath: /OSLab/lib/kernel/print.h
 * @Description: 
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved. 
 */
#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char* message);
void put_int(uint32_t num);	 // 以16进制打印
#endif
