/*
 * @Author: 
 * @Date: 2023-06-11 18:27:51
 * @LastEditTime: 2023-06-12 15:36:26
 * @FilePath: /OSLab/kernel/interrupt.h
 * @Description:
 * Copyright (c) 2023 by GZHU-FWM, All Rights Reserved.
 */

#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
typedef void* intr_handler;
void idt_init(void);

/* 定义中断的两种状态:
 * INTR_OFF值为0,表示关中断,
 * INTR_ON值为1,表示开中断 */
enum intr_status {		 // 中断状态
    INTR_OFF,			 // 中断关闭
    INTR_ON		         // 中断打开
};

enum intr_status intr_get_status(void);
enum intr_status intr_set_status (enum intr_status);
enum intr_status intr_enable (void);
enum intr_status intr_disable (void);
#endif