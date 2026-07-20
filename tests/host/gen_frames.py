#!/usr/bin/env python3
"""Generate protocol frames (byte-identical to host/uploader.py) for the C
parser interop test. Writes length-prefixed frames to the given file."""
import struct
import sys
import zlib

SOF = 0xA5


def frame(cmd, payload=b""):
    body = bytes([cmd]) + struct.pack("<H", len(payload)) + payload
    return bytes([SOF]) + body + struct.pack("<I", zlib.crc32(body) & 0xFFFFFFFF)


def main():
    frames = [
        frame(0x01),                                        # HELLO
        frame(0x03, struct.pack("<I", 0) + bytes(range(16))),  # WRITE
        frame(0x05),                                        # JUMP
    ]
    out = bytearray()
    for f in frames:
        out += struct.pack("<I", len(f)) + f
    bad = bytearray(frame(0x01))
    bad[1] ^= 0xFF                                          # corrupt -> CRC mismatch
    out += struct.pack("<I", len(bad)) + bad
    with open(sys.argv[1], "wb") as fp:
        fp.write(out)


if __name__ == "__main__":
    main()
