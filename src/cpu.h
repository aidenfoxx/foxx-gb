#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "mmu.h"

#define FLAG_Z 0
#define FLAG_N 1
#define FLAG_H 2
#define FLAG_C 3

typedef struct {
	uint8_t a; uint8_t b;
	uint8_t c; uint8_t d;
	uint8_t e; uint8_t f;
	uint8_t h; uint8_t l;
	uint16_t sp;
	uint16_t pc;
} CPURegs;

typedef struct {
	CPURegs regs;
	unsigned cycles;
	bool cb;
	bool ei; bool ime;
	bool halt; bool stop;
} CPU;

void cpuInit(CPU*);
int cpuGetFlag(CPU*, int);
void cpuSetFlag(CPU*, int, int);
void cpuStep(CPU*, MMU*);

#endif
