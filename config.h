#ifndef __CONFIG_H__
#define __CONFIG_H__

// DISPLAY
#define TFT_BG		BLACK
#define TFT_FG		GREEN
#define CHARS_PER_LINE  40
#define DISPLAY_LINES	25
#define DISPLAY_RAM_SIZE	0x0400

#if defined(USE_UTFT)
#define TFT_ORIENT	landscape
#elif defined(USE_ESPI)
#define TFT_ORIENT	reverse_landscape
#endif

// SPI-RAM
#if defined(USE_SPIRAM)
#define SPIRAM_BASE     RAM_SIZE
#define SPIRAM_EXTENT	((0x8000 - RAM_SIZE) / Memory::page_size)
#endif

#define RAM_PAGES	(RAM_SIZE / ram::page_size)

// number of CPU instructions to run per loop
#define CPU_INSTRUCTIONS  750

// where programs and images are stored
#if defined(USE_SD)
#define PROGRAMS        "/pet/"
#else
#define PROGRAMS        "/"
#endif

// rom set
#define SERIES4_ROMS

#endif
