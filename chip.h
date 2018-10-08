#pragma once

#define MEM_SIZE 4 * 1024
#define MAX_ROM_SIZE (0xFFF - 0x200) + 1

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

extern char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];

void chip_init(void);
void chip_load(char *filename);
void chip_tick(void);
void chip_cycle(void);
