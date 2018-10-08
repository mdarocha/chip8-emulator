#include <stdio.h>
#include <GL/glut.h>
#include "chip.h"

#define DISPLAY_ZOOM 5

void update() {
    chip_cycle();
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, display);
    glutSwapBuffers();
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

    chip_init();
    chip_load(argv[1]);

    glutMainLoop();

    return 1;
}
