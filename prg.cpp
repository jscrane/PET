#include <Energia.h>
#include <hardware.h>
#include <memory.h>
#include <sdtape.h>
#include <keyboard.h>

#include "kbd.h"
#include "port.h"
#include "petio.h"

bool petio::load_prg()
{
	if (!tape.more())
		return false;

	byte lo = tape.read();
	byte hi = tape.read();

	Memory::address a = lo + (hi << 8);
	while (tape.more())
		memory[a++] = tape.read();
	
	// based on mem_get/set_basic_text() from vice/petmem.c
	memory[0xc7] = memory[0x28];
	memory[0xc8] = memory[0x29];

	lo = (byte)(a & 0xff);
	memory[0x2a] = lo;
	memory[0x2c] = lo;
	memory[0x2e] = lo;
	memory[0xc9] = lo;

	hi = (byte)(a >> 8);
	memory[0x2b] = hi;
	memory[0x2d] = hi;
	memory[0x2f] = hi;
	memory[0xca] = hi;
	
	return true;
}
