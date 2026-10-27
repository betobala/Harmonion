#include "Harmonion.hpp"
#include "Screen.hpp"
#include "Theme.hpp"
#include "Controls.hpp"
#include "Stencil.hpp"

namespace hm {

static std::shared_ptr<window::Font> pluginFont(const char* file) {
	std::shared_ptr<window::Font> f = APP->window->loadFont(asset::plugin(pluginInstance, std::string("res/fonts/") + file));
	if (!f)
		f = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
	return f;
}

// Panel geometry in millimeters
static const float JACK_X[4] = {8.4f, 21.0f, 33.7f, 47.2f};
static const float JACK_Y[8] = {15.9f, 29.9f, 43.4f, 57.0f, 70.5f, 84.0f, 97.8f, 111.3f};
static const float GRID_X[8] = {109.1f, 122.2f, 135.4f, 150.4f, 163.6f, 176.5f, 191.6f, 204.8f};
static const float GRID_Y[4] = {16.2f, 29.3f, 42.5f, 55.6f};
static const float LOW_Y[4] = {73.3f, 86.4f, 99.5f, 112.7f};
static const float SMALL_X[4] = {61.6f, 72.4f, 83.5f, 94.4f};
static const float SMALL_Y[3] = {95.6f, 105.1f, 114.7f};

// (the name stays; in the betobullet identity the lights are white)
struct AmberLight : GrayModuleLightWidget {
	AmberLight() {
		addBaseColor(nvgRGB(0xf4, 0xf4, 0xf2));
	}
};

static const Theme& themeOf(const engine::Module* m) {
	return themeColors(m ? static_cast<const Harmonion*>(m)->theme : 0);
}

using FlatBtn = ctl::Button;

static FlatBtn* makeFlat(Vec pos, Harmonion* module, int paramId, int lightId, float disc) {
	FlatBtn* b = createParamCentered<FlatBtn>(pos, module, paramId);
	b->lightId = lightId;
	b->discMm = disc;
	return b;
}

/** Panel background, section outlines and tabs. Drawn in code so the theme can change. */
struct PanelArt : TransparentWidget {
	Harmonion* module = nullptr;

	void line(NVGcontext* vg, float x1, float y1, float x2, float y2, bool dotted) {
		if (!dotted) {
			nvgBeginPath(vg);
			nvgMoveTo(vg, mm2px(x1), mm2px(y1));
			nvgLineTo(vg, mm2px(x2), mm2px(y2));
			nvgStroke(vg);
			return;
		}
		float len = std::hypot(x2 - x1, y2 - y1);
		for (float d = 0.f; d < len; d += 1.4f) {
			float a = d / len, b = std::min(d + 0.8f, len) / len;
			nvgBeginPath(vg);
			nvgMoveTo(vg, mm2px(x1 + (x2 - x1) * a), mm2px(y1 + (y2 - y1) * a));
			nvgLineTo(vg, mm2px(x1 + (x2 - x1) * b), mm2px(y1 + (y2 - y1) * b));
			nvgStroke(vg);
		}
	}

	void draw(const DrawArgs& args) override {
		const Theme& t = themeColors(module ? module->theme : 0);
		NVGcontext* vg = args.vg;
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, t.panel);
		nvgFill(vg);

		struct R {
			float x, y, w, h;
		};
		static const R sections[] = {
			{1.6f, 5.6f, 53.0f, 119.2f}, {56.4f, 5.6f, 44.6f, 119.2f}, {102.6f, 5.6f, 109.2f, 57.8f},
			{102.6f, 64.0f, 25.4f, 60.8f}, {129.4f, 64.0f, 41.6f, 60.8f}, {172.4f, 64.0f, 39.4f, 60.8f},
		};
		static const R tabs[] = {
			{13.5f, 3.4f, 29.0f, 4.4f}, {104.0f, 3.4f, 37.6f, 4.4f}, {145.0f, 3.4f, 37.0f, 4.4f}, {186.0f, 3.4f, 24.4f, 4.4f},
			{106.6f, 62.0f, 17.4f, 4.2f}, {140.2f, 62.0f, 20.0f, 4.2f}, {181.0f, 62.0f, 22.2f, 4.2f}, {106.6f, 102.0f, 17.4f, 4.0f},
		};
		bb::Ink k = {t.panel, t.ink};
		for (const R& r : sections)
			bb::notchedFrame(vg, Rect(Vec(r.x, r.y), Vec(r.w, r.h)), k, 2.4f, 1.6f);
		// hairline dividers
		nvgStrokeColor(vg, t.outline);
		nvgStrokeWidth(vg, mm2px(0.25f));
		line(vg, 1.6f, 23.0f, 54.6f, 23.0f, false);
		line(vg, 1.6f, 36.6f, 40.5f, 36.6f, true);
		line(vg, 1.6f, 50.2f, 54.6f, 50.2f, false);
		line(vg, 1.6f, 77.2f, 54.6f, 77.2f, false);
		line(vg, 1.6f, 104.6f, 54.6f, 104.6f, false);
		line(vg, 40.5f, 23.0f, 40.5f, 50.2f, false);
		line(vg, 40.5f, 77.2f, 40.5f, 124.8f, false);
		line(vg, 143.0f, 8.6f, 143.0f, 62.6f, false);
		line(vg, 184.1f, 8.6f, 184.1f, 62.6f, false);
		line(vg, 102.6f, 104.0f, 128.0f, 104.0f, false);
		line(vg, 128.8f, 9.0f, 128.8f, 51.2f, true);
		line(vg, 128.8f, 51.2f, 134.0f, 51.2f, true);

