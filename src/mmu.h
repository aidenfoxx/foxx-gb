#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  union {
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
  } system;
  bool extendedRam;
  uint8_t romBank;
  uint8_t ramBank;
  uint8_t romBanks[128][0x4000];
  uint8_t ramBanks[16][0x2000];
} MMU;

void mmuInit(MMU*, uint8_t*, size_t);
uint8_t mmuReadByte(MMU*, uint16_t);
uint16_t mmuReadWord(MMU*, uint16_t);
void mmuWriteByte(MMU*, uint16_t, uint8_t);
void mmuWriteWord(MMU*, uint16_t, uint16_t);
void mmuTransferDMA(MMU*);

#endif
