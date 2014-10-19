#include <Energia.h>
#include <UTFT.h>
#include <memory.h>
#include <hardware.h>

#include "config.h"
#include "display.h"
#include "roms/characters_vic20.h"

static unsigned cx, cy, dx, dy, oxs;

void display::begin()
{
  utft.fillScr(TFT_BG);
  dx = utft.getDisplayXSize();
  dy = utft.getDisplayYSize();
  cx = utft.getFontXsize();  
  cy = utft.getFontYsize();
  oxs = dx;
}

static struct resolution {
  const char *name;
  unsigned cw, ch;
  boolean double_size;
} resolutions[] = {
  {"40x25", 8, 8, false},
};

void display::clear()
{
  utft.fillScr(TFT_BG);
}

void display::error(char *s)
{
  utft.setColor(TFT_FG);
  char *lines[5];
  int l = 0;
  for (char *p = s, *q = s; *p; p++)
    if (*p == '\n') {
      *p++ = 0;
      lines[l++] = q;
      q = p;
    }
  unsigned y = (dy - l*cy)/2;
  for (int i = 0; i < l; i++) {
    char *p = lines[i];
    unsigned x = (dx - strlen(p)*cx)/2;
    utft.print(p, x, y);
    y += cy;
  }
}

void display::status(const char *s)
{
  utft.setColor(TFT_FG);
  unsigned y = dy - cy, n = strlen(s), xs = dx - n*cx;
  for (unsigned x = oxs; x < xs; x += cx)
    utft.print(" ", x, y);
  utft.print(s, xs, y);
  oxs = xs;
}

void display::_set(Memory::address a, byte c)
{
  if (c != _mem[a]) {
    _mem[a] = c;  
    struct resolution &r = resolutions[_resolution];
    int x = r.cw * (a % CHARS_PER_LINE - X_OFF);  // hack to view left edge of screen
    if (x < 0 || x >= dx)
      return;
     
    unsigned y = (r.double_size? 2*r.ch: r.ch) * (a / CHARS_PER_LINE);    
    if (y >= dy)
      return;

    for (unsigned i = 0; i < r.ch; i++) {
      byte b = charset[c][i];
      for (unsigned j = 0; j < r.cw; j++) {
        int cx = x + r.cw - j;
        utft.setColor((b & (1 << j))? TFT_FG: TFT_BG);
        if (r.double_size) {
          utft.drawPixel(cx, y + 2*i);
          utft.drawPixel(cx, y + 2*i + 1);
        } else
          utft.drawPixel(cx, y + i);
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
  for (int i = 0; i < sizeof(_mem); i++)
    _set(i, s.read());
}

