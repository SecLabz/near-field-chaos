#include "./nfc/nfc.h"
#include "./st25tb/st25tb_tag_storage.h"
#include "console/console.h"
#if DEVICE == DEVICE_RP2040_ZERO_PN532
#include "standalone_mode/standalone_mode.h"
#endif

int main()
{
    stdio_init_all();
    st25tb_tag_storage_init(false);

#if DEVICE == DEVICE_RP2040_ZERO_PN532
    ws2812_led_init();
#endif

#if CHIP == CHIP_PN532
    if (pn532_hsu_test_communication() == 0)
    {
        led_green();
        pn532_hsu_usb_bridge_start();
    }
#endif

    nfc_init();
    nfc_st25tb_init();

#if DEVICE == DEVICE_RP2040_ZERO_PN532
    standalone_mode_start();
#else
    console_start();
#endif

    return 0;
}