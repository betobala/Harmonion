#include "Harmony.hpp"
#include <cmath>
#include <algorithm>

namespace harmony {

int wrap(int x, int n) {
	int r = x % n;
	return r < 0 ? r + n : r;
}

// Seven-note scale given as ascending intervals: ROOT 9 3 11 5 13 7
static Scale diatonic(const char* simple, const char* jazz, std::initializer_list<int> iv) {
	std::vector<int> s(iv);
	Scale sc = {simple, jazz, {}, -1, ""};
	const int slot[7] = {T1, T9, T3, T11, T5, T13, T7};
	for (int i = 0; i < NUM_TONES; i++) {
		sc.tone[i][0] = -1;
		sc.tone[i][1] = -1;
	}
	for (int i = 0; i < 7; i++)
		sc.tone[slot[i]][0] = s[i];
	return sc;
}

// Explicit mapping in panel order: 1 3 5 7 9 11 13. A second note may follow with b.
struct T {
	int a;
	int b = -1;
	T(int a) : a(a) {}
	T(int a, int b) : a(a), b(b) {}
};
static Scale explicitScale(const char* simple, const char* jazz, T t1, T t3, T t5, T t7, T t9, T t11, T t13) {
	Scale sc = {simple, jazz, {}, -1, ""};
	T t[7] = {t1, t3, t5, t7, t9, t11, t13};
	for (int i = 0; i < NUM_TONES; i++) {
		sc.tone[i][0] = t[i].a;
		sc.tone[i][1] = t[i].b;
	}
	return sc;
}

// Slash chord: triad (root, 3rd, 5th offsets) above the bass. 3RD/5TH/7TH play the triad,
// 9TH/11TH/13TH fill the gaps of the parent scale in ascending order.
static Scale slash(int triadRoot, const char* quality, int third, int fifth, std::initializer_list<int> rest) {
	std::vector<int> r(rest);
	std::sort(r.begin(), r.end());
	Scale sc = explicitScale("", "", 0, wrap(triadRoot, 12), wrap(triadRoot + third, 12), wrap(triadRoot + fifth, 12), r[0], r[1], r[2]);
	sc.slashRoot = triadRoot;
	sc.slashQuality = quality;
	return sc;
}

enum ScaleId {
	// Mode "Chords"
	LYD, MAJ, DOM, SUS, ALT, HM5, DOR, MIN, HM, PHR, DIM, AUG,
	// Mode "Chords2"
	DOM_S11, DOM_B13, SUS_B9, HALFWHOLE, MELMIN, LYDAUG, LOC,
	// Remaining modes of melodic minor, harmonic minor/major and double harmonic major
	DORB2, LOCS2,
	LOC6, IONAUG, DORS4, LYDS2, ULTRALOC,
	HARMAJ, DORB5, PHRB4, LYDB3, MIXB2, LYDAUGS2, LOCBB7,
	DHM, LYDS2S6, ULTRAPHR, HUNGMIN, ORIENTAL, IONS2S5, LOCBB3,
	// Slash chords
	SL_FIRST
};

const std::vector<Scale>& scales() {
	static const std::vector<Scale> s = [] {
		std::vector<Scale> v;
		v.push_back(diatonic("lyd", "maj7#11", {0, 2, 4, 6, 7, 9, 11}));
		v.push_back(diatonic("maj", "maj7", {0, 2, 4, 5, 7, 9, 11}));
		v.push_back(diatonic("7", "7", {0, 2, 4, 5, 7, 9, 10}));
		// sus: mixolydian with 3RD and 11TH swapped
		v.push_back(explicitScale("sus", "7sus4", 0, 5, 7, 10, 2, 4, 9));
		// reconstruído: altered scale laid out as a dominant chord with #5
		v.push_back(explicitScale("alt", "7alt", 0, 4, 8, 10, 1, 6, 3));
		v.push_back(diatonic("hm5", "7b9b13", {0, 1, 4, 5, 7, 8, 10}));
		v.push_back(diatonic("dor", "m7", {0, 2, 3, 5, 7, 9, 10}));
		v.push_back(diatonic("min", "m7b13", {0, 2, 3, 5, 7, 8, 10}));
		v.push_back(diatonic("hm", "m-maj7", {0, 2, 3, 5, 7, 8, 11}));
		v.push_back(diatonic("phr", "m7b9", {0, 1, 3, 5, 7, 8, 10}));
		// Diminished (whole-half): 13TH selects two notes
		v.push_back(explicitScale("dim", "dim7", 0, 3, 6, 9, 2, 5, T(8, 11)));
		// Augmented (whole tone): six notes, 13TH unused
		v.push_back(explicitScale("aug", "7#5", 0, 4, 8, 10, 2, 6, -1));

		v.push_back(diatonic("7#11", "7#11", {0, 2, 4, 6, 7, 9, 10}));
		v.push_back(diatonic("7b13", "7b13", {0, 2, 4, 5, 7, 8, 10}));
		v.push_back(explicitScale("sus4b9", "7sus4b9", 0, 5, 7, 10, 1, 4, 9));
		// Half-whole: 9TH selects b9 and #9
		v.push_back(explicitScale("7#9", "7#9", 0, 4, 7, 10, T(1, 3), 6, 9));
		v.push_back(diatonic("-maj7", "m-maj7", {0, 2, 3, 5, 7, 9, 11}));
		v.push_back(diatonic("maj7#5", "maj7#5", {0, 2, 4, 6, 8, 9, 11}));
		v.push_back(diatonic("-7b5", "m7b5", {0, 1, 3, 5, 6, 8, 10}));

		v.push_back(diatonic("dorb9", "m7b9", {0, 1, 3, 5, 7, 9, 10}));
		v.push_back(diatonic("loc#2", "m9b5", {0, 2, 3, 5, 6, 8, 10}));

		v.push_back(diatonic("loc6", "m7b5b9", {0, 1, 3, 5, 6, 9, 10}));
		v.push_back(diatonic("ion#5", "maj7#5", {0, 2, 4, 5, 8, 9, 11}));
		v.push_back(diatonic("dor#11", "m7#11", {0, 2, 3, 6, 7, 9, 10}));
		v.push_back(diatonic("lyd#9", "maj7#9", {0, 3, 4, 6, 7, 9, 11}));
		v.push_back(explicitScale("ultraloc", "dim7b9", 0, 3, 6, 9, 1, 4, 8));

		v.push_back(diatonic("hmaj", "maj7b13", {0, 2, 4, 5, 7, 8, 11}));
		v.push_back(diatonic("dorb5", "m7b5", {0, 2, 3, 5, 6, 9, 10}));
		v.push_back(explicitScale("phrb4", "7b9#5", 0, 4, 8, 10, 1, 3, 5));
		v.push_back(diatonic("lydb3", "m-maj7#11", {0, 2, 3, 6, 7, 9, 11}));
		v.push_back(diatonic("mixb9", "7b9", {0, 1, 4, 5, 7, 9, 10}));
		v.push_back(explicitScale("lydaug#9", "maj7#5#9", 0, 4, 8, 11, 3, 6, 9));
		v.push_back(diatonic("locbb7", "dim7b9", {0, 1, 3, 5, 6, 8, 9}));

		v.push_back(diatonic("dhm", "maj7b9b13", {0, 1, 4, 5, 7, 8, 11}));
		v.push_back(diatonic("lyd#2#6", "maj7#9#13", {0, 3, 4, 6, 7, 10, 11}));
		v.push_back(diatonic("ultraphr", "m6b9", {0, 1, 3, 4, 7, 8, 9}));
		v.push_back(diatonic("hungmin", "m-maj7#11", {0, 2, 3, 6, 7, 8, 11}));
		v.push_back(explicitScale("oriental", "7b5b9", 0, 4, 6, 10, 1, 5, 9));
		v.push_back(explicitScale("ion#2#5", "maj7#5#9", 0, 4, 8, 11, 3, 5, 9));
		v.push_back(diatonic("locbb3", "dim7b9", {0, 1, 2, 5, 6, 8, 9}));

		// reconstruído: slash chords whose triad does not contain the bass
		v.push_back(slash(2, "", 4, 7, {4, 7, 11}));    // D/C
		v.push_back(slash(10, "", 4, 7, {4, 7, 9}));    // Bb/C
		v.push_back(slash(1, "", 4, 7, {3, 7, 10}));    // Db/C
		v.push_back(slash(4, "", 4, 7, {2, 6, 9}));     // E/C
		v.push_back(slash(7, "m", 3, 7, {4, 5, 9}));    // Gm/C
		v.push_back(slash(9, "", 4, 7, {3, 7, 10}));    // A/C
		v.push_back(slash(6, "", 4, 7, {4, 7, 9}));     // F#/C
		v.push_back(slash(11, "", 4, 7, {2, 5, 8}));    // B/C
		v.push_back(slash(2, "m", 3, 7, {4, 7, 11}));   // Dm/C
		v.push_back(slash(8, "m", 3, 7, {2, 5, 7}));    // Abm/C
		v.push_back(slash(10, "m", 3, 7, {3, 7, 8}));   // Bbm/C
		v.push_back(slash(3, "m", 3, 7, {2, 5, 8}));    // Ebm/C
		return v;
	}();
	return s;
}

// Seven degrees of a cadence plus five lydian chords on the non-scale notes
static Mode cadence(const char* name, std::initializer_list<int> offsets, std::initializer_list<int> ids) {
	Mode m = {name, {}};
	std::vector<int> o(offsets), id(ids);
	for (int pc = 0; pc < 12; pc++) {
		auto it = std::find(o.begin(), o.end(), pc);
		if (it != o.end())
			m.degrees.push_back({(int8_t) pc, (int16_t) id[it - o.begin()]});
		else
			m.degrees.push_back({(int8_t) pc, (int16_t) LYD});
	}
	return m;
}

const std::vector<Mode>& modes() {
	static const std::vector<Mode> m = [] {
		std::vector<Mode> v;
		Mode chords = {"Chords", {}};
		for (int i = LYD; i <= AUG; i++)
			chords.degrees.push_back({0, (int16_t) i});
		v.push_back(chords);
		Mode chords2 = {"Chords2", {}};
		for (int i = DOM_S11; i <= LOC; i++)
			chords2.degrees.push_back({0, (int16_t) i});
		v.push_back(chords2);
		Mode sl = {"Slashch.", {}};
		for (int i = SL_FIRST; i < (int) scales().size(); i++)
			sl.degrees.push_back({0, (int16_t) i});
		v.push_back(sl);
		// reconstruído: extra chords on the five non-scale degrees
		Mode major = {"Major", {}};
		const int majorIds[12] = {MAJ, LYD, DOR, LYD, PHR, LYD, ALT, DOM, LYD, MIN, DOM_S11, LOC};
		for (int pc = 0; pc < 12; pc++)
			major.degrees.push_back({(int8_t) pc, (int16_t) majorIds[pc]});
		v.push_back(major);
		Mode minor = {"Minor", {}};
		const int minorIds[12] = {MIN, LYD, LOC, MAJ, LYD, DOR, LOC, PHR, LYD, LOC, DOM, DIM};
		for (int pc = 0; pc < 12; pc++)
			minor.degrees.push_back({(int8_t) pc, (int16_t) minorIds[pc]});
		v.push_back(minor);
		v.push_back(cadence("Mel.Minor", {0, 2, 3, 5, 7, 9, 11}, {MELMIN, DORB2, LYDAUG, DOM_S11, DOM_B13, LOCS2, ALT}));
		v.push_back(cadence("Har.Minor", {0, 2, 3, 5, 7, 8, 11}, {HM, LOC6, IONAUG, DORS4, HM5, LYDS2, ULTRALOC}));
		v.push_back(cadence("Har.Major", {0, 2, 4, 5, 7, 8, 11}, {HARMAJ, DORB5, PHRB4, LYDB3, MIXB2, LYDAUGS2, LOCBB7}));
		v.push_back(cadence("Db.H.Maj", {0, 1, 4, 5, 7, 8, 11}, {DHM, LYDS2S6, ULTRAPHR, HUNGMIN, ORIENTAL, IONS2S5, LOCBB3}));
		return v;
	}();
	return m;
}

void Harmony::set(int keyRoot, int mode, int degree) {
	const auto& ms = modes();
	this->keyRoot = wrap(keyRoot, 12);
	this->mode = wrap(mode, ms.size());
	const Mode& m = ms[this->mode];
	this->degree = wrap(degree, m.degrees.size());
	const Degree& d = m.degrees[this->degree];
	chordRoot = wrap(this->keyRoot + d.rootOffset, 12);
	scale = &scales()[d.scale];

	uint16_t used = 0;
	numScaleNotes = 0;
	for (int f = 0; f < NUM_TONES; f++) {
		for (int k = 0; k < 2; k++) {
			int o = scale->tone[f][k];
			notes[f][k] = (o < 0) ? -1 : wrap(chordRoot + o, 12);
			if (o >= 0) {
				if (!(used & (1 << wrap(o, 12))))
					numScaleNotes++;
				used |= 1 << wrap(o, 12);
			}
		}
	}
	// Fill notes: the remaining semitones, low to high. Six of them for six-note scales,
	// in which case FILL5 takes the last two.
	int f = FILL1;
	for (int f2 = FILL1; f2 <= FILL5; f2++)
		notes[f2][0] = notes[f2][1] = -1;
	for (int o = 0; o < 12; o++) {
		if (used & (1 << o))
			continue;
		int pc = wrap(chordRoot + o, 12);
		if (f <= FILL5) {
			notes[f][0] = pc;
			f++;
		}
		else {
			notes[FILL5][1] = pc;
		}
	}
}

uint16_t Harmony::pitchMask(uint16_t funcMask) const {
	uint16_t m = 0;
	for (int f = 0; f < NUM_FUNCS; f++) {
		if (!(funcMask & (1 << f)))
			continue;
		for (int k = 0; k < 2; k++)
			if (notes[f][k] >= 0)
				m |= 1 << notes[f][k];
	}
	return m;
}

int Harmony::funcOfPitch(int pc) const {
	pc = wrap(pc, 12);
	for (int f = 0; f < NUM_FUNCS; f++)
		for (int k = 0; k < 2; k++)
			if (notes[f][k] == pc)
				return f;
	return -1;
}

static const char* const SHARP_NAMES[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char* const FLAT_NAMES[12] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};

std::string noteName(int pc, bool flat) {
	return (flat ? FLAT_NAMES : SHARP_NAMES)[wrap(pc, 12)];
}

std::string pitchName(float voltage) {
	int semi = (int) std::round(voltage * 12.f);
	int oct = (int) std::floor(semi / 12.f);
	return noteName(semi, false) + std::to_string(oct);
}

bool preferFlats(int chordRoot, const Scale& scale) {
	// Spell the scale with one letter per note starting at both enharmonic spellings of the
	// root and keep the one with fewer accidentals
	static const int natural[7] = {0, 2, 4, 5, 7, 9, 11};
	std::vector<int> pcs;
	for (int f = 0; f < NUM_TONES; f++)
		if (scale.tone[f][0] >= 0)
			pcs.push_back(wrap(scale.tone[f][0], 12));
	std::sort(pcs.begin(), pcs.end());
	int bestCost = 1 << 30;
	bool bestFlat = false;
	for (int letter = 0; letter < 7; letter++) {
		int acc = wrap(chordRoot - natural[letter] + 6, 12) - 6;
		if (std::abs(acc) > 1)
			continue;
		int c = 0, flats = 0, sharps = 0;
		for (size_t i = 0; i < pcs.size(); i++) {
			int l = (letter + (int) i) % 7;
			int pc = wrap(chordRoot + pcs[i], 12);
			int d = wrap(pc - natural[l] + 6, 12) - 6;
			c += std::abs(d) + (std::abs(d) > 1 ? 10 : 0);
			if (d < 0)
				flats++;
			if (d > 0)
				sharps++;
		}
		if (c < bestCost) {
			bestCost = c;
			bestFlat = flats > sharps;
		}
	}
	return bestFlat;
}

std::string Harmony::chordName() const {
	bool flat = preferFlats(chordRoot, *scale);
	if (scale->slashRoot >= 0)
		return noteName(chordRoot + scale->slashRoot, flat) + scale->slashQuality + "/" + noteName(chordRoot, flat);
	const char* suffix = (mode == 0) ? scale->simple : scale->jazz;
	return noteName(chordRoot, flat) + suffix;
}

std::string Harmony::modeName() const {
	return modes()[mode].name;
}

std::string Harmony::scaleTones() const {
	bool flat = preferFlats(chordRoot, *scale);
	std::vector<int> offs;
	for (int f = 0; f < NUM_TONES; f++)
		for (int k = 0; k < 2; k++)
			if (scale->tone[f][k] >= 0)
				offs.push_back(wrap(scale->tone[f][k], 12));
	std::sort(offs.begin(), offs.end());
	std::string s;
	for (int o : offs) {
		if (!s.empty())
			s += " ";
		s += noteName(chordRoot + o, flat);
	}
	return s;
}

std::string Harmony::scaleStructure() const {
	uint16_t m = 0;
	for (int f = 0; f < NUM_TONES; f++)
		for (int k = 0; k < 2; k++)
			if (scale->tone[f][k] >= 0)
				m |= 1 << wrap(scale->tone[f][k], 12);
	std::string s;
	for (int o = 0; o <= 12; o++)
		s += (m & (1 << (o % 12))) ? "o" : "~";
	return s;
}

const std::vector<uint8_t>& voicings() {
	// Lexicographic order over 1 3 5 7 9 11 13: all 4-note, then 3-, 2- and 1-note voicings
	static const std::vector<uint8_t> v = [] {
		std::vector<uint8_t> r;
		r.push_back(0);
		for (int size = 4; size >= 1; size--) {
			std::vector<int> idx(size);
			for (int i = 0; i < size; i++)
				idx[i] = i;
			while (true) {
				uint8_t m = 0;
				for (int i : idx)
					m |= 1 << i;
				r.push_back(m);
				int i = size - 1;
				while (i >= 0 && idx[i] == 7 - size + i)
					i--;
				if (i < 0)
					break;
				idx[i]++;
				for (int j = i + 1; j < size; j++)
					idx[j] = idx[j - 1] + 1;
			}
		}
		return r;
	}();
	return v;
}

float intonationCents(int intonation, int semi) {
	semi = wrap(semi, 12);
	static const double justRatio[12] = {1., 16. / 15., 9. / 8., 6. / 5., 5. / 4., 4. / 3., 45. / 32., 3. / 2., 8. / 5., 5. / 3., 9. / 5., 15. / 8.};
	static const float meantone[12] = {0.f, 76.0f, 193.2f, 310.3f, 386.3f, 503.4f, 579.5f, 696.6f, 772.6f, 889.7f, 1006.8f, 1082.9f};
	switch (intonation) {
		case JUST: return (float) (1200. * std::log2(justRatio[semi]));
		case MEANTONE: return meantone[semi];
		default: return 100.f * semi;
	}
}

}  // namespace harmony
