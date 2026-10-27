#pragma once
// Visual themes of the panel (see docs/art/STYLE.md): betobullet stencil, black and white only.
// "Black" (default) and "White" (inverse); the "accent" that marks everything that is on is white
// on the black panel and black on the white one.
#include "plugin.hpp"

namespace hm {

struct Theme {
	NVGcolor panel, plate, outline, ink, grey, black, off, accent, accentDim;
};

static const int NUM_THEMES = 2;
static const char* const THEME_NAMES[NUM_THEMES] = {"Black", "White"};

inline const Theme& themeColors(int t) {
	static const Theme T[NUM_THEMES] = {
		// Black
		{nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0xF1, 0xF1, 0xEE),
		 nvgRGB(0xB4, 0xB4, 0xB2), nvgRGB(0x00, 0x00, 0x00), nvgRGB(0x26, 0x26, 0x26), nvgRGB(0xF4, 0xF4, 0xF2),
		 nvgRGB(0x5E, 0x5E, 0x5C)},
		// White
		{nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0xF1, 0xF1, 0xEE), nvgRGB(0x0B, 0x0B, 0x0B), nvgRGB(0x0B, 0x0B, 0x0B),
		 nvgRGB(0x50, 0x50, 0x50), nvgRGB(0x00, 0x00, 0x00), nvgRGB(0xDA, 0xDA, 0xD6), nvgRGB(0xF4, 0xF4, 0xF2),
		 nvgRGB(0x8A, 0x8A, 0x88)},
	};
	return T[t < 0 ? 0 : (t >= NUM_THEMES ? NUM_THEMES - 1 : t)];
}

inline NVGcolor mixColor(NVGcolor a, NVGcolor b, float t) {
	return nvgRGBAf(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 1.f);
}

}  // namespace hm
