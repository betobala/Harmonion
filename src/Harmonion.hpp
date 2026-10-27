#pragma once
#include "plugin.hpp"
#include "Harmony.hpp"
#include <array>

namespace hm {

using namespace harmony;

// Large buttons. The 8x4 field at the top holds the buttons of Channel 1, 2 and 3 and doubles
// as step field (SONG page), routing matrix (OCT page) and calibration field.
inline int top(int row, int col) {
	return row * 8 + col;
}
static const int NUM_TOP = 32;
static const int CHORD_BTN = 32;  // 1 3 5 7 9 11 13 INV SLEW
static const int ARP_BTN = 41;    // 1 3 5 7 9 11 13 PATTERN BEGIN
static const int GATE_BTN = 50;
static const int NUM_BIG = 52;

enum ChordBtn { CB_INV = 7, CB_SLEW = 8 };
enum ArpBtn { AB_PATTERN = 7, AB_BEGIN = 8 };

// Channel 1 field (columns 0-2)
static const int C1A_TONE[7] = {top(0, 0), top(0, 1), top(1, 0), top(1, 1), top(2, 0), top(2, 1), top(3, 0)};
static const int C1A_FILL = top(3, 1);
static const int C1B_ROOT = top(0, 2);
static const int C1B_5TH = top(1, 2);
static const int C1B_SCALE = top(2, 2);
static const int C1_SLEW = top(3, 2);
// Channel 2 field (columns 3-5), indexed by function T1..FILL5
static const int C2_FUNC[12] = {top(0, 3), top(0, 4), top(1, 3), top(1, 4), top(2, 3), top(2, 4), top(3, 3),
	top(3, 4), top(0, 5), top(1, 5), top(2, 5), top(3, 5)};
// Channel 3 field (columns 6-7)
static const int C3_TONE[7] = {top(0, 6), top(0, 7), top(1, 6), top(1, 7), top(2, 6), top(2, 7), top(3, 6)};
static const int C3_EQDIST = top(3, 7);

// Sections that have octave switches and input options
enum Section { SEC_CH1, SEC_CH2, SEC_CH3, SEC_CHORDS, SEC_ARP, NUM_SECTIONS };

// OCT page assignment (manual, chapter 6): octave up, octave down, IN follows ROOT, input scaling,
// bypass, glide detection. Channel 3 goes down with 3RD and detects glides with 5TH; the arpeggio
// has IN follows ROOT on PATTERN and the input scaling on BEGIN.
struct OctMap {
	int up, down, follow, scaling, bypass, glide;
};
static const OctMap OCT_MAP[NUM_SECTIONS] = {
	{top(0, 0), top(1, 0), top(3, 1), top(3, 2), top(3, 0), top(0, 2)},
	{top(0, 3), top(1, 3), top(3, 4), top(3, 5), top(3, 3), top(0, 5)},
	{top(0, 6), top(0, 7), top(3, 7), top(2, 7), top(3, 6), top(1, 6)},
	{CHORD_BTN + T1, CHORD_BTN + T5, CHORD_BTN + CB_INV, CHORD_BTN + CB_SLEW, -1, -1},
	{ARP_BTN + T1, ARP_BTN + T5, ARP_BTN + AB_PATTERN, ARP_BTN + AB_BEGIN, -1, -1},
};

/** The setting of all 52 large buttons plus the five octave switches */
struct Preset {
	uint8_t ch1A = 0;       // tones T1..T13
	bool ch1Fill = false;
	uint8_t ch1B = 0;       // bit0 ROOT, bit1 5TH, bit2 SCALE
	bool ch1Slew = false;
	uint16_t ch2 = 0;       // functions T1..FILL5
	uint8_t ch3 = 0;
	bool eqDist = false;
	uint8_t chords = 0;
	uint8_t inversion = 0;  // 0 free, 1 root position, 2..4 first..third inversion
	bool chordSlew = false;
	uint8_t arp = 0;
	uint8_t pattern = 0;
	int8_t begin = -1;      // -1 "Next Note", else tone T1..T13
	bool gate[2] = {};
	int8_t octave[NUM_SECTIONS] = {};

