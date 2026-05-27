#include "relay.h"

void relay_init(void)
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF);
}

void relay_on(void)
{
    digitalWrite(RELAY_PIN, RELAY_ON);
}

void relay_off(void)
{
    digitalWrite(RELAY_PIN, RELAY_OFF);
}