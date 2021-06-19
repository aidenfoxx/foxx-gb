#include "mmu.h"

void mmuInit(MMU *mmu, Cartridge* cartridge)
{
	mmu->cartridge = cartridge;
}

uint8_t mmuReadByte(MMU *mmu, uint16_t address)
{
	switch (address & 0xF000) {
		case 0x0000:
			return mmu->cartridge->rom0[address];

		case 0x1000:
		case 0x2000:
		case 0x3000:
			return mmu->cartridge->rom0[address];

		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			return mmu->cartridge->rom1[address & 0x3FFF];

		case 0x8000:
		case 0x9000:
			return mmu->memory.vram[address & 0x1FFF];

		case 0xA000:
		case 0xB000:
			/* TODO: Implement external RAM */

		case 0xC000:
		case 0xD000:
		case 0xE000:
			return mmu->memory.wram[address & 0x1FFF];

		case 0xF000:
			switch (address & 0xF00) {
				case 0x0000: case 0x0100: case 0x0200: case 0x0300:
				case 0x0400: case 0x0500: case 0x0600: case 0x0700:
				case 0x0800: case 0x0900: case 0x0A00: case 0x0B00:
				case 0x0C00: case 0x0D00:
					return mmu->memory.wram[address & 0x1DFF];

				case 0x0E00:
					if (address < 0xFEA0) {
						return mmu->memory.oam[address & 0xFF];
					}

				case 0x0F00:
					if (address < 0xFF80) {
						return mmu->memory.io[address & 0x7F];
					}
					if (address < 0xFFFF) {
						return mmu->memory.hram[address & 0x7F];
					}
					return mmu->memory.ie;

				default:
					printf("WARNING: Could not read address: `%x`\n", address);
					return 0x00;
			}

		default:
			printf("WARNING: Could not read address: `%x`\n", address);
		 	return 0x00;
	}
}

uint16_t mmuReadWord(MMU *mmu, uint16_t address)
{
    return (mmuReadByte(mmu, address + 0x01) << 8) + mmuReadByte(mmu, address);
}

void mmuWriteByte(MMU *mmu, uint16_t address, uint8_t data)
{
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
			mmu->memory.vram[address & 0x1FFF] = data;
			return;

		case 0xA000:
		case 0xB000:
			/* TODO: Implement external ram? */

		case 0xC000:
		case 0xD000:
			mmu->memory.wram[address & 0x1FFF] = data;
			return;

		case 0xF000:
			switch (address & 0x0F00) {
				case 0x0E00:
					if (address < 0xFEA0) {
						mmu->memory.oam[address & 0xFF] = data;
						return;
					}

				case 0x0F00:
					if (address < 0xFF80) {
						mmu->memory.io[address & 0x7F] = data;
						return;
					}
					if (address < 0xFFFF) {
						mmu->memory.hram[address & 0x7F] = data;
						return;
					}
					mmu->memory.ie = data;
					return;

				default:
					printf("WARNING: Could not write to address: `%x`\n", address);
					return;
			}

		default:
			printf("WARNING: Could not write to address: `%x`\n", address);
			return;
	}
}

void mmuWriteWord(MMU *mmu, uint16_t address, uint16_t data)
{
    mmuWriteByte(mmu, address, data & 0xFF);
    mmuWriteByte(mmu, address + 0x01, data >> 8);
}
