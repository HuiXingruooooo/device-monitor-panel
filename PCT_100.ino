#include "key.h"
#include "relay.h"
#include "led.h"

uint8_t work_step = 0;
uint8_t system_mode = 1;        // 1=自动模式  0=手动模式
uint8_t last_key1 = 0;
uint8_t blink_en = 0;

unsigned long led_timer = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    led_init();
    key_init();
    peripherals_init();

    Serial.println("==================================");
    Serial.println("System Ready | Default: AUTO 模式");
    Serial.println("==================================");
}

void loop() {
    // 运行灯闪烁
    if (blink_en) {
        unsigned long now = millis();
        if (now - led_timer >= 500) {
            LED_TOGGLE();
            led_timer = now;
        }
    } else {
        LED_OFF();
    }

    // KEY1 总开关
    uint8_t curr_key1 = key1_read();

    if (curr_key1 == 0) {
        if (last_key1 == 1) {
            work_step = 0;
            peripherals_set(0, 0);
            blink_en = 0;
            system_mode = 1;        // 关机恢复自动
            Serial.println("[INFO] KEY1 关闭 → 全部停止，恢复自动模式");
        }
        last_key1 = 0;
        key2_scan();
        return;
    }

    // KEY1 打开
    if (last_key1 == 0) {
        Serial.println("[INFO] KEY1 开启 → 系统启动");
        last_key1 = 1;
    }

    // KEY2 动作
    uint8_t key_act = key2_scan();

    // ======================
    // 长按2秒：切换模式
    // ======================
    if (key_act == 2) {
        if (system_mode == 0) {
            // 手动 → 自动：强制关闭灯和风扇
            system_mode = 1;
            work_step = 0;
            peripherals_set(0, 0);
            blink_en = 0;
            Serial.println("[MODE] → 自动模式（已关闭所有设备）");
        } else {
            // 自动 → 手动：状态重置为00，重新开始
            system_mode = 0;
            work_step = 0;
            peripherals_set(0, 0);
            blink_en = 0;
            Serial.println("[MODE] → 手动模式（状态重置为00）");
        }
    }

    // ======================
    // 手动模式
    // ======================
    if (system_mode == 0) {
        if (key_act == 1) {
            work_step = (work_step + 1) % 4;
            blink_en = (work_step != 0);

            Serial.print("手动状态：");
            switch (work_step) {
                case 0: peripherals_set(0,0); Serial.println("00 全部关闭"); break;
                case 1: peripherals_set(0,1); Serial.println("01 风扇开启"); break;
                case 2: peripherals_set(1,0); Serial.println("10 灯光开启"); break;
                case 3: peripherals_set(1,1); Serial.println("11 全部开启"); break;
            }
        }
    }

    // ======================
    // 自动模式（仅占位，可扩展）
    // ======================
    else {
        blink_en = 1;
    }
}