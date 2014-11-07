#ifndef __KBD_H
#define __KBD_H

class kbd: public Keyboard {
public:
	kbd(): _row(0), _ctrl(false), _shift(false) {}

	void up(byte);
	void down(byte);
	void reset();

	void write(byte r) { _row = r; }
	byte read() { return _rows[_row] ^ 0xff; }
	byte row() { return _row; }
private:
	byte _map(byte);
	void _set(byte);
	void _reset(byte);
	byte _rows[10];
	byte pattern();
	byte _row;
	bool _ctrl, _shift, _ext;
};

#endif
