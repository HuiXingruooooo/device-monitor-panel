#include "ds18b20.h"
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

void ds18b20_init(void) {
    sensors.begin();
    sensors.setWaitForConversion(false);  // 非阻塞模式 ✅ 核心优化
    sensors.requestTemperatures();
}

float ds18b20_read_temp(void) {
    float temp = sensors.getTempCByIndex(0);
    sensors.requestTemperatures();  // 异步发起下一次转换
    return temp;
}