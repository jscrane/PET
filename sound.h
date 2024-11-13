#pragma once

// https://www.zimmers.net/cbmpics/cbm/PETx/petfaq.html
// see: HOW DO I MAKE SOUND ON MY PET?
class sound {
public:
	sound(): _octave(0), _freq(0) {}

	void on();
	void off();
	void frequency(uint8_t);
	void octave(uint8_t);

private:
	uint8_t _octave;
	uint32_t _freq;
};
