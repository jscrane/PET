#include <Arduino.h>
#include <hardware.h>
#include "sound.h"

void sound::off() {
#if defined(PWM_SOUND)
	noTone(PWM_SOUND);
#endif
}

void sound::on() {
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

void sound::frequency(uint8_t p) {
	if (p == 0)
		off();
	else {
		_freq = 1000000 / (16*p + 30);
		on();
	}
}

void sound::octave(uint8_t o) {
	_octave = o;
	on();
}
