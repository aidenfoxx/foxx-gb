#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "cartridge.h"

typedef union {
  uint8_t data[0xFFFF];
  struct {
      uint8_t rom[2][0x4000];
      uint8_t vram[0x2000];
      uint8_t eram[0x2000];
      uint8_t wram[0x2000];
      uint8_t echo[0x1E00];
      uint8_t oam[0xA0];
      uint8_t undef[0x60];
      uint8_t io[0x80];
      uint8_t hram[0x7F];
      uint8_t ie;
  } map;
} MMU;

void mmuInit(MMU*, Cartridge*);
uint8_t mmuReadByte(MMU*, uint16_t);
uint16_t mmuReadWord(MMU*, uint16_t);
void mmuWriteByte(MMU*, uint16_t, uint8_t);
void mmuWriteWord(MMU*, uint16_t, uint16_t);

#endif
