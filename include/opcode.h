#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>

#include "cpu.h"
#include "mmu.h"

uint8_t cpuOpcode(CPU*, MMU*, uint8_t);
uint8_t cpuOpcodeCB(CPU*, MMU*, uint8_t);

#endif