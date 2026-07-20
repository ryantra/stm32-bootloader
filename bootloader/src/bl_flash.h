#ifndef BL_FLASH_H_
#define BL_FLASH_H_

#include <stddef.h>
#include <stdint.h>

/* Application region on the STM32F411RE (512 KB flash):
 *   sector 2 @ 0x08008000 .. sector 7 @ 0x0807FFFF  =  480 KB.
 * The bootloader occupies sectors 0-1 and must never touch them. */
#define BL_APP_BASE   0x08008000U
#define BL_APP_SIZE   (480U * 1024U)

/* Pure, hardware-independent bounds check (host-testable).
 * True iff [offset, offset+len) fits entirely within the app region, with no
 * unsigned overflow. */
static inline int bl_offset_in_range(uint32_t offset, uint32_t len)
{
	if (len == 0U) {
		return 1;
	}
	if (offset >= BL_APP_SIZE) {
		return 0;
	}
	if (len > (BL_APP_SIZE - offset)) {
		return 0;
	}
	return 1;
}

/* Unlock the flash controller (call once before erase/write). */
void bl_flash_init(void);

/* Erase the whole application region (sectors 2-7). Returns 0 on success. */
int bl_flash_erase_app(void);

/* Program `len` bytes at BL_APP_BASE + offset. `offset` must be word-aligned;
 * the region must already be erased. Returns 0 on success, negative on error
 * (out of range, misaligned, or flash reported/verify failure). */
int bl_flash_write(uint32_t offset, const uint8_t *data, uint32_t len);

#endif /* BL_FLASH_H_ */
