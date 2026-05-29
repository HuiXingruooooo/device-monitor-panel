#include "relay.h"

void peripherals_init() {
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    peripherals_set(0, 0);
}

void peripherals_set(uint8_t light, uint8_t fan) {
    digitalWrite(LIGHT_PIN, light ? HIGH : LOW);
    digitalWrite(FAN_PIN, fan ? HIGH : LOW);
}