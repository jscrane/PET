#pragma once

#include <display.h>

class screen: public Display, public Memory::Device {
public:
	void operator= (uint8_t c) override { _set(_acc, c); }
	operator uint8_t () override { return _mem[_acc]; }

	void checkpoint(Checkpoint &s) override;
	void restore(Checkpoint &s) override;

	screen(): Memory::Device(sizeof(_mem)), _resolution(0), _upr(false) {}
	void begin();

	void set_upper(bool);

private:
	void _set(Memory::address a, uint8_t c);
	void _redraw(void);

	uint8_t _mem[SCREEN_RAM_SIZE];
	uint8_t _resolution;
	bool _upr;
};