		// tabs sit on the top outline of their section
		for (const R& r : tabs) {
			bb::chamferPath(vg, Rect(Vec(r.x, r.y), Vec(r.w, r.h)), 1.0f);
			nvgFillColor(vg, t.panel);
			nvgFill(vg);
			nvgStrokeColor(vg, t.ink);
			nvgStrokeWidth(vg, mm2px(0.5f));
			nvgStroke(vg);
		}

		// display frame, with stencil brackets
		nvgBeginPath(vg);
		nvgRect(vg, mm2px(61.0f), mm2px(29.4f), mm2px(34.2f), mm2px(19.6f));
		nvgFillColor(vg, t.black);
		nvgFill(vg);
		bb::bracketFrame(vg, Rect(Vec(61.0f, 29.4f), Vec(34.2f, 19.6f)), k, 2.6f);

		// the name, in stencil type
		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (stencil) {
			nvgFontFaceId(vg, stencil->handle);
			nvgFontSize(vg, mm2px(4.2f));
			nvgTextLetterSpacing(vg, mm2px(0.3f));
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgFillColor(vg, t.ink);
			nvgText(vg, mm2px(78.7f), mm2px(2.9f), "HARMONION", NULL);
			nvgTextLetterSpacing(vg, 0.f);
		}
	}
};

/** Text labels, drawn at runtime since SVG text is not rendered by Rack */
struct PanelLabels : TransparentWidget {
	struct Label {
		Vec pos;
		std::string text;
		float size;
		bool header;
		bool vertical;
	};
	std::vector<Label> labels;
	Harmonion* module = nullptr;

	void add(float x, float y, std::string text, float size = 1.9f, bool header = false, bool vertical = false) {
		labels.push_back({mm2px(Vec(x, y)), text, size, header, vertical});
	}

	void draw(const DrawArgs& args) override {
		std::shared_ptr<window::Font> font = pluginFont("MartianMono-Label.ttf");
		std::shared_ptr<window::Font> stencil = pluginFont("BlackOpsOne-Regular.ttf");
		if (!font || !stencil)
			return;
		nvgFontFaceId(args.vg, font->handle);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		const Theme& t = themeColors(module ? module->theme : 0);
		for (const Label& l : labels) {
			nvgFontSize(args.vg, mm2px(l.size) * ((l.header || l.vertical) ? 1.55f : 1.9f));
			nvgFillColor(args.vg, (l.header || l.size >= 2.4f) ? t.ink : t.grey);
			nvgFontFaceId(args.vg, l.header ? stencil->handle : font->handle);
			if (l.vertical) {
				nvgSave(args.vg);
				nvgTranslate(args.vg, l.pos.x, l.pos.y);
				nvgRotate(args.vg, -M_PI / 2);
				nvgText(args.vg, 0, 0, l.text.c_str(), NULL);
				nvgRestore(args.vg);
			}
			else {
				nvgText(args.vg, l.pos.x, l.pos.y, l.text.c_str(), NULL);
			}
		}
	}
};

/** Emulated 128x64 OLED: paints the primitives of Harmonion::buildScreen() (see Screen.hpp) */
struct Display : TransparentWidget {
	Harmonion* module = nullptr;
	Screen screen;

	enum Glyph { G_NONE, G_FLAT, G_SHARP, G_TRIANGLE, G_DOT, G_ARROW };

