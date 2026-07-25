#pragma once

class kbd: public matrix_keyboard {
public:
	kbd(): _row(0), _ctrl(false), _shift(false) {}

	void up(uint8_t);
	void down(uint8_t);
	void reset();

	void write(uint8_t r) { _row = (r & 0x0f); }
	uint8_t read() { return _row > 9? 0xff: _rows[_row] ^ 0xff; }
	uint8_t row() { return _row; }
private:
	uint8_t _map(uint8_t);
	void _set(uint8_t);
	void _reset(uint8_t);
	uint8_t _rows[16];
	uint8_t pattern();
	uint8_t _row;
	bool _ctrl, _shift, _ext;
};
