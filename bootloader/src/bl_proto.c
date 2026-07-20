#include "bl_proto.h"
#include "bl_crc32.h"

/* Frame layout (see docs/protocol.md):
 *   [0]      SOF (0xA5)
 *   [1]      CMD
 *   [2..3]   LEN  (little-endian uint16)
 *   [4..]    PAYLOAD (LEN bytes)
 *   [4+LEN.] CRC32 (little-endian) over CMD+LEN+PAYLOAD
 */
int bl_frame_parse(const uint8_t *buf, size_t buf_len, struct bl_frame *out)
{
	if (buf_len < 8U) {
		return BL_ERR_SHORT;
	}
	if (buf[0] != BL_SOF) {
		return BL_ERR_SOF;
	}

	uint16_t len = (uint16_t)(buf[2] | ((uint16_t)buf[3] << 8));
	if (len > BL_MAX_PAYLOAD || buf_len < (size_t)(8U + len)) {
		return BL_ERR_LEN;
	}

	/* CRC covers CMD+LEN+PAYLOAD, i.e. buf[1 .. 4+len). */
	uint32_t want = bl_crc32(&buf[1], (size_t)(3U + len));
	const uint8_t *c = &buf[4U + len];
	uint32_t got = (uint32_t)c[0] | ((uint32_t)c[1] << 8) |
		       ((uint32_t)c[2] << 16) | ((uint32_t)c[3] << 24);
	if (want != got) {
		return BL_ERR_CRC;
	}

	out->cmd = buf[1];
	out->len = len;
	for (uint16_t i = 0U; i < len; i++) {
		out->payload[i] = buf[4U + i];
	}
	return BL_OK;
}