	/** Splits UTF-8 text into plain text and the glyphs that are drawn as vectors */
	static int glyphAt(const std::string& t, size_t i, size_t* len) {
		static const struct {
			const char* utf8;
			int glyph;
		} map[] = {{"♭", G_FLAT}, {"♯", G_SHARP}, {"△", G_TRIANGLE}, {"·", G_DOT}, {"→", G_ARROW}};
		for (const auto& m : map) {
			size_t n = strlen(m.utf8);
			if (t.compare(i, n, m.utf8) == 0) {
				*len = n;
				return m.glyph;
			}
		}
		*len = 1;
		return G_NONE;
	}

	/** Advance of a glyph in em */
	static float glyphWidth(int g) {
		switch (g) {
			case G_FLAT: return 0.45f;
			case G_SHARP: return 0.55f;
			case G_TRIANGLE: return 0.72f;
			case G_DOT: return 0.32f;
			case G_ARROW: return 0.62f;
			default: return 0.f;
		}
	}

	/** Draws a glyph with its left edge at x and the baseline at y, in em units of size e */
	static void drawGlyph(NVGcontext* vg, int g, float x, float y, float e) {
		nvgStrokeWidth(vg, std::max(0.6f, 0.09f * e));
		nvgBeginPath(vg);
		switch (g) {
			case G_FLAT:
				nvgMoveTo(vg, x + 0.1f * e, y - 0.78f * e);
				nvgLineTo(vg, x + 0.1f * e, y);
				nvgBezierTo(vg, x + 0.45f * e, y - 0.12f * e, x + 0.42f * e, y - 0.42f * e, x + 0.1f * e, y - 0.3f * e);
				nvgStroke(vg);
				break;
			case G_SHARP:
				nvgMoveTo(vg, x + 0.18f * e, y - 0.75f * e);
				nvgLineTo(vg, x + 0.16f * e, y + 0.05f * e);
				nvgMoveTo(vg, x + 0.37f * e, y - 0.8f * e);
				nvgLineTo(vg, x + 0.35f * e, y);
				nvgMoveTo(vg, x + 0.04f * e, y - 0.45f * e);
				nvgLineTo(vg, x + 0.5f * e, y - 0.55f * e);
				nvgMoveTo(vg, x + 0.02f * e, y - 0.2f * e);
				nvgLineTo(vg, x + 0.48f * e, y - 0.3f * e);
				nvgStroke(vg);
				break;
			case G_TRIANGLE:
				nvgMoveTo(vg, x + 0.06f * e, y);
				nvgLineTo(vg, x + 0.64f * e, y);
				nvgLineTo(vg, x + 0.35f * e, y - 0.68f * e);
				nvgClosePath(vg);
				nvgStroke(vg);
				break;
			case G_DOT:
				nvgRect(vg, x + 0.09f * e, y - 0.42f * e, 0.14f * e, 0.14f * e);
				nvgFill(vg);
				break;
			case G_ARROW:
				nvgMoveTo(vg, x + 0.04f * e, y - 0.36f * e);
				nvgLineTo(vg, x + 0.54f * e, y - 0.36f * e);
				nvgMoveTo(vg, x + 0.34f * e, y - 0.54f * e);
				nvgLineTo(vg, x + 0.56f * e, y - 0.36f * e);
				nvgLineTo(vg, x + 0.34f * e, y - 0.18f * e);
				nvgStroke(vg);
				break;
		}
	}

