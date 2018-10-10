#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip.h"
#include "text.h"

#define X(op) ((op >> 8) & 0x000F)
#define Y(op) ((op >> 4) & 0x000F)
#define KK(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

//utility functions
uint8_t sub(uint8_t x, uint8_t y);
void draw_sprite(uint8_t x, uint8_t y, uint8_t n);
uint8_t random_num(void);

char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];
char keyboard[KEYS_NUM];

uint8_t memory[MEM_SIZE];
uint8_t V[16];
uint16_t I;
uint8_t delay;
uint8_t sound;
uint16_t PC;
uint16_t SP;
uint16_t stack[16];

int should_draw;

void chip_init() {
    memset(memory, 0, sizeof(uint8_t) * MEM_SIZE);
    //copy character data to the beggining of ram
    memcpy(memory, characters, sizeof(characters) / sizeof(uint8_t));

    memset(V, 0, sizeof(uint8_t) * 16);
    I = 0;
    delay = 0;
    sound = 0;
    PC = 0x200;
    SP = 0x0;
    memset(stack, 0, sizeof(uint16_t) * 16);

    memset(display, 0, sizeof(char) * DISPLAY_HEIGHT * DISPLAY_WIDTH);
    memset(keyboard, 0, sizeof(char) * KEYS_NUM);

    srand(time(NULL));
    should_draw = 0;
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

    should_draw = 0;

    switch(op & 0xF000) {
        case 0x0000:
            switch(op & 0x00FF) {
                case 0x00E0: dprint("0x00E0, clear display\n");
                    memset(display, 0, DISPLAY_HEIGHT * DISPLAY_WIDTH);
                    should_draw = 1;
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
        case 0xC000: dprint("0xCxkk, store random AND kk in Vx\n");
            V[X(op)] = random_num() & KK(op);
            PC += 2;
            break;
        case 0xD000: dprint("0xDxyn, draw n-byte sprite starting from I at Vx, Vy\n");
            draw_sprite(V[X(op)], V[Y(op)], op & 0x000F);
            PC += 2;
            break;
        case 0xE000:
            switch(op & 0x00FF) {
                case 0x009E: dprint("0xEx9E, skip next line if key with value of Vx is pressed\n");
                    PC += (keyboard[V[X(op)]] == 1) ? 4 : 2;
                    break;
                case 0x00A1: dprint("0xExA1, skip next line if key with value of Vx is not pressed\n");
                    PC += (keyboard[V[X(op)]] == 0) ? 4 : 2;
                    break;
            }
            break;
        case 0xF000:
            switch(KK(op)) {
                case 0x07: dprint("0xFx07, set Vx = delay timer\n");
                    V[X(op)] = delay;
                    break;
                case 0x0A: dprint("0xFx0A, wait for key press and store it in Vx\n");
                    //TODO
                    break;
                case 0x15: dprint("0xFx15, set delay timer = Vx\n");
                    delay = V[X(op)];
                    break;
                case 0x18: dprint("0xFx18, set sound timer = Vx\n");
                    sound = V[X(op)];
                    break;
                case 0x1E: dprint("0xFx1E, set I = I + Vx\n");
                    I += V[X(op)];
                    break;
                case 0x29: dprint("0xFx29, set I = location of sprite for digit in Vx\n");
                    I = 0x0 + CHAR_LEN * V[X(op)];
                    break;
                case 0x33: dprint("0xFx33, store BCD of Vx in memory, starting at I\n");
                    uint8_t i = V[X(op)];
                    memory[I + 0] = (i % 1000) / 100;
                    memory[I + 1] = (i % 100) / 10;
                    memory[I + 2] = (i % 10);
                    break;
                case 0x55: dprint("0xFx55, store registers V0-Vx in memory, starting at I\n");
                    for(int i = 0; i < X(op); i++)
                        memory[I + i] = V[i];
                    break;
                case 0x65: dprint("0xF65, read register V0-Vx from memory, starting at I\n");
                    for(int i = 0; i < X(op); i++)
                        V[i] = memory[I + i];
                    break;
            }
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

#define BITAT(x, i) ((x >> i) & 0x1)

void draw_sprite(uint8_t xpos, uint8_t ypos, uint8_t n) {
    dprint("drawing sprite at %d, %d, memory %x @ %d\n", xpos, ypos, I, n);

    V[0xf] = 0;
    for(int byte = 0; byte < n; byte++) {
        for(int bit = 0; bit < 8; bit++) {
            int x = (xpos + (7 - bit)) % DISPLAY_WIDTH;
            int y = (ypos + byte) % DISPLAY_HEIGHT;

            int color = (BITAT(memory[I + byte], bit) == 1) ? WHITE : BLACK;
            if(color == WHITE) {
                should_draw = 1;
                if(display[PIXEL(x, y)] == BLACK) {
                    display[PIXEL(x, y)] = WHITE;
                } else {
                    V[0xf] = 1;
                    display[PIXEL(x, y)] = BLACK;
                }
            }
        }
    }
}

uint8_t random_num() {
    return rand() % 256;
}
