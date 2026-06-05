#ifndef __OLED_H
#define __OLED_H
#include <Arduino.h>

void oled_init(void);
void oled_show(uint8_t mode, uint8_t power_state, int light_val, float temp, uint8_t light_state, uint8_t fan_state);

#endif