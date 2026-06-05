#include "oled.h"
#include <U8g2lib.h>
#include <Wire.h>

#define I2C_SDA 4
#define I2C_SCL 5

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA);

void oled_init(void) {
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

void oled_show(uint8_t mode, uint8_t power_state, int light_val, float temp, uint8_t light_state, uint8_t fan_state) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  char buf[32];

  if (mode == 1) {
    u8g2.drawUTF8(2, 14, "模式：自动");
  } else {
    u8g2.drawUTF8(2, 14, "模式：手动");
  }

  if (power_state == 1) {
    u8g2.drawUTF8(72, 14, "总闸：开");
  } else {
    u8g2.drawUTF8(72, 14, "总闸：关");
  }

  snprintf(buf, sizeof(buf), "光照：%d / 200", light_val);
  u8g2.drawUTF8(2, 30, buf);

  snprintf(buf, sizeof(buf), "温度：%.1f℃ / 35.0℃", temp);
  u8g2.drawUTF8(2, 46, buf);

  u8g2.drawUTF8(2, 62, light_state ? "灯光：开" : "灯光：关");
  u8g2.drawUTF8(66, 62, fan_state ? "风扇：开" : "风扇：关");

  u8g2.sendBuffer();
}