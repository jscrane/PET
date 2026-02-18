#pragma once

// Screen
#define BG_COLOUR	BLACK
#define FG_COLOUR	GREEN
#define CHARS_PER_LINE  40
#define SCREEN_LINES	25
#define SCREEN_RAM_SIZE	0x0400

#if defined(USE_VGA) || defined(USE_DVI)
#define ORIENT	landscape
#elif defined(USE_ESPI)
#define ORIENT	reverse_landscape
#endif

// SPI-RAM
#if defined(USE_SPIRAM)
#define SPIRAM_BASE     RAM_SIZE
#define SPIRAM_EXTENT	((0x8000 - RAM_SIZE) / Memory::page_size)
#endif

#define RAM_PAGES	(RAM_SIZE / ram<>::page_size)

// where programs and images are stored
#if defined(USE_SD)
#define PROGRAMS        "/pet/"
#else
#define PROGRAMS        "/"
#endif

// rom set
#if !defined(ROM_SET)
#define ROM_SET	series4
//#define ROM_SET series2
#endif

// character set
#if !defined(CHARSET_ROM)
#define CHARSET_ROM	"roms/characters2.h"
#endif
