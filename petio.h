#ifndef __PETIO_H
#define __PETIO_H

// this occupies all of the addresses (and more) dedicated to the two PIAs and the VIA
class petio: public Memory::Device, public Keyboard {
public:
	void operator= (byte);
	operator byte() { return pattern(); }

	void up(byte);
	void down(byte);
	void reset();

	petio(): Memory::Device(256), _row(0) {}

private:
	byte _map(byte);
	void _set(byte);
	void _reset(byte);
	byte _rows[10];
	byte pattern();
	byte _row;
	bool _ctrl, _shift;
};

#endif
