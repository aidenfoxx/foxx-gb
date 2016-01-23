#include "opcode.h"

/**
 * Arithmetic Operations
 */
inline static uint8_t ADD_BB(CPU *cpu, uint8_t a, uint8_t b) { cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_C, (0xFF - a) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (0x0F - (a & 0x0F)) < (b & 0x0F) ? 1 : 0); a += b; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); return a; }
inline static uint16_t ADD_BW(CPU *cpu, uint16_t a, uint8_t b) { cpuSetFlag(cpu, FLAG_Z, 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_C, (0xFF - (a & 0xFF)) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (0x0F - (a & 0x0F)) < (b & 0x0F) ? 1 : 0); return a + b; }
inline static uint16_t ADD_WW(CPU *cpu, uint16_t a, uint16_t b) { cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_C, (0xFFFF - a) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (0x0FFF - (a & 0x0FFF)) < (b & 0x0FFF) ? 1 : 0); return a + b; }
inline static uint8_t ADC(CPU *cpu, uint8_t a, uint8_t b) { uint8_t cf = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_C, (0xFF - a) < cf || (0xFF - (a + cf)) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (0x0F - (a & 0x0F)) < (cf & 0x0F) || (0x0F - ((a + cf) & 0x0F)) < (b & 0x0F) ? 1 : 0); a += b + cf; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); return a; }
inline static uint8_t SUB(CPU *cpu, uint8_t a, uint8_t b) { cpuSetFlag(cpu, FLAG_N, 1); cpuSetFlag(cpu, FLAG_C, a < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (a & 0x0F) < (b & 0x0F) ? 1 : 0); a -= b; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); return a; }
inline static uint8_t SBC(CPU *cpu, uint8_t a, uint8_t b) { uint8_t cf = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_N, 1); cpuSetFlag(cpu, FLAG_C, a < cf || (a - cf) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (a & 0x0F) < (cf & 0x0F) || ((a + cf) & 0x0F) < (b & 0x0F) ? 1 : 0); a -= b - cf; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); return a; }
inline static uint8_t INC_B(CPU *cpu, uint8_t a) { a++; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, !(a & 0x0F) ? 1 : 0); return a; }
inline static uint16_t INC_W(uint16_t a) { return ++a; }
inline static uint8_t DEC_B(CPU *cpu, uint8_t a) { a--; cpuSetFlag(cpu, FLAG_Z, !(a & 0xFF) ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 1); cpuSetFlag(cpu, FLAG_H, (a & 0x0F) == 0x0F ? 1 : 0); return a; }
inline static uint16_t DEC_W(uint16_t a) { return --a; }

/**
 * Binary Operations
 */
inline static uint8_t AND(CPU *cpu, uint8_t a, uint8_t b) { a &= b; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 1); cpuSetFlag(cpu, FLAG_C, 0); return a; }
inline static uint8_t OR(CPU *cpu, uint8_t a, uint8_t b) { a |= b; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); cpuSetFlag(cpu, FLAG_C, 0); return a; }
inline static uint8_t XOR(CPU *cpu, uint8_t a, uint8_t b) { a ^= b; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); cpuSetFlag(cpu, FLAG_C, 0); return a; }

/**
 * Stack Operations
 */
inline static void PUSH(MMU *mmu, uint16_t *sp, uint16_t a) { *sp -= 0x02; mmuWriteWord(mmu, *sp, a); }
inline static uint16_t POP(MMU *mmu, uint16_t *sp) { uint16_t a = mmuReadWord(mmu, *sp); *sp += 0x02; return a; }

/**
 * Other Operations
 */
