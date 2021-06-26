#include "cpu.h"
#include "opcode.h"

void cpuInit(CPU *cpu, MMU *mmu)
{
	cpu->mmu = mmu;

	/**
	 * Initial CPU state
	 * http://bgb.bircd.org/pandocs.htm#powerupsequence
	 */
	cpu->regs.a = 0x1; cpu->regs.f = 0xB0;
	cpu->regs.b = 0x0; cpu->regs.c = 0x13;
	cpu->regs.d = 0x0; cpu->regs.e = 0xD8;
	cpu->regs.h = 0x1; cpu->regs.l = 0x4D;
	cpu->regs.sp = 0xFFFE;
	cpu->regs.pc = 0x100;

	mmuWriteByte(mmu, 0xFF40, 0x91); // 0xFF40 = LCDC
}

unsigned cpuStep(CPU *cpu)
{
	unsigned cycles = 0;
	uint8_t intrEnabled = mmuReadByte(cpu->mmu, 0xFFFF);
	uint8_t intrFlag = mmuReadByte(cpu->mmu, 0xFF0F);
	uint8_t interrupt = (intrEnabled & intrFlag) & 0x1F;

	if (interrupt) {
		cpu->halt = false;
		cpu->stop = false;

		if (cpu->ime) {
			cycles += 16;

			cpu->ime = false;
			cpu->regs.sp -= 2;

			mmuWriteWord(cpu->mmu, cpu->regs.sp, cpu->regs.pc);

			if (interrupt & 0x1) { // Vblank
				mmuWriteByte(cpu->mmu, 0xFF0F, intrFlag & 0xFE);
				cpu->regs.pc = 0x40;
			} else if (interrupt & 0x2) { // Stat
				mmuWriteByte(cpu->mmu, 0xFF0F, intrFlag & 0xFD);
				cpu->regs.pc = 0x48;
			} else if (interrupt & 0x4) { // Timer
				mmuWriteByte(cpu->mmu, 0xFF0F, intrFlag & 0xFB);
				cpu->regs.pc = 0x50;
			} else if (interrupt & 0x8) { // Serial
				mmuWriteByte(cpu->mmu, 0xFF0F, intrFlag & 0xF7);
				cpu->regs.pc = 0x58;
			} else if (interrupt & 0x10) { // Joypad
				mmuWriteByte(cpu->mmu, 0xFF0F, intrFlag & 0xEF);
				cpu->regs.pc = 0x60;
			}
		}
	}

	if (cpu->halt || cpu->stop) {
		return 1;
	}

	/**
	 * Wait a cycle before IME on EI
	 */
	if (cpu->ei) {
		cpu->ei = false;
		cpu->ime = true;
	}

	uint8_t opcode = mmuReadByte(cpu->mmu, cpu->regs.pc);
	cpu->regs.pc++;

	if (cpu->cb) {
		cpu->cb = false;
		cycles += cpuOpcodeCB(cpu, opcode);
	} else {
		cycles += cpuOpcode(cpu, opcode);
	}

	return cycles;
}

unsigned cpuGetFlag(CPU *cpu, unsigned flag)
{
	switch (flag) {
		case FLAG_Z:
			return cpu->regs.f >> 7;

		case FLAG_N:
			return (cpu->regs.f >> 6) & 0x1;

		case FLAG_H:
			return (cpu->regs.f >> 5) & 0x1;

		case FLAG_C:
			return (cpu->regs.f >> 4) & 0x1;

		default:
			return 0;
	}
}

void cpuSetFlag(CPU *cpu, unsigned flag, unsigned value)
{
	switch (flag) {
		case FLAG_Z:
			cpu->regs.f = value ? cpu->regs.f | 0x80 : cpu->regs.f & 0x7F;
			break;

		case FLAG_N:
			cpu->regs.f = value ? cpu->regs.f | 0x40 : cpu->regs.f & 0xBF;
			break;

		case FLAG_H:
			cpu->regs.f = value ? cpu->regs.f | 0x20 : cpu->regs.f & 0xDF;
			break;

		case FLAG_C:
			cpu->regs.f = value ? cpu->regs.f | 0x10 : cpu->regs.f & 0xEF;
			break;
	}
}
