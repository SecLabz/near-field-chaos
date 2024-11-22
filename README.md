<h1 align="center">Near Field Chaos</h1>

<div align="center">
  <strong>Interact with ST25TB tags and glitch their counters.</strong>
</div>

## Introduction

The project allows reading and writing ST25TB tags and manipulating their counters. 
Using a Raspberry Pi Pico (or other RP2040 boards) paired with NFC shields (PN532, TRF7970A), it employs a "tear-off" glitching method to override tag's firmware restrictions, enabling otherwise limted counter operations.

## Features

- Web UI for reading, writing, loading, and saving ST25TB tags
- Command line interface
- Standalone mode with learning, restoring, and emulating capabilities

## Cheapest minimal setup

<img src="docs/RP2040-Zero PN532 - Minimal Setup.jpg" width="400">

## Hardware Requirements

* RP2040 board (Raspberry Pi Pico, RP2040-Zero, etc...)
* NFC Shield: any PN532 with SPI or TRF7070A shield (Elechouse PN532 V3, DLP-7970ABP, etc...)
* ST25TB Tag

## Wiring

<img src="docs/RP2040-Zero PN532 - Wiring.jpg" width="600">

For TRF7970A wiring, check [firmware/nfc/devices.h](firmware/nfc/devices.h) 

## Flashing the firmware

* Download the latest firmware from the release page
* Hold the BOOTSEL button on the Pico, connect it via USB, and release when RPI-RP2 appears
* Drag and drop the UF2 file onto the RPI-RP2 drive

## Web UI

Visit this [page](https://seclabz.github.io/near-field-chaos/index.html) in a web browser with Web Serial support (e.g., Chrome).

Connect your Pico via USB, click "Open", select your Pico's serial port, and you're ready to go.


https://github.com/user-attachments/assets/353a3dfa-78bd-42c6-95cd-c02ece3ccb4e


## CLI

Use your favorite serial tool to interact with the CLI.

Here is a browser terminal console: https://console.zacharyschneider.ca/

```sh
screen /dev/tty.usbmodem101
```

<img width="741" alt="CLI screenshot" src="docs/CLI - Help.png">

<img width="356" alt="CLI Read screenshot" src="docs/CLI - Read.png">


## Standalone mode

In standalone mode, no computer is needed to interact with tags.

The reset button cycles through three functions, indicated by LED colors:

- **Learn**: Reads and saves a tag to flash memory (blue LED 🔵).
- **Restore**: Restores a saved tag (yellow LED 🟡).
- **Emulator**: Allows a smartphone to interact with tags via the PN532 (purple LED 🟣).

> **_NOTE:_** This mode is only available for the RP2040-Zero board / PN532 combination.

Emulation demo

https://github.com/user-attachments/assets/96c9d6df-3fb3-468c-ab25-1cb0502a20f2



## Tear off notes

- It should work with most tags, but if for some reason it doesn’t, you can adjust a few parameters such as ```tear_off_adjustment_us``` or ```write_passes```.
- The TRF7970A is a faster, though more expensive, NFC chip that offers quicker counter glitching.
- The PN532 performs best when the tag is correctly positioned; the optimal sweet spot is shown in the photo below.

<img width="300" alt="Sweet spot" src="docs/PN532 - Sweet spot.png">

## Build and deploy

- Install the pico's sdk
- Define the target device in [firmware/CMakeLists.txt](firmware/CMakeLists.txt)
- To customize the pins, modify the [firmware/nfc/devices.h](firmware/nfc/devices.h) file accordingly.

```sh
chmod +x ./build_deploy.sh
./build_deploy.sh
```

## References

- **[YouTube Video: "Hardwear.io NL 2021: EEPROM: It Will All End In Tears by Philippe Teuwen & Christian Herrmann"](https://www.youtube.com/watch?v=zZp5h0Tdkhk)**  
- **[Quarkslab Blog - "New Proxmark3 Tear-Off Features and New Findings"](https://blog.quarkslab.com/rfid-new-proxmark3-tear-off-features-and-new-findings.html)**  
- **[Quarkslab Blog - "RFID Monotonic Counter Anti-Tearing Defeated"](https://blog.quarkslab.com/rfid-monotonic-counter-anti-tearing-defeated.html)**  
- **[Quarkslab Blog - "EEPROM: When Tearing Off Becomes a Security Issue"](https://blog.quarkslab.com/eeprom-when-tearing-off-becomes-a-security-issue.html)**  
- **[SSTIC 2021 Paper: "EEPROM: It Will All End in Tears"](https://www.sstic.org/media/SSTIC2021/SSTIC-actes/eeprom_it_will_all_end_in_tears/SSTIC2021-Article-eeprom_it_will_all_end_in_tears-herrmann_teuwen.pdf)**
- **[SSTIC 2021 Video: "EEPROM: It Will All End in Tears"](https://static.sstic.org/videos2021/1080p/vostfr-eeprom_it_will_all_end_in_tears.mp4)**
- **[Paper: "Study of failure behavior in EEPROM memories applied to RFID/NFC devices"](http://proxmark.org/files/Documents/13.56%20MHz%20-%20MIFARE%20Ultralight/PFI%20-%20Federico%20Gabriel%20Ukmar%20LU1052979%20-%20Nahuel%20Grisolia%20LU1038395%20-%20Ingeniería%20Informática.pdf)**
- **[Slides: "ST25TB series NFC tags for fun in French public transports"](https://github.com/gentilkiwi/st25tb_kiemul/blob/main/ST25TB_transport.pdf)**
- **[YouTube Video: "ST25TB series NFC tags for fun in French public transports"](https://www.youtube.com/watch?v=CP0lt2ulJwE)**
- **[SSTIC 2024 Paper: "Tears for Fears: Breaking an RFID Counter"](https://www.sstic.org/media/SSTIC2024/SSTIC-actes/tears_for_fears_breaking_an_rfid_counter/SSTIC2024-Article-tears_for_fears_breaking_an_rfid_counter-marty_granier_delion_JHhE6Td.pdf)**  
- **[SSTIC 2024 Video: "Tears for Fears: Breaking an RFID Counter"](https://static.sstic.org/videos2024/1080p/tears_for_fears_breaking_an_rfid_counter.mp4)**  

## Hall of fame

- **C. Herrmann and P. Teuwen**: For opening Pandora's box.
- **G. Nahuel and Ukmar, F. Gabriel**: The first to use the PN532 for tear-off purposes.
- **R. Delion, P. Granier and J.J. Marty**: Pioneers in enabling tear-off on ST25TB tags.
- **Benjamin Delpy**: Special thanks to him for our discussions, which were invaluable in helping me with this project.

## License

This project is licensed under the **GPL v3 License**. See the [LICENSE](LICENSE.md) file for more details.

### Summary:

- You are free to share, modify, and distribute the code, as long as you comply with the terms of the GPL v3 license.
- Any modifications must be shared under the same GPL v3 license.
- You must provide a copy of the license with any redistribution of the code.

For more information, you can refer to the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.html)

**Copyright (C) 2024 Seclabz. All rights reserved.**
