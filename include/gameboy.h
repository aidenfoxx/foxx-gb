#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "timer.h"
#include "display.h"
#include "input.h"

typedef struct {
	CPU *cpu;
	MMU *mmu;
	Timer *timer;
	Display *display;
	Joypad *joypad;
} Gameboy;

void gameboyInit(Gameboy*, Cartridge*);
void gameboyStep(Gameboy*);
void gameboyFree(Gameboy*);

#endif