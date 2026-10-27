#pragma once
#include <string>
#include <vector>

namespace hm {

/** Content of the emulated 128x64 OLED as a list of primitives in display pixels.

Built by Harmonion::buildScreen() without NanoVG, so it can be tested and previewed headless;
the Display widget in Widget.cpp paints it. Layout and contents follow the photos of the
display in the Sinfonion manual.

Text is drawn in runs of segments. Besides the font's characters a segment may contain glyphs
that the painter draws as vectors: "♭" "♯" "△" "·" (small dot) and "→". */
struct Screen {
	enum Align { LEFT, CENTER, RIGHT };

	struct Seg {
		std::string text;
		float size;  // font size in display pixels
		float rise;  // raise above the baseline, for superscripts
	};
	struct Run {
		float x, y;  // y is the baseline; x is the left edge, center or right edge
		int align;
		std::vector<Seg> segs;
	};
	struct Shape {
		enum Kind { RECT, CIRCLE, LINE, DOTTED };
		int kind;
		float x, y, w, h;  // RECT: box. CIRCLE: center, radius in w. LINE/DOTTED: from x,y to w,h.
		bool fill;
	};
	std::vector<Run> runs;
	std::vector<Shape> shapes;

	// Font sizes. BIG is the single text size of the original; SMALL is used for superscripts.
	static constexpr float BIG = 14.f;
	static constexpr float SMALL = 8.5f;
	static constexpr float RISE = 4.6f;
	// The painter compresses text horizontally to imitate the tall OLED font, and further if a run
	// is wider than the display
	static constexpr float X_SCALE = 0.78f;
	static constexpr float MAX_WIDTH = 124.f;

	void clear() {
		runs.clear();
		shapes.clear();
	}
	Run& run(float x, float y, int align) {
		runs.push_back(Run{x, y, align, {}});
		return runs.back();
	}
	void text(float x, float y, const std::string& t, int align = LEFT, float size = BIG) {
		run(x, y, align).segs.push_back(Seg{t, size, 0.f});
	}
	void rect(float x, float y, float w, float h, bool fill) {
		shapes.push_back(Shape{Shape::RECT, x, y, w, h, fill});
	}
	void circle(float x, float y, float r, bool fill) {
		shapes.push_back(Shape{Shape::CIRCLE, x, y, r, 0.f, fill});
	}
	void line(float x1, float y1, float x2, float y2, bool dotted = false) {
		shapes.push_back(Shape{dotted ? Shape::DOTTED : Shape::LINE, x1, y1, x2, y2, false});
	}

	/** All text, one run per line, for tests */
	std::string allText() const;
};

/** Appends a note name ("Eb", "F#") to a run, with the accidental small and raised like the original */
void addNote(Screen::Run& r, const std::string& name, float size = Screen::BIG);
/** Converts the ASCII accidentals of a chord suffix into the display glyphs ("-7b5" -> "-7♭5") */
std::string suffixGlyphs(const std::string& suffix);

}  // namespace hm
