PROCESSOR_FAMILY := lm4f
BOARD := lplm4f120h5qr
SKETCH = pet.ino
SOURCES = display.cpp Memory.cpp petio.cpp ps2drv.cpp r6502.cpp spiram.cpp

include ~/src/uC-Makefile/energia.mk
