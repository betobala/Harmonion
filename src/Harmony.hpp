#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Harmonic system: scales are seven (sometimes six or eight) notes mapped onto the chord
// functions ROOT, 3RD, 5TH, 7TH, 9TH, 11TH, 13TH. The notes missing from the scale become
// FILL1..FILL5, ordered from low to high.
namespace harmony {

// Chord tone functions in panel order, followed by the fill notes
enum Func {
	T1, T3, T5, T7, T9, T11, T13,
	FILL1, FILL2, FILL3, FILL4, FILL5,
	NUM_FUNCS
};
static const int NUM_TONES = 7;

struct Scale {
	const char* simple;  // Name used in the "Chords" mode, e.g. "lyd"
	const char* jazz;    // Name used in all other modes, e.g. "maj7#11"
	// Semitone offsets from the chord root for each chord tone, -1 = unused.
	// A tone can select two notes (diminished and half-whole scales).
	int8_t tone[NUM_TONES][2];
	// Slash chord: offset of the triad root and quality ("", "m"), -1 if not a slash chord
	int8_t slashRoot;
	const char* slashQuality;
};

struct Degree {
	int8_t rootOffset;  // Offset of the chord root from the key root
	int16_t scale;      // Index into scales()
};

struct Mode {
	const char* name;
	std::vector<Degree> degrees;
};

const std::vector<Scale>& scales();
const std::vector<Mode>& modes();

/** The fully resolved harmony at a point in time */
struct Harmony {
	int keyRoot = 0;     // ROOT knob (0..11)
	int mode = 0;
	int degree = 0;
	int chordRoot = 0;   // Absolute pitch class of the chord root
	const Scale* scale = nullptr;
	// Absolute pitch classes for each function (up to two notes), -1 = none
	int8_t notes[NUM_FUNCS][2];
	int numScaleNotes = 0;

	void set(int keyRoot, int mode, int degree);
	/** Bitmask of the 12 pitch classes selected by a function mask */
	uint16_t pitchMask(uint16_t funcMask) const;
	/** Function that a pitch class belongs to, -1 if none */
	int funcOfPitch(int pc) const;
	std::string chordName() const;
	std::string modeName() const;
	std::string scaleTones() const;
	std::string scaleStructure() const;
};

int wrap(int x, int n);
std::string noteName(int pc, bool flat);
std::string pitchName(float voltage);
/** True if the scale on this root should be spelled with flats */
bool preferFlats(int chordRoot, const Scale& scale);

/** Chord voicings addressable by CV: 1..98, each a mask of chord tones T1..T13 */
const std::vector<uint8_t>& voicings();

/** Intonation offsets in cents for semitones relative to the root */
enum Intonation { EQUAL, JUST, MEANTONE, NUM_INTONATIONS };
float intonationCents(int intonation, int semitonesFromRoot);

}  // namespace harmony
