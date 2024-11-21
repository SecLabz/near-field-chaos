#pragma once
#include "ws2812.pio.h"

#if DEVICE == DEVICE_RP2040_ZERO_PN532
#define led_red() ws2812_led_put_rgb(255, 0, 0)
#define led_green() ws2812_led_put_rgb(0, 255, 0)
#define led_blue() ws2812_led_put_rgb(0, 0, 255)
#define led_cyan() ws2812_led_put_rgb(0, 255, 255)
#define led_purple() ws2812_led_put_rgb(255, 0, 255)
#define led_yellow() ws2812_led_put_rgb(255, 255, 0)
#define led_orange() ws2812_led_put_rgb(255, 128, 0)
#define led_white() ws2812_led_put_rgb(255, 255, 255)
#define led_off() ws2812_led_put_rgb(0, 0, 0)
#else
#define led_red()
#define led_green()
#define led_blue()
#define led_cyan()
#define led_purple()
#define led_yellow()
#define led_orange()
#define led_white()
#define led_off()
#endif


void ws2812_led_init();
void ws2812_led_put_pixel(uint32_t pixel_grb);
void ws2812_led_put_rgb(uint8_t red, uint8_t green, uint8_t blue);