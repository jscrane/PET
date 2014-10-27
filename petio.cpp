#include <Energia.h>
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
#define ACR	0x0b
#define PCR	0x0c
#define IFR	0x0d
#define IER	0x0e

byte petio::read() {
	byte r = 0x00;

	switch (_acc) {
	// keyboard in
	case PIA1 + PORTB:
		r = keyboard.read();
		break;

	case PIA1 + PORTA:
		// diagnostic sense high (otherwise get monitor)
		r = 0x80 + keyboard.row();
		break;

	case PIA1 + CRA:
		// video sync in???
//		r = 0x87;
		break;

	case VIA + VPORTA:
		// ~DAV + ~NRFD + ~NDAC
		r = 0x00;
		break;

	case VIA + VPORTB:
		// for now
		break;

	case VIA + IFR:
		// timer #1
		r = 0x40;
		break;

	default:
		// some other device...
		Serial.print(millis());
		Serial.print(' ');
		Serial.println(_acc, 16);
	}
	return r;
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

	default:
		Serial.print(millis());
		Serial.print(' ');
		Serial.print(_acc, 16);
		Serial.print(' ');
		Serial.println(r, 16);
		break;
	}
}
