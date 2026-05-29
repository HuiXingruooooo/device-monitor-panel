#ifndef __RELAY_H
#define __RELAY_H

#include "Arduino.h"

#define LIGHT_PIN 6
#define FAN_PIN   7

void peripherals_init(void);
void peripherals_set(uint8_t light, uint8_t fan);

#endif