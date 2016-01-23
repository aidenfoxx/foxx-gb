#include "cpu.h"
#include "opcode.h"

void cpuInit(CPU *cpu)
{
	Registers *registers = malloc(sizeof(Registers));

	registers->a = 0x00; registers->b = 0x00; registers->c = 0x00; registers->d = 0x00;
	registers->e = 0x00; registers->f = 0x00; registers->h = 0x00; registers->l = 0x00;
	registers->sp = 0x0000; registers->pc = 0x0000;

	cpu->r = registers;

	cpu->cycles = 0x00;

	cpu->cb = 0;
	cpu->ei = 0; cpu->ime = 0;
	cpu->halt = 0; cpu->stop = 0;
}

void cpuFree(CPU *cpu)
{
	free(cpu->r);
}

int cpuGetFlag(CPU *cpu, int flag)
{
	switch (flag)
	{
		case FLAG_Z:
			return cpu->r->f >> 7;

		case FLAG_N:
			return (cpu->r->f >> 6) & 0x01;

		case FLAG_H:
			return (cpu->r->f >> 5) & 0x01;

		case FLAG_C:
			return (cpu->r->f >> 4) & 0x01;

		default:
			return 0;
	}
}

void cpuSetFlag(CPU *cpu, int flag, int value)
{
	switch (flag)
	{
		case FLAG_Z:
			cpu->r->f = (value ? cpu->r->f | 0x80 : cpu->r->f & 0x7F);
			break;

		case FLAG_N:
			cpu->r->f = (value ? cpu->r->f | 0x40 : cpu->r->f & 0xBF);
			break;

		case FLAG_H:
			cpu->r->f = (value ? cpu->r->f | 0x20 : cpu->r->f & 0xDF);
			break;

		case FLAG_C:
			cpu->r->f = (value ? cpu->r->f | 0x10 : cpu->r->f & 0xEF);
			break;
	}
}

void cpuStep(CPU *cpu, MMU *mmu)
{
	uint8_t opcode;
	uint8_t cycles = 0x00;
	uint8_t intEnabled = mmuReadByte(mmu, 0xFFFF);
	uint8_t intFlag = mmuReadByte(mmu, 0xFF0F);

	if (cpu->r->pc == 0x100)
	{
		mmu->bios = 0;
	}

	if (cpu->ime && intEnabled && intFlag)
	{
		uint8_t interrupt = intEnabled & intFlag;

		cpu->halt = 0;
		cpu->stop = 0;

		if (interrupt)
		{
			cpu->ime = 0;

			if (interrupt & 0x01) /* Vblank */
			{
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFE); cpu->r->sp -= 2; mmuWriteWord(mmu, cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x40;
				cycles += 0x10;
			}

			if (interrupt & 0x02) /* Stat */
			{
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFD); cpu->r->sp -= 2; mmuWriteWord(mmu, cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x48;
				cycles += 0x10;
			}

			if (interrupt & 0x04) /* Timer */
			{
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xFB); cpu->r->sp -= 2; mmuWriteWord(mmu, cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x50;
				cycles += 0x10;
			}

			if (interrupt & 0x08) /* Serial */
			{
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xF7); cpu->r->sp -= 2; mmuWriteWord(mmu, cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x58;
				cycles += 0x10;
			}

			if (interrupt & 0x10) /* Joypad */
			{
				mmuWriteByte(mmu, 0xFF0F, intFlag & 0xEF); cpu->r->sp -= 2; mmuWriteWord(mmu, cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x60;
				cycles += 0x10;
			}
		}
	}

	/**
	 * Wait a cycle before IME on EI
	 */
	if (cpu->ei)
	{
		cpu->ei = 0;
		cpu->ime = 1;
	}

	opcode = mmuReadByte(mmu, cpu->r->pc);
	cpu->r->pc++;

	if (!cpu->halt && !cpu->stop)
	{
		if (cpu->cb == 1)
		{
			cpu->cb = 0;
			cycles += cpuOpcodeCB(cpu, mmu, opcode);
		}
		else
		{
			cycles += cpuOpcode(cpu, mmu, opcode);
		}	
	}

	cpu->cycles = cycles;
}