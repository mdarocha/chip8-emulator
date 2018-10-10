#include <stdio.h>
#include <unistd.h>
#include <GL/glut.h>
#include "chip.h"

#define DISPLAY_ZOOM 5

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPixelZoom(DISPLAY_ZOOM, DISPLAY_ZOOM);
    glDrawPixels(DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, display);
    glutSwapBuffers();
}

void update() {
    chip_cycle();
    draw();

    chip_tick();
}

int keyboardMap(unsigned char key) {
    switch(key) {
        case '1':
            return 0x1;
        case '2':
            return 0x2;
        case '3':
            return 0x3;
        case '4':
            return 0xC;
        case 'q':
            return 0x4;
        case 'w':
            return 0x5;
        case 'e':
            return 0x6;
        case 'r':
            return 0xD;
        case 'a':
            return 0x7;
        case 's':
            return 0x8;
        case 'd':
            return 0x9;
        case 'f':
            return 0xE;
        case 'z':
            return 0xA;
        case 'x':
            return 0x0;
        case 'c':
            return 0xB;
        case 'v':
            return 0xF;
    }
    return -1;
}

void keyboardUp(unsigned char key, int x, int y) {
    if(keyboardMap(key) != -1)
        keyboard[keyboardMap(key)] = 0;
}

void keyboardDown(unsigned char key, int x, int y) {
    if(keyboardMap(key) != -1)
        keyboard[keyboardMap(key)] = 1;
}

int main(int argc, char *argv[]) {
    printf("Loading file %s\n", argv[1]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
    glutInitWindowPosition(10, 10);
    glutInitWindowSize(DISPLAY_WIDTH * DISPLAY_ZOOM, DISPLAY_HEIGHT * DISPLAY_ZOOM);
    glutCreateWindow("chip8-emulator");

    glutDisplayFunc(draw);
    glutIdleFunc(update);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    chip_init();
    chip_load(argv[1]);

    glutMainLoop();

    return 1;
}
