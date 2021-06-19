#include "mmu.h"

const static uint8_t bios[256] = {
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

void mmuInit(MMU *mmu, Cartridge* cartridge)
{
	mmu->bios = 1;
	mmu->cartridge = cartridge;
}

uint8_t mmuReadByte(MMU *mmu, uint16_t address)
{
	switch (address & 0xF000) {
		case 0x0000:
			if(address < 0x0100 && mmu->bios) {
				return bios[address];
			}
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
