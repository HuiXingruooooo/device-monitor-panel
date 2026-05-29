#ifndef __EXTI_H
#define __EXTI_H

#include "Arduino.h"

#define INT_FUNC_SW_PIN   21    // 外部中断功能按键引脚

// 共享变量声明 - 注释补充
extern volatile uint8_t led_working_flag;          // LED工作状态标志
extern volatile unsigned long interrupt_timestamp; // 中断时间戳(消抖用)

// 函数声明重构
void exti_interrupt_init(void);
void IRAM_ATTR func_key_isr_handler(void); // 中断服务函数(IRAM加速)

#endif