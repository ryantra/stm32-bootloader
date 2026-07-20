cmake_minimum_required(VERSION 3.20)
project(app C ASM)

# NOTE: add a Cortex-M4 startup file to src/ before this will link.
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

add_executable(app.elf
    src/main.c
    # src/startup_stm32f411retx.s   # TODO: add startup
)

target_compile_options(app.elf PRIVATE ${MCU_FLAGS} -Wall -ffunction-sections -fdata-sections -Os -g3)
target_link_options(app.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_SOURCE_DIR}/linker/app.ld
    --specs=nano.specs --specs=nosys.specs
    -Wl,--gc-sections -Wl,-Map=app.map -Wl,--print-memory-usage
)
add_custom_command(TARGET app.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:app.elf> app.bin)
