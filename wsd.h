#ifndef WSD_H
#define WSD_H

#include <Arduino.h>

// 初始化彩灯
void wsd_init(void);

// 开机：蓝色闪烁3次
void wsd_boot_blue_blink(void);

// 设置为红色常亮（未联网）
void wsd_red(void);

// 设置为绿色常亮（已联网）
void wsd_green(void);

#endif