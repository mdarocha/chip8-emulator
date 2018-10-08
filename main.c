#include <stdio.h>
#include "chip.h"

int main(int argc, char *argv[]) {
    printf("Hello\n");
    chip_init();
    chip_load(argv[1]);
}
