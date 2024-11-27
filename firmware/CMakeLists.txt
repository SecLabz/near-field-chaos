# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(nfc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_definitions(-DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    console/console_commands.c
    console/console.c
    led/ws2812_led.c
    nfc/drivers/trf7970a/trf7970a.c
    nfc/drivers/pn532/pn532_hsu.c
    nfc/drivers/pn532/pn532_spi.c
    nfc/drivers/pn532/pn532.c
    nfc/nfc.c
    spi/pio_spi.c
    spi/pio_spi.h
    st25tb/st25tb_common.c
    st25tb/st25tb_cmd.c
    st25tb/st25tb_tag_storage.c
    st25tb/st25tb.c
    st25tb/st25tb_tear_off.c
    standalone_mode/standalone_mode.c
    standalone_mode/t4t_ce.c
    utils/utils.c
    utils/task.c
    main.c
)

# Generate pio headers
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/spi/spi.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/led/ws2812.pio)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pio
    hardware_spi
    hardware_flash
    hardware_sync
    hardware_adc
    pico_multicore
    pico_rand
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Emulate a CP210x UART Bridge
# add_compile_definitions(USBD_VID=0x067B)
# add_compile_definitions(USBD_PID=0xAAA3)

add_compile_definitions(DEVICE=DEVICE_RP2040_ZERO_PN532)
# add_compile_definitions(DEVICE=DEVICE_SHADOWTAG)
# add_compile_definitions(DEVICE=DEVICE_ST25TB_KAMELEON)
# add_compile_definitions(DEVICE=DEVICE_PN532_KIMONOBOARD)
# add_compile_definitions(DEVICE=DEVICE_TRF7970_KIMONOBOARD)