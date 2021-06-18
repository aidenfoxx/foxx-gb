#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "mmu.h"

typedef void (*RenderCallback)(int, int, int);
typedef void (*DrawCallback)();

#define DISPLAY_MODE_HBLANK 0
#define DISPLAY_MODE_VBLANK 1
#define DISPLAY_MODE_OAM 2
#define DISPLAY_MODE_VRAM 3

typedef struct {
	RenderCallback render;
	DrawCallback draw;
	uint16_t cycles;
	int mode;
	int scanline;
} Display;

void displayInit(Display*);
void displaySetRenderCallback(Display*, RenderCallback);
void displaySetDrawCallback(Display*, DrawCallback);
void displayStep(Display*, MMU*, uint8_t);
void displayScanline(Display*, MMU*);
int displayGetColor(uint16_t, uint8_t);

#endif