#ifndef _SCREEN_H
#define _SCREEN_H

class screen: public Display, public Memory::Device {
public:
	virtual void operator= (uint8_t c) { _set(_acc, c); }
	virtual operator uint8_t () { return _mem[_acc]; }

	virtual void checkpoint(Stream &s);
	virtual void restore(Stream &s);

	screen(port &upr): Memory::Device(sizeof(_mem)), _resolution(0), _upr(upr) {}
	void begin();

private:
	inline void _set(Memory::address a, uint8_t c) {
		if (c != _mem[a]) { _draw(a, c); _mem[a] = c; }
	}
	void _draw(Memory::address a, uint8_t c);

	uint8_t _mem[SCREEN_RAM_SIZE];
	int _resolution, _yoff;
	port &_upr;
};
#endif
