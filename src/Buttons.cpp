#include "Harmonion.hpp"

// Button handling, menu and lights
namespace hm {

static const uint16_t FILLS = 0x1F << FILL1;

void Harmonion::handleButtons() {
	for (int b = 0; b < NUM_BIG; b++) {
		bool down = params[BIG_PARAM + b].getValue() > 0.f;
		if (bigTrig[b].process(down)) {
			bigHeld[b] = true;
			comboUsed[b] = false;
			pressBig(b);
		}
		else if (!down && bigHeld[b]) {
			bigHeld[b] = false;
			releaseBig(b);
		}
	}
	for (int i = 0; i < 12; i++) {
		if (smallTrig[i].process(params[MENU_PARAM + i].getValue() > 0.f))
			pressSmall(i);
	}
}

void Harmonion::pressBig(int b) {
	if (calibrating) {
		pressCalib(b);
		return;
	}
	switch (page) {
		case PAGE_OCT: pressOct(b); break;
		case PAGE_MATRIX: pressMatrix(b); break;
		case PAGE_SONG: pressSong(b); break;
		default: pressMain(b); break;
	}
}

void Harmonion::releaseBig(int b) {
	if (calibrating || page != PAGE_MAIN || comboUsed[b])
		return;
	// PATTERN, BEGIN and INV step through their settings unless used as a shortcut
	Preset& p = editPreset();
	if (b == ARP_BTN + AB_PATTERN) {
		p.pattern = (p.pattern + 1) % 8;
		showPopup(POPUP_PATTERN);
	}
	else if (b == ARP_BTN + AB_BEGIN) {
		p.begin = (p.begin >= NUM_TONES - 1) ? -1 : p.begin + 1;
		showPopup(POPUP_BEGIN);
	}
	else if (b == CHORD_BTN + CB_INV) {
		p.inversion = (p.inversion + 1) % 5;
		showPopup(POPUP_INV);
	}
}

void Harmonion::pressMain(int b) {
	Preset& p = editPreset();
	// Shortcuts: hold PATTERN, BEGIN or INV and press another button of the section
	if (b >= ARP_BTN && b < ARP_BTN + 9) {
		int k = b - ARP_BTN;
		if (bigHeld[ARP_BTN + AB_PATTERN] && k != AB_PATTERN) {
			comboUsed[ARP_BTN + AB_PATTERN] = true;
			p.pattern = (k == AB_BEGIN) ? 7 : k;
			showPopup(POPUP_PATTERN);
			return;
		}
		if (bigHeld[ARP_BTN + AB_BEGIN] && k != AB_BEGIN) {
			comboUsed[ARP_BTN + AB_BEGIN] = true;
			p.begin = (k == AB_PATTERN) ? -1 : k;
			showPopup(POPUP_BEGIN);
			return;
		}
		if (k < NUM_TONES)
			p.arp ^= 1 << k;
		return;
	}
	if (b >= CHORD_BTN && b < CHORD_BTN + 9) {
		int k = b - CHORD_BTN;
		if (bigHeld[CHORD_BTN + CB_INV] && k != CB_INV) {
			comboUsed[CHORD_BTN + CB_INV] = true;
			switch (k) {
				case T1: p.inversion = 4; break;
				case T5: p.inversion = 3; break;
				case T9: p.inversion = 2; break;
				case T13: p.inversion = 1; break;
				default: p.inversion = 0; break;
			}
			showPopup(POPUP_INV);
			return;
		}
		if (k < NUM_TONES)
			p.chords ^= 1 << k;
		else if (k == CB_SLEW)
			p.chordSlew ^= true;
		return;
	}
	if (b == GATE_BTN || b == GATE_BTN + 1) {
		p.gate[b - GATE_BTN] ^= true;
		return;
	}
	for (int t = 0; t < NUM_TONES; t++) {
		if (b == C1A_TONE[t]) {
			p.ch1A ^= 1 << t;
			return;
		}
		if (b == C3_TONE[t]) {
			p.ch3 ^= 1 << t;
			return;
		}
	}
	for (int f = 0; f < NUM_FUNCS; f++) {
		if (b == C2_FUNC[f]) {
			// Single note mode: Channel 2 becomes a little keyboard
			if (settings.ch2Single)
				p.ch2 = 1 << f;
			else
				p.ch2 ^= 1 << f;
			return;
		}
	}
	if (b == C1A_FILL)
		p.ch1Fill ^= true;
	else if (b == C1B_ROOT)
		p.ch1B ^= 1;
	else if (b == C1B_5TH)
		p.ch1B ^= 2;
	else if (b == C1B_SCALE)
		p.ch1B ^= 4;
	else if (b == C1_SLEW)
		p.ch1Slew ^= true;
	else if (b == C3_EQDIST)
		p.eqDist ^= true;
}

void Harmonion::pressOct(int b) {
	if (b == GATE_BTN + 1) {
		page = PAGE_MATRIX;
		return;
	}
	Preset& p = editPreset();
	for (int s = 0; s < NUM_SECTIONS; s++) {
		const OctMap& m = OCT_MAP[s];
		if (b == m.up)
			p.octave[s] = std::min(3, p.octave[s] + 1);
		else if (b == m.down)
			p.octave[s] = std::max(-3, p.octave[s] - 1);
		else if (b == m.follow)
			settings.follow[s] ^= true;
		else if (b == m.scaling)
			settings.inScale[s] = (settings.inScale[s] + 1) % 4;
		else if (s < 3 && b == m.bypass)
			settings.bypass[s] = (settings.bypass[s] + 1) % 3;
		else if (s < 3 && b == m.glide)
			settings.glide[s] ^= true;
	}
}

void Harmonion::pressMatrix(int b) {
	if (b == GATE_BTN) {
		page = PAGE_OCT;
		return;
	}
	if (b >= NUM_TOP)
		return;
	int row = b / 8, col = b % 8;
	if (col < 4)
		settings.inMix[row][col] ^= true;
	else
		settings.outSrc[col - 4] = row;  // Switching, not mixing: one source per output
}

void Harmonion::pressSong(int b) {
	if (b < NUM_TOP) {
		if (b < (int) currentPart().steps.size()) {
			step = b;
			beat = 0;
			if (playing)
				waitFirstClock = true;
			outPulse[OUT_SONG_STEP].trigger();
			onStepChange();
		}
		return;
	}
	int p = -1;
	if (b == GATE_BTN)
		p = 0;
	else if (b == GATE_BTN + 1)
		p = 1;
	else if (b == CHORD_BTN + T13)
		p = 2;
	else if (b == CHORD_BTN + CB_INV)
		p = 3;
	else if (b == ARP_BTN + AB_BEGIN)
		settings.liveMode ^= true;
	if (p < 0)
		return;
	// First press: switch at the end of the current part. Second press: switch now.
	if (pendingPart == p)
		gotoPart(p);
	else
		pendingPart = p;
}

void Harmonion::pressCalib(int b) {
	float& v = calib[calibLast][calib5V ? 1 : 0];
	if (b < NUM_TOP) {
		static const int delta[4] = {50, 1, -1, -50};
		calibLast = b % 8;
		calib[calibLast][calib5V ? 1 : 0] += delta[b / 8] * DAC_UNIT_V;
	}
	else if (b == GATE_BTN) {
		v -= 1000 * DAC_UNIT_V;
	}
	else if (b == GATE_BTN + 1) {
		v += 1000 * DAC_UNIT_V;
	}
}

void Harmonion::pressSmall(int i) {
	enum { MENU, TUNE, SONG, OCT, SEQ, LEFT, RIGHT, PLAY, DEL, MINUS, PLUS, ADD };
	if (calibrating) {
		if (i == MENU)
			calibrating = false;
		else if (i == TUNE)
			calib5V ^= true;
		return;
	}
	switch (i) {
		case MENU:
			menuOpen ^= true;
			resetArmed = false;
			break;
		case TUNE:
			tuning ^= true;
			break;
		case SONG:
			if (seqMode && !syncSlave)
				page = (page == PAGE_SONG) ? PAGE_MAIN : PAGE_SONG;
			break;
		case OCT:
			page = (page == PAGE_OCT || page == PAGE_MATRIX) ? PAGE_MAIN : PAGE_OCT;
			break;
		case SEQ:
			if (syncSlave)
				break;
			seqMode ^= true;
			frozen = false;
			lastSongCv = -1;
			if (seqMode) {
				step = 0;
				part = 0;
				beat = 0;
				clockIdx = 0;
				waitFirstClock = true;
				onStepChange();
			}
			else if (page == PAGE_SONG) {
				page = PAGE_MAIN;
			}
			break;
		case LEFT:
		case RIGHT: {
			int d = (i == RIGHT) ? 1 : -1;
			if (menuOpen) {
				menuItem = wrap(menuItem + d, NUM_MENU_ITEMS);
				resetArmed = false;
			}
			else if (tuning) {
				tuneOctave = clamp(tuneOctave + d, -2, 8);
			}
			else if (seqMode && !syncSlave) {
				seqAdvance(d, true);
			}
		} break;
		case PLAY:
			if (!seqMode || syncSlave)
				break;
			if (time - lastPlayPress < 0.4) {
				// Double click: freeze (the first click already toggled play)
				playing = !playing;
				if (!frozen) {
					frozenPreset = audiblePreset();
					// Only the step values are frozen; CV modifications stay live
					Step& s = currentStep();
					frozenRoot = s.root;
					frozenMode = s.mode;
					frozenDegree = s.degree;
					frozen = true;
				}
				lastPlayPress = -10.0;
				break;
			}
			lastPlayPress = time;
			if (frozen) {
				frozen = false;
				playing = true;
				waitFirstClock = true;
			}
			else {
				playing = !playing;
			}
			break;
		case MINUS:
		case PLUS: {
			int d = (i == PLUS) ? 1 : -1;
			if (menuOpen) {
				menuChange(d);
			}
			else if (seqMode && !syncSlave) {
				Step& s = currentStep();
				// Above 32 beats the buttons jump in larger steps
				int stepSize = (d > 0 ? s.beats >= 32 : s.beats > 32) ? 4 : 1;
				s.beats = clamp(s.beats + d * stepSize, 1, MAX_BEATS);
			}
		} break;
		case DEL:
			if (seqMode && !syncSlave && !menuOpen) {
				Part& p = currentPart();
				if (p.steps.size() <= 1)
					p.steps[0] = Step();
				else
					p.steps.erase(p.steps.begin() + step);
				step = std::min<int>(step, p.steps.size() - 1);
				onStepChange();
			}
			break;
		case ADD:
			if (seqMode && !syncSlave && !menuOpen) {
				Part& p = currentPart();
				if ((int) p.steps.size() < MAX_STEPS) {
					Step copy = currentStep();
					p.steps.insert(p.steps.begin() + step + 1, copy);
					step++;
					onStepChange();
				}
			}
			break;
	}
}

void Harmonion::menuChange(int d) {
	switch (menuItem) {
		case MI_DISPLAY: settings.display = wrap(settings.display + d, NUM_DISP_MODES); break;
		case MI_CV1: case MI_CV2: case MI_CV3: case MI_CV4: {
			int& f = settings.cvFunc[menuItem - MI_CV1];
			f = wrap(f + d, NUM_CV_FUNCS);
		} break;
		case MI_POT1: case MI_POT2: {
			int k = menuItem - MI_POT1;
			int& f = settings.potFunc[k];
			// Pots cannot act as voltmeter, and both pots cannot share a function
			do {
				f = wrap(f + d, NUM_CV_FUNCS);
			} while (f == CV_VOLTMETER || (f != CV_NONE && f == settings.potFunc[1 - k]));
		} break;
		case MI_IN1: case MI_IN2: {
			int k = menuItem - MI_IN1;
			int& f = settings.inFunc[k];
			do {
				f = wrap(f + d, NUM_IN_FUNCS);
			} while (k == 1 && f == IN_SYNC_SLAVE);
		} break;
		case MI_OUT1: case MI_OUT2: {
			int k = menuItem - MI_OUT1;
			int& f = settings.outFunc[k];
			do {
				f = wrap(f + d, NUM_OUT_FUNCS);
			} while (k == 1 && f == OUT_SYNC_MASTER);
		} break;
		case MI_CLOCK_DIV:
			if (seqMode)
				currentPart().clockDiv = clamp(currentPart().clockDiv + d, 0, NUM_CLOCK_DIVS - 1);
			else
				settings.nonSeqClockDiv = clamp(settings.nonSeqClockDiv + d, 0, NUM_CLOCK_DIVS - 1);
			break;
		case MI_ARP_CLOCK: settings.arpClock = clamp(settings.arpClock + d, 0, 15); break;
		case MI_ARP_DELAY: settings.arpDelayMs = clamp(settings.arpDelayMs + d, 0, 20); break;
		case MI_CH1_AB: settings.abMode = wrap(settings.abMode + d, NUM_AB_MODES); break;
		case MI_CH2_BUTTONS: settings.ch2Single ^= true; break;
		case MI_TRIG_WINDOW: settings.trigWindowMs = clamp(settings.trigWindowMs + d, 0, 20); break;
		case MI_SLEW_CURVE: settings.slewExp ^= true; break;
		case MI_INTONATION: settings.intonation = wrap(settings.intonation + d, NUM_INTONATIONS); break;
		case MI_BEND_DELAY: settings.bendDelay = clamp(settings.bendDelay + d, 0, 3); break;
		case MI_RESET:
			// Needs + two times in a row
			if (d > 0) {
				if (resetArmed) {
					resetSettings();
					resetArmed = false;
				}
				else {
					resetArmed = true;
				}
			}
			else {
				resetArmed = false;
			}
			break;
	}
}

std::string Harmonion::menuTitle(int item) {
	static const char* const titles[NUM_MENU_ITEMS] = {
		"Display", "CV 1", "CV 2", "CV 3", "CV 4", "Pot 1", "Pot 2", "In 1", "In 2", "Out 1", "Out 2",
		"Clock Divider", "Arpeggio Clock", "Arp. Trigger Delay", "Channel 1 A/B", "Channel 2 Buttons",
		"Trigger Time Window", "Slew Curve", "Intonation", "Pitch Bend Delay", "Reset To Defaults",
	};
	return titles[clamp(item, 0, NUM_MENU_ITEMS - 1)];
}

std::string Harmonion::menuValue(int item) {
	switch (item) {
		case MI_DISPLAY: return DISPLAY_MODE_NAMES[settings.display];
		case MI_CV1: case MI_CV2: case MI_CV3: case MI_CV4: return CV_FUNC_NAMES[settings.cvFunc[item - MI_CV1]];
		case MI_POT1: case MI_POT2: return CV_FUNC_NAMES[settings.potFunc[item - MI_POT1]];
		case MI_IN1: case MI_IN2: return IN_FUNC_NAMES[settings.inFunc[item - MI_IN1]];
		case MI_OUT1: case MI_OUT2: return OUT_FUNC_NAMES[settings.outFunc[item - MI_OUT1]];
		case MI_CLOCK_DIV:
			if (seqMode)
				return string::f("%d (for %d%c)", CLOCK_DIVS[currentPart().clockDiv], song + 1, 'A' + part);
			return string::f("%d", CLOCK_DIVS[settings.nonSeqClockDiv]);
		case MI_ARP_CLOCK: {
			std::string s = string::f("%d/%d", ARP_RATIO[settings.arpClock][0], ARP_RATIO[settings.arpClock][1]);
			if (funcActive[CV_ARP_CLOCK]) {
				int r = clamp(settings.arpClock + (int) std::round(funcCv[CV_ARP_CLOCK] * 12.f), 0, 15);
				s += string::f(" (via CV: %d/%d)", ARP_RATIO[r][0], ARP_RATIO[r][1]);
			}
			return s;
		}
		case MI_ARP_DELAY: return string::f("%d ms", settings.arpDelayMs);
		case MI_CH1_AB: return AB_MODE_NAMES[settings.abMode];
		case MI_CH2_BUTTONS: return settings.ch2Single ? "Single Note" : "Normal";
		case MI_TRIG_WINDOW: return string::f("%d ms", settings.trigWindowMs);
		case MI_SLEW_CURVE: return settings.slewExp ? "Exponential" : "Linear";
		case MI_INTONATION: return INTONATION_MODE_NAMES[settings.intonation];
		case MI_BEND_DELAY: return BEND_DELAY_MODE_NAMES[settings.bendDelay];
		case MI_RESET: return resetArmed ? "Press + again!" : "Press + twice";
	}
	return "";
}

void Harmonion::updateLights() {
	const float DIM = 0.15f, ON = 1.f;
	float l[NUM_BIG] = {};
	bool blink = std::fmod(time, 0.5) < 0.25;
	const Preset& p = editPreset();
	const Preset& a = audiblePreset();

	if (calibrating) {
		for (int b = 0; b < NUM_TOP; b++)
			l[b] = (b % 8 == calibLast) ? ON : DIM;
		l[GATE_BTN] = l[GATE_BTN + 1] = DIM;
	}
	else if (page == PAGE_OCT) {
		for (int s = 0; s < NUM_SECTIONS; s++) {
			const OctMap& m = OCT_MAP[s];
			int o = p.octave[s];
			l[m.up] = (o >= 0) ? ON : DIM;
			l[m.down] = (o <= 0) ? ON : DIM;
			l[m.follow] = settings.follow[s] ? ON : DIM;
			l[m.scaling] = settings.inScale[s] == 0 ? ON : DIM;
			if (s < 3) {
				l[m.bypass] = settings.bypass[s] == 0 ? ON : settings.bypass[s] == 1 ? DIM : 0.f;
				l[m.glide] = settings.glide[s] ? ON : 0.f;
			}
		}
		l[GATE_BTN] = ON;
		l[GATE_BTN + 1] = DIM;
	}
	else if (page == PAGE_MATRIX) {
		for (int b = 0; b < NUM_TOP; b++) {
			int row = b / 8, col = b % 8;
			bool on = (col < 4) ? settings.inMix[row][col] : settings.outSrc[col - 4] == row;
			l[b] = on ? ON : 0.f;
		}
		l[GATE_BTN] = DIM;
		l[GATE_BTN + 1] = ON;
	}
	else if (page == PAGE_SONG) {
		int n = currentPart().steps.size();
		for (int b = 0; b < NUM_TOP; b++)
			l[b] = (b == step) ? ON : (b < n) ? DIM : 0.f;
		const int partBtn[4] = {GATE_BTN, GATE_BTN + 1, CHORD_BTN + T13, CHORD_BTN + CB_INV};
		for (int k = 0; k < 4; k++)
			l[partBtn[k]] = (k == pendingPart) ? (blink ? ON : 0.f) : (k == part) ? ON : DIM;
		l[ARP_BTN + AB_BEGIN] = settings.liveMode ? ON : DIM;
	}
	else {
		// Selected notes are dim, the note being played is bright
		const ChannelState& c1 = ch[0];
		uint16_t aFuncs = p.ch1A | (p.ch1Fill ? FILLS : 0);
		bool bActive = c1.usingB && p.ch1B;
		for (int t = 0; t < NUM_TONES; t++) {
			bool sel = p.ch1A & (1 << t);
			l[C1A_TONE[t]] = sel ? ((!bActive && c1.outFunc == t) ? ON : DIM) : 0.f;
			l[C3_TONE[t]] = (p.ch3 & (1 << t)) ? (ch[2].outFunc == t ? ON : DIM) : 0.f;
			l[ARP_BTN + t] = (p.arp & (1 << t)) ? (arpFunc == t ? ON : DIM) : 0.f;
			l[CHORD_BTN + t] = (p.chords & (1 << t)) ? ((chordPicked & (1 << t)) ? ON : DIM) : 0.f;
		}
		(void) aFuncs;
		l[C1A_FILL] = p.ch1Fill ? ((!bActive && c1.outFunc >= FILL1) ? ON : DIM) : 0.f;
		l[C1B_ROOT] = (p.ch1B & 1) ? ((bActive && c1.outFunc == T1) ? ON : DIM) : 0.f;
		l[C1B_5TH] = (p.ch1B & 2) ? ((bActive && c1.outFunc == T5) ? ON : DIM) : 0.f;
		l[C1B_SCALE] = (p.ch1B & 4) ? ((bActive && c1.outFunc >= 0 && c1.outFunc < NUM_TONES) ? ON : DIM) : 0.f;
		l[C1_SLEW] = p.ch1Slew ? ON : 0.f;
		for (int f = 0; f < NUM_FUNCS; f++)
			l[C2_FUNC[f]] = (p.ch2 & (1 << f)) ? (ch[1].outFunc == f ? ON : DIM) : 0.f;
		l[C3_EQDIST] = p.eqDist ? ON : 0.f;
		l[CHORD_BTN + CB_INV] = p.inversion ? ON : 0.f;
		l[CHORD_BTN + CB_SLEW] = p.chordSlew ? ON : 0.f;
		l[ARP_BTN + AB_PATTERN] = bigHeld[ARP_BTN + AB_PATTERN] ? ON : DIM;
		l[ARP_BTN + AB_BEGIN] = p.begin >= 0 ? ON : 0.f;
		l[GATE_BTN] = p.gate[0] ? ON : 0.f;
		l[GATE_BTN + 1] = p.gate[1] ? ON : 0.f;
	}
	(void) a;
	for (int b = 0; b < NUM_BIG; b++)
		lights[BIG_LIGHT + b].setBrightness(l[b]);

	// Small buttons
	bool pressed[12];
	for (int i = 0; i < 12; i++)
		pressed[i] = params[MENU_PARAM + i].getValue() > 0.f;
	lights[SMALL_LIGHT + 0].setBrightness(menuOpen || calibrating ? ON : 0.f);
	lights[SMALL_LIGHT + 1].setBrightness(calibrating ? (blink ? ON : 0.f) : (tuning ? ON : 0.f));
	lights[SMALL_LIGHT + 2].setBrightness(page == PAGE_SONG ? ON : (seqMode ? DIM : 0.f));
	lights[SMALL_LIGHT + 3].setBrightness(page == PAGE_OCT || page == PAGE_MATRIX ? ON : 0.f);
	lights[SMALL_LIGHT + 4].setBrightness(seqMode ? ON : 0.f);
	lights[SMALL_LIGHT + 7].setBrightness(!seqMode ? 0.f : frozen ? (blink ? ON : 0.f) : playing ? ON : DIM);
	for (int i : {5, 6, 8, 9, 10, 11})
		lights[SMALL_LIGHT + i].setBrightness(pressed[i] ? ON : (seqMode || menuOpen) ? DIM : 0.f);

	// Jack LEDs
	for (int c = 0; c < 3; c++)
		lights[TRIG_LIGHT + c].setBrightness(inputs[CH_TRIG_INPUT + c].getVoltage() > 1.f ? ON : 0.f);
	lights[TRIG_LIGHT + 3].setBrightness(inputs[CHORD_TRIG_INPUT].getVoltage() > 1.f ? ON : 0.f);
	lights[AB_LIGHT].setBrightness(ch[0].usingB ? ON : 0.f);
	for (int i = 0; i < 2; i++) {
		lights[IN_LIGHT + i].setBrightness(inHigh[i] ? ON : 0.f);
		lights[OUT_LIGHT + i].setBrightness(outputs[OUT_OUTPUT + i].getVoltage() > 1.f ? ON : 0.f);
		lights[GATE_LIGHT + i].setBrightness(a.gate[i] ? ON : 0.f);
	}
	lights[ARP_CLK_LIGHT].setBrightness(clkLight[0].remaining > 0.f ? ON : 0.f);
	lights[SEQ_CLK_LIGHT].setBrightness(clkLight[1].remaining > 0.f ? ON : 0.f);
	float dt = lightDivider.getDivision() * sampleTime;
	clkLight[0].process(dt);
	clkLight[1].process(dt);
}

}  // namespace hm
