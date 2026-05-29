#ifndef __LED_H
#define __LED_H

#include "Arduino.h"

#define LED_PIN 8

#define LED_ON()      digitalWrite(LED_PIN, LOW)
#define LED_OFF()     digitalWrite(LED_PIN, HIGH)
#define LED_TOGGLE()  digitalWrite(LED_PIN, !digitalRead(LED_PIN))

void led_init(void);

#endif