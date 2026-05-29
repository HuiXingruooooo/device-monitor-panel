#include "led.h"

void led_init() {
    pinMode(LED_PIN, OUTPUT);
    LED_OFF();
}