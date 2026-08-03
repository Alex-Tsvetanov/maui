#!/usr/bin/env python3
"""Where do two captures diverge? Per-row differing-pixel fraction.

A page-level "21.5% differ" says nothing about WHICH element broke. Banding the page means the
first sustained divergent band names the culprit -- and because a layout shift makes everything
BELOW it differ too, the TOP of that first band is the thing to fix, not the biggest band.
"""
import sys, zlib, struct

def rows(path):
    d = open(path, "rb").read()
    pos, idat, w, h, bd, ct = 8, b"", 0, 0, 0, 0
    while pos < len(d):
        ln = struct.unpack(">I", d[pos:pos+4])[0]; typ = d[pos+4:pos+8]
        if typ == b"IHDR":
            w, h, bd, ct = *struct.unpack(">II", d[pos+8:pos+16]), d[pos+16], d[pos+17]
        elif typ == b"IDAT":
            idat += d[pos+8:pos+8+ln]
        pos += 12 + ln
    ch = {0:1, 2:3, 4:2, 6:4}[ct]
    raw = zlib.decompress(idat); stride = w*ch; out = []; prev = bytearray(stride)
    i = 0
    for _ in range(h):
        f = raw[i]; i += 1
        line = bytearray(raw[i:i+stride]); i += stride
        for x in range(stride):
            a = line[x-ch] if x >= ch else 0
            b = prev[x]; c = prev[x-ch] if x >= ch else 0
            if f == 1: line[x] = (line[x]+a) & 255
            elif f == 2: line[x] = (line[x]+b) & 255
            elif f == 3: line[x] = (line[x]+(a+b)//2) & 255
            elif f == 4:
                p = a+b-c; pa, pb, pc = abs(p-a), abs(p-b), abs(p-c)
                line[x] = (line[x] + (a if pa<=pb and pa<=pc else b if pb<=pc else c)) & 255
        out.append(bytes(line)); prev = line
    return w, h, ch, out


def main():
    wa, ha, ca, a = rows(sys.argv[1])
    wb, hb, cb, b = rows(sys.argv[2])
    band = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    # Tolerance matters: this port has a known uniform +-1 background delta, so EXACT equality
    # flags every background pixel and drowns the structure we are looking for.
    tol = int(sys.argv[4]) if len(sys.argv) > 4 else 8
    h = min(ha, hb)
    print(f"a={wa}x{ha}({ca}ch)  b={wb}x{hb}({cb}ch)  comparing {h} rows")
    for top in range(0, h, band):
        tot = dif = 0
        for y in range(top, min(top + band, h)):
            ra, rb = a[y], b[y]
            for x in range(0, min(wa, wb)):
                tot += 1
                pa, pb = ra[x*ca:x*ca+3], rb[x*cb:x*cb+3]
                if max(abs(pa[k] - pb[k]) for k in range(3)) > tol:
                    dif += 1
        pct = 100.0 * dif / max(tot, 1)
        print(f"  y {top:5}-{min(top+band,h):5}  {pct:6.1f}%  {'#' * int(pct / 2)}")


# Guarded so rowshift.py can import `rows` (one PNG decoder in the tree, not two).
if __name__ == "__main__":
    main()
