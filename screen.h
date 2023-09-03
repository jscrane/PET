#ifndef _SCREEN_H
#define _SCREEN_H

class screen: public Display, public Memory::Device {
public:
	virtual void operator= (uint8_t c) { _set(_acc, c); }
	virtual operator uint8_t () { return _get(_acc); }

	virtual void checkpoint(Stream &s);
	virtual void restore(Stream &s);

	screen(port &upr): Memory::Device(sizeof(_mem)), _resolution(0), _upr(upr) {}
	void begin();

private:
	void _set(Memory::address a, uint8_t c);
	uint8_t _get(Memory::address a);

	uint8_t _mem[SCREEN_RAM_SIZE], _inv[SCREEN_RAM_SIZE / 8];
	int _resolution;
	port &_upr;
};
#endif
