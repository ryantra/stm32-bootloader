/*
 * Demo application, linked to run from 0x08008000.
 *
 * The bootloader already sets SCB->VTOR before jumping here, but we set it
 * again defensively so the app is also correct when flashed/run standalone.
 */

#include <stdint.h>

#define APP_BASE   0x08008000U
#define SCB_VTOR   (*(volatile uint32_t *)0xE000ED08U)

/* TODO M1: replace with a real GPIO toggle on the NUCLEO user LED (PA5). */
static void led_blink(void)
{
    for (volatile uint32_t i = 0; i < 1000000U; ++i) { }
}

int main(void)
{
    SCB_VTOR = APP_BASE;

    for (;;) {
        led_blink();
    }
    return 0;
}
