#include "mmu.h"

void mmuInit(MMU *mmu, Cartridge* cartridge)
{
	memcpy(mmu->data, cartridge->rom, 0x8000);
}

uint8_t mmuReadByte(MMU *mmu, uint16_t address)
{
	return mmu->data[address];
}

uint16_t mmuReadWord(MMU *mmu, uint16_t address)
{
    return (mmuReadByte(mmu, address + 1) << 8) + mmuReadByte(mmu, address);
}

void mmuWriteByte(MMU *mmu, uint16_t address, uint8_t data)
{
	mmu->data[address] = data;
}

void mmuWriteWord(MMU *mmu, uint16_t address, uint16_t data)
{
    mmuWriteByte(mmu, address, data);
    mmuWriteByte(mmu, address + 1, data >> 8);
}
