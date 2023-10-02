#ifndef _PORT_H
#define _PORT_H

// FIXME: rename this to Line
class port {
public:
	port(): _state(false) {}
	bool read() { return _state; }
	void write(bool state) { _state = state; }

	// Line interface
	operator bool() { return _state; }
	void set(bool state) { _state = state; }
	void clear() { set(false); }
	void set() { set(true); }

private:
	volatile bool _state;
};

#endif
