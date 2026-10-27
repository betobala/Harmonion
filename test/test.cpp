// Headless tests: drives the module's process() and checks the pitch outputs.
#include "../src/Harmonion.hpp"
#include "../src/Screen.hpp"
#include <cstdio>
#include <cstring>

Plugin* pluginInstance = nullptr;
using namespace hm;
using H = Harmonion;

static int failures = 0;
static H* m;
static const float SR = 48000.f;

static void run(int samples) {
	Module::ProcessArgs args;
	args.sampleRate = SR;
	args.sampleTime = 1.f / SR;
	for (int i = 0; i < samples; i++) {
		args.frame++;
		m->process(args);
	}
}
static void ms(float t) {
	run((int) (t * SR / 1000.f));
}
static void press(int param) {
	m->params[param].setValue(1.f);
	ms(5);
	m->params[param].setValue(0.f);
	ms(5);
}
static void connect(int input, float v) {
	m->inputs[input].channels = 1;
	m->inputs[input].setVoltage(v);
}
static void disconnect(int input) {
	m->inputs[input].channels = 0;
	m->inputs[input].setVoltage(0.f);
}
static void pulse(int input) {
	connect(input, 10.f);
	ms(2);
	connect(input, 0.f);
	ms(8);
}
static float out(int o) {
	return m->outputs[o].getVoltage();
}
static void expect(const char* what, float got, float want) {
	bool ok = std::fabs(got - want) < 0.002f;
	if (!ok)
		failures++;
	printf("%s %-46s got %7.4f  want %7.4f\n", ok ? "ok  " : "FAIL", what, got, want);
}
static void expectStr(const char* what, std::string got, std::string want) {
	bool ok = got == want;
	if (!ok)
		failures++;
	printf("%s %-46s got '%s'  want '%s'\n", ok ? "ok  " : "FAIL", what, got.c_str(), want.c_str());
}
static float st(int semis) {
	return semis / 12.f;
}
static std::string screenText() {
	Screen s;
	m->buildScreen(s);
	return s.allText();
}
static void expectHas(const char* what, std::string text, std::string want) {
	bool ok = text.find(want) != std::string::npos;
	if (!ok)
		failures++;
	for (char& c : text)
		if (c == '\n')
			c = '|';
	for (char& c : want)
		if (c == '\n')
			c = '|';
	printf("%s %-46s '%s' in '%s'\n", ok ? "ok  " : "FAIL", what, want.c_str(), text.c_str());
}
static void harmonyAt(int root, int mode, int degree) {
	m->params[H::ROOT_PARAM].setValue(root);
	m->params[H::MODE_PARAM].setValue(mode);
	m->params[H::DEGREE_PARAM].setValue(degree);
	ms(5);
}

// --dump-screens DIR: writes the display of characteristic states as JSON for tools/preview_display.py
static std::string dumpDir;
static void dump(const char* name) {
	if (dumpDir.empty())
		return;
	Screen s;
	m->buildScreen(s);
	json_t* root = json_object();
	json_t* runs = json_array();
	for (const Screen::Run& r : s.runs) {
		json_t* jr = json_object();
		json_object_set_new(jr, "x", json_real(r.x));
		json_object_set_new(jr, "y", json_real(r.y));
		json_object_set_new(jr, "align", json_integer(r.align));
		json_t* segs = json_array();
		for (const Screen::Seg& g : r.segs) {
			json_t* jg = json_object();
			json_object_set_new(jg, "text", json_string(g.text.c_str()));
			json_object_set_new(jg, "size", json_real(g.size));
			json_object_set_new(jg, "rise", json_real(g.rise));
			json_array_append_new(segs, jg);
		}
		json_object_set_new(jr, "segs", segs);
		json_array_append_new(runs, jr);
	}
	json_t* shapes = json_array();
	for (const Screen::Shape& sh : s.shapes) {
		json_t* js = json_pack("{s:i, s:f, s:f, s:f, s:f, s:b}", "kind", sh.kind, "x", sh.x, "y", sh.y, "w", sh.w, "h", sh.h, "fill", sh.fill);
		json_array_append_new(shapes, js);
	}
	json_object_set_new(root, "runs", runs);
	json_object_set_new(root, "shapes", shapes);
	std::string path = dumpDir + "/" + name + ".json";
	json_dump_file(root, path.c_str(), JSON_INDENT(1));
	json_decref(root);
}

