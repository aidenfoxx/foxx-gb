#include "opcode.h"

static uint8_t cpuOpcode(CPU*, uint8_t);
static uint8_t cpuOpcodeCB(CPU*, uint8_t);

#define DEF_LD_B_B(A, B)\
static inline uint8_t LD_##A##_##B(CPU *cpu)\
{\
  cpu->regs.A = cpu->regs.B;\
  return 4;\
}

#define DEF_LD_B_P(A, B, C)\
static inline uint8_t LD_##A##_##B##C(CPU *cpu)\
{\
  cpu->regs.A = mmuReadByte(cpu->mmu, (cpu->regs.B << 8) + cpu->regs.C);\
  return 8;\
}

#define DEF_LD_P_B(A, B, C)\
static inline uint8_t LD_##A##B##_##C(CPU *cpu)\
{\
  mmuWriteByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B, cpu->regs.C);\
  return 8;\
}

DEF_LD_B_B(b, c)
DEF_LD_B_B(b, d)
DEF_LD_B_B(b, e)
DEF_LD_B_B(b, h)
DEF_LD_B_B(b, l)
DEF_LD_B_P(b, h, l)
DEF_LD_B_B(b, a)

DEF_LD_B_B(c, b)
DEF_LD_B_B(c, d)
DEF_LD_B_B(c, e)
DEF_LD_B_B(c, h)
DEF_LD_B_B(c, l)
DEF_LD_B_P(c, h, l)
DEF_LD_B_B(c, a)

DEF_LD_B_B(d, b)
DEF_LD_B_B(d, c)
DEF_LD_B_B(d, e)
DEF_LD_B_B(d, h)
DEF_LD_B_B(d, l)
DEF_LD_B_P(d, h, l)
DEF_LD_B_B(d, a)

DEF_LD_B_B(e, b)
DEF_LD_B_B(e, c)
DEF_LD_B_B(e, d)
DEF_LD_B_B(e, h)
DEF_LD_B_B(e, l)
DEF_LD_B_P(e, h, l)
DEF_LD_B_B(e, a)

DEF_LD_B_B(h, b)
DEF_LD_B_B(h, c)
DEF_LD_B_B(h, d)
DEF_LD_B_B(h, e)
DEF_LD_B_B(h, l)
DEF_LD_B_P(h, h, l)
DEF_LD_B_B(h, a)

DEF_LD_B_B(l, b)
DEF_LD_B_B(l, c)
DEF_LD_B_B(l, d)
DEF_LD_B_B(l, e)
DEF_LD_B_B(l, h)
DEF_LD_B_P(l, h, l)
DEF_LD_B_B(l, a)

DEF_LD_P_B(h, l, a)
DEF_LD_P_B(h, l, b)
DEF_LD_P_B(h, l, c)
DEF_LD_P_B(h, l, d)
DEF_LD_P_B(h, l, e)
DEF_LD_P_B(h, l, h)
DEF_LD_P_B(h, l, l)

DEF_LD_B_B(a, b)
DEF_LD_B_B(a, c)
DEF_LD_B_B(a, d)
DEF_LD_B_B(a, e)
DEF_LD_B_B(a, h)
DEF_LD_B_B(a, l)
DEF_LD_B_P(a, h, l)

inline static uint8_t NOP()
{
  return 4;
}

inline static uint8_t HALT(CPU *cpu)
{
  cpu->halt = true;
  return 4;
}

/**
 * Arithmetic Operations
 */
inline static uint8_t ADD_WW(CPU *cpu, uint16_t op)
{
  uint16_t hl = (cpu->regs.h << 8) + cpu->regs.l;
  uint32_t result = hl + op;

  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, (hl & 0xFFF) + (op & 0xFFF) > 0xFFF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFFFF);

  cpu->regs.h = result >> 8;
  cpu->regs.l = result;

  return 8;
}

inline static uint8_t ADD(CPU *cpu, uint8_t op)
{
  uint16_t result = cpu->regs.a + op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) + (op & 0xF) > 0xF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFF);

  cpu->regs.a = result;

	return 4;
}

inline static uint8_t ADC(CPU *cpu, uint8_t op)
{
  uint8_t carry = cpuGetFlag(cpu, FLAG_C);
  uint16_t result = cpu->regs.a + op + carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) + (op & 0xF) + carry > 0xF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFF);

  cpu->regs.a = result;

	return 4;
}

inline static uint8_t SUB(CPU *cpu, uint8_t op)
{
  int16_t result = cpu->regs.a - op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, 1);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (op & 0xF) < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  cpu->regs.a = result;

  return 4;
}

inline static uint8_t SBC(CPU *cpu, uint8_t op)
{
  uint8_t carry = cpuGetFlag(cpu, FLAG_C);
  int16_t result = cpu->regs.a - op - carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, 1);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (op & 0xF) - carry < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  cpu->regs.a = result;

  return 4;
}

inline static uint8_t CP(CPU *cpu, uint8_t op)
{
  int16_t result = cpu->regs.a - op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, 1);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (op & 0xF) < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  return 4;
}

/* TODO: Do these after macro. */
inline static uint8_t INC(CPU *cpu, uint8_t op)
{
  cpuSetFlag(cpu, FLAG_Z, !(++op));
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, !(op & 0xF));

  return op;
}

inline static uint16_t INC_W(uint16_t op)
{
	return ++op;
}

inline static uint8_t DEC(CPU *cpu, uint8_t op)
{
  uint8_t result = op - 1;

	cpuSetFlag(cpu, FLAG_Z, !result);
	cpuSetFlag(cpu, FLAG_N, 1);
	cpuSetFlag(cpu, FLAG_H, (result & 0xF) == 0xF);

	return result;
}

inline static uint16_t DEC_W(uint16_t op)
{
	return --op;
}

/**
 * Binary Operations
 */
inline static uint8_t AND(CPU *cpu, uint8_t op)
{
  cpu->regs.a &= op;
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 1);
  cpuSetFlag(cpu, FLAG_C, 0);

  return 4;
}

inline static uint8_t OR(CPU *cpu, uint8_t op)
{
  cpu->regs.a |= op;
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);
  cpuSetFlag(cpu, FLAG_C, 0);

  return 4;
}

inline static uint8_t XOR(CPU *cpu, uint8_t op)
{
  cpu->regs.a ^= op;
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);
  cpuSetFlag(cpu, FLAG_C, 0);

  return 4;
}

/**
 * Stack Operations
 */
inline static void PUSH(CPU *cpu, uint16_t val)
{
	cpu->regs.sp -= 2;
	mmuWriteWord(cpu->mmu, cpu->regs.sp, val);
}

inline static uint16_t POP(CPU *cpu)
{
	uint16_t result = mmuReadWord(cpu->mmu, cpu->regs.sp);
	cpu->regs.sp += 2;

	return result;
}

/**
 * Other Operations
 */
inline static uint8_t SWAP(CPU *cpu, uint8_t a)
{
  uint8_t result = (a >> 4) | (a << 4);
  cpuSetFlag(cpu, FLAG_Z, !result);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);
  cpuSetFlag(cpu, FLAG_C, 0);

  return result;
}

inline static uint8_t RLC(CPU *cpu, uint8_t a)
{
  uint8_t r = (a & 0x80) >> 7;
  cpuSetFlag(cpu, FLAG_C, r);
  a = (a << 1) + r;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t RL(CPU *cpu, uint8_t a)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);
  cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7);
  a = (a << 1) + c;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t RRC(CPU *cpu, uint8_t a)
{
  uint8_t r = a & 0x1; cpuSetFlag(cpu, FLAG_C, r);
  a = (a >> 1) + (r << 7);
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t RR(CPU *cpu, uint8_t a)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);
  cpuSetFlag(cpu, FLAG_C, a & 0x1); a = (a >> 1) + (c << 7);
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t SLA(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, (a & 0x80) >> 7);
  a <<= 1;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t SRA(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, a & 0x1);
  a >>= 1;
  a |= ((a & 0x40) << 1);
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static uint8_t SRL(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, a & 0x1);
  a >>= 1;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 0);

  return a;
}

inline static void BIT(CPU *cpu, uint8_t a, uint8_t b) {
  cpuSetFlag(cpu, FLAG_Z, !(a & (1 << b)));
  cpuSetFlag(cpu, FLAG_N, 0);
  cpuSetFlag(cpu, FLAG_H, 1);
}

