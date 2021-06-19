#include "cpu.h"
#include "opcode.h"

void cpuInit(CPU *cpu)
{
	cpu->regs.a = 0x1;
	cpu->regs.f = 0xB0;
	cpu->regs.b = 0x0;
	cpu->regs.c = 0x13;
	cpu->regs.d = 0x0;
	cpu->regs.e = 0xD8;
	cpu->regs.h = 0x1;
	cpu->regs.l = 0x4D;
	cpu->regs.sp = 0xFFFe;
	cpu->regs.pc = 0x100;
}

int cpuGetFlag(CPU *cpu, int flag)
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

void cpuSetFlag(CPU *cpu, int flag, int value)
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

void cpuStep(CPU *cpu, MMU *mmu)
{
	uint8_t cycles = 0;
	uint8_t intEnabled = mmuReadByte(mmu, 0xFFFF);
	uint8_t intFlag = mmuReadByte(mmu, 0xFF0F);

	if (cpu->ime && intEnabled && intFlag) {
		uint8_t interrupt = intEnabled & intFlag;

		if (interrupt) {
			cpu->ime = false;
			cpu->halt = false;

			if (interrupt & 0x01) { /* Vblank */
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFE);
				cpu->regs.sp -= 2;
				mmuWriteWord(mmu, cpu->regs.sp, cpu->regs.pc);
				cpu->regs.pc = 0x40;
				cycles += 16;
			}

			if (interrupt & 0x02) { /* Stat */
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFD);
				cpu->regs.sp -= 2;
				mmuWriteWord(mmu, cpu->regs.sp, cpu->regs.pc);
				cpu->regs.pc = 0x48;
				cycles += 16;
			}

			if (interrupt & 0x04) { /* Timer */
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFB);
				cpu->regs.sp -= 2;
				mmuWriteWord(mmu, cpu->regs.sp, cpu->regs.pc);
				cpu->regs.pc = 0x50;
				cycles += 16;
			}

			if (interrupt & 0x08) { /* Serial */
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xF7);
				cpu->regs.sp -= 2;
				mmuWriteWord(mmu, cpu->regs.sp, cpu->regs.pc);
				cpu->regs.pc = 0x58;
				cycles += 16;
			}

			if (interrupt & 0x10) { /* Joypad */
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xEF);
				cpu->regs.sp -= 2;
				mmuWriteWord(mmu, cpu->regs.sp, cpu->regs.pc);
				cpu->regs.pc = 0x60;
				cpu->stop = false;
				cycles += 16;
			}
		}
	}

	/**
	 * Wait a cycle before IME on EI
	 */
	if (cpu->ei) {
		cpu->ei = false;
		cpu->ime = true;
	}

	uint8_t opcode = mmuReadByte(mmu, cpu->regs.pc);
	cpu->regs.pc++;

	if (!cpu->halt && !cpu->stop) {
		if (cpu->cb) {
			cycles += cpuOpcodeCB(cpu, mmu, opcode);
			cpu->cb = false;
		} else {
			cycles += cpuOpcode(cpu, mmu, opcode);
		}
	}

	cpu->cycles = cycles;
}
