#include "exti.h"

// 全局变量重命名
volatile uint8_t led_working_flag = 0; 
volatile unsigned long interrupt_timestamp = 0; // 非阻塞消抖时间戳

// 外部中断初始化
void exti_interrupt_init(void) {
    pinMode(INT_FUNC_SW_PIN, INPUT_PULLDOWN); 
    // 绑定外部中断：上升沿触发
    attachInterrupt(digitalPinToInterrupt(INT_FUNC_SW_PIN), func_key_isr_handler, RISING); 
}

// 中断服务函数 (ISR) - 快进快出，禁止阻塞操作
void IRAM_ATTR func_key_isr_handler(void) {
    unsigned long curr_interrupt_time = millis(); 
    
    // 中断级消抖：200ms内重复触发忽略
    if (curr_interrupt_time - interrupt_timestamp > 200) {
        led_working_flag = !led_working_flag; // 翻转状态标志
    }
    interrupt_timestamp = curr_interrupt_time; 
}