#ifndef __RELAY_H
#define __RELAY_H

#include "Arduino.h"

#define RELAY_PIN   2
#define RELAY_ON    HIGH
#define RELAY_OFF   LOW

void relay_init(void);
void relay_on(void);
void relay_off(void);

#endif