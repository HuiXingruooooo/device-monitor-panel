#include "adc.h"

void adc_init(void) {
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);
}

int adc_read(uint8_t adc_pin) {
    return analogRead(adc_pin);
}

// 按你给的公式实现 lux 换算
float adc_to_lux(int rawADC) {
    int invADC = 4095 - rawADC;       // 反相ADC
    float lux = (float)(invADC * invADC) / 30000.0f;
    return lux;
}