#ifndef BL_CRC32_H_
#define BL_CRC32_H_

#include <stddef.h>
#include <stdint.h>

/* Standard CRC-32 (IEEE 802.3, reflected, init 0xFFFFFFFF, xorout 0xFFFFFFFF).
 * Bit-compatible with Python's zlib.crc32, which the host uploader uses.
 * Canonical check: bl_crc32("123456789", 9) == 0xCBF43926. */
uint32_t bl_crc32(const uint8_t *data, size_t len);

#endif /* BL_CRC32_H_ */
