#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip.h"

#define X(op) ((op >> 8) & 0x000F)
#define Y(op) ((op >> 4) & 0x000F)
#define KK(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];

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

    memset(display, 0, sizeof(char) * DISPLAY_HEIGHT * DISPLAY_WIDTH);
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

void print_registers() {
    for(int i = 0; i < 16; i++) {
        printf("V[%x] = %x, ", i, V[i]);
        if(i == 8)
            putchar('\n');
    }
    printf("\nPC = %x, SP = %x, I = %x\n--\n", PC, SP, I);
}

void chip_cycle() {
#ifdef DEBUG
    print_registers();
#endif

    if(PC >= MAX_ROM_SIZE) {
        printf("Error: memory out of bounds\n");
        exit(-1);
    }

    uint16_t op = memory[PC] << 8 | memory[PC+1];

    dprint("Running opcode %x\n", op);

    switch(op & 0xF000) {
        case 0x0000:
            switch(op & 0x00FF) {
                case 0x00E0: dprint("0x00E0, clear display\n");
                    memset(display, 0, DISPLAY_HEIGHT * DISPLAY_WIDTH);
                    PC += 2;
                    break;
                case 0x00EE: dprint("0x00EE, return from subroutine\n");
                    PC = stack[--SP];
                    break;
            }
            break;
        case 0x1000: dprint("0x1nnn, jump to nnn\n");
            PC = NNN(op);
            break;
        case 0x2000: dprint("0x2nnn, call subroutine\n");
            stack[SP++] = PC + 2;
            PC = NNN(op);
            break;
        case 0x3000: dprint("0x3xkk, skip next if Vx == kk\n");
            PC += (V[X(op)] == KK(op)) ? 4 : 2;
            break;
        case 0x4000: dprint("0x4xkk, skip next if Vx != kk\n");
            PC += (V[X(op)] != KK(op)) ? 4 : 2;
            break;
        case 0x5000: dprint("0x5xy0, skip next if Vx == Vy\n");
            PC += (V[X(op)] == V[Y(op)]) ? 4 : 2;
            break;
        case 0x6000: dprint("0x6xkk, set Vx = kk\n");
            V[X(op)] = KK(op);
            PC += 2;
            break;
        case 0x7000: dprint("0x7xkk, set Vx += kk\n");
            V[X(op)] += KK(op);
            PC += 2;
            break;
        case 0x8000:
            switch(op & 0x000F) {

            }
            PC += 2;
            break;
        case 0x9000: dprint("0x9xy0, skip next if Vx != Vy\n");
            PC += (V[X(op)] != V[Y(op)]) ? 4 : 2;
            break;
        case 0xA000: dprint("0xAnnn, set I = nnn\n");
            I = NNN(op);
            PC += 2;
            break;
        default:
            dprint("Unknown opcode: %x\n", op);
            PC+=2;
    }
}
