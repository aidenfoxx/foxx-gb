#include "mmu.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void mmuInit(MMU *mmu, uint8_t *romData, size_t romSize)
{
	mmu->romBank = 1;

	memcpy(mmu->system.data, romData, 0x8000);
	memcpy(mmu->romBanks, romData, romSize);
}

uint8_t mmuReadByte(MMU *mmu, uint16_t address)
{
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			return mmu->romBanks[mmu->romBank][address - 0x4000];

		case 0xA000:
		case 0xB000:
			return mmu->romBanks[mmu->ramBank][address - 0xA000];
	}

	return mmu->system.data[address];
}

uint16_t mmuReadWord(MMU *mmu, uint16_t address)
{
    return (mmuReadByte(mmu, address + 1) << 8) + mmuReadByte(mmu, address);
}

void mmuWriteByte(MMU *mmu, uint16_t address, uint8_t data)
{
	switch (address & 0xF000) {
		case 0:
		case 0x1000:
			return;

		case 0x2000:
		case 0x3000:
			mmu->romBank = (mmu->romBank & 0x60) + MAX(data & 0x1F, 1); // TODO: Is this right?
			return;

		case 0x4000:
		case 0x5000:
			if (mmu->extendedRam) {
				mmu->ramBank = data & 0x3;
				return;
			}
			mmu->romBank = (mmu->romBank & 0x1F) + ((data & 0x3) << 5);
			return;

		case 0x6000:
		case 0x7000:
			mmu->extendedRam = data & 0x1;
			return;

		case 0xA000:
		case 0xB000:
			mmu->ramBanks[mmu->ramBank][address - 0xA000] = data;
			return;
	}

	mmu->system.data[address] = data;
}

void mmuWriteWord(MMU *mmu, uint16_t address, uint16_t data)
{
    mmuWriteByte(mmu, address, data);
    mmuWriteByte(mmu, address + 1, data >> 8);
}

void mmuTransferDMA(MMU *mmu)
{
	uint16_t addr = mmuReadByte(mmu, 0xFF46) * 0x100;
	memcpy(mmu->system.data + 0xFE00, mmu->system.data + addr, 0xA0);
}
