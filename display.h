#ifndef _DISPLAY_H
#define _DISPLAY_H

class display: public UTFTDisplay, public Memory::Device {
public:
	virtual void operator= (byte c) { _set(_acc, c); }
	virtual operator byte () { return _mem[_acc]; }

	virtual void checkpoint(Stream &s);
	virtual void restore(Stream &s);

	display(port &upr): Memory::Device(sizeof(_mem)), _resolution(0), _upr(upr) {}
	void begin();

private:
	inline void _set(Memory::address a, byte c) {
		if (c != _mem[a]) { _draw(a, c); _mem[a] = c; }
	}
	void _draw(Memory::address a, byte c);

	byte _mem[DISPLAY_RAM_SIZE];
	int _resolution, _yoff;
	port &_upr;
};
#endif
