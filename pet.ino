#include <SPI.h>
#include <SpiRAM.h>
#include <FS.h>
#include <SPIFFS.h>
#include <UTFT.h>
#include <r65emu.h>
#include <r6502.h>

#include <stdarg.h>

#include "config.h"
#include "port.h"
#include "display.h"
#include "kbd.h"
#include "petio.h"

#if defined(SERIES4_ROMS)
#include "roms/basic4_b000.h"
#include "roms/basic4_c000.h"
#include "roms/basic4_d000.h"
#include "roms/kernal4.h"
#include "roms/edit4.h"

#else
#include "roms/basic2_c000.h"
#include "roms/basic2_d000.h"
#include "roms/kernal2.h"
#include "roms/edit2.h"
#endif

#if defined(SERIES4_ROMS)
prom basica(basic4_b000, 4096);
prom basicb(basic4_c000, 4096);
prom basicc(basic4_d000, 4096);
prom kernal(kernal4, 4096);
prom edit(edit4, 2048);

#else
prom basicb(basic2_c000, 4096);
prom basicc(basic2_d000, 4096);
prom kernal(kernal2, 4096);
prom edit(edit2, 2048);
#endif

port irq;
ram pages[RAM_SIZE / 1024];
petio io(irq);
display disp(io.CA2);
r6502 cpu(memory);

const char *filename;

void reset() {
	bool sd = hardware_reset();

	io.reset();
	disp.begin();
	if (sd)
		io.tape.start(PROGRAMS);
	else
		disp.status("No SD Card");
}

void setup() {
	Serial.begin(115200);
	hardware_init(cpu);

	for (int i = 0; i < RAM_SIZE; i += 1024)
		memory.put(pages[i / 1024], i);

#if defined(SPIRAM_CS)
	memory.put(sram, SPIRAM_BASE, SPIRAM_EXTENT);
#endif
	memory.put(disp, 0x8000);
	memory.put(io, 0xe800);

#if defined(SERIES4_ROMS)
	memory.put(basica, 0xb000);
#endif
	memory.put(basicb, 0xc000);
	memory.put(basicc, 0xd000);
	memory.put(edit, 0xe000);
	memory.put(kernal, 0xf000);

	reset();
}

void loop() {
	if (ps2.available()) {
		unsigned scan = ps2.read2();
		uint8_t key = scan & 0xff;
		if (is_down(scan))
			io.keyboard.down(key);
		else {
			char buf[32];
			switch (key) {
			case PS2_F1:
				reset();
				break;
			case PS2_F2:
				filename = io.tape.advance();
				disp.status(filename);
				break;
			case PS2_F3:
				filename = io.tape.rewind();
				disp.status(filename);
				break;
			case PS2_F4:
				if (io.load_prg())
					snprintf(buf, sizeof(buf), "Loaded %s", filename);
				else
					snprintf(buf, sizeof(buf), "Failed to load %s", filename);
				disp.status(buf);
				break;
			case PS2_F6:
				disp.status(checkpoint(io.tape, PROGRAMS));
				break;
			case PS2_F7:
				if (filename)
					restore(io.tape, PROGRAMS, filename);
				break;
			default:
				io.keyboard.up(key);
				break;
			}
		}
	} else if (!cpu.halted()) {
		cpu.run(CPU_INSTRUCTIONS);
		if (irq.read()) {
			irq.write(false);
			cpu.raise(0);
		}
	}
}

