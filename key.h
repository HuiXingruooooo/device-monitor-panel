#ifndef __KEY_H
#define __KEY_H

#include "Arduino.h"

#define KEY_PIN 0

// 函数声明
void key_init(void);
bool key_get_press_flag(void);  // 获取按键按下标志（非阻塞）
void key_clear_press_flag(void); // 清除按键标志

#endif