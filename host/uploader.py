#!/usr/bin/env python3
"""Host-side firmware uploader for the STM32 UART bootloader.

Sends a raw application binary to the bootloader using the framed protocol
described in docs/protocol.md. This host side is fully implemented; it is the
reference the bootloader firmware is being built against.

Usage:
    python3 uploader.py --port /dev/ttyACM0 --image build/app/app.bin
"""

import argparse
import struct
import sys
import time
import zlib

try:
    import serial  # pyserial
except ImportError:
    sys.exit("pyserial is required:  pip install pyserial")

SOF = 0xA5
CMD_HELLO, CMD_ERASE, CMD_WRITE, CMD_VERIFY, CMD_JUMP = 0x01, 0x02, 0x03, 0x04, 0x05
ACK, NACK = 0x79, 0x1F
CHUNK = 256


def crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def frame(cmd: int, payload: bytes = b"") -> bytes:
    body = bytes([cmd]) + struct.pack("<H", len(payload)) + payload
    return bytes([SOF]) + body + struct.pack("<I", crc32(body))


def send(port: serial.Serial, cmd: int, payload: bytes = b"", expect_extra: int = 0) -> bytes:
    port.write(frame(cmd, payload))
    resp = port.read(1)
    if not resp:
        raise TimeoutError(f"no response to cmd 0x{cmd:02X}")
    if resp[0] == NACK:
        raise RuntimeError(f"bootloader NACKed cmd 0x{cmd:02X}")
    if resp[0] != ACK:
        raise RuntimeError(f"unexpected response 0x{resp[0]:02X} to cmd 0x{cmd:02X}")
    return port.read(expect_extra) if expect_extra else b""


def main() -> int:
    ap = argparse.ArgumentParser(description="Upload firmware to the STM32 UART bootloader")
    ap.add_argument("--port", required=True, help="serial port, e.g. /dev/ttyACM0")
    ap.add_argument("--image", required=True, help="application .bin to flash")
    ap.add_argument("--baud", type=int, default=115200)
    args = ap.parse_args()

    with open(args.image, "rb") as f:
        image = f.read()
    # pad to a word boundary
    if len(image) % 4:
        image += b"\xff" * (4 - len(image) % 4)

    print(f"Image: {len(image)} bytes, CRC32 = 0x{crc32(image):08X}")

    with serial.Serial(args.port, args.baud, timeout=2) as port:
        time.sleep(0.2)
        ver = send(port, CMD_HELLO, expect_extra=1)
        print(f"Bootloader v{ver[0] if ver else '?'}")

        print("Erasing application region...")
        send(port, CMD_ERASE)

        print("Writing...")
        for offset in range(0, len(image), CHUNK):
            data = image[offset:offset + CHUNK]
            send(port, CMD_WRITE, struct.pack("<I", offset) + data)
            done = (offset + len(data)) * 100 // len(image)
            print(f"\r  {done:3d}%", end="", flush=True)
        print()

        print("Verifying...")
        send(port, CMD_VERIFY, struct.pack("<I", crc32(image)))

        print("Jumping to application.")
        send(port, CMD_JUMP)

    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
