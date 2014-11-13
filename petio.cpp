#include <Energia.h>
#include <memory.h>
#include <keyboard.h>
#include <sdtape.h>
#include <timed.h>

#include "port.h"
#include "kbd.h"
#include "petio.h"

// see http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/PET-Interfaces.txt

// base is 0xe800
#define PIA1	0x0010
#define PIA2	0x0020
#define PORTA	0x00
#define CRA	0x01
#define PORTB	0x02
#define CRB	0x03

#define VIA	0x0040
#define VPORTB	0x00
#define VPORTA	0x01
#define DDRB	0x02
#define DDRA	0x03
#define T1LO	0x04
#define T1HI	0x05
#define T1LLO	0x06
#define T1LHI	0x07
#define T2LO	0x08
#define T2HI	0x09
#define SHIFT	0x0a
#define ACR	0x0b
#define PCR	0x0c
#define IFR	0x0d
#define IER	0x0e

static petio *io;

// 1ms internal clock tick
#define TICK_FREQ	1000

// 50Hz system interrupt frequency
#define SYS_TICKS	20

void petio::reset() {
	keyboard.reset();

	timer_create(TICK_FREQ, this);
}

#define IER_MASTER	0x80
#define IER_TIMER1	0x40
#define IER_TIMER2	0x20
#define IER_CB1_ACTIVE	0x10
#define IER_CB2_ACTIVE	0x08
#define IER_SHIFT_REG	0x04
#define IER_CA1_ACTIVE	0x02
#define IER_CA2_ACTIVE	0x01

bool petio::tick() {
	if (_ticks++ == SYS_TICKS) {
		_ticks = 0;
		_portb |= 0x20;
		_irq.write(true);
	}

	if (_timer2) {
		if (_t2 < 1000) {
			_t2 = 0;
			_timer2 = false;
			_ifr |= IER_TIMER2;
			if ((_ier & IER_MASTER) && (_ier & IER_TIMER2))
				_irq.write(true);
		} else
			_t2 -= 1000;
	}
	// FIXME: timer1
	return true;
}

static void print(const char *msg, Memory::address a)
{
	Serial.print(millis());
	Serial.print(msg);
	Serial.println(a, 16);
}

static void print(const char *msg, Memory::address a, byte r) {
	Serial.print(millis());
	Serial.print(msg);
	Serial.print(a, 16);
	Serial.print(' ');
	Serial.println(r, 16);
}

byte petio::read() {
	byte r = 0x00;

	switch (_acc) {
	// keyboard in
	case PIA1 + PORTB:
		r = keyboard.read();
		break;

	case PIA1 + PORTA:
		// diagnostic sense high (otherwise get monitor)
		// with cassette sense get "press play on tape #n"
		r = 0x80 + keyboard.row();
		break;

	case PIA1 + CRA:
		// video sync in???
//		r = 0x87;
		break;

	case PIA1 + CRB:
		// FIXME: this is required for tape
		break;

	case VIA + VPORTA:
		// ~DAV + ~NRFD + ~NDAC
		r = _porta;
		break;

	case VIA + VPORTB:
		// screen retrace in
		//r = ((0xff - 0x20) & ~_ddrb);
		break;

	case VIA + ACR:
		print(" acr ", _acc);
		// acr bits 5-7 relate to timers
		break;

	case VIA + IFR:
		print(" ifr ", _acc);
		// timer #1
//		r = 0x40;
		r = _ifr;
		_ifr = 0;
		break;

	case VIA + T1LO:
		r = (_t1 & 0xff);
		break;

	case VIA + T1HI:
		r = (_t1 / 0xff);
		break;

	case VIA + T1LLO:
		r = (_t1_latch & 0xff);
		break;

	case VIA + T1LHI:
		r = (_t1_latch / 0xff);
		break;

	case VIA + T2LO:
		r = (_t2 & 0xff);
		break;

	case VIA + T2HI:
		r = (_t2 / 0xff);
		break;

	default:
		// some other device...
		print(" <??? ", _acc);
		break;
	}
if (VIA < _acc && _acc <= VIA + 0x0f)
	print(" <via ", _acc, r);

	return r;
}

void petio::write(byte r) {
if (VIA < _acc && _acc <= VIA + 0x0f)
	print(" >via ", _acc, r);

	switch (_acc) {
	case PIA1 + PORTA:
		keyboard.write(r & 0x0f);
		break;

	case PIA1 + CRB:
		// for now
		break;

	case VIA + PCR:
		CA2.write(r & 0x02);	
		break;

	case VIA + VPORTB:
		_portb = (r & _ddrb);
		break;

	case VIA + VPORTA:
		_porta = (r & _ddra);
		break;

	case VIA + DDRB:
		_ddrb = r;
		break;

	case VIA + DDRA:
		_ddra = r;
		break;

	case VIA + ACR:
		print(" acr ", _acc, r);
		break;

	case VIA + IER:
		_ier = r;
		break;

	case VIA + T1LO:
		_t1 = r;
		_timer1 = false;
		break;

	case VIA + T1HI:
		_t1 = (r * 256) + _t1;
		_timer1 = true;
		break;

	case VIA + T1LLO:
		_t1_latch = r;
		break;

	case VIA + T1LHI:
		_t1_latch = (r * 256) + _t1_latch;
		break;

	case VIA + T2LO:
		_t2 = r;
		_timer2 = false;
		break;

	case VIA + T2HI:
		_t2 = (r * 256) + _t2;
		_timer2 = true;
		break;

	default:
		print(" >??? ", _acc, r);
		break;
	}
}
