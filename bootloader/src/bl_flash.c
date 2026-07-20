#include "bl_flash.h"

/* --- STM32F411 embedded-flash-controller registers (base 0x40023C00) --- */
#define FLASH_BASE      0x40023C00U
#define FLASH_KEYR      (*(volatile uint32_t *)(FLASH_BASE + 0x04U))
#define FLASH_SR        (*(volatile uint32_t *)(FLASH_BASE + 0x0CU))
#define FLASH_CR        (*(volatile uint32_t *)(FLASH_BASE + 0x10U))

#define FLASH_KEY1      0x45670123U
#define FLASH_KEY2      0xCDEF89ABU

#define SR_BSY          (1U << 16)
#define SR_ERR_MASK     ((1U << 4) | (1U << 5) | (1U << 6) | (1U << 7)) /* WRP/PGA/PGP/PGS */

#define CR_PG           (1U << 0)
#define CR_SER          (1U << 1)
#define CR_SNB_POS      3
#define CR_SNB_MASK     (0xFU << CR_SNB_POS)
#define CR_STRT         (1U << 16)
#define CR_LOCK         (1U << 31)
#define CR_PSIZE_MASK   (3U << 8)
#define CR_PSIZE_X32    (2U << 8)      /* program 32 bits at a time (Vdd >= 2.7V) */

/* First and last app sectors (see bl_flash.h). */
#define APP_FIRST_SECTOR 2U
#define APP_LAST_SECTOR  7U

static void flash_wait(void)
{
	while (FLASH_SR & SR_BSY) {
	}
}

static int flash_clear_errors(void)
{
	if (FLASH_SR & SR_ERR_MASK) {
		FLASH_SR = SR_ERR_MASK;   /* write-1-to-clear */
		return -1;
	}
	return 0;
}

void bl_flash_init(void)
{
	if (FLASH_CR & CR_LOCK) {
		FLASH_KEYR = FLASH_KEY1;
		FLASH_KEYR = FLASH_KEY2;
	}
	(void)flash_clear_errors();
}

int bl_flash_erase_app(void)
{
	for (uint32_t s = APP_FIRST_SECTOR; s <= APP_LAST_SECTOR; s++) {
		flash_wait();
		FLASH_CR &= ~(CR_SNB_MASK | CR_PSIZE_MASK);
		FLASH_CR |= CR_SER | (s << CR_SNB_POS) | CR_PSIZE_X32;
		FLASH_CR |= CR_STRT;
		flash_wait();
		FLASH_CR &= ~CR_SER;

		if (flash_clear_errors() != 0) {
			return -1;
		}
	}
	return 0;
}

int bl_flash_write(uint32_t offset, const uint8_t *data, uint32_t len)
{
	if (!bl_offset_in_range(offset, len)) {
		return -1;
	}
	if ((offset & 3U) != 0U) {
		return -1;   /* must be word-aligned */
	}

	volatile uint32_t *dst = (volatile uint32_t *)(BL_APP_BASE + offset);
	uint32_t words = (len + 3U) / 4U;

	for (uint32_t i = 0U; i < words; i++) {
		/* assemble a little-endian word, padding the tail with 0xFF
		 * (the erased state, so padding is effectively a no-op) */
		uint8_t wb[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
		for (uint32_t b = 0U; b < 4U; b++) {
			uint32_t idx = (i * 4U) + b;
			if (idx < len) {
				wb[b] = data[idx];
			}
		}
		uint32_t w = (uint32_t)wb[0] | ((uint32_t)wb[1] << 8) |
			     ((uint32_t)wb[2] << 16) | ((uint32_t)wb[3] << 24);

		flash_wait();
		FLASH_CR &= ~CR_PSIZE_MASK;
		FLASH_CR |= CR_PSIZE_X32 | CR_PG;
		dst[i] = w;
		flash_wait();
		FLASH_CR &= ~CR_PG;

		if (flash_clear_errors() != 0 || dst[i] != w) {
			return -1;   /* flash error or readback mismatch */
		}
	}
	return 0;
}
