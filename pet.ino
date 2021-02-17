#include <stdarg.h>

#include <SPI.h>
#include <r65emu.h>
#include <r6502.h>

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
ram pages[RAM_PAGES];
flash_filer files(PROGRAMS);
petio io(files, irq);
display disp(io.CA2);
r6502 cpu(memory);

const char *filename;

void reset() {
	bool sd = hardware_reset();

	io.reset();
	disp.begin();
	if (!sd)
		disp.status("No SD Card");
	else if (!io.start())
		disp.status("Failed to open " PROGRAMS);
}

void setup() {
#if defined(DEBUGGING) || defined(CPU_DEBUG)
	Serial.begin(115200);
#endif

	hardware_init(cpu);

	for (int i = 0; i < RAM_PAGES; i++)
		memory.put(pages[i], i * ram::page_size);

#if defined(USE_SPIRAM)
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
#if defined(CPU_DEBUG)
	static bool cpu_debug;
#endif

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
				filename = io.files.advance();
				disp.status(filename? filename: "No file");
				break;
			case PS2_F3:
				filename = io.files.rewind();
				disp.status(filename? filename: "No file");
				break;
			case PS2_F4:
				if (io.load_prg())
					snprintf(buf, sizeof(buf), "Loaded %s", filename);
				else
					snprintf(buf, sizeof(buf), "Failed to load %s", filename);
				disp.status(buf);
				break;
			case PS2_F6:
				disp.status(io.files.checkpoint());
				break;
			case PS2_F7:
				if (filename)
					io.files.restore(filename);
				break;
#if defined(CPU_DEBUG)
			case PS2_F10:
				cpu_debug = !cpu_debug;
				break;
#endif
			default:
				io.keyboard.up(key);
				break;
			}
		}
	} else if (!cpu.halted()) {
#if defined(CPU_DEBUG)
		if (cpu_debug) {
			char buf[256];
			Serial.println(cpu.status(buf, sizeof(buf)));
			cpu.run(1);
		} else
			cpu.run(CPU_INSTRUCTIONS);
#else
		cpu.run(CPU_INSTRUCTIONS);
#endif
		if (irq.read()) {
			irq.write(false);
			cpu.raise(0);
		}
	}
}