	static Preset defaults();
	json_t* toJson() const;
	void fromJson(json_t* j);
};

struct Step {
	int8_t root = 0;
	int8_t mode = 0;
	int8_t degree = 0;
	int16_t beats = 4;
	Preset preset = Preset::defaults();
	json_t* toJson() const;
	void fromJson(json_t* j);
};

static const int MAX_STEPS = 32;

struct Part {
	// Capacity is reserved so the display can read steps while the engine edits them
	std::vector<Step> steps;
	int clockDiv = 0;  // index into CLOCK_DIVS
	Part() {
		steps.reserve(MAX_STEPS);
		steps.push_back(Step());
	}
};

static const int NUM_SONGS = 16;
static const int NUM_PARTS = 4;
static const int MAX_BEATS = 128;
static const int NUM_PRESETS = 16;
static const int CLOCK_DIVS[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 16, 24, 32, 48, 64};
static const int NUM_CLOCK_DIVS = 15;
// Arpeggio clock ratios: multiplier / divider. The manual's photo shows 1/1 as the 8th of 16 values;
// reconstruído: the values between 1/4 and 4/1
static const int NUM_ARP_RATIOS = 16;
static const int ARP_RATIO[NUM_ARP_RATIOS][2] = {{1, 4}, {1, 3}, {2, 5}, {1, 2}, {3, 5}, {2, 3}, {3, 4}, {1, 1},
	{4, 3}, {3, 2}, {5, 3}, {2, 1}, {5, 2}, {3, 1}, {7, 2}, {4, 1}};
static const int ARP_RATIO_UNITY = 7;
// Menu ranges, from the number of values in the manual's photos
static const int MAX_TRIG_WINDOW_MS = 14;
static const int MAX_ARP_DELAY_MS = 13;

enum CvFunc {
	CV_NONE, CV_TRANSPOSE, CV_SHUFFLE, CV_DETUNE, CV_HARMONIC_SHIFT, CV_PRESET, CV_SLEW_TIME, CV_VOLTMETER,
	CV_ROOT, CV_DEGREE, CV_MODE, CV_PRE_CH1, CV_PRE_CH2, CV_PRE_CH3, CV_PRE_ARP,
	CV_BEND_CH1, CV_BEND_CH2, CV_BEND_CH3, CV_BEND_CHORDS, CV_SPREAD, CV_VOICING,
	CV_ARP_CLOCK, CV_ARP_RANDOM, CV_ARP_RANGE, CV_SONG, CV_PART, CV_SONG_PART, CV_STEP,
	NUM_CV_FUNCS
};
enum InFunc {
	IN_NONE, IN_SYNC_SLAVE, IN_SLEW_CH1, IN_SLEW_CH2, IN_SLEW_CH3, IN_SLEW_CHORDS,
	IN_QUANT_CH1, IN_QUANT_CH2, IN_QUANT_CH3, IN_QUANT_CHORDS, IN_CHORDS_SHUFFLE, IN_ARP_RESET,
	IN_SONG_RESET, IN_PART_RESET, IN_STEP_FWD, IN_STEP_BACK, IN_PART_FWD, IN_PART_BACK,
	IN_TUNING, IN_PLAY,
	NUM_IN_FUNCS
};
enum OutFunc {
	OUT_NONE, OUT_SYNC_MASTER, OUT_AUTOGATE_CH1, OUT_AUTOGATE_CH2, OUT_AUTOGATE_CH3, OUT_CH2_MANUAL_GATE,
	OUT_ARP_BEAT, OUT_SEQ_CLOCK, OUT_SEQ_BEAT, OUT_SONG_STEP, OUT_SONG_RESET, OUT_TUNING,
	OUT_CH1_AB, OUT_CHORD_CHANGE,
	NUM_OUT_FUNCS
};
enum AbMode {
	AB_OFF, AB_JACK, AB_BEAT1, AB_ODD_BEATS, AB_EVEN_BEATS, AB_STEP_START, AB_CLOCK1, AB_ODD_CLOCKS, AB_EVEN_CLOCKS,
	NUM_AB_MODES
};
enum DisplayMode { DISP_TONES, DISP_STRUCTURE, DISP_CHORD, DISP_OCTAVES, NUM_DISP_MODES };
// Arpeggio patterns. The ids are saved in patches; PATTERN_ORDER is the order of the button.
enum Pattern { PAT_UP, PAT_DOWN, PAT_UPDOWN, PAT_DOWNUP, PAT_BAROQUE, PAT_OCTAVES, PAT_BROWNIAN, PAT_RANDOM, NUM_PATTERNS };

extern const char* const CV_FUNC_NAMES[NUM_CV_FUNCS];
extern const char* const IN_FUNC_NAMES[NUM_IN_FUNCS];
extern const char* const OUT_FUNC_NAMES[NUM_OUT_FUNCS];
extern const char* const AB_MODE_NAMES[NUM_AB_MODES];
extern const char* const DISPLAY_MODE_NAMES[NUM_DISP_MODES];
extern const char* const INTONATION_MODE_NAMES[NUM_INTONATIONS];
extern const char* const BEND_DELAY_MODE_NAMES[4];
extern const char* const PATTERN_NAMES[NUM_PATTERNS];
extern const char* const BEGIN_NAMES[8];
extern const char* const INVERSION_NAMES[5];
// The order in which the menu and the buttons step through the values (ids are kept stable for
// saved patches). Taken from the position indicators in the manual's photos where visible.
extern const int CV_MENU_ORDER[NUM_CV_FUNCS];
extern const int IN_MENU_ORDER[NUM_IN_FUNCS];
extern const int OUT_MENU_ORDER[NUM_OUT_FUNCS];
extern const int AB_MENU_ORDER[NUM_AB_MODES];
extern const int PATTERN_ORDER[NUM_PATTERNS];
/** Position of an id in one of the order tables */
int orderIndex(const int* order, int n, int id);
// One step of the 16 bit output DAC of the original hardware (-2 V..8 V)
static const float DAC_UNIT_V = 10.f / 65536.f;

// Menu items in the order of the original (position indicators in the manual's photos)
enum MenuItem {
	MI_CV1, MI_CV2, MI_CV3, MI_CV4, MI_POT1, MI_POT2, MI_IN1, MI_IN2, MI_OUT1, MI_OUT2,
	MI_CLOCK_DIV, MI_ARP_CLOCK, MI_INTONATION, MI_SLEW_CURVE, MI_TRIG_WINDOW, MI_ARP_DELAY, MI_BEND_DELAY,
	MI_CH1_AB, MI_CH2_BUTTONS, MI_DISPLAY, MI_RESET,
	NUM_MENU_ITEMS
};

/** Global settings changed via the menu, OCT page and matrix page */
struct Settings {
	int display = DISP_TONES;
	int cvFunc[4] = {};
	int potFunc[2] = {};
	int inFunc[2] = {};
	int outFunc[2] = {};
	int nonSeqClockDiv = 3;
	int arpClock = ARP_RATIO_UNITY;
	int arpDelayMs = 0;
	int abMode = AB_OFF;
	bool ch2Single = false;
	int trigWindowMs = 0;
	bool slewExp = false;
	int intonation = EQUAL;
	int bendDelay = 0;
	// Section options (OCT page)
	int bypass[3] = {};           // 0 normal, 1 bypass + transpose, 2 true bypass
	bool follow[NUM_SECTIONS] = {};
	int inScale[NUM_SECTIONS] = {};  // 0..3 = 1, 2, 3, 5 V/oct
	bool glide[3] = {};
	// Matrices: inMix[channel][jack], outSrc[jack] = channel
	bool inMix[4][4] = {};
	int outSrc[4] = {0, 1, 2, 3};
	bool liveMode = false;

