#include "key.h"

void key_init() {
    pinMode(KEY1_PIN, INPUT_PULLDOWN);
    pinMode(KEY2_PIN, INPUT_PULLDOWN);
}

uint8_t key1_read() {
    return digitalRead(KEY1_PIN);
}

// 长按2秒触发模式切换
uint8_t key2_scan() {
    static uint8_t key_sta = 0;
    static unsigned long t_start = 0;
    uint8_t val = digitalRead(KEY2_PIN);

    switch(key_sta) {
        case 0:
            if(val == 1) {
                delay(15);
                if(digitalRead(KEY2_PIN) == 1) {
                    key_sta = 1;
                    t_start = millis();
                }
            }
            break;
        case 1:
            if(val == 0) {
                key_sta = 0;
                return 1;
            }
            if(millis() - t_start >= 2000) {
                key_sta = 2;
                return 2;
            }
            break;
        case 2:
            if(val == 0) key_sta = 0;
            break;
    }
    return 0;
}