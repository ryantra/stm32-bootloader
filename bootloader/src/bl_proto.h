#ifndef BL_PROTO_H_
#define BL_PROTO_H_

#include <stddef.h>
#include <stdint.h>

/* Framing and command set — see docs/protocol.md. Kept hardware-independent
 * so the parser can be unit-tested on the host against the Python uploader. */

#define BL_SOF          0xA5U
#define BL_VERSION      0x01U

/* commands (host -> bootloader) */
#define BL_CMD_HELLO    0x01U
#define BL_CMD_ERASE    0x02U
#define BL_CMD_WRITE    0x03U
#define BL_CMD_VERIFY   0x04U
#define BL_CMD_JUMP     0x05U

/* responses (bootloader -> host) */
#define BL_ACK          0x79U
#define BL_NACK         0x1FU

#define BL_MAX_PAYLOAD  512U

struct bl_frame {
	uint8_t  cmd;
	uint16_t len;
	uint8_t  payload[BL_MAX_PAYLOAD];
};

/* parse result codes */
enum bl_parse_result {
	BL_OK          =  0,
	BL_ERR_SHORT   = -1,   /* buffer too small to hold a full frame */
	BL_ERR_SOF     = -2,   /* first byte is not the start-of-frame marker */
	BL_ERR_LEN     = -3,   /* declared length exceeds BL_MAX_PAYLOAD / buffer */
	BL_ERR_CRC     = -4,   /* CRC mismatch */
};

/* Parse one complete frame from `buf` (raw bytes: SOF..CRC inclusive).
 * On success fills `out` and returns BL_OK; otherwise a negative bl_parse_result. */
int bl_frame_parse(const uint8_t *buf, size_t buf_len, struct bl_frame *out);

#endif /* BL_PROTO_H_ */
