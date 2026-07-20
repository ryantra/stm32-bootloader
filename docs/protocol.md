# UART firmware-update protocol

Simple, framed, request/response protocol between the host and the bootloader.

## Serial settings

- 115200 baud, 8 data bits, no parity, 1 stop bit (8N1)

## Frame format

```
+--------+--------+-----------------+-----------+
|  SOF   |  CMD   |  LEN (2 bytes)  |  PAYLOAD   |  CRC32 (4 bytes)  |
| 0xA5   | 1 byte |  little-endian  |  LEN bytes |  over CMD..PAYLOAD |
+--------+--------+-----------------+-----------+
```

- `SOF` — start of frame, always `0xA5`
- `CMD` — command byte (below)
- `LEN` — payload length, little-endian uint16
- `PAYLOAD` — command-specific
- `CRC32` — CRC-32 (IEEE 802.3, poly `0x04C11DB7`) over CMD + LEN + PAYLOAD

## Commands (host → bootloader)

| CMD  | Name   | Payload | Meaning |
|------|--------|---------|---------|
| 0x01 | HELLO  | none | probe; bootloader replies with version |
| 0x02 | ERASE  | none | erase the application region (sectors 2–7) |
| 0x03 | WRITE  | `offset:u32` + data | write `data` at `app_base + offset` |
| 0x04 | VERIFY | `crc32:u32` | verify CRC32 of the whole written image |
| 0x05 | JUMP   | none | relocate VTOR and boot the application |

## Responses (bootloader → host)

| Byte | Name | Meaning |
|------|------|---------|
| 0x79 | ACK  | success (HELLO also returns 1 version byte) |
| 0x1F | NACK | failure (bad CRC, out-of-range offset, flash error) |

## Notes

- The host waits for an ACK/NACK after every frame before sending the next.
- WRITE offsets must be word-aligned; the bootloader rejects unaligned or out-of-range writes.
- The bootloader never erases its own region (sectors 0–1).
