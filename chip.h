#pragma once

#ifdef DEBUG
    #define dprint(...) printf(__VA_ARGS__)
#else
    #define dprint(...)
#endif

#define MEM_SIZE 4 * 1024
#define MAX_ROM_SIZE (0xFFF - 0x200) + 1

#define TICK_US (1.0f/60) * 1000 * 1000
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define PIXEL(x, y) ((DISPLAY_HEIGHT - y - 1) * DISPLAY_WIDTH + x)
#define WHITE 255
#define BLACK 0

#define KEYS_NUM 16

extern char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];
extern char keyboard[KEYS_NUM];

extern int should_draw;

void chip_init(void);
void chip_load(char *filename);
void chip_tick(void);
void chip_cycle(void);
