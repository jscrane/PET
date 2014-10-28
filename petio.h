#ifndef __PETIO_H
#define __PETIO_H

class port;

// this occupies all of the addresses (and more) dedicated to the 
// two PIAs and the VIA
class petio: public Memory::Device {
public:
	void operator= (byte b) { write(b); }
	operator byte() { return read(); }

	petio(port &irq): Memory::Device(256), _irq(irq) {}
	void reset();
	void tick();

	port CA2;

	sdtape tape;
	kbd keyboard;

private:
	void write(byte b);
	byte read();

	port &_irq;
	volatile bool _timer1, _timer2;
	volatile unsigned short _t1, _t2;
};

#endif