	/** Walks the segments of a run: measures (draw = false) or draws them, returns the width */
	float layout(NVGcontext* vg, float s, const Screen::Run& r, bool draw) {
		float x = 0.f;
		for (const Screen::Seg& g : r.segs) {
			float e = g.size * s;
			nvgFontSize(vg, e);
			float y = -g.rise * s;
			size_t i = 0;
			std::string plain;
			auto flush = [&]() {
				if (plain.empty())
					return;
				if (draw)
					nvgText(vg, x, y, plain.c_str(), NULL);
				x += nvgTextBounds(vg, 0, 0, plain.c_str(), NULL, NULL);
				plain.clear();
			};
			while (i < g.text.size()) {
				size_t len;
				int gl = glyphAt(g.text, i, &len);
				if (gl == G_NONE) {
					plain += g.text.substr(i, len);
				}
				else {
					flush();
					if (draw)
						drawGlyph(vg, gl, x, y, e);
					x += glyphWidth(gl) * e;
				}
				i += len;
			}
			flush();
		}
		return x;
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1)
			return;
		std::shared_ptr<window::Font> font = pluginFont("MartianMono-Display.ttf");
		if (!font)
			return;
		NVGcontext* vg = args.vg;
		float s = box.size.x / 128.f;  // pixels per display pixel
		NVGcolor color = themeOf(module).accent;
		if (module) {
			module->buildScreen(screen);
		}
		else {
			// Module browser: a static example
			screen.clear();
			Screen::Run& l = screen.run(3.4f, 15.f, Screen::LEFT);
			l.segs.push_back(Screen::Seg{"C", Screen::BIG, 0.f});
			screen.text(24.5f, 15.f, "Chords");
			Screen::Run& c = screen.run(125.5f, 15.f, Screen::RIGHT);
			c.segs.push_back(Screen::Seg{"C", Screen::BIG, 0.f});
			c.segs.push_back(Screen::Seg{"lyd", Screen::SMALL, Screen::RISE});
			Screen::Run& n = screen.run(64.f, 38.f, Screen::CENTER);
			static const char* const notes[7] = {"C", "D", "E", "F#", "G", "A", "B"};
			for (int i = 0; i < 7; i++) {
				if (i)
					n.segs.push_back(Screen::Seg{" ", Screen::SMALL, 0.f});
				addNote(n, notes[i]);
			}
			screen.text(3.4f, 61.f, "+0");
			screen.text(125.5f, 61.f, "1", Screen::RIGHT);
		}
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, color);
		nvgStrokeColor(vg, color);
		nvgLineCap(vg, NVG_ROUND);

		for (const Screen::Shape& sh : screen.shapes) {
			nvgStrokeWidth(vg, 0.6f * s);
			nvgBeginPath(vg);
			switch (sh.kind) {
				case Screen::Shape::RECT:
					nvgRect(vg, sh.x * s, sh.y * s, sh.w * s, sh.h * s);
					break;
				case Screen::Shape::CIRCLE:
					nvgCircle(vg, sh.x * s, sh.y * s, sh.w * s);
					break;
				case Screen::Shape::LINE:
					nvgMoveTo(vg, sh.x * s, sh.y * s);
					nvgLineTo(vg, sh.w * s, sh.h * s);
					break;
				case Screen::Shape::DOTTED: {
					float len = std::hypot(sh.w - sh.x, sh.h - sh.y);
					for (float d = 0.f; d <= len + 1e-3f; d += 1.8f) {
						float a = (len > 0.f) ? d / len : 0.f;
						nvgRect(vg, (sh.x + (sh.w - sh.x) * a - 0.4f) * s, (sh.y + (sh.h - sh.y) * a - 0.4f) * s, 0.8f * s, 0.8f * s);
					}
					nvgFill(vg);
					continue;
				}
			}
			if (sh.fill)
				nvgFill(vg);
			else
				nvgStroke(vg);
		}

		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BASELINE);
		for (const Screen::Run& r : screen.runs) {
			// Text is compressed horizontally like the tall OLED font, and more if it would not fit
			float w = layout(vg, s, r, false) * Screen::X_SCALE;
			float kx = Screen::X_SCALE;
			if (w > Screen::MAX_WIDTH * s) {
				kx *= Screen::MAX_WIDTH * s / w;
				w = Screen::MAX_WIDTH * s;
			}
			float x = r.x * s;
			if (r.align == Screen::CENTER)
				x -= w / 2.f;
			else if (r.align == Screen::RIGHT)
				x -= w;
			nvgSave(vg);
			nvgTranslate(vg, x, r.y * s);
			nvgScale(vg, kx, 1.f);
			layout(vg, s, r, true);
			nvgRestore(vg);
		}
		nvgResetScissor(vg);
	}
};

