#ifndef __PETIO_H
#define __PETIO_H

class Line;

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device, public PIA {
public:
	petio(filer &files, Line &irq): Memory::Device(256), _irq(irq), _ticks(0), _octave(0), _freq(0), files(files) {}
	void reset();
	bool start();

	void operator= (uint8_t b);
	operator uint8_t();

	static void on_tick();

	Line CA2;

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

	Line &_irq;
	uint8_t _ticks;

	// sound
	void sound_on();
	void sound_off();
	void sound_freq(uint8_t p);
	void sound_octave(uint8_t o);

	uint8_t _octave;
	uint32_t _freq;

	// via
	volatile bool _timer1, _timer2;
	volatile uint16_t _t1, _t2;
	uint16_t _t1_latch;
	uint8_t _acr, _ier, _ifr, _ddra, _ddrb;
	volatile uint8_t _porta, _portb;
};

#endif
