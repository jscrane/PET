#pragma once

#include <ps2_raw_kbd.h>

class kbd: public matrix_keyboard {
public:
	kbd() {}

	void up(uint16_t) override;
	void down(uint16_t) override;
	void reset() override;

	void write(uint8_t r) { _row = (r & 0x0f); }
	uint8_t read() const { return _row > 9? 0xff: _rows[_row] ^ 0xff; }
	uint8_t row() const { return _row; }

private:
	uint8_t _map(uint8_t) const;
	void _set(uint8_t);
	void _reset(uint8_t);

	uint8_t _rows[16];
	uint8_t _row = 0;
	bool _ctrl = false, _shift = false;
};
