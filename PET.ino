#include <stdarg.h>

#include <SPI.h>
#include <r65emu.h>
#include <r6502.h>
#include <pia.h>
#include <line.h>
#include <via.h>

#include "config.h"
#include "screen.h"
#include "kbd.h"
#include "petio.h"

#if ROM_SET == series4
#include "roms/basic4_b000.h"
#include "roms/basic4_c000.h"
#include "roms/basic4_d000.h"
#include "roms/kernal4.h"
#include "roms/edit4.h"

prom basic_b000(basic4_b000, 4096);
prom basic_c000(basic4_c000, 4096);
prom basic_d000(basic4_d000, 4096);
prom kernal(kernal4, 4096);
prom edit(edit4, 2048);

#elif ROM_SET == series2
#include "roms/basic2_c000.h"
#include "roms/basic2_d000.h"
#include "roms/kernal2.h"
#include "roms/edit2.h"

prom basic_c000(basic2_c000, 4096);
prom basic_d000(basic2_d000, 4096);
prom kernal(kernal2, 4096);
prom edit(edit2, 2048);

#else
#error "ROM_SET not defined"
#endif

Line irq;
ram<> pages[RAM_PAGES];
flash_filer files(PROGRAMS);
petio io(files, irq);
ps2_raw_kbd kbd(io.keyboard);
screen screen(io.CA2);
Memory memory;
r6502 cpu(memory);

const char *filename;

bool reset() {

	bool sd = hardware_reset();
	io.reset();
	kbd.reset();
	screen.begin();
	return sd;
}

void function_keys(uint8_t key) {

	char buf[32];
	switch (key) {
	case 1:
		reset();
		break;
	case 2:
		filename = io.files.advance();
		screen.status(filename? filename: "No file");
		break;
	case 3:
		filename = io.files.rewind();
		screen.status(filename? filename: "No file");
		break;
	case 4:
		if (io.load_prg())
			snprintf(buf, sizeof(buf), "Loaded %s", filename);
		else
			snprintf(buf, sizeof(buf), "Failed to load %s", filename);
		screen.status(buf);
		break;
	case 6:
		screen.status(io.files.checkpoint());
		break;
	case 7:
		if (filename)
			io.files.restore(filename);
		break;
	case 10:
		hardware_debug_cpu();
		break;
	}
}

void setup() {

	hardware_init(cpu);

	for (int i = 0; i < RAM_PAGES; i++)
		memory.put(pages[i], i * ram<>::page_size);

#if defined(USE_SPIRAM)
	memory.put(sram, SPIRAM_BASE, SPIRAM_EXTENT);
#endif
	memory.put(screen, 0x8000);
	memory.put(io, 0xe800);

#if ROM_SET == series4
	memory.put(basic_b000, 0xb000);
#endif
	memory.put(basic_c000, 0xc000);
	memory.put(basic_d000, 0xd000);
	memory.put(edit, 0xe000);
	memory.put(kernal, 0xf000);

	kbd.register_fnkey_handler(function_keys);

	bool sd = reset();

	if (!sd)
		screen.status("No SD Card");
	else if (!io.start())
		screen.status("Failed to open " PROGRAMS);
}

void loop() {

	kbd.poll();

	if (hardware_run() && irq) {
		irq.clear();
		cpu.raise(0);
	}
}
