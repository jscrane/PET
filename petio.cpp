#include <Arduino.h>
#include <memory.h>
#include <keyboard.h>
#include <serialio.h>
#include <filer.h>
#include <timed.h>
#include <hardware.h>
#include <pia.h>

#include "line.h"
#include "via.h"
#include "kbd.h"
#include "petio.h"

// see http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/PET-Interfaces.txt
// and http://www.6502.org/users/andre/petindex/progmod.html

// device offsets (base is 0xe800)
#define PIA1_OFF	0x0010
#define PIA2_OFF	0x0020
#define VIA_OFF		0x0040

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

#define IER_MASTER	0x80
#define IER_TIMER1	0x40
#define IER_TIMER2	0x20
#define IER_CB1_ACTIVE	0x10
#define IER_CB2_ACTIVE	0x08
#define IER_SHIFT_REG	0x04
#define IER_CA1_ACTIVE	0x02
#define IER_CA2_ACTIVE	0x01

#define VIA_VIDEO_RETRACE	0x20
#define VIA_ACR_SHIFT_MASK	0x1c
#define VIA_ACR_T1_CONTINUOUS	0x40

static petio *io;

// 1ms internal clock tick
#define TICK_FREQ	1000

// 50Hz system interrupt frequency
#define SYS_TICKS	20

void petio::reset() {
	_acr = _ifr = _ier = 0x00;
	_t1 = _t2 = 0;
	_timer1 = _timer2 = false;

	sound_off();
	keyboard.reset();
	PIA::reset();
	VIA::reset();
}

bool petio::start() {
	io = this;

	timer_create(TICK_FREQ, petio::on_tick);

	return files.start();
}

void IRAM_ATTR petio::on_tick() {
	io->tick();
}

void IRAM_ATTR petio::tick() {
	if (_ticks++ == SYS_TICKS) {
		_ticks = 0;
		_portb |= VIA_VIDEO_RETRACE;
		_irq.set();
	} else
		_portb &= ~VIA_VIDEO_RETRACE;

	if (_timer1) {
		if (_t1 < TICK_FREQ) {
			_t1 = 0;
			_timer1 = false;
			_ifr |= IER_TIMER1;
			if ((_ier & IER_MASTER) && (_ier & IER_TIMER1))
				_irq.set();
		} else
			_t1 -= TICK_FREQ;
	}

	if (_timer2) {
		if (_t2 < TICK_FREQ) {
			_t2 = 0;
			_timer2 = false;
			_ifr |= IER_TIMER2;
			if ((_ier & IER_MASTER) && (_ier & IER_TIMER2))
				_irq.set();
		} else
			_t2 -= TICK_FREQ;
	}
}

static void print(const char *msg, Memory::address a)
{
#if defined(DEBUG_IO)
	Serial.print(millis());
	Serial.print(msg);
	Serial.println(a, 16);
#endif
}

static void print(const char *msg, Memory::address a, uint8_t r) {
#if defined(DEBUG_IO)
	Serial.print(millis());
	Serial.print(msg);
	Serial.print(a, 16);
	Serial.print(' ');
	Serial.println(r, 16);
#endif
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

	if (PIA1_OFF <= _acc && _acc < PIA2_OFF)
		return PIA::read(_acc & 0x0f);

	if (PIA2_OFF <= _acc && _acc < VIA_OFF)
		return 0x00;

	uint8_t r = 0x00;
	switch (_acc - VIA_OFF) {
	case VPORTA:
		// ~DAV + ~NRFD + ~NDAC
		r = _porta;
		_ifr &= ~(IER_CA1_ACTIVE | IER_CA2_ACTIVE);
		break;

	case VPORTB:
		// screen retrace in
		r = _portb;
		_ifr &= ~(IER_CB1_ACTIVE | IER_CB2_ACTIVE);
		break;

	case SHIFT:
		print(" shift ", _acc);
		_ifr &= ~IER_SHIFT_REG;
		break;

	case ACR:
		print(" acr ", _acc);
		// acr bits 5-7 relate to timers
		r = _acr;
		break;

	case IER:
		print(" ifr ", _acc);
		r = _ier | 0x80;
		break;

	case IFR:
		print(" ifr ", _acc);
		r = _ifr;
		break;

	case T1LO:
		r = (_t1 & 0xff);
		_ifr &= ~IER_TIMER1;
		break;

	case T1HI:
		r = (_t1 / 0xff);
		break;

	case T1LLO:
		r = (_t1_latch & 0xff);
		break;

	case T1LHI:
		r = (_t1_latch / 0xff);
		break;

	case T2LO:
		r = (_t2 & 0xff);
		_ifr &= ~IER_TIMER2;
		break;

	case T2HI:
		r = (_t2 / 0xff);
		break;

	default:
		// some other device...
		print(" <??? ", _acc);
		break;
	}
	if (VIA_OFF <= _acc && _acc < VIA_OFF+ 0x10)
		print(" <via ", _acc, r);

	return r;
}

