#pragma once

// https://www.zimmers.net/cbmpics/cbm/PETx/petfaq.html
// see: HOW DO I MAKE SOUND ON MY PET?
class sound {
public:
	sound(): _octave(0), _freq(0) {}

	void on_off(bool o) { if (o) on(); else off(); }
	void frequency(uint8_t);
	void octave(uint8_t);
	void reset() { off(); _freq = 0; _octave = 0; }

private:
	void on();
	void off();

	uint8_t _octave;
	uint32_t _freq;
};
