#include "gameboy.h"
#include "debug.h"

static uint8_t debug = 0;

void gameboyInit(Gameboy *gameboy, uint8_t *romData, size_t romSize)
{
	mmuInit(&gameboy->mmu, romData, romSize);
	cpuInit(&gameboy->cpu, &gameboy->mmu);
	timerInit(&gameboy->timer, &gameboy->mmu);
	displayInit(&gameboy->display, &gameboy->mmu);
	inputInit(&gameboy->input, &gameboy->mmu);
}

void gameboyStep(Gameboy *gameboy)
{
	// if (gameboy->cpu.regs.pc == 0xC2EC) debug = 1;

	if (debug) {
		debugCPU(&gameboy->cpu, &gameboy->mmu);
	}


	unsigned cycles = cpuStep(&gameboy->cpu);

	inputStep(&gameboy->input);
	timerStep(&gameboy->timer, cycles);
	displayStep(&gameboy->display, cycles);
}
