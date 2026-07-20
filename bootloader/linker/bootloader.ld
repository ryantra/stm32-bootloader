/* Bootloader linker script — STM32F411RE.
 * Occupies sectors 0-1 (32 KB) at the start of flash and keeps the reset vector.
 */
ENTRY(Reset_Handler)

_estack = 0x20020000;   /* top of 128 KB SRAM */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Base of the application region — the bootloader jumps here. */
_app_base = 0x08008000;

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text       : { *(.text*) *(.rodata*) } > FLASH
  .ARM        : { *(.ARM.exidx*) } > FLASH

  _sidata = LOADADDR(.data);
  .data : { _sdata = .; *(.data*) _edata = .; } > RAM AT > FLASH
  .bss  : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } > RAM
}
