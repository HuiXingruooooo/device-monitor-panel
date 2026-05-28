#include "key.h"

// 全局变量
static volatile bool key_press_flag = false;  // 按键按下标志（中断修改）
static unsigned long last_debounce_time = 0; // 消抖计时
const unsigned long DEBOUNCE_DELAY = 20;      // 消抖时间20ms（非阻塞）

// 中断服务函数：按键触发中断
void IRAM_ATTR key_isr(void)
{
  unsigned long current_time = millis();
  // 非阻塞消抖：距离上次触发超过20ms才有效
  if (current_time - last_debounce_time >= DEBOUNCE_DELAY)
  {
    last_debounce_time = current_time;
    key_press_flag = true;
  }
}

void key_init(void)
{
  // 初始化按键引脚：上拉输入
  pinMode(KEY_PIN, INPUT_PULLUP);
  // 绑定外部中断：下降沿触发（按键按下）
  attachInterrupt(KEY_PIN, key_isr, FALLING);
}

// 获取按键按下标志
bool key_get_press_flag(void)
{
  return key_press_flag;
}

// 清除按键标志（处理完按键事件后调用）
void key_clear_press_flag(void)
{
  key_press_flag = false;
}