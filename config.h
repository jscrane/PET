#ifndef __CONFIG_H__
#define __CONFIG_H__

// DISPLAY
#define TFT_BG		VGA_BLACK
#define TFT_FG		VGA_LIME
#define CHARS_PER_LINE  40
#define X_OFF  0
#define DISPLAY_RAM_SIZE  1024

// RAM provided by uC (must be a multiple of 1024)
#define RAM_SIZE	0x2000

// SPI-RAM
#define SPIRAM_SIZE     0x6000
#define SPIRAM_BASE     0x2000

// number of CPU instructions to run per loop
#define CPU_INSTRUCTIONS  750

#endif
