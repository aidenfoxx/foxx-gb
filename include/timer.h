#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "mmu.h"

typedef struct {
	uint8_t divcount;
	uint16_t timacount;
} Timer;

void timerInit(Timer*);
void timerStep(Timer*, MMU*, uint8_t);

#endif