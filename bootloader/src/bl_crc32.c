#include "bl_crc32.h"

uint32_t bl_crc32(const uint8_t *data, size_t len)
{
	uint32_t crc = 0xFFFFFFFFU;

	for (size_t i = 0U; i < len; i++) {
		crc ^= data[i];
		for (int bit = 0; bit < 8; bit++) {
			/* if LSB set, shift and xor with the reflected polynomial */
			uint32_t mask = (uint32_t)(-(int32_t)(crc & 1U));
			crc = (crc >> 1) ^ (0xEDB88320U & mask);
		}
	}

	return ~crc;
}
