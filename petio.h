#ifndef __PETIO_H
#define __PETIO_H

class Line;

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device, public PIA, public VIA {
public:
	petio(filer &files, Line &irq): Memory::Device(256), VIA(irq), _irq(irq), _ticks(0), _octave(0), _freq(0), files(files) {}
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

	// VIA
	virtual void write_sr(uint8_t);
	virtual void write_acr(uint8_t);
	virtual void write_t2lo(uint8_t);

private:
	void tick();

	Line &_irq;
	uint8_t _ticks;

	void sound_on();
	void sound_off();
	void sound_freq(uint8_t);
	void sound_octave(uint8_t);

	uint8_t _octave;
	uint32_t _freq;
};

#endif