struct HarmonionWidget : ModuleWidget {
	HarmonionWidget(Harmonion* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Harmonion.svg")));

		PanelArt* art = new PanelArt;
		art->module = module;
		art->box.size = box.size;
		addChild(art);

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		PanelLabels* labels = new PanelLabels;
		labels->module = module;
		labels->box.size = box.size;
		addChild(labels);

		// Patchbay
		using H = Harmonion;
		labels->add(28.0f, 5.6f, "PATCHBAY", 2.1f, true);
		struct Jack {
			int row, col;
			bool output;
			int id;
			const char* label;
			int light;
		};
		const Jack jacks[] = {
			{0, 0, false, H::CH_IN_INPUT + 0, "IN", -1}, {0, 1, true, H::CH_OUTPUT + 0, "OUT", -1},
			{0, 2, false, H::CH_TRIG_INPUT + 0, "TRIG", H::TRIG_LIGHT + 0}, {0, 3, false, H::AB_INPUT, "A/B", H::AB_LIGHT},
			{1, 0, false, H::CH_IN_INPUT + 1, "IN", -1}, {1, 1, true, H::CH_OUTPUT + 1, "OUT", -1},
			{1, 2, false, H::CH_TRIG_INPUT + 1, "TRIG", H::TRIG_LIGHT + 1}, {1, 3, false, H::ROOT_INPUT, "ROOT", -1},
			{2, 0, false, H::CH_IN_INPUT + 2, "IN", -1}, {2, 1, true, H::CH_OUTPUT + 2, "OUT", -1},
			{2, 2, false, H::CH_TRIG_INPUT + 2, "TRIG", H::TRIG_LIGHT + 2}, {2, 3, false, H::TRANS_INPUT, "TRANS", -1},
			{3, 0, false, H::CV_INPUT + 0, "CV 1", -1}, {3, 1, false, H::CV_INPUT + 2, "CV 3", -1},
			{3, 2, false, H::IN_INPUT + 0, "IN 1", H::IN_LIGHT + 0}, {3, 3, true, H::OUT_OUTPUT + 0, "OUT 1", H::OUT_LIGHT + 0},
			{4, 0, false, H::CV_INPUT + 1, "CV 2", -1}, {4, 1, false, H::CV_INPUT + 3, "CV 4", -1},
			{4, 2, false, H::IN_INPUT + 1, "IN 2", H::IN_LIGHT + 1}, {4, 3, true, H::OUT_OUTPUT + 1, "OUT 2", H::OUT_LIGHT + 1},
			{5, 0, false, H::CHORD_IN_INPUT, "IN", -1}, {5, 1, true, H::CHORD_OUTPUT + 0, "OUT 1", -1},
			{5, 2, false, H::CHORD_TRIG_INPUT, "TRIG", H::TRIG_LIGHT + 3}, {5, 3, true, H::GATE_OUTPUT + 0, "GATE 1", H::GATE_LIGHT + 0},
			{6, 0, true, H::CHORD_OUTPUT + 1, "OUT 2", -1}, {6, 1, true, H::CHORD_OUTPUT + 2, "OUT 3", -1},
			{6, 2, true, H::CHORD_OUTPUT + 3, "OUT 4", -1}, {6, 3, true, H::GATE_OUTPUT + 1, "GATE 2", H::GATE_LIGHT + 1},
			{7, 0, false, H::ARP_IN_INPUT, "IN", -1}, {7, 1, true, H::ARP_OUTPUT, "OUT", -1},
			{7, 2, false, H::ARP_CLK_INPUT, "CLK", H::ARP_CLK_LIGHT}, {7, 3, false, H::SEQ_CLK_INPUT, "CLK", H::SEQ_CLK_LIGHT},
		};
		for (const Jack& j : jacks) {
			Vec pos = mm2px(Vec(JACK_X[j.col], JACK_Y[j.row]));
			if (j.output)
				addOutput(createOutputCentered<ctl::Jack>(pos, module, j.id));
			else
				addInput(createInputCentered<ctl::Jack>(pos, module, j.id));
			labels->add(JACK_X[j.col], JACK_Y[j.row] + 5.9f, j.label, 1.7f);
			if (j.light >= 0)
				addChild(createLightCentered<SmallLight<AmberLight>>(mm2px(Vec(JACK_X[j.col] + 5.2f, JACK_Y[j.row] + 4.2f)), module, j.light));
		}
		labels->add(3.2f, 15.6f, "CH 1", 1.6f, false, true);
		labels->add(3.2f, 29.6f, "CH 2", 1.6f, false, true);
		labels->add(3.2f, 43.2f, "CH 3", 1.6f, false, true);
		labels->add(3.2f, 63.7f, "ASSIGN", 1.6f, false, true);
		labels->add(3.2f, 90.9f, "CHORDS", 1.6f, false, true);
		labels->add(3.2f, 114.6f, "ARP", 1.6f, false, true);
		labels->add(42.3f, 104.4f, "", 1.6f);
		labels->add(52.6f, 97.8f, "GATES", 1.4f, false, true);
		labels->add(52.6f, 111.3f, "SEQ", 1.4f, false, true);

		// Center column (the name is drawn by PanelArt)
		addParam(ctl::knob(mm2px(Vec(66.8f, 16.6f)), module, H::ROOT_PARAM, 6.2f));
		addParam(ctl::knob(mm2px(Vec(89.0f, 16.6f)), module, H::DEGREE_PARAM, 6.2f));
		labels->add(66.8f, 25.0f, "ROOT");
		labels->add(89.0f, 25.0f, "DEGREE");
		Display* display = new Display;
		display->module = module;
		display->box.pos = mm2px(Vec(61.8f, 30.4f));
		display->box.size = mm2px(Vec(32.6f, 16.3f));
		addChild(display);
		const struct {
			float x, y;
			int id;
			const char* label;
		} knobs[] = {
			{66.8f, 62.4f, H::TRANS_PARAM, "TRANS"}, {89.0f, 62.4f, H::MODE_PARAM, "MODE"},
			{66.8f, 79.4f, H::SLEW_PARAM, "SLEW"}, {89.0f, 79.4f, H::PRESET_PARAM, "PRESET"},
		};
		for (const auto& k : knobs) {
			addParam(ctl::knob(mm2px(Vec(k.x, k.y)), module, k.id, 4.6f));
			labels->add(k.x, k.y + 8.0f, k.label);
		}
		static const char* const smallLabels[12] = {"MENU", "TUNE", "SONG", "OCT", "SEQ", "<", ">", ">/||", "DEL", "-", "+", "ADD"};
		for (int i = 0; i < 12; i++) {
			int r = i / 4, col = i % 4;
			addParam(makeFlat(mm2px(Vec(SMALL_X[col], SMALL_Y[r])), module, H::MENU_PARAM + i, H::SMALL_LIGHT + i, 4.6f));
			labels->add(SMALL_X[col], SMALL_Y[r] + 4.7f, smallLabels[i], 1.7f);
		}
		labels->add(78.0f, SMALL_Y[2] + 4.7f, "BEATS", 1.4f);

		// Channel field
		labels->add(122.8f, 5.6f, "A   CHANNEL 1   B", 2.0f, true);
		labels->add(163.5f, 5.6f, "CHANNEL 2", 2.0f, true);
		labels->add(198.2f, 5.6f, "CHANNEL 3", 2.0f, true);
		static const char* const gridLabels[4][8] = {
			{"ROOT", "3RD", "ROOT", "ROOT", "3RD", "FILL 2", "ROOT", "3RD"},
			{"5TH", "7TH", "5TH", "5TH", "7TH", "FILL 3", "5TH", "7TH"},
			{"9TH", "11TH", "SCALE", "9TH", "11TH", "FILL 4", "9TH", "11TH"},
			{"13TH", "FILL", "SLEW", "13TH", "FILL 1", "FILL 5", "13TH", "EQ-DIST"},
		};
		for (int r = 0; r < 4; r++) {
			for (int col = 0; col < 8; col++) {
				int b = top(r, col);
				addParam(makeFlat(mm2px(Vec(GRID_X[col], GRID_Y[r])), module, H::BIG_PARAM + b, H::BIG_LIGHT + b, 6.2f));
				labels->add(GRID_X[col], GRID_Y[r] + 6.3f, gridLabels[r][col], 1.6f);
			}
		}

		// Assign + gates
		labels->add(115.3f, 64.1f, "ASSIGN", 1.9f, true);
		addParam(ctl::knob(mm2px(Vec(115.3f, 75.4f)), module, H::POT1_PARAM, 4.6f));
		addParam(ctl::knob(mm2px(Vec(115.3f, 92.4f)), module, H::POT2_PARAM, 4.6f));
		labels->add(115.3f, 83.4f, "POT 1");
		labels->add(115.3f, 100.4f, "POT 2");
		labels->add(115.3f, 104.0f, "GATES", 1.9f, true);
		for (int g = 0; g < 2; g++) {
			int b = GATE_BTN + g;
			addParam(makeFlat(mm2px(Vec(GRID_X[g], LOW_Y[3])), module, H::BIG_PARAM + b, H::BIG_LIGHT + b, 6.2f));
			labels->add(GRID_X[g], LOW_Y[3] + 6.3f, g ? "2" : "1", 1.7f);
		}

		// Chords and arpeggio
		labels->add(150.2f, 64.1f, "CHORDS", 1.9f, true);
		labels->add(192.1f, 64.1f, "ARPEGGIO", 1.9f, true);
		static const char* const toneLabels[7] = {"ROOT", "3RD", "5TH", "7TH", "9TH", "11TH", "13TH"};
		const float chordX[2] = {137.3f, 150.4f};
		const float arpX[2] = {178.5f, 191.6f};
		for (int t = 0; t < NUM_TONES; t++) {
			int r = t / 2, col = t % 2;
			addParam(makeFlat(mm2px(Vec(chordX[col], LOW_Y[r])), module, H::BIG_PARAM + CHORD_BTN + t, H::BIG_LIGHT + CHORD_BTN + t, 6.2f));
			labels->add(chordX[col], LOW_Y[r] + 6.3f, toneLabels[t], 1.6f);
			addParam(makeFlat(mm2px(Vec(arpX[col], LOW_Y[r])), module, H::BIG_PARAM + ARP_BTN + t, H::BIG_LIGHT + ARP_BTN + t, 6.2f));
			labels->add(arpX[col], LOW_Y[r] + 6.3f, toneLabels[t], 1.6f);
		}
		const struct {
			float x;
			int b;
			const char* label;
		} extra[] = {
			{chordX[1], CHORD_BTN + CB_INV, "INV"}, {163.6f, CHORD_BTN + CB_SLEW, "SLEW"},
			{arpX[1], ARP_BTN + AB_PATTERN, "PATTERN"}, {204.8f, ARP_BTN + AB_BEGIN, "BEGIN"},
		};
		for (const auto& e : extra) {
			addParam(makeFlat(mm2px(Vec(e.x, LOW_Y[3])), module, H::BIG_PARAM + e.b, H::BIG_LIGHT + e.b, 6.2f));
			labels->add(e.x, LOW_Y[3] + 6.3f, e.label, 1.6f);
		}
		addParam(ctl::knob(mm2px(Vec(163.6f, 74.6f)), module, H::CHORD_PITCH_PARAM, 4.6f));
		addParam(ctl::knob(mm2px(Vec(163.6f, 91.8f)), module, H::SPREAD_PARAM, 4.6f));
		addParam(ctl::knob(mm2px(Vec(204.8f, 74.6f)), module, H::ARP_PITCH_PARAM, 4.6f));
		addParam(ctl::knob(mm2px(Vec(204.8f, 91.8f)), module, H::RANGE_PARAM, 4.6f));
		labels->add(163.6f, 82.6f, "PITCH");
		labels->add(163.6f, 99.8f, "SPREAD");
		labels->add(204.8f, 82.6f, "PITCH");
		labels->add(204.8f, 99.8f, "RANGE");
	}

