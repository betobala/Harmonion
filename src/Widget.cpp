#include "Harmonion.hpp"
#include "Theme.hpp"
#include "Controls.hpp"

namespace hm {

static std::shared_ptr<window::Font> pluginFont(const char* file) {
	std::shared_ptr<window::Font> f = APP->window->loadFont(asset::plugin(pluginInstance, std::string("res/fonts/") + file));
	if (!f)
		f = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
	return f;
}

/** Font size (display pixels) that makes n monospaced characters fit the 128 px wide screen */
static float fitSize(size_t n, float maxSize) {
	return std::max(7.f, std::min(maxSize, 118.f / (0.62f * (float) std::max<size_t>(n, 1))));
}

// Panel geometry in millimeters
static const float JACK_X[4] = {8.4f, 21.0f, 33.7f, 47.2f};
static const float JACK_Y[8] = {15.9f, 29.9f, 43.4f, 57.0f, 70.5f, 84.0f, 97.8f, 111.3f};
static const float GRID_X[8] = {109.1f, 122.2f, 135.4f, 150.4f, 163.6f, 176.5f, 191.6f, 204.8f};
static const float GRID_Y[4] = {16.2f, 29.3f, 42.5f, 55.6f};
static const float LOW_Y[4] = {73.3f, 86.4f, 99.5f, 112.7f};
static const float SMALL_X[4] = {61.6f, 72.4f, 83.5f, 94.4f};
static const float SMALL_Y[3] = {95.6f, 105.1f, 114.7f};

struct AmberLight : GrayModuleLightWidget {
	AmberLight() {
		addBaseColor(nvgRGB(0xff, 0xb3, 0x47));
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

/** The logotype, one SVG per theme (graphite / paper). */
struct LogoWidget : TransparentWidget {
	Harmonion* module = nullptr;
	std::shared_ptr<window::Svg> svg[2];

	void load(float widthMm) {
		svg[0] = window::Svg::load(asset::plugin(pluginInstance, "res/logo_graphite.svg"));
		svg[1] = window::Svg::load(asset::plugin(pluginInstance, "res/logo_paper.svg"));
		float w = svg[0]->handle->width, h = svg[0]->handle->height;
		box.size = Vec(mm2px(widthMm), mm2px(widthMm) * h / w);
	}

	void draw(const DrawArgs& args) override {
		int i = (module && module->theme == 1) ? 1 : 0;
		if (!svg[i])
			return;
		float sc = box.size.x / svg[i]->handle->width;
		nvgSave(args.vg);
		nvgScale(args.vg, sc, sc);
		window::svgDraw(args.vg, svg[i]->handle);
		nvgRestore(args.vg);
	}
};

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
		nvgStrokeWidth(vg, mm2px(0.3f));
		for (const R& r : sections) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, mm2px(r.x), mm2px(r.y), mm2px(r.w), mm2px(r.h), mm2px(1.6f));
			nvgFillColor(vg, t.plate);
			nvgFill(vg);
			nvgStrokeColor(vg, t.outline);
			nvgStroke(vg);
		}
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
		nvgStrokeWidth(vg, mm2px(0.3f));
		for (const R& r : tabs) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, mm2px(r.x), mm2px(r.y), mm2px(r.w), mm2px(r.h), mm2px(1.0f));
			nvgFillColor(vg, t.plate);
			nvgFill(vg);
			nvgStrokeColor(vg, t.outline);
			nvgStroke(vg);
		}

		// display frame
		nvgBeginPath(vg);
		nvgRoundedRect(vg, mm2px(61.0f), mm2px(29.4f), mm2px(34.2f), mm2px(19.6f), mm2px(1.2f));
		nvgFillColor(vg, t.black);
		nvgFill(vg);
		nvgStrokeColor(vg, t.outline);
		nvgStroke(vg);
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
		if (!font)
			return;
		nvgFontFaceId(args.vg, font->handle);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		const Theme& t = themeColors(module ? module->theme : 0);
		for (const Label& l : labels) {
			nvgFontSize(args.vg, mm2px(l.size) * ((l.header || l.vertical) ? 1.55f : 1.9f));
			nvgFillColor(args.vg, (l.header || l.size >= 2.4f) ? t.ink : t.grey);
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

/** Emulated 128x64 OLED */
struct Display : TransparentWidget {
	Harmonion* module = nullptr;

	struct Ctx {
		NVGcontext* vg;
		float s;  // Pixels per display pixel
		int font;
	};

	void text(const Ctx& c, float x, float y, float size, const std::string& t, int align) {
		nvgFontSize(c.vg, size * c.s);
		nvgTextAlign(c.vg, align | NVG_ALIGN_BASELINE);
		nvgText(c.vg, x * c.s, y * c.s, t.c_str(), NULL);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1)
			return;
		std::shared_ptr<window::Font> font = pluginFont("MartianMono-Display.ttf");
		if (!font)
			return;
		NVGcontext* vg = args.vg;
		Ctx c = {vg, box.size.x / 128.f, font->handle};
		NVGcolor displayColor = themeOf(module).accent;
		nvgFontFaceId(vg, c.font);
		nvgFillColor(vg, displayColor);
		nvgStrokeColor(vg, displayColor);
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);
		if (!module) {
			text(c, 2, 11, 11, "C Chords", NVG_ALIGN_LEFT);
			text(c, 126, 11, 11, "Clyd", NVG_ALIGN_RIGHT);
			text(c, 64, 38, 13, "C D E F# G A B", NVG_ALIGN_CENTER);
			text(c, 2, 61, 10, "+0", NVG_ALIGN_LEFT);
			text(c, 126, 61, 10, "1", NVG_ALIGN_RIGHT);
			nvgResetScissor(vg);
			return;
		}
		Harmonion* m = module;
		if (m->calibrating) {
			text(c, 2, 16, 13, "Calibrate", NVG_ALIGN_LEFT);
			text(c, 126, 16, 13, m->calib5V ? "5V" : "0V", NVG_ALIGN_RIGHT);
			static const char* const outNames[8] = {"Channel 1", "Channel 2", "Channel 3", "Arpeggio", "Chords 1", "Chords 2", "Chords 3", "Chords 4"};
			int dac = (int) std::round(m->calib[m->calibLast][m->calib5V ? 1 : 0] / DAC_UNIT_V);
			text(c, 64, 40, 12, string::f("%+d %s", dac, outNames[m->calibLast]), NVG_ALIGN_CENTER);
		}
		else if (m->menuOpen) {
			// Item position as a row of dots
			float x0 = 64 - NUM_MENU_ITEMS * 2.5f;
			for (int i = 0; i < NUM_MENU_ITEMS; i++) {
				nvgBeginPath(vg);
				if (i == m->menuItem)
					nvgRect(vg, (x0 + i * 5) * c.s, 2 * c.s, 2.5f * c.s, 5 * c.s);
				else
					nvgRect(vg, (x0 + i * 5) * c.s, 4 * c.s, 2.5f * c.s, 2.5f * c.s);
				nvgFill(vg);
			}
			text(c, 64, 28, 13, m->menuTitle(m->menuItem), NVG_ALIGN_CENTER);
			std::string v = m->menuValue(m->menuItem);
			text(c, 64, 46, fitSize(v.size(), 13), v, NVG_ALIGN_CENTER);
		}
		else if (m->tuning || m->inFuncHigh[IN_TUNING]) {
			text(c, 64, 16, 13, "Tuning Mode", NVG_ALIGN_CENTER);
			float hz = 261.626f * std::pow(2.f, m->tuneOctave - 4);
			text(c, 64, 38, 13, string::f("C%d  %.1fV", m->tuneOctave, (float) m->tuneOctave), NVG_ALIGN_CENTER);
			text(c, 64, 56, 11, string::f("%.1f Hz", hz), NVG_ALIGN_CENTER);
		}
		else if (m->popup != Harmonion::POPUP_NONE && m->time < m->popupUntil) {
			const Preset& p = m->editPreset();
			static const char* const titles[4] = {"", "Arpeggio Pattern", "Arpeggio Begin", "Chord Inversion"};
			std::string v;
			if (m->popup == Harmonion::POPUP_PATTERN)
				v = PATTERN_NAMES[p.pattern % 8];
			else if (m->popup == Harmonion::POPUP_BEGIN)
				v = BEGIN_NAMES[clamp(p.begin + 1, 0, 7)];
			else
				v = INVERSION_NAMES[p.inversion % 5];
			text(c, 64, 24, 12, titles[m->popup], NVG_ALIGN_CENTER);
			text(c, 64, 46, 14, v, NVG_ALIGN_CENTER);
		}
		else if (m->voltmeterActive) {
			text(c, 64, 16, 11, "Voltmeter", NVG_ALIGN_CENTER);
			text(c, 64, 46, 22, string::f("%+.3f V", m->voltmeter), NVG_ALIGN_CENTER);
		}
		else if (m->page == Harmonion::PAGE_MATRIX) {
			static const char* const rows[4] = {"CH1", "CH2", "CH3", "ARP"};
			static const char outChar[4] = {'1', '2', '3', 'A'};
			for (int r = 0; r < 4; r++) {
				std::string in, out;
				for (int j = 0; j < 4; j++) {
					in += m->settings.inMix[r][j] ? "#" : "|";
					out += (m->settings.outSrc[j] == r) ? outChar[j] : '_';
				}
				text(c, 64, 14 + r * 15, 12, in + " >" + rows[r] + "> " + out, NVG_ALIGN_CENTER);
			}
		}
		else {
			const Harmony& h = m->frozen ? m->editHarmony : m->harmony;
			if (!h.scale) {
				// the engine has not computed a harmony yet (first frames after adding the module)
				nvgResetScissor(vg);
				return;
			}
			bool flat = preferFlats(h.chordRoot, *h.scale);
			text(c, 2, 11, 9.5f, noteName(h.keyRoot, flat) + " " + h.modeName(), NVG_ALIGN_LEFT);
			text(c, 126, 11, 9.5f, h.chordName(), NVG_ALIGN_RIGHT);
			std::string center;
			int disp = (m->page == Harmonion::PAGE_OCT) ? DISP_OCTAVES : m->settings.display;
			const Preset& p = m->editPreset();
			switch (disp) {
				case DISP_TONES: center = h.scaleTones(); break;
				case DISP_STRUCTURE: center = h.scaleStructure(); break;
				case DISP_CHORD: {
					static const char* const toneNum[7] = {"1", "3", "5", "7", "9", "11", "13"};
					int idx[4] = {0, 1, 2, 3};
					std::sort(idx, idx + 4, [&](int a, int b) { return m->chordNotes[a] < m->chordNotes[b]; });
					for (int k = 0; k < 4; k++)
						center += std::string(k ? " " : "") + toneNum[clamp(m->chordFuncs[idx[k]], 0, 6)];
					center += string::f("  %+d", p.octave[SEC_CHORDS]);
				} break;
				case DISP_OCTAVES:
					for (int s = 0; s < NUM_SECTIONS; s++)
						center += string::f("%s%+d", s ? " " : "", p.octave[s]);
					break;
			}
			text(c, 64, 37, fitSize(center.size(), 13), center, NVG_ALIGN_CENTER);
			text(c, 2, 61, 10, string::f("%+d", m->transSemis), NVG_ALIGN_LEFT);
			if (m->syncSlave) {
				text(c, 64, 61, 10, m->syncAlive ? "Sync: slave" : "Sync: dead", NVG_ALIGN_CENTER);
				text(c, 126, 61, 10, string::f("%d", m->presetIndex + 1), NVG_ALIGN_RIGHT);
			}
			else if (m->seqMode) {
				text(c, 126, 61, 10, string::f("%d%c", m->song + 1, 'A' + m->part), NVG_ALIGN_RIGHT);
				// Beats as circles, steps as boxes
				int beats = m->currentStep().beats;
				int steps = m->currentPart().steps.size();
				float x = 20;
				if (beats <= 8) {
					for (int b = 0; b < beats; b++) {
						nvgBeginPath(vg);
						nvgCircle(vg, (x + 2) * c.s, 57.5f * c.s, 1.7f * c.s);
						if (b == m->beat)
							nvgFill(vg);
						else {
							nvgStrokeWidth(vg, 0.5f * c.s);
							nvgStroke(vg);
						}
						x += 4.5f;
					}
				}
				else {
					text(c, x, 61, 9, string::f("%d/%d", m->beat + 1, beats), NVG_ALIGN_LEFT);
					x += 22;
				}
				x += 3;
				if (steps <= 16) {
					for (int s = 0; s < steps; s++) {
						nvgBeginPath(vg);
						nvgRect(vg, x * c.s, 55.5f * c.s, 3 * c.s, 3.5f * c.s);
						if (s == m->step)
							nvgFill(vg);
						else {
							nvgStrokeWidth(vg, 0.5f * c.s);
							nvgStroke(vg);
						}
						x += 4.2f;
					}
				}
				else {
					text(c, x, 61, 9, string::f("%d/%d", m->step + 1, steps), NVG_ALIGN_LEFT);
				}
				if (m->frozen)
					text(c, 64, 22, 9, "FREEZE", NVG_ALIGN_CENTER);
			}
			else {
				text(c, 126, 61, 10, string::f("%d", m->presetIndex + 1), NVG_ALIGN_RIGHT);
			}
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

		// Center column
		LogoWidget* logo = new LogoWidget;
		logo->module = module;
		logo->load(30.0f);
		logo->box.pos = Vec(mm2px(78.7f) - logo->box.size.x / 2, mm2px(3.2f) - logo->box.size.y / 2);
		addChild(logo);
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
		menu->addChild(createMenuLabel("Settings (same as the MENU button)"));
		auto names = [](const char* const* arr, int n) {
			return std::vector<std::string>(arr, arr + n);
		};
		menu->addChild(createIndexPtrSubmenuItem("Display", names(DISPLAY_MODE_NAMES, NUM_DISP_MODES), &s.display));
		for (int i = 0; i < 4; i++)
			menu->addChild(createIndexPtrSubmenuItem(string::f("CV %d", i + 1), names(CV_FUNC_NAMES, NUM_CV_FUNCS), &s.cvFunc[i]));
		for (int i = 0; i < 2; i++)
			menu->addChild(createIndexPtrSubmenuItem(string::f("Pot %d", i + 1), names(CV_FUNC_NAMES, NUM_CV_FUNCS), &s.potFunc[i]));
		for (int i = 0; i < 2; i++)
			menu->addChild(createIndexPtrSubmenuItem(string::f("In %d", i + 1), names(IN_FUNC_NAMES, NUM_IN_FUNCS), &s.inFunc[i]));
		for (int i = 0; i < 2; i++)
			menu->addChild(createIndexPtrSubmenuItem(string::f("Out %d", i + 1), names(OUT_FUNC_NAMES, NUM_OUT_FUNCS), &s.outFunc[i]));
		std::vector<std::string> divs, ratios, ms;
		for (int d : CLOCK_DIVS)
			divs.push_back(string::f("%d", d));
		for (auto& r : ARP_RATIO)
			ratios.push_back(string::f("%d/%d", r[0], r[1]));
		for (int i = 0; i <= 20; i++)
			ms.push_back(string::f("%d ms", i));
		menu->addChild(createIndexPtrSubmenuItem("Clock Divider (non-sequenced)", divs, &s.nonSeqClockDiv));
		menu->addChild(createIndexPtrSubmenuItem("Arpeggio Clock", ratios, &s.arpClock));
		menu->addChild(createIndexPtrSubmenuItem("Arpeggio Trigger Delay", ms, &s.arpDelayMs));
		menu->addChild(createIndexPtrSubmenuItem("Channel 1 A/B", names(AB_MODE_NAMES, NUM_AB_MODES), &s.abMode));
		menu->addChild(createBoolPtrMenuItem("Channel 2 single note mode", "", &s.ch2Single));
		menu->addChild(createIndexPtrSubmenuItem("Trigger Time Window", ms, &s.trigWindowMs));
		menu->addChild(createBoolPtrMenuItem("Exponential slew curve", "", &s.slewExp));
		menu->addChild(createIndexPtrSubmenuItem("Intonation", names(INTONATION_MODE_NAMES, NUM_INTONATIONS), &s.intonation));
		menu->addChild(createIndexPtrSubmenuItem("Pitch Bend Delay", names(BEND_DELAY_MODE_NAMES, 4), &s.bendDelay));
		menu->addChild(createBoolPtrMenuItem("Live mode (buttons not sequenced)", "", &s.liveMode));
		// Hold-button shortcuts of the hardware, which need two fingers
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Current preset / step"));
		menu->addChild(createIndexSubmenuItem("Arpeggio pattern", names(PATTERN_NAMES, 8),
			[=]() { return (size_t) m->editPreset().pattern; },
			[=](size_t i) { m->editPreset().pattern = i; }));
		menu->addChild(createIndexSubmenuItem("Arpeggio begin", names(BEGIN_NAMES, 8),
			[=]() { return (size_t) (m->editPreset().begin + 1); },
			[=](size_t i) { m->editPreset().begin = (int) i - 1; }));
		menu->addChild(createIndexSubmenuItem("Chord inversion", names(INVERSION_NAMES, 5),
			[=]() { return (size_t) m->editPreset().inversion; },
			[=](size_t i) { m->editPreset().inversion = i; }));
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuItem("Reset To Defaults (menu + matrices)", "", [=]() { m->resetSettings(); }));
		menu->addChild(createMenuItem("Calibration mode", "", [=]() {
			m->calibrating = true;
			m->menuOpen = false;
		}));
		menu->addChild(createSubmenuItem("Factory reset", "", [=](Menu* sub) {
			sub->addChild(createMenuItem("Erase songs, presets and settings", "", [=]() { m->factoryReset(); }));
		}));
	}
};

}  // namespace hm

