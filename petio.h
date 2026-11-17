#pragma once

class flash_file;
class sound_sink;

class petio: public Memory::Devices {
public:
	petio(flash_file &tape, sound_sink &s);

	PIA pia1;
	PIA pia2;
	VIA via;

	void reset();

	void operator= (uint8_t);

	// loads the currently-selected file
	bool load_prg();

private:
	uint8_t _ticks = 0;
	int8_t _timer = -1;
	void tick();

	void sound_on_off(bool o) { if (o) sound_on(); else sound_off(); }
	void sound_frequency(uint8_t);
	void sound_octave(uint8_t);
	void sound_reset() { sound_off(); _freq = 0; _octave = 0; }
	void sound_on();
	void sound_off();

	uint8_t _octave = 0;
	uint32_t _freq = 0;

	flash_file &_tape;
	sound_sink &_sound;
};
