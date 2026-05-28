#ifndef __RELAY_H
#define __RELAY_H

#include "Arduino.h"

// 继电器引脚定义
#define RELAY_PIN 2

void relay_init(void);
void relay_on(void);
void relay_off(void);
void relay_toggle(void); // 新增：翻转继电器状态的函数声明

#endif