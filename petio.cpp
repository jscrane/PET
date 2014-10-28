#include <Energia.h>
#include <stdint.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include <memory.h>
#include <keyboard.h>
#include <sdtape.h>

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
#define VPORTA	0x01
#define VPORTB	0x00
#define T1LO	0x04
#define T1HI	0x05
#define T1LLO	0x06
#define T1LHI	0x07
#define T2LO	0x08
#define T2HI	0x09
#define ACR	0x0b
#define PCR	0x0c
#define IFR	0x0d
#define IER	0x0e

static petio *io;

static void timer0isr(void) {
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	io->tick();
}

void petio::reset() {
	keyboard.reset();

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerIntRegister(TIMER0_BASE, TIMER_A, timer0isr);
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / 20);
	io = this;
}

void petio::tick() {
	_irq.write(true);

	if (_timer2) {
		if (_t2 < 0x100) {
			_t2 = 0;
			_timer2 = false;
		} else
			_t2 -= 0x100;
	}
}

static void print(const char *msg, Memory::address a)
{
	Serial.print(millis());
	Serial.print(msg);
	Serial.println(a, 16);
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
		r = 0x00;
		break;

	case VIA + VPORTB:
		// for now
		break;

	case VIA + ACR:
		print(" acr ", _acc);
		break;

	case VIA + IFR:
		print(" ifr ", _acc);
		// timer #1
		r = 0x40;
		break;

	case VIA + T1LO:
	case VIA + T1HI:
	case VIA + T1LLO:
	case VIA + T1LHI:
		print(" timer1 ", _acc);
		break;

	case VIA + T2LO:
		r = (_t2 & 0xff);
		break;

	case VIA + T2HI:
		r = (_t2 / 0xff);
		break;
	default:
		// some other device...
		print(" ", _acc);
		break;
	}
	return r;
}

static void print(const char *msg, Memory::address a, byte r) {
	Serial.print(millis());
	Serial.print(msg);
	Serial.print(a, 16);
	Serial.print(' ');
	Serial.println(r, 16);
}

void petio::write(byte r) {
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
		// for now
		break;

	case VIA + ACR:
		print(" acr ", _acc, r);
		break;

	case VIA + IER:
		print(" ier ", _acc, r);
		break;

	case VIA + T1LO:
	case VIA + T1HI:
	case VIA + T1LLO:
	case VIA + T1LHI:
		print(" timer1 ", _acc, r);
		break;

	case VIA + T2LO:
		_t2 = r;
		_timer2 = false;
		break;

	case VIA + T2HI:
		_t2 = (r * 256) + r;
		_timer2 = true;
		break;

	default:
		print(" ", _acc, r);
		break;
	}
}
