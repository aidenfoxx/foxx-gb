#include "gameboy.h"

void gameboyInit(Gameboy *gameboy, Cartridge *cartridge)
{
	CPU *cpu = malloc(sizeof(CPU));
	cpuInit(cpu);

	MMU *mmu = malloc(sizeof(MMU));
	mmuInit(mmu, cartridge);

	Timer *timer = malloc(sizeof(Timer));
	timerInit(timer);

	Display *display = malloc(sizeof(Display));
	displayInit(display);

	Joypad *joypad = malloc(sizeof(Joypad));
	inputInit(joypad);

	gameboy->cpu = cpu;
	gameboy->mmu = mmu;
	gameboy->timer = timer;
	gameboy->display = display;
	gameboy->joypad = joypad;
}

void gameboyFree(Gameboy *gameboy)
{
	cpuFree(gameboy->cpu);
	mmuFree(gameboy->mmu);
	free(gameboy->cpu);
	free(gameboy->mmu);
	free(gameboy->timer);
	free(gameboy->display);
	free(gameboy->joypad);
}

void gameboyStep(Gameboy *gameboy)
{
	inputStep(gameboy->joypad, gameboy->mmu);
	cpuStep(gameboy->cpu, gameboy->mmu);
	timerStep(gameboy->timer, gameboy->mmu, gameboy->cpu->cycles);
	displayStep(gameboy->display, gameboy->mmu, gameboy->cpu->cycles);
}