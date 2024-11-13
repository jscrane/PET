#pragma once

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device, public PIA {
public:
	petio(filer &files): Memory::Device(256), _ticks(0),  files(files) {}

	VIA via;

	void reset();
	bool start();

	void operator= (uint8_t b);
	operator uint8_t();

	static void on_tick();

	filer &files;
	kbd keyboard;

	// loads the file currently selected by tape
	bool load_prg();

protected:
	// PIA
	virtual uint8_t read_portb();
	virtual uint8_t read_porta();
	virtual void write_porta(uint8_t);

private:
	void tick();

	uint8_t _ticks;
};
