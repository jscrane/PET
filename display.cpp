#include <UTFT.h>
#include <memory.h>
#include <utftdisplay.h>
#include <hardware.h>

#include "config.h"
#include "port.h"
#include "display.h"
#include "roms/characters_vic20.h"

void display::begin()
{
	UTFTDisplay::begin(TFT_BG, TFT_FG);
	clear();
}

static struct resolution {
	const char *name;
	unsigned cw, ch;
	boolean double_size;
} resolutions[] = {
	{"40x25", 8, 8, false},
};

void display::_draw(Memory::address a, byte c)
{
	struct resolution &r = resolutions[_resolution];
	unsigned x = r.cw * (a % CHARS_PER_LINE);
	if (x >= _dx)
		return;

	unsigned y = (r.double_size? 2*r.ch: r.ch) * (a / CHARS_PER_LINE);
	if (y >= _dy)
		return;

	unsigned short ch = c;
	if (_upr.read())
		ch += 256;
	for (unsigned i = 0; i < r.ch; i++) {
		byte b = charset[ch][i];
		for (unsigned j = 0; j < r.cw; j++) {
			int _cx = x + r.cw - j;
			utft.setColor((b & (1 << j))? TFT_FG: TFT_BG);
			if (r.double_size) {
				utft.drawPixel(_cx, y + 2*i);
				utft.drawPixel(_cx, y + 2*i + 1);
			} else
				utft.drawPixel(_cx, y + i);
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
