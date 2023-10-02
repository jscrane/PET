#ifndef _SCREEN_H
#define _SCREEN_H

class screen: public Display, public Memory::Device {
public:
	virtual void operator= (uint8_t c) { _set(_acc, c); }
	virtual operator uint8_t () { return _mem[_acc]; }

	virtual void checkpoint(Stream &s);
	virtual void restore(Stream &s);

	screen(Line &upr): Memory::Device(sizeof(_mem)), _resolution(0), _upr(upr) {}
	void begin();

private:
	void _set(Memory::address a, uint8_t c);

	uint8_t _mem[SCREEN_RAM_SIZE];
	int _resolution;
	Line &_upr;
};
#endif
