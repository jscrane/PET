#ifndef _PORT_H
#define _PORT_H

class port {
public:
	port(): _state(false) {}
	bool read() { return _state; }
	void write(bool state) { _state = state; }

private:
	bool _state;
};

#endif
