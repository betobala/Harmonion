#pragma once
// Visual themes of the panel (see docs/art/STYLE.md): "Graphite" (dark, default) and "Paper" (light).
// Minimal palette: neutral greys plus one accent (amber) that marks everything that is on.
#include "plugin.hpp"

namespace hm {

struct Theme {
	NVGcolor panel, plate, outline, ink, grey, black, off, accent, accentDim;
};

static const int NUM_THEMES = 2;
static const char* const THEME_NAMES[NUM_THEMES] = {"Graphite", "Paper"};

inline const Theme& themeColors(int t) {
	static const Theme T[NUM_THEMES] = {
		// Graphite
		{nvgRGB(0x26, 0x27, 0x2A), nvgRGB(0x2E, 0x30, 0x33), nvgRGB(0x56, 0x59, 0x60), nvgRGB(0xE9, 0xE7, 0xE1),
		 nvgRGB(0x9A, 0x9C, 0xA2), nvgRGB(0x0D, 0x0D, 0x0F), nvgRGB(0x44, 0x46, 0x4B), nvgRGB(0xFF, 0xB3, 0x47),
		 nvgRGB(0x7A, 0x5A, 0x24)},
		// Paper
		{nvgRGB(0xDA, 0xD7, 0xCF), nvgRGB(0xE6, 0xE3, 0xDC), nvgRGB(0x8C, 0x8A, 0x84), nvgRGB(0x1C, 0x1D, 0x20),
		 nvgRGB(0x6A, 0x6B, 0x6F), nvgRGB(0x0D, 0x0D, 0x0F), nvgRGB(0xF6, 0xF4, 0xEF), nvgRGB(0xE8, 0x96, 0x0C),
		 nvgRGB(0xC9, 0xA2, 0x5E)},
	};
	return T[t < 0 ? 0 : (t >= NUM_THEMES ? NUM_THEMES - 1 : t)];
}

inline NVGcolor mixColor(NVGcolor a, NVGcolor b, float t) {
	return nvgRGBAf(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 1.f);
}

}  // namespace hm
