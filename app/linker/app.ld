/* Application linker script — STM32F411RE.
 * Linked to run from 0x08008000 (sectors 2-7). The app relocates its vector
 * table to this base via SCB->VTOR before running.
 */
ENTRY(Reset_Handler)

_estack = 0x20020000;

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08008000, LENGTH = 480K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text       : { *(.text*) *(.rodata*) } > FLASH
  .ARM        : { *(.ARM.exidx*) } > FLASH

  _sidata = LOADADDR(.data);
  .data : { _sdata = .; *(.data*) _edata = .; } > RAM AT > FLASH
  .bss  : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } > RAM
}