void petio::write_porta(uint8_t r) {
	keyboard.write(r & 0x0f);
	PIA::write_porta(r);
}

void petio::operator=(uint8_t r) {

	if (PIA1_OFF <= _acc && _acc < PIA2_OFF) {
		PIA::write(_acc & 0x0f, r);
		return;
	}

	if (PIA2_OFF <= _acc && _acc < VIA_OFF)
		return;

	if (VIA_OFF <= _acc && _acc < VIA_OFF+0x10)
		print(" >via ", _acc, r);

	switch (_acc - VIA_OFF) {
	case PCR:
		CA2.set(r & 0x02);
		break;

	case VPORTB:
		_portb = (r & _ddrb);
		_ifr &= ~(IER_CB1_ACTIVE | IER_CB2_ACTIVE);
		break;

	case VPORTA:
		_porta = (r & _ddra);
		_ifr &= ~(IER_CA1_ACTIVE | IER_CA2_ACTIVE);
		break;

	case DDRB:
		_ddrb = r;
		break;

	case DDRA:
		_ddra = r;
		break;

	case SHIFT:
		print(" shift ", _acc, r);
		sound_octave(r);
		_ifr &= ~IER_SHIFT_REG;
		break;

	case ACR:
		print(" acr ", _acc, r);
		_acr = r;
		if ((r & VIA_ACR_SHIFT_MASK) == 0x10)
			sound_on();
		else
			sound_off();
		if (r & VIA_ACR_T1_CONTINUOUS)
			_timer1 = true;
		break;

	case IER:
		if (r & 0x80)
			_ier |= r & 0x7f;
		else
			_ier &= ~(r & 0x7f);
		break;

	case IFR:
		_ifr &= ~r;
		break;

	case T1LO:
	case T1LLO:
		_t1_latch = (_t1_latch & 0xff00) | r;
		break;

	case T1LHI:
		_t1_latch = (_t1_latch & 0x00ff) | (r << 8);
		_ifr &= ~IER_TIMER1;
		break;

	case T1HI:
		_t1 = _t1_latch;
		_ifr &= ~IER_TIMER1;
		_timer1 = true;
		break;

	case T2LO:
		_t2 = r;
		_timer2 = false;
		_ifr &= ~IER_TIMER2;
		sound_freq(r);
		break;

	case T2HI:
		_t2 += (r << 8);
		_ifr &= ~IER_TIMER2;
		_timer2 = true;
		break;

	default:
		print(" >??? ", _acc, r);
		break;
	}
}

void petio::sound_off() {
#if defined(PWM_SOUND)
	noTone(PWM_SOUND);
#endif
}

void petio::sound_on() {
	if (_freq > 0) {
		unsigned f = _freq;
		if (_octave == 15)
			f /= 2;
		else if (_octave == 85)
			f *= 2;
#if defined(PWM_SOUND)
		tone(PWM_SOUND, f);
#endif
	}
}

void petio::sound_freq(uint8_t p) {
	if (p == 0)
		sound_off();
	else {
		_freq = 1000000 / (16*p + 30);
		sound_on();
	}
}

void petio::sound_octave(uint8_t o) {
	_octave = o;
	sound_on();
}
