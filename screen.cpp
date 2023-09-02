#include <Arduino.h>
#include <Stream.h>
#include <stdint.h>
#include <memory.h>
#include <display.h>
#include <hardware.h>

#include "config.h"
#include "port.h"
#include "screen.h"
#include CHARSET_ROM

static struct resolution {
	const char *name;
	const unsigned cw, ch;
} resolutions[] = {
	{"40x25", 8, 8},
};

void screen::begin()
{
	struct resolution &r = resolutions[_resolution];

	Display::begin(BG_COLOUR, FG_COLOUR, ORIENT, CHARS_PER_LINE*r.cw, SCREEN_LINES*r.ch);
	clear();
}

void screen::_draw(Memory::address a, uint8_t c)
{
	if (a >= CHARS_PER_LINE * SCREEN_LINES)
		return;

	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	unsigned y = r.ch * (a / CHARS_PER_LINE);

	uint8_t ch = (c & 0x7f);
	if (_upr.read())
		ch |= 0x80;

	for (unsigned j = 0; j < r.ch; j++) {
		uint8_t b = pgm_read_byte(&charset[ch][j]);
		if (c & 0x80)	// invert?
			b = ~b;

		drawPixel(x + 7, y + j, (b & 1)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 6, y + j, (b & 2)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 5, y + j, (b & 4)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 4, y + j, (b & 8)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 3, y + j, (b & 16)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 2, y + j, (b & 32)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 1, y + j, (b & 64)? FG_COLOUR: BG_COLOUR);
		drawPixel(x + 0, y + j, (b & 128)? FG_COLOUR: BG_COLOUR);
	}
}

void screen::checkpoint(Stream &s)
{
	s.write(_resolution); 
	s.write(_mem, sizeof(_mem));
}

void screen::restore(Stream &s)
{
	_resolution = s.read();
	for (unsigned p = 0; p < sizeof(_mem); p += Memory::page_size) {
		char buf[Memory::page_size];
		s.readBytes(buf, sizeof(buf));
		for (unsigned i = 0; i < Memory::page_size; i++)
			_set(p + i, buf[i]);
	}
}
