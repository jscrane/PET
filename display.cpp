#include <UTFT.h>
#include <memory.h>
#include <utftdisplay.h>
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
	UTFTDisplay::begin(TFT_BG, TFT_FG);
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
		uint8_t b = charset[ch][j], m = charset[cm][j];
		if (b != m) {
			uint8_t d = (b ^ m);
			if (d & 1) {
				utft.setColor((b & 1)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 7, y + j);
			}
			if (d & 2) {
				utft.setColor((b & 2)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 6, y + j);
			}
			if (d & 4) {
				utft.setColor((b & 4)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 5, y + j);
			}
			if (d & 8) {
				utft.setColor((b & 8)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 4, y + j);
			}
			if (d & 16) {
				utft.setColor((b & 16)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 3, y + j);
			}
			if (d & 32) {
				utft.setColor((b & 32)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 2, y + j);
			}
			if (d & 64) {
				utft.setColor((b & 64)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 1, y + j);
			}
			if (d & 128) {
				utft.setColor((b & 128)? TFT_FG: TFT_BG);
				utft.drawPixel(x + 0, y + j);
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
