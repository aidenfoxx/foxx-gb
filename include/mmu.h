#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "cartridge.h"

typedef struct {
	uint8_t vram[8192];
	uint8_t wram[8192];
    uint8_t oam[160];
    uint8_t io[128];
    uint8_t hram[127];
    uint8_t ie;
} Memory;

typedef struct {
	Memory *memory;
	Cartridge *cartridge;
	int bios;
} MMU;

void mmuInit(MMU*, Cartridge*);
void mmuFree(MMU*);
uint8_t mmuReadByte(MMU*, uint16_t);
uint16_t mmuReadWord(MMU*, uint16_t);
void mmuWriteByte(MMU*, uint16_t, uint8_t);
void mmuWriteWord(MMU*, uint16_t, uint16_t);

#endif