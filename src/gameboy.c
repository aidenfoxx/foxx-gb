#include "gameboy.h"

void gameboyInit(Gameboy *gameboy, Cartridge *cartridge)
{
	mmuInit(&gameboy->mmu, cartridge);
	cpuInit(&gameboy->cpu, &gameboy->mmu);
	timerInit(&gameboy->timer, &gameboy->mmu);
	displayInit(&gameboy->display, &gameboy->mmu);
	inputInit(&gameboy->input, &gameboy->mmu);
}

void gameboyStep(Gameboy *gameboy)
{
	uint8_t cycles = cpuStep(&gameboy->cpu);

	inputStep(&gameboy->input);
	timerStep(&gameboy->timer, cycles);
	displayStep(&gameboy->display, cycles);
}
