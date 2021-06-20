#include "opcode.h"

/* TODO: Refactor this. */
inline static uint16_t ADD_WW(CPU *cpu, uint16_t a, uint16_t b) { cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_C, (0xFFFF - a) < b ? 1 : 0); cpuSetFlag(cpu, FLAG_H, (0xFFF - (a & 0xFFF)) < (b & 0xFFF) ? 1 : 0); return a + b; }

inline static uint8_t _SUB(CPU *cpu, uint8_t op, uint8_t carry) {
  int16_t result = cpu->regs.a - op - carry;

  cpuSetFlag(cpu, FLAG_Z, !result);
  cpuSetFlag(cpu, FLAG_N, 1);
  cpuSetFlag(cpu, FLAG_H, !!((cpu->regs.a & 0xF) - (op & 0xF) - carry < 0));
  cpuSetFlag(cpu, FLAG_C, !!(result < 0));

  return result;
}

inline static uint8_t _ADD(CPU *cpu, uint8_t op, uint8_t carry) {
	uint16_t result = cpu->regs.a + op + carry;

	cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
	cpuSetFlag(cpu, FLAG_N, 0);
	cpuSetFlag(cpu, FLAG_H, !!(((cpu->regs.a & 0xF) + (op & 0xF) + carry) > 0xF));
	cpuSetFlag(cpu, FLAG_C, !!(result > 0xFF));

	return result;
}

inline static uint8_t _BITWISE(CPU *cpu, uint8_t res, uint8_t h) {
	cpuSetFlag(cpu, FLAG_Z, !res);
	cpuSetFlag(cpu, FLAG_N, 0);
	cpuSetFlag(cpu, FLAG_H, h);
	cpuSetFlag(cpu, FLAG_C, 0);

	return res;
}

/**
 * Arithmetic Operations
 */
inline static uint8_t ADD(CPU *cpu, uint8_t op) {
	return _ADD(cpu, op, 0);
}
inline static uint8_t ADC(CPU *cpu, uint8_t op) {
	return _ADD(cpu, op, cpuGetFlag(cpu, FLAG_C));
}
inline static uint8_t SUB(CPU *cpu, uint8_t op) {
	return _SUB(cpu, op, 0);
}
inline static uint8_t SBC(CPU *cpu, uint8_t op) {
	return _SUB(cpu, op, cpuGetFlag(cpu, FLAG_C));
}

inline static uint8_t INC_B(CPU *cpu, uint8_t op) {
  cpuSetFlag(cpu, FLAG_Z, !(++op));
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, !(op & 0xF));

  return op;
}
inline static uint16_t INC_W(uint16_t op) {
	return ++op;
}
inline static uint8_t DEC_B(CPU *cpu, uint8_t op) {
	cpuSetFlag(cpu, FLAG_Z, !(--op));
	cpuSetFlag(cpu, FLAG_N, 1);
	cpuSetFlag(cpu, FLAG_H, (op & 0xF) == 0xF);

	return op;
}
inline static uint16_t DEC_W(uint16_t op) {
	return --op;
}

/**
 * Binary Operations
 */
inline static uint8_t AND(CPU *cpu, uint8_t a, uint8_t b) {
	return _BITWISE(cpu, a &= b, 1);
}
inline static uint8_t OR(CPU *cpu, uint8_t a, uint8_t b) {
	return _BITWISE(cpu, a |= b, 0);
}
inline static uint8_t XOR(CPU *cpu, uint8_t a, uint8_t b) {
	return _BITWISE(cpu, a ^= b, 0);
}

/**
 * Stack Operations
 */
inline static void PUSH(CPU *cpu, MMU *mmu, uint16_t val) {
	cpu->regs.sp -= 2;
	mmuWriteWord(mmu, cpu->regs.sp, val);
}
inline static uint16_t POP(CPU *cpu, MMU *mmu) {
	uint16_t val = mmuReadWord(mmu, cpu->regs.sp);
	cpu->regs.sp += 2;

	return val;
}

/**
 * Other Operations
 */
