#!/usr/bin/env python3
"""Renders display screens dumped by `build/test --dump-screens DIR` to PNG files.

Mirrors the painter of the Display widget (src/Widget.cpp) closely enough to check layouts against
the photos in the Sinfonion manual without starting VCV Rack. Needs Pillow.

    python3 tools/preview_display.py DIR [OUT_DIR]
"""
import json
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

SCALE = 4  # image pixels per display pixel
X_SCALE = 0.78  # keep in sync with Screen::X_SCALE
MAX_WIDTH = 124.0
AMBER = (255, 179, 71)
FONT = os.path.join(os.path.dirname(__file__), "..", "res", "fonts", "MartianMono-Display.ttf")
GLYPHS = {"♭": 0.45, "♯": 0.55, "△": 0.72, "·": 0.32, "→": 0.62}
_fonts = {}


def font(size):
    key = round(size, 2)
    if key not in _fonts:
        _fonts[key] = ImageFont.truetype(FONT, max(1, int(round(size))))
    return _fonts[key]


def pieces(run):
    for seg in run["segs"]:
        e = seg["size"] * SCALE
        plain = ""
        for ch in seg["text"]:
            if ch in GLYPHS:
                if plain:
                    yield ("text", plain, e, seg["rise"])
                    plain = ""
                yield ("glyph", ch, e, seg["rise"])
            else:
                plain += ch
        if plain:
            yield ("text", plain, e, seg["rise"])


def width(p):
    kind, t, e, _ = p
    return font(e).getlength(t) if kind == "text" else GLYPHS[t] * e


def draw_glyph(d, g, x, y, e):
    w = max(2, int(0.09 * e))
    if g == "♭":
        d.line([(x + 0.1 * e, y - 0.78 * e), (x + 0.1 * e, y)], AMBER, w)
        pts = [(x + 0.1 * e + 0.3 * e * math.sin(t / 10 * math.pi), y - 0.3 * e + 0.3 * e * t / 10) for t in range(11)]
        d.line(pts, AMBER, w)
    elif g == "♯":
        for a, b in [((0.18, -0.75), (0.16, 0.05)), ((0.37, -0.8), (0.35, 0)), ((0.04, -0.45), (0.5, -0.55)), ((0.02, -0.2), (0.48, -0.3))]:
            d.line([(x + a[0] * e, y + a[1] * e), (x + b[0] * e, y + b[1] * e)], AMBER, w)
    elif g == "△":
        d.polygon([(x + 0.06 * e, y), (x + 0.64 * e, y), (x + 0.35 * e, y - 0.68 * e)], outline=AMBER, width=w)
    elif g == "·":
        d.rectangle([x + 0.09 * e, y - 0.42 * e, x + 0.23 * e, y - 0.28 * e], fill=AMBER)
    elif g == "→":
        d.line([(x + 0.04 * e, y - 0.36 * e), (x + 0.54 * e, y - 0.36 * e)], AMBER, w)
        d.line([(x + 0.34 * e, y - 0.54 * e), (x + 0.56 * e, y - 0.36 * e), (x + 0.34 * e, y - 0.18 * e)], AMBER, w)


def render(screen):
    img = Image.new("RGB", (128 * SCALE, 64 * SCALE), "black")
    d = ImageDraw.Draw(img)
    lw = max(1, int(0.6 * SCALE))
    for sh in screen["shapes"]:
        x, y, w, h = (v * SCALE for v in (sh["x"], sh["y"], sh["w"], sh["h"]))
        kind = sh["kind"]
        if kind == 0:
            d.rectangle([x, y, x + w, y + h], fill=AMBER if sh["fill"] else None, outline=AMBER, width=lw)
        elif kind == 1:
            d.ellipse([x - w, y - w, x + w, y + w], fill=AMBER if sh["fill"] else None, outline=AMBER, width=lw)
        elif kind == 2:
            d.line([(x, y), (w, h)], AMBER, lw)
        else:
            length = math.hypot(w - x, h - y)
            n = int(length / (1.8 * SCALE)) + 1
            for i in range(n):
                a = i * 1.8 * SCALE / length if length else 0
                px, py = x + (w - x) * a, y + (h - y) * a
                d.rectangle([px - 0.4 * SCALE, py - 0.4 * SCALE, px + 0.4 * SCALE, py + 0.4 * SCALE], fill=AMBER)
    for run in screen["runs"]:
        ps = list(pieces(run))
        total = sum(width(p) for p in ps)
        if total <= 0:
            continue
        layer = Image.new("RGB", (int(math.ceil(total)) + 8, 64 * SCALE), "black")
        ld = ImageDraw.Draw(layer)
        base = run["y"] * SCALE
        x = 0.0
        for p in ps:
            kind, t, e, rise = p
            y = base - rise * SCALE
            if kind == "text":
                ld.text((x, y), t, font=font(e), fill=AMBER, anchor="ls")
            else:
                draw_glyph(ld, t, x, y, e)
            x += width(p)
        kx = X_SCALE
        w = total * kx
        if w > MAX_WIDTH * SCALE:
            kx *= MAX_WIDTH * SCALE / w
            w = MAX_WIDTH * SCALE
        layer = layer.resize((max(1, int(layer.width * kx)), layer.height), Image.LANCZOS)
        left = run["x"] * SCALE - (w / 2 if run["align"] == 1 else w if run["align"] == 2 else 0)
        mask = layer.convert("L")
        img.paste(layer, (int(round(left)), 0), mask)
    return img


def main():
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else src
    os.makedirs(out, exist_ok=True)
    for name in sorted(os.listdir(src)):
        if name.endswith(".json"):
            with open(os.path.join(src, name), encoding="utf-8") as f:
                render(json.load(f)).save(os.path.join(out, name[:-5] + ".png"))
            print(name[:-5] + ".png")


if __name__ == "__main__":
    main()
