#include "relay.h"

void relay_init(void)
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // 初始状态：继电器断开
}

void relay_on(void)
{
    digitalWrite(RELAY_PIN, HIGH);
}

void relay_off(void)
{
    digitalWrite(RELAY_PIN, LOW);
}

// 新增：翻转继电器状态的函数实现
void relay_toggle(void)
{
    // 读取当前状态，取反后写入
    bool current_state = digitalRead(RELAY_PIN);
    digitalWrite(RELAY_PIN, !current_state);
}