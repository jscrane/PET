#include <Arduino.h>
#include <memory.h>
#include <keyboard.h>
#include <serialio.h>
#include <filer.h>
#include <timed.h>
#include <sound_pwm.h>
#include <hardware.h>

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

static PWM pwm;

void petio::reset() {
	io = this;

	_acr = _ifr = _ier = 0x00;
	_t1 = _t2 = 0;
	_timer1 = _timer2 = false;

	keyboard.reset();
}

bool petio::start() {
#if defined(PWM_SOUND)
	pwm.begin(PWM_SOUND);
#endif

	timer_create(TICK_FREQ, petio::on_tick);

	return files.start();
}

#if !defined(ESP32)
#define IRAM_ATTR
#endif

void IRAM_ATTR petio::on_tick() {
	io->tick();
}

void IRAM_ATTR petio::tick() {
	if (_ticks++ == SYS_TICKS) {
		_ticks = 0;
		_portb |= VIA_VIDEO_RETRACE;
		_irq.write(true);
	} else
		_portb &= ~VIA_VIDEO_RETRACE;

	if (_timer1) {
		if (_t1 < TICK_FREQ) {
			_t1 = 0;
			_timer1 = false;
			_ifr |= IER_TIMER1;
			if ((_ier & IER_MASTER) && (_ier & IER_TIMER1))
				_irq.write(true);
		} else
			_t1 -= TICK_FREQ;
	}

	if (_timer2) {
		if (_t2 < TICK_FREQ) {
			_t2 = 0;
			_timer2 = false;
			_ifr |= IER_TIMER2;
			if ((_ier & IER_MASTER) && (_ier & IER_TIMER2))
				_irq.write(true);
		} else
			_t2 -= TICK_FREQ;
	}
}

static void print(const char *msg, Memory::address a)
{
#if defined(DEBUGGING)
	Serial.print(millis());
	Serial.print(msg);
	Serial.println(a, 16);
#endif
}

static void print(const char *msg, Memory::address a, uint8_t r) {
#if defined(DEBUGGING)
	Serial.print(millis());
	Serial.print(msg);
	Serial.print(a, 16);
	Serial.print(' ');
	Serial.println(r, 16);
#endif
}

uint8_t petio::read() {
	uint8_t r = 0x00;

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
		_ifr &= ~(IER_CA1_ACTIVE | IER_CA2_ACTIVE);
		break;

	case VIA + VPORTB:
		// screen retrace in
		r = _portb;
		_ifr &= ~(IER_CB1_ACTIVE | IER_CB2_ACTIVE);
		break;

	case VIA + SHIFT:
		print(" shift ", _acc);
		_ifr &= ~IER_SHIFT_REG;
		break;

	case VIA + ACR:
		print(" acr ", _acc);
		// acr bits 5-7 relate to timers
		r = _acr;
		break;

	case VIA + IER:
		print(" ifr ", _acc);
		r = _ier | 0x80;
		break;

	case VIA + IFR:
		print(" ifr ", _acc);
		r = _ifr;
		break;

	case VIA + T1LO:
		r = (_t1 & 0xff);
		_ifr &= ~IER_TIMER1;
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
		_ifr &= ~IER_TIMER2;
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

void petio::write(uint8_t r) {
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
		_ifr &= ~(IER_CB1_ACTIVE | IER_CB2_ACTIVE);
		break;

	case VIA + VPORTA:
		_porta = (r & _ddra);
		_ifr &= ~(IER_CA1_ACTIVE | IER_CA2_ACTIVE);
		break;

	case VIA + DDRB:
		_ddrb = r;
		break;

	case VIA + DDRA:
		_ddra = r;
		break;

	case VIA + SHIFT:
		print(" shift ", _acc, r);
		sound_octave(r);
		_ifr &= ~IER_SHIFT_REG;
		break;

	case VIA + ACR:
		print(" acr ", _acc, r);
		_acr = r;
		if ((r & VIA_ACR_SHIFT_MASK) == 0x10)
			sound_on();
		else
			sound_off();
		if (r & VIA_ACR_T1_CONTINUOUS)
			_timer1 = true;
		break;

	case VIA + IER:
		if (r & 0x80)
			_ier |= r & 0x7f;
		else
			_ier &= ~(r & 0x7f);
		break;

	case VIA + IFR:
		_ifr &= ~r;
		break;

	case VIA + T1LO:
	case VIA + T1LLO:
		_t1_latch = (_t1_latch & 0xff00) | r;
		break;

	case VIA + T1LHI:
		_t1_latch = (_t1_latch & 0x00ff) | (r << 8);
		_ifr &= ~IER_TIMER1;
		break;

	case VIA + T1HI:
		_t1 = _t1_latch;
		_ifr &= ~IER_TIMER1;
		_timer1 = true;
		break;

	case VIA + T2LO:
		_t2 = r;
		_timer2 = false;
		_ifr &= ~IER_TIMER2;
		sound_freq(r);
		break;

	case VIA + T2HI:
		_t2 += (r << 8);
		_ifr &= ~IER_TIMER2;
		_timer2 = true;
		break;

	default:
		print(" >??? ", _acc, r);
		break;
	}
}

void petio::sound_on() {
#if defined(PWM_DUTY)
	pwm.set_duty(PWM_DUTY);
#endif
}

void petio::sound_off() {
	pwm.stop();
}

void petio::sound() {
	if (_freq > 0) {
		unsigned f = _freq;
		if (_octave == 15)
			f /= 2;
		else if (_octave == 85)
			f *= 2;
		sound_on();
		pwm.set_freq(f);
	}
}

void petio::sound_freq(uint8_t p) {
	if (p == 0)
		sound_off();
	else {
		_freq = 1000000 / (16*p + 30);
		sound();
	}
}

void petio::sound_octave(uint8_t o) {
	_octave = o;
	sound();
}
