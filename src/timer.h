#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "mmu.h"

typedef struct {
	MMU *mmu;
	uint8_t divcount;
	uint16_t timacount;
} Timer;

void timerInit(Timer*, MMU*);
void timerStep(Timer*, unsigned);

#endif