	Settings() {
		for (int i = 0; i < 4; i++)
			inMix[i][i] = true;
	}
	json_t* toJson() const;
	void fromJson(json_t* j);
};

/** Slew limiter for one pitch output */
struct Slew {
	float value = 0.f;
	float start = 0.f;
	float target = 0.f;
	bool init = false;
	float process(float in, float time, bool exponential, float dt);
};

/** Short output trigger */
struct Pulse {
	float remaining = 0.f;
	void trigger(float duration = 0.01f) {
		remaining = std::max(remaining, duration);
	}
	bool process(float dt) {
		if (remaining <= 0.f)
			return false;
		remaining -= dt;
		return true;
	}
};

/** Quantizer channel 1-3 runtime state */
struct ChannelState {
	float held = 0.f;      // Sampled input in semitones (after semitone quantization and Pre Trans)
	float trigWindow = 0.f;
	int note = 0;          // Last quantized semitone (before shuffle)
	int outFunc = -1;      // Function of the playing note, for the button lights
	bool usingB = false;
	float lastIn = 0.f;
	int moveTicks = 0;
	float stillTime = 1.f;
	bool gliding = false;
	bool hasNote = false;
	Slew slew;
	Pulse autogate;
	dsp::SchmittTrigger trig;
};

struct Screen;

struct Harmonion : Module {
	enum ParamId {
		ENUMS(BIG_PARAM, NUM_BIG),
		MENU_PARAM, TUNE_PARAM, SONG_PARAM, OCT_PARAM,
		SEQ_PARAM, LEFT_PARAM, RIGHT_PARAM, PLAY_PARAM,
		DEL_PARAM, MINUS_PARAM, PLUS_PARAM, ADD_PARAM,
		ROOT_PARAM, DEGREE_PARAM, TRANS_PARAM, MODE_PARAM, SLEW_PARAM, PRESET_PARAM,
		POT1_PARAM, POT2_PARAM, CHORD_PITCH_PARAM, SPREAD_PARAM, ARP_PITCH_PARAM, RANGE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(CH_IN_INPUT, 3),
		ENUMS(CH_TRIG_INPUT, 3),
		AB_INPUT, ROOT_INPUT, TRANS_INPUT,
		ENUMS(CV_INPUT, 4),
		ENUMS(IN_INPUT, 2),
		CHORD_IN_INPUT, CHORD_TRIG_INPUT,
		ARP_IN_INPUT, ARP_CLK_INPUT, SEQ_CLK_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(CH_OUTPUT, 3),
		ENUMS(CHORD_OUTPUT, 4),
		ARP_OUTPUT,
		ENUMS(OUT_OUTPUT, 2),
		ENUMS(GATE_OUTPUT, 2),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(BIG_LIGHT, NUM_BIG),
		ENUMS(SMALL_LIGHT, 12),
		ENUMS(TRIG_LIGHT, 4),
		AB_LIGHT,
		ENUMS(IN_LIGHT, 2),
		ENUMS(OUT_LIGHT, 2),
		ENUMS(GATE_LIGHT, 2),
		ARP_CLK_LIGHT, SEQ_CLK_LIGHT,
		LIGHTS_LEN
	};
	enum Page { PAGE_MAIN, PAGE_OCT, PAGE_MATRIX, PAGE_SONG };

