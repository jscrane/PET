#include <SPI.h>
#include <SpiRAM.h>
#include <UTFT.h>
#include <SD.h>
#include <r65emu.h>

#include <setjmp.h>
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

static bool halted = false;

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

void status(const char *fmt, ...) {
  char tmp[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  disp.clear();
  disp.error(tmp);
  Serial.println(tmp);
  va_end(args);
}

jmp_buf ex;
r6502 cpu(&memory, &ex, status);

const char *filename;

void reset() {
  bool sd = hardware_reset();

  io.reset();
  disp.begin();
  if (sd)
    io.tape.start(PROGRAMS);
  else
    disp.status("No SD Card");

  halted = (setjmp(ex) != 0);
}

void setup() {
  Serial.begin(115200);
  hardware_init(cpu);

  for (int i = 0; i < RAM_SIZE; i += 1024)
    memory.put(pages[i / 1024], i);

  memory.put(sram, SPIRAM_BASE, SPIRAM_EXTENT);
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
    unsigned key = ps2.read();
    if (!ps2.isbreak())
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
  } else if (!halted) {
    cpu.run(CPU_INSTRUCTIONS);
    if (irq.read()) {
      irq.write(false);
      cpu.raise(0);
    }
  }
}

