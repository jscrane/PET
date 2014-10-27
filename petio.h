#ifndef __PETIO_H
#define __PETIO_H

// this occupies all of the addresses (and more) dedicated to the two PIAs and the VIA
class petio: public Memory::Device {
public:
	void operator= (byte b) { write(b); }
	operator byte() { return read(); }

	petio(): Memory::Device(256) {}

	port CA2;

	sdtape tape;
	kbd keyboard;

private:
	void write(byte b);
	byte read();
};

#endif
