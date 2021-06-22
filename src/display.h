#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "mmu.h"

typedef void (*RenderCallback)(int, int, int);
typedef void (*DrawCallback)();

typedef enum {
	DISPLAY_HBLANK,
	DISPLAY_VBLANK,
	DISPLAY_OAM,
	DISPLAY_VRAM
} DisplayMode;

typedef struct {
	MMU *mmu;
	RenderCallback render;
	DrawCallback draw;
	unsigned mode;
	unsigned cycles;
	unsigned scanline;
} Display;

void displayInit(Display*, MMU*);
void displaySetRenderCallback(Display*, RenderCallback);
void displaySetDrawCallback(Display*, DrawCallback);
void displayStep(Display*, unsigned);

#endif
