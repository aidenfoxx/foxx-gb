#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

#include "mmu.h"

#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_START 4
#define BUTTON_SELECT 5
#define BUTTON_B 6
#define BUTTON_A 7

typedef struct {
	uint8_t directions;
	uint8_t buttons; 
} Joypad;

void inputInit(Joypad*);
void inputStep(Joypad*, MMU*);
void inputTrigger(Joypad*, MMU*, int);
void inputRelease(Joypad*, int);

#endif