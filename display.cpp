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
	unsigned cw, ch;
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

void display::_draw(Memory::address a, byte c)
{
	if (a >= CHARS_PER_LINE * DISPLAY_LINES)
		return;

	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	if (x >= _dx)
		return;

	unsigned y = r.ch * (a / CHARS_PER_LINE) + _yoff;
	if (y >= _dy)
		return;

	unsigned short ch = c;
	if (_upr.read())
		ch += 256;
	for (unsigned j = 0; j < r.ch; j++) {
		byte b = charset[ch][j];
		for (unsigned i = 0; i < r.cw; i++) {
			int _cx = x + r.cw - i;
			utft.setColor((b & (1 << i))? TFT_FG: TFT_BG);
			utft.drawPixel(_cx, y + j);
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
