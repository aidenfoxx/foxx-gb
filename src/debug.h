#ifndef DEBUG_H
#define DEBUG_H

#include "cpu.h"
#include "mmu.h"

void debugCPU(CPU*, MMU*, uint8_t);
char *debugCPUOpcode(uint8_t);
char *debugCPUOpcodeCB(uint8_t);

#endif