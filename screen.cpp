#include <Arduino.h>
#include <Stream.h>
#include <stdint.h>
#include <memory.h>
#include <display.h>
#include <hardware.h>
#include <line.h>

#include "config.h"
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

/*
 * c is "nearly" the character to be written at address a:
 * bit 7 of c indicates whether it should be inverted (reverse video)
 * bit 1 of VIA register CA2 indicates whether the displayed character
 * should be taken from the lower- or upper-half of the character set;
 * this bit is set by POKE 59468, 14 and reset by POKE 59468, 12.
 */
void screen::_set(Memory::address a, uint8_t c)
{
	if (a >= CHARS_PER_LINE * SCREEN_LINES)
		return;

	uint8_t cm = _mem[a];
	if (c == cm)
		return;

	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	unsigned y = r.ch * (a / CHARS_PER_LINE);

	uint8_t ch = (c & 0x7f);
	bool invert = (c & 0x80), inverted = (cm & 0x80);
	cm &= 0x7f;
	if (_upr) {
		ch |= 0x80;
		cm |= 0x80;
	}

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
	_mem[a] = c;
}

void screen::checkpoint(Stream &s)
{
	s.write(_resolution); 
	s.write(_mem, sizeof(_mem));
}

void screen::restore(Stream &s)
{
	_resolution = s.read();

	for (Memory::address p = 0; p < sizeof(_mem); p += Memory::page_size) {
		uint8_t buf[Memory::page_size];
		s.readBytes(buf, sizeof(buf));
		for (unsigned i = 0; i < Memory::page_size; i++)
			_set(p + i, buf[i]);
	}
}
