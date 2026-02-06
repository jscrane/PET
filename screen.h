#pragma once

class screen: public Display, public Memory::Device {
public:
	virtual void operator= (uint8_t c) { _set(_acc, c); }
	virtual operator uint8_t () { return _mem[_acc]; }

	virtual void checkpoint(Checkpoint &s);
	virtual void restore(Checkpoint &s);

	screen(): Memory::Device(sizeof(_mem)), _resolution(0), _upr(false) {}
	void begin();

	void set_upper(bool u) { _upr = u; }

private:
	void _set(Memory::address a, uint8_t c);

	uint8_t _mem[SCREEN_RAM_SIZE];
	int _resolution;
	bool _upr;
};