	void appendContextMenu(Menu* menu) override {
		Harmonion* m = getModule<Harmonion>();
		if (!m)
			return;
		Settings& s = m->settings;
		menu->addChild(new MenuSeparator);
		menu->addChild(createIndexPtrSubmenuItem("Theme", std::vector<std::string>(THEME_NAMES, THEME_NAMES + NUM_THEMES), &m->theme));
		menu->addChild(new MenuSeparator);
		menu->addChild(createBoolPtrMenuItem("Gates at 5 V like the hardware (default 10 V)", "", &m->gate5V));
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Settings (same as the MENU button)"));
		auto names = [](const char* const* arr, int n) {
			return std::vector<std::string>(arr, arr + n);
		};
		// Lists in the order of the display menu; the settings hold stable ids
		auto ordered = [=](const char* label, const char* const* arr, const int* order, int n, int* value, int skip) {
			std::vector<std::string> labels;
			std::vector<int> ids;
			for (int i = 0; i < n; i++) {
				if (order[i] == skip)
					continue;
				labels.push_back(order[i] == 0 ? "None" : arr[order[i]]);
				ids.push_back(order[i]);
			}
			return createIndexSubmenuItem(label, labels,
				[=]() {
					for (size_t i = 0; i < ids.size(); i++)
						if (ids[i] == *value)
							return i;
					return (size_t) 0;
				},
				[=](size_t i) { *value = ids[i]; });
		};
		for (int i = 0; i < 4; i++)
			menu->addChild(ordered(string::f("CV %d", i + 1).c_str(), CV_FUNC_NAMES, CV_MENU_ORDER, NUM_CV_FUNCS, &s.cvFunc[i], -1));
		for (int i = 0; i < 2; i++)
			menu->addChild(ordered(string::f("Pot %d", i + 1).c_str(), CV_FUNC_NAMES, CV_MENU_ORDER, NUM_CV_FUNCS, &s.potFunc[i], CV_VOLTMETER));
		for (int i = 0; i < 2; i++)
			menu->addChild(ordered(string::f("In %d", i + 1).c_str(), IN_FUNC_NAMES, IN_MENU_ORDER, NUM_IN_FUNCS, &s.inFunc[i], i ? IN_SYNC_SLAVE : -1));
		for (int i = 0; i < 2; i++)
			menu->addChild(ordered(string::f("Out %d", i + 1).c_str(), OUT_FUNC_NAMES, OUT_MENU_ORDER, NUM_OUT_FUNCS, &s.outFunc[i], i ? OUT_SYNC_MASTER : -1));
		std::vector<std::string> divs, ratios, trigMs, arpMs;
		for (int d : CLOCK_DIVS)
			divs.push_back(string::f("%d", d));
		for (auto& r : ARP_RATIO)
			ratios.push_back(string::f("%d/%d", r[0], r[1]));
		for (int i = 0; i <= MAX_TRIG_WINDOW_MS; i++)
			trigMs.push_back(string::f("%d ms", i));
		for (int i = 0; i <= MAX_ARP_DELAY_MS; i++)
			arpMs.push_back(string::f("%d ms", i));
		menu->addChild(createIndexPtrSubmenuItem("Clock Divider (non-sequenced)", divs, &s.nonSeqClockDiv));
		menu->addChild(createIndexPtrSubmenuItem("Arpeggio Clock", ratios, &s.arpClock));
		menu->addChild(createIndexPtrSubmenuItem("Intonation", names(INTONATION_MODE_NAMES, NUM_INTONATIONS), &s.intonation));
		menu->addChild(createBoolPtrMenuItem("Exponential slew curve", "", &s.slewExp));
		menu->addChild(createIndexPtrSubmenuItem("Trigger Time Window", trigMs, &s.trigWindowMs));
		menu->addChild(createIndexPtrSubmenuItem("Arp. Trigger Delay", arpMs, &s.arpDelayMs));
		menu->addChild(createIndexPtrSubmenuItem("Pitch Bend Delay", names(BEND_DELAY_MODE_NAMES, 4), &s.bendDelay));
		menu->addChild(ordered("Channel 1 A/B", AB_MODE_NAMES, AB_MENU_ORDER, NUM_AB_MODES, &s.abMode, -1));
		menu->addChild(createBoolPtrMenuItem("Channel 2 single note mode", "", &s.ch2Single));
		menu->addChild(createIndexPtrSubmenuItem("Display", names(DISPLAY_MODE_NAMES, NUM_DISP_MODES), &s.display));
		menu->addChild(createBoolPtrMenuItem("Live mode (buttons + octaves not sequenced)", "", &s.liveMode));
		// Hold-button shortcuts of the hardware, which need two fingers
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Current preset / step"));
		std::vector<std::string> patterns;
		for (int i = 0; i < NUM_PATTERNS; i++)
			patterns.push_back(PATTERN_NAMES[PATTERN_ORDER[i]]);
		menu->addChild(createIndexSubmenuItem("Arpeggio pattern", patterns,
			[=]() { return (size_t) orderIndex(PATTERN_ORDER, NUM_PATTERNS, m->editPreset().pattern); },
			[=](size_t i) { m->editPreset().pattern = PATTERN_ORDER[i]; }));
		menu->addChild(createIndexSubmenuItem("Arpeggio begin", names(BEGIN_NAMES, 8),
			[=]() { return (size_t) (m->editPreset().begin + 1); },
			[=](size_t i) { m->editPreset().begin = (int) i - 1; }));
		menu->addChild(createIndexSubmenuItem("Chord inversion", names(INVERSION_NAMES, 5),
			[=]() { return (size_t) m->editPreset().inversion; },
			[=](size_t i) { m->editPreset().inversion = i; }));
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuItem("Reset To Defaults (menu + matrices)", "", [=]() { m->resetSettings(); }));
		menu->addChild(createMenuItem("Calibration mode", "TUNE + ← + Ch 1 3RD", [=]() { m->enterCalibration(); }));
		menu->addChild(createSubmenuItem("Factory reset", "Ch 3 3RD 7TH 11TH EQ-DIST + BEGIN", [=](Menu* sub) {
			sub->addChild(createMenuItem("Erase songs, presets and settings", "", [=]() { m->factoryReset(); }));
		}));
	}
};

}  // namespace hm

Model* modelHarmonion = createModel<hm::Harmonion, hm::HarmonionWidget>("Harmonion");

namespace ctl {
const Colors& colors(const engine::Module* m) {
	static const Colors black = bb::controlColors(0), white = bb::controlColors(1);
	int t = m ? static_cast<const hm::Harmonion*>(m)->theme : 0;
	return t == 1 ? white : black;
}
}  // namespace ctl
