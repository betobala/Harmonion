#include "Screen.hpp"
#include "Harmonion.hpp"

namespace hm {

constexpr float Screen::BIG;
constexpr float Screen::SMALL;
constexpr float Screen::RISE;
constexpr float Screen::X_SCALE;
constexpr float Screen::MAX_WIDTH;

std::string Screen::allText() const {
	std::string s;
	for (const Run& r : runs) {
		for (const Seg& g : r.segs)
			s += g.text;
		s += "\n";
	}
	return s;
}

void addNote(Screen::Run& r, const std::string& name, float size) {
	r.segs.push_back(Screen::Seg{name.substr(0, 1), size, 0.f});
	if (name.size() > 1) {
		std::string acc = (name[1] == 'b') ? "♭" : (name[1] == '#') ? "♯" : name.substr(1);
		r.segs.push_back(Screen::Seg{acc, size * Screen::SMALL / Screen::BIG, Screen::RISE * size / Screen::BIG});
	}
}

std::string suffixGlyphs(const std::string& suffix) {
	std::string s;
	for (char c : suffix) {
		if (c == 'b')
			s += "♭";
		else if (c == '#')
			s += "♯";
		else
			s += c;
	}
	return s;
}

// Layout in display pixels, measured on the photos of the original's display
static const float LEFT_X = 3.4f, RIGHT_X = 125.5f, MODE_X = 24.5f;
static const float TOP_Y = 15.f, CENTER_Y = 38.f, BOTTOM_Y = 61.f;
static const float TITLE_Y = 30.f, VALUE_Y = 45.f;
// Advance of one BIG character, for laying out shapes next to text
static const float CHAR_W = Screen::BIG * 0.62f * Screen::X_SCALE;

/** Row of small squares with one tall filled block: the position indicator of menu and tuning */
static void positionBar(Screen& s, int n, int pos, float pitch) {
	float x0 = 64.f - (n - 1) * pitch / 2.f;
	for (int i = 0; i < n; i++) {
		float cx = x0 + i * pitch;
		if (i == pos)
			s.rect(cx - 1.2f, 6.f, 2.4f, 4.6f, true);
		else
			s.rect(cx - 1.2f, 7.1f, 2.4f, 2.4f, false);
	}
}

/** Dashes with the current value as a block, below the menu value. Two rows above 20 values. */
static void valueBar(Screen& s, int n, int pos) {
	int rows = (n > 20) ? 2 : 1;
	int perRow = (n + rows - 1) / rows;
	const float pitch = 6.f, w = 3.7f;
	float x0 = 64.f - (perRow - 1) * pitch / 2.f;
	for (int i = 0; i < n; i++) {
		int row = i / perRow, col = i % perRow;
		float cx = x0 + col * pitch;
		float y = (rows == 1) ? 57.9f : (row == 0 ? 57.4f : 61.8f);
		if (i == pos)
			s.rect(cx - w / 2, y - 1.4f, w, 2.8f, true);
		else
			s.rect(cx - w / 2, y - 0.6f, w, 1.2f, true);
	}
}

/** Octave switch drawn as a fader: dotted track, cap at -3..+3, filled at the neutral position */
static void octaveSlider(Screen& s, float cx, int octave) {
	s.line(cx, 27.2f, cx, 39.8f, true);
	float cy = 33.7f - 1.15f * octave;
	s.rect(cx - 4.3f, cy - 2.3f, 8.6f, 4.6f, octave == 0);
}

void Harmonion::buildScreen(Screen& s) {
	s.clear();
	typedef Screen S;

	if (time < splashUntil) {
		s.text(64, TITLE_Y, "Harmonion", S::CENTER);
		std::string v = (pluginInstance) ? pluginInstance->version : "";
		s.text(64, VALUE_Y, v.empty() ? "" : "v" + v, S::CENTER);
		return;
	}

	if (calibrating) {
		static const char* const outNames[8] = {"Channel 1", "Channel 2", "Channel 3", "Arpeggio", "Chords 1", "Chords 2", "Chords 3", "Chords 4"};
		s.text(LEFT_X, TOP_Y, "Calibrate");
		s.text(RIGHT_X, TOP_Y, calib5V ? "5V" : "0V", S::RIGHT);
		// Absolute value of the 16 bit DAC whose range is -2 V..8 V
		float nominal = ((calib5V ? 5.f : 0.f) + 2.f) / DAC_UNIT_V;
		int dac = (int) std::round(nominal + calib[calibLast][calib5V ? 1 : 0] / DAC_UNIT_V);
		s.text(64, CENTER_Y, string::f("%d %s", dac, outNames[clamp(calibLast, 0, 7)]), S::CENTER);
		return;
	}

	if (menuOpen) {
		positionBar(s, NUM_MENU_ITEMS, menuItem, 4.7f);
		s.text(64, TITLE_Y, menuTitle(menuItem), S::CENTER);
		s.text(64, VALUE_Y, menuValue(menuItem), S::CENTER);
		int n = 1;
		int pos = menuValuePos(menuItem, &n);
		valueBar(s, n, pos);
		return;
	}

	if (tuning || inFuncHigh[IN_TUNING]) {
		positionBar(s, 11, tuneOctave + 2, 5.f);
		s.text(64, TITLE_Y + 2.f, "Tuning Mode", S::CENTER);
		// C0 as the original computes it: its display shows 16.4, 523.2 and 16743.7 Hz
		float hz = 16.3513f * std::pow(2.f, (float) (tuneOctave + 2));
		s.text(64, VALUE_Y + 1.f, string::f("%dV/C%d/%.1f Hz", tuneOctave, tuneOctave + 2, hz), S::CENTER);
		return;
	}

	if (popup != POPUP_NONE && time < popupUntil) {
		const Preset& p = editPreset();
		std::string title, value;
		switch (popup) {
			case POPUP_PATTERN:
				title = "Arpeggio Mode";
				value = PATTERN_NAMES[clamp((int) p.pattern, 0, NUM_PATTERNS - 1)];
				break;
			case POPUP_BEGIN:
				title = "Arpeggio Begin";
				value = BEGIN_NAMES[clamp(p.begin + 1, 0, 7)];
				break;
			case POPUP_INV:
				title = "Chord Inversion";
				value = INVERSION_NAMES[p.inversion % 5];
				break;
			default:
				title = "Buttons + Octaves";
				value = settings.liveMode ? "are Live" : "are Sequenced";
				break;
		}
		s.text(64, TITLE_Y, title, S::CENTER);
		s.text(64, VALUE_Y, value, S::CENTER);
		return;
	}

	if (page == PAGE_MATRIX) {
		// Input mixing matrix as a grid of the four IN jacks over the four rows, then the outputs
		// of each row ("1", "2", "3", "A") in their output column
		static const char* const rows[4] = {"CH1", "CH2", "CH3", "ARP"};
		static const char outChar[4] = {'1', '2', '3', 'A'};
		const float colX[4] = {14.f, 25.f, 36.f, 46.5f};
		for (int j = 0; j < 4; j++)
			s.line(colX[j], 4.f, colX[j], 60.f);
		for (int r = 0; r < 4; r++) {
			float y = 9.f + 15.f * r;
			s.line(9.f, y, 50.f, y);
			for (int j = 0; j < 4; j++)
				if (settings.inMix[r][j])
					s.rect(colX[j] - 1.8f, y - 1.8f, 3.6f, 3.6f, false);
			std::string out;
			for (int j = 0; j < 4; j++)
				out += (settings.outSrc[j] == r) ? outChar[j] : '_';
			s.text(50.5f, y + 5.f, std::string("→") + rows[r] + "→" + out);
		}
		return;
	}

	const Harmony& h = frozen ? editHarmony : harmony;
	if (!h.scale) {
		// The engine has not computed a harmony yet (first frames after adding the module)
		return;
	}
	const Preset& p = editPreset();

	// Top: key root and mode at the left, chord symbol at the right with a raised suffix
	addNote(s.run(LEFT_X, TOP_Y, S::LEFT), noteName(h.keyRoot, preferFlats(h.chordRoot, *h.scale)));
	s.text(MODE_X, TOP_Y, h.modeName());
	Screen::Run& chord = s.run(RIGHT_X, TOP_Y, S::RIGHT);
	addNote(chord, h.chordRootName());
	chord.segs.push_back(Screen::Seg{suffixGlyphs(h.chordSuffix()), S::SMALL, S::RISE});

	// Center
	int disp = (page == PAGE_OCT) ? DISP_OCTAVES : settings.display;
	if (voltmeterActive) {
		s.text(64, CENTER_Y, string::f("%+.3f Volts", voltmeter), S::CENTER);
	}
	else if (disp == DISP_OCTAVES) {
		// Channel 1-3, chords and arpeggio
		for (int k = 0; k < NUM_SECTIONS; k++)
			octaveSlider(s, 30.3f + 17.2f * k, p.octave[k]);
	}
	else if (disp == DISP_STRUCTURE) {
		// Bullets are scale notes, tildes the semitones in between, from root to octave
		uint16_t mask = h.scaleMask();
		for (int o = 0; o <= 12; o++) {
			float cx = 64.f + (o - 6) * 6.7f;
			if (mask & (1 << (o % 12)))
				s.circle(cx, 33.5f, 2.3f, true);
			else
				s.text(cx, 36.6f, "~", S::CENTER, S::SMALL);
		}
	}
	else if (disp == DISP_CHORD) {
		octaveSlider(s, 28.6f, p.octave[SEC_CHORDS]);
		// From the lowest to the highest voice every pitch of a selected chord tone: its number if a
		// voice plays it, a dot if it is skipped (spread, or more than four tones selected)
		static const char* const toneNum[NUM_TONES] = {"1", "3", "5", "7", "9", "11", "13"};
		uint16_t funcs = sectionFuncs(SEC_CHORDS, p) & 0x7F;
		int lo = chordNotes[0], hi = chordNotes[0];
		for (int k = 1; k < 4; k++) {
			lo = std::min(lo, chordNotes[k]);
			hi = std::max(hi, chordNotes[k]);
		}
		std::string str;
		for (int n = lo; n <= hi && hi - lo < 120; n++) {
			int voice = -1;
			for (int k = 0; k < 4; k++)
				if (chordNotes[k] == n)
					voice = k;
			if (voice >= 0) {
				str += toneNum[clamp(chordFuncs[voice], 0, NUM_TONES - 1)];
				continue;
			}
			for (int t = 0; t < NUM_TONES; t++) {
				if ((funcs & (1 << t)) && h.notes[t][0] >= 0 && wrap(n, 12) == h.notes[t][0]) {
					str += "·";
					break;
				}
			}
		}
		s.text(66.f, CENTER_Y - 0.4f, str, S::CENTER);
		// Inversion as four note heads, top to bottom 3rd, 2nd, 1st inversion and root position
		// (like the INV shortcuts ROOT, 5TH, 9TH, 13TH). All hollow means free.
		for (int i = 0; i < 4; i++) {
			float y = 28.f + 3.8f * i;
			bool sel = p.inversion == 4 - i;
			s.rect(99.6f, y - 1.3f, 4.7f, 2.6f, sel);
			if (sel) {
				s.line(104.3f, y, 104.3f, 26.5f);
				s.line(104.3f, 29.5f, 106.8f, 30.8f);
			}
		}
	}
	else {
		// Scale tones in button order, with a narrow gap between the notes
		Screen::Run& r = s.run(64, CENTER_Y, S::CENTER);
		std::vector<std::string> names = h.scaleToneNames();
		for (size_t i = 0; i < names.size(); i++) {
			if (i)
				r.segs.push_back(Screen::Seg{" ", S::SMALL, 0.f});
			addNote(r, names[i]);
		}
	}

	// Bottom: transposition, preset or song part, and the sequencer position or the sync state
	s.text(LEFT_X, BOTTOM_Y, string::f("%+d", transSemis));
	if (syncSlave) {
		s.text(64, BOTTOM_Y, syncAlive ? "Sync: alive" : "Sync: dead", S::CENTER);
		s.text(RIGHT_X, BOTTOM_Y, string::f("%d", presetIndex + 1), S::RIGHT);
	}
	else if (seqMode) {
		s.text(RIGHT_X, BOTTOM_Y, string::f("%d%c", song + 1, 'A' + part), S::RIGHT);
		// Beats as circles (four per row), steps as boxes (eight per row); "12/16" when there are
		// more than 8 beats or 16 steps
		int beats = currentStep().beats;
		int steps = currentPart().steps.size();
		const float bPitch = 6.6f, bD = 4.5f, sPitch = 7.7f, sW = 5.5f, gap = 8.8f;
		std::string beatText = string::f("%d/%d", beat + 1, beats);
		std::string stepText = string::f("%d/%d", step + 1, steps);
		float beatsW = (beats <= 8) ? (std::min(beats, 4) - 1) * bPitch + bD : beatText.size() * CHAR_W;
		float stepsW = (steps <= 16) ? (std::min(steps, 8) - 1) * sPitch + sW : stepText.size() * CHAR_W;
		float x = 64.8f - (beatsW + gap + stepsW) / 2.f;
		if (beats <= 8) {
			for (int b = 0; b < beats; b++)
				s.circle(x + bD / 2 + (b % 4) * bPitch, (b < 4) ? 51.2f : 58.1f, bD / 2, b == beat);
		}
		else {
			s.text(x, BOTTOM_Y, beatText);
		}
		x += beatsW + gap;
		if (steps <= 16) {
			for (int k = 0; k < steps; k++)
				s.rect(x + (k % 8) * sPitch, (k < 8) ? 49.3f : 55.9f, sW, 4.5f, k == step);
		}
		else {
			s.text(x, BOTTOM_Y, stepText);
		}
	}
	else {
		s.text(RIGHT_X, BOTTOM_Y, string::f("%d", presetIndex + 1), S::RIGHT);
	}
}

}  // namespace hm
