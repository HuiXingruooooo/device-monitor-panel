#include "wsd.h"
#include <Adafruit_NeoPixel.h>

#define WS2812_PIN    0
#define WS2812_NUM    1

Adafruit_NeoPixel wsd(WS2812_NUM, WS2812_PIN, NEO_GRB + NEO_KHZ800);

void wsd_init(void) {
  wsd.begin();
  wsd.clear();
  wsd.show();
}

void wsd_boot_blue_blink(void) {
  for (uint8_t i = 0; i < 3; i++) {
    wsd.setPixelColor(0, wsd.Color(0, 0, 255));
    wsd.show();
    delay(300);

    wsd.setPixelColor(0, wsd.Color(0, 0, 0));
    wsd.show();
    delay(300);
  }
}

void wsd_red(void) {
  wsd.setPixelColor(0, wsd.Color(255, 0, 0));
  wsd.show();
}

void wsd_green(void) {
  wsd.setPixelColor(0, wsd.Color(0, 255, 0));
  wsd.show();
}