# chip8-emulator
CHIP-8 emulator/interpreter written in C.
It uses OpenGL and GLUT for display and keyboard.
Currently tested only under Linux.

## Installing
Make sure you have OpenGL and GLUT installed and working.
Clone the repo
```
$ git clone https://github.com/mdarocha/chip8-emulator.git
```
Enter the repository and run Make
```
$ cd chip8-emulator
$ make
```
## Usage
```
chip game.c8
```
The program loads any binary file given in the arguments and
attemps to run it as a CHIP-8 rom.

The program maps the following keys to the CHIP-8 keypad:
```
1 2 3 4    1 2 3 C
Q W E R >> 4 5 6 D
A S D F    7 8 9 E
Z X C V    A 0 B F
```
