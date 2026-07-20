#include "bl_uart.h"

/* --- register map (STM32F411, USART2 on APB1) --- */
#define RCC_BASE       0x40023800U
#define RCC_AHB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x30U))
#define RCC_APB1ENR    (*(volatile uint32_t *)(RCC_BASE + 0x40U))
#define GPIOAEN        (1U << 0)
#define USART2EN       (1U << 17)

#define GPIOA_BASE     0x40020000U
#define GPIOA_MODER    (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_AFRL     (*(volatile uint32_t *)(GPIOA_BASE + 0x20U))

#define USART2_BASE    0x40004400U
#define USART2_SR      (*(volatile uint32_t *)(USART2_BASE + 0x00U))
#define USART2_DR      (*(volatile uint32_t *)(USART2_BASE + 0x04U))
#define USART2_BRR     (*(volatile uint32_t *)(USART2_BASE + 0x08U))
#define USART2_CR1     (*(volatile uint32_t *)(USART2_BASE + 0x0CU))

#define SR_RXNE        (1U << 5)
#define SR_TXE         (1U << 7)
#define CR1_RE         (1U << 2)
#define CR1_TE         (1U << 3)
#define CR1_UE         (1U << 13)

/* After reset the core runs on HSI = 16 MHz and APB1 prescaler = 1, so
 * PCLK1 = 16 MHz. BRR = PCLK1 / baud = 16e6 / 115200 ~= 139 (0x8B). */
#define BRR_115200_AT_16MHZ  139U

void bl_uart_init(void)
{
	RCC_AHB1ENR |= GPIOAEN;
	RCC_APB1ENR |= USART2EN;

	/* PA2, PA3 -> alternate function mode (MODER = 0b10) */
	GPIOA_MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
	GPIOA_MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2)));

	/* AF7 (USART2) on PA2 and PA3 */
	GPIOA_AFRL &= ~((0xFU << (2 * 4)) | (0xFU << (3 * 4)));
	GPIOA_AFRL |=  ((7U   << (2 * 4)) | (7U   << (3 * 4)));

	USART2_BRR = BRR_115200_AT_16MHZ;
	USART2_CR1 = CR1_TE | CR1_RE | CR1_UE;
}

void bl_uart_putc(uint8_t c)
{
	while (!(USART2_SR & SR_TXE)) {
	}
	USART2_DR = (uint32_t)c;
}

uint8_t bl_uart_getc(void)
{
	while (!(USART2_SR & SR_RXNE)) {
	}
	return (uint8_t)(USART2_DR & 0xFFU);
}

int bl_uart_has_byte(void)
{
	return (USART2_SR & SR_RXNE) ? 1 : 0;
}