inline static uint8_t SET(uint8_t a, uint8_t b)
{
  return a | (1 << b);
}

inline static uint8_t RES(uint8_t a, uint8_t b)
{
  return a & ~(1 << b);
}

inline static uint16_t JR(uint16_t pc, uint8_t a)
{
  if ((a & 0x80) == 0x80) {
    a = ~(a - 0x1);
    return pc - a;
  }

  return pc + a;
}

uint8_t cpuExec(CPU *cpu)
{
  uint8_t op = mmuReadByte(cpu->mmu, cpu->regs.pc);
  cpu->regs.pc++;

  if (cpu->cb) {
    cpu->cb = false;
    return cpuOpcodeCB(cpu, op);
  }

  return cpuOpcode(cpu, op);
}

uint8_t cpuOpcode(CPU *cpu, uint8_t opcode)
{
  switch (opcode) {
    case 0: /* NOP */
      return NOP();

    case 0x1: /* LD BC,d16 */
    {
      uint16_t val = mmuReadWord(cpu->mmu, cpu->regs.pc);
      cpu->regs.b = val >> 8;
      cpu->regs.c = val;
      cpu->regs.pc += 2;
      return 12;
    }

    case 0x2: /* LD (BC),A */
      mmuWriteByte(cpu->mmu, (cpu->regs.b << 8) + cpu->regs.c, cpu->regs.a);
      return 8;

    case 0x3: /* INC BC */
    {
      uint16_t val = INC_W((cpu->regs.b << 8) + cpu->regs.c);
      cpu->regs.b = val >> 8;
      cpu->regs.c = val;
      return 8;
    }

    case 0x4: /* INC B */
      cpu->regs.b = INC(cpu, cpu->regs.b);
      return 4;

    case 0x5: /* DEC B */
      cpu->regs.b = DEC(cpu, cpu->regs.b);
      return 4;

    case 0x6: /* LD B,d8 */
      cpu->regs.b = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x7: /* RLCA */
      cpu->regs.a = RLC(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

    case 0x8: /* LD (a16),SP */
      mmuWriteWord(cpu->mmu, mmuReadWord(cpu->mmu, cpu->regs.pc), cpu->regs.sp);
      cpu->regs.pc += 2;
      return 20;

    case 0x9: /* ADD HL,BC */
      return ADD_WW(cpu, (cpu->regs.b << 8) + cpu->regs.c);

    case 0xA: /* LD A,(BC) */
      cpu->regs.a = mmuReadByte(cpu->mmu, (cpu->regs.b << 8) + cpu->regs.c);
      return 8;

    case 0xB: /* DEC BC */
    {
      uint16_t val = DEC_W((cpu->regs.b << 8) + cpu->regs.c);
      cpu->regs.b = val >> 8;
      cpu->regs.c = val;
      return 8;
    }

    case 0xC: /* INC C */
      cpu->regs.c = INC(cpu, cpu->regs.c);
      return 4;

    case 0xD: /* DEC C */
      cpu->regs.c = DEC(cpu, cpu->regs.c);
      return 4;

    case 0xE: /* LD C,d8 */
      cpu->regs.c = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0xF: /* RRCA */
      cpu->regs.a = RRC(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

    case 0x10: /* STOP 0 */
      return NOP();

    case 0x11: /* LD DE,d16 */
    {
      uint16_t val = mmuReadWord(cpu->mmu, cpu->regs.pc);
      cpu->regs.d = val >> 8;
      cpu->regs.e = val;
      cpu->regs.pc += 2;
      return 12;
    }

    case 0x12: /* LD (DE),A */
      mmuWriteByte(cpu->mmu, (cpu->regs.d << 8) + cpu->regs.e, cpu->regs.a);
      return 8;

    case 0x13: /* INC DE */
    {
      uint16_t val = INC_W((cpu->regs.d << 8) + cpu->regs.e);
      cpu->regs.d = val >> 8;
      cpu->regs.e = val;
      return 8;
    }

    case 0x14: /* INC D */
      cpu->regs.d = INC(cpu, cpu->regs.d);
      return 4;

    case 0x15: /* DEC D */
      cpu->regs.d = DEC(cpu, cpu->regs.d);
      return 4;

    case 0x16: /* LD D,d8 */
      cpu->regs.d = mmuReadByte(cpu->mmu, cpu->regs.pc); cpu->regs.pc++;
      return 8;

    case 0x17: /* RLA */
      cpu->regs.a = RL(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, false);
      return 4;

    case 0x18: /* JR r8 */
      cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(cpu->mmu, cpu->regs.pc)); cpu->regs.pc++;
      return 12;

    case 0x19: /* ADD HL,DE */
      return ADD_WW(cpu, (cpu->regs.d << 8) + cpu->regs.e);

    case 0x1A: /* LD A,(DE) */
      cpu->regs.a = mmuReadByte(cpu->mmu, (cpu->regs.d << 8) + cpu->regs.e);
      return 8;

    case 0x1B: /* DEC DE */
    {
      uint16_t val = DEC_W((cpu->regs.d << 8) + cpu->regs.e);
      cpu->regs.d = val >> 8;
      cpu->regs.e = val;
      return 8;
    }

    case 0x1C: /* INC E */
      cpu->regs.e = INC(cpu, cpu->regs.e);
      return 4;

    case 0x1D: /* DEC E */
      cpu->regs.e = DEC(cpu, cpu->regs.e);
      return 4;

    case 0x1E: /* LD E,d8 */
      cpu->regs.e = mmuReadByte(cpu->mmu, cpu->regs.pc); cpu->regs.pc++;
      return 8;

    case 0x1F: /* RRA */
      cpu->regs.a = RR(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, false);
      return 4;

    case 0x20: /* JR NZ,r8 */
      if (!cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(cpu->mmu, cpu->regs.pc));
        cpu->regs.pc++;
        return 12;
      }
      cpu->regs.pc++;
      return 8;

    case 0x21: /* LD HL,d16 */
    {
      uint16_t val = mmuReadWord(cpu->mmu, cpu->regs.pc);
      cpu->regs.h = val >> 8;
      cpu->regs.l = val;
      cpu->regs.pc += 2;
      return 12;
    }

    case 0x22: /* LD (HL+),A */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, cpu->regs.a);
      cpu->regs.h = (++addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x23: /* INC HL */
    {
      uint16_t val = INC_W((cpu->regs.h << 8) + cpu->regs.l);
      cpu->regs.h = val >> 8;
      cpu->regs.l = val;
      return 8;
    }

    case 0x24: /* INC H */
      cpu->regs.h = INC(cpu, cpu->regs.h);
      return 4;

    case 0x25: /* DEC H */
      cpu->regs.h = DEC(cpu, cpu->regs.h);
      return 4;

    case 0x26: /* LD H,d8 */
      cpu->regs.h = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

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
      if (cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(cpu->mmu, cpu->regs.pc));
        cpu->regs.pc++;
        return 12;
      }
      cpu->regs.pc++;
      return 8;

    case 0x29: /* ADD HL,HL */
      return ADD_WW(cpu, (cpu->regs.h << 8) + cpu->regs.l);

    case 0x2A: /* LD A,(HL+) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      cpu->regs.a = mmuReadByte(cpu->mmu, addr);
      cpu->regs.h = (++addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x2B: /* DEC HL */
    {
      uint16_t val = DEC_W((cpu->regs.h << 8) + cpu->regs.l);
      cpu->regs.h = val >> 8;
      cpu->regs.l = val;
      return 8;
    }

    case 0x2C: /* INC L */
      cpu->regs.l = INC(cpu, cpu->regs.l);
      return 4;

    case 0x2D: /* DEC L */
      cpu->regs.l = DEC(cpu, cpu->regs.l);
      return 4;

    case 0x2E: /* LD L,d8 */
      cpu->regs.l = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x2F: /* CPL */
      cpu->regs.a = ~cpu->regs.a;
      cpuSetFlag(cpu, FLAG_N, 1);
      cpuSetFlag(cpu, FLAG_H, 1);
      return 4;

    case 0x30: /* JR NC,r8 */
      if (!cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(cpu->mmu, cpu->regs.pc));
        cpu->regs.pc++;
        return 12;
      }
      cpu->regs.pc++;
      return 8;

    case 0x31: /* LD SP,d16 */
      cpu->regs.sp = mmuReadWord(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc += 2;
      return 12;

    case 0x32: /* LD (HL-),A */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, cpu->regs.a);
      cpu->regs.h = (--addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x33: /* INC SP */
      cpu->regs.sp = INC_W(cpu->regs.sp);
      return 8;

    case 0x34: /* INC (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, INC(cpu, mmuReadByte(cpu->mmu, addr)));
      return 12;
    }

    case 0x35: /* DEC (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, DEC(cpu, mmuReadByte(cpu->mmu, addr)));
      return 12;
    }

    case 0x36: /* LD (HL),d8 */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, mmuReadByte(cpu->mmu, cpu->regs.pc));
      cpu->regs.pc++;
      return 12;
    }

    case 0x37: /* SCF */
      cpuSetFlag(cpu, FLAG_C, 1);
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, 0);
      return 4;

    case 0x38: /* JR C,r8 */
      if (cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = JR(cpu->regs.pc, mmuReadByte(cpu->mmu, cpu->regs.pc));
        cpu->regs.pc++;
        return 12;
      }
      cpu->regs.pc++;
      return 8;

    case 0x39: /* ADD HL,SP */
      return ADD_WW(cpu, cpu->regs.sp);

    case 0x3A: /* LD A,(HL-) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      cpu->regs.a = mmuReadByte(cpu->mmu, addr);
      cpu->regs.h = (--addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x3B: /* DEC SP */
      cpu->regs.sp = DEC_W(cpu->regs.sp);
      return 8;

    case 0x3C: /* INC A */
      cpu->regs.a = INC(cpu, cpu->regs.a);
      return 4;

    case 0x3D: /* DEC A */
      cpu->regs.a = DEC(cpu, cpu->regs.a);
      return 4;

    case 0x3E: /* LD A,d8 */
      cpu->regs.a = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x3F: /* CCF */
      cpuSetFlag(cpu, FLAG_C, !cpuGetFlag(cpu, FLAG_C));
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, 0);
      return 4;

    case 0x40: /* LD B,B */
      return NOP();

    case 0x41: /* LD B,C */
      return LD_b_c(cpu);

    case 0x42: /* LD B,D */
      return LD_b_d(cpu);

    case 0x43: /* LD B,E */
      return LD_b_e(cpu);

    case 0x44: /* LD B,H */
      return LD_b_h(cpu);

    case 0x45: /* LD B,L */
      return LD_b_l(cpu);

    case 0x46: /* LD B,(HL) */
      return LD_b_hl(cpu);

    case 0x47: /* LD B,A */
      return LD_b_a(cpu);

    case 0x48: /* LD C,B */
      return LD_c_b(cpu);

    case 0x49: /* LD C,C */
      return NOP();

    case 0x4A: /* LD C,D */
      return LD_c_d(cpu);

    case 0x4B: /* LD C,E */
      return LD_c_e(cpu);

    case 0x4C: /* LD C,H */
      return LD_c_h(cpu);

    case 0x4D: /* LD C,L */
      return LD_c_l(cpu);

    case 0x4E: /* LD C,(HL) */
      return LD_c_hl(cpu);

    case 0x4F: /* LD C,A */
      return LD_c_a(cpu);

    case 0x50: /* LD D,B */
      return LD_d_b(cpu);

    case 0x51: /* LD D,C */
      return LD_d_c(cpu);

    case 0x52: /* LD D,D */
      return NOP();

    case 0x53: /* LD D,E */
      return LD_d_e(cpu);

    case 0x54: /* LD D,H */
      return LD_d_h(cpu);

    case 0x55: /* LD D,L */
      return LD_d_l(cpu);

    case 0x56: /* LD D,(HL) */
      return LD_d_hl(cpu);

    case 0x57: /* LD D,A */
      return LD_d_a(cpu);

    case 0x58: /* LD E,B */
      return LD_e_b(cpu);

    case 0x59: /* LD E,C */
      return LD_e_c(cpu);

    case 0x5A: /* LD E,D */
      return LD_e_d(cpu);

    case 0x5B: /* LD E,E */
      return NOP();

    case 0x5C: /* LD E,H */
      return LD_e_h(cpu);

    case 0x5D: /* LD E,L */
      return LD_e_l(cpu);

    case 0x5E: /* LD E,(HL) */
      return LD_e_hl(cpu);

    case 0x5F: /* LD E,A */
      return LD_e_a(cpu);

    case 0x60: /* LD H,B */
      return LD_h_b(cpu);

    case 0x61: /* LD H,C */
      return LD_h_c(cpu);

    case 0x62: /* LD H,D */
      return LD_h_d(cpu);

    case 0x63: /* LD H,E */
      return LD_h_e(cpu);

    case 0x64: /* LD H,H */
      return NOP();

    case 0x65: /* LD H,L */
      return LD_h_l(cpu);

    case 0x66: /* LD H,(HL) */
      return LD_h_hl(cpu);

    case 0x67: /* LD H,A */
      return LD_h_a(cpu);

    case 0x68: /* LD L,B */
      return LD_l_b(cpu);

    case 0x69: /* LD L,C */
      return LD_l_c(cpu);

    case 0x6A: /* LD L,D */
      return LD_l_d(cpu);

    case 0x6B: /* LD L,E */
      return LD_l_e(cpu);

    case 0x6C: /* LD L,H */
      return LD_l_h(cpu);

    case 0x6D: /* LD L,L */
      return NOP();

    case 0x6E: /* LD L,(HL) */
      return LD_l_hl(cpu);

    case 0x6F: /* LD L,A */
      return LD_l_a(cpu);

    case 0x70: /* LD (HL),B */
      return LD_hl_b(cpu);

    case 0x71: /* LD (HL),C */
      return LD_hl_c(cpu);

    case 0x72: /* LD (HL),D */
      return LD_hl_d(cpu);

    case 0x73: /* LD (HL),E */
      return LD_hl_e(cpu);

    case 0x74: /* LD (HL),H */
      return LD_hl_h(cpu);

    case 0x75: /* LD (HL),L */
      return LD_hl_l(cpu);

    case 0x76: /* HALT */
      return HALT(cpu);

    case 0x77: /* LD (HL),A */
      return LD_hl_a(cpu);

    case 0x78: /* LD A,B */
      return LD_a_b(cpu);

    case 0x79: /* LD A,C */
      return LD_a_c(cpu);

    case 0x7A: /* LD A,D */
      return LD_a_d(cpu);

    case 0x7B: /* LD A,E */
      return LD_a_e(cpu);

    case 0x7C: /* LD A,H */
      return LD_a_h(cpu);

    case 0x7D: /* LD A,L */
      return LD_a_l(cpu);

    case 0x7E: /* LD A,(HL) */
      return LD_a_hl(cpu);

    case 0x7F: /* LD A,A */
      return NOP();

    case 0x80: /* ADD A,B */
      return ADD(cpu, cpu->regs.b);

    case 0x81: /* ADD A,C */
      return ADD(cpu, cpu->regs.c);

    case 0x82: /* ADD A,D */
      return ADD(cpu, cpu->regs.d);

    case 0x83: /* ADD A,E */
      return ADD(cpu, cpu->regs.e);

    case 0x84: /* ADD A,H */
      return ADD(cpu, cpu->regs.h);

    case 0x85: /* ADD A,L */
      return ADD(cpu, cpu->regs.l);

    case 0x86: /* ADD A,(HL) */
      return ADD(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0x87: /* ADD A,A */
      return ADD(cpu, cpu->regs.a);

    case 0x88: /* ADC A,B */
      return ADC(cpu, cpu->regs.b);

    case 0x89: /* ADC A,C */
      return ADC(cpu, cpu->regs.c);

    case 0x8A: /* ADC A,D */
      return ADC(cpu, cpu->regs.d);

    case 0x8B: /* ADC A,E */
      return ADC(cpu, cpu->regs.e);

    case 0x8C: /* ADC A,H */
      return ADC(cpu, cpu->regs.h);

    case 0x8D: /* ADC A,L */
      return ADC(cpu, cpu->regs.l);

    case 0x8E: /* ADC A,(HL) */
      return ADC(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0x8F: /* ADC A,A */
      return ADC(cpu, cpu->regs.a);

    case 0x90: /* SUB A,B */
      return SUB(cpu, cpu->regs.b);

    case 0x91: /* SUB A,C */
      return SUB(cpu, cpu->regs.c);

    case 0x92: /* SUB A,D */
      return SUB(cpu, cpu->regs.d);

    case 0x93: /* SUB A,E */
      return SUB(cpu, cpu->regs.e);

    case 0x94: /* SUB A,H */
      return SUB(cpu, cpu->regs.h);

    case 0x95: /* SUB A,L */
      return SUB(cpu, cpu->regs.l);

    case 0x96: /* SUB A,(HL) */
      return SUB(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0x97: /* SUB A,A */
      return SUB(cpu, cpu->regs.a);

    case 0x98: /* SBC A,B */
      return SBC(cpu, cpu->regs.b);

    case 0x99: /* SBC A,C */
      return SBC(cpu, cpu->regs.c);

    case 0x9A: /* SBC A,D */
      return SBC(cpu, cpu->regs.d);

    case 0x9B: /* SBC A,E */
      return SBC(cpu, cpu->regs.e);

    case 0x9C: /* SBC A,H */
      return SBC(cpu, cpu->regs.h);

    case 0x9D: /* SBC A,L */
      return SBC(cpu, cpu->regs.l);

    case 0x9E: /* SBC A,(HL) */
      return SBC(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0x9F: /* SBC A,A */
      return SBC(cpu, cpu->regs.a);

    case 0xA0: /* AND B */
      return AND(cpu, cpu->regs.b);

    case 0xA1: /* AND C */
      return AND(cpu, cpu->regs.c);

    case 0xA2: /* AND D */
      return AND(cpu, cpu->regs.d);

    case 0xA3: /* AND E */
      return AND(cpu, cpu->regs.e);

    case 0xA4: /* AND H */
      return AND(cpu, cpu->regs.h);

    case 0xA5: /* AND L */
      return AND(cpu, cpu->regs.l);

    case 0xA6: /* AND (HL) */
      return AND(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0xA7: /* AND A */
      return AND(cpu, cpu->regs.a);

    case 0xA8: /* XOR B */
      return XOR(cpu, cpu->regs.b);

    case 0xA9: /* XOR C */
      return XOR(cpu, cpu->regs.c);

    case 0xAA: /* XOR D */
      return XOR(cpu, cpu->regs.d);

    case 0xAB: /* XOR E */
      return XOR(cpu, cpu->regs.e);

    case 0xAC: /* XOR H */
      return XOR(cpu, cpu->regs.h);

    case 0xAD: /* XOR L */
      return XOR(cpu, cpu->regs.l);

    case 0xAE: /* XOR (HL) */
      return XOR(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0xAF: /* XOR A */
      return XOR(cpu, cpu->regs.a);

    case 0xB0: /* OR B */
      return OR(cpu, cpu->regs.b);

    case 0xB1: /* OR C */
      return OR(cpu, cpu->regs.c);

    case 0xB2: /* OR D */
      return OR(cpu, cpu->regs.d);

    case 0xB3: /* OR E */
      return OR(cpu, cpu->regs.e);

    case 0xB4: /* OR H */
      return OR(cpu, cpu->regs.h);

    case 0xB5: /* OR L */
      return OR(cpu, cpu->regs.l);

    case 0xB6: /* OR (HL) */
      return OR(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l));

    case 0xB7: /* OR A */
      return OR(cpu, cpu->regs.a);

    case 0xB8: /* CP B */
      return CP(cpu, cpu->regs.b);

    case 0xB9: /* CP C */
      return CP(cpu, cpu->regs.c);

    case 0xBA: /* CP D */
      return CP(cpu, cpu->regs.d);

    case 0xBB: /* CP E */
      return CP(cpu, cpu->regs.e);

    case 0xBC: /* CP H */
      return CP(cpu, cpu->regs.h);

    case 0xBD: /* CP L */
      return CP(cpu, cpu->regs.l);

    case 0xBE: /* CP (HL) */
      return CP(cpu, mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l)) + 4;

    case 0xBF: /* CP A */
      return CP(cpu, cpu->regs.a);

    case 0xC0: /* RET NZ */
      if (!cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = POP(cpu);
        return 20;
      }
      return 8;

    case 0xC1: /* POP BC */
    {
      uint16_t result = POP(cpu);
      cpu->regs.b = result >> 8;
      cpu->regs.c = result;
      return 12;
    }

    case 0xC2: /* JP NZ,a16 */
      if (!cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 20;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xC3: /* JP a16 */
      cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
      return 16;

    case 0xC4: /* CALL NZ,a16 */
      if (!cpuGetFlag(cpu, FLAG_Z)) {
        PUSH(cpu, cpu->regs.pc + 0x2);
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 0x18;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xC5: /* PUSH BC */
      PUSH(cpu, (cpu->regs.b << 8) + cpu->regs.c);
      return 16;

    case 0xC6: /* ADD A,d8 */
    {
      uint8_t cycles = ADD(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xC7: /* RST 00H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0;
      return 16;

    case 0xC8: /* RET Z */
      if (cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = POP(cpu);
        return 20;
      }
      return 8;

    case 0xC9: /* RET */
      cpu->regs.pc = POP(cpu);
      return 16;

    case 0xCA: /* JP Z,a16 */
      if (cpuGetFlag(cpu, FLAG_Z)) {
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 16;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xCB: /* PREFIX CB */
      cpu->cb = true;
      return 4;

    case 0xCC: /* CALL Z,a16 */
      if (cpuGetFlag(cpu, FLAG_Z)) {
        PUSH(cpu, cpu->regs.pc + 0x2);
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 0x18;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xCD: /* CALL a16 */
      PUSH(cpu, cpu->regs.pc + 0x2);
      cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
      return 0x18;

    case 0xCE: /* ADC A,d8 */
    {
      uint8_t cycles = ADC(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xCF: /* RST 08H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 10;
      return 16;

    case 0xD0: /* RET NC */
      if (!cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = POP(cpu);
        return 0x20;
      }
      return 8;

    case 0xD1: /* POP DE */
    {
      uint16_t val = POP(cpu);
      cpu->regs.d = val >> 8;
      cpu->regs.e = val;
      return 12;
    }

    case 0xD2: /* JP NC,a16 */
      if (!cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 16;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xD4: /* CALL NC,a16 */
      if (!cpuGetFlag(cpu, FLAG_C)) {
        PUSH(cpu, cpu->regs.pc + 0x2);
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 0x18;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xD5: /* PUSH DE */
      PUSH(cpu, (cpu->regs.d << 8) + cpu->regs.e);
      return 16;

    case 0xD6: /* SUB A,d8 */
    {
      uint8_t cycles = SUB(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xD7: /* RST 10H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 16;
      return 16;

    case 0xD8: /* RET C */
      if (cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = POP(cpu);
        return 20;
      }
      return 8;

    case 0xD9: /* RETI */
      cpu->regs.pc = POP(cpu);
      cpu->ime = true;
      return 16;

    case 0xDA: /* JP C,a16 */
      if (cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 16;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xDC: /* CALL C,a16 */
      if (cpuGetFlag(cpu, FLAG_C)) {
        PUSH(cpu, cpu->regs.pc + 0x2);
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 0x18;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xDE: /* SBC A,d8 */
    {
      uint8_t cycles = SBC(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xDF: /* RST 18H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0x18;
      return 16;

    case 0xE0: /* LDH (a8),A */
      mmuWriteByte(cpu->mmu, 0xFF00 + mmuReadByte(cpu->mmu, cpu->regs.pc), cpu->regs.a);
      cpu->regs.pc++;
      return 12;

    case 0xE1: /* POP HL */
    {
      uint16_t val = POP(cpu);
      cpu->regs.h = val >> 8;
      cpu->regs.l = val;
      return 12;
    }

    case 0xE2: /* LD (C),A */
      mmuWriteByte(cpu->mmu, 0xFF00 + cpu->regs.c, cpu->regs.a);
      return 8;

    case 0xE5: /* PUSH HL */
      PUSH(cpu, (cpu->regs.h << 8) + cpu->regs.l);
      return 16;

    case 0xE6: /* AND d8 */
    {
      uint8_t cycles = AND(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xE7: /* RST 20H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0x20;
      return 16;

    case 0xE8: /* ADD SP,r8 */
    {
      int8_t op = mmuReadByte(cpu->mmu, cpu->regs.pc);
      uint16_t result = cpu->regs.sp + op;

      cpuSetFlag(cpu, FLAG_Z, 0);
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, (cpu->regs.sp & 0xF) + (op & 0xF) > 0xF);
      cpuSetFlag(cpu, FLAG_C, (cpu->regs.sp & 0xFF) + (op & 0xFF) > 0xFF);

      cpu->regs.sp = result;
      cpu->regs.pc++;

      return 16;
    }

    case 0xE9: /* JP (HL) */
      cpu->regs.pc = (cpu->regs.h << 8) + cpu->regs.l;
      return 4;

    case 0xEA: /* LD (a16),A */
      mmuWriteByte(cpu->mmu, mmuReadWord(cpu->mmu, cpu->regs.pc), cpu->regs.a);
      cpu->regs.pc += 2;
      return 16;

    case 0xEE: /* XOR d8 */
    {
      uint8_t cycles = XOR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xEF: /* RST 28H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0x28;
      return 16;

    case 0xF0: /* LDH A,(a8) */
      cpu->regs.a = mmuReadByte(cpu->mmu, 0xFF00 + mmuReadByte(cpu->mmu, cpu->regs.pc));
      cpu->regs.pc++;
      return 12;

    case 0xF1: /* POP AF */
    {
      uint16_t val = POP(cpu);
      cpu->regs.a = val >> 8;
      cpu->regs.f = val & 0xF0;
      return 12;
    }

    case 0xF2: /* LD A,(C) */
      cpu->regs.a = mmuReadByte(cpu->mmu, 0xFF00 + cpu->regs.c);
      return 8;

    case 0xF3: /* DI */
      cpu->ime = false;
      return 4;

    case 0xF5: /* PUSH AF */
      PUSH(cpu, (cpu->regs.a << 8) + cpu->regs.f);
      return 16;

    case 0xF6: /* OR d8 */
    {
      uint8_t cycles = OR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xF7: /* RST 30H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0x30;
      return 16;

    case 0xF8: /* LD HL,SP+r8 */
     {
      int8_t op = mmuReadByte(cpu->mmu, cpu->regs.pc);
      uint16_t result = cpu->regs.sp + op;

      cpuSetFlag(cpu, FLAG_Z, 0);
      cpuSetFlag(cpu, FLAG_N, 0);
      cpuSetFlag(cpu, FLAG_H, (cpu->regs.sp & 0xF) + (op & 0xF) > 0xF);
      cpuSetFlag(cpu, FLAG_C, (cpu->regs.sp & 0xFF) + (op & 0xFF) > 0xFF);

      cpu->regs.h = result >> 8;
      cpu->regs.l = result;
      cpu->regs.pc++;

      return 12;
    }

    case 0xF9: /* LD SP,HL */
      cpu->regs.sp = (cpu->regs.h << 8) + cpu->regs.l;
      return 8;

    case 0xFA: /* LD A,(a16) */
      cpu->regs.a = mmuReadByte(cpu->mmu, mmuReadWord(cpu->mmu, cpu->regs.pc));
      cpu->regs.pc += 2;
      return 16;

    case 0xFB: /* EI */
      cpu->ei = true;
      return 4;

    case 0xFE: /* CP d8 */
    {
      uint8_t cycles = CP(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc)) + 4;
      cpu->regs.pc++;
      return cycles;
    }

    case 0xFF: /* RST 38H */
      PUSH(cpu, cpu->regs.pc);
      cpu->regs.pc = 0x38;
      return 16;
  }

  return 0;
}

uint8_t cpuOpcodeCB(CPU *cpu, uint8_t opcode)
{
  switch (opcode) {
    case 0x0: /* RLC B */
      cpu->regs.b = RLC(cpu, cpu->regs.b);
      return 8;

    case 0x1: /* RLC C */
      cpu->regs.c = RLC(cpu, cpu->regs.c);
      return 8;

    case 0x2: /* RLC D */
      cpu->regs.d = RLC(cpu, cpu->regs.d);
      return 8;

    case 0x3: /* RLC E */
      cpu->regs.e = RLC(cpu, cpu->regs.e);
      return 8;

    case 0x4: /* RLC H */
      cpu->regs.h = RLC(cpu, cpu->regs.h);
      return 8;

    case 0x5: /* RLC L */
      cpu->regs.l = RLC(cpu, cpu->regs.l);
      return 8;

    case 0x6: /* RLC (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RLC(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x7: /* RLC A */
      cpu->regs.a = RLC(cpu, cpu->regs.a);
      return 8;

    case 0x8: /* RRC B */
      cpu->regs.b = RRC(cpu, cpu->regs.b);
      return 8;

    case 0x9: /* RRC C */
      cpu->regs.c = RRC(cpu, cpu->regs.c);
      return 8;

    case 0xA: /* RRC D */
      cpu->regs.d = RRC(cpu, cpu->regs.d);
      return 8;

    case 0xB: /* RRC E */
      cpu->regs.e = RRC(cpu, cpu->regs.e);
      return 8;

    case 0xC: /* RRC H */
      cpu->regs.h = RRC(cpu, cpu->regs.h);
      return 8;

    case 0xD: /* RRC L */
      cpu->regs.l = RRC(cpu, cpu->regs.l);
      return 8;

    case 0xE: /* RRC (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RRC(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0xF: /* RRC A */
      cpu->regs.a = RRC(cpu, cpu->regs.a);
      return 8;

    case 0x10: /* RL B */
      cpu->regs.b = RL(cpu, cpu->regs.b);
      return 8;

    case 0x11: /* RL C */
      cpu->regs.c = RL(cpu, cpu->regs.c);
      return 8;

    case 0x12: /* RL D */
      cpu->regs.d = RL(cpu, cpu->regs.d);
      return 8;

    case 0x13: /* RL E */
      cpu->regs.e = RL(cpu, cpu->regs.e);
      return 8;

    case 0x14: /* RL H */
      cpu->regs.h = RL(cpu, cpu->regs.h);
      return 8;

    case 0x15: /* RL L */
      cpu->regs.l = RL(cpu, cpu->regs.l);
      return 8;

    case 0x16: /* RL (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RL(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x17: /* RL A */
      cpu->regs.a = RL(cpu, cpu->regs.a);
      return 8;

    case 0x18: /* RR B */
      cpu->regs.b = RR(cpu, cpu->regs.b);
      return 8;

    case 0x19: /* RR C */
      cpu->regs.c = RR(cpu, cpu->regs.c);
      return 8;

    case 0x1A: /* RR D */
      cpu->regs.d = RR(cpu, cpu->regs.d);
      return 8;

    case 0x1B: /* RR E */
      cpu->regs.e = RR(cpu, cpu->regs.e);
      return 8;

    case 0x1C: /* RR H */
      cpu->regs.h = RR(cpu, cpu->regs.h);
      return 8;

    case 0x1D: /* RR L */
      cpu->regs.l = RR(cpu, cpu->regs.l);
      return 8;

    case 0x1E: /* RR (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RR(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x1F: /* RR A */
      cpu->regs.a = RR(cpu, cpu->regs.a);
      return 8;

    case 0x20: /* SLA B */
      cpu->regs.b = SLA(cpu, cpu->regs.b);
      return 8;

    case 0x21: /* SLA C */
      cpu->regs.c = SLA(cpu, cpu->regs.c);
      return 8;

    case 0x22: /* SLA D */
      cpu->regs.d = SLA(cpu, cpu->regs.d);
      return 8;

    case 0x23: /* SLA E */
      cpu->regs.e = SLA(cpu, cpu->regs.e);
      return 8;

    case 0x24: /* SLA H */
      cpu->regs.h = SLA(cpu, cpu->regs.h);
      return 8;

    case 0x25: /* SLA L */
      cpu->regs.l = SLA(cpu, cpu->regs.l);
      return 8;

    case 0x26: /* SLA (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SLA(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x27: /* SLA A */
      cpu->regs.a = SLA(cpu, cpu->regs.a);
      return 8;

    case 0x28: /* SRA B */
      cpu->regs.b = SRA(cpu, cpu->regs.b);
      return 8;

    case 0x29: /* SRA C */
      cpu->regs.c = SRA(cpu, cpu->regs.c);
      return 8;

    case 0x2A: /* SRA D */
      cpu->regs.d = SRA(cpu, cpu->regs.d);
      return 8;

    case 0x2B: /* SRA E */
      cpu->regs.e = SRA(cpu, cpu->regs.e);
      return 8;

    case 0x2C: /* SRA H */
      cpu->regs.h = SRA(cpu, cpu->regs.h);
      return 8;

    case 0x2D: /* SRA L */
      cpu->regs.l = SRA(cpu, cpu->regs.l);
      return 8;

    case 0x2E: /* SRA (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SRA(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x2F: /* SRA A */
      cpu->regs.a = SRA(cpu, cpu->regs.a);
      return 8;

    case 0x30: /* SWAP B */
      cpu->regs.b = SWAP(cpu, cpu->regs.b);
      return 8;

    case 0x31: /* SWAP C */
      cpu->regs.c = SWAP(cpu, cpu->regs.c);
      return 8;

    case 0x32: /* SWAP D */
      cpu->regs.d = SWAP(cpu, cpu->regs.d);
      return 8;

    case 0x33: /* SWAP E */
      cpu->regs.e = SWAP(cpu, cpu->regs.e);
      return 8;

    case 0x34: /* SWAP H */
      cpu->regs.h = SWAP(cpu, cpu->regs.h);
      return 8;

    case 0x35: /* SWAP L */
      cpu->regs.l = SWAP(cpu, cpu->regs.l);
      return 8;

    case 0x36: /* SWAP (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SWAP(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x37: /* SWAP A */
      cpu->regs.a = SWAP(cpu, cpu->regs.a);
      return 8;

    case 0x38: /* SRL B */
      cpu->regs.b = SRL(cpu, cpu->regs.b);
      return 8;

    case 0x39: /* SRL C */
      cpu->regs.c = SRL(cpu, cpu->regs.c);
      return 8;

    case 0x3A: /* SRL D */
      cpu->regs.d = SRL(cpu, cpu->regs.d);
      return 8;

    case 0x3B: /* SRL E */
      cpu->regs.e = SRL(cpu, cpu->regs.e);
      return 8;

    case 0x3C: /* SRL H */
      cpu->regs.h = SRL(cpu, cpu->regs.h);
      return 8;

    case 0x3D: /* SRL L */
      cpu->regs.l = SRL(cpu, cpu->regs.l);
      return 8;

    case 0x3E: /* SRL (HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SRL(cpu, mmuReadByte(cpu->mmu, addr)));
      return 16;
    }

    case 0x3F: /* SRL A */
      cpu->regs.a = SRL(cpu, cpu->regs.a);
      return 8;

    case 0x40: /* BIT 0,B */
      BIT(cpu, cpu->regs.b, 0);
      return 8;

    case 0x41: /* BIT 0,C */
      BIT(cpu, cpu->regs.c, 0);
      return 8;

    case 0x42: /* BIT 0,D */
      BIT(cpu, cpu->regs.d, 0);
      return 8;

    case 0x43: /* BIT 0,E */
      BIT(cpu, cpu->regs.e, 0);
      return 8;

    case 0x44: /* BIT 0,H */
      BIT(cpu, cpu->regs.h, 0);
      return 8;

    case 0x45: /* BIT 0,L */
      BIT(cpu, cpu->regs.l, 0);
      return 8;

    case 0x46: /* BIT 0,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 0);
      return 16;
    }

    case 0x47: /* BIT 0,A */
      BIT(cpu, cpu->regs.a, 0);
      return 8;

    case 0x48: /* BIT 1,B */
      BIT(cpu, cpu->regs.b, 1);
      return 8;

    case 0x49: /* BIT 1,C */
      BIT(cpu, cpu->regs.c, 1);
      return 8;

    case 0x4A: /* BIT 1,D */
      BIT(cpu, cpu->regs.d, 1);
      return 8;

    case 0x4B: /* BIT 1,E */
      BIT(cpu, cpu->regs.e, 1);
      return 8;

    case 0x4C: /* BIT 1,H */
      BIT(cpu, cpu->regs.h, 1);
      return 8;

    case 0x4D: /* BIT 1,L */
      BIT(cpu, cpu->regs.l, 1);
      return 8;

    case 0x4E: /* BIT 1,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 1);
      return 16;
    }

    case 0x4F: /* BIT 1,A */
      BIT(cpu, cpu->regs.a, 1);
      return 8;

    case 0x50: /* BIT 2,B */
      BIT(cpu, cpu->regs.b, 2);
      return 8;

    case 0x51: /* BIT 2,C */
      BIT(cpu, cpu->regs.c, 2);
      return 8;

    case 0x52: /* BIT 2,D */
      BIT(cpu, cpu->regs.d, 2);
      return 8;

    case 0x53: /* BIT 2,E */
      BIT(cpu, cpu->regs.e, 2);
      return 8;

    case 0x54: /* BIT 2,H */
      BIT(cpu, cpu->regs.h, 2);
      return 8;

    case 0x55: /* BIT 2,L */
      BIT(cpu, cpu->regs.l, 2);
      return 8;

    case 0x56: /* BIT 2,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 2);
      return 16;
    }

    case 0x57: /* BIT 2,A */
      BIT(cpu, cpu->regs.a, 2);
      return 8;

    case 0x58: /* BIT 3,B */
      BIT(cpu, cpu->regs.b, 3);
      return 8;

    case 0x59: /* BIT 3,C */
      BIT(cpu, cpu->regs.c, 3);
      return 8;

    case 0x5A: /* BIT 3,D */
      BIT(cpu, cpu->regs.d, 3);
      return 8;

    case 0x5B: /* BIT 3,E */
      BIT(cpu, cpu->regs.e, 3);
      return 8;

    case 0x5C: /* BIT 3,H */
      BIT(cpu, cpu->regs.h, 3);
      return 8;

    case 0x5D: /* BIT 3,L */
      BIT(cpu, cpu->regs.l, 3);
      return 8;

    case 0x5E: /* BIT 3,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 3);
      return 16;
    }

    case 0x5F: /* BIT 3,A */
      BIT(cpu, cpu->regs.a, 3);
      return 8;

    case 0x60: /* BIT 4,B */
      BIT(cpu, cpu->regs.b, 4);
      return 8;

    case 0x61: /* BIT 4,C */
      BIT(cpu, cpu->regs.c, 4);
      return 8;

    case 0x62: /* BIT 4,D */
      BIT(cpu, cpu->regs.d, 4);
      return 8;

    case 0x63: /* BIT 4,E */
      BIT(cpu, cpu->regs.e, 4);
      return 8;

    case 0x64: /* BIT 4,H */
      BIT(cpu, cpu->regs.h, 4);
      return 8;

    case 0x65: /* BIT 4,L */
      BIT(cpu, cpu->regs.l, 4);
      return 8;

    case 0x66: /* BIT 4,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 4);
      return 16;
    }

    case 0x67: /* BIT 4,A */
      BIT(cpu, cpu->regs.a, 4);
      return 8;

    case 0x68: /* BIT 5,B */
      BIT(cpu, cpu->regs.b, 5);
      return 8;

    case 0x69: /* BIT 5,C */
      BIT(cpu, cpu->regs.c, 5);
      return 8;

    case 0x6A: /* BIT 5,D */
      BIT(cpu, cpu->regs.d, 5);
      return 8;

    case 0x6B: /* BIT 5,E */
      BIT(cpu, cpu->regs.e, 5);
      return 8;

    case 0x6C: /* BIT 5,H */
      BIT(cpu, cpu->regs.h, 5);
      return 8;

    case 0x6D: /* BIT 5,L */
      BIT(cpu, cpu->regs.l, 5);
      return 8;

    case 0x6E: /* BIT 5,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 5);
      return 16;
    }

    case 0x6F: /* BIT 5,A */
      BIT(cpu, cpu->regs.a, 5);
      return 8;

    case 0x70: /* BIT 6,B */
      BIT(cpu, cpu->regs.b, 6);
      return 8;

    case 0x71: /* BIT 6,C */
      BIT(cpu, cpu->regs.c, 6);
      return 8;

    case 0x72: /* BIT 6,D */
      BIT(cpu, cpu->regs.d, 6);
      return 8;

    case 0x73: /* BIT 6,E */
      BIT(cpu, cpu->regs.e, 6);
      return 8;

    case 0x74: /* BIT 6,H */
      BIT(cpu, cpu->regs.h, 6);
      return 8;

    case 0x75: /* BIT 6,L */
      BIT(cpu, cpu->regs.l, 6);
      return 8;

    case 0x76: /* BIT 6,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 6);
      return 16;
    }

    case 0x77: /* BIT 6,A */
      BIT(cpu, cpu->regs.a, 6);
      return 8;

    case 0x78: /* BIT 7,B */
      BIT(cpu, cpu->regs.b, 7);
      return 8;

    case 0x79: /* BIT 7,C */
      BIT(cpu, cpu->regs.c, 7);
      return 8;

    case 0x7A: /* BIT 7,D */
      BIT(cpu, cpu->regs.d, 7);
      return 8;

    case 0x7B: /* BIT 7,E */
      BIT(cpu, cpu->regs.e, 7);
      return 8;

    case 0x7C: /* BIT 7,H */
      BIT(cpu, cpu->regs.h, 7);
      return 8;

    case 0x7D: /* BIT 7,L */
      BIT(cpu, cpu->regs.l, 7);
      return 8;

    case 0x7E: /* BIT 7,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      BIT(cpu, mmuReadByte(cpu->mmu, addr), 7);
      return 16;
    }

    case 0x7F: /* BIT 7,A */
      BIT(cpu, cpu->regs.a, 7);
      return 8;

    case 0x80: /* RES 0,B */
      cpu->regs.b = RES(cpu->regs.b, 0);
      return 8;

    case 0x81: /* RES 0,C */
      cpu->regs.c = RES(cpu->regs.c, 0);
      return 8;

    case 0x82: /* RES 0,D */
      cpu->regs.d = RES(cpu->regs.d, 0);
      return 8;

    case 0x83: /* RES 0,E */
      cpu->regs.e = RES(cpu->regs.e, 0);
      return 8;

    case 0x84: /* RES 0,H */
      cpu->regs.h = RES(cpu->regs.h, 0);
      return 8;

    case 0x85: /* RES 0,L */
      cpu->regs.l = RES(cpu->regs.l, 0);
      return 8;

    case 0x86: /* RES 0,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 0));
      return 16;
    }

    case 0x87: /* RES 0,A */
      cpu->regs.a = RES(cpu->regs.a, 0);
      return 8;

    case 0x88: /* RES 1,B */
      cpu->regs.b = RES(cpu->regs.b, 1);
      return 8;

    case 0x89: /* RES 1,C */
      cpu->regs.c = RES(cpu->regs.c, 1);
      return 8;

    case 0x8A: /* RES 1,D */
      cpu->regs.d = RES(cpu->regs.d, 1);
      return 8;

    case 0x8B: /* RES 1,E */
      cpu->regs.e = RES(cpu->regs.e, 1);
      return 8;

    case 0x8C: /* RES 1,H */
      cpu->regs.h = RES(cpu->regs.h, 1);
      return 8;

    case 0x8D: /* RES 1,L */
      cpu->regs.l = RES(cpu->regs.l, 1);
      return 8;

    case 0x8E: /* RES 1,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 1));
      return 16;
    }

    case 0x8F: /* RES 1,A */
      cpu->regs.a = RES(cpu->regs.a, 1);
      return 8;

    case 0x90: /* RES 2,B */
      cpu->regs.b = RES(cpu->regs.b, 2);
      return 8;

    case 0x91: /* RES 2,C */
      cpu->regs.c = RES(cpu->regs.c, 2);
      return 8;

    case 0x92: /* RES 2,D */
      cpu->regs.d = RES(cpu->regs.d, 2);
      return 8;

    case 0x93: /* RES 2,E */
      cpu->regs.e = RES(cpu->regs.e, 2);
      return 8;

    case 0x94: /* RES 2,H */
      cpu->regs.h = RES(cpu->regs.h, 2);
      return 8;

    case 0x95: /* RES 2,L */
      cpu->regs.l = RES(cpu->regs.l, 2);
      return 8;

    case 0x96: /* RES 2,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 2));
      return 16;
    }

    case 0x97: /* RES 2,A */
      cpu->regs.a = RES(cpu->regs.a, 2);
      return 8;

    case 0x98: /* RES 3,B */
      cpu->regs.b = RES(cpu->regs.b, 3);
      return 8;

    case 0x99: /* RES 3,C */
      cpu->regs.c = RES(cpu->regs.c, 3);
      return 8;

    case 0x9A: /* RES 3,D */
      cpu->regs.d = RES(cpu->regs.d, 3);
      return 8;

    case 0x9B: /* RES 3,E */
      cpu->regs.e = RES(cpu->regs.e, 3);
      return 8;

    case 0x9C: /* RES 3,H */
      cpu->regs.h = RES(cpu->regs.h, 3);
      return 8;

    case 0x9D: /* RES 3,L */
      cpu->regs.l = RES(cpu->regs.l, 3);
      return 8;

    case 0x9E: /* RES 3,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 3));
      return 16;
    }

    case 0x9F: /* RES 3,A */
      cpu->regs.a = RES(cpu->regs.a, 3);
      return 8;

    case 0xA0: /* RES 4,B */
      cpu->regs.b = RES(cpu->regs.b, 4);
      return 8;

    case 0xA1: /* RES 4,C */
      cpu->regs.c = RES(cpu->regs.c, 4);
      return 8;

    case 0xA2: /* RES 4,D */
      cpu->regs.d = RES(cpu->regs.d, 4);
      return 8;

    case 0xA3: /* RES 4,E */
      cpu->regs.e = RES(cpu->regs.e, 4);
      return 8;

    case 0xA4: /* RES 4,H */
      cpu->regs.h = RES(cpu->regs.h, 4);
      return 8;

    case 0xA5: /* RES 4,L */
      cpu->regs.l = RES(cpu->regs.l, 4);
      return 8;

    case 0xA6: /* RES 4,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 4));
      return 16;
    }

    case 0xA7: /* RES 4,A */
      cpu->regs.a = RES(cpu->regs.a, 4);
      return 8;

    case 0xA8: /* RES 5,B */
      cpu->regs.b = RES(cpu->regs.b, 5);
      return 8;

    case 0xA9: /* RES 5,C */
      cpu->regs.c = RES(cpu->regs.c, 5);
      return 8;

    case 0xAA: /* RES 5,D */
      cpu->regs.d = RES(cpu->regs.d, 5);
      return 8;

    case 0xAB: /* RES 5,E */
      cpu->regs.e = RES(cpu->regs.e, 5);
      return 8;

    case 0xAC: /* RES 5,H */
      cpu->regs.h = RES(cpu->regs.h, 5);
      return 8;

    case 0xAD: /* RES 5,L */
      cpu->regs.l = RES(cpu->regs.l, 5);
      return 8;

    case 0xAE: /* RES 5,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 5));
      return 16;
    }

    case 0xAF: /* RES 5,A */
      cpu->regs.a = RES(cpu->regs.a, 5);
      return 8;

    case 0xB0: /* RES 6,B */
      cpu->regs.b = RES(cpu->regs.b, 6);
      return 8;

    case 0xB1: /* RES 6,C */
      cpu->regs.c = RES(cpu->regs.c, 6);
      return 8;

    case 0xB2: /* RES 6,D */
      cpu->regs.d = RES(cpu->regs.d, 6);
      return 8;

    case 0xB3: /* RES 6,E */
      cpu->regs.e = RES(cpu->regs.e, 6);
      return 8;

    case 0xB4: /* RES 6,H */
      cpu->regs.h = RES(cpu->regs.h, 6);
      return 8;

    case 0xB5: /* RES 6,L */
      cpu->regs.l = RES(cpu->regs.l, 6);
      return 8;

    case 0xB6: /* RES 6,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 6));
      return 16;
    }

    case 0xB7: /* RES 6,A */
      cpu->regs.a = RES(cpu->regs.a, 6);
      return 8;

    case 0xB8: /* RES 7,B */
      cpu->regs.b = RES(cpu->regs.b, 7);
      return 8;

    case 0xB9: /* RES 7,C */
      cpu->regs.c = RES(cpu->regs.c, 7);
      return 8;

    case 0xBA: /* RES 7,D */
      cpu->regs.d = RES(cpu->regs.d, 7);
      return 8;

    case 0xBB: /* RES 7,E */
      cpu->regs.e = RES(cpu->regs.e, 7);
      return 8;

    case 0xBC: /* RES 7,H */
      cpu->regs.h = RES(cpu->regs.h, 7);
      return 8;

    case 0xBD: /* RES 7,L */
      cpu->regs.l = RES(cpu->regs.l, 7);
      return 8;

    case 0xBE: /* RES 7,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, RES(mmuReadByte(cpu->mmu, addr), 7));
      return 16;
    }

    case 0xBF: /* RES 7,A */
      cpu->regs.a = RES(cpu->regs.a, 7);
      return 8;

    case 0xC0: /* SET 0,B */
      cpu->regs.b = SET(cpu->regs.b, 0);
      return 8;

    case 0xC1: /* SET 0,C */
      cpu->regs.c = SET(cpu->regs.c, 0);
      return 8;

    case 0xC2: /* SET 0,D */
      cpu->regs.d = SET(cpu->regs.d, 0);
      return 8;

    case 0xC3: /* SET 0,E */
      cpu->regs.e = SET(cpu->regs.e, 0);
      return 8;

    case 0xC4: /* SET 0,H */
      cpu->regs.h = SET(cpu->regs.h, 0);
      return 8;

    case 0xC5: /* SET 0,L */
      cpu->regs.l = SET(cpu->regs.l, 0);
      return 8;

    case 0xC6: /* SET 0,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 0));
      return 16;
    }

    case 0xC7: /* SET 0,A */
      cpu->regs.a = SET(cpu->regs.a, 0);
      return 8;

    case 0xC8: /* SET 1,B */
      cpu->regs.b = SET(cpu->regs.b, 1);
      return 8;

    case 0xC9: /* SET 1,C */
      cpu->regs.c = SET(cpu->regs.c, 1);
      return 8;

    case 0xCA: /* SET 1,D */
      cpu->regs.d = SET(cpu->regs.d, 1);
      return 8;

    case 0xCB: /* SET 1,E */
      cpu->regs.e = SET(cpu->regs.e, 1);
      return 8;

    case 0xCC: /* SET 1,H */
      cpu->regs.h = SET(cpu->regs.h, 1);
      return 8;

    case 0xCD: /* SET 1,L */
      cpu->regs.l = SET(cpu->regs.l, 1);
      return 8;

    case 0xCE: /* SET 1,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 1));
      return 16;
    }

    case 0xCF: /* SET 1,A */
      cpu->regs.a = SET(cpu->regs.a, 1);
      return 8;

    case 0xD0: /* SET 2,B */
      cpu->regs.b = SET(cpu->regs.b, 2);
      return 8;

    case 0xD1: /* SET 2,C */
      cpu->regs.c = SET(cpu->regs.c, 2);
      return 8;

    case 0xD2: /* SET 2,D */
      cpu->regs.d = SET(cpu->regs.d, 2);
      return 8;

    case 0xD3: /* SET 2,E */
      cpu->regs.e = SET(cpu->regs.e, 2);
      return 8;

    case 0xD4: /* SET 2,H */
      cpu->regs.h = SET(cpu->regs.h, 2);
      return 8;

    case 0xD5: /* SET 2,L */
      cpu->regs.l = SET(cpu->regs.l, 2);
      return 8;

    case 0xD6: /* SET 2,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 2));
      return 16;
    }

    case 0xD7: /* SET 2,A */
      cpu->regs.a = SET(cpu->regs.a, 2);
      return 8;

    case 0xD8: /* SET 3,B */
      cpu->regs.b = SET(cpu->regs.b, 3);
      return 8;

    case 0xD9: /* SET 3,C */
      cpu->regs.c = SET(cpu->regs.c, 3);
      return 8;

    case 0xDA: /* SET 3,D */
      cpu->regs.d = SET(cpu->regs.d, 3);
      return 8;

    case 0xDB: /* SET 3,E */
      cpu->regs.e = SET(cpu->regs.e, 3);
      return 8;

    case 0xDC: /* SET 3,H */
      cpu->regs.h = SET(cpu->regs.h, 3);
      return 8;

    case 0xDD: /* SET 3,L */
      cpu->regs.l = SET(cpu->regs.l, 3);
      return 8;

    case 0xDE: /* SET 3,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 3));
      return 16;
    }

    case 0xDF: /* SET 3,A */
      cpu->regs.a = SET(cpu->regs.a, 3);
      return 8;

    case 0xE0: /* SET 4,B */
      cpu->regs.b = SET(cpu->regs.b, 4);
      return 8;

    case 0xE1: /* SET 4,C */
      cpu->regs.c = SET(cpu->regs.c, 4);
      return 8;

    case 0xE2: /* SET 4,D */
      cpu->regs.d = SET(cpu->regs.d, 4);
      return 8;

    case 0xE3: /* SET 4,E */
      cpu->regs.e = SET(cpu->regs.e, 4);
      return 8;

    case 0xE4: /* SET 4,H */
      cpu->regs.h = SET(cpu->regs.h, 4);
      return 8;

    case 0xE5: /* SET 4,L */
      cpu->regs.l = SET(cpu->regs.l, 4);
      return 8;

    case 0xE6: /* SET 4,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 4));
      return 16;
    }

    case 0xE7: /* SET 4,A */
      cpu->regs.a = SET(cpu->regs.a, 4);
      return 8;

    case 0xE8: /* SET 5,B */
      cpu->regs.b = SET(cpu->regs.b, 5);
      return 8;

    case 0xE9: /* SET 5,C */
      cpu->regs.c = SET(cpu->regs.c, 5);
      return 8;

    case 0xEA: /* SET 5,D */
      cpu->regs.d = SET(cpu->regs.d, 5);
      return 8;

    case 0xEB: /* SET 5,E */
      cpu->regs.e = SET(cpu->regs.e, 5);
      return 8;

    case 0xEC: /* SET 5,H */
      cpu->regs.h = SET(cpu->regs.h, 5);
      return 8;

    case 0xED: /* SET 5,L */
      cpu->regs.l = SET(cpu->regs.l, 5);
      return 8;

    case 0xEE: /* SET 5,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 5));
      return 16;
    }

    case 0xEF: /* SET 5,A */
      cpu->regs.a = SET(cpu->regs.a, 5);
      return 8;

    case 0xF0: /* SET 6,B */
      cpu->regs.b = SET(cpu->regs.b, 6);
      return 8;

    case 0xF1: /* SET 6,C */
      cpu->regs.c = SET(cpu->regs.c, 6);
      return 8;

    case 0xF2: /* SET 6,D */
      cpu->regs.d = SET(cpu->regs.d, 6);
      return 8;

    case 0xF3: /* SET 6,E */
      cpu->regs.e = SET(cpu->regs.e, 6);
      return 8;

    case 0xF4: /* SET 6,H */
      cpu->regs.h = SET(cpu->regs.h, 6);
      return 8;

    case 0xF5: /* SET 6,L */
      cpu->regs.l = SET(cpu->regs.l, 6);
      return 8;

    case 0xF6: /* SET 6,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 6));
      return 16;
    }

    case 0xF7: /* SET 6,A */
      cpu->regs.a = SET(cpu->regs.a, 6);
      return 8;

    case 0xF8: /* SET 7,B */
      cpu->regs.b = SET(cpu->regs.b, 7);
      return 8;

    case 0xF9: /* SET 7,C */
      cpu->regs.c = SET(cpu->regs.c, 7);
      return 8;

    case 0xFA: /* SET 7,D */
      cpu->regs.d = SET(cpu->regs.d, 7);
      return 8;

    case 0xFB: /* SET 7,E */
      cpu->regs.e = SET(cpu->regs.e, 7);
      return 8;

    case 0xFC: /* SET 7,H */
      cpu->regs.h = SET(cpu->regs.h, 7);
      return 8;

    case 0xFD: /* SET 7,L */
      cpu->regs.l = SET(cpu->regs.l, 7);
      return 8;

    case 0xFE: /* SET 7,(HL) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, SET(mmuReadByte(cpu->mmu, addr), 7));
      return 16;
    }

    case 0xFF: /* SET 7,A */
      cpu->regs.a = SET(cpu->regs.a, 7);
      return 8;
  }

  return 0;
}