inline static uint8_t SWAP(CPU *cpu, uint8_t a) { a = (a >> 4) | ((a & 0x0F) << 4); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); cpuSetFlag(cpu, FLAG_Z, 0); return a; }
inline static uint8_t DAA(CPU *cpu, uint8_t a) { uint8_t cf = cpuGetFlag(cpu, FLAG_C) || a > 0x99 ? 0x60 : 0x00; cf += cpuGetFlag(cpu, FLAG_H) || (a & 0x0f) > 0x09 ? 0x06 : 0x00; int temp = cpuGetFlag(cpu, FLAG_Z) ? a - cf : a + cf; if (temp & 0x100) { cpuSetFlag(cpu, FLAG_C, 1); } temp &= 0xFF; cpuSetFlag(cpu, FLAG_Z, !temp ? 1 : 0); cpuSetFlag(cpu, FLAG_H, 0); return temp; }
inline static uint8_t RLC(CPU *cpu, uint8_t a) { uint8_t r = (a & 0x80) >> 7; cpuSetFlag(cpu, FLAG_C, r); a = (a << 1) + r; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RL(CPU *cpu, uint8_t a) { uint8_t c = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7); a = (a << 1) + c; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RRC(CPU *cpu, uint8_t a) { uint8_t r = a & 0x01; cpuSetFlag(cpu, FLAG_C, r); a = (a >> 1) + (r << 7); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RR(CPU *cpu, uint8_t a) { uint8_t c = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_C, a & 0x01); a = (a >> 1) + (c << 7); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SLA(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7); a <<= 1; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SRA(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, a & 0x01); a >>= 1; a |= ((a & 0x40) << 1); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SRL(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, a & 0x01); a >>= 1; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static void BIT(CPU *cpu, uint8_t a, uint8_t b) { cpuSetFlag(cpu, FLAG_Z, !(a & (0x01 << b)) ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 1); }
inline static uint8_t SET(uint8_t a, uint8_t b) { return a | (0x01 << b); }
inline static uint8_t RES(uint8_t a, uint8_t b) { return a & ~(0x01 << b); }
inline static uint16_t JR(uint16_t pc, uint8_t a) { if ((a & 0x80) == 0x80) { a = ~(a - 0x01); return pc - a; } return pc + a; }

uint8_t cpuOpcode(CPU *cpu, MMU *mmu, uint8_t opcode)
{
	uint8_t byte;
	uint16_t word;

	switch (opcode)
	{
		case 0x00: /* NOP */
			return 0x04;

		case 0x01: /* LD BC,d16 */
			word = mmuReadWord(mmu, cpu->r->pc); cpu->r->b = word >> 8; cpu->r->c = word & 0xFF; cpu->r->pc += 0x02;
			return 0x0C;

		case 0x02: /* LD (BC),A */
			mmuWriteByte(mmu, (cpu->r->b << 8) + cpu->r->c, cpu->r->a);
			return 0x08;

		case 0x03: /* INC BC */
			word = INC_W((cpu->r->b << 8) + cpu->r->c); cpu->r->b = word >> 8; cpu->r->c = word & 0xFF;
			return 0x08;

		case 0x04: /* INC B */
			cpu->r->b = INC_B(cpu, cpu->r->b);
			return 0x04;

		case 0x05: /* DEC B */
			cpu->r->b = DEC_B(cpu, cpu->r->b);
			return 0x04;

		case 0x06: /* LD B,d8 */
			cpu->r->b = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x07: /* RLCA */
			cpu->r->a = RLC(cpu, cpu->r->a); cpuSetFlag(cpu, FLAG_Z, 0);
			return 0x04;

		case 0x08: /* LD (a16),SP */
			mmuWriteWord(mmu, mmuReadWord(mmu, cpu->r->pc), cpu->r->sp); cpu->r->pc += 0x02;
			return 0x14;

		case 0x09: /* ADD HL,BC */
			word = ADD_WW(cpu, (cpu->r->h << 8) + cpu->r->l, (cpu->r->b << 8) + cpu->r->c); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x0A: /* LD A,(BC) */
			cpu->r->a = mmuReadByte(mmu, (cpu->r->b << 8) + cpu->r->c);
			return 0x08;

		case 0x0B: /* DEC BC */
			word = DEC_W((cpu->r->b << 8) + cpu->r->c); cpu->r->b = word >> 8; cpu->r->c = word & 0xFF;
			return 0x08;

		case 0x0C: /* INC C */
			cpu->r->c = INC_B(cpu, cpu->r->c);
			return 0x04;

		case 0x0D: /* DEC C */
			cpu->r->c = DEC_B(cpu, cpu->r->c);
			return 0x04;

		case 0x0E: /* LD C,d8 */
			cpu->r->c = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x0F: /* RRCA */
			cpu->r->a = RRC(cpu, cpu->r->a); cpuSetFlag(cpu, FLAG_Z, 0);
			return 0x04;

		case 0x10: /* STOP 0 */
			cpu->stop = 1;
			return 0x04;

		case 0x11: /* LD DE,d16 */
			word = mmuReadWord(mmu, cpu->r->pc); cpu->r->d = word >> 8; cpu->r->e = word & 0xFF; cpu->r->pc += 0x02;
			return 0x0C;

		case 0x12: /* LD (DE),A */
			mmuWriteByte(mmu, (cpu->r->d << 8) + cpu->r->e, cpu->r->a);
			return 0x08;

		case 0x13: /* INC DE */
			word = INC_W((cpu->r->d << 8) + cpu->r->e); cpu->r->d = word >> 8; cpu->r->e = word & 0xFF;
			return 0x08;

		case 0x14: /* INC D */
			cpu->r->d = INC_B(cpu, cpu->r->d);
			return 0x04;

		case 0x15: /* DEC D */
			cpu->r->d = DEC_B(cpu, cpu->r->d);
			return 0x04;

		case 0x16: /* LD D,d8 */
			cpu->r->d = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x17: /* RLA */
			cpu->r->a = RL(cpu, cpu->r->a);
			return 0x04;

		case 0x18: /* JR r8 */
			cpu->r->pc = JR(cpu->r->pc, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x0C;

		case 0x19: /* ADD HL,DE */
			word = ADD_WW(cpu, (cpu->r->h << 8) + cpu->r->l, (cpu->r->d << 8) + cpu->r->e); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x1A: /* LD A,(DE) */
			cpu->r->a = mmuReadByte(mmu, (cpu->r->d << 8) + cpu->r->e);
			return 0x08;

		case 0x1B: /* DEC DE */
			word = DEC_W((cpu->r->d << 8) + cpu->r->e); cpu->r->d = word >> 8; cpu->r->e = word & 0xFF;
			return 0x08;

		case 0x1C: /* INC E */
			cpu->r->e = INC_B(cpu, cpu->r->e);
			return 0x04;

		case 0x1D: /* DEC E */
			cpu->r->e = DEC_B(cpu, cpu->r->e);
			return 0x04;

		case 0x1E: /* LD E,d8 */
			cpu->r->e = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x1F: /* RRA */
			cpu->r->a = RR(cpu, cpu->r->a);
			return 0x04;

		case 0x20: /* JR NZ,r8 */
			if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = JR(cpu->r->pc, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++; return 0x0C; } cpu->r->pc++;
			return 0x08;

		case 0x21: /* LD HL,d16 */
			word = mmuReadWord(mmu, cpu->r->pc); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF; cpu->r->pc += 0x02;
			return 0x0C;

		case 0x22: /* LD (HL+),A */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, cpu->r->a); word = INC_W(word); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x23: /* INC HL */
			word = INC_W((cpu->r->h << 8) + cpu->r->l); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x24: /* INC H */
			cpu->r->h = INC_B(cpu, cpu->r->h);
			return 0x04;

		case 0x25: /* DEC H */
			cpu->r->h = DEC_B(cpu, cpu->r->h);
			return 0x04;

		case 0x26: /* LD H,d8 */
			cpu->r->h = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x27: /* DAA */
			cpu->r->a = DAA(cpu, cpu->r->a);
			return 0x04;

		case 0x28: /* JR Z,r8 */
			if (cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = JR(cpu->r->pc, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++; return 0x0C; } cpu->r->pc++;
			return 0x08;

		case 0x29: /* ADD HL,HL */
			word = ADD_WW(cpu, (cpu->r->h << 8) + cpu->r->l, (cpu->r->h << 8) + cpu->r->l); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x2A: /* LD A,(HL+) */
			word = (cpu->r->h << 8) + cpu->r->l; cpu->r->a = mmuReadByte(mmu, word); word = INC_W(word); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x2B: /* DEC HL */
			word = DEC_W((cpu->r->h << 8) + cpu->r->l); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x2C: /* INC L */
			cpu->r->l = INC_B(cpu, cpu->r->l);
			return 0x04;

		case 0x2D: /* DEC L */
			cpu->r->l = DEC_B(cpu, cpu->r->l);
			return 0x04;

		case 0x2E: /* LD L,d8 */
			cpu->r->l = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x2F: /* CPL */
			cpu->r->a = ~cpu->r->a; cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0);
			return 0x04;

		case 0x30: /* JR NC,r8 */
			if (!cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = JR(cpu->r->pc, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++; return 0x0C; } cpu->r->pc++;
			return 0x08;

		case 0x31: /* LD SP,d16 */
			cpu->r->sp = mmuReadWord(mmu, cpu->r->pc); cpu->r->pc += 0x02;
			return 0x0C;

		case 0x32: /* LD (HL-),A */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, cpu->r->a); word = DEC_W(word); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x33: /* INC SP */
			cpu->r->sp = INC_W(cpu->r->sp);
			return 0x08;

		case 0x34: /* INC (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; byte = INC_B(cpu, mmuReadByte(mmu, word)); mmuWriteByte(mmu, word, byte);
			return 0x0C;

		case 0x35: /* DEC (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; byte = DEC_B(cpu, mmuReadByte(mmu, word)); mmuWriteByte(mmu, word, byte);
			return 0x0C;

		case 0x36: /* LD (HL),d8 */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x0C;

		case 0x37: /* SCF */
			cpuSetFlag(cpu, FLAG_C, 1); cpuSetFlag(cpu, FLAG_N, 1); cpuSetFlag(cpu, FLAG_H, 1);
			return 0x04;

		case 0x38: /* JR C,r8 */
			if (cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = JR(cpu->r->pc, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++; return 0x0C; } cpu->r->pc++;
			return 0x08;

		case 0x39: /* ADD HL,SP */
			word = ADD_WW(cpu, (cpu->r->h << 8) + cpu->r->l, cpu->r->sp); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x3A: /* LD A,(HL-) */
			word = (cpu->r->h << 8) + cpu->r->l; cpu->r->a = mmuReadByte(mmu, word); word = INC_W(word); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x08;

		case 0x3B: /* DEC SP */
			cpu->r->sp = DEC_W(cpu->r->sp);
			return 0x08;

		case 0x3C: /* INC A */
			cpu->r->a = INC_B(cpu, cpu->r->a);
			return 0x04;

		case 0x3D: /* DEC A */
			cpu->r->a = DEC_B(cpu, cpu->r->a);
			return 0x04;

		case 0x3E: /* LD A,d8 */
			cpu->r->a = mmuReadByte(mmu, cpu->r->pc); cpu->r->pc++;
			return 0x08;

		case 0x3F: /* CCF */
			cpuSetFlag(cpu, FLAG_C, cpuGetFlag(cpu, FLAG_C) ? 0 : 1); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0);
			return 0x04;

		case 0x40: /* LD B,B */
			cpu->r->b = cpu->r->b;
			return 0x04;

		case 0x41: /* LD B,C */
			cpu->r->b = cpu->r->c;
			return 0x04;

		case 0x42: /* LD B,D */
			cpu->r->b = cpu->r->d;
			return 0x04;

		case 0x43: /* LD B,E */
			cpu->r->b = cpu->r->e;
			return 0x04;

		case 0x44: /* LD B,H */
			cpu->r->b = cpu->r->h;
			return 0x04;

		case 0x45: /* LD B,L */
			cpu->r->b = cpu->r->l;
			return 0x04;

		case 0x46: /* LD B,(HL) */
			cpu->r->b = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x47: /* LD B,A */
			cpu->r->b = cpu->r->a;
			return 0x04;

		case 0x48: /* LD C,B */
			cpu->r->c = cpu->r->b;
			return 0x04;

		case 0x49: /* LD C,C */
			cpu->r->c = cpu->r->c;
			return 0x04;

		case 0x4A: /* LD C,D */
			cpu->r->c = cpu->r->d;
			return 0x04;

		case 0x4B: /* LD C,E */
			cpu->r->c = cpu->r->e;
			return 0x04;

		case 0x4C: /* LD C,H */
			cpu->r->c = cpu->r->h;
			return 0x04;

		case 0x4D: /* LD C,L */
			cpu->r->c = cpu->r->l;
			return 0x04;

		case 0x4E: /* LD C,(HL) */
			cpu->r->c = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x4F: /* LD C,A */
			cpu->r->c = cpu->r->a;
			return 0x04;

		case 0x50: /* LD D,B */
			cpu->r->d = cpu->r->b;
			return 0x04;

		case 0x51: /* LD D,C */
			cpu->r->d = cpu->r->c;
			return 0x04;

		case 0x52: /* LD D,D */
			cpu->r->d = cpu->r->d;
			return 0x04;

		case 0x53: /* LD D,E */
			cpu->r->d = cpu->r->e;
			return 0x04;

		case 0x54: /* LD D,H */
			cpu->r->d = cpu->r->h;
			return 0x04;

		case 0x55: /* LD D,L */
			cpu->r->d = cpu->r->l;
			return 0x04;

		case 0x56: /* LD D,(HL) */
			cpu->r->d = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x57: /* LD D,A */
			cpu->r->d = cpu->r->a;
			return 0x04;

		case 0x58: /* LD E,B */
			cpu->r->e = cpu->r->b;
			return 0x04;

		case 0x59: /* LD E,C */
			cpu->r->e = cpu->r->c;
			return 0x04;

		case 0x5A: /* LD E,D */
			cpu->r->e = cpu->r->d;
			return 0x04;

		case 0x5B: /* LD E,E */
			cpu->r->e = cpu->r->e;
			return 0x04;

		case 0x5C: /* LD E,H */
			cpu->r->e = cpu->r->h;
			return 0x04;

		case 0x5D: /* LD E,L */
			cpu->r->e = cpu->r->l;
			return 0x04;

		case 0x5E: /* LD E,(HL) */
			cpu->r->e = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x5F: /* LD E,A */
			cpu->r->e = cpu->r->a;
			return 0x04;

		case 0x60: /* LD H,B */
			cpu->r->h = cpu->r->b;
			return 0x04;

		case 0x61: /* LD H,C */
			cpu->r->h = cpu->r->c;
			return 0x04;

		case 0x62: /* LD H,D */
			cpu->r->h = cpu->r->d;
			return 0x04;

		case 0x63: /* LD H,E */
			cpu->r->h = cpu->r->e;
			return 0x04;

		case 0x64: /* LD H,H */
			cpu->r->h = cpu->r->h;
			return 0x04;

		case 0x65: /* LD H,L */
			cpu->r->h = cpu->r->l;
			return 0x04;

		case 0x66: /* LD H,(HL) */
			cpu->r->h = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x67: /* LD H,A */
			cpu->r->h = cpu->r->a;
			return 0x04;

		case 0x68: /* LD L,B */
			cpu->r->l = cpu->r->b;
			return 0x04;

		case 0x69: /* LD L,C */
			cpu->r->l = cpu->r->c;
			return 0x04;

		case 0x6A: /* LD L,D */
			cpu->r->l = cpu->r->d;
			return 0x04;

		case 0x6B: /* LD L,E */
			cpu->r->l = cpu->r->e;
			return 0x04;

		case 0x6C: /* LD L,H */
			cpu->r->l = cpu->r->h;
			return 0x04;

		case 0x6D: /* LD L,L */
			cpu->r->l = cpu->r->l;
			return 0x04;

		case 0x6E: /* LD L,(HL) */
			cpu->r->l = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x6F: /* LD L,A */
			cpu->r->l = cpu->r->a;
			return 0x04;

		case 0x70: /* LD (HL),B */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->b);
			return 0x08;

		case 0x71: /* LD (HL),C */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->c);
			return 0x08;

		case 0x72: /* LD (HL),D */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->d);
			return 0x08;

		case 0x73: /* LD (HL),E */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->e);
			return 0x08;

		case 0x74: /* LD (HL),H */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->h);
			return 0x08;

		case 0x75: /* LD (HL),L */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->l);
			return 0x08;

		case 0x76: /* HALT */
			cpu->halt = 1;
			return 0x04;

		case 0x77: /* LD (HL),A */
			mmuWriteByte(mmu, (cpu->r->h << 8) + cpu->r->l, cpu->r->a);
			return 0x08;

		case 0x78: /* LD A,B */
			cpu->r->a = cpu->r->b;
			return 0x04;

		case 0x79: /* LD A,C */
			cpu->r->a = cpu->r->c;
			return 0x04;

		case 0x7A: /* LD A,D */
			cpu->r->a = cpu->r->d;
			return 0x04;

		case 0x7B: /* LD A,E */
			cpu->r->a = cpu->r->e;
			return 0x04;

		case 0x7C: /* LD A,H */
			cpu->r->a = cpu->r->h;
			return 0x04;

		case 0x7D: /* LD A,L */
			cpu->r->a = cpu->r->l;
			return 0x04;

		case 0x7E: /* LD A,(HL) */
			cpu->r->a = mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l);
			return 0x08;

		case 0x7F: /* LD A,A */
			cpu->r->a = cpu->r->a;
			return 0x04;

		case 0x80: /* ADD A,B */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0x81: /* ADD A,C */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0x82: /* ADD A,D */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0x83: /* ADD A,E */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0x84: /* ADD A,H */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0x85: /* ADD A,L */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0x86: /* ADD A,(HL) */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0x87: /* ADD A,A */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0x88: /* ADC A,B */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0x89: /* ADC A,C */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0x8A: /* ADC A,D */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0x8B: /* ADC A,E */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0x8C: /* ADC A,H */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0x8D: /* ADC A,L */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0x8E: /* ADC A,(HL) */
			cpu->r->a = ADC(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0x8F: /* ADC A,A */
			cpu->r->a = ADC(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0x90: /* SUB A,B */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0x91: /* SUB A,C */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0x92: /* SUB A,D */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0x93: /* SUB A,E */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0x94: /* SUB A,H */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0x95: /* SUB A,L */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0x96: /* SUB A,(HL) */
			cpu->r->a = SUB(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0x97: /* SUB A,A */
			cpu->r->a = SUB(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0x98: /* SBC A,B */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0x99: /* SBC A,C */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0x9A: /* SBC A,D */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0x9B: /* SBC A,E */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0x9C: /* SBC A,H */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0x9D: /* SBC A,L */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0x9E: /* SBC A,(HL) */
			cpu->r->a = SBC(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0x9F: /* SBC A,A */
			cpu->r->a = SBC(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0xA0: /* AND B */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0xA1: /* AND C */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0xA2: /* AND D */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0xA3: /* AND E */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0xA4: /* AND H */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0xA5: /* AND L */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0xA6: /* AND (HL) */
			cpu->r->a = AND(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0xA7: /* AND A */
			cpu->r->a = AND(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0xA8: /* XOR B */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0xA9: /* XOR C */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0xAA: /* XOR D */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0xAB: /* XOR E */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0xAC: /* XOR H */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0xAD: /* XOR L */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0xAE: /* XOR (HL) */
			cpu->r->a = XOR(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0xAF: /* XOR A */
			cpu->r->a = XOR(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0xB0: /* OR B */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0xB1: /* OR C */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0xB2: /* OR D */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0xB3: /* OR E */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0xB4: /* OR H */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0xB5: /* OR L */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0xB6: /* OR (HL) */
			cpu->r->a = OR(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0xB7: /* OR A */
			cpu->r->a = OR(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0xB8: /* CP B */
			SUB(cpu, cpu->r->a, cpu->r->b);
			return 0x04;

		case 0xB9: /* CP C */
			SUB(cpu, cpu->r->a, cpu->r->c);
			return 0x04;

		case 0xBA: /* CP D */
			SUB(cpu, cpu->r->a, cpu->r->d);
			return 0x04;

		case 0xBB: /* CP E */
			SUB(cpu, cpu->r->a, cpu->r->e);
			return 0x04;

		case 0xBC: /* CP H */
			SUB(cpu, cpu->r->a, cpu->r->h);
			return 0x04;

		case 0xBD: /* CP L */
			SUB(cpu, cpu->r->a, cpu->r->l);
			return 0x04;

		case 0xBE: /* CP (HL) */
			SUB(cpu, cpu->r->a, mmuReadByte(mmu, (cpu->r->h << 8) + cpu->r->l));
			return 0x08;

		case 0xBF: /* CP A */
			SUB(cpu, cpu->r->a, cpu->r->a);
			return 0x04;

		case 0xC0: /* RET NZ */
			if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = POP(mmu, &cpu->r->sp); return 0x14; }
			return 0x08;

		case 0xC1: /* POP BC */
			word = POP(mmu, &cpu->r->sp); cpu->r->b = word >> 8; cpu->r->c = word & 0xFF;
			return 0x0C;

		case 0xC2: /* JP NZ,a16 */
			if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x14; } cpu->r->pc += 2;
			return 0x0C;

		case 0xC3: /* JP a16 */
			cpu->r->pc = mmuReadWord(mmu, cpu->r->pc);
			return 0x10;

		case 0xC4: /* CALL NZ,a16 */
			if (!cpuGetFlag(cpu, FLAG_Z)) { PUSH(mmu, &cpu->r->sp, cpu->r->pc + 0x02); cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x18; } cpu->r->pc += 2;
			return 0x0C;

		case 0xC5: /* PUSH BC */
			PUSH(mmu, &cpu->r->sp, (cpu->r->b << 8) + cpu->r->c);
			return 0x10;

		case 0xC6: /* ADD A,d8 */
			cpu->r->a = ADD_BB(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xC7: /* RST 00H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x00;
			return 0x10;

		case 0xC8: /* RET Z */
			if (cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = POP(mmu, &cpu->r->sp); return 0x14; }
			return 0x08;

		case 0xC9: /* RET */
			cpu->r->pc = POP(mmu, &cpu->r->sp);
			return 0x10;

		case 0xCA: /* JP Z,a16 */
			if (cpuGetFlag(cpu, FLAG_Z)) { cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x10; } cpu->r->pc += 2;
			return 0x0C;

		case 0xCB: /* PREFIX CB */
			cpu->cb = 1;
			return 0x04;

		case 0xCC: /* CALL Z,a16 */
			if (cpuGetFlag(cpu, FLAG_Z)) { PUSH(mmu, &cpu->r->sp, cpu->r->pc + 0x02); cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x18; } cpu->r->pc += 2;
			return 0x0C;

		case 0xCD: /* CALL a16 */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc + 0x02); cpu->r->pc = mmuReadWord(mmu, cpu->r->pc);
			return 0x18;

		case 0xCE: /* ADC A,d8 */
			cpu->r->a = ADC(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xCF: /* RST 08H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x08;
			return 0x10;

		case 0xD0: /* RET NC */
			if (!cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = POP(mmu, &cpu->r->sp); return 0x20; }
			return 0x08;

		case 0xD1: /* POP DE */
			word = POP(mmu, &cpu->r->sp); cpu->r->d = word >> 8; cpu->r->e = word & 0xFF;
			return 0x0C;

		case 0xD2: /* JP NC,a16 */
			if (!cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x10; } cpu->r->pc += 2;
			return 0x0C;

		case 0xD4: /* CALL NC,a16 */
			if (!cpuGetFlag(cpu, FLAG_C)) { PUSH(mmu, &cpu->r->sp, cpu->r->pc + 0x02); cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x18; } cpu->r->pc += 2;
			return 0x0C;

		case 0xD5: /* PUSH DE */
			PUSH(mmu, &cpu->r->sp, (cpu->r->d << 8) + cpu->r->e);
			return 0x10;

		case 0xD6: /* SUB A,d8 */
			cpu->r->a = SUB(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xD7: /* RST 10H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x10;
			return 0x10;

		case 0xD8: /* RET C */
			if (cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = POP(mmu, &cpu->r->sp); return 0x14; }
			return 0x08;

		case 0xD9: /* RETI */
			cpu->r->pc = POP(mmu, &cpu->r->sp); cpu->ime = 1;
			return 0x10;

		case 0xDA: /* JP C,a16 */
			if (cpuGetFlag(cpu, FLAG_C)) { cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x10; } cpu->r->pc += 2;
			return 0x0C;

		case 0xDC: /* CALL C,a16 */
			if (cpuGetFlag(cpu, FLAG_C)) { PUSH(mmu, &cpu->r->sp, cpu->r->pc + 0x02); cpu->r->pc = mmuReadWord(mmu, cpu->r->pc); return 0x18; } cpu->r->pc += 2;
			return 0x0C;

		case 0xDE: /* SBC A,d8 */
			cpu->r->a = SBC(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xDF: /* RST 18H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x18;
			return 0x10;

		case 0xE0: /* LDH (a8),A */
			mmuWriteByte(mmu, 0xFF00 + mmuReadByte(mmu, cpu->r->pc), cpu->r->a); cpu->r->pc++;
			return 0x0C;

		case 0xE1: /* POP HL */
			word = POP(mmu, &cpu->r->sp); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF;
			return 0x0C;

		case 0xE2: /* LD (C),A */
			mmuWriteByte(mmu, 0xFF00 + cpu->r->c, cpu->r->a);
			return 0x08;

		case 0xE5: /* PUSH HL */
			PUSH(mmu, &cpu->r->sp, (cpu->r->h << 8) + cpu->r->l);
			return 0x10;

		case 0xE6: /* AND d8 */
			cpu->r->a = AND(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xE7: /* RST 20H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x20;
			return 0x10;

		case 0xE8: /* ADD SP,r8 */
			cpu->r->sp = ADD_BW(cpu, cpu->r->sp, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x10;

		case 0xE9: /* JP (HL) */
			cpu->r->pc = (cpu->r->h << 8) + cpu->r->l;
			return 0x04;

		case 0xEA: /* LD (a16),A */
			mmuWriteByte(mmu, mmuReadWord(mmu, cpu->r->pc), cpu->r->a); cpu->r->pc += 2;
			return 0x10;

		case 0xEE: /* XOR d8 */
			cpu->r->a = XOR(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xEF: /* RST 28H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x28;
			return 0x10;

		case 0xF0: /* LDH A,(a8) */
			cpu->r->a = mmuReadByte(mmu, 0xFF00 + mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x0C;

		case 0xF1: /* POP AF */
			word = POP(mmu, &cpu->r->sp); cpu->r->a = word >> 8; cpu->r->f = word & 0xFF;
			return 0x0C;

		case 0xF2: /* LD A,(C) */
			cpu->r->a = mmuReadByte(mmu, 0xFF00 + cpu->r->c);
			return 0x08;

		case 0xF3: /* DI */
			cpu->ime = 0;
			return 0x04;

		case 0xF5: /* PUSH AF */
			PUSH(mmu, &cpu->r->sp, (cpu->r->a << 8) + cpu->r->f);
			return 0x10;

		case 0xF6: /* OR d8 */
			cpu->r->a = OR(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xF7: /* RST 30H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x30;
			return 0x10;

		case 0xF8: /* LD HL,SP+r8 */
			word = ADD_BW(cpu, cpu->r->sp, mmuReadByte(mmu, cpu->r->pc)); cpu->r->h = word >> 8; cpu->r->l = word & 0xFF; cpu->r->pc++;
			return 0x0C;

		case 0xF9: /* LD SP,HL */
			cpu->r->sp = (cpu->r->h << 8) + cpu->r->l;
			return 0x08;

		case 0xFA: /* LD A,(a16) */
			cpu->r->a = mmuReadByte(mmu, mmuReadWord(mmu, cpu->r->pc)); cpu->r->pc += 2;
			return 0x10;

		case 0xFB: /* EI */
			cpu->ei = 1;
			return 0x04;

		case 0xFE: /* CP d8 */
			SUB(cpu, cpu->r->a, mmuReadByte(mmu, cpu->r->pc)); cpu->r->pc++;
			return 0x08;

		case 0xFF: /* RST 38H */
			PUSH(mmu, &cpu->r->sp, cpu->r->pc); cpu->r->pc = 0x38;
			return 0x10;

		default:
			printf("WARNING: Could not process operation: `%x`\n", opcode);
			return 0x00;
	}
}

uint8_t cpuOpcodeCB(CPU *cpu, MMU *mmu, uint8_t opcode)
{
	uint16_t word;

	switch (opcode)
	{
		case 0x00: /* RLC B */
			cpu->r->b = RLC(cpu, cpu->r->b);
			return 0x08;

		case 0x01: /* RLC C */
			cpu->r->c = RLC(cpu, cpu->r->c);
			return 0x08;

		case 0x02: /* RLC D */
			cpu->r->d = RLC(cpu, cpu->r->d);
			return 0x08;

		case 0x03: /* RLC E */
			cpu->r->e = RLC(cpu, cpu->r->e);
			return 0x08;

		case 0x04: /* RLC H */
			cpu->r->h = RLC(cpu, cpu->r->h);
			return 0x08;

		case 0x05: /* RLC L */
			cpu->r->l = RLC(cpu, cpu->r->l);
			return 0x08;

		case 0x06: /* RLC (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RLC(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x07: /* RLC A */
			cpu->r->a = RLC(cpu, cpu->r->a);
			return 0x08;

		case 0x08: /* RRC B */
			cpu->r->b = RRC(cpu, cpu->r->b);
			return 0x08;

		case 0x09: /* RRC C */
			cpu->r->c = RRC(cpu, cpu->r->c);
			return 0x08;

		case 0x0A: /* RRC D */
			cpu->r->d = RRC(cpu, cpu->r->d);
			return 0x08;

		case 0x0B: /* RRC E */
			cpu->r->e = RRC(cpu, cpu->r->e);
			return 0x08;

		case 0x0C: /* RRC H */
			cpu->r->h = RRC(cpu, cpu->r->h);
			return 0x08;

		case 0x0D: /* RRC L */
			cpu->r->l = RRC(cpu, cpu->r->l);
			return 0x08;

		case 0x0E: /* RRC (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RRC(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x0F: /* RRC A */
			cpu->r->a = RRC(cpu, cpu->r->a);
			return 0x08;

		case 0x10: /* RL B */
			cpu->r->b = RL(cpu, cpu->r->b);
			return 0x08;

		case 0x11: /* RL C */
			cpu->r->c = RL(cpu, cpu->r->c);
			return 0x08;

		case 0x12: /* RL D */
			cpu->r->d = RL(cpu, cpu->r->d);
			return 0x08;

		case 0x13: /* RL E */
			cpu->r->e = RL(cpu, cpu->r->e);
			return 0x08;

		case 0x14: /* RL H */
			cpu->r->h = RL(cpu, cpu->r->h);
			return 0x08;

		case 0x15: /* RL L */
			cpu->r->l = RL(cpu, cpu->r->l);
			return 0x08;

		case 0x16: /* RL (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RL(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x17: /* RL A */
			cpu->r->a = RL(cpu, cpu->r->a);
			return 0x08;

		case 0x18: /* RR B */
			cpu->r->b = RR(cpu, cpu->r->b);
			return 0x08;

		case 0x19: /* RR C */
			cpu->r->c = RR(cpu, cpu->r->c);
			return 0x08;

		case 0x1A: /* RR D */
			cpu->r->d = RR(cpu, cpu->r->d);
			return 0x08;

		case 0x1B: /* RR E */
			cpu->r->e = RR(cpu, cpu->r->e);
			return 0x08;

		case 0x1C: /* RR H */
			cpu->r->h = RR(cpu, cpu->r->h);
			return 0x08;

		case 0x1D: /* RR L */
			cpu->r->l = RR(cpu, cpu->r->l);
			return 0x08;

		case 0x1E: /* RR (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RR(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x1F: /* RR A */
			cpu->r->a = RR(cpu, cpu->r->a);
			return 0x08;

		case 0x20: /* SLA B */
			cpu->r->b = SLA(cpu, cpu->r->b);
			return 0x08;

		case 0x21: /* SLA C */
			cpu->r->c = SLA(cpu, cpu->r->c);
			return 0x08;

		case 0x22: /* SLA D */
			cpu->r->d = SLA(cpu, cpu->r->d);
			return 0x08;

		case 0x23: /* SLA E */
			cpu->r->e = SLA(cpu, cpu->r->e);
			return 0x08;

		case 0x24: /* SLA H */
			cpu->r->h = SLA(cpu, cpu->r->h);
			return 0x08;

		case 0x25: /* SLA L */
			cpu->r->l = SLA(cpu, cpu->r->l);
			return 0x08;

		case 0x26: /* SLA (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SLA(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x27: /* SLA A */
			cpu->r->a = SLA(cpu, cpu->r->a);
			return 0x08;

		case 0x28: /* SRA B */
			cpu->r->b = SRA(cpu, cpu->r->b);
			return 0x08;

		case 0x29: /* SRA C */
			cpu->r->c = SRA(cpu, cpu->r->c);
			return 0x08;

		case 0x2A: /* SRA D */
			cpu->r->d = SRA(cpu, cpu->r->d);
			return 0x08;

		case 0x2B: /* SRA E */
			cpu->r->e = SRA(cpu, cpu->r->e);
			return 0x08;

		case 0x2C: /* SRA H */
			cpu->r->h = SRA(cpu, cpu->r->h);
			return 0x08;

		case 0x2D: /* SRA L */
			cpu->r->l = SRA(cpu, cpu->r->l);
			return 0x08;

		case 0x2E: /* SRA (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SRA(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x2F: /* SRA A */
			cpu->r->a = SRA(cpu, cpu->r->a);
			return 0x08;

		case 0x30: /* SWAP B */
			cpu->r->b = SWAP(cpu, cpu->r->b);
			return 0x08;

		case 0x31: /* SWAP C */
			cpu->r->c = SWAP(cpu, cpu->r->c);
			return 0x08;

		case 0x32: /* SWAP D */
			cpu->r->d = SWAP(cpu, cpu->r->d);
			return 0x08;

		case 0x33: /* SWAP E */
			cpu->r->e = SWAP(cpu, cpu->r->e);
			return 0x08;

		case 0x34: /* SWAP H */
			cpu->r->h = SWAP(cpu, cpu->r->h);
			return 0x08;

		case 0x35: /* SWAP L */
			cpu->r->l = SWAP(cpu, cpu->r->l);
			return 0x08;

		case 0x36: /* SWAP (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SWAP(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x37: /* SWAP A */
			cpu->r->a = SWAP(cpu, cpu->r->a);
			return 0x08;

		case 0x38: /* SRL B */
			cpu->r->b = SRL(cpu, cpu->r->b);
			return 0x08;

		case 0x39: /* SRL C */
			cpu->r->c = SRL(cpu, cpu->r->c);
			return 0x08;

		case 0x3A: /* SRL D */
			cpu->r->d = SRL(cpu, cpu->r->d);
			return 0x08;

		case 0x3B: /* SRL E */
			cpu->r->e = SRL(cpu, cpu->r->e);
			return 0x08;

		case 0x3C: /* SRL H */
			cpu->r->h = SRL(cpu, cpu->r->h);
			return 0x08;

		case 0x3D: /* SRL L */
			cpu->r->l = SRL(cpu, cpu->r->l);
			return 0x08;

		case 0x3E: /* SRL (HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SRL(cpu, mmuReadByte(mmu, word)));
			return 0x10;

		case 0x3F: /* SRL A */
			cpu->r->a = SRL(cpu, cpu->r->a);
			return 0x08;

		case 0x40: /* BIT 0,B */
			BIT(cpu, cpu->r->b, 0);
			return 0x08;

		case 0x41: /* BIT 0,C */
			BIT(cpu, cpu->r->c, 0);
			return 0x08;

		case 0x42: /* BIT 0,D */
			BIT(cpu, cpu->r->d, 0);
			return 0x08;

		case 0x43: /* BIT 0,E */
			BIT(cpu, cpu->r->e, 0);
			return 0x08;

		case 0x44: /* BIT 0,H */
			BIT(cpu, cpu->r->h, 0);
			return 0x08;

		case 0x45: /* BIT 0,L */
			BIT(cpu, cpu->r->l, 0);
			return 0x08;

		case 0x46: /* BIT 0,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 0);
			return 0x10;

		case 0x47: /* BIT 0,A */
			BIT(cpu, cpu->r->a, 0);
			return 0x08;

		case 0x48: /* BIT 1,B */
			BIT(cpu, cpu->r->b, 1);
			return 0x08;

		case 0x49: /* BIT 1,C */
			BIT(cpu, cpu->r->c, 1);
			return 0x08;

		case 0x4A: /* BIT 1,D */
			BIT(cpu, cpu->r->d, 1);
			return 0x08;

		case 0x4B: /* BIT 1,E */
			BIT(cpu, cpu->r->e, 1);
			return 0x08;

		case 0x4C: /* BIT 1,H */
			BIT(cpu, cpu->r->h, 1);
			return 0x08;

		case 0x4D: /* BIT 1,L */
			BIT(cpu, cpu->r->l, 1);
			return 0x08;

		case 0x4E: /* BIT 1,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 1);
			return 0x10;

		case 0x4F: /* BIT 1,A */
			BIT(cpu, cpu->r->a, 1);
			return 0x08;

		case 0x50: /* BIT 2,B */
			BIT(cpu, cpu->r->b, 2);
			return 0x08;

		case 0x51: /* BIT 2,C */
			BIT(cpu, cpu->r->c, 2);
			return 0x08;

		case 0x52: /* BIT 2,D */
			BIT(cpu, cpu->r->d, 2);
			return 0x08;

		case 0x53: /* BIT 2,E */
			BIT(cpu, cpu->r->e, 2);
			return 0x08;

		case 0x54: /* BIT 2,H */
			BIT(cpu, cpu->r->h, 2);
			return 0x08;

		case 0x55: /* BIT 2,L */
			BIT(cpu, cpu->r->l, 2);
			return 0x08;

		case 0x56: /* BIT 2,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 2);
			return 0x10;

		case 0x57: /* BIT 2,A */
			BIT(cpu, cpu->r->a, 2);
			return 0x08;

		case 0x58: /* BIT 3,B */
			BIT(cpu, cpu->r->b, 3);
			return 0x08;

		case 0x59: /* BIT 3,C */
			BIT(cpu, cpu->r->c, 3);
			return 0x08;

		case 0x5A: /* BIT 3,D */
			BIT(cpu, cpu->r->d, 3);
			return 0x08;

		case 0x5B: /* BIT 3,E */
			BIT(cpu, cpu->r->e, 3);
			return 0x08;

		case 0x5C: /* BIT 3,H */
			BIT(cpu, cpu->r->h, 3);
			return 0x08;

		case 0x5D: /* BIT 3,L */
			BIT(cpu, cpu->r->l, 3);
			return 0x08;

		case 0x5E: /* BIT 3,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 3);
			return 0x10;

		case 0x5F: /* BIT 3,A */
			BIT(cpu, cpu->r->a, 3);
			return 0x08;

		case 0x60: /* BIT 4,B */
			BIT(cpu, cpu->r->b, 4);
			return 0x08;

		case 0x61: /* BIT 4,C */
			BIT(cpu, cpu->r->c, 4);
			return 0x08;

		case 0x62: /* BIT 4,D */
			BIT(cpu, cpu->r->d, 4);
			return 0x08;

		case 0x63: /* BIT 4,E */
			BIT(cpu, cpu->r->e, 4);
			return 0x08;

		case 0x64: /* BIT 4,H */
			BIT(cpu, cpu->r->h, 4);
			return 0x08;

		case 0x65: /* BIT 4,L */
			BIT(cpu, cpu->r->l, 4);
			return 0x08;

		case 0x66: /* BIT 4,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 4);
			return 0x10;

		case 0x67: /* BIT 4,A */
			BIT(cpu, cpu->r->a, 4);
			return 0x08;

		case 0x68: /* BIT 5,B */
			BIT(cpu, cpu->r->b, 5);
			return 0x08;

		case 0x69: /* BIT 5,C */
			BIT(cpu, cpu->r->c, 5);
			return 0x08;

		case 0x6A: /* BIT 5,D */
			BIT(cpu, cpu->r->d, 5);
			return 0x08;

		case 0x6B: /* BIT 5,E */
			BIT(cpu, cpu->r->e, 5);
			return 0x08;

		case 0x6C: /* BIT 5,H */
			BIT(cpu, cpu->r->h, 5);
			return 0x08;

		case 0x6D: /* BIT 5,L */
			BIT(cpu, cpu->r->l, 5);
			return 0x08;

		case 0x6E: /* BIT 5,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 5);
			return 0x10;

		case 0x6F: /* BIT 5,A */
			BIT(cpu, cpu->r->a, 5);
			return 0x08;

		case 0x70: /* BIT 6,B */
			BIT(cpu, cpu->r->b, 6);
			return 0x08;

		case 0x71: /* BIT 6,C */
			BIT(cpu, cpu->r->c, 6);
			return 0x08;

		case 0x72: /* BIT 6,D */
			BIT(cpu, cpu->r->d, 6);
			return 0x08;

		case 0x73: /* BIT 6,E */
			BIT(cpu, cpu->r->e, 6);
			return 0x08;

		case 0x74: /* BIT 6,H */
			BIT(cpu, cpu->r->h, 6);
			return 0x08;

		case 0x75: /* BIT 6,L */
			BIT(cpu, cpu->r->l, 6);
			return 0x08;

		case 0x76: /* BIT 6,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 6);
			return 0x10;

		case 0x77: /* BIT 6,A */
			BIT(cpu, cpu->r->a, 6);
			return 0x08;

		case 0x78: /* BIT 7,B */
			BIT(cpu, cpu->r->b, 7);
			return 0x08;

		case 0x79: /* BIT 7,C */
			BIT(cpu, cpu->r->c, 7);
			return 0x08;

		case 0x7A: /* BIT 7,D */
			BIT(cpu, cpu->r->d, 7);
			return 0x08;

		case 0x7B: /* BIT 7,E */
			BIT(cpu, cpu->r->e, 7);
			return 0x08;

		case 0x7C: /* BIT 7,H */
			BIT(cpu, cpu->r->h, 7);
			return 0x08;

		case 0x7D: /* BIT 7,L */
			BIT(cpu, cpu->r->l, 7);
			return 0x08;

		case 0x7E: /* BIT 7,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; BIT(cpu, mmuReadByte(mmu, word), 7);
			return 0x10;

		case 0x7F: /* BIT 7,A */
			BIT(cpu, cpu->r->a, 7);
			return 0x08;

		case 0x80: /* RES 0,B */
			cpu->r->b = RES(cpu->r->b, 0);
			return 0x08;

		case 0x81: /* RES 0,C */
			cpu->r->c = RES(cpu->r->c, 0);
			return 0x08;

		case 0x82: /* RES 0,D */
			cpu->r->d = RES(cpu->r->d, 0);
			return 0x08;

		case 0x83: /* RES 0,E */
			cpu->r->e = RES(cpu->r->e, 0);
			return 0x08;

		case 0x84: /* RES 0,H */
			cpu->r->h = RES(cpu->r->h, 0);
			return 0x08;

		case 0x85: /* RES 0,L */
			cpu->r->l = RES(cpu->r->l, 0);
			return 0x08;

		case 0x86: /* RES 0,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 0));
			return 0x10;

		case 0x87: /* RES 0,A */
			cpu->r->a = RES(cpu->r->a, 0);
			return 0x08;

		case 0x88: /* RES 1,B */
			cpu->r->b = RES(cpu->r->b, 1);
			return 0x08;

		case 0x89: /* RES 1,C */
			cpu->r->c = RES(cpu->r->c, 1);
			return 0x08;

		case 0x8A: /* RES 1,D */
			cpu->r->d = RES(cpu->r->d, 1);
			return 0x08;

		case 0x8B: /* RES 1,E */
			cpu->r->e = RES(cpu->r->e, 1);
			return 0x08;

		case 0x8C: /* RES 1,H */
			cpu->r->h = RES(cpu->r->h, 1);
			return 0x08;

		case 0x8D: /* RES 1,L */
			cpu->r->l = RES(cpu->r->l, 1);
			return 0x08;

		case 0x8E: /* RES 1,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 1));
			return 0x10;

		case 0x8F: /* RES 1,A */
			cpu->r->a = RES(cpu->r->a, 1);
			return 0x08;

		case 0x90: /* RES 2,B */
			cpu->r->b = RES(cpu->r->b, 2);
			return 0x08;

		case 0x91: /* RES 2,C */
			cpu->r->c = RES(cpu->r->c, 2);
			return 0x08;

		case 0x92: /* RES 2,D */
			cpu->r->d = RES(cpu->r->d, 2);
			return 0x08;

		case 0x93: /* RES 2,E */
			cpu->r->e = RES(cpu->r->e, 2);
			return 0x08;

		case 0x94: /* RES 2,H */
			cpu->r->h = RES(cpu->r->h, 2);
			return 0x08;

		case 0x95: /* RES 2,L */
			cpu->r->l = RES(cpu->r->l, 2);
			return 0x08;

		case 0x96: /* RES 2,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 2));
			return 0x10;

		case 0x97: /* RES 2,A */
			cpu->r->a = RES(cpu->r->a, 2);
			return 0x08;

		case 0x98: /* RES 3,B */
			cpu->r->b = RES(cpu->r->b, 3);
			return 0x08;

		case 0x99: /* RES 3,C */
			cpu->r->c = RES(cpu->r->c, 3);
			return 0x08;

		case 0x9A: /* RES 3,D */
			cpu->r->d = RES(cpu->r->d, 3);
			return 0x08;

		case 0x9B: /* RES 3,E */
			cpu->r->e = RES(cpu->r->e, 3);
			return 0x08;

		case 0x9C: /* RES 3,H */
			cpu->r->h = RES(cpu->r->h, 3);
			return 0x08;

		case 0x9D: /* RES 3,L */
			cpu->r->l = RES(cpu->r->l, 3);
			return 0x08;

		case 0x9E: /* RES 3,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 3));
			return 0x10;

		case 0x9F: /* RES 3,A */
			cpu->r->a = RES(cpu->r->a, 3);
			return 0x08;

		case 0xA0: /* RES 4,B */
			cpu->r->b = RES(cpu->r->b, 4);
			return 0x08;

		case 0xA1: /* RES 4,C */
			cpu->r->c = RES(cpu->r->c, 4);
			return 0x08;

		case 0xA2: /* RES 4,D */
			cpu->r->d = RES(cpu->r->d, 4);
			return 0x08;

		case 0xA3: /* RES 4,E */
			cpu->r->e = RES(cpu->r->e, 4);
			return 0x08;

		case 0xA4: /* RES 4,H */
			cpu->r->h = RES(cpu->r->h, 4);
			return 0x08;

		case 0xA5: /* RES 4,L */
			cpu->r->l = RES(cpu->r->l, 4);
			return 0x08;

		case 0xA6: /* RES 4,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 4));
			return 0x10;

		case 0xA7: /* RES 4,A */
			cpu->r->a = RES(cpu->r->a, 4);
			return 0x08;

		case 0xA8: /* RES 5,B */
			cpu->r->b = RES(cpu->r->b, 5);
			return 0x08;

		case 0xA9: /* RES 5,C */
			cpu->r->c = RES(cpu->r->c, 5);
			return 0x08;

		case 0xAA: /* RES 5,D */
			cpu->r->d = RES(cpu->r->d, 5);
			return 0x08;

		case 0xAB: /* RES 5,E */
			cpu->r->e = RES(cpu->r->e, 5);
			return 0x08;

		case 0xAC: /* RES 5,H */
			cpu->r->h = RES(cpu->r->h, 5);
			return 0x08;

		case 0xAD: /* RES 5,L */
			cpu->r->l = RES(cpu->r->l, 5);
			return 0x08;

		case 0xAE: /* RES 5,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 5));
			return 0x10;

		case 0xAF: /* RES 5,A */
			cpu->r->a = RES(cpu->r->a, 5);
			return 0x08;

		case 0xB0: /* RES 6,B */
			cpu->r->b = RES(cpu->r->b, 6);
			return 0x08;

		case 0xB1: /* RES 6,C */
			cpu->r->c = RES(cpu->r->c, 6);
			return 0x08;

		case 0xB2: /* RES 6,D */
			cpu->r->d = RES(cpu->r->d, 6);
			return 0x08;

		case 0xB3: /* RES 6,E */
			cpu->r->e = RES(cpu->r->e, 6);
			return 0x08;

		case 0xB4: /* RES 6,H */
			cpu->r->h = RES(cpu->r->h, 6);
			return 0x08;

		case 0xB5: /* RES 6,L */
			cpu->r->l = RES(cpu->r->l, 6);
			return 0x08;

		case 0xB6: /* RES 6,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 6));
			return 0x10;

		case 0xB7: /* RES 6,A */
			cpu->r->a = RES(cpu->r->a, 6);
			return 0x08;

		case 0xB8: /* RES 7,B */
			cpu->r->b = RES(cpu->r->b, 7);
			return 0x08;

		case 0xB9: /* RES 7,C */
			cpu->r->c = RES(cpu->r->c, 7);
			return 0x08;

		case 0xBA: /* RES 7,D */
			cpu->r->d = RES(cpu->r->d, 7);
			return 0x08;

		case 0xBB: /* RES 7,E */
			cpu->r->e = RES(cpu->r->e, 7);
			return 0x08;

		case 0xBC: /* RES 7,H */
			cpu->r->h = RES(cpu->r->h, 7);
			return 0x08;

		case 0xBD: /* RES 7,L */
			cpu->r->l = RES(cpu->r->l, 7);
			return 0x08;

		case 0xBE: /* RES 7,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 7));
			return 0x10;

		case 0xBF: /* RES 7,A */
			cpu->r->a = RES(cpu->r->a, 7);
			return 0x08;

		case 0xC0: /* SET 0,B */
			cpu->r->b = SET(cpu->r->b, 0);
			return 0x08;

		case 0xC1: /* SET 0,C */
			cpu->r->c = SET(cpu->r->c, 0);
			return 0x08;

		case 0xC2: /* SET 0,D */
			cpu->r->d = SET(cpu->r->d, 0);
			return 0x08;

		case 0xC3: /* SET 0,E */
			cpu->r->e = SET(cpu->r->e, 0);
			return 0x08;

		case 0xC4: /* SET 0,H */
			cpu->r->h = SET(cpu->r->h, 0);
			return 0x08;

		case 0xC5: /* SET 0,L */
			cpu->r->l = SET(cpu->r->l, 0);
			return 0x08;

		case 0xC6: /* SET 0,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 0));
			return 0x10;

		case 0xC7: /* SET 0,A */
			cpu->r->a = SET(cpu->r->a, 0);
			return 0x08;

		case 0xC8: /* SET 1,B */
			cpu->r->b = SET(cpu->r->b, 1);
			return 0x08;

		case 0xC9: /* SET 1,C */
			cpu->r->c = SET(cpu->r->c, 1);
			return 0x08;

		case 0xCA: /* SET 1,D */
			cpu->r->d = SET(cpu->r->d, 1);
			return 0x08;

		case 0xCB: /* SET 1,E */
			cpu->r->e = SET(cpu->r->e, 1);
			return 0x08;

		case 0xCC: /* SET 1,H */
			cpu->r->h = SET(cpu->r->h, 1);
			return 0x08;

		case 0xCD: /* SET 1,L */
			cpu->r->l = SET(cpu->r->l, 1);
			return 0x08;

		case 0xCE: /* SET 1,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 1));
			return 0x10;

		case 0xCF: /* SET 1,A */
			cpu->r->a = SET(cpu->r->a, 1);
			return 0x08;

		case 0xD0: /* SET 2,B */
			cpu->r->b = SET(cpu->r->b, 2);
			return 0x08;

		case 0xD1: /* SET 2,C */
			cpu->r->c = SET(cpu->r->c, 2);
			return 0x08;

		case 0xD2: /* SET 2,D */
			cpu->r->d = SET(cpu->r->d, 2);
			return 0x08;

		case 0xD3: /* SET 2,E */
			cpu->r->e = SET(cpu->r->e, 2);
			return 0x08;

		case 0xD4: /* SET 2,H */
			cpu->r->h = SET(cpu->r->h, 2);
			return 0x08;

		case 0xD5: /* SET 2,L */
			cpu->r->l = SET(cpu->r->l, 2);
			return 0x08;

		case 0xD6: /* SET 2,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 2));
			return 0x10;

		case 0xD7: /* SET 2,A */
			cpu->r->a = SET(cpu->r->a, 2);
			return 0x08;

		case 0xD8: /* SET 3,B */
			cpu->r->b = SET(cpu->r->b, 3);
			return 0x08;

		case 0xD9: /* SET 3,C */
			cpu->r->c = SET(cpu->r->c, 3);
			return 0x08;

		case 0xDA: /* SET 3,D */
			cpu->r->d = SET(cpu->r->d, 3);
			return 0x08;

		case 0xDB: /* SET 3,E */
			cpu->r->e = SET(cpu->r->e, 3);
			return 0x08;

		case 0xDC: /* SET 3,H */
			cpu->r->h = SET(cpu->r->h, 3);
			return 0x08;

		case 0xDD: /* SET 3,L */
			cpu->r->l = SET(cpu->r->l, 3);
			return 0x08;

		case 0xDE: /* SET 3,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 3));
			return 0x10;

		case 0xDF: /* SET 3,A */
			cpu->r->a = SET(cpu->r->a, 3);
			return 0x08;

		case 0xE0: /* SET 4,B */
			cpu->r->b = SET(cpu->r->b, 4);
			return 0x08;

		case 0xE1: /* SET 4,C */
			cpu->r->c = SET(cpu->r->c, 4);
			return 0x08;

		case 0xE2: /* SET 4,D */
			cpu->r->d = SET(cpu->r->d, 4);
			return 0x08;

		case 0xE3: /* SET 4,E */
			cpu->r->e = SET(cpu->r->e, 4);
			return 0x08;

		case 0xE4: /* SET 4,H */
			cpu->r->h = SET(cpu->r->h, 4);
			return 0x08;

		case 0xE5: /* SET 4,L */
			cpu->r->l = SET(cpu->r->l, 4);
			return 0x08;

		case 0xE6: /* SET 4,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 4));
			return 0x10;

		case 0xE7: /* SET 4,A */
			cpu->r->a = SET(cpu->r->a, 4);
			return 0x08;

		case 0xE8: /* SET 5,B */
			cpu->r->b = SET(cpu->r->b, 5);
			return 0x08;

		case 0xE9: /* SET 5,C */
			cpu->r->c = SET(cpu->r->c, 5);
			return 0x08;

		case 0xEA: /* SET 5,D */
			cpu->r->d = SET(cpu->r->d, 5);
			return 0x08;

		case 0xEB: /* SET 5,E */
			cpu->r->e = SET(cpu->r->e, 5);
			return 0x08;

		case 0xEC: /* SET 5,H */
			cpu->r->h = SET(cpu->r->h, 5);
			return 0x08;

		case 0xED: /* SET 5,L */
			cpu->r->l = SET(cpu->r->l, 5);
			return 0x08;

		case 0xEE: /* SET 5,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 5));
			return 0x10;

		case 0xEF: /* SET 5,A */
			cpu->r->a = SET(cpu->r->a, 5);
			return 0x08;

		case 0xF0: /* SET 6,B */
			cpu->r->b = SET(cpu->r->b, 6);
			return 0x08;

		case 0xF1: /* SET 6,C */
			cpu->r->c = SET(cpu->r->c, 6);
			return 0x08;

		case 0xF2: /* SET 6,D */
			cpu->r->d = SET(cpu->r->d, 6);
			return 0x08;

		case 0xF3: /* SET 6,E */
			cpu->r->e = SET(cpu->r->e, 6);
			return 0x08;

		case 0xF4: /* SET 6,H */
			cpu->r->h = SET(cpu->r->h, 6);
			return 0x08;

		case 0xF5: /* SET 6,L */
			cpu->r->l = SET(cpu->r->l, 6);
			return 0x08;

		case 0xF6: /* SET 6,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 6));
			return 0x10;

		case 0xF7: /* SET 6,A */
			cpu->r->a = SET(cpu->r->a, 6);
			return 0x08;

		case 0xF8: /* SET 7,B */
			cpu->r->b = SET(cpu->r->b, 7);
			return 0x08;

		case 0xF9: /* SET 7,C */
			cpu->r->c = SET(cpu->r->c, 7);
			return 0x08;

		case 0xFA: /* SET 7,D */
			cpu->r->d = SET(cpu->r->d, 7);
			return 0x08;

		case 0xFB: /* SET 7,E */
			cpu->r->e = SET(cpu->r->e, 7);
			return 0x08;

		case 0xFC: /* SET 7,H */
			cpu->r->h = SET(cpu->r->h, 7);
			return 0x08;

		case 0xFD: /* SET 7,L */
			cpu->r->l = SET(cpu->r->l, 7);
			return 0x08;

		case 0xFE: /* SET 7,(HL) */
			word = (cpu->r->h << 8) + cpu->r->l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 7));
			return 0x10;

		case 0xFF: /* SET 7,A */
			cpu->r->a = SET(cpu->r->a, 7);
			return 0x08;

		default:
			printf("WARNING: Could not process CB operation: `%x`\n", opcode);
			return 0x00;
	}
}