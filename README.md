PET-Energia
===========

An implementation of the Commodore PET on the same hardware configuration
as the UK101-Energia.

This PET emulates the graphics keyboard:
- graphics characters are produced with L-GUI and R-GUI (instead of L-SHIFT and 
R-SHIFT)
- the usual control keys work with additional mappings:
  - ^T = Delete = Backspace
  - ^Q = Down
  - ^] = Right
  - ^S = Home

All of the firmware is from [zimmers.net](http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/index.html).

Currently this is pretty basic, there is no cassette or disk I/O to load
programs. This is next on the TODO list.

Function Keys:
- F1 = reset