Model* modelHarmonion = createModel<hm::Harmonion, hm::HarmonionWidget>("Harmonion");

namespace ctl {
const Colors& colors(const engine::Module* m) {
	static const Colors graphite = {
		nvgRGB(0x0B, 0x0B, 0x0D), nvgRGB(0x3A, 0x3B, 0x3F), nvgRGB(0xF2, 0xF0, 0xEA), nvgRGB(0xE9, 0xE7, 0xE1),
		nvgRGB(0xC8, 0xC8, 0xC5), nvgRGB(0xED, 0xED, 0xEA), nvgRGB(0x6C, 0x6D, 0x70), nvgRGB(0x4A, 0x4A, 0x4C), nvgRGB(0x05, 0x05, 0x05),
		nvgRGB(0x44, 0x46, 0x4B), nvgRGB(0x5A, 0x5C, 0x62), nvgRGB(0xFF, 0xB3, 0x47), nvgRGB(0xFF, 0xD0, 0x8A), nvgRGB(0x0D, 0x0D, 0x0F),
		nvgRGB(0xF2, 0xF0, 0xEA)};
	static const Colors paper = {
		nvgRGB(0x11, 0x11, 0x11), nvgRGB(0x3A, 0x3A, 0x3A), nvgRGB(0xF6, 0xF4, 0xEF), nvgRGB(0x1C, 0x1D, 0x20),
		nvgRGB(0xD3, 0xD3, 0xD0), nvgRGB(0xF7, 0xF7, 0xF5), nvgRGB(0x8E, 0x8E, 0x8A), nvgRGB(0x5C, 0x5C, 0x5A), nvgRGB(0x05, 0x05, 0x05),
		nvgRGB(0xF6, 0xF4, 0xEF), nvgRGB(0xFF, 0xFF, 0xFF), nvgRGB(0xE8, 0x96, 0x0C), nvgRGB(0xF5, 0xBC, 0x5A), nvgRGB(0x1C, 0x1D, 0x20),
		nvgRGB(0xFB, 0xFB, 0xF9)};
	int t = m ? static_cast<const hm::Harmonion*>(m)->theme : 0;
	return t == 1 ? paper : graphite;
}
}  // namespace ctl
