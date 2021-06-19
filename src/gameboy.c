#include "gameboy.h"

void gameboyInit(Gameboy *gameboy, Cartridge *cartridge)
{
	mmuInit(&gameboy->mmu, cartridge);
	displayInit(&gameboy->display);
	inputInit(&gameboy->joypad);
}

void gameboyStep(Gameboy *gameboy)
{
	inputStep(&gameboy->joypad, &gameboy->mmu);
	cpuStep(&gameboy->cpu, &gameboy->mmu);
	timerStep(&gameboy->timer, &gameboy->mmu, gameboy->cpu.cycles);
	displayStep(&gameboy->display, &gameboy->mmu, gameboy->cpu.cycles);
}
