#pragma once

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device {
public:
	petio(filer &files): Memory::Device(256), _ticks(0),  _timer(-1), files(files) {}

	PIA pia1;
	PIA pia2;
	VIA via;

	void reset();
	bool start() { return files.start(); }

	void operator= (uint8_t b);
	operator uint8_t();

	filer &files;

	// loads the file currently selected by tape
	bool load_prg();

private:
	uint8_t _ticks;
	int _timer;
	void tick();
};
