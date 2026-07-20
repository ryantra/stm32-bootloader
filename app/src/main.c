/*
 * Demo application (M1), linked to run from 0x08008000.
 *
 * Proves the bootloader handed over control correctly by blinking the Nucleo
 * user LED (LD2 = PA5) with bare-metal register access. The bootloader sets
 * SCB->VTOR before jumping; we set it again defensively.
 */

#include <stdint.h>

#define APP_BASE    0x08008000U
#define SCB_VTOR    (*(volatile uint32_t *)0xE000ED08U)

/* RCC */
#define RCC_BASE    0x40023800U
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30U))
#define GPIOAEN     (1U << 0)

/* GPIOA (LD2 = PA5) */
#define GPIOA_BASE  0x40020000U
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
#define LED_PIN     5U

static void delay(volatile uint32_t n)
{
	while (n--) {
		__asm volatile ("nop");
	}
}

int main(void)
{
	SCB_VTOR = APP_BASE;

	/* enable GPIOA clock, set PA5 as output (MODER[11:10] = 01) */
	RCC_AHB1ENR |= GPIOAEN;
	GPIOA_MODER &= ~(3U << (LED_PIN * 2U));
	GPIOA_MODER |=  (1U << (LED_PIN * 2U));

	for (;;) {
		GPIOA_ODR ^= (1U << LED_PIN);   /* toggle LD2 */
		delay(800000U);
	}

	return 0;
}