inline static uint8_t SWAP(CPU *cpu, uint8_t a) { a = (a >> 4) | ((a & 0xF) << 4); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); cpuSetFlag(cpu, FLAG_Z, 0); return a; }
inline static uint8_t RLC(CPU *cpu, uint8_t a) { uint8_t r = (a & 0x80) >> 7; cpuSetFlag(cpu, FLAG_C, r); a = (a << 1) + r; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RL(CPU *cpu, uint8_t a) { uint8_t c = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7); a = (a << 1) + c; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RRC(CPU *cpu, uint8_t a) { uint8_t r = a & 0x1; cpuSetFlag(cpu, FLAG_C, r); a = (a >> 1) + (r << 7); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t RR(CPU *cpu, uint8_t a) { uint8_t c = cpuGetFlag(cpu, FLAG_C); cpuSetFlag(cpu, FLAG_C, a & 0x1); a = (a >> 1) + (c << 7); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SLA(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7); a <<= 1; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SRA(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, a & 0x1); a >>= 1; a |= ((a & 0x40) << 1); cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static uint8_t SRL(CPU *cpu, uint8_t a) { cpuSetFlag(cpu, FLAG_C, a & 0x1); a >>= 1; cpuSetFlag(cpu, FLAG_Z, !a ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0); return a; }
inline static void BIT(CPU *cpu, uint8_t a, uint8_t b) { cpuSetFlag(cpu, FLAG_Z, !(a & (1 << b)) ? 1 : 0); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 1); }
inline static uint8_t SET(uint8_t a, uint8_t b) { return a | (1 << b); }
inline static uint8_t RES(uint8_t a, uint8_t b) { return a & ~(1 << b); }
inline static uint16_t JR(uint16_t pc, uint8_t a) { if ((a & 0x80) == 0x80) { a = ~(a - 0x1); return pc - a; } return pc + a; }

uint8_t cpuOpcode(CPU *cpu, MMU *mmu, uint8_t opcode)
{
  uint8_t byte;
  uint16_t word;

  switch (opcode) {
    case 0: /* NOP */
      return 4;

    case 0x1: /* LD BC,d16 */
      word = mmuReadWord(mmu, cpu->regs.pc); cpu->regs.b = word >> 8; cpu->regs.c = word & 0xFF; cpu->regs.pc += 2;
      return 12;

    case 0x2: /* LD (BC),A */
      mmuWriteByte(mmu, (cpu->regs.b << 8) + cpu->regs.c, cpu->regs.a);
      return 10;

    case 0x3: /* INC BC */
      word = INC_W((cpu->regs.b << 8) + cpu->regs.c); cpu->regs.b = word >> 8; cpu->regs.c = word & 0xFF;
      return 10;

    case 0x4: /* INC B */
      cpu->regs.b = INC_B(cpu, cpu->regs.b);
      return 4;

    case 0x5: /* DEC B */
      cpu->regs.b = DEC_B(cpu, cpu->regs.b);
      return 4;

    case 0x6: /* LD B,d8 */
      cpu->regs.b = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x7: /* RLCA */
      cpu->regs.a = RLC(cpu, cpu->regs.a); cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

    case 0x8: /* LD (a16),SP */
      mmuWriteWord(mmu, mmuReadWord(mmu, cpu->regs.pc), cpu->regs.sp); cpu->regs.pc += 2;
      return 20;

    case 0x9: /* ADD HL,BC */
      word = ADD_WW(cpu, (cpu->regs.h << 8) + cpu->regs.l, (cpu->regs.b << 8) + cpu->regs.c); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0xA: /* LD A,(BC) */
      cpu->regs.a = mmuReadByte(mmu, (cpu->regs.b << 8) + cpu->regs.c);
      return 10;

    case 0xB: /* DEC BC */
      word = DEC_W((cpu->regs.b << 8) + cpu->regs.c); cpu->regs.b = word >> 8; cpu->regs.c = word & 0xFF;
      return 10;

    case 0xC: /* INC C */
      cpu->regs.c = INC_B(cpu, cpu->regs.c);
      return 4;

    case 0xD: /* DEC C */
      cpu->regs.c = DEC_B(cpu, cpu->regs.c);
      return 4;

    case 0xE: /* LD C,d8 */
      cpu->regs.c = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0xF: /* RRCA */
      cpu->regs.a = RRC(cpu, cpu->regs.a); cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

    case 0x10: /* STOP 0 */
      cpu->stop = true;
      return 4;

    case 0x11: /* LD DE,d16 */
      word = mmuReadWord(mmu, cpu->regs.pc); cpu->regs.d = word >> 8; cpu->regs.e = word & 0xFF; cpu->regs.pc += 2;
      return 12;

    case 0x12: /* LD (DE),A */
      mmuWriteByte(mmu, (cpu->regs.d << 8) + cpu->regs.e, cpu->regs.a);
      return 10;

    case 0x13: /* INC DE */
      word = INC_W((cpu->regs.d << 8) + cpu->regs.e); cpu->regs.d = word >> 8; cpu->regs.e = word & 0xFF;
      return 10;

    case 0x14: /* INC D */
      cpu->regs.d = INC_B(cpu, cpu->regs.d);
      return 4;

    case 0x15: /* DEC D */
      cpu->regs.d = DEC_B(cpu, cpu->regs.d);
      return 4;

    case 0x16: /* LD D,d8 */
      cpu->regs.d = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x17: /* RLA */
      cpu->regs.a = RL(cpu, cpu->regs.a);
      return 4;

    case 0x18: /* JR r8 */
      cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 12;

    case 0x19: /* ADD HL,DE */
      word = ADD_WW(cpu, (cpu->regs.h << 8) + cpu->regs.l, (cpu->regs.d << 8) + cpu->regs.e); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x1A: /* LD A,(DE) */
      cpu->regs.a = mmuReadByte(mmu, (cpu->regs.d << 8) + cpu->regs.e);
      return 10;

    case 0x1B: /* DEC DE */
      word = DEC_W((cpu->regs.d << 8) + cpu->regs.e); cpu->regs.d = word >> 8; cpu->regs.e = word & 0xFF;
      return 10;

    case 0x1C: /* INC E */
      cpu->regs.e = INC_B(cpu, cpu->regs.e);
      return 4;

    case 0x1D: /* DEC E */
      cpu->regs.e = DEC_B(cpu, cpu->regs.e);
      return 4;

    case 0x1E: /* LD E,d8 */
      cpu->regs.e = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x1F: /* RRA */
      cpu->regs.a = RR(cpu, cpu->regs.a);
      return 4;

    case 0x20: /* JR NZ,r8 */
      if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++; return 12; } cpu->regs.pc++;
      return 10;

    case 0x21: /* LD HL,d16 */
      word = mmuReadWord(mmu, cpu->regs.pc); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF; cpu->regs.pc += 2;
      return 12;

    case 0x22: /* LD (HL+),A */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, cpu->regs.a); word = INC_W(word); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x23: /* INC HL */
      word = INC_W((cpu->regs.h << 8) + cpu->regs.l); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x24: /* INC H */
      cpu->regs.h = INC_B(cpu, cpu->regs.h);
      return 4;

    case 0x25: /* DEC H */
      cpu->regs.h = DEC_B(cpu, cpu->regs.h);
      return 4;

    case 0x26: /* LD H,d8 */
      cpu->regs.h = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x27: /* DAA */
    {
      uint16_t a = cpu->regs.a;

      if(cpuGetFlag(cpu, FLAG_N)) {
        a -= cpuGetFlag(cpu, FLAG_H) ? 0x6 : 0;
        a -= cpuGetFlag(cpu, FLAG_C) ? 0x60 : 0;
      } else {
        a += cpuGetFlag(cpu, FLAG_H) || (a & 0xF) > 0x9 ? 0x6 : 0;
        if (cpuGetFlag(cpu, FLAG_C) || a > 0x9F) {
          a += 0x60;
          cpuSetFlag(cpu, FLAG_C, 1);
        }
      }

      cpu->regs.a = a;

      cpuSetFlag(cpu, FLAG_H, 0);
      cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);

      return 4;
    }

    case 0x28: /* JR Z,r8 */
      if (cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++; return 12; } cpu->regs.pc++;
      return 10;

    case 0x29: /* ADD HL,HL */
      word = ADD_WW(cpu, (cpu->regs.h << 8) + cpu->regs.l, (cpu->regs.h << 8) + cpu->regs.l); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x2A: /* LD A,(HL+) */
      word = (cpu->regs.h << 8) + cpu->regs.l; cpu->regs.a = mmuReadByte(mmu, word); word = INC_W(word); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x2B: /* DEC HL */
      word = DEC_W((cpu->regs.h << 8) + cpu->regs.l); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x2C: /* INC L */
      cpu->regs.l = INC_B(cpu, cpu->regs.l);
      return 4;

    case 0x2D: /* DEC L */
      cpu->regs.l = DEC_B(cpu, cpu->regs.l);
      return 4;

    case 0x2E: /* LD L,d8 */
      cpu->regs.l = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x2F: /* CPL */
      cpu->regs.a = ~cpu->regs.a; cpuSetFlag(cpu, FLAG_N, 1); cpuSetFlag(cpu, FLAG_H, 1);
      return 4;

    case 0x30: /* JR NC,r8 */
      if (!cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++; return 12; } cpu->regs.pc++;
      return 10;

    case 0x31: /* LD SP,d16 */
      cpu->regs.sp = mmuReadWord(mmu, cpu->regs.pc); cpu->regs.pc += 2;
      return 12;

    case 0x32: /* LD (HL-),A */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, cpu->regs.a); word = DEC_W(word); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x33: /* INC SP */
      cpu->regs.sp = INC_W(cpu->regs.sp);
      return 10;

    case 0x34: /* INC (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; byte = INC_B(cpu, mmuReadByte(mmu, word)); mmuWriteByte(mmu, word, byte);
      return 12;

    case 0x35: /* DEC (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; byte = DEC_B(cpu, mmuReadByte(mmu, word)); mmuWriteByte(mmu, word, byte);
      return 12;

    case 0x36: /* LD (HL),d8 */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 12;

    case 0x37: /* SCF */
      cpuSetFlag(cpu, FLAG_C, 1); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0);
      return 4;

    case 0x38: /* JR C,r8 */
      if (cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++; return 12; } cpu->regs.pc++;
      return 10;

    case 0x39: /* ADD HL,SP */
      word = ADD_WW(cpu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.sp); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x3A: /* LD A,(HL-) */
      word = (cpu->regs.h << 8) + cpu->regs.l; cpu->regs.a = mmuReadByte(mmu, word); word = INC_W(word); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 10;

    case 0x3B: /* DEC SP */
      cpu->regs.sp = DEC_W(cpu->regs.sp);
      return 10;

    case 0x3C: /* INC A */
      cpu->regs.a = INC_B(cpu, cpu->regs.a);
      return 4;

    case 0x3D: /* DEC A */
      cpu->regs.a = DEC_B(cpu, cpu->regs.a);
      return 4;

    case 0x3E: /* LD A,d8 */
      cpu->regs.a = mmuReadByte(mmu, cpu->regs.pc); cpu->regs.pc++;
      return 10;

    case 0x3F: /* CCF */
      cpuSetFlag(cpu, FLAG_C, !cpuGetFlag(cpu, FLAG_C)); cpuSetFlag(cpu, FLAG_N, 0); cpuSetFlag(cpu, FLAG_H, 0);
      return 4;

    case 0x40: /* LD B,B */
      cpu->regs.b = cpu->regs.b;
      return 4;

    case 0x41: /* LD B,C */
      cpu->regs.b = cpu->regs.c;
      return 4;

    case 0x42: /* LD B,D */
      cpu->regs.b = cpu->regs.d;
      return 4;

    case 0x43: /* LD B,E */
      cpu->regs.b = cpu->regs.e;
      return 4;

    case 0x44: /* LD B,H */
      cpu->regs.b = cpu->regs.h;
      return 4;

    case 0x45: /* LD B,L */
      cpu->regs.b = cpu->regs.l;
      return 4;

    case 0x46: /* LD B,(HL) */
      cpu->regs.b = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x47: /* LD B,A */
      cpu->regs.b = cpu->regs.a;
      return 4;

    case 0x48: /* LD C,B */
      cpu->regs.c = cpu->regs.b;
      return 4;

    case 0x49: /* LD C,C */
      cpu->regs.c = cpu->regs.c;
      return 4;

    case 0x4A: /* LD C,D */
      cpu->regs.c = cpu->regs.d;
      return 4;

    case 0x4B: /* LD C,E */
      cpu->regs.c = cpu->regs.e;
      return 4;

    case 0x4C: /* LD C,H */
      cpu->regs.c = cpu->regs.h;
      return 4;

    case 0x4D: /* LD C,L */
      cpu->regs.c = cpu->regs.l;
      return 4;

    case 0x4E: /* LD C,(HL) */
      cpu->regs.c = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x4F: /* LD C,A */
      cpu->regs.c = cpu->regs.a;
      return 4;

    case 0x50: /* LD D,B */
      cpu->regs.d = cpu->regs.b;
      return 4;

    case 0x51: /* LD D,C */
      cpu->regs.d = cpu->regs.c;
      return 4;

    case 0x52: /* LD D,D */
      cpu->regs.d = cpu->regs.d;
      return 4;

    case 0x53: /* LD D,E */
      cpu->regs.d = cpu->regs.e;
      return 4;

    case 0x54: /* LD D,H */
      cpu->regs.d = cpu->regs.h;
      return 4;

    case 0x55: /* LD D,L */
      cpu->regs.d = cpu->regs.l;
      return 4;

    case 0x56: /* LD D,(HL) */
      cpu->regs.d = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x57: /* LD D,A */
      cpu->regs.d = cpu->regs.a;
      return 4;

    case 0x58: /* LD E,B */
      cpu->regs.e = cpu->regs.b;
      return 4;

    case 0x59: /* LD E,C */
      cpu->regs.e = cpu->regs.c;
      return 4;

    case 0x5A: /* LD E,D */
      cpu->regs.e = cpu->regs.d;
      return 4;

    case 0x5B: /* LD E,E */
      cpu->regs.e = cpu->regs.e;
      return 4;

    case 0x5C: /* LD E,H */
      cpu->regs.e = cpu->regs.h;
      return 4;

    case 0x5D: /* LD E,L */
      cpu->regs.e = cpu->regs.l;
      return 4;

    case 0x5E: /* LD E,(HL) */
      cpu->regs.e = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x5F: /* LD E,A */
      cpu->regs.e = cpu->regs.a;
      return 4;

    case 0x60: /* LD H,B */
      cpu->regs.h = cpu->regs.b;
      return 4;

    case 0x61: /* LD H,C */
      cpu->regs.h = cpu->regs.c;
      return 4;

    case 0x62: /* LD H,D */
      cpu->regs.h = cpu->regs.d;
      return 4;

    case 0x63: /* LD H,E */
      cpu->regs.h = cpu->regs.e;
      return 4;

    case 0x64: /* LD H,H */
      cpu->regs.h = cpu->regs.h;
      return 4;

    case 0x65: /* LD H,L */
      cpu->regs.h = cpu->regs.l;
      return 4;

    case 0x66: /* LD H,(HL) */
      cpu->regs.h = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x67: /* LD H,A */
      cpu->regs.h = cpu->regs.a;
      return 4;

    case 0x68: /* LD L,B */
      cpu->regs.l = cpu->regs.b;
      return 4;

    case 0x69: /* LD L,C */
      cpu->regs.l = cpu->regs.c;
      return 4;

    case 0x6A: /* LD L,D */
      cpu->regs.l = cpu->regs.d;
      return 4;

    case 0x6B: /* LD L,E */
      cpu->regs.l = cpu->regs.e;
      return 4;

    case 0x6C: /* LD L,H */
      cpu->regs.l = cpu->regs.h;
      return 4;

    case 0x6D: /* LD L,L */
      cpu->regs.l = cpu->regs.l;
      return 4;

    case 0x6E: /* LD L,(HL) */
      cpu->regs.l = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x6F: /* LD L,A */
      cpu->regs.l = cpu->regs.a;
      return 4;

    case 0x70: /* LD (HL),B */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.b);
      return 10;

    case 0x71: /* LD (HL),C */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.c);
      return 10;

    case 0x72: /* LD (HL),D */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.d);
      return 10;

    case 0x73: /* LD (HL),E */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.e);
      return 10;

    case 0x74: /* LD (HL),H */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.h);
      return 10;

    case 0x75: /* LD (HL),L */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.l);
      return 10;

    case 0x76: /* HALT */
      cpu->halt = true;
      return 4;

    case 0x77: /* LD (HL),A */
      mmuWriteByte(mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.a);
      return 10;

    case 0x78: /* LD A,B */
      cpu->regs.a = cpu->regs.b;
      return 4;

    case 0x79: /* LD A,C */
      cpu->regs.a = cpu->regs.c;
      return 4;

    case 0x7A: /* LD A,D */
      cpu->regs.a = cpu->regs.d;
      return 4;

    case 0x7B: /* LD A,E */
      cpu->regs.a = cpu->regs.e;
      return 4;

    case 0x7C: /* LD A,H */
      cpu->regs.a = cpu->regs.h;
      return 4;

    case 0x7D: /* LD A,L */
      cpu->regs.a = cpu->regs.l;
      return 4;

    case 0x7E: /* LD A,(HL) */
      cpu->regs.a = mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 10;

    case 0x7F: /* LD A,A */
      cpu->regs.a = cpu->regs.a;
      return 4;

    case 0x80: /* ADD A,B */
      cpu->regs.a = ADD(cpu, cpu->regs.b);
      return 4;

    case 0x81: /* ADD A,C */
      cpu->regs.a = ADD(cpu, cpu->regs.c);
      return 4;

    case 0x82: /* ADD A,D */
      cpu->regs.a = ADD(cpu, cpu->regs.d);
      return 4;

    case 0x83: /* ADD A,E */
      cpu->regs.a = ADD(cpu, cpu->regs.e);
      return 4;

    case 0x84: /* ADD A,H */
      cpu->regs.a = ADD(cpu, cpu->regs.h);
      return 4;

    case 0x85: /* ADD A,L */
      cpu->regs.a = ADD(cpu, cpu->regs.l);
      return 4;

    case 0x86: /* ADD A,(HL) */
      cpu->regs.a = ADD(cpu, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0x87: /* ADD A,A */
      cpu->regs.a = ADD(cpu, cpu->regs.a);
      return 4;

    case 0x88: /* ADC A,B */
      cpu->regs.a = ADC(cpu, cpu->regs.b);
      return 4;

    case 0x89: /* ADC A,C */
      cpu->regs.a = ADC(cpu, cpu->regs.c);
      return 4;

    case 0x8A: /* ADC A,D */
      cpu->regs.a = ADC(cpu, cpu->regs.d);
      return 4;

    case 0x8B: /* ADC A,E */
      cpu->regs.a = ADC(cpu, cpu->regs.e);
      return 4;

    case 0x8C: /* ADC A,H */
      cpu->regs.a = ADC(cpu, cpu->regs.h);
      return 4;

    case 0x8D: /* ADC A,L */
      cpu->regs.a = ADC(cpu, cpu->regs.l);
      return 4;

    case 0x8E: /* ADC A,(HL) */
      cpu->regs.a = ADC(cpu, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0x8F: /* ADC A,A */
      cpu->regs.a = ADC(cpu, cpu->regs.a);
      return 4;

    case 0x90: /* SUB A,B */
      cpu->regs.a = SUB(cpu, cpu->regs.b);
      return 4;

    case 0x91: /* SUB A,C */
      cpu->regs.a = SUB(cpu, cpu->regs.c);
      return 4;

    case 0x92: /* SUB A,D */
      cpu->regs.a = SUB(cpu, cpu->regs.d);
      return 4;

    case 0x93: /* SUB A,E */
      cpu->regs.a = SUB(cpu, cpu->regs.e);
      return 4;

    case 0x94: /* SUB A,H */
      cpu->regs.a = SUB(cpu, cpu->regs.h);
      return 4;

    case 0x95: /* SUB A,L */
      cpu->regs.a = SUB(cpu, cpu->regs.l);
      return 4;

    case 0x96: /* SUB A,(HL) */
      cpu->regs.a = SUB(cpu, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0x97: /* SUB A,A */
      cpu->regs.a = SUB(cpu, cpu->regs.a);
      return 4;

    case 0x98: /* SBC A,B */
      cpu->regs.a = SBC(cpu, cpu->regs.b);
      return 4;

    case 0x99: /* SBC A,C */
      cpu->regs.a = SBC(cpu, cpu->regs.c);
      return 4;

    case 0x9A: /* SBC A,D */
      cpu->regs.a = SBC(cpu, cpu->regs.d);
      return 4;

    case 0x9B: /* SBC A,E */
      cpu->regs.a = SBC(cpu, cpu->regs.e);
      return 4;

    case 0x9C: /* SBC A,H */
      cpu->regs.a = SBC(cpu, cpu->regs.h);
      return 4;

    case 0x9D: /* SBC A,L */
      cpu->regs.a = SBC(cpu, cpu->regs.l);
      return 4;

    case 0x9E: /* SBC A,(HL) */
      cpu->regs.a = SBC(cpu, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0x9F: /* SBC A,A */
      cpu->regs.a = SBC(cpu, cpu->regs.a);
      return 4;

    case 0xA0: /* AND B */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.b);
      return 4;

    case 0xA1: /* AND C */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.c);
      return 4;

    case 0xA2: /* AND D */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.d);
      return 4;

    case 0xA3: /* AND E */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.e);
      return 4;

    case 0xA4: /* AND H */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.h);
      return 4;

    case 0xA5: /* AND L */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.l);
      return 4;

    case 0xA6: /* AND (HL) */
      cpu->regs.a = AND(cpu, cpu->regs.a, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0xA7: /* AND A */
      cpu->regs.a = AND(cpu, cpu->regs.a, cpu->regs.a);
      return 4;

    case 0xA8: /* XOR B */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.b);
      return 4;

    case 0xA9: /* XOR C */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.c);
      return 4;

    case 0xAA: /* XOR D */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.d);
      return 4;

    case 0xAB: /* XOR E */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.e);
      return 4;

    case 0xAC: /* XOR H */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.h);
      return 4;

    case 0xAD: /* XOR L */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.l);
      return 4;

    case 0xAE: /* XOR (HL) */
      cpu->regs.a = XOR(cpu, cpu->regs.a, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0xAF: /* XOR A */
      cpu->regs.a = XOR(cpu, cpu->regs.a, cpu->regs.a);
      return 4;

    case 0xB0: /* OR B */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.b);
      return 4;

    case 0xB1: /* OR C */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.c);
      return 4;

    case 0xB2: /* OR D */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.d);
      return 4;

    case 0xB3: /* OR E */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.e);
      return 4;

    case 0xB4: /* OR H */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.h);
      return 4;

    case 0xB5: /* OR L */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.l);
      return 4;

    case 0xB6: /* OR (HL) */
      cpu->regs.a = OR(cpu, cpu->regs.a, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0xB7: /* OR A */
      cpu->regs.a = OR(cpu, cpu->regs.a, cpu->regs.a);
      return 4;

    case 0xB8: /* CP B */
      SUB(cpu, cpu->regs.b);
      return 4;

    case 0xB9: /* CP C */
      SUB(cpu, cpu->regs.c);
      return 4;

    case 0xBA: /* CP D */
      SUB(cpu, cpu->regs.d);
      return 4;

    case 0xBB: /* CP E */
      SUB(cpu, cpu->regs.e);
      return 4;

    case 0xBC: /* CP H */
      SUB(cpu, cpu->regs.h);
      return 4;

    case 0xBD: /* CP L */
      SUB(cpu, cpu->regs.l);
      return 4;

    case 0xBE: /* CP (HL) */
      SUB(cpu, mmuReadByte(mmu, (cpu->regs.h << 8) + cpu->regs.l));
      return 10;

    case 0xBF: /* CP A */
      SUB(cpu, cpu->regs.a);
      return 4;

    case 0xC0: /* RET NZ */
      if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = POP(cpu, mmu); return 20; }
      return 10;

    case 0xC1: /* POP BC */
      word = POP(cpu, mmu); cpu->regs.b = word >> 8; cpu->regs.c = word & 0xFF;
      return 12;

    case 0xC2: /* JP NZ,a16 */
      if (!cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 20; } cpu->regs.pc += 2;
      return 12;

    case 0xC3: /* JP a16 */
      cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc);
      return 16;

    case 0xC4: /* CALL NZ,a16 */
      if (!cpuGetFlag(cpu, FLAG_Z)) { PUSH(cpu, mmu, cpu->regs.pc + 0x2); cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 0x18; } cpu->regs.pc += 2;
      return 12;

    case 0xC5: /* PUSH BC */
      PUSH(cpu, mmu, (cpu->regs.b << 8) + cpu->regs.c);
      return 16;

    case 0xC6: /* ADD A,d8 */
      cpu->regs.a = ADD(cpu, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xC7: /* RST 00H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0;
      return 16;

    case 0xC8: /* RET Z */
      if (cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = POP(cpu, mmu); return 20; }
      return 10;

    case 0xC9: /* RET */
      cpu->regs.pc = POP(cpu, mmu);
      return 16;

    case 0xCA: /* JP Z,a16 */
      if (cpuGetFlag(cpu, FLAG_Z)) { cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 16; } cpu->regs.pc += 2;
      return 12;

    case 0xCB: /* PREFIX CB */
      cpu->cb = true;
      return 4;

    case 0xCC: /* CALL Z,a16 */
      if (cpuGetFlag(cpu, FLAG_Z)) { PUSH(cpu, mmu, cpu->regs.pc + 0x2); cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 0x18; } cpu->regs.pc += 2;
      return 12;

    case 0xCD: /* CALL a16 */
      PUSH(cpu, mmu, cpu->regs.pc + 0x2); cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc);
      return 0x18;

    case 0xCE: /* ADC A,d8 */
      cpu->regs.a = ADC(cpu, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xCF: /* RST 08H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 10;
      return 16;

    case 0xD0: /* RET NC */
      if (!cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = POP(cpu, mmu); return 0x20; }
      return 10;

    case 0xD1: /* POP DE */
      word = POP(cpu, mmu); cpu->regs.d = word >> 8; cpu->regs.e = word & 0xFF;
      return 12;

    case 0xD2: /* JP NC,a16 */
      if (!cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 16; } cpu->regs.pc += 2;
      return 12;

    case 0xD4: /* CALL NC,a16 */
      if (!cpuGetFlag(cpu, FLAG_C)) { PUSH(cpu, mmu, cpu->regs.pc + 0x2); cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 0x18; } cpu->regs.pc += 2;
      return 12;

    case 0xD5: /* PUSH DE */
      PUSH(cpu, mmu, (cpu->regs.d << 8) + cpu->regs.e);
      return 16;

    case 0xD6: /* SUB A,d8 */
      cpu->regs.a = SUB(cpu, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xD7: /* RST 10H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 16;
      return 16;

    case 0xD8: /* RET C */
      if (cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = POP(cpu, mmu); return 20; }
      return 10;

    case 0xD9: /* RETI */
      cpu->regs.pc = POP(cpu, mmu); cpu->ime = true;
      return 16;

    case 0xDA: /* JP C,a16 */
      if (cpuGetFlag(cpu, FLAG_C)) { cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 16; } cpu->regs.pc += 2;
      return 12;

    case 0xDC: /* CALL C,a16 */
      if (cpuGetFlag(cpu, FLAG_C)) { PUSH(cpu, mmu, cpu->regs.pc + 0x2); cpu->regs.pc = mmuReadWord(mmu, cpu->regs.pc); return 0x18; } cpu->regs.pc += 2;
      return 12;

    case 0xDE: /* SBC A,d8 */
      cpu->regs.a = SBC(cpu, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xDF: /* RST 18H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0x18;
      return 16;

    case 0xE0: /* LDH (a8),A */
      mmuWriteByte(mmu, 0xFF00 + mmuReadByte(mmu, cpu->regs.pc), cpu->regs.a); cpu->regs.pc++;
      return 12;

    case 0xE1: /* POP HL */
      word = POP(cpu, mmu); cpu->regs.h = word >> 8; cpu->regs.l = word & 0xFF;
      return 12;

    case 0xE2: /* LD (C),A */
      mmuWriteByte(mmu, 0xFF00 + cpu->regs.c, cpu->regs.a);
      return 10;

    case 0xE5: /* PUSH HL */
      PUSH(cpu, mmu, (cpu->regs.h << 8) + cpu->regs.l);
      return 16;

    case 0xE6: /* AND d8 */
      cpu->regs.a = AND(cpu, cpu->regs.a, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xE7: /* RST 20H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0x20;
      return 16;

    case 0xE8: /* ADD SP,r8 */
    {
      uint8_t data = mmuReadByte(mmu, cpu->regs.pc);

      cpuSetFlag(cpu, FLAG_Z, 0);
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, !!((cpu->regs.sp & 0xF) + (data & 0xF) > 0xF)); // TODO: Implement this carry elsewhere?
      cpuSetFlag(cpu, FLAG_C, !!((cpu->regs.sp & 0xFF) + data > 0xFF));

      cpu->regs.sp = cpu->regs.sp + data;
      cpu->regs.pc++;

      return 16;
    }

    case 0xE9: /* JP (HL) */
      cpu->regs.pc = (cpu->regs.h << 8) + cpu->regs.l;
      return 4;

    case 0xEA: /* LD (a16),A */
      mmuWriteByte(mmu, mmuReadWord(mmu, cpu->regs.pc), cpu->regs.a); cpu->regs.pc += 2;
      return 16;

    case 0xEE: /* XOR d8 */
      cpu->regs.a = XOR(cpu, cpu->regs.a, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xEF: /* RST 28H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0x28;
      return 16;

    case 0xF0: /* LDH A,(a8) */
      cpu->regs.a = mmuReadByte(mmu, 0xFF00 + mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 12;

    case 0xF1: /* POP AF */
      word = POP(cpu, mmu); cpu->regs.a = word >> 8; cpu->regs.f = word & 0xF0;
      return 12;

    case 0xF2: /* LD A,(C) */
      cpu->regs.a = mmuReadByte(mmu, 0xFF00 + cpu->regs.c);
      return 10;

    case 0xF3: /* DI */
      cpu->ime = false;
      return 4;

    case 0xF5: /* PUSH AF */
      PUSH(cpu, mmu, (cpu->regs.a << 8) + cpu->regs.f);
      return 16;

    case 0xF6: /* OR d8 */
      cpu->regs.a = OR(cpu, cpu->regs.a, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xF7: /* RST 30H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0x30;
      return 16;

    case 0xF8: /* LD HL,SP+r8 */
     {
      uint8_t data = mmuReadByte(mmu, cpu->regs.pc);
      uint16_t word = cpu->regs.sp + data;

      cpuSetFlag(cpu, FLAG_Z, 0);
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, !!((cpu->regs.sp & 0xF) + (data & 0xF) > 0xF)); // TODO: Implement this carry elsewhere?
      cpuSetFlag(cpu, FLAG_C, !!((cpu->regs.sp & 0xFF) + data > 0xFF));

      cpu->regs.h = word >> 8;
      cpu->regs.l = word;
      cpu->regs.pc++;

      return 12;
    }

    case 0xF9: /* LD SP,HL */
      cpu->regs.sp = (cpu->regs.h << 8) + cpu->regs.l;
      return 10;

    case 0xFA: /* LD A,(a16) */
      cpu->regs.a = mmuReadByte(mmu, mmuReadWord(mmu, cpu->regs.pc)); cpu->regs.pc += 2;
      return 16;

    case 0xFB: /* EI */
      cpu->ei = true;
      return 4;

    case 0xFE: /* CP d8 */
      SUB(cpu, mmuReadByte(mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 10;

    case 0xFF: /* RST 38H */
      PUSH(cpu, mmu, cpu->regs.pc); cpu->regs.pc = 0x38;
      return 16;
  }

  return 0;
}

uint8_t cpuOpcodeCB(CPU *cpu, MMU *mmu, uint8_t opcode)
{
  uint16_t word;

  switch (opcode) {
    case 0x0: /* RLC B */
      cpu->regs.b = RLC(cpu, cpu->regs.b);
      return 10;

    case 0x1: /* RLC C */
      cpu->regs.c = RLC(cpu, cpu->regs.c);
      return 10;

    case 0x2: /* RLC D */
      cpu->regs.d = RLC(cpu, cpu->regs.d);
      return 10;

    case 0x3: /* RLC E */
      cpu->regs.e = RLC(cpu, cpu->regs.e);
      return 10;

    case 0x4: /* RLC H */
      cpu->regs.h = RLC(cpu, cpu->regs.h);
      return 10;

    case 0x5: /* RLC L */
      cpu->regs.l = RLC(cpu, cpu->regs.l);
      return 10;

    case 0x6: /* RLC (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RLC(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x7: /* RLC A */
      cpu->regs.a = RLC(cpu, cpu->regs.a);
      return 10;

    case 0x8: /* RRC B */
      cpu->regs.b = RRC(cpu, cpu->regs.b);
      return 10;

    case 0x9: /* RRC C */
      cpu->regs.c = RRC(cpu, cpu->regs.c);
      return 10;

    case 0xA: /* RRC D */
      cpu->regs.d = RRC(cpu, cpu->regs.d);
      return 10;

    case 0xB: /* RRC E */
      cpu->regs.e = RRC(cpu, cpu->regs.e);
      return 10;

    case 0xC: /* RRC H */
      cpu->regs.h = RRC(cpu, cpu->regs.h);
      return 10;

    case 0xD: /* RRC L */
      cpu->regs.l = RRC(cpu, cpu->regs.l);
      return 10;

    case 0xE: /* RRC (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RRC(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0xF: /* RRC A */
      cpu->regs.a = RRC(cpu, cpu->regs.a);
      return 10;

    case 0x10: /* RL B */
      cpu->regs.b = RL(cpu, cpu->regs.b);
      return 10;

    case 0x11: /* RL C */
      cpu->regs.c = RL(cpu, cpu->regs.c);
      return 10;

    case 0x12: /* RL D */
      cpu->regs.d = RL(cpu, cpu->regs.d);
      return 10;

    case 0x13: /* RL E */
      cpu->regs.e = RL(cpu, cpu->regs.e);
      return 10;

    case 0x14: /* RL H */
      cpu->regs.h = RL(cpu, cpu->regs.h);
      return 10;

    case 0x15: /* RL L */
      cpu->regs.l = RL(cpu, cpu->regs.l);
      return 10;

    case 0x16: /* RL (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RL(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x17: /* RL A */
      cpu->regs.a = RL(cpu, cpu->regs.a);
      return 10;

    case 0x18: /* RR B */
      cpu->regs.b = RR(cpu, cpu->regs.b);
      return 10;

    case 0x19: /* RR C */
      cpu->regs.c = RR(cpu, cpu->regs.c);
      return 10;

    case 0x1A: /* RR D */
      cpu->regs.d = RR(cpu, cpu->regs.d);
      return 10;

    case 0x1B: /* RR E */
      cpu->regs.e = RR(cpu, cpu->regs.e);
      return 10;

    case 0x1C: /* RR H */
      cpu->regs.h = RR(cpu, cpu->regs.h);
      return 10;

    case 0x1D: /* RR L */
      cpu->regs.l = RR(cpu, cpu->regs.l);
      return 10;

    case 0x1E: /* RR (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RR(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x1F: /* RR A */
      cpu->regs.a = RR(cpu, cpu->regs.a);
      return 10;

    case 0x20: /* SLA B */
      cpu->regs.b = SLA(cpu, cpu->regs.b);
      return 10;

    case 0x21: /* SLA C */
      cpu->regs.c = SLA(cpu, cpu->regs.c);
      return 10;

    case 0x22: /* SLA D */
      cpu->regs.d = SLA(cpu, cpu->regs.d);
      return 10;

    case 0x23: /* SLA E */
      cpu->regs.e = SLA(cpu, cpu->regs.e);
      return 10;

    case 0x24: /* SLA H */
      cpu->regs.h = SLA(cpu, cpu->regs.h);
      return 10;

    case 0x25: /* SLA L */
      cpu->regs.l = SLA(cpu, cpu->regs.l);
      return 10;

    case 0x26: /* SLA (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SLA(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x27: /* SLA A */
      cpu->regs.a = SLA(cpu, cpu->regs.a);
      return 10;

    case 0x28: /* SRA B */
      cpu->regs.b = SRA(cpu, cpu->regs.b);
      return 10;

    case 0x29: /* SRA C */
      cpu->regs.c = SRA(cpu, cpu->regs.c);
      return 10;

    case 0x2A: /* SRA D */
      cpu->regs.d = SRA(cpu, cpu->regs.d);
      return 10;

    case 0x2B: /* SRA E */
      cpu->regs.e = SRA(cpu, cpu->regs.e);
      return 10;

    case 0x2C: /* SRA H */
      cpu->regs.h = SRA(cpu, cpu->regs.h);
      return 10;

    case 0x2D: /* SRA L */
      cpu->regs.l = SRA(cpu, cpu->regs.l);
      return 10;

    case 0x2E: /* SRA (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SRA(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x2F: /* SRA A */
      cpu->regs.a = SRA(cpu, cpu->regs.a);
      return 10;

    case 0x30: /* SWAP B */
      cpu->regs.b = SWAP(cpu, cpu->regs.b);
      return 10;

    case 0x31: /* SWAP C */
      cpu->regs.c = SWAP(cpu, cpu->regs.c);
      return 10;

    case 0x32: /* SWAP D */
      cpu->regs.d = SWAP(cpu, cpu->regs.d);
      return 10;

    case 0x33: /* SWAP E */
      cpu->regs.e = SWAP(cpu, cpu->regs.e);
      return 10;

    case 0x34: /* SWAP H */
      cpu->regs.h = SWAP(cpu, cpu->regs.h);
      return 10;

    case 0x35: /* SWAP L */
      cpu->regs.l = SWAP(cpu, cpu->regs.l);
      return 10;

    case 0x36: /* SWAP (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SWAP(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x37: /* SWAP A */
      cpu->regs.a = SWAP(cpu, cpu->regs.a);
      return 10;

    case 0x38: /* SRL B */
      cpu->regs.b = SRL(cpu, cpu->regs.b);
      return 10;

    case 0x39: /* SRL C */
      cpu->regs.c = SRL(cpu, cpu->regs.c);
      return 10;

    case 0x3A: /* SRL D */
      cpu->regs.d = SRL(cpu, cpu->regs.d);
      return 10;

    case 0x3B: /* SRL E */
      cpu->regs.e = SRL(cpu, cpu->regs.e);
      return 10;

    case 0x3C: /* SRL H */
      cpu->regs.h = SRL(cpu, cpu->regs.h);
      return 10;

    case 0x3D: /* SRL L */
      cpu->regs.l = SRL(cpu, cpu->regs.l);
      return 10;

    case 0x3E: /* SRL (HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SRL(cpu, mmuReadByte(mmu, word)));
      return 16;

    case 0x3F: /* SRL A */
      cpu->regs.a = SRL(cpu, cpu->regs.a);
      return 10;

    case 0x40: /* BIT 0,B */
      BIT(cpu, cpu->regs.b, 0);
      return 10;

    case 0x41: /* BIT 0,C */
      BIT(cpu, cpu->regs.c, 0);
      return 10;

    case 0x42: /* BIT 0,D */
      BIT(cpu, cpu->regs.d, 0);
      return 10;

    case 0x43: /* BIT 0,E */
      BIT(cpu, cpu->regs.e, 0);
      return 10;

    case 0x44: /* BIT 0,H */
      BIT(cpu, cpu->regs.h, 0);
      return 10;

    case 0x45: /* BIT 0,L */
      BIT(cpu, cpu->regs.l, 0);
      return 10;

    case 0x46: /* BIT 0,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 0);
      return 16;

    case 0x47: /* BIT 0,A */
      BIT(cpu, cpu->regs.a, 0);
      return 10;

    case 0x48: /* BIT 1,B */
      BIT(cpu, cpu->regs.b, 1);
      return 10;

    case 0x49: /* BIT 1,C */
      BIT(cpu, cpu->regs.c, 1);
      return 10;

    case 0x4A: /* BIT 1,D */
      BIT(cpu, cpu->regs.d, 1);
      return 10;

    case 0x4B: /* BIT 1,E */
      BIT(cpu, cpu->regs.e, 1);
      return 10;

    case 0x4C: /* BIT 1,H */
      BIT(cpu, cpu->regs.h, 1);
      return 10;

    case 0x4D: /* BIT 1,L */
      BIT(cpu, cpu->regs.l, 1);
      return 10;

    case 0x4E: /* BIT 1,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 1);
      return 16;

    case 0x4F: /* BIT 1,A */
      BIT(cpu, cpu->regs.a, 1);
      return 10;

    case 0x50: /* BIT 2,B */
      BIT(cpu, cpu->regs.b, 2);
      return 10;

    case 0x51: /* BIT 2,C */
      BIT(cpu, cpu->regs.c, 2);
      return 10;

    case 0x52: /* BIT 2,D */
      BIT(cpu, cpu->regs.d, 2);
      return 10;

    case 0x53: /* BIT 2,E */
      BIT(cpu, cpu->regs.e, 2);
      return 10;

    case 0x54: /* BIT 2,H */
      BIT(cpu, cpu->regs.h, 2);
      return 10;

    case 0x55: /* BIT 2,L */
      BIT(cpu, cpu->regs.l, 2);
      return 10;

    case 0x56: /* BIT 2,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 2);
      return 16;

    case 0x57: /* BIT 2,A */
      BIT(cpu, cpu->regs.a, 2);
      return 10;

    case 0x58: /* BIT 3,B */
      BIT(cpu, cpu->regs.b, 3);
      return 10;

    case 0x59: /* BIT 3,C */
      BIT(cpu, cpu->regs.c, 3);
      return 10;

    case 0x5A: /* BIT 3,D */
      BIT(cpu, cpu->regs.d, 3);
      return 10;

    case 0x5B: /* BIT 3,E */
      BIT(cpu, cpu->regs.e, 3);
      return 10;

    case 0x5C: /* BIT 3,H */
      BIT(cpu, cpu->regs.h, 3);
      return 10;

    case 0x5D: /* BIT 3,L */
      BIT(cpu, cpu->regs.l, 3);
      return 10;

    case 0x5E: /* BIT 3,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 3);
      return 16;

    case 0x5F: /* BIT 3,A */
      BIT(cpu, cpu->regs.a, 3);
      return 10;

    case 0x60: /* BIT 4,B */
      BIT(cpu, cpu->regs.b, 4);
      return 10;

    case 0x61: /* BIT 4,C */
      BIT(cpu, cpu->regs.c, 4);
      return 10;

    case 0x62: /* BIT 4,D */
      BIT(cpu, cpu->regs.d, 4);
      return 10;

    case 0x63: /* BIT 4,E */
      BIT(cpu, cpu->regs.e, 4);
      return 10;

    case 0x64: /* BIT 4,H */
      BIT(cpu, cpu->regs.h, 4);
      return 10;

    case 0x65: /* BIT 4,L */
      BIT(cpu, cpu->regs.l, 4);
      return 10;

    case 0x66: /* BIT 4,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 4);
      return 16;

    case 0x67: /* BIT 4,A */
      BIT(cpu, cpu->regs.a, 4);
      return 10;

    case 0x68: /* BIT 5,B */
      BIT(cpu, cpu->regs.b, 5);
      return 10;

    case 0x69: /* BIT 5,C */
      BIT(cpu, cpu->regs.c, 5);
      return 10;

    case 0x6A: /* BIT 5,D */
      BIT(cpu, cpu->regs.d, 5);
      return 10;

    case 0x6B: /* BIT 5,E */
      BIT(cpu, cpu->regs.e, 5);
      return 10;

    case 0x6C: /* BIT 5,H */
      BIT(cpu, cpu->regs.h, 5);
      return 10;

    case 0x6D: /* BIT 5,L */
      BIT(cpu, cpu->regs.l, 5);
      return 10;

    case 0x6E: /* BIT 5,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 5);
      return 16;

    case 0x6F: /* BIT 5,A */
      BIT(cpu, cpu->regs.a, 5);
      return 10;

    case 0x70: /* BIT 6,B */
      BIT(cpu, cpu->regs.b, 6);
      return 10;

    case 0x71: /* BIT 6,C */
      BIT(cpu, cpu->regs.c, 6);
      return 10;

    case 0x72: /* BIT 6,D */
      BIT(cpu, cpu->regs.d, 6);
      return 10;

    case 0x73: /* BIT 6,E */
      BIT(cpu, cpu->regs.e, 6);
      return 10;

    case 0x74: /* BIT 6,H */
      BIT(cpu, cpu->regs.h, 6);
      return 10;

    case 0x75: /* BIT 6,L */
      BIT(cpu, cpu->regs.l, 6);
      return 10;

    case 0x76: /* BIT 6,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 6);
      return 16;

    case 0x77: /* BIT 6,A */
      BIT(cpu, cpu->regs.a, 6);
      return 10;

    case 0x78: /* BIT 7,B */
      BIT(cpu, cpu->regs.b, 7);
      return 10;

    case 0x79: /* BIT 7,C */
      BIT(cpu, cpu->regs.c, 7);
      return 10;

    case 0x7A: /* BIT 7,D */
      BIT(cpu, cpu->regs.d, 7);
      return 10;

    case 0x7B: /* BIT 7,E */
      BIT(cpu, cpu->regs.e, 7);
      return 10;

    case 0x7C: /* BIT 7,H */
      BIT(cpu, cpu->regs.h, 7);
      return 10;

    case 0x7D: /* BIT 7,L */
      BIT(cpu, cpu->regs.l, 7);
      return 10;

    case 0x7E: /* BIT 7,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; BIT(cpu, mmuReadByte(mmu, word), 7);
      return 16;

    case 0x7F: /* BIT 7,A */
      BIT(cpu, cpu->regs.a, 7);
      return 10;

    case 0x80: /* RES 0,B */
      cpu->regs.b = RES(cpu->regs.b, 0);
      return 10;

    case 0x81: /* RES 0,C */
      cpu->regs.c = RES(cpu->regs.c, 0);
      return 10;

    case 0x82: /* RES 0,D */
      cpu->regs.d = RES(cpu->regs.d, 0);
      return 10;

    case 0x83: /* RES 0,E */
      cpu->regs.e = RES(cpu->regs.e, 0);
      return 10;

    case 0x84: /* RES 0,H */
      cpu->regs.h = RES(cpu->regs.h, 0);
      return 10;

    case 0x85: /* RES 0,L */
      cpu->regs.l = RES(cpu->regs.l, 0);
      return 10;

    case 0x86: /* RES 0,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 0));
      return 16;

    case 0x87: /* RES 0,A */
      cpu->regs.a = RES(cpu->regs.a, 0);
      return 10;

    case 0x88: /* RES 1,B */
      cpu->regs.b = RES(cpu->regs.b, 1);
      return 10;

    case 0x89: /* RES 1,C */
      cpu->regs.c = RES(cpu->regs.c, 1);
      return 10;

    case 0x8A: /* RES 1,D */
      cpu->regs.d = RES(cpu->regs.d, 1);
      return 10;

    case 0x8B: /* RES 1,E */
      cpu->regs.e = RES(cpu->regs.e, 1);
      return 10;

    case 0x8C: /* RES 1,H */
      cpu->regs.h = RES(cpu->regs.h, 1);
      return 10;

    case 0x8D: /* RES 1,L */
      cpu->regs.l = RES(cpu->regs.l, 1);
      return 10;

    case 0x8E: /* RES 1,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 1));
      return 16;

    case 0x8F: /* RES 1,A */
      cpu->regs.a = RES(cpu->regs.a, 1);
      return 10;

    case 0x90: /* RES 2,B */
      cpu->regs.b = RES(cpu->regs.b, 2);
      return 10;

    case 0x91: /* RES 2,C */
      cpu->regs.c = RES(cpu->regs.c, 2);
      return 10;

    case 0x92: /* RES 2,D */
      cpu->regs.d = RES(cpu->regs.d, 2);
      return 10;

    case 0x93: /* RES 2,E */
      cpu->regs.e = RES(cpu->regs.e, 2);
      return 10;

    case 0x94: /* RES 2,H */
      cpu->regs.h = RES(cpu->regs.h, 2);
      return 10;

    case 0x95: /* RES 2,L */
      cpu->regs.l = RES(cpu->regs.l, 2);
      return 10;

    case 0x96: /* RES 2,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 2));
      return 16;

    case 0x97: /* RES 2,A */
      cpu->regs.a = RES(cpu->regs.a, 2);
      return 10;

    case 0x98: /* RES 3,B */
      cpu->regs.b = RES(cpu->regs.b, 3);
      return 10;

    case 0x99: /* RES 3,C */
      cpu->regs.c = RES(cpu->regs.c, 3);
      return 10;

    case 0x9A: /* RES 3,D */
      cpu->regs.d = RES(cpu->regs.d, 3);
      return 10;

    case 0x9B: /* RES 3,E */
      cpu->regs.e = RES(cpu->regs.e, 3);
      return 10;

    case 0x9C: /* RES 3,H */
      cpu->regs.h = RES(cpu->regs.h, 3);
      return 10;

    case 0x9D: /* RES 3,L */
      cpu->regs.l = RES(cpu->regs.l, 3);
      return 10;

    case 0x9E: /* RES 3,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 3));
      return 16;

    case 0x9F: /* RES 3,A */
      cpu->regs.a = RES(cpu->regs.a, 3);
      return 10;

    case 0xA0: /* RES 4,B */
      cpu->regs.b = RES(cpu->regs.b, 4);
      return 10;

    case 0xA1: /* RES 4,C */
      cpu->regs.c = RES(cpu->regs.c, 4);
      return 10;

    case 0xA2: /* RES 4,D */
      cpu->regs.d = RES(cpu->regs.d, 4);
      return 10;

    case 0xA3: /* RES 4,E */
      cpu->regs.e = RES(cpu->regs.e, 4);
      return 10;

    case 0xA4: /* RES 4,H */
      cpu->regs.h = RES(cpu->regs.h, 4);
      return 10;

    case 0xA5: /* RES 4,L */
      cpu->regs.l = RES(cpu->regs.l, 4);
      return 10;

    case 0xA6: /* RES 4,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 4));
      return 16;

    case 0xA7: /* RES 4,A */
      cpu->regs.a = RES(cpu->regs.a, 4);
      return 10;

    case 0xA8: /* RES 5,B */
      cpu->regs.b = RES(cpu->regs.b, 5);
      return 10;

    case 0xA9: /* RES 5,C */
      cpu->regs.c = RES(cpu->regs.c, 5);
      return 10;

    case 0xAA: /* RES 5,D */
      cpu->regs.d = RES(cpu->regs.d, 5);
      return 10;

    case 0xAB: /* RES 5,E */
      cpu->regs.e = RES(cpu->regs.e, 5);
      return 10;

    case 0xAC: /* RES 5,H */
      cpu->regs.h = RES(cpu->regs.h, 5);
      return 10;

    case 0xAD: /* RES 5,L */
      cpu->regs.l = RES(cpu->regs.l, 5);
      return 10;

    case 0xAE: /* RES 5,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 5));
      return 16;

    case 0xAF: /* RES 5,A */
      cpu->regs.a = RES(cpu->regs.a, 5);
      return 10;

    case 0xB0: /* RES 6,B */
      cpu->regs.b = RES(cpu->regs.b, 6);
      return 10;

    case 0xB1: /* RES 6,C */
      cpu->regs.c = RES(cpu->regs.c, 6);
      return 10;

    case 0xB2: /* RES 6,D */
      cpu->regs.d = RES(cpu->regs.d, 6);
      return 10;

    case 0xB3: /* RES 6,E */
      cpu->regs.e = RES(cpu->regs.e, 6);
      return 10;

    case 0xB4: /* RES 6,H */
      cpu->regs.h = RES(cpu->regs.h, 6);
      return 10;

    case 0xB5: /* RES 6,L */
      cpu->regs.l = RES(cpu->regs.l, 6);
      return 10;

    case 0xB6: /* RES 6,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 6));
      return 16;

    case 0xB7: /* RES 6,A */
      cpu->regs.a = RES(cpu->regs.a, 6);
      return 10;

    case 0xB8: /* RES 7,B */
      cpu->regs.b = RES(cpu->regs.b, 7);
      return 10;

    case 0xB9: /* RES 7,C */
      cpu->regs.c = RES(cpu->regs.c, 7);
      return 10;

    case 0xBA: /* RES 7,D */
      cpu->regs.d = RES(cpu->regs.d, 7);
      return 10;

    case 0xBB: /* RES 7,E */
      cpu->regs.e = RES(cpu->regs.e, 7);
      return 10;

    case 0xBC: /* RES 7,H */
      cpu->regs.h = RES(cpu->regs.h, 7);
      return 10;

    case 0xBD: /* RES 7,L */
      cpu->regs.l = RES(cpu->regs.l, 7);
      return 10;

    case 0xBE: /* RES 7,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, RES(mmuReadByte(mmu, word), 7));
      return 16;

    case 0xBF: /* RES 7,A */
      cpu->regs.a = RES(cpu->regs.a, 7);
      return 10;

    case 0xC0: /* SET 0,B */
      cpu->regs.b = SET(cpu->regs.b, 0);
      return 10;

    case 0xC1: /* SET 0,C */
      cpu->regs.c = SET(cpu->regs.c, 0);
      return 10;

    case 0xC2: /* SET 0,D */
      cpu->regs.d = SET(cpu->regs.d, 0);
      return 10;

    case 0xC3: /* SET 0,E */
      cpu->regs.e = SET(cpu->regs.e, 0);
      return 10;

    case 0xC4: /* SET 0,H */
      cpu->regs.h = SET(cpu->regs.h, 0);
      return 10;

    case 0xC5: /* SET 0,L */
      cpu->regs.l = SET(cpu->regs.l, 0);
      return 10;

    case 0xC6: /* SET 0,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 0));
      return 16;

    case 0xC7: /* SET 0,A */
      cpu->regs.a = SET(cpu->regs.a, 0);
      return 10;

    case 0xC8: /* SET 1,B */
      cpu->regs.b = SET(cpu->regs.b, 1);
      return 10;

    case 0xC9: /* SET 1,C */
      cpu->regs.c = SET(cpu->regs.c, 1);
      return 10;

    case 0xCA: /* SET 1,D */
      cpu->regs.d = SET(cpu->regs.d, 1);
      return 10;

    case 0xCB: /* SET 1,E */
      cpu->regs.e = SET(cpu->regs.e, 1);
      return 10;

    case 0xCC: /* SET 1,H */
      cpu->regs.h = SET(cpu->regs.h, 1);
      return 10;

    case 0xCD: /* SET 1,L */
      cpu->regs.l = SET(cpu->regs.l, 1);
      return 10;

    case 0xCE: /* SET 1,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 1));
      return 16;

    case 0xCF: /* SET 1,A */
      cpu->regs.a = SET(cpu->regs.a, 1);
      return 10;

    case 0xD0: /* SET 2,B */
      cpu->regs.b = SET(cpu->regs.b, 2);
      return 10;

    case 0xD1: /* SET 2,C */
      cpu->regs.c = SET(cpu->regs.c, 2);
      return 10;

    case 0xD2: /* SET 2,D */
      cpu->regs.d = SET(cpu->regs.d, 2);
      return 10;

    case 0xD3: /* SET 2,E */
      cpu->regs.e = SET(cpu->regs.e, 2);
      return 10;

    case 0xD4: /* SET 2,H */
      cpu->regs.h = SET(cpu->regs.h, 2);
      return 10;

    case 0xD5: /* SET 2,L */
      cpu->regs.l = SET(cpu->regs.l, 2);
      return 10;

    case 0xD6: /* SET 2,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 2));
      return 16;

    case 0xD7: /* SET 2,A */
      cpu->regs.a = SET(cpu->regs.a, 2);
      return 10;

    case 0xD8: /* SET 3,B */
      cpu->regs.b = SET(cpu->regs.b, 3);
      return 10;

    case 0xD9: /* SET 3,C */
      cpu->regs.c = SET(cpu->regs.c, 3);
      return 10;

    case 0xDA: /* SET 3,D */
      cpu->regs.d = SET(cpu->regs.d, 3);
      return 10;

    case 0xDB: /* SET 3,E */
      cpu->regs.e = SET(cpu->regs.e, 3);
      return 10;

    case 0xDC: /* SET 3,H */
      cpu->regs.h = SET(cpu->regs.h, 3);
      return 10;

    case 0xDD: /* SET 3,L */
      cpu->regs.l = SET(cpu->regs.l, 3);
      return 10;

    case 0xDE: /* SET 3,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 3));
      return 16;

    case 0xDF: /* SET 3,A */
      cpu->regs.a = SET(cpu->regs.a, 3);
      return 10;

    case 0xE0: /* SET 4,B */
      cpu->regs.b = SET(cpu->regs.b, 4);
      return 10;

    case 0xE1: /* SET 4,C */
      cpu->regs.c = SET(cpu->regs.c, 4);
      return 10;

    case 0xE2: /* SET 4,D */
      cpu->regs.d = SET(cpu->regs.d, 4);
      return 10;

    case 0xE3: /* SET 4,E */
      cpu->regs.e = SET(cpu->regs.e, 4);
      return 10;

    case 0xE4: /* SET 4,H */
      cpu->regs.h = SET(cpu->regs.h, 4);
      return 10;

    case 0xE5: /* SET 4,L */
      cpu->regs.l = SET(cpu->regs.l, 4);
      return 10;

    case 0xE6: /* SET 4,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 4));
      return 16;

    case 0xE7: /* SET 4,A */
      cpu->regs.a = SET(cpu->regs.a, 4);
      return 10;

    case 0xE8: /* SET 5,B */
      cpu->regs.b = SET(cpu->regs.b, 5);
      return 10;

    case 0xE9: /* SET 5,C */
      cpu->regs.c = SET(cpu->regs.c, 5);
      return 10;

    case 0xEA: /* SET 5,D */
      cpu->regs.d = SET(cpu->regs.d, 5);
      return 10;

    case 0xEB: /* SET 5,E */
      cpu->regs.e = SET(cpu->regs.e, 5);
      return 10;

    case 0xEC: /* SET 5,H */
      cpu->regs.h = SET(cpu->regs.h, 5);
      return 10;

    case 0xED: /* SET 5,L */
      cpu->regs.l = SET(cpu->regs.l, 5);
      return 10;

    case 0xEE: /* SET 5,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 5));
      return 16;

    case 0xEF: /* SET 5,A */
      cpu->regs.a = SET(cpu->regs.a, 5);
      return 10;

    case 0xF0: /* SET 6,B */
      cpu->regs.b = SET(cpu->regs.b, 6);
      return 10;

    case 0xF1: /* SET 6,C */
      cpu->regs.c = SET(cpu->regs.c, 6);
      return 10;

    case 0xF2: /* SET 6,D */
      cpu->regs.d = SET(cpu->regs.d, 6);
      return 10;

    case 0xF3: /* SET 6,E */
      cpu->regs.e = SET(cpu->regs.e, 6);
      return 10;

    case 0xF4: /* SET 6,H */
      cpu->regs.h = SET(cpu->regs.h, 6);
      return 10;

    case 0xF5: /* SET 6,L */
      cpu->regs.l = SET(cpu->regs.l, 6);
      return 10;

    case 0xF6: /* SET 6,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 6));
      return 16;

    case 0xF7: /* SET 6,A */
      cpu->regs.a = SET(cpu->regs.a, 6);
      return 10;

    case 0xF8: /* SET 7,B */
      cpu->regs.b = SET(cpu->regs.b, 7);
      return 10;

    case 0xF9: /* SET 7,C */
      cpu->regs.c = SET(cpu->regs.c, 7);
      return 10;

    case 0xFA: /* SET 7,D */
      cpu->regs.d = SET(cpu->regs.d, 7);
      return 10;

    case 0xFB: /* SET 7,E */
      cpu->regs.e = SET(cpu->regs.e, 7);
      return 10;

    case 0xFC: /* SET 7,H */
      cpu->regs.h = SET(cpu->regs.h, 7);
      return 10;

    case 0xFD: /* SET 7,L */
      cpu->regs.l = SET(cpu->regs.l, 7);
      return 10;

    case 0xFE: /* SET 7,(HL) */
      word = (cpu->regs.h << 8) + cpu->regs.l; mmuWriteByte(mmu, word, SET(mmuReadByte(mmu, word), 7));
      return 16;

    case 0xFF: /* SET 7,A */
      cpu->regs.a = SET(cpu->regs.a, 7);
      return 10;
  }

  return 0;
}
