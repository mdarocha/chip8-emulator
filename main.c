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
    sleep(1);
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
