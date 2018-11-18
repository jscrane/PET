#include <Stream.h>
#include <stdint.h>
#include <memory.h>
#include <tftdisplay.h>
#include <hardware.h>

#include "config.h"
#include "port.h"
#include "display.h"
#include "roms/characters_vic20.h"

static struct resolution {
	const char *name;
	const unsigned cw, ch;
} resolutions[] = {
	{"40x25", 8, 8},
};

void display::begin()
{
	TFTDisplay::begin(TFT_BG, TFT_FG, TFT_ORIENT);
	clear();

	struct resolution &r = resolutions[_resolution];
	unsigned dh = DISPLAY_LINES * r.ch;
	_yoff = _dy < dh? 0: (_dy - dh) / 2;
}

void display::_draw(Memory::address a, uint8_t c)
{
	if (a >= CHARS_PER_LINE * DISPLAY_LINES)
		return;

	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	unsigned y = r.ch * (a / CHARS_PER_LINE) + _yoff;

	unsigned short ch = c, cm = _mem[a];
	if (!_upr.read()) {
		ch += 256;
		cm += 256;
	}
	for (unsigned j = 0; j < r.ch; j++) {
		uint8_t b = pgm_read_byte(&charset[ch][j]);
		uint8_t m = pgm_read_byte(&charset[cm][j]);
		if (b != m) {
			uint8_t d = (b ^ m);
			if (d & 1) {
				unsigned c = (b & 1)? TFT_FG: TFT_BG;
				drawPixel(x + 7, y + j, c);
			}
			if (d & 2) {
				unsigned c = (b & 2)? TFT_FG: TFT_BG;
				drawPixel(x + 6, y + j, c);
			}
			if (d & 4) {
				unsigned c = (b & 4)? TFT_FG: TFT_BG;
				drawPixel(x + 5, y + j, c);
			}
			if (d & 8) {
				unsigned c = (b & 8)? TFT_FG: TFT_BG;
				drawPixel(x + 4, y + j, c);
			}
			if (d & 16) {
				unsigned c = (b & 16)? TFT_FG: TFT_BG;
				drawPixel(x + 3, y + j, c);
			}
			if (d & 32) {
				unsigned c = (b & 32)? TFT_FG: TFT_BG;
				drawPixel(x + 2, y + j, c);
			}
			if (d & 64) {
				unsigned c = (b & 64)? TFT_FG: TFT_BG;
				drawPixel(x + 1, y + j, c);
			}
			if (d & 128) {
				unsigned c = (b & 128)? TFT_FG: TFT_BG;
				drawPixel(x + 0, y + j, c);
			}
		}
	}
}

void display::checkpoint(Stream &s)
{
	s.write(_resolution); 
	s.write(_mem, sizeof(_mem));
}

void display::restore(Stream &s)
{
	_resolution = s.read();
	for (unsigned p = 0; p < sizeof(_mem); p += Memory::page_size) {
		char buf[Memory::page_size];
		s.readBytes(buf, sizeof(buf));
		for (unsigned i = 0; i < Memory::page_size; i++)
			_set(p + i, buf[i]);
	}
}
