cmake_minimum_required(VERSION 3.20)
project(bootloader C ASM)

set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

add_executable(bootloader.elf
    src/main.c
    src/bl_uart.c
    src/bl_proto.c
    src/bl_crc32.c
    src/startup_stm32f411retx.s
    # src/bl_flash.c  # TODO M3
)

target_compile_options(bootloader.elf PRIVATE ${MCU_FLAGS} -Wall -ffunction-sections -fdata-sections -Os -g3)
target_link_options(bootloader.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_SOURCE_DIR}/linker/bootloader.ld
    --specs=nano.specs --specs=nosys.specs
    -Wl,--gc-sections -Wl,-Map=bootloader.map -Wl,--print-memory-usage
)
add_custom_command(TARGET bootloader.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:bootloader.elf> bootloader.bin)
