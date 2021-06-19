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
	RenderCallback render;
	DrawCallback draw;
	DisplayMode mode;
	unsigned cycles;
	unsigned scanline;
} Display;

void displaySetRenderCallback(Display*, RenderCallback);
void displaySetDrawCallback(Display*, DrawCallback);
void displayStep(Display*, MMU*, uint8_t);

#endif
