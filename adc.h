#ifndef __ADC_H
#define __ADC_H
#include <Arduino.h>

#define ADC_LIGHT_PIN            1
#define LIGHT_LUX_THRESHOLD      200  // 建议阈值：200 lux，暗于这个值灯亮

void adc_init(void);
int adc_read(uint8_t adc_pin);
float adc_to_lux(int rawADC);  // 新增：按公式计算 lux

#endif