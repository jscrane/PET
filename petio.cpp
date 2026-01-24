#include <Arduino.h>
#include <memory.h>
#include <serialio.h>
#include <filer.h>
#include <hardware.h>
#include <pia.h>
#include <via.h>

#include "petio.h"
#include "sound.h"

// see http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/PET-Interfaces.txt
// and http://www.6502.org/users/andre/petindex/progmod.html

// device offsets (base is 0xe800)
#define PIA1_OFFSET	0x0010
#define PIA2_OFFSET	0x0020
#define VIA_OFFSET	0x0040

// via port-b
#define VIDEO_RETRACE	0x20

// 50Hz system interrupt frequency
#define SYS_TICKS	20

sound sound;

void petio::reset() {

	sound.reset();
	pia1.reset();
	pia2.reset();
	via.reset();
}

void petio::tick() {

	if (_ticks++ == SYS_TICKS) {
		_ticks = 0;
		via.write_portb_in_bit(VIDEO_RETRACE, true);
		via.set_interrupt();
	} else
		via.write_portb_in_bit(VIDEO_RETRACE, false);

	via.tick();
}

petio::operator uint8_t() {

	if (PIA1_OFFSET <= _acc && _acc < PIA2_OFFSET)
		return pia1.read((_acc - PIA1_OFFSET) & 0x0f);

	if (PIA2_OFFSET <= _acc && _acc < VIA_OFFSET)
		return pia2.read((_acc - PIA2_OFFSET) & 0x0f);

	return via.read(_acc - VIA_OFFSET);
}

void petio::operator=(uint8_t r) {

	if (PIA1_OFFSET <= _acc && _acc < PIA2_OFFSET) {
		pia1.write((_acc - PIA1_OFFSET) & 0x0f, r);
		return;
	}

	if (PIA2_OFFSET <= _acc && _acc < VIA_OFFSET) {
		pia2.write((_acc - PIA2_OFFSET) & 0x0f, r);
		return;
	}

	via.write(_acc - VIA_OFFSET, r);

	switch (_acc - VIA_OFFSET) {
	case 0x0a:
		sound.octave(r);
		break;
	case 0x08:
		sound.frequency(r);
		break;
	case 0x0b:
		sound.on_off((r & VIA::ACR_SHIFT_MASK) == VIA::ACR_SO_T2_RATE);
		break;
	}
}

bool petio::load_prg()
{
	extern Memory memory;

	if (!files.more())
		return false;

	uint8_t lo = files.read();
	uint8_t hi = files.read();
	Memory::address a = lo + (hi << 8);
	while (files.more())
		memory[a++] = files.read();
	
	// based on mem_get/set_basic_text() from vice/petmem.c
	memory[0xc7] = memory[0x28];
	memory[0xc8] = memory[0x29];

	lo = (uint8_t)(a & 0xff);
	memory[0x2a] = lo;
	memory[0x2c] = lo;
	memory[0x2e] = lo;
	memory[0xc9] = lo;

	hi = (uint8_t)(a >> 8);
	memory[0x2b] = hi;
	memory[0x2d] = hi;
	memory[0x2f] = hi;
	memory[0xca] = hi;
	
	return true;
}
