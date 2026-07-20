# STM32 Custom Bootloader + UART Firmware Updater

![CI](https://github.com/ryantra/stm32-bootloader/actions/workflows/ci.yml/badge.svg)
![MCU](https://img.shields.io/badge/MCU-STM32F411RE-03234B?logo=stmicroelectronics&logoColor=white)
![Core](https://img.shields.io/badge/Core-ARM_Cortex--M4-0091BD?logo=arm&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green)

A custom bootloader for the STM32F411RE that can flash a new application image over UART,
verify it with a CRC, and hand control to the application. Includes a Python host tool that
sends the image, and a demo application that lives in a separate flash region.

> **Status: in progress.** M1 is done — the bootloader and application build, link to
> their separate flash regions, and the bootloader validates and jumps to the app with
> vector-table relocation. UART transfer and flash programming (M2–M3) are next. Files
> marked `TODO` are under active development.

## Why a bootloader

Field firmware updates are a core embedded problem, and a bootloader touches nearly every
low-level skill at once: flash programming, linker scripts and memory layout, vector-table
relocation, and communication-protocol design. That is exactly why I picked it.

## Memory map (STM32F411RE, 512 KB flash)

| Region | Address range | Size | Contents |
|---|---|---|---|
| Bootloader | `0x08000000`–`0x08007FFF` | 32 KB (sectors 0–1) | this bootloader, runs first at reset |
| Application | `0x08008000`–`0x0807FFFF` | 480 KB (sectors 2–7) | the user application |

The bootloader keeps the reset vector. The application relocates its vector table to
`0x08008000` (`SCB->VTOR`) before running.

## Update protocol (UART)

```mermaid
sequenceDiagram
    participant H as Host (uploader.py)
    participant B as Bootloader
    H->>B: HELLO
    B-->>H: ACK (+ bootloader version)
    H->>B: ERASE (app region)
    B-->>H: ACK
    loop each chunk
        H->>B: WRITE(offset, data, CRC32)
        B-->>H: ACK / NACK
    end
    H->>B: VERIFY(total CRC32)
    B-->>H: ACK
    H->>B: JUMP
    B-->>H: (boots application)
```

Full frame format and command bytes are in [`docs/protocol.md`](docs/protocol.md).

## Repository layout

```
├── bootloader/     bootloader firmware (UART + flash driver + jump logic)
│   ├── src/        main.c, bl_uart, bl_flash, bl_commands  (TODO: logic)
│   └── linker/     bootloader.ld  (linked at 0x08000000)
├── app/            demo application linked at 0x08008000
│   ├── src/        main.c  (blinky + VTOR relocation)
│   └── linker/     app.ld
├── host/           uploader.py — sends an image over serial
├── cmake/          arm-none-eabi-gcc toolchain file
└── docs/           protocol specification
```

## Build

```bash
sudo apt-get install gcc-arm-none-eabi cmake
cmake -S bootloader -B build/bl  -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake
cmake --build build/bl
cmake -S app -B build/app -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake
cmake --build build/app
```

## Flash a firmware update

```bash
# one-time: flash the bootloader itself
st-flash write build/bl/bootloader.bin 0x08000000

# then push application updates over UART, no debugger needed:
python3 host/uploader.py --port /dev/ttyACM0 --image build/app/app.bin
```

## Milestones

- [x] M1 — App links at `0x08008000`; bootloader validates and jumps to it (VTOR relocation working). Verified in CI: both images build and land at the correct addresses.
- [ ] M2 — UART command interface in the bootloader (HELLO/ACK)
- [ ] M3 — Flash erase + chunked write driver
- [ ] M4 — Host uploader with per-chunk and whole-image CRC32
- [ ] M5 — README demo GIF + `v1.0` release

## Skills demonstrated

Bare-metal C, ARM Cortex-M4, flash programming, linker scripts, vector-table relocation,
UART protocol design, CRC verification, and host/target tooling (Python).

## License

MIT — see [LICENSE](LICENSE).
