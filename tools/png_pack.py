#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Pack PNG files in place: the host simulator writes its screenshots with stored (unpacked) data, about
173 KB a screen. The pixels are kept as they are; only the IDAT stream is deflated again.

  tools/png_pack.py FILE.png ..."""
import struct
import sys
import zlib

SIG = b"\x89PNG\r\n\x1a\n"


def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)


def pack(path):
    raw = open(path, "rb").read()
    if raw[:8] != SIG:
        raise SystemExit(f"{path}: not a PNG")
    pos, head, idat, tail = 8, [], b"", []
    while pos < len(raw):
        n, kind = struct.unpack(">I4s", raw[pos:pos + 8])
        data = raw[pos + 8:pos + 8 + n]
        pos += 12 + n
        if kind == b"IDAT":
            idat += data
        elif kind == b"IEND":
            break
        elif idat:
            tail.append(chunk(kind, data))
        else:
            head.append(chunk(kind, data))
    out = SIG + b"".join(head) + chunk(b"IDAT", zlib.compress(zlib.decompress(idat), 9)) + b"".join(tail) + chunk(b"IEND", b"")
    if len(out) < len(raw):
        open(path, "wb").write(out)
    return len(raw), min(len(out), len(raw))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    a = b = 0
    for p in sys.argv[1:]:
        x, y = pack(p)
        a += x
        b += y
    print(f"png_pack: {len(sys.argv) - 1} files, {a} B to {b} B")
