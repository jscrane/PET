PET
===

An emulation of the Commodore PET on the same hardware configurations
as the [UK101](https://github.com/jscrane/UK101).

This PET emulates the graphics keyboard:
- graphics characters are produced with L-Shift
- R-Shift acts like normal shift
- the control keys work with additional mappings:
  - ^T = Delete = Backspace
  - ^Q = Down
  - ^] = Right
  - ^S = Home
  - NUM = ^
- the numeric keypad is mapped
- HOME/END/INS/DEL and the arrow keys are mapped (END == CLR)

All of the firmware is from [zimmers.net](http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/index.html).

[Zimmers.net](http://www.zimmers.net/anonftp/pub/cbm/pet/games/english/index.html) also has some nice games.

Function Keys:
- F1 = reset
- F2 = tape advance
- F3 = tape rewind
- F4 = load .prg file selected by tape
- F6 = checkpoint the machine state to disk
- F7 = restore from selected checkpoint file
- F10 = watch the CPU execute instructions on the serial port (if compiled with DEBUGGING)
