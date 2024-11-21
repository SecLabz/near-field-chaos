
#include "ws2812_led.h"

void ws2812_led_init()
{
    PIO pio = pio0;
    int sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &ws2812_program);
    int result = 0;

    ws2812_program_init(pio, sm, offset, 16, 800000, true);
}

void ws2812_led_put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

void ws2812_led_put_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t mask = (green << 16) | (red << 8) | (blue << 0);
    ws2812_led_put_pixel(mask);
}