int main(int argc, char** argv) {
	if (argc > 2 && !strcmp(argv[1], "--dump-screens"))
		dumpDir = argv[2];
	random::init();
	m = new H;
	for (int i = 0; i < H::OUTPUTS_LEN; i++)
		m->outputs[i].channels = 1;
	ms(20);

	printf("-- Harmony\n");
	expectStr("default chord", m->harmony.chordName(), "Clyd");
	expectStr("default scale", m->harmony.scaleTones(), "C D E F# G A B");
	m->params[H::ROOT_PARAM].setValue(2);
	m->params[H::DEGREE_PARAM].setValue(7);
	ms(5);
	expectStr("D min", m->harmony.chordName(), "Dmin");
	expectStr("D min scale", m->harmony.scaleTones(), "D E F G A Bb C");
	m->params[H::ROOT_PARAM].setValue(10);
	m->params[H::DEGREE_PARAM].setValue(1);
	ms(5);
	expectStr("Bb maj spelled with flats", m->harmony.scaleTones(), "Bb C D Eb F G A");
	m->params[H::MODE_PARAM].setValue(3);  // Major
	m->params[H::ROOT_PARAM].setValue(0);
	m->params[H::DEGREE_PARAM].setValue(7);
	ms(5);
	expectStr("Major mode degree G", m->harmony.chordName(), "G7");
	m->params[H::MODE_PARAM].setValue(2);  // Slash chords
	m->params[H::DEGREE_PARAM].setValue(0);
	ms(5);
	expectStr("Slash chord", m->harmony.chordName(), "D/C");
	// Spellings and chord symbols of the manual's display photos
	harmonyAt(1, 0, 4);
	expectStr("C# alt in button order", m->harmony.scaleTones(), "C# D F E A G B");
	harmonyAt(5, 0, 4);
	expectStr("F alt in button order", m->harmony.scaleTones(), "F F# A G# C# B D#");
	harmonyAt(0, 0, 6);
	expectStr("C dor", m->harmony.scaleTones(), "C D Eb F G A Bb");
	harmonyAt(3, 5, 0);
	expectStr("Eb Mel.Minor chord", m->harmony.chordName(), "Eb-△7");
	expectStr("Eb Mel.Minor scale", m->harmony.scaleTones(), "Eb F Gb Ab Bb C D");
	harmonyAt(0, 1, 6);
	expectStr("Chords2 locrian", m->harmony.chordName(), "C-7b5");
	harmonyAt(0, 1, 5);
	expectStr("Chords2 lydian augmented", m->harmony.chordName(), "C△7#5#11");
	harmonyAt(0, 0, 0);

	printf("-- Quantizers\n");
	connect(H::CH_IN_INPUT + 1, 0.30f);
	ms(5);
	expect("Ch2 all tones, 0.30V -> E0", out(H::CH_OUTPUT + 1), st(4));
	connect(H::CH_IN_INPUT + 0, 0.50f);
	ms(5);
	expect("Ch1 A 1/3/5/7, 0.50V -> G0", out(H::CH_OUTPUT + 0), st(7));
	connect(H::CH_IN_INPUT + 2, 1.24f);
	ms(5);
	expect("Ch3 1/3/5, 1.24V -> E1", out(H::CH_OUTPUT + 2), st(16));
	press(H::BIG_PARAM + C2_FUNC[FILL2]);  // E in C lydian is 3rd; fill 2 = D#
	press(H::BIG_PARAM + C2_FUNC[T3]);
	connect(H::CH_IN_INPUT + 1, 0.26f);
	ms(5);
	expect("Ch2 3RD off, FILL2 on -> D#0", out(H::CH_OUTPUT + 1), st(3));
	press(H::BIG_PARAM + C2_FUNC[FILL2]);
	press(H::BIG_PARAM + C2_FUNC[T3]);
	// EQ-DIST: ROOT, 5TH, 9TH equally spaced
	m->editPreset().ch3 = (1 << T1) | (1 << T5) | (1 << T9);
	m->editPreset().eqDist = true;
	connect(H::CH_IN_INPUT + 2, 0.40f);  // 40% of the octave -> second of three notes (D)
	ms(5);
	expect("Ch3 EQ-DIST 0.40V -> D0", out(H::CH_OUTPUT + 2), st(2));
	connect(H::CH_IN_INPUT + 2, 0.70f);
	ms(5);
	expect("Ch3 EQ-DIST 0.70V -> G0", out(H::CH_OUTPUT + 2), st(7));
	m->editPreset().eqDist = false;
	m->editPreset().ch3 = (1 << T1) | (1 << T3) | (1 << T5);
	// Transpose and octave
	m->params[H::TRANS_PARAM].setValue(2);
	connect(H::CH_IN_INPUT + 1, 0.30f);
	ms(5);
	expect("TRANS +2", out(H::CH_OUTPUT + 1), st(6));
	m->params[H::TRANS_PARAM].setValue(0);
	press(H::OCT_PARAM);
	press(H::BIG_PARAM + OCT_MAP[SEC_CH2].up);
	press(H::OCT_PARAM);
	ms(5);
	expect("Ch2 octave +1", out(H::CH_OUTPUT + 1), st(16));
	m->editPreset().octave[SEC_CH2] = 0;
	// Sample & hold
	connect(H::CH_TRIG_INPUT + 1, 0.f);
	connect(H::CH_IN_INPUT + 1, 0.60f);
	ms(5);
	expect("Ch2 with TRIG, no trigger yet -> held", out(H::CH_OUTPUT + 1), st(4));
	pulse(H::CH_TRIG_INPUT + 1);
	expect("Ch2 after trigger -> G0", out(H::CH_OUTPUT + 1), st(7));
	disconnect(H::CH_TRIG_INPUT + 1);
	// Input scaling 5V/oct
	m->settings.inScale[SEC_CH2] = 3;
	connect(H::CH_IN_INPUT + 1, 5.f);
	ms(5);
	expect("Ch2 5V/oct, 5V -> C1", out(H::CH_OUTPUT + 1), st(12));
	m->settings.inScale[SEC_CH2] = 0;
	// Bypass
	m->settings.bypass[1] = 2;
	connect(H::CH_IN_INPUT + 1, 1.2345f);
	ms(5);
	expect("Ch2 true bypass", out(H::CH_OUTPUT + 1), 1.2345f);
	m->settings.bypass[1] = 1;
	connect(H::CH_TRIG_INPUT + 1, 0.f);
	connect(H::CH_IN_INPUT + 1, 0.2f);
	ms(5);
	expect("Ch2 soft bypass ignores TRIG, keeps TRANS", out(H::CH_OUTPUT + 1), 0.2f);
	disconnect(H::CH_TRIG_INPUT + 1);
	m->settings.bypass[1] = 0;
	// Pre Trans is added after the semitone quantization and before sample & hold
	m->settings.cvFunc[2] = CV_PRE_CH2;
	connect(H::CV_INPUT + 2, st(2));
	connect(H::CH_IN_INPUT + 1, 0.30f);
	ms(5);
	expect("Ch2 Pre Trans +2: E0 -> F#0", out(H::CH_OUTPUT + 1), st(6));
	m->settings.cvFunc[2] = CV_NONE;
	disconnect(H::CV_INPUT + 2);
	// Channel 3 OCT page: 3RD is octave down, 5TH glide detection (manual p. 30)
	expect("OCT Ch3 down is 3RD", OCT_MAP[SEC_CH3].down, top(0, 7));
	expect("OCT Ch3 glide is 5TH", OCT_MAP[SEC_CH3].glide, top(1, 6));
	expect("OCT Arp follow is PATTERN", OCT_MAP[SEC_ARP].follow, ARP_BTN + AB_PATTERN);
	// Ch1 A/B via jack
	m->settings.abMode = AB_JACK;
	m->editPreset().ch1B = 1;  // ROOT only
	connect(H::CH_IN_INPUT + 0, 0.60f);
	connect(H::AB_INPUT, 10.f);
	ms(5);
	expect("Ch1 side B (ROOT) -> C1", out(H::CH_OUTPUT + 0), st(12));
	connect(H::AB_INPUT, 0.f);
	ms(5);
	expect("Ch1 side A -> G0", out(H::CH_OUTPUT + 0), st(7));
	connect(H::CH_IN_INPUT + 0, 0.50f);
	m->settings.abMode = AB_OFF;

	printf("-- Chords\n");
	ms(5);
	expect("Chord voice 1 = C2", out(H::CHORD_OUTPUT + 0), st(24));
	expect("Chord voice 2 = E2", out(H::CHORD_OUTPUT + 1), st(28));
	expect("Chord voice 3 = G2", out(H::CHORD_OUTPUT + 2), st(31));
	expect("Chord voice 4 = B2", out(H::CHORD_OUTPUT + 3), st(35));
	m->editPreset().inversion = 2;  // first inversion: 3rd at the bottom
	ms(5);
	expect("1st inversion voice 2 (3rd) lowest = E2", out(H::CHORD_OUTPUT + 1), st(28));
	expect("1st inversion voice 1 (root) = C3", out(H::CHORD_OUTPUT + 0), st(36));
	m->editPreset().inversion = 0;
	m->editPreset().chords = (1 << T1) | (1 << T5);
	ms(5);
	expect("Root+5th doubled: voice 3 = C3", out(H::CHORD_OUTPUT + 2), st(36));
	expect("Root+5th doubled: voice 4 = G3", out(H::CHORD_OUTPUT + 3), st(43));
	m->editPreset().chords = (1 << T1) | (1 << T3) | (1 << T5) | (1 << T7);
	m->params[H::SPREAD_PARAM].setValue(12);
	ms(5);
	expect("Spread 12: top voice B3", out(H::CHORD_OUTPUT + 3), st(47));
	m->params[H::SPREAD_PARAM].setValue(0);
	// The manual's Chord Structure photo: Cmaj7, lowest note 5th, spread -> G E B C
	harmonyAt(0, 0, 1);
	m->editPreset().inversion = 3;
	m->params[H::SPREAD_PARAM].setValue(8);
	m->settings.display = DISP_CHORD;
	m->splashUntil = 0.0;
	ms(5);
	expectHas("Chord Structure 5··3·71", screenText(), "5··3·71");
	dump("chord_structure");
	m->params[H::SPREAD_PARAM].setValue(0);
	m->editPreset().inversion = 0;
	m->settings.display = DISP_TONES;
	harmonyAt(0, 0, 0);
	// Voicing CV 1 = 1/3/5/7, 36 = 1/3/5, 98 = 13
	m->settings.cvFunc[0] = CV_VOICING;
	connect(H::CV_INPUT + 0, 36.f / 12.f);
	ms(5);
	expect("Voicing CV 36 -> 1/3/5 (voice 4 = C3)", out(H::CHORD_OUTPUT + 3), st(36));
	m->settings.cvFunc[0] = CV_NONE;
	disconnect(H::CV_INPUT + 0);
	m->editPreset().chords = (1 << T1) | (1 << T3) | (1 << T5) | (1 << T7);

	printf("-- Harmonic shift\n");
	m->settings.cvFunc[1] = CV_HARMONIC_SHIFT;
	connect(H::CV_INPUT + 1, 5.f);
	connect(H::CH_IN_INPUT + 1, 0.60f);
	ms(5);
	expect("Shift 5V forces root (Ch2 0.6V -> C1)", out(H::CH_OUTPUT + 1), st(12));
	connect(H::CV_INPUT + 1, 4.3f);
	ms(5);
	expect("Shift 4.3V root+5th (Ch2 0.6V -> G0)", out(H::CH_OUTPUT + 1), st(7));
	m->settings.cvFunc[1] = CV_NONE;
	disconnect(H::CV_INPUT + 1);

	printf("-- Arpeggiator\n");
	m->params[H::ARP_PITCH_PARAM].setValue(2.f);
	m->params[H::RANGE_PARAM].setValue(12.f);
	const int upSeq[] = {24, 28, 31, 35, 36, 24};
	for (int i = 0; i < 6; i++) {
		pulse(H::ARP_CLK_INPUT);
		char buf[64];
		snprintf(buf, sizeof(buf), "Arp Up step %d", i + 1);
		expect(buf, out(H::ARP_OUTPUT), st(upSeq[i]));
	}
	m->editPreset().pattern = 2;  // Up/Down
	m->arpIdx = -1;
	const int udSeq[] = {24, 28, 31, 35, 36, 35, 31};
	for (int i = 0; i < 7; i++) {
		pulse(H::ARP_CLK_INPUT);
		char buf[64];
		snprintf(buf, sizeof(buf), "Arp Up/Down step %d", i + 1);
		expect(buf, out(H::ARP_OUTPUT), st(udSeq[i]));
	}
	m->editPreset().pattern = 0;
	// Clock multiplier 2/1: one input clock gives two steps
	m->arpIdx = -1;
	m->settings.arpClock = 11;
	pulse(H::ARP_CLK_INPUT);
	ms(490);
	pulse(H::ARP_CLK_INPUT);  // period now known: 500 ms
	expect("Arp 2/1 after 2nd clock (3rd beat)", out(H::ARP_OUTPUT), st(31));
	ms(300);
	expect("Arp 2/1 interpolated 4th beat", out(H::ARP_OUTPUT), st(35));
	m->settings.arpClock = ARP_RATIO_UNITY;
	disconnect(H::ARP_CLK_INPUT);

	printf("-- Sequencer\n");
	press(H::SEQ_PARAM);
	expectStr("Seq starts at Clyd", m->harmony.chordName(), "Clyd");
	m->currentStep().beats = 2;
	press(H::ADD_PARAM);  // step 2, copy
	m->currentStep().root = 5;
	m->currentStep().degree = 1;
	m->currentStep().beats = 2;
	press(H::LEFT_PARAM);
	expect("Back on step 1", m->step, 0);
	// Clock: 1st clock = beat 1, 2nd = beat 2, 3rd -> step 2
	pulse(H::SEQ_CLK_INPUT);
	pulse(H::SEQ_CLK_INPUT);
	expectStr("Still step 1 after 2 clocks", m->harmony.chordName(), "Clyd");
	pulse(H::SEQ_CLK_INPUT);
	expectStr("Step 2 after 3 clocks", m->harmony.chordName(), "Fmaj");
	pulse(H::SEQ_CLK_INPUT);
	pulse(H::SEQ_CLK_INPUT);
	expectStr("Wraps to step 1", m->harmony.chordName(), "Clyd");
	// Song reset via IN 1
	m->settings.inFunc[0] = IN_SONG_RESET;
	pulse(H::SEQ_CLK_INPUT);
	pulse(H::SEQ_CLK_INPUT);
	expectStr("Step 2 again", m->harmony.chordName(), "Fmaj");
	pulse(H::IN_INPUT + 0);
	expectStr("Reset to step 1", m->harmony.chordName(), "Clyd");
	m->settings.inFunc[0] = IN_NONE;
	// Encoder-like knob editing changes the current step
	m->params[H::ROOT_PARAM].setValue(m->params[H::ROOT_PARAM].getValue() + 2);
	ms(5);
	expectStr("ROOT knob +2 edits step", m->harmony.chordName(), "Dlyd");
	press(H::SEQ_PARAM);
	// Follow root on Ch2: 0.0V in, root D -> shifted by 2 semitones
	m->params[H::ROOT_PARAM].setValue(2);
	m->settings.follow[SEC_CH2] = true;
	connect(H::CH_IN_INPUT + 1, 0.f);
	ms(5);
	expect("IN follows ROOT: C0 in D lydian -> D0", out(H::CH_OUTPUT + 1), st(2));
	m->settings.follow[SEC_CH2] = false;
	m->params[H::ROOT_PARAM].setValue(0);

	printf("-- Tuning\n");
	press(H::TUNE_PARAM);
	expect("Tune mode C4", out(H::CHORD_OUTPUT + 2), 4.f);
	press(H::RIGHT_PARAM);
	expect("Tune mode C5", out(H::CH_OUTPUT + 0), 5.f);
	press(H::TUNE_PARAM);

	printf("-- Display\n");
	m->splashUntil = 0.0;
	ms(5);
	expectHas("main: root and mode", screenText(), "C\nChords\n");
	expectHas("main: chord with raised suffix", screenText(), "Clyd");
	expectHas("main: scale tones", screenText(), "C D E F♯ G A B");
	expectHas("main: transposition and preset", screenText(), "+0\n1\n");
	dump("main");
	m->settings.display = DISP_STRUCTURE;
	dump("scale_structure");
	m->settings.display = DISP_OCTAVES;
	m->editPreset().octave[SEC_CH1] = -3;
	m->editPreset().octave[SEC_CH2] = 2;
	dump("octave_switches");
	m->editPreset().octave[SEC_CH1] = 0;
	m->editPreset().octave[SEC_CH2] = 0;
	m->settings.display = DISP_TONES;
	m->menuOpen = true;
	m->menuItem = MI_CV1;
	expectHas("menu: CV 1 not assigned", screenText(), "CV 1\n(not assigned)");
	dump("menu_cv1");
	m->menuItem = MI_IN1;
	m->settings.inFunc[0] = IN_SLEW_CH2;
	int count = 0;
	expect("menu: Chan 2 Slew is the 10th In value", m->menuValuePos(MI_IN1, &count), 9);
	dump("menu_in1");
	m->settings.inFunc[0] = IN_NONE;
	m->menuItem = MI_DISPLAY;
	expect("menu: Display is item 20", MI_DISPLAY, 19);
	expectHas("menu: Display", screenText(), "Display\nScale Tones");
	m->menuItem = MI_RESET;
	expectHas("menu: reset", screenText(), "Reset To Defaults\npress + to reset");
	dump("menu_reset");
	m->settings.abMode = AB_ODD_BEATS;
	expect("menu: Each Odd Beat is the 3rd A/B value", m->menuValuePos(MI_CH1_AB, &count), 2);
	expect("menu: nine A/B values", count, 9);
	m->settings.abMode = AB_OFF;
	m->menuOpen = false;
	m->tuning = true;
	m->tuneOctave = 3;
	expectHas("tuning 3V", screenText(), "3V/C5/523.2 Hz");
	dump("tuning");
	m->tuneOctave = -2;
	expectHas("tuning -2V", screenText(), "-2V/C0/16.4 Hz");
	m->tuneOctave = 8;
	expectHas("tuning 8V", screenText(), "8V/C10/16743.7 Hz");
	m->tuneOctave = 4;
	m->tuning = false;
	m->calibrating = true;
	m->calibLast = 0;
	m->calib5V = false;
	expectHas("calibration 0V", screenText(), "13107 Channel 1");
	dump("calibration");
	m->calib5V = true;
	expectHas("calibration 5V", screenText(), "45875 Channel 1");
	m->calib5V = false;
	m->calibrating = false;
	m->settings.inMix[1][0] = true;
	m->settings.outSrc[2] = 1;
	m->page = H::PAGE_MATRIX;
	expectHas("matrix rows", screenText(), "→CH2→_23_");
	dump("matrix");
	m->page = H::PAGE_MAIN;
	m->settings.inMix[1][0] = false;
	m->settings.outSrc[2] = 2;
	// Section options of the OCT page are confirmed on the display
	press(H::OCT_PARAM);
	press(H::BIG_PARAM + OCT_MAP[SEC_CH2].scaling);
	expectHas("OCT: Ch2 input scaling", screenText(), "Channel 2\n2 Volts / Octave");
	dump("popup_oct");
	for (int i = 0; i < 3; i++)
		press(H::BIG_PARAM + OCT_MAP[SEC_CH2].scaling);
	expectHas("OCT: Ch2 back to 1 V/oct", screenText(), "Channel 2\n1 Volt / Octave");
	press(H::BIG_PARAM + OCT_MAP[SEC_ARP].scaling);
	expectHas("OCT: arpeggio input scaling (BEGIN)", screenText(), "Arpeggio\n2 Volts / Octave");
	for (int i = 0; i < 3; i++)
		press(H::BIG_PARAM + OCT_MAP[SEC_ARP].scaling);
	press(H::BIG_PARAM + OCT_MAP[SEC_CH1].bypass);
	expectHas("OCT: Ch1 soft bypass", screenText(), "Channel 1\nBypass + Transpose");
	press(H::BIG_PARAM + OCT_MAP[SEC_CH1].bypass);
	press(H::BIG_PARAM + OCT_MAP[SEC_CH1].bypass);
	expectHas("OCT: Ch1 back to normal", screenText(), "Channel 1\nNormal, Quantize");
	press(H::BIG_PARAM + OCT_MAP[SEC_CHORDS].follow);
	expectHas("OCT: chords follow root", screenText(), "Chords\nIN follows ROOT");
	press(H::BIG_PARAM + OCT_MAP[SEC_CHORDS].follow);
	press(H::BIG_PARAM + OCT_MAP[SEC_CH3].glide);
	expectHas("OCT: Ch3 slew detection", screenText(), "Channel 3\nSlew Detection on");
	press(H::BIG_PARAM + OCT_MAP[SEC_CH3].glide);
	press(H::OCT_PARAM);
	m->popupUntil = 0.0;
	m->editPreset().pattern = PAT_UP;
	press(H::BIG_PARAM + ARP_BTN + AB_PATTERN);
	press(H::BIG_PARAM + ARP_BTN + AB_PATTERN);
	press(H::BIG_PARAM + ARP_BTN + AB_PATTERN);
	press(H::BIG_PARAM + ARP_BTN + AB_PATTERN);
	expectHas("PATTERN steps like the manual's screens", screenText(), "Arpeggio Mode\nOctaves");
	dump("popup_pattern");
	m->editPreset().pattern = PAT_UP;
	m->popupUntil = 0.0;
	m->settings.cvFunc[3] = CV_VOLTMETER;
	connect(H::CV_INPUT + 3, 2.227f);
	ms(5);
	expectHas("voltmeter in the center line", screenText(), "+2.227 Volts");
	dump("voltmeter");
	m->settings.cvFunc[3] = CV_NONE;
	disconnect(H::CV_INPUT + 3);
	m->settings.inFunc[0] = IN_SYNC_SLAVE;
	ms(100);
	expectHas("sync slave without master", screenText(), "Sync: dead");
	dump("sync");
	m->settings.inFunc[0] = IN_NONE;
	ms(5);
	// Sequenced: 8 beats in two rows, 3 steps (manual photo of Eb Mel.Minor)
	press(H::SEQ_PARAM);
	m->currentStep().beats = 8;
	press(H::ADD_PARAM);
	press(H::ADD_PARAM);
	m->currentStep().root = 3;
	m->currentStep().mode = 5;
	m->currentStep().degree = 0;
	ms(5);
	expectHas("seq: song part", screenText(), "1A");
	dump("sequencer");
	press(H::SEQ_PARAM);
	m->splashUntil = 10.0;
	dump("splash");
	m->splashUntil = 0.0;

	printf("-- Button combinations\n");
	// Calibration: TUNE + LEFT + Channel 1 3RD held at once; MENU leaves
	m->params[H::TUNE_PARAM].setValue(1.f);
	m->params[H::LEFT_PARAM].setValue(1.f);
	ms(5);
	m->params[H::BIG_PARAM + C1A_TONE[T3]].setValue(1.f);
	ms(5);
	m->params[H::TUNE_PARAM].setValue(0.f);
	m->params[H::LEFT_PARAM].setValue(0.f);
	m->params[H::BIG_PARAM + C1A_TONE[T3]].setValue(0.f);
	ms(5);
	expect("TUNE + LEFT + 3RD enters calibration", m->calibrating, 1);
	expect("tuning mode is left", m->tuning, 0);
	press(H::SEQ_PARAM);
	expect("SEQ switches calibration to 5 V", m->calib5V, 1);
	press(H::MENU_PARAM);
	expect("MENU leaves calibration", m->calibrating, 0);
	m->calib5V = false;
	// Factory reset: Channel 3 3RD, 7TH, 11TH, EQ-DIST and Arpeggio BEGIN held at once
	m->presets[5].chords = 0x55;
	const int resetBtns[5] = {C3_TONE[T3], C3_TONE[T7], C3_TONE[T11], C3_EQDIST, ARP_BTN + AB_BEGIN};
	for (int b : resetBtns) {
		m->params[H::BIG_PARAM + b].setValue(1.f);
		ms(5);
	}
	for (int b : resetBtns)
		m->params[H::BIG_PARAM + b].setValue(0.f);
	ms(5);
	expect("factory reset combination restores presets", m->presets[5].chords, Preset::defaults().chords);
	expect("factory reset keeps BEGIN at Next Note", m->presets[0].begin, -1);

	printf("-- Persistence\n");
	m->presets[3].chords = 0x55;
	json_t* j = m->dataToJson();
	H* m2 = new H;
	m2->dataFromJson(j);
	expect("Preset survives JSON", m2->presets[3].chords, 0x55);
	json_decref(j);
	// A version 1 patch saved the arpeggio clock as index into the old list (index 7 was 5/4)
	json_t* old = json_pack("{s:{s:i, s:i}}", "settings", "arpClock", 6, "trigWindowMs", 20);
	m2->dataFromJson(old);
	expect("v1 arpClock 1/1 keeps 1/1", m2->settings.arpClock, ARP_RATIO_UNITY);
	expect("v1 trigger window clamped", m2->settings.trigWindowMs, MAX_TRIG_WINDOW_MS);
	json_decref(old);
	old = json_pack("{s:{s:i}}", "settings", "arpClock", 11);
	m2->dataFromJson(old);
	expect("v1 arpClock 2/1 keeps 2/1", ARP_RATIO[m2->settings.arpClock][0] * 10 + ARP_RATIO[m2->settings.arpClock][1], 21);
	json_decref(old);

	printf("\n%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
	return failures ? 1 : 0;
}
