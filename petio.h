#ifndef __PETIO_H
#define __PETIO_H

class port;

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device {
public:
	void operator= (uint8_t b) { write(b); }
	operator uint8_t() { return read(); }

	petio(port &irq): Memory::Device(256), _irq(irq), _ticks(0) {}
	void reset();

	static void on_tick();

	port CA2;

	sdtape tape;
	kbd keyboard;

	// loads the file currently selected by tape
	bool load_prg();

private:
	void tick();

	void write(uint8_t b);
	uint8_t read();

	port &_irq;
	uint8_t _ticks;

	// sound
	void sound_on();
	void sound_off();
	void sound_freq(uint8_t);
	void sound_octave(uint8_t);

	// via
	volatile bool _timer1, _timer2;
	volatile unsigned short _t1, _t2;
	unsigned short _t1_latch;
	uint8_t _ier, _ifr, _ddra, _ddrb;
	volatile uint8_t _porta, _portb;
};

#endif
