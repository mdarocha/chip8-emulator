#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip.h"

uint8_t memory[MEM_SIZE];
uint8_t V[16];
uint16_t I;
uint8_t delay;
uint8_t sound;
uint16_t PC;
uint16_t SP;
uint16_t stack[16];

void chip_init() {
    memset(memory, 0, sizeof(uint8_t) * MEM_SIZE);
    memset(V, 0, sizeof(uint8_t) * 16);
    I = 0;
    delay = 0;
    sound = 0;
    PC = 0x200;
    SP = 0x0;
    memset(stack, 0, sizeof(uint16_t) * 16);
}

void chip_load(char *filename) {
    FILE *file;
    file = fopen(filename, "rb");
    if(!file) {
        printf("Can't open file\n");
        exit(-1);
    }

    fread(memory + 0x200, sizeof(uint8_t), MAX_ROM_SIZE, file);
    fclose(file);
}

void chip_tick() {
    if(delay > 0)
        delay--;
    if(sound > 0)
        sound--;
}

void chip_cycle() {

}
