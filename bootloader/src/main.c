/*
 * STM32F411RE bootloader — entry point.
 *
 * Boot policy:
 *   - Briefly listen on the UART. If the host starts talking, enter update mode.
 *   - Otherwise, if a valid application is present, jump to it.
 *   - If neither, stay in the update loop waiting for a host.
 *
 * M1: validate + jump to the application (done).
 * M2: UART command interface (this file + bl_uart / bl_proto).
 * M3: flash erase/write/verify (TODO — ERASE/WRITE/VERIFY currently NACK).
 */

#include <stdint.h>
#include "bl_uart.h"
#include "bl_proto.h"

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

/* Read one framed command off the UART into `buf` and parse it.
 * Blocks until a full frame is received. Returns a bl_parse_result. */
static int read_command(uint8_t *buf, struct bl_frame *out)
{
	/* sync to start-of-frame */
	while (bl_uart_getc() != BL_SOF) {
	}
	buf[0] = BL_SOF;
	buf[1] = bl_uart_getc();                 /* CMD */
	buf[2] = bl_uart_getc();                 /* LEN lo */
	buf[3] = bl_uart_getc();                 /* LEN hi */

	uint16_t len = (uint16_t)(buf[2] | ((uint16_t)buf[3] << 8));
	if (len > BL_MAX_PAYLOAD) {
		return BL_ERR_LEN;
	}

	size_t total = (size_t)(8U + len);       /* SOF+CMD+LEN + payload + CRC */
	for (size_t i = 4U; i < total; i++) {
		buf[i] = bl_uart_getc();
	}
	return bl_frame_parse(buf, total, out);
}

static void handle_frame(const struct bl_frame *f)
{
	switch (f->cmd) {
	case BL_CMD_HELLO:
		bl_uart_putc(BL_ACK);
		bl_uart_putc(BL_VERSION);
		break;

	case BL_CMD_JUMP:
		bl_uart_putc(BL_ACK);
		if (app_is_present()) {
			boot_application();          /* does not return */
		}
		break;

	case BL_CMD_ERASE:
	case BL_CMD_WRITE:
	case BL_CMD_VERIFY:
		/* TODO M3: implement flash erase / chunked write / verify. */
		bl_uart_putc(BL_NACK);
		break;

	default:
		bl_uart_putc(BL_NACK);
		break;
	}
}

/* Poll the UART for a short window; return non-zero if the host is knocking. */
static int host_knocking(uint32_t spins)
{
	while (spins--) {
		if (bl_uart_has_byte()) {
			return 1;
		}
	}
	return 0;
}

int main(void)
{
	static uint8_t buf[8U + BL_MAX_PAYLOAD];
	struct bl_frame frame;

	bl_uart_init();

	/* No host activity and a valid app present -> just run the app. */
	if (!host_knocking(2000000U) && app_is_present()) {
		boot_application();
	}

	/* Update mode: serve framed commands until a JUMP boots the app. */
	for (;;) {
		if (read_command(buf, &frame) == BL_OK) {
			handle_frame(&frame);
		} else {
			bl_uart_putc(BL_NACK);
		}
	}

	return 0;
}
