#pragma once

class petio: public Memory::Devices {
public:
	petio(filer &files);

	PIA pia1;
	PIA pia2;
	VIA via;

	void reset();
	bool start() { return files.start(); }

	void operator= (uint8_t);

	filer &files;

	// loads the currently-selected file
	bool load_prg();

private:
	uint8_t _ticks;
	int _timer;
	void tick();
};
