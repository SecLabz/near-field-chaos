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

    nfc_init();
    nfc_st25tb_init();

#if DEVICE == DEVICE_RP2040_ZERO_PN532
    standalone_mode_start();
#else
    console_start();
#endif

    return 0;
}