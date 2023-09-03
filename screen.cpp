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

uint8_t screen::_get(Memory::address a)
{
	uint8_t c = _mem[a];
	uint8_t bit = (1 << (a % 8)), b = a / 8;
	if (_inv[b] & bit)
		return c | 0x80;
	return c;
}

void screen::_set(Memory::address a, uint8_t c)
{
	if (a >= CHARS_PER_LINE * SCREEN_LINES)
		return;

	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	unsigned y = r.ch * (a / CHARS_PER_LINE);

	uint8_t ch = (c & 0x7f), cm = _mem[a];
	if (_upr.read())
		ch |= 0x80;

	uint8_t bit = (1 << (a % 8)), b = a / 8;
	bool invert = (c & 0x80), inverted = (_inv[b] & bit);

	if (invert == inverted && ch == cm)
		return;

	for (unsigned j = 0; j < r.ch; j++) {
		uint8_t b = pgm_read_byte(&charset[ch][j]);
		if (invert)
			b = ~b;

		uint8_t m = pgm_read_byte(&charset[cm][j]);
		if (inverted)
			m = ~m;

		if (b != m) {
			uint8_t d = (b ^ m);
			if (d & 1)
				drawPixel(x + 7, y + j, (b & 1)? FG_COLOUR: BG_COLOUR);

			if (d & 2)
				drawPixel(x + 6, y + j, (b & 2)? FG_COLOUR: BG_COLOUR);

			if (d & 4)
				drawPixel(x + 5, y + j, (b & 4)? FG_COLOUR: BG_COLOUR);

			if (d & 8)
				drawPixel(x + 4, y + j, (b & 8)? FG_COLOUR: BG_COLOUR);

			if (d & 16)
				drawPixel(x + 3, y + j, (b & 16)? FG_COLOUR: BG_COLOUR);

			if (d & 32)
				drawPixel(x + 2, y + j, (b & 32)? FG_COLOUR: BG_COLOUR);

			if (d & 64)
				drawPixel(x + 1, y + j, (b & 64)? FG_COLOUR: BG_COLOUR);

			if (d & 128)
				drawPixel(x + 0, y + j, (b & 128)? FG_COLOUR: BG_COLOUR);
		}
	}
	_mem[a] = ch;
	if (invert)
		_inv[b] |= bit;
	else
		_inv[b] &= ~bit;
}

void screen::checkpoint(Stream &s)
{
	s.write(_resolution); 
	s.write(_inv, sizeof(_inv));
	s.write(_mem, sizeof(_mem));
}

void screen::restore(Stream &s)
{
	_resolution = s.read();

	s.readBytes(_inv, sizeof(_inv));

	for (unsigned p = 0; p < sizeof(_mem); p += Memory::page_size) {
		char buf[Memory::page_size];
		s.readBytes(buf, sizeof(buf));
		for (unsigned i = 0; i < Memory::page_size; i++)
			_set(p + i, buf[i]);
	}
}
