#include "Harmonion.hpp"

namespace hm {

const char* const CV_FUNC_NAMES[NUM_CV_FUNCS] = {
	"None", "Transpose", "Chaotic Note Shuffle", "Chaotic detune", "Harmonic Shift", "Button Preset",
	"Slew Time", "Voltmeter", "Root", "Degree", "Mode", "Chan 1 Pre Trans", "Chan 2 Pre Trans",
	"Chan 3 Pre Trans", "Arpeggio Pre Trans", "Chan 1 Pitch Bend", "Chan 2 Pitch Bend", "Chan 3 Pitch Bend",
	"Chords Pitch Bend", "Chords Spread", "Chords Voicing", "Arpeggio Clock", "Arpeggio Randomize",
	"Arpeggio Range", "Song", "Song Part 1V/Part", "Song + Part", "Sequence Step",
};
const char* const IN_FUNC_NAMES[NUM_IN_FUNCS] = {
	"None", "Sync Slave", "Chan 1 Slew", "Chan 2 Slew", "Chan 3 Slew", "Chords Slew",
	"Quantize Ch 1", "Quantize Ch 2", "Quantize Ch 3", "Quantize Chords", "Chords Shuffle",
	"Arpeggio Reset", "Seq. Song Reset", "Seq. Part Reset", "Seq. Step Forward", "Seq. Step Backward",
	"Seq. Part Forward", "Seq. Part Backward", "Tuning Mode", "Seq. Play/Pause",
};
const char* const OUT_FUNC_NAMES[NUM_OUT_FUNCS] = {
	"None", "Sync Master", "Chan 1 Autogate", "Chan 2 Autogate", "Chan 3 Autogate", "Ch 2 Manual Gate",
	"Arpeggio Beat", "Sequencer Clock", "Sequencer Beat", "Song Step", "Song Reset",
	"Tuning Mode Active", "Ch 1 A/B State", "Chord Change",
};
const char* const AB_MODE_NAMES[NUM_AB_MODES] = {
	"Off (always A)", "via Jack A/B", "B on Beat 1", "Each Odd Beat", "Each Even Beat",
	"B on 1st Beat of Step", "B on Clock 1", "Each Odd Clock", "Each Even Clock",
};
const char* const DISPLAY_MODE_NAMES[NUM_DISP_MODES] = {"Scale Tones", "Scale Structure", "Chord Structure", "Octave Switches"};
const char* const INTONATION_MODE_NAMES[NUM_INTONATIONS] = {"Equal", "Just", "Quart. C. Meantone"};
const char* const BEND_DELAY_MODE_NAMES[4] = {"None", "Short", "Medium", "Long"};
const char* const PATTERN_NAMES[NUM_PATTERNS] = {"Up", "Down", "Up/Down", "Down/Up", "Baroque", "Octaves", "Brownian", "Random"};
// BEGIN: index 0 is "Next Note" (-1), then the seven tones
const char* const BEGIN_NAMES[8] = {"Next Note", "Root", "3rd", "5th", "7th", "9th", "11th", "13th"};
const char* const INVERSION_NAMES[5] = {"Free", "Root position", "1st inversion", "2nd inversion", "3rd inversion"};

// Fixed by the photos: "(not assigned)" first, Harmonic Shift 6th, Chaotic detune 7th.
// reconstruído: the order of the others
const int CV_MENU_ORDER[NUM_CV_FUNCS] = {
	CV_NONE, CV_ROOT, CV_DEGREE, CV_MODE, CV_TRANSPOSE, CV_HARMONIC_SHIFT, CV_DETUNE, CV_SHUFFLE, CV_PRESET,
	CV_SLEW_TIME, CV_VOLTMETER, CV_PRE_CH1, CV_PRE_CH2, CV_PRE_CH3, CV_BEND_CH1, CV_BEND_CH2, CV_BEND_CH3,
	CV_BEND_CHORDS, CV_SPREAD, CV_VOICING, CV_PRE_ARP, CV_ARP_CLOCK, CV_ARP_RANDOM, CV_ARP_RANGE,
	CV_SONG, CV_PART, CV_SONG_PART, CV_STEP,
};
// Fixed by the photos: Chan 2 Slew 10th, Sync Slave last. reconstruído: the others
const int IN_MENU_ORDER[NUM_IN_FUNCS] = {
	IN_NONE, IN_SONG_RESET, IN_PART_RESET, IN_STEP_FWD, IN_STEP_BACK, IN_PART_FWD, IN_PART_BACK, IN_PLAY,
	IN_SLEW_CH1, IN_SLEW_CH2, IN_SLEW_CH3, IN_SLEW_CHORDS, IN_QUANT_CH1, IN_QUANT_CH2, IN_QUANT_CH3,
	IN_QUANT_CHORDS, IN_CHORDS_SHUFFLE, IN_ARP_RESET, IN_TUNING, IN_SYNC_SLAVE,
};
// Fixed by the photos: Chan 1 Autogate 6th, Sync Master last. reconstruído: the others
const int OUT_MENU_ORDER[NUM_OUT_FUNCS] = {
	OUT_NONE, OUT_SEQ_CLOCK, OUT_SEQ_BEAT, OUT_SONG_STEP, OUT_SONG_RESET, OUT_AUTOGATE_CH1, OUT_AUTOGATE_CH2,
	OUT_AUTOGATE_CH3, OUT_CH2_MANUAL_GATE, OUT_ARP_BEAT, OUT_TUNING, OUT_CH1_AB, OUT_CHORD_CHANGE, OUT_SYNC_MASTER,
};
// Fixed by the photos: nine settings, "Each Odd Beat" 3rd. reconstruído: the others
const int AB_MENU_ORDER[NUM_AB_MODES] = {
	AB_OFF, AB_BEAT1, AB_ODD_BEATS, AB_EVEN_BEATS, AB_STEP_START, AB_CLOCK1, AB_ODD_CLOCKS, AB_EVEN_CLOCKS, AB_JACK,
};
// Order of the eight pattern screens in the manual (page 43)
const int PATTERN_ORDER[NUM_PATTERNS] = {
	PAT_UP, PAT_DOWN, PAT_UPDOWN, PAT_DOWNUP, PAT_OCTAVES, PAT_BAROQUE, PAT_RANDOM, PAT_BROWNIAN,
};

int orderIndex(const int* order, int n, int id) {
	for (int i = 0; i < n; i++)
		if (order[i] == id)
			return i;
	return 0;
}

// Ratio list of version 1 patches, to convert their saved index
static const int OLD_ARP_RATIO[16][2] = {{1, 4}, {1, 3}, {2, 5}, {1, 2}, {2, 3}, {3, 4}, {1, 1}, {5, 4},
	{4, 3}, {3, 2}, {5, 3}, {2, 1}, {5, 2}, {3, 1}, {7, 2}, {4, 1}};

static int nearestArpRatio(float r) {
	int best = ARP_RATIO_UNITY;
	float bestD = 1e9f;
	for (int i = 0; i < NUM_ARP_RATIOS; i++) {
		float d = std::fabs(std::log((float) ARP_RATIO[i][0] / ARP_RATIO[i][1]) - std::log(r));
		if (d < bestD - 1e-6f) {
			bestD = d;
			best = i;
		}
	}
	return best;
}
static const float BEND_DELAY_SEC[4] = {0.f, 0.5f, 1.f, 2.f};
static const float IN_SCALE_DIV[4] = {1.f, 2.f, 3.f, 5.f};
static const float DETUNE_FACTOR[8] = {1.f, -0.8f, 0.6f, -1.f, 0.9f, -0.5f, 0.3f, -0.7f};
static const int SHUFFLE_SWAPS[6][2] = {{0, 7}, {2, 9}, {4, 11}, {5, 1}, {10, 3}, {8, 6}};
static const uint16_t FILLS = 0x1F << FILL1;
static const uint16_t TONES = 0x7F;


// Output index of the eight pitch outputs: Ch1-3, Arpeggio, Chords 1-4
static const int OUT_SECTION[8] = {SEC_CH1, SEC_CH2, SEC_CH3, SEC_ARP, SEC_CHORDS, SEC_CHORDS, SEC_CHORDS, SEC_CHORDS};

static json_t* intArray(const int* v, int n) {
	json_t* a = json_array();
	for (int i = 0; i < n; i++)
		json_array_append_new(a, json_integer(v[i]));
	return a;
}
static void readIntArray(json_t* a, int* v, int n) {
	for (int i = 0; i < n && a && i < (int) json_array_size(a); i++)
		v[i] = json_integer_value(json_array_get(a, i));
}
static int jint(json_t* j, const char* key, int def) {
	json_t* v = json_object_get(j, key);
	return v ? (int) json_integer_value(v) : def;
}
static bool jbool(json_t* j, const char* key, bool def) {
	json_t* v = json_object_get(j, key);
	return v ? json_is_true(v) : def;
}

Preset Preset::defaults() {
	Preset p;
	p.ch1A = (1 << T1) | (1 << T3) | (1 << T5) | (1 << T7);
	p.ch2 = TONES;
	p.ch3 = (1 << T1) | (1 << T3) | (1 << T5);
	p.chords = (1 << T1) | (1 << T3) | (1 << T5) | (1 << T7);
	p.arp = (1 << T1) | (1 << T3) | (1 << T5) | (1 << T7);
	return p;
}

json_t* Preset::toJson() const {
	json_t* j = json_object();
	json_object_set_new(j, "ch1A", json_integer(ch1A));
	json_object_set_new(j, "ch1Fill", json_boolean(ch1Fill));
	json_object_set_new(j, "ch1B", json_integer(ch1B));
	json_object_set_new(j, "ch1Slew", json_boolean(ch1Slew));
	json_object_set_new(j, "ch2", json_integer(ch2));
	json_object_set_new(j, "ch3", json_integer(ch3));
	json_object_set_new(j, "eqDist", json_boolean(eqDist));
	json_object_set_new(j, "chords", json_integer(chords));
	json_object_set_new(j, "inv", json_integer(inversion));
	json_object_set_new(j, "chordSlew", json_boolean(chordSlew));
	json_object_set_new(j, "arp", json_integer(arp));
	json_object_set_new(j, "pattern", json_integer(pattern));
	json_object_set_new(j, "begin", json_integer(begin));
	json_object_set_new(j, "gate1", json_boolean(gate[0]));
	json_object_set_new(j, "gate2", json_boolean(gate[1]));
	int oct[NUM_SECTIONS];
	for (int i = 0; i < NUM_SECTIONS; i++)
		oct[i] = octave[i];
	json_object_set_new(j, "oct", intArray(oct, NUM_SECTIONS));
	return j;
}

void Preset::fromJson(json_t* j) {
	if (!j)
		return;
	ch1A = jint(j, "ch1A", ch1A);
	ch1Fill = jbool(j, "ch1Fill", ch1Fill);
	ch1B = jint(j, "ch1B", ch1B);
	ch1Slew = jbool(j, "ch1Slew", ch1Slew);
	ch2 = jint(j, "ch2", ch2);
	ch3 = jint(j, "ch3", ch3);
	eqDist = jbool(j, "eqDist", eqDist);
	chords = jint(j, "chords", chords);
	inversion = jint(j, "inv", inversion);
	chordSlew = jbool(j, "chordSlew", chordSlew);
	arp = jint(j, "arp", arp);
	pattern = jint(j, "pattern", pattern);
	begin = jint(j, "begin", begin);
	gate[0] = jbool(j, "gate1", gate[0]);
	gate[1] = jbool(j, "gate2", gate[1]);
	int oct[NUM_SECTIONS] = {};
	readIntArray(json_object_get(j, "oct"), oct, NUM_SECTIONS);
	for (int i = 0; i < NUM_SECTIONS; i++)
		octave[i] = clamp(oct[i], -3, 3);
}

json_t* Step::toJson() const {
	json_t* j = json_object();
	json_object_set_new(j, "root", json_integer(root));
	json_object_set_new(j, "mode", json_integer(mode));
	json_object_set_new(j, "degree", json_integer(degree));
	json_object_set_new(j, "beats", json_integer(beats));
	json_object_set_new(j, "preset", preset.toJson());
	return j;
}

void Step::fromJson(json_t* j) {
	root = jint(j, "root", 0);
	mode = jint(j, "mode", 0);
	degree = jint(j, "degree", 0);
	beats = clamp(jint(j, "beats", 4), 1, MAX_BEATS);
	preset.fromJson(json_object_get(j, "preset"));
}

json_t* Settings::toJson() const {
	json_t* j = json_object();
	json_object_set_new(j, "display", json_integer(display));
	json_object_set_new(j, "cvFunc", intArray(cvFunc, 4));
	json_object_set_new(j, "potFunc", intArray(potFunc, 2));
	json_object_set_new(j, "inFunc", intArray(inFunc, 2));
	json_object_set_new(j, "outFunc", intArray(outFunc, 2));
	json_object_set_new(j, "nonSeqClockDiv", json_integer(nonSeqClockDiv));
	json_object_set_new(j, "arpClock", json_integer(arpClock));
	int ratio[2] = {ARP_RATIO[clamp(arpClock, 0, NUM_ARP_RATIOS - 1)][0], ARP_RATIO[clamp(arpClock, 0, NUM_ARP_RATIOS - 1)][1]};
	json_object_set_new(j, "arpClockRatio", intArray(ratio, 2));
	json_object_set_new(j, "arpDelayMs", json_integer(arpDelayMs));
	json_object_set_new(j, "abMode", json_integer(abMode));
	json_object_set_new(j, "ch2Single", json_boolean(ch2Single));
	json_object_set_new(j, "trigWindowMs", json_integer(trigWindowMs));
	json_object_set_new(j, "slewExp", json_boolean(slewExp));
	json_object_set_new(j, "intonation", json_integer(intonation));
	json_object_set_new(j, "bendDelay", json_integer(bendDelay));
	json_object_set_new(j, "bypass", intArray(bypass, 3));
	int f[NUM_SECTIONS], g[3], mix[16];
	for (int i = 0; i < NUM_SECTIONS; i++)
		f[i] = follow[i];
	for (int i = 0; i < 3; i++)
		g[i] = glide[i];
	for (int i = 0; i < 16; i++)
		mix[i] = inMix[i / 4][i % 4];
	json_object_set_new(j, "follow", intArray(f, NUM_SECTIONS));
	json_object_set_new(j, "inScale", intArray(inScale, NUM_SECTIONS));
	json_object_set_new(j, "glide", intArray(g, 3));
	json_object_set_new(j, "inMix", intArray(mix, 16));
	json_object_set_new(j, "outSrc", intArray(outSrc, 4));
	json_object_set_new(j, "liveMode", json_boolean(liveMode));
	return j;
}

void Settings::fromJson(json_t* j) {
	if (!j)
		return;
	display = jint(j, "display", display);
	readIntArray(json_object_get(j, "cvFunc"), cvFunc, 4);
	readIntArray(json_object_get(j, "potFunc"), potFunc, 2);
	readIntArray(json_object_get(j, "inFunc"), inFunc, 2);
	readIntArray(json_object_get(j, "outFunc"), outFunc, 2);
	nonSeqClockDiv = jint(j, "nonSeqClockDiv", nonSeqClockDiv);
	json_t* ratio = json_object_get(j, "arpClockRatio");
	if (ratio) {
		int r[2] = {1, 1};
		readIntArray(ratio, r, 2);
		arpClock = nearestArpRatio((float) std::max(r[0], 1) / std::max(r[1], 1));
	}
	else if (json_object_get(j, "arpClock")) {
		// Patches of version 1 saved an index into a slightly different list
		int old = clamp(jint(j, "arpClock", 6), 0, 15);
		arpClock = nearestArpRatio((float) OLD_ARP_RATIO[old][0] / OLD_ARP_RATIO[old][1]);
	}
	arpDelayMs = clamp(jint(j, "arpDelayMs", arpDelayMs), 0, MAX_ARP_DELAY_MS);
	abMode = clamp(jint(j, "abMode", abMode), 0, NUM_AB_MODES - 1);
	ch2Single = jbool(j, "ch2Single", ch2Single);
	trigWindowMs = clamp(jint(j, "trigWindowMs", trigWindowMs), 0, MAX_TRIG_WINDOW_MS);
	slewExp = jbool(j, "slewExp", slewExp);
	intonation = jint(j, "intonation", intonation);
	bendDelay = jint(j, "bendDelay", bendDelay);
	readIntArray(json_object_get(j, "bypass"), bypass, 3);
	int f[NUM_SECTIONS] = {}, g[3] = {}, mix[16] = {};
	for (int i = 0; i < 16; i++)
		mix[i] = inMix[i / 4][i % 4];
	readIntArray(json_object_get(j, "follow"), f, NUM_SECTIONS);
	readIntArray(json_object_get(j, "inScale"), inScale, NUM_SECTIONS);
	readIntArray(json_object_get(j, "glide"), g, 3);
	readIntArray(json_object_get(j, "inMix"), mix, 16);
	readIntArray(json_object_get(j, "outSrc"), outSrc, 4);
	for (int i = 0; i < NUM_SECTIONS; i++)
		follow[i] = f[i];
	for (int i = 0; i < 3; i++)
		glide[i] = g[i];
	for (int i = 0; i < 16; i++)
		inMix[i / 4][i % 4] = mix[i];
	liveMode = jbool(j, "liveMode", liveMode);
}

float Slew::process(float in, float time, bool exponential, float dt) {
	if (!init || time <= 0.f) {
		init = true;
		value = start = target = in;
		return value;
	}
	if (in != target) {
		start = value;
		target = in;
	}
	if (exponential) {
		value += (target - value) * (1.f - std::exp(-dt * 4.f / time));
	}
	else {
		// Linear: every glide takes the same time regardless of the distance
		float rate = std::max(std::fabs(target - start), 1e-6f) / time;
		float d = target - value;
		float stepSize = rate * dt;
		value = (std::fabs(d) <= stepSize) ? target : value + (d > 0 ? stepSize : -stepSize);
	}
	return value;
}

static const char* const NOTE_LABELS[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

Harmonion::Harmonion() {
	config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	static const char* const toneNames[7] = {"ROOT", "3RD", "5TH", "7TH", "9TH", "11TH", "13TH"};
	static const char* const fillNames[5] = {"FILL 1", "FILL 2", "FILL 3", "FILL 4", "FILL 5"};
	for (int t = 0; t < 7; t++) {
		configButton(BIG_PARAM + C1A_TONE[t], std::string("Channel 1 A ") + toneNames[t]);
		configButton(BIG_PARAM + C2_FUNC[t], std::string("Channel 2 ") + toneNames[t]);
		configButton(BIG_PARAM + C3_TONE[t], std::string("Channel 3 ") + toneNames[t]);
		configButton(BIG_PARAM + CHORD_BTN + t, std::string("Chords ") + toneNames[t]);
		configButton(BIG_PARAM + ARP_BTN + t, std::string("Arpeggio ") + toneNames[t]);
	}
	for (int f = 0; f < 5; f++)
		configButton(BIG_PARAM + C2_FUNC[FILL1 + f], std::string("Channel 2 ") + fillNames[f]);
	configButton(BIG_PARAM + C1A_FILL, "Channel 1 A FILL");
	configButton(BIG_PARAM + C1B_ROOT, "Channel 1 B ROOT");
	configButton(BIG_PARAM + C1B_5TH, "Channel 1 B 5TH");
	configButton(BIG_PARAM + C1B_SCALE, "Channel 1 B SCALE");
	configButton(BIG_PARAM + C1_SLEW, "Channel 1 SLEW");
	configButton(BIG_PARAM + C3_EQDIST, "Channel 3 EQ-DIST");
	configButton(BIG_PARAM + CHORD_BTN + CB_INV, "Chords INV (inversion)");
	configButton(BIG_PARAM + CHORD_BTN + CB_SLEW, "Chords SLEW");
	configButton(BIG_PARAM + ARP_BTN + AB_PATTERN, "Arpeggio PATTERN");
	configButton(BIG_PARAM + ARP_BTN + AB_BEGIN, "Arpeggio BEGIN");
	configButton(BIG_PARAM + GATE_BTN, "GATE 1");
	configButton(BIG_PARAM + GATE_BTN + 1, "GATE 2");

	configButton(MENU_PARAM, "MENU");
	configButton(TUNE_PARAM, "TUNE");
	configButton(SONG_PARAM, "SONG");
	configButton(OCT_PARAM, "OCT");
	configButton(SEQ_PARAM, "SEQ");
	configButton(LEFT_PARAM, "Previous (step / menu item)");
	configButton(RIGHT_PARAM, "Next (step / menu item)");
	configButton(PLAY_PARAM, "Play / Pause (double click: freeze)");
	configButton(DEL_PARAM, "DEL");
	configButton(MINUS_PARAM, "- BEATS / value");
	configButton(PLUS_PARAM, "+ BEATS / value");
	configButton(ADD_PARAM, "ADD");

	configSwitch(ROOT_PARAM, 0.f, 11.f, 0.f, "Root", {NOTE_LABELS, NOTE_LABELS + 12});
	configParam(DEGREE_PARAM, 0.f, 11.f, 0.f, "Degree", "", 0.f, 1.f, 1.f);
	paramQuantities[DEGREE_PARAM]->snapEnabled = true;
	configParam(TRANS_PARAM, -12.f, 12.f, 0.f, "Transpose", " semitones");
	paramQuantities[TRANS_PARAM]->snapEnabled = true;
	std::vector<std::string> modeNames;
	for (const Mode& m : modes())
		modeNames.push_back(m.name);
	configSwitch(MODE_PARAM, 0.f, modes().size() - 1, 0.f, "Mode", modeNames);
	configParam(SLEW_PARAM, 0.f, 1.f, 0.f, "Slew");
	configParam(PRESET_PARAM, 0.f, 15.f, 0.f, "Preset / Song", "", 0.f, 1.f, 1.f);
	paramQuantities[PRESET_PARAM]->snapEnabled = true;
	configParam(POT1_PARAM, 0.f, 1.f, 0.f, "POT 1 (assignable)");
	configParam(POT2_PARAM, 0.f, 1.f, 0.f, "POT 2 (assignable)");
	configParam(CHORD_PITCH_PARAM, 0.f, 4.f, 2.f, "Chords pitch", " V");
	configParam(SPREAD_PARAM, 0.f, 24.f, 0.f, "Chords spread", " semitones");
	configParam(ARP_PITCH_PARAM, 0.f, 4.f, 2.f, "Arpeggio pitch", " V");
	configParam(RANGE_PARAM, 0.f, 60.f, 12.f, "Arpeggio range", " semitones");
	paramQuantities[RANGE_PARAM]->snapEnabled = true;

	for (int i = 0; i < 3; i++) {
		configInput(CH_IN_INPUT + i, string::f("Channel %d pitch", i + 1));
		configInput(CH_TRIG_INPUT + i, string::f("Channel %d trigger (sample & hold)", i + 1));
		configOutput(CH_OUTPUT + i, string::f("Channel %d pitch", i + 1));
	}
	configInput(AB_INPUT, "Channel 1 A/B");
	configInput(ROOT_INPUT, "Root CV");
	configInput(TRANS_INPUT, "Transpose CV (semitone steps)");
	for (int i = 0; i < 4; i++)
		configInput(CV_INPUT + i, string::f("CV %d (assignable)", i + 1));
	for (int i = 0; i < 2; i++) {
		configInput(IN_INPUT + i, string::f("IN %d (assignable gate)", i + 1));
		configOutput(OUT_OUTPUT + i, string::f("OUT %d (assignable gate)", i + 1));
		configOutput(GATE_OUTPUT + i, string::f("Gate %d", i + 1));
	}
	configInput(CHORD_IN_INPUT, "Chords pitch");
	configInput(CHORD_TRIG_INPUT, "Chords trigger (sample & hold)");
	configInput(ARP_IN_INPUT, "Arpeggio pitch");
	configInput(ARP_CLK_INPUT, "Arpeggio clock (normalled to sequencer clock)");
	configInput(SEQ_CLK_INPUT, "Sequencer clock");
	for (int i = 0; i < 4; i++)
		configOutput(CHORD_OUTPUT + i, string::f("Chords voice %d", i + 1));
	configOutput(ARP_OUTPUT, "Arpeggio pitch");

	controlDivider.setDivision(4);
	lightDivider.setDivision(128);
	rng.seed(random::u64(), random::u64());
	onReset();
}

void Harmonion::onReset() {
	factoryReset();
}

void Harmonion::resetSettings() {
	// Menu settings and routing matrices go back to defaults; channel options stay
	Settings s;
	for (int i = 0; i < 3; i++) {
		s.bypass[i] = settings.bypass[i];
		s.glide[i] = settings.glide[i];
	}
	for (int i = 0; i < NUM_SECTIONS; i++) {
		s.follow[i] = settings.follow[i];
		s.inScale[i] = settings.inScale[i];
	}
	s.liveMode = settings.liveMode;
	settings = s;
}

void Harmonion::factoryReset() {
	// The original restarts after a factory reset and shows its start screen
	splashUntil = time + 2.0;
	knobsInit = false;
	settings = Settings();
	for (int i = 0; i < NUM_PRESETS; i++)
		presets[i] = Preset::defaults();
	livePreset = Preset::defaults();
	for (int s = 0; s < NUM_SONGS; s++)
		for (int p = 0; p < NUM_PARTS; p++)
			songs[s][p] = Part();
	seqMode = false;
	playing = true;
	song = part = step = 0;
	beat = 0;
	clockIdx = 0;
	waitFirstClock = true;
	frozen = false;
	pendingPart = -1;
	page = PAGE_MAIN;
	menuOpen = false;
	tuning = false;
	calibrating = false;
	lastRootKnob = lastDegreeKnob = lastModeKnob = lastSongKnob = -1;
}

Part& Harmonion::currentPart() {
	return songs[clamp(song, 0, NUM_SONGS - 1)][clamp(part, 0, NUM_PARTS - 1)];
}

Step& Harmonion::currentStep() {
	Part& p = currentPart();
	step = clamp(step, 0, (int) p.steps.size() - 1);
	return p.steps[step];
}

Preset& Harmonion::editPreset() {
	if (seqMode && !syncSlave) {
		if (settings.liveMode)
			return livePreset;
		return currentStep().preset;
	}
	return presets[clamp(presetIndex, 0, NUM_PRESETS - 1)];
}

const Preset& Harmonion::audiblePreset() {
	if (frozen)
		return frozenPreset;
	return editPreset();
}

bool Harmonion::potAttenuates(int k) {
	int f = settings.potFunc[clamp(k, 0, 1)];
	if (f <= CV_NONE || f >= NUM_CV_FUNCS)
		return false;
	for (int i = 0; i < 4; i++)
		if (settings.cvFunc[i] == f)
			return true;
	return false;
}

bool Harmonion::isBipolar(int f) {
	switch (f) {
		case CV_TRANSPOSE: case CV_DETUNE: case CV_PRE_CH1: case CV_PRE_CH2: case CV_PRE_CH3: case CV_PRE_ARP:
		case CV_BEND_CH1: case CV_BEND_CH2: case CV_BEND_CH3: case CV_BEND_CHORDS: case CV_ARP_CLOCK:
			return true;
		default:
			return false;
	}
}

// Voltage that an assignable pot produces for a function when no CV is assigned to it
float Harmonion::potVoltage(int f, float pot) {
	switch (f) {
		case CV_TRANSPOSE: return (pot * 2.f - 1.f) * 2.f;
		case CV_DETUNE: case CV_BEND_CH1: case CV_BEND_CH2: case CV_BEND_CH3: case CV_BEND_CHORDS:
			return (pot * 2.f - 1.f) * 5.f;
		case CV_PRE_CH1: case CV_PRE_CH2: case CV_PRE_CH3: case CV_PRE_ARP:
			return (pot * 2.f - 1.f) * 1.f;
		case CV_ARP_CLOCK: return (pot * 2.f - 1.f) * 8.f / 12.f;
		case CV_PRESET: case CV_SONG: return pot * 15.f / 12.f;
		case CV_ROOT: case CV_DEGREE: return pot * 11.f / 12.f;
		case CV_MODE: return pot * (modes().size() - 1) / 12.f;
		case CV_VOICING: return pot * 98.f / 12.f;
		case CV_PART: return pot * 4.f;
		case CV_SONG_PART: return pot * 64.f / 12.f;
		case CV_STEP: return pot * 31.f / 12.f;
		default: return pot * 5.f;
	}
}

static int semis(float v) {
	return (int) std::round(v * 12.f);
}

void Harmonion::readKnobs() {
	int rk = (int) std::round(params[ROOT_PARAM].getValue());
	int dk = (int) std::round(params[DEGREE_PARAM].getValue());
	int mk = (int) std::round(params[MODE_PARAM].getValue());
	int pk = (int) std::round(params[PRESET_PARAM].getValue());
	int numModes = modes().size();

	if (seqMode && !syncSlave) {
		// In sequenced mode the knobs behave like encoders and edit the current step
		Step& s = currentStep();
		if (lastRootKnob >= 0 && rk != lastRootKnob)
			s.root = wrap(s.root + rk - lastRootKnob, 12);
		if (lastModeKnob >= 0 && mk != lastModeKnob) {
			s.mode = wrap(s.mode + mk - lastModeKnob, numModes);
			s.degree = std::min<int>(s.degree, modes()[s.mode].degrees.size() - 1);
		}
		if (lastDegreeKnob >= 0 && dk != lastDegreeKnob)
			s.degree = wrap(s.degree + dk - lastDegreeKnob, modes()[s.mode].degrees.size());
		// PRESET selects the song
		// Only a change of knob or CV selects another song, so other song CVs can override it
		int sel = pk + (funcActive[CV_SONG] ? semis(funcCv[CV_SONG]) : 0);
		sel = wrap(sel, NUM_SONGS);
		bool changed = sel != lastSongCv;
		lastSongCv = sel;
		if (changed && sel != song) {
			song = sel;
			part = 0;
			step = 0;
			beat = 0;
			pendingPart = -1;
			waitFirstClock = true;
			onStepChange();
		}
	}
	else {
		nsRoot = rk;
		nsMode = clamp(mk, 0, numModes - 1);
		nsDegree = clamp(dk, 0, (int) modes()[nsMode].degrees.size() - 1);
		int sel = pk + (funcActive[CV_PRESET] ? semis(funcCv[CV_PRESET]) : 0);
		presetIndex = wrap(sel, NUM_PRESETS);
	}
	lastRootKnob = rk;
	lastDegreeKnob = dk;
	lastModeKnob = mk;
	lastSongKnob = pk;

	transSemis = (int) std::round(params[TRANS_PARAM].getValue());
	if (syncSlave && syncAlive)
		transSemis = (int) std::round(syncTrans);
	transSemis += semis(clamp(inputs[TRANS_INPUT].getVoltage(), -10.f, 10.f));
}

void Harmonion::updateHarmony() {
	int r, m, d;
	if (syncSlave) {
		r = (int) std::round(syncRoot);
		m = (int) std::round(syncMode);
		d = (int) std::round(syncDegree);
	}
	else if (seqMode) {
		Step& s = currentStep();
		r = s.root;
		m = s.mode;
		d = s.degree;
	}
	else {
		r = nsRoot;
		m = nsMode;
		d = nsDegree;
	}
	editHarmony.set(r, m, d);
	if (frozen && !syncSlave) {
		r = frozenRoot;
		m = frozenMode;
		d = frozenDegree;
	}
	// CV modifications (wrap around)
	r += semis(inputs[ROOT_INPUT].getVoltage());
	if (funcActive[CV_ROOT])
		r += semis(funcCv[CV_ROOT]);
	if (funcActive[CV_MODE])
		m = wrap(m + semis(funcCv[CV_MODE]), modes().size());
	m = wrap(m, modes().size());
	if (funcActive[CV_DEGREE])
		d += semis(funcCv[CV_DEGREE]);
	harmony.set(r, m, d);

	int key = harmony.keyRoot * 10000 + harmony.mode * 100 + harmony.degree;
	if (key != lastChordKey) {
		if (lastChordKey >= 0) {
			outPulse[OUT_CHORD_CHANGE].trigger();
			if (settings.bendDelay > 0)
				bendFade = 0.f;
			// Non-sequenced mode: BEGIN restarts the arpeggio on every chord change
			if (!seqMode && audiblePreset().begin >= 0) {
				arpResetPending = true;
				arpBeginPending = true;
			}
		}
		lastChordKey = key;
	}
}

void Harmonion::onStepChange() {
	step = clamp(step, 0, (int) currentPart().steps.size() - 1);
	// Sequenced mode: BEGIN restarts the arpeggio on every step, even with the same chord
	if (audiblePreset().begin >= 0) {
		arpResetPending = true;
		arpBeginPending = true;
	}
	if (settings.bendDelay > 0)
		bendFade = 0.f;
}

void Harmonion::seqReset(bool toPartA) {
	if (toPartA)
		part = 0;
	step = 0;
	beat = 0;
	pendingPart = -1;
	// The reset counts as the first clock; clocks arriving within 5 ms are ignored
	int div = CLOCK_DIVS[clamp(currentPart().clockDiv, 0, NUM_CLOCK_DIVS - 1)];
	clockIdx = 1 % div;
	waitFirstClock = false;
	ignoreClockUntil = time + 0.005;
	outPulse[OUT_SONG_RESET].trigger();
	outPulse[OUT_SONG_STEP].trigger();
	onStepChange();
}

void Harmonion::gotoPart(int p) {
	part = wrap(p, NUM_PARTS);
	step = 0;
	beat = 0;
	pendingPart = -1;
	waitFirstClock = true;
	outPulse[OUT_SONG_STEP].trigger();
	onStepChange();
}

void Harmonion::seqAdvance(int delta, bool manual) {
	int n = currentPart().steps.size();
	int s = step + delta;
	if (s >= n) {
		s = 0;
		outPulse[OUT_SONG_RESET].trigger();
		if (pendingPart >= 0) {
			part = pendingPart;
			pendingPart = -1;
		}
	}
	if (s < 0)
		s = n - 1;
	step = s;
	beat = 0;
	if (manual && playing)
		waitFirstClock = true;
	outPulse[OUT_SONG_STEP].trigger();
	onStepChange();
}

void Harmonion::seqBeat() {
	outPulse[OUT_SEQ_BEAT].trigger();
	if (!seqMode || syncSlave || !playing || frozen)
		return;
	if (waitFirstClock) {
		waitFirstClock = false;
		return;
	}
	bool stepFwdMapped = settings.inFunc[0] == IN_STEP_FWD || settings.inFunc[1] == IN_STEP_FWD;
	if (stepFwdMapped)
		return;
	beat++;
	if (beat >= currentStep().beats)
		seqAdvance(1, false);
}

void Harmonion::seqClock() {
	outPulse[OUT_SEQ_CLOCK].trigger();
	clkLight[1].trigger(0.05f);
	if (time < ignoreClockUntil)
		return;
	if (seqMode && !syncSlave) {
		int div = CLOCK_DIVS[clamp(currentPart().clockDiv, 0, NUM_CLOCK_DIVS - 1)];
		if (clockIdx % div == 0)
			seqBeat();
		clockIdx = (clockIdx + 1) % div;
	}
	else {
		// Without the sequencer the clock is only counted for the A/B switch (4 beats per bar)
		int div = CLOCK_DIVS[clamp(settings.nonSeqClockDiv, 0, NUM_CLOCK_DIVS - 1)];
		if (nonSeqClock % div == 0) {
			nonSeqBeat = (nonSeqBeat + 1) % 4;
			outPulse[OUT_SEQ_BEAT].trigger();
		}
		nonSeqClock = (nonSeqClock + 1) % div;
	}
}

uint16_t Harmonion::applyHarmonicShift(uint16_t funcs) {
	if (!funcActive[CV_HARMONIC_SHIFT])
		return funcs;
	float v = funcCv[CV_HARMONIC_SHIFT];
	int level = clamp((int) std::floor(std::fabs(v) / (5.f / 7.f) + 1e-3f), 0, 7);
	static const uint16_t up[7] = {FILLS, 1 << T11, 1 << T13, 1 << T9, 1 << T7, 1 << T3, 1 << T5};
	static const uint16_t down[7] = {1 << T1, 1 << T5, 1 << T3, 1 << T7, 1 << T9, 1 << T13, 1 << T11};
	const uint16_t* order = v >= 0.f ? up : down;
	for (int i = 0; i < level; i++)
		funcs &= ~order[i];
	return funcs;
}

uint16_t Harmonion::sectionFuncs(int sec, const Preset& p) {
	uint16_t f = 0;
	switch (sec) {
		case SEC_CH1: {
			uint16_t a = p.ch1A | (p.ch1Fill ? FILLS : 0);
			uint16_t b = ((p.ch1B & 1) ? (1 << T1) : 0) | ((p.ch1B & 2) ? (1 << T5) : 0) | ((p.ch1B & 4) ? TONES : 0);
			// If nothing is selected on one side, the other side is used
			bool useB = ch[0].usingB ? (b != 0) : (a == 0 && b != 0);
			f = useB ? b : a;
		} break;
		case SEC_CH2: f = p.ch2; break;
		case SEC_CH3: f = p.ch3; break;
		case SEC_CHORDS: f = p.chords; break;
		case SEC_ARP: f = p.arp; break;
	}
	f = applyHarmonicShift(f);
	if (!f)
		f = 1 << T1;
	return f;
}

float Harmonion::followShift(int sec) {
	if (!settings.follow[sec])
		return 0.f;
	// Shift by the distance of the current root from C, or from the first chord of the song part
	int ref = 0;
	if (seqMode && !syncSlave) {
		const Step& first = currentPart().steps[0];
		Harmony h;
		h.set(first.root, first.mode, first.degree);
		ref = h.chordRoot;
	}
	int shift = wrap(harmony.chordRoot - ref, 12);
	if (shift >= 7)
		shift -= 12;
	return shift / 12.f;
}

// Pitch after the input scaling and the input mixing matrix (the start of the manual's flow chart)
float Harmonion::mixedInput(int sec) {
	if (sec == SEC_CHORDS)
		return inputs[CHORD_IN_INPUT].getVoltage() / IN_SCALE_DIV[clamp(settings.inScale[SEC_CHORDS], 0, 3)];
	// Each IN jack is scaled by the setting of its own section before it enters the matrix.
	// Rows are channels 1-3 and arpeggio, columns are the IN jacks.
	static const int jack[4] = {CH_IN_INPUT + 0, CH_IN_INPUT + 1, CH_IN_INPUT + 2, ARP_IN_INPUT};
	static const int jackSec[4] = {SEC_CH1, SEC_CH2, SEC_CH3, SEC_ARP};
	int row = (sec == SEC_ARP) ? 3 : sec;
	float v = 0.f;
	int n = 0;
	for (int j = 0; j < 4; j++) {
		if (settings.inMix[row][j]) {
			v += inputs[jack[j]].getVoltage() / IN_SCALE_DIV[clamp(settings.inScale[jackSec[j]], 0, 3)];
			n++;
		}
	}
	// Every additional jack subtracts two octaves, so C3 + C3 is C4
	if (n > 1)
		v -= 2.f * (n - 1);
	return v;
}

// Input pitch of the chords and the arpeggio: mixed input, IN follows ROOT and Pre Trans
float Harmonion::inputFor(int sec) {
	float v = mixedInput(sec) + followShift(sec);
	static const int pre[NUM_SECTIONS] = {CV_PRE_CH1, CV_PRE_CH2, CV_PRE_CH3, CV_NONE, CV_PRE_ARP};
	if (pre[sec] != CV_NONE && funcActive[pre[sec]])
		v += funcCv[pre[sec]];
	return v;
}

float Harmonion::toVolts(int n) {
	if (settings.intonation == EQUAL)
		return n / 12.f;
	int root = harmony.chordRoot;
	int d = n - root;
	int oct = (int) std::floor(d / 12.f);
	return (root + 12 * oct) / 12.f + intonationCents(settings.intonation, d - 12 * oct) / 1200.f;
}

int Harmonion::shuffleNote(int n) {
	if (!funcActive[CV_SHUFFLE])
		return n;
	int k = clamp((int) std::round(funcCv[CV_SHUFFLE] / 5.f * 6.f), 0, 6);
	int perm[12];
	for (int i = 0; i < 12; i++)
		perm[i] = i;
	for (int i = 0; i < k; i++)
		std::swap(perm[SHUFFLE_SWAPS[i][0]], perm[SHUFFLE_SWAPS[i][1]]);
	int pc = wrap(n, 12);
	return n - pc + perm[pc];
}

static float slewSeconds(float knob) {
	if (knob < 0.005f)
		return 0.f;
	return 0.005f * std::pow(400.f, knob);
}

float Harmonion::slewTime() {
	float t = slewSeconds(params[SLEW_PARAM].getValue());
	if (funcActive[CV_SLEW_TIME])
		t *= std::max(0.f, funcCv[CV_SLEW_TIME] / 2.5f);
	return t;
}

float Harmonion::postProcess(int out, float v, int sec, bool slewOn, float dt) {
	Slew* slew = (out < 3) ? &ch[out].slew : (out == 3) ? &arpSlew : &chordSlew[out - 4];
	v += transSemis / 12.f + audiblePreset().octave[sec];
	float t = slewOn ? slewTime() : 0.f;
	v = slew->process(v, t, settings.slewExp, dt);
	float detune = (funcActive[CV_DETUNE] ? funcCv[CV_DETUNE] : 0.f) + (syncSlave && syncAlive ? syncDetune : 0.f);
	v += DETUNE_FACTOR[out] * clamp(detune, -5.f, 5.f) / 5.f * 2.f / 12.f;
	if (funcActive[CV_TRANSPOSE])
		v += clamp(funcCv[CV_TRANSPOSE], -10.f, 10.f);
	static const int bend[NUM_SECTIONS] = {CV_BEND_CH1, CV_BEND_CH2, CV_BEND_CH3, CV_BEND_CHORDS, CV_NONE};
	if (bend[sec] != CV_NONE && funcActive[bend[sec]]) {
		float b = funcCv[bend[sec]] / 5.f * 2.f / 12.f;
		if (sec == SEC_CHORDS)
			b *= bendFade;
		v += b;
	}
	return v;
}

// Nearest allowed semitone with a little hysteresis around the previous note
static int quantizeNearest(float t, uint16_t mask, int prev, bool hasPrev) {
	int best = 0;
	float bestD = 1e9f;
	int base = (int) std::floor(t);
	for (int n = base - 12; n <= base + 13; n++) {
		if (!(mask & (1 << wrap(n, 12))))
			continue;
		float d = std::fabs(n - t);
		if (d < bestD) {
			bestD = d;
			best = n;
		}
	}
	if (hasPrev && (mask & (1 << wrap(prev, 12))) && std::fabs(prev - t) < bestD + 0.1f)
		return prev;
	return best;
}

static int funcOfPitchIn(const Harmony& h, int n, uint16_t funcs) {
	int pc = wrap(n, 12);
	for (int f = 0; f < NUM_FUNCS; f++)
		if ((funcs & (1 << f)) && (h.notes[f][0] == pc || h.notes[f][1] == pc))
			return f;
	return h.funcOfPitch(pc);
}

void Harmonion::processChannel(int c, float dt) {
	ChannelState& s = ch[c];
	const Preset& p = audiblePreset();
	// Flow chart of the manual: input matrix -> IN follows ROOT -> quantize to semitone (or EQ-DIST)
	// -> + Pre Trans -> sample & hold -> nearest allowed note
	float raw = mixedInput(c);
	float in = raw + followShift(c);
	static const int pre[3] = {CV_PRE_CH1, CV_PRE_CH2, CV_PRE_CH3};
	float preTrans = funcActive[pre[c]] ? funcCv[pre[c]] : 0.f;

	// A/B switch of channel 1
	if (c == 0) {
		bool seq = seqMode && !syncSlave;
		int beatInBar = seq ? beat : nonSeqBeat;
		int div = seq ? CLOCK_DIVS[clamp(currentPart().clockDiv, 0, NUM_CLOCK_DIVS - 1)]
		              : CLOCK_DIVS[clamp(settings.nonSeqClockDiv, 0, NUM_CLOCK_DIVS - 1)];
		int clockPos = wrap((seq ? clockIdx : nonSeqClock) - 1, div);
		switch (settings.abMode) {
			case AB_JACK: s.usingB = inputs[AB_INPUT].getVoltage() > 3.f; break;
			case AB_BEAT1: s.usingB = beatInBar % 4 == 0; break;
			case AB_ODD_BEATS: s.usingB = beatInBar % 2 == 0; break;
			case AB_EVEN_BEATS: s.usingB = beatInBar % 2 == 1; break;
			case AB_STEP_START: s.usingB = seq ? beat == 0 : beatInBar % 4 == 0; break;
			case AB_CLOCK1: s.usingB = clockPos == 0; break;
			case AB_ODD_CLOCKS: s.usingB = clockPos % 2 == 0; break;
			case AB_EVEN_CLOCKS: s.usingB = clockPos % 2 == 1; break;
			default: s.usingB = false; break;
		}
	}

	uint16_t funcs = sectionFuncs(c, p);
	uint16_t mask = harmony.pitchMask(funcs);
	if (!mask)
		mask = 1 << harmony.chordRoot;

	// Glide detection: a moving input is passed through continuously
	if (settings.glide[c]) {
		if (std::fabs(in - s.lastIn) > 1e-5f) {
			s.moveTicks++;
			s.stillTime = 0.f;
		}
		else {
			s.stillTime += dt;
		}
		if (s.moveTicks >= 3)
			s.gliding = true;
		if (s.stillTime > 0.005f) {
			s.gliding = false;
			s.moveTicks = 0;
		}
	}
	else {
		s.gliding = false;
	}
	s.lastIn = in;

	int bypass = settings.bypass[c];
	if (bypass == 2) {
		// True bypass: the pitch after the input matrix without any modification
		pitchOut[c] = raw;
		s.outFunc = -1;
		return;
	}

	// Quantize to semitone. EQ-DIST instead gives every allowed note the same share of the octave.
	float t = in * 12.f;
	float semi;
	if (c == 2 && p.eqDist) {
		int offs[12], cnt = 0;
		for (int o = 0; o < 12; o++)
			if (mask & (1 << wrap(harmony.chordRoot + o, 12)))
				offs[cnt++] = o;
		float x = t - harmony.chordRoot;
		int oct = (int) std::floor(x / 12.f);
		float frac = (x - 12.f * oct) / 12.f;
		int idx = clamp((int) (frac * cnt), 0, cnt - 1);
		semi = harmony.chordRoot + 12 * oct + offs[idx];
	}
	else {
		semi = std::round(t);
	}

	// Sample & hold: with a cable in TRIG, the input is only read on triggers (plus time window).
	// A bypassed channel ignores the trigger.
	bool trigConnected = inputs[CH_TRIG_INPUT + c].isConnected() && bypass == 0;
	bool force = inFuncHigh[IN_QUANT_CH1 + c];
	if (s.trig.process(inputs[CH_TRIG_INPUT + c].getVoltage(), 1.f, 3.f))
		s.trigWindow = settings.trigWindowMs / 1000.f + 1e-6f;
	if (!trigConnected || force || s.trigWindow > 0.f)
		s.held = semi + preTrans * 12.f;
	s.trigWindow -= dt;

	float v;
	if (bypass == 1) {
		// Bypass + Transpose: the continuous input including IN follows ROOT, then octave,
		// transposition, slew, detune and pitch bend below
		v = in;
		s.outFunc = -1;
	}
	else if (s.gliding) {
		v = in + preTrans;
		s.outFunc = -1;
	}
	else {
		int n = quantizeNearest(s.held, mask, s.note, s.hasNote);
		int out = shuffleNote(n);
		if (s.hasNote && n != s.note)
			s.autogate.trigger();
		s.note = n;
		s.hasNote = true;
		s.outFunc = funcOfPitchIn(harmony, n, funcs);
		v = toVolts(out);
	}
	static const int slewIn[3] = {IN_SLEW_CH1, IN_SLEW_CH2, IN_SLEW_CH3};
	bool slewOn = inFuncHigh[slewIn[c]] || (c == 0 && p.ch1Slew);
	pitchOut[c] = postProcess(c, v, c, slewOn, dt);
}

void Harmonion::processChords(float dt) {
	const Preset& p = audiblePreset();
	float in = inputFor(SEC_CHORDS) + params[CHORD_PITCH_PARAM].getValue();
	bool trigConnected = inputs[CHORD_TRIG_INPUT].isConnected();
	if (chordTrig.process(inputs[CHORD_TRIG_INPUT].getVoltage(), 1.f, 3.f))
		chordTrigWindow = settings.trigWindowMs / 1000.f + 1e-6f;
	if (!trigConnected || inFuncHigh[IN_QUANT_CHORDS] || chordTrigWindow > 0.f)
		chordHeld = in;
	chordTrigWindow -= dt;

	// The first four selected tones; fewer are doubled
	uint16_t funcs = sectionFuncs(SEC_CHORDS, p);
	int picked[4];
	int n = 0;
	for (int t = 0; t < NUM_TONES && n < 4; t++)
		if ((funcs & (1 << t)) && harmony.notes[t][0] >= 0)
			picked[n++] = t;
	if (n == 0)
		picked[n++] = T1;
	chordPicked = 0;
	for (int i = 0; i < n; i++)
		chordPicked |= 1 << picked[i];
	int pc[4];
	for (int k = 0; k < 4; k++) {
		chordFuncs[k] = picked[k % n];
		pc[k] = harmony.notes[chordFuncs[k]][0];
	}

	float b = chordHeld * 12.f;
	auto atOrAbove = [&](int pcv, float from) {
		int m = (int) std::ceil(from - 1e-4f);
		while (wrap(m, 12) != pcv)
			m++;
		return m;
	};
	// Which voice is the lowest
	bool slashBass = harmony.scale->slashRoot >= 0 && picked[0] == T1;
	int lowest = 0;
	int inv = p.inversion;
	if (slashBass) {
		lowest = 0;
	}
	else if (inv == 0) {
		int best = 1 << 30;
		for (int k = 0; k < n; k++) {
			int m = atOrAbove(pc[k], b);
			if (m < best) {
				best = m;
				lowest = k;
			}
		}
	}
	else {
		lowest = (inv - 1) % n;
	}
	int order[4];
	if (slashBass) {
		// The bass stays at the bottom, only the triad above it is inverted
		order[0] = 0;
		int start = (inv == 0) ? 1 : 1 + (inv - 1) % 3;
		for (int i = 1; i < 4; i++)
			order[i] = 1 + (start - 1 + i - 1) % 3;
	}
	else {
		for (int i = 0; i < 4; i++)
			order[i] = (lowest + i) % 4;
	}
	int pitch[4];
	pitch[order[0]] = atOrAbove(pc[order[0]], b);
	for (int i = 1; i < 4; i++)
		pitch[order[i]] = atOrAbove(pc[order[i]], pitch[order[i - 1]] + 1);

	// Spread: notes except the lowest move up by octaves (manual 10.1), one step at a time the lowest
	// of the upper notes, until the range between lowest and highest note is the closest to the close
	// range plus the spread in semitones. This reproduces the manual's Chord Structure photo, where
	// Cmaj7 in 2nd inversion spreads to G E B C ("5··3·71").
	float spread = params[SPREAD_PARAM].getValue();
	if (funcActive[CV_SPREAD])
		spread *= clamp(funcCv[CV_SPREAD] / 5.f, 0.f, 1.f);
	int s = (int) std::round(spread);
	if (s > 0) {
		int low = order[0];
		int target = pitch[order[3]] - pitch[low] + s;
		int cur[4], best[4];
		for (int k = 0; k < 4; k++)
			cur[k] = best[k] = pitch[k];
		int bestErr = s;
		for (int level = 0; level < 12; level++) {
			int v = order[1];
			for (int i = 2; i < 4; i++)
				if (cur[order[i]] < cur[v])
					v = order[i];
			cur[v] += 12;
			int top = std::max(cur[order[1]], std::max(cur[order[2]], cur[order[3]]));
			int err = std::abs(top - cur[low] - target);
			if (err < bestErr) {
				bestErr = err;
				for (int k = 0; k < 4; k++)
					best[k] = cur[k];
			}
			if (top - cur[low] > target + 12)
				break;
		}
		for (int k = 0; k < 4; k++)
			pitch[k] = best[k];
	}

	if (bendFade < 1.f) {
		float d = BEND_DELAY_SEC[clamp(settings.bendDelay, 0, 3)];
		bendFade = (d > 0.f) ? std::min(1.f, bendFade + dt / d) : 1.f;
	}
	bool slewOn = p.chordSlew || inFuncHigh[IN_SLEW_CHORDS];
	for (int k = 0; k < 4; k++) {
		chordNotes[k] = pitch[k];
		pitchOut[4 + k] = postProcess(4 + k, toVolts(shuffleNote(pitch[k])), SEC_CHORDS, slewOn, dt);
	}
}

void Harmonion::chordsShuffle() {
	// Random new chord selection with the same number of notes
	Preset& p = editPreset();
	int count = 0;
	for (int t = 0; t < NUM_TONES; t++)
		if (p.chords & (1 << t))
			count++;
	if (count == 0 || count == NUM_TONES)
		return;
	uint8_t m = 0;
	while (__builtin_popcount(m) < count)
		m |= 1 << (rng() % NUM_TONES);
	p.chords = m;
}

void Harmonion::arpStep() {
	outPulse[OUT_ARP_BEAT].trigger();
	const Preset& p = audiblePreset();
	float b = (inputFor(SEC_ARP) + params[ARP_PITCH_PARAM].getValue()) * 12.f;
	float rangeF = params[RANGE_PARAM].getValue() + (funcActive[CV_ARP_RANGE] ? funcCv[CV_ARP_RANGE] * 12.f : 0.f);
	int range = clamp((int) std::round(rangeF), 0, 60);
	uint16_t funcs = sectionFuncs(SEC_ARP, p);
	uint16_t mask = harmony.pitchMask(funcs);
	if (!mask)
		mask = 1 << harmony.chordRoot;

	if (range == 0) {
		// Behaves like a quantizer that reads its input on every clock
		arpNote = quantizeNearest(b, mask, arpNote, false);
		arpFunc = funcOfPitchIn(harmony, arpNote, funcs);
		arpResetPending = arpBeginPending = false;
		return;
	}
	int low = (int) std::ceil(b - 1e-4f);
	while (!(mask & (1 << wrap(low, 12))))
		low++;
	std::vector<int> list;
	for (int m = low; m <= low + range; m++)
		if (mask & (1 << wrap(m, 12)))
			list.push_back(m);
	int n = list.size();
	bool downward = p.pattern == PAT_DOWN || p.pattern == PAT_DOWNUP;

	if (arpResetPending) {
		arpResetPending = false;
		if (arpBeginPending && p.begin >= 0 && harmony.notes[p.begin][0] >= 0) {
			arpBeginPending = false;
			// Restart with the BEGIN interval, even if it is not selected
			int target = low;
			while (wrap(target, 12) != harmony.notes[p.begin][0])
				target++;
			arpNote = target;
			arpFunc = p.begin;
			arpIdx = 0;
			while (arpIdx < n - 1 && list[arpIdx] < target)
				arpIdx++;
			arpDir = downward ? -1 : 1;
			arpPhase = 0;
			return;
		}
		arpBeginPending = false;
		arpIdx = -1;
	}
	if (arpIdx >= n)
		arpIdx = n - 1;

	auto bounce = [&]() {
		if (n == 1) {
			arpIdx = 0;
			return;
		}
		arpIdx += arpDir;
		if (arpIdx >= n) {
			arpIdx = n - 2;
			arpDir = -1;
		}
		if (arpIdx < 0) {
			arpIdx = 1;
			arpDir = 1;
		}
	};
	int octaveUp = 0;
	switch (p.pattern) {
		case PAT_UP:
			arpIdx = (arpIdx < 0) ? 0 : (arpIdx + 1) % n;
			break;
		case PAT_DOWN:
			arpIdx = (arpIdx < 0) ? n - 1 : wrap(arpIdx - 1, n);
			break;
		case PAT_UPDOWN:
			if (arpIdx < 0) {
				arpIdx = 0;
				arpDir = 1;
			}
			else bounce();
			break;
		case PAT_DOWNUP:
			if (arpIdx < 0) {
				arpIdx = n - 1;
				arpDir = -1;
			}
			else bounce();
			break;
		case PAT_BAROQUE:
			// Two steps forward, one step back
			if (arpIdx < 0) {
				arpIdx = 0;
				arpDir = 1;
				arpPhase = 0;
			}
			else if (n == 1) {
				arpIdx = 0;
			}
			else {
				int move = (arpPhase == 2) ? -arpDir : arpDir;
				arpPhase = (arpPhase + 1) % 3;
				arpIdx += move;
				if (arpIdx >= n) {
					arpIdx = std::max(0, n - 2);
					arpDir = -1;
					arpPhase = 0;
				}
				if (arpIdx < 0) {
					arpIdx = std::min(1, n - 1);
					arpDir = 1;
					arpPhase = 0;
				}
			}
			break;
		case PAT_OCTAVES:
			if (arpIdx < 0) {
				arpIdx = 0;
				arpPhase = 0;
			}
			else if (arpPhase == 0) {
				arpPhase = 1;
			}
			else {
				arpPhase = 0;
				arpIdx = (arpIdx + 1) % n;
			}
			octaveUp = arpPhase ? 12 : 0;
			break;
		case PAT_BROWNIAN:
			if (arpIdx < 0) {
				arpIdx = 0;
			}
			else if (n > 1) {
				arpIdx += (rng() & 1) ? 1 : -1;
				if (arpIdx >= n)
					arpIdx = n - 2;
				if (arpIdx < 0)
					arpIdx = 1;
			}
			break;
		case PAT_RANDOM:
		default:
			if (n == 1) {
				arpIdx = 0;
			}
			else {
				int r;
				do {
					r = rng() % n;
				} while (r == arpIdx);
				arpIdx = r;
			}
			break;
	}
	arpNote = list[clamp(arpIdx, 0, n - 1)] + octaveUp;
	arpFunc = funcOfPitchIn(harmony, arpNote, funcs);
}

void Harmonion::processArp(float dt) {
	// The arpeggio clock is normalled to the sequencer clock
	float clkV = inputs[ARP_CLK_INPUT].isConnected() ? inputs[ARP_CLK_INPUT].getVoltage() : inputs[SEQ_CLK_INPUT].getVoltage();
	auto fire = [&]() {
		if (funcActive[CV_ARP_RANDOM]) {
			float prob = clamp(funcCv[CV_ARP_RANDOM] / 5.f, 0.f, 1.f);
			if (random::uniform() >= prob)
				return;
		}
		if (settings.arpDelayMs > 0)
			arpDelayed.push_back(time + settings.arpDelayMs / 1000.0);
		else
			arpStep();
	};
	if (arpClkTrig.process(clkV, 1.f, 3.f)) {
		clkLight[0].trigger(0.05f);
		if (arpLastClock >= 0.0)
			arpPeriod = std::min(std::max(time - arpLastClock, 0.001), 10.0);
		arpLastClock = time;
		int r = settings.arpClock + (funcActive[CV_ARP_CLOCK] ? semis(funcCv[CV_ARP_CLOCK]) : 0);
		r = clamp(r, 0, 15);
		int mul = ARP_RATIO[r][0], div = ARP_RATIO[r][1];
		int k = arpClockCount % div;
		arpClockCount = (arpClockCount + 1) % (div * 64);
		// The incoming clock has priority over interpolated ones
		arpScheduled.clear();
		int j0 = (int) std::ceil((double) k * mul / div - 1e-9);
		for (int j = j0; (double) j * div / mul < k + 1 - 1e-9; j++) {
			double delay = ((double) j * div / mul - k) * arpPeriod;
			if (delay <= 1e-6)
				fire();
			else
				arpScheduled.push_back(time + delay);
		}
	}
	for (size_t i = 0; i < arpScheduled.size();) {
		if (arpScheduled[i] <= time) {
			arpScheduled.erase(arpScheduled.begin() + i);
			fire();
		}
		else i++;
	}
	for (size_t i = 0; i < arpDelayed.size();) {
		if (arpDelayed[i] <= time) {
			arpDelayed.erase(arpDelayed.begin() + i);
			arpStep();
		}
		else i++;
	}
	pitchOut[3] = postProcess(3, toVolts(shuffleNote(arpNote)), SEC_ARP, false, dt);
}

void Harmonion::processInFuncs() {
	for (int f = 0; f < NUM_IN_FUNCS; f++)
		inFuncHigh[f] = false;
	syncSlave = settings.inFunc[0] == IN_SYNC_SLAVE;
	for (int i = 0; i < 2; i++) {
		int f = settings.inFunc[i];
		float v = inputs[IN_INPUT + i].getVoltage();
		bool rise = inTrig[i].process(v, 1.f, 3.f);
		inHigh[i] = inTrig[i].isHigh();
		if (f == IN_SYNC_SLAVE)
			continue;
		if (inHigh[i])
			inFuncHigh[f] = true;
		if (!rise)
			continue;
		switch (f) {
			case IN_CHORDS_SHUFFLE: chordsShuffle(); break;
			case IN_ARP_RESET:
				arpResetPending = true;
				arpBeginPending = audiblePreset().begin >= 0;
				break;
			case IN_SONG_RESET: if (seqMode && playing) seqReset(true); break;
			case IN_PART_RESET: if (seqMode && playing) seqReset(false); break;
			case IN_STEP_FWD: if (seqMode) seqAdvance(1, false); break;
			case IN_STEP_BACK: if (seqMode) seqAdvance(-1, false); break;
			case IN_PART_FWD: if (seqMode) gotoPart(part + 1); break;
			case IN_PART_BACK: if (seqMode) gotoPart(part - 1); break;
			case IN_PLAY: if (seqMode) playing = !playing; break;
			default: break;
		}
	}

	// Harmonic link, slave side: a polyphonic cable from the master's OUT 1
	if (syncSlave) {
		Input& in = inputs[IN_INPUT + 0];
		if (in.getChannels() >= 9) {
			float hb = in.getVoltage(7);
			if (hb != syncLastBeat) {
				syncLastBeat = hb;
				syncSilence = 0.f;
			}
		}
		syncAlive = syncSilence < 0.05f;
		if (syncAlive) {
			syncRoot = in.getVoltage(0);
			syncMode = in.getVoltage(1);
			syncDegree = in.getVoltage(2);
			syncTrans = in.getVoltage(3);
			syncDetune = in.getVoltage(4);
			bool clk = in.getVoltage(5) > 1.f;
			if (clk && !syncClockHigh)
				seqClock();
			syncClockHigh = clk;
			bool rst = in.getVoltage(6) > 1.f;
			if (rst && !syncResetHigh) {
				nonSeqClock = 0;
				nonSeqBeat = 0;
				outPulse[OUT_SONG_RESET].trigger();
			}
			syncResetHigh = rst;
		}
	}
	else {
		syncAlive = false;
	}
}

void Harmonion::processOutputs(float dt) {
	for (int f = 0; f < NUM_OUT_FUNCS; f++)
		outHigh[f] = outPulse[f].process(dt);
	for (int c = 0; c < 3; c++)
		outHigh[OUT_AUTOGATE_CH1 + c] = ch[c].autogate.process(dt);
	bool tuneActive = tuning || inFuncHigh[IN_TUNING];
	outHigh[OUT_TUNING] = tuneActive;
	outHigh[OUT_CH1_AB] = ch[0].usingB;
	bool manual = false;
	if (settings.ch2Single && page == PAGE_MAIN)
		for (int f = 0; f < NUM_FUNCS; f++)
			manual |= bigHeld[C2_FUNC[f]];
	outHigh[OUT_CH2_MANUAL_GATE] = manual;

	// Tuning mode: arpeggio beats at a fixed rate so the attached voice is audible
	if (tuneActive && time >= tuneArpNext) {
		tuneArpNext = time + 0.25;
		outPulse[OUT_ARP_BEAT].trigger(0.05f);
	}

	float v[8];
	for (int i = 0; i < 8; i++) {
		if (calibrating)
			v[i] = calib5V ? 5.f : 0.f;
		else if (tuneActive)
			v[i] = tuneOctave + audiblePreset().octave[OUT_SECTION[i]];
		else
			v[i] = pitchOut[i];
	}
	// Output switching matrix for Channel 1-3 and Arpeggio
	float routed[4];
	for (int j = 0; j < 4; j++)
		routed[j] = v[clamp(settings.outSrc[j], 0, 3)];
	for (int j = 0; j < 4; j++)
		v[j] = (calibrating || tuneActive) ? v[j] : routed[j];
	static const int outIds[8] = {CH_OUTPUT + 0, CH_OUTPUT + 1, CH_OUTPUT + 2, ARP_OUTPUT,
		CHORD_OUTPUT + 0, CHORD_OUTPUT + 1, CHORD_OUTPUT + 2, CHORD_OUTPUT + 3};
	for (int i = 0; i < 8; i++) {
		float x = clamp(v[i], -2.f, 8.f);
		x += calib[i][0] + (calib[i][1] - calib[i][0]) * x / 5.f;
		outputs[outIds[i]].setVoltage(x);
	}

	for (int i = 0; i < 2; i++) {
		int f = settings.outFunc[i];
		Output& o = outputs[OUT_OUTPUT + i];
		if (f == OUT_SYNC_MASTER && i == 0) {
			// Harmonic link, master side
			o.setChannels(16);
			o.setVoltage(harmony.keyRoot, 0);
			o.setVoltage(harmony.mode, 1);
			o.setVoltage(harmony.degree, 2);
			o.setVoltage(transSemis, 3);
			o.setVoltage(funcActive[CV_DETUNE] ? funcCv[CV_DETUNE] : 0.f, 4);
			o.setVoltage(outHigh[OUT_SEQ_CLOCK] ? gateV() : 0.f, 5);
			o.setVoltage(outHigh[OUT_SONG_RESET] ? gateV() : 0.f, 6);
			o.setVoltage(o.getVoltage(7) > 0.f ? 0.f : 1.f, 7);
			o.setVoltage(outHigh[OUT_SEQ_BEAT] ? gateV() : 0.f, 8);
			for (int c = 9; c < 16; c++)
				o.setVoltage(0.f, c);
		}
		else {
			o.setChannels(1);
			o.setVoltage((f > OUT_NONE && f < NUM_OUT_FUNCS && f != OUT_SYNC_MASTER && outHigh[f]) ? gateV() : 0.f);
		}
	}
	const Preset& p = audiblePreset();
	for (int i = 0; i < 2; i++)
		outputs[GATE_OUTPUT + i].setVoltage(p.gate[i] ? gateV() : 0.f);
}

void Harmonion::tick(float dt) {
	// Assignable CVs and pots
	voltmeterActive = false;
	for (int f = 0; f < NUM_CV_FUNCS; f++) {
		funcActive[f] = false;
		funcCv[f] = 0.f;
	}
	bool cvActive[NUM_CV_FUNCS] = {};    // assigned to a jack with a cable
	bool cvAssigned[NUM_CV_FUNCS] = {};  // assigned to a jack, with or without cable
	for (int i = 0; i < 4; i++) {
		int f = settings.cvFunc[i];
		if (f == CV_VOLTMETER) {
			voltmeterActive = true;
			voltmeter = inputs[CV_INPUT + i].getVoltage();
			continue;
		}
		if (f <= CV_NONE || f >= NUM_CV_FUNCS)
			continue;
		cvAssigned[f] = true;
		if (!inputs[CV_INPUT + i].isConnected())
			continue;
		cvActive[f] = true;
		funcCv[f] += inputs[CV_INPUT + i].getVoltage();
	}
	for (int f = 1; f < NUM_CV_FUNCS; f++) {
		int pot = -1;
		for (int i = 0; i < 2; i++)
			if (settings.potFunc[i] == f)
				pot = i;
		float pv = (pot >= 0) ? params[POT1_PARAM + pot].getValue() : 0.f;
		if (pot >= 0 && cvAssigned[f]) {
			// The pot turns into an attenuator of the CV. Like the jacks of the original, which are
			// normalled to 0 V, an unpatched jack gives 0.
			funcActive[f] = true;
			funcCv[f] *= pv;
		}
		else if (cvActive[f]) {
			funcActive[f] = true;
		}
		else if (pot >= 0 && f != CV_VOLTMETER) {
			funcActive[f] = true;
			funcCv[f] = potVoltage(f, pv);
		}
	}
	if (tuning || inFuncHigh[IN_TUNING]) {
		// Pitch modifying CVs are disabled in tuning mode
		funcActive[CV_TRANSPOSE] = funcActive[CV_DETUNE] = false;
		for (int f = CV_BEND_CH1; f <= CV_BEND_CHORDS; f++)
			funcActive[f] = false;
	}

	processInFuncs();
	if (syncSlave)
		syncSilence += dt;
	if (seqClkTrig.process(inputs[SEQ_CLK_INPUT].getVoltage(), 1.f, 3.f))
		seqClock();

	readKnobs();

	// Sequencer CVs
	if (seqMode && !syncSlave) {
		if (funcActive[CV_PART]) {
			int v = clamp((int) std::round(funcCv[CV_PART]), 0, 4);
			if (v != lastPartCv && v > 0)
				gotoPart(v - 1);
			lastPartCv = v;
		}
		if (funcActive[CV_SONG_PART]) {
			int v = semis(funcCv[CV_SONG_PART]);
			if (v != lastSongPartCv && v > 0) {
				int idx = (v - 1) % (NUM_SONGS * NUM_PARTS);
				song = idx / NUM_PARTS;
				gotoPart(idx % NUM_PARTS);
			}
			lastSongPartCv = v;
		}
		if (funcActive[CV_STEP] && !playing) {
			int v = semis(funcCv[CV_STEP]);
			if (v != lastStepCv) {
				step = wrap(v, currentPart().steps.size());
				onStepChange();
			}
			lastStepCv = v;
		}
	}
	if (funcActive[CV_VOICING]) {
		int v = semis(std::fabs(funcCv[CV_VOICING])) % 99;
		if (v != lastVoicing && v > 0)
			editPreset().chords = voicings()[v];
		lastVoicing = v;
	}

	updateHarmony();
	for (int c = 0; c < 3; c++)
		processChannel(c, dt);
	processChords(dt);
	processArp(dt);
	processOutputs(dt);
}

static const int WATCHED_KNOBS[Harmonion::NUM_WATCHED_KNOBS] = {
	Harmonion::SLEW_PARAM, Harmonion::POT1_PARAM, Harmonion::POT2_PARAM, Harmonion::CHORD_PITCH_PARAM,
	Harmonion::SPREAD_PARAM, Harmonion::ARP_PITCH_PARAM, Harmonion::RANGE_PARAM,
};

void Harmonion::watchKnobs() {
	for (int i = 0; i < NUM_WATCHED_KNOBS; i++) {
		const ParamQuantity* pq = paramQuantities[WATCHED_KNOBS[i]];
		float v = params[WATCHED_KNOBS[i]].getValue();
		float range = pq->maxValue - pq->minValue;
		if (knobsInit && std::fabs(v - lastKnob[i]) > 0.002f * range)
			showPopup(POPUP_KNOB, WATCHED_KNOBS[i]);
		if (!knobsInit || std::fabs(v - lastKnob[i]) > 0.002f * range)
			lastKnob[i] = v;
	}
	knobsInit = true;
}

void Harmonion::process(const ProcessArgs& args) {
	time += args.sampleTime;
	sampleTime = args.sampleTime;
	if (controlDivider.process()) {
		float dt = args.sampleTime * controlDivider.getDivision();
		handleButtons();
		watchKnobs();
		tick(dt);
	}
	if (lightDivider.process())
		updateLights();
}

json_t* Harmonion::dataToJson() {
	json_t* root = json_object();
	json_object_set_new(root, "settings", settings.toJson());
	json_t* pr = json_array();
	for (int i = 0; i < NUM_PRESETS; i++)
		json_array_append_new(pr, presets[i].toJson());
	json_object_set_new(root, "presets", pr);
	json_object_set_new(root, "livePreset", livePreset.toJson());
	json_t* sg = json_array();
	for (int s = 0; s < NUM_SONGS; s++) {
		json_t* parts = json_array();
		for (int p = 0; p < NUM_PARTS; p++) {
			json_t* pj = json_object();
			json_object_set_new(pj, "clockDiv", json_integer(songs[s][p].clockDiv));
			json_t* steps = json_array();
			for (const Step& st : songs[s][p].steps)
				json_array_append_new(steps, st.toJson());
			json_object_set_new(pj, "steps", steps);
			json_array_append_new(parts, pj);
		}
		json_array_append_new(sg, parts);
	}
	json_object_set_new(root, "songs", sg);
	json_object_set_new(root, "seqMode", json_boolean(seqMode));
	json_object_set_new(root, "playing", json_boolean(playing));
	json_object_set_new(root, "song", json_integer(song));
	json_object_set_new(root, "part", json_integer(part));
	json_object_set_new(root, "step", json_integer(step));
	json_t* cal = json_array();
	for (int i = 0; i < 8; i++)
		for (int k = 0; k < 2; k++)
			json_array_append_new(cal, json_real(calib[i][k]));
	json_object_set_new(root, "calib", cal);
	json_object_set_new(root, "theme", json_integer(theme));
	json_object_set_new(root, "gate5V", json_boolean(gate5V));
	json_object_set_new(root, "version", json_integer(2));
	return root;
}

void Harmonion::dataFromJson(json_t* root) {
	settings.fromJson(json_object_get(root, "settings"));
	json_t* pr = json_object_get(root, "presets");
	for (int i = 0; pr && i < NUM_PRESETS && i < (int) json_array_size(pr); i++)
		presets[i].fromJson(json_array_get(pr, i));
	livePreset.fromJson(json_object_get(root, "livePreset"));
	json_t* sg = json_object_get(root, "songs");
	for (int s = 0; sg && s < NUM_SONGS && s < (int) json_array_size(sg); s++) {
		json_t* parts = json_array_get(sg, s);
		for (int p = 0; p < NUM_PARTS && p < (int) json_array_size(parts); p++) {
			json_t* pj = json_array_get(parts, p);
			Part& part = songs[s][p];
			part.clockDiv = clamp(jint(pj, "clockDiv", 0), 0, NUM_CLOCK_DIVS - 1);
			json_t* steps = json_object_get(pj, "steps");
			part.steps.clear();
			for (int k = 0; steps && k < (int) json_array_size(steps) && k < MAX_STEPS; k++) {
				Step st;
				st.fromJson(json_array_get(steps, k));
				part.steps.push_back(st);
			}
			if (part.steps.empty())
				part.steps.push_back(Step());
		}
	}
	seqMode = jbool(root, "seqMode", false);
	playing = jbool(root, "playing", true);
	song = clamp(jint(root, "song", 0), 0, NUM_SONGS - 1);
	// Like the original after power up: the song starts in part A at its first step
	part = 0;
	step = 0;
	beat = 0;
	pendingPart = -1;
	knobsInit = false;
	json_t* cal = json_object_get(root, "calib");
	for (int i = 0; cal && i < 16 && i < (int) json_array_size(cal); i++)
		calib[i / 2][i % 2] = json_real_value(json_array_get(cal, i));
	theme = clamp(jint(root, "theme", 0), 0, 3);
	gate5V = jbool(root, "gate5V", false);
	lastRootKnob = lastDegreeKnob = lastModeKnob = lastSongKnob = -1;
	waitFirstClock = true;
}

}  // namespace hm
