#pragma once

// https://www.zimmers.net/cbmpics/cbm/PETx/petfaq.html
// see: HOW DO I MAKE SOUND ON MY PET?
// poke 59464, freq (or 0 for no sound): 0xe848 = VIA T2LO
// poke 59466, octave: 0xe84a = VIA SR
// poke 59467, 16=on 0=off: 0xe84b = VIA ACR
// also: http://blog.tynemouthsoftware.co.uk/2022/05/pet-sounds.html
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
