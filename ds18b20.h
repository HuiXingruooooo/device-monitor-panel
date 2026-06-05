#ifndef __DS18B20_H
#define __DS18B20_H

#include <Arduino.h>

// 硬件引脚
#define DS18B20_PIN    10

// 温度阈值（℃，超过就开风扇）
#define TEMP_THRESHOLD 30.0f

// 函数
void ds18b20_init(void);
float ds18b20_read_temp(void);

#endif