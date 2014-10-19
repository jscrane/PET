#ifndef _DISPLAY_H
#define _DISPLAY_H

class display: public UTFTDisplay {
public:
  virtual void operator= (byte c) { _set(_acc, c); }
  virtual operator byte () { return _mem[_acc]; }

  void checkpoint(Stream &s);
  void restore(Stream &s);

  display() : UTFTDisplay(sizeof(_mem)), _resolution(0) {}
  void begin();

private:
  void _set(Memory::address a, byte c);

  byte _mem[DISPLAY_RAM_SIZE];
  int _resolution;
};
#endif