	// Persistent state
	Settings settings;
	Preset presets[NUM_PRESETS];
	Preset livePreset;
	Part songs[NUM_SONGS][NUM_PARTS];
	bool seqMode = false;
	bool playing = true;
	int song = 0, part = 0, step = 0;
	float calib[8][2] = {};  // DAC offsets at 0 V and 5 V, in volts

	// UI state
	int page = PAGE_MAIN;
	bool menuOpen = false;
	int menuItem = 0;
	bool resetArmed = false;
	double resetDoneUntil = -1.0;
	bool tuning = false;
	int tuneOctave = 4;
	bool calibrating = false;
	int theme = 0;  // panel theme, see Theme.hpp
	bool gate5V = false;  // gate level of the hardware instead of the VCV standard
	double splashUntil = 2.0;  // start screen with the version
	bool calib5V = false;
	int calibLast = 0;
	bool frozen = false;
	int pendingPart = -1;

	// Harmony as heard and as edited
	Harmony harmony;
	Harmony editHarmony;
	Preset frozenPreset;
	int frozenRoot = 0, frozenMode = 0, frozenDegree = 0;
	int lastChordKey = -1;
	int transSemis = 0;
	int presetIndex = 0;

	// Knobs that behave like encoders in sequenced mode
	int lastRootKnob = -1, lastDegreeKnob = -1, lastModeKnob = -1, lastSongKnob = -1;
	int nsRoot = 0, nsMode = 0, nsDegree = 0;
	float pitchOut[8] = {};
	bool inFuncHigh[NUM_IN_FUNCS] = {};
	bool outHigh[NUM_OUT_FUNCS] = {};
	int lastVoicing = 0;
	int lastSongCv = 0, lastPartCv = 0, lastSongPartCv = 0, lastStepCv = 0;

	// Sequencer runtime
	int beat = 0;
	int clockIdx = 0;
	bool waitFirstClock = true;
	double time = 0.0;
	double ignoreClockUntil = -1.0;
	int nonSeqBeat = 0;
	int nonSeqClock = 0;
	bool beatIsFirstOfStep = true;

	// Section runtime
	ChannelState ch[3];
	Slew chordSlew[4];
	float chordHeld = 0.f;
	float chordTrigWindow = 0.f;
	dsp::SchmittTrigger chordTrig;
	int chordNotes[4] = {};
	int chordFuncs[4] = {};
	int chordPicked = 0;      // Mask of tones actually used by the chord
	float bendFade = 1.f;

