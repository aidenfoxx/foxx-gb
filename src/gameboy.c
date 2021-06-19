#include "gameboy.h"

void gameboyInit(Gameboy *gameboy, Cartridge *cartridge)
{
	cpuInit(&gameboy->cpu);
	mmuInit(&gameboy->mmu, cartridge);
	inputInit(&gameboy->joypad);
}

void gameboyStep(Gameboy *gameboy)
{
	inputStep(&gameboy->joypad, &gameboy->mmu);
	cpuStep(&gameboy->cpu, &gameboy->mmu);
	timerStep(&gameboy->timer, &gameboy->mmu, gameboy->cpu.cycles);
	displayStep(&gameboy->display, &gameboy->mmu, gameboy->cpu.cycles);
}
