/*
 * STM32F411RE bootloader — entry point.
 *
 * Responsibilities:
 *   1. On reset, decide whether to enter update mode or boot the app.
 *   2. In update mode, speak the UART protocol (see docs/protocol.md).
 *   3. Otherwise, relocate to the application and jump.
 *
 * NOTE: this is the milestone scaffold. The UART and flash drivers are being
 * filled in (see the TODOs). The jump logic below is the M1 target.
 */

#include <stdint.h>

#define APP_BASE   0x08008000U

typedef void (*app_entry_t)(void);

/* Does APP_BASE hold a plausible application?
 * A valid image starts with an initial stack pointer that lives in SRAM
 * (0x2000_0000..0x2002_0000 on the F411RE's 128 KB). If flash is erased the
 * word reads as 0xFFFFFFFF and we must NOT jump. */
static int app_is_present(void)
{
    uint32_t app_sp = *(volatile uint32_t *)(APP_BASE);
    return (app_sp >= 0x20000000U) && (app_sp <= 0x20020000U);
}

/* Jump to the application at APP_BASE: load its stack pointer and reset vector,
 * relocate the vector table, then transfer control. Never returns. */
static void boot_application(void)
{
    uint32_t app_sp    = *(volatile uint32_t *)(APP_BASE);
    uint32_t app_reset = *(volatile uint32_t *)(APP_BASE + 4U);

    /* Relocate the vector table to the application. SCB->VTOR = 0xE000ED08. */
    *(volatile uint32_t *)0xE000ED08U = APP_BASE;

    __asm volatile ("msr msp, %0" :: "r" (app_sp) : );
    ((app_entry_t)app_reset)();
}

/* TODO M2: initialise UART, run the command loop from docs/protocol.md. */
/* TODO M3: implement flash erase + chunked write (bl_flash.c). */

int main(void)
{
    /* TODO M2: if an update is requested (e.g. button held, or magic word in
     * a known RAM/flash location), stay in the bootloader and serve updates.
     * For now, boot straight through to the application when one is present. */
    if (app_is_present()) {
        boot_application();
    }

    /* No valid application (or boot_application unexpectedly returned):
     * spin here. M2 will replace this with the UART update loop. */
    for (;;) { }
    return 0;
}
