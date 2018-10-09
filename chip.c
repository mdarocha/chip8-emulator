#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip.h"

#define X(op) ((op >> 8) & 0x000F)
#define Y(op) ((op >> 4) & 0x000F)
#define KK(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

//utility functions
uint8_t sub(uint8_t x, uint8_t y);
void draw_sprite(uint8_t x, uint8_t y, uint8_t n);

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
    display[PIXEL(0,0)] = WHITE;
    display[PIXEL(1,1)] = WHITE;
    display[PIXEL(63,0)] = WHITE;
    display[PIXEL(0,31)] = WHITE;
    display[PIXEL(63,31)] = WHITE;
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
        case 0x8000: //math operations on registers
            switch(op & 0x000F) {
                case 0x0000: dprint("0x8xy0, set Vx = Vy\n");
                    V[X(op)] = V[Y(op)];
                    break;
                case 0x0001: dprint("0x8xy1, Vx = Vx OR Vy\n");
                    V[X(op)] = V[X(op)] | V[Y(op)];
                    break;
                case 0x0002: dprint("0x8xy2, Vx = Vx AND Vy\n");
                    V[X(op)] = V[X(op)] & V[Y(op)];
                    break;
                case 0x0003: dprint("0x8xy3, Vx = Vx XOR Vy\n");
                    V[X(op)] = V[X(op)] ^ V[Y(op)];
                    break;
                case 0x0004: dprint("0x8xy4, Vx = Vx + Vy, Vf = carry\n");
                    //TODO: figure out a nicer way to set carry
                    int result = V[X(op)] + V[Y(op)];
                    V[0xf] = (result > 255) ? 1 : 0;
                    V[X(op)] = (uint8_t)result;
                    break;
                case 0x0005: dprint("0x8xy5, Vx = Vx - Vy, Vf = NOT borrow\n");
                    V[X(op)] = sub(V[X(op)], V[Y(op)]);
                    break;
                case 0x0006: dprint("0x8xy6, divide Vx by 2 with carry\n");
                    V[0xf] = ((V[X(op)] & 0x000F) == 1) ? 1 : 0;
                    V[X(op)] /= 2;
                    break;
                case 0x0007: dprint("0x8xy7, Vx = Vy - Vx, Vf = NOT borrow\n");
                    V[X(op)] = sub(V[Y(op)], V[X(op)]);
                    break;
                case 0x000E: dprint("0x8xyE, multiply Vx by 2 with carry\n");
                    V[0xf] = (((V[X(op)] >> 8) & 0x000F) == 1) ? 1 : 0;
                    V[X(op)] *= 2;
                    break;
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
        case 0xB000: dprint("0xBnnn, jump to nnn + V0\n");
            PC = NNN(op) + V[0];
            break;
        case 0xD000: dprint("0xDxyn, draw n-byte sprite starting from I at Vx, Vy\n");
            draw_sprite(V[X(op)], V[Y(op)], op & 0x000F);
            PC += 2;
            break;
        default:
            dprint("Unknown opcode: %x\n", op);
            PC+=2;
    }
}

uint8_t sub(uint8_t x, uint8_t y) {
    if(x > y) {
        V[0xf] = 1;
        return x - y;
    } else {
        V[0xf] = 0;
        return y - x;
    }
}

#define BITAT(x, i) ((x >> i) & 0x0F)

void draw_sprite(uint8_t x, uint8_t y, uint8_t n) {
    dprint("drawing sprite at %d, %d, memory %x @ %d\n", x, y, I, n);

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < 8; j++) {
            display[PIXEL(x + i, y + j)] = (BITAT(memory[I + i], j)) ? WHITE : BLACK;
        }
    }
}
