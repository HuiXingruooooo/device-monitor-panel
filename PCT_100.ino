#include "key.h"
#include "relay.h"

void setup()
{
    key_init();    // 初始化按键（中断+非阻塞消抖）
    relay_init();  // 初始化继电器
    Serial.begin(9600);
}

void loop()
{
    // 检测按键按下（非阻塞，不卡程序）
    if (key_get_press_flag())
    {
        // 处理按键事件
        Serial.println("key press!");
        relay_toggle(); // 翻转继电器状态

        // 处理完事件，清除标志
        key_clear_press_flag();
    }
}