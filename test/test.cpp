// Headless tests: drives the module's process() and checks the pitch outputs.
#include "../src/Harmonion.hpp"
#include <cstdio>

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

int main() {
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
	m->params[H::MODE_PARAM].setValue(0);
	m->params[H::DEGREE_PARAM].setValue(0);
	ms(5);

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
	m->settings.bypass[1] = 0;
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

	printf("-- Persistence\n");
	m->presets[3].chords = 0x55;
	json_t* j = m->dataToJson();
	H* m2 = new H;
	m2->dataFromJson(j);
	expect("Preset survives JSON", m2->presets[3].chords, 0x55);
	json_decref(j);

	printf("\n%s: %d failure(s)\n", failures ? "FAILED" : "PASSED", failures);
	return failures ? 1 : 0;
}
