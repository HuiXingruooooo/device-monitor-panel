#ifndef __KEY_H
#define __KEY_H

#include "Arduino.h"

#define KEY1_PIN  20
#define KEY2_PIN  21

void key_init(void);
uint8_t key1_read(void);
uint8_t key2_scan(void);

#endif