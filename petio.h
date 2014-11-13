#ifndef __PETIO_H
#define __PETIO_H

class port;

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device, Timed {
public:
	void operator= (byte b) { write(b); }
	operator byte() { return read(); }

	petio(port &irq): Memory::Device(256), _irq(irq), _ticks(0) {}
	void reset();
	virtual bool tick();

	port CA2;

	sdtape tape;
	kbd keyboard;

	// loads the file currently selected by tape
	bool load_prg();
private:
	void write(byte b);
	byte read();

	port &_irq;
	byte _ticks;

	// via
	volatile bool _timer1, _timer2;
	volatile unsigned short _t1, _t2;
	unsigned short _t1_latch;
	byte _ier, _ifr, _ddra, _ddrb;
	volatile byte _porta, _portb;
};

#endif
