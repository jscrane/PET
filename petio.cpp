#include <Arduino.h>
#include <memory.h>
#include <ps2_raw_kbd.h>
#include <serialio.h>
#include <filer.h>
#include <timed.h>
#include <hardware.h>
#include <pia.h>
#include <via.h>

#include "kbd.h"
#include "petio.h"

// see http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/PET-Interfaces.txt
// and http://www.6502.org/users/andre/petindex/progmod.html

// device offsets (base is 0xe800)
#define PIA1_OFFSET	0x0010
#define PIA2_OFFSET	0x0020
#define VIA_OFFSET		0x0040

// via port-b
#define VIDEO_RETRACE	0x20

static petio *io;

// 1ms internal clock tick
#define TICK_FREQ	1000

// 50Hz system interrupt frequency
#define SYS_TICKS	20

void petio::reset() {
	keyboard.reset();
	PIA::reset();
	via.reset();
}

bool petio::start() {
	io = this;

	timer_create(TICK_FREQ, petio::on_tick);

	return files.start();
}

void IRAM_ATTR petio::on_tick() {
	io->tick();
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

uint8_t petio::read_portb() {
	return keyboard.read();
}

uint8_t petio::read_porta() {
	// diagnostic sense high (otherwise get monitor)
	// with cassette sense get "press play on tape #n"
	//return 0x80 + keyboard.row();
	return PIA::read_porta() | 0x80;
}

petio::operator uint8_t() {

	if (PIA1_OFFSET <= _acc && _acc < PIA2_OFFSET)
		return PIA::read(_acc & 0x0f);

	if (PIA2_OFFSET <= _acc && _acc < VIA_OFFSET)
		return 0x00;

	return via.read(_acc - VIA_OFFSET);
}

void petio::write_porta(uint8_t r) {
	keyboard.write(r & 0x0f);
	PIA::write_porta(r);
}

void petio::operator=(uint8_t r) {

	if (PIA1_OFFSET <= _acc && _acc < PIA2_OFFSET) {
		PIA::write(_acc & 0x0f, r);
		return;
	}

	if (PIA2_OFFSET <= _acc && _acc < VIA_OFFSET) {
		// PIA2 NYI
		return;
	}

	via.write(_acc - VIA_OFFSET, r);
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