	// Arpeggiator runtime
	int arpIdx = -1;
	int arpDir = 1;
	int arpPhase = 0;
	int arpNote = 0;
	int arpFunc = -1;
	bool arpResetPending = false;
	bool arpBeginPending = false;
	int arpClockCount = 0;
	double arpLastClock = -1.0;
	double arpPeriod = 0.5;
	std::vector<double> arpScheduled;
	std::vector<double> arpDelayed;
	Slew arpSlew;
	double tuneArpNext = 0.0;

	// Jacks
	dsp::SchmittTrigger seqClkTrig, arpClkTrig, inTrig[2];
	bool inHigh[2] = {};
	Pulse outPulse[NUM_OUT_FUNCS];
	Pulse clkLight[2];
	float lastFuncCv[NUM_CV_FUNCS] = {};
	bool funcActive[NUM_CV_FUNCS] = {};
	float funcCv[NUM_CV_FUNCS] = {};

	// Sync link
	bool syncSlave = false;
	bool syncAlive = false;
	float syncLastBeat = 0.f;
	float syncSilence = 1.f;
	float syncRoot = 0, syncMode = 0, syncDegree = 0, syncTrans = 0, syncDetune = 0;
	bool syncClockHigh = false, syncResetHigh = false;

	// Buttons
	dsp::BooleanTrigger bigTrig[NUM_BIG];
	bool bigHeld[NUM_BIG] = {};
	bool comboUsed[NUM_BIG] = {};
	dsp::BooleanTrigger smallTrig[12];
	bool smallHeld[12] = {};
	double lastPlayPress = -10.0;
	float voltmeter = 0.f;
	bool voltmeterActive = false;
	// Short message on the display after PATTERN, BEGIN, INV or the live mode changed
	enum Popup { POPUP_NONE, POPUP_PATTERN, POPUP_BEGIN, POPUP_INV, POPUP_LIVE };
	int popup = POPUP_NONE;
	double popupUntil = 0.0;
	void showPopup(int kind) {
		popup = kind;
		popupUntil = time + 1.5;
	}

	float sampleTime = 1.f / 44100.f;
	dsp::ClockDivider controlDivider;
	dsp::ClockDivider lightDivider;
	random::Xoroshiro128Plus rng;

	Harmonion();
	void onReset() override;
	void process(const ProcessArgs& args) override;
	json_t* dataToJson() override;
	void dataFromJson(json_t* root) override;

	void resetSettings();
	void factoryReset();
	Step& currentStep();
	Part& currentPart();
	Preset& editPreset();
	const Preset& audiblePreset();
	void readKnobs();
	void updateHarmony();
	void tick(float dt);
	void handleButtons();
	void pressBig(int b);
	void releaseBig(int b);
	void pressSmall(int b);
	void pressMain(int b);
	void pressOct(int b);
	void pressMatrix(int b);
	void pressSong(int b);
	void pressCalib(int b);
	void menuChange(int delta);
	std::string menuTitle(int item);
	std::string menuValue(int item);
	/** Position of the current value among all values of a menu item, for the indicator */
	int menuValuePos(int item, int* count);
	void enterCalibration();
	bool comboHeld(std::initializer_list<int> big, std::initializer_list<int> small);
	/** Display content, see Screen.hpp */
	void buildScreen(Screen& s);

	void seqClock();
	void seqBeat();
	void seqAdvance(int delta, bool manual);
	void seqReset(bool toPartA);
	void gotoPart(int p);
	void onStepChange();

	uint16_t applyHarmonicShift(uint16_t funcs);
	uint16_t sectionFuncs(int sec, const Preset& p);
	float mixedInput(int sec);
	float inputFor(int sec);
	float followShift(int sec);
	float postProcess(int out, float semis, int sec, bool slewOn, float dt);
	float toVolts(int semis);
	int shuffleNote(int n);
	void processChannel(int c, float dt);
	void processChords(float dt);
	void processArp(float dt);
	void arpStep();
	void processInFuncs();
	void processOutputs(float dt);
	void updateLights();
	void chordsShuffle();

	float potVoltage(int func, float pot);
	bool isBipolar(int func);
	float gateV() const {
		return gate5V ? 5.f : 10.f;
	}
};

}  // namespace hm
