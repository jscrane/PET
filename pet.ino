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

#include "roms/basic4_b000.h"
#include "roms/basic4_c000.h"
#include "roms/basic4_d000.h"
#include "roms/kernal4.h"
#include "roms/edit4.h"

static bool halted = false;

prom basica(basic4_b000, 4096);
prom basicb(basic4_c000, 4096);
prom basicc(basic4_d000, 4096);
prom kernal(kernal4, 4096);
prom edit(edit4, 2048);

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
char chkpt[] = { "CHKPOINT" };
int cpid = 0;

void reset() {
  bool sd = hardware_init(cpu);

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

  for (int i = 0; i < RAM_SIZE; i += 1024)
    memory.put(pages[i / 1024], i);

  memory.put(sram, SPIRAM_BASE, SPIRAM_EXTENT);
  memory.put(disp, 0x8000);
  memory.put(io, 0xe800);

  memory.put(basica, 0xb000);
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
      char cpbuf[32];
      int n;
      File file;
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
      case PS2_F6:
        io.tape.stop();
        snprintf(cpbuf, sizeof(cpbuf), PROGRAMS"%s.%03d", chkpt, cpid++);
        file = SD.open(cpbuf, O_WRITE | O_CREAT | O_TRUNC);
        hardware_checkpoint(file);
        file.close();
        io.tape.start(PROGRAMS);
        disp.status(cpbuf);
        break;
      case PS2_F7:
        if (filename) {
          io.tape.stop();
          snprintf(cpbuf, sizeof(cpbuf), PROGRAMS"%s", filename);
          file = SD.open(cpbuf, O_READ);
          hardware_restore(file);
          file.close();
          n = sscanf(cpbuf + strlen(PROGRAMS), "%[A-Z0-9].%d", chkpt, &cpid);
          cpid = (n == 1)? 0: cpid+1;
          io.tape.start(PROGRAMS);
        }
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

