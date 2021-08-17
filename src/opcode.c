#include "opcode.h"

static uint8_t cpuOpcode(CPU*, uint8_t);

/**
 * CPU operations
 */
static uint8_t NOP(CPU *cpu)
{
  return 4;
}

static uint8_t HALT(CPU *cpu)
{
  cpu->halt = true;
  return 4;
}

static uint8_t CB(CPU *cpu)
{
  cpu->cb = true;
  return 4;
}

static uint8_t DI(CPU *cpu)
{
  cpu->ime = false;
  return 4;
}

static uint8_t EI(CPU *cpu)
{
  cpu->ei = true;
  return 4;
}

static uint8_t DAA(CPU *cpu)
{
  uint16_t a = cpu->regs.a;

  if(cpuGetFlag(cpu, FLAG_N)) {
    a -= cpuGetFlag(cpu, FLAG_H) ? 0x6 : 0;
    a -= cpuGetFlag(cpu, FLAG_C) ? 0x60 : 0;
  } else {
    a += cpuGetFlag(cpu, FLAG_H) || (a & 0xF) > 0x9 ? 0x6 : 0;
    if (cpuGetFlag(cpu, FLAG_C) || a > 0x9F) {
      a += 0x60;
      cpuSetFlag(cpu, FLAG_C, true);
    }
  }

  cpu->regs.a = a;

  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);

  return 4;
}

/**
 * Stack operations
 */
static void PUSH(CPU *cpu, uint16_t val)
{
  cpu->regs.sp -= 2;
  mmuWriteWord(cpu->mmu, cpu->regs.sp, val);
}

static uint16_t POP(CPU *cpu)
{
  uint16_t result = mmuReadWord(cpu->mmu, cpu->regs.sp);
  cpu->regs.sp += 2;
  return result;
}

static uint8_t RST(CPU *cpu, uint8_t val)
{
  PUSH(cpu, cpu->regs.pc);
  cpu->regs.pc = val;
  return 16;
}

/**
 * Bitshift operations
 */
static uint8_t JR(CPU *cpu)
{
  cpu->regs.pc += (int8_t)mmuReadByte(cpu->mmu, cpu->regs.pc);
  cpu->regs.pc++;
  return 12;
}

static uint8_t RLC(CPU *cpu, uint8_t *ref)
{
  uint8_t r = (*ref & 0x80) >> 7;

  cpuSetFlag(cpu, FLAG_C, r);
  *ref = (*ref << 1) + r;
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t RLCA(CPU *cpu)
{
  RLC(cpu, &cpu->regs.a);
  cpuSetFlag(cpu, FLAG_Z, false);
  return 4;
}

static uint8_t RRC(CPU *cpu, uint8_t *ref)
{
  uint8_t r = *ref & 0x1;

  cpuSetFlag(cpu, FLAG_C, r);
  *ref = (*ref >> 1) + (r << 7);
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t RRCA(CPU *cpu)
{
  RRC(cpu, &cpu->regs.a);
  cpuSetFlag(cpu, FLAG_Z, false);
  return 4;
}

static uint8_t RL(CPU *cpu, uint8_t *ref)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);

  cpuSetFlag(cpu, FLAG_C, *ref & 0x80);
  *ref = (*ref << 1) + c;
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t RLA(CPU *cpu)
{
  RL(cpu, &cpu->regs.a);
  cpuSetFlag(cpu, FLAG_Z, false);
  return 4;
}

static uint8_t RR(CPU *cpu, uint8_t *ref)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);

  cpuSetFlag(cpu, FLAG_C, *ref & 0x1);
  *ref = (*ref >> 1) + (c << 7);
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t RRA(CPU *cpu)
{
  RR(cpu, &cpu->regs.a);
  cpuSetFlag(cpu, FLAG_Z, false);
  return 4;
}

static uint8_t SLA(CPU *cpu, uint8_t *ref)
{
  cpuSetFlag(cpu, FLAG_C, *ref & 0x80);
  *ref <<= 1;
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t SRA(CPU *cpu, uint8_t *ref)
{
  cpuSetFlag(cpu, FLAG_C, *ref & 0x1);
  *ref >>= 1;
  *ref |= (*ref & 0x40) << 1;
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t SWAP(CPU *cpu, uint8_t *ref)
{
  *ref = (*ref >> 4) | (*ref << 4);

  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return 8;
}

static uint8_t SRL(CPU *cpu, uint8_t *ref)
{
  cpuSetFlag(cpu, FLAG_C, *ref & 0x1);
  *ref >>= 1;
  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 8;
}

static uint8_t BIT(CPU *cpu, uint8_t val, uint8_t *ref)
{
  cpuSetFlag(cpu, FLAG_Z, !(*ref & (1 << val)));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, true);

  return 8;
}

static uint8_t RES(CPU *cpu, uint8_t val, uint8_t *ref)
{
  *ref = *ref & ~(1 << val);
  return 8;
}

static uint8_t SET(CPU *cpu, uint8_t val, uint8_t *ref)
{
  *ref = *ref | (1 << val);
  return 8;
}

/**
 * Flag operations
 */
static uint8_t SCF(CPU *cpu)
{
  cpuSetFlag(cpu, FLAG_C, true);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 4;
}

static uint8_t CCF(CPU *cpu)
{
  cpuSetFlag(cpu, FLAG_C, !cpuGetFlag(cpu, FLAG_C));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return 4;
}

/**
 * Load/store operations
 */
#define DEF_LD_B(A, B)\
static uint8_t LD_##A##_##B(CPU *cpu)\
{\
  cpu->regs.A = cpu->regs.B;\
  return 4;\
}

#define DEF_LD_B_HL(A)\
static uint8_t LD_##A##_hl(CPU *cpu)\
{\
  cpu->regs.A = mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l);\
  return 8;\
}

#define DEF_LD_HL_B(A)\
static uint8_t LD_hl_##A(CPU *cpu)\
{\
  mmuWriteByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l, cpu->regs.A);\
  return 8;\
}

/**
 * Arithmetic operations
 */
static uint8_t ADD_W(CPU *cpu, uint16_t *op)
{
  uint16_t hl = (cpu->regs.h << 8) + cpu->regs.l;
  uint32_t result = hl + *op;

  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, (hl & 0xFFF) + (*op & 0xFFF) > 0xFFF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFFFF);

  cpu->regs.h = result >> 8;
  cpu->regs.l = result;

  return 8;
}

static uint8_t ADD(CPU *cpu, uint8_t *op)
{
  uint16_t result = cpu->regs.a + *op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) + (*op & 0xF) > 0xF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFF);

  cpu->regs.a = result;

  return 4;
}

static uint8_t ADC(CPU *cpu, uint8_t *op)
{
  uint8_t carry = cpuGetFlag(cpu, FLAG_C);
  uint16_t result = cpu->regs.a + *op + carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) + (*op & 0xF) + carry > 0xF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFF);

  cpu->regs.a = result;

  return 4;
}

static uint8_t SUB(CPU *cpu, uint8_t *op)
{
  int16_t result = cpu->regs.a - *op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (*op & 0xF) < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  cpu->regs.a = result;

  return 4;
}

static uint8_t SBC(CPU *cpu, uint8_t *op)
{
  uint8_t carry = cpuGetFlag(cpu, FLAG_C);
  int16_t result = cpu->regs.a - *op - carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (*op & 0xF) - carry < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  cpu->regs.a = result;

  return 4;
}

static uint8_t CP(CPU *cpu, uint8_t *op)
{
  int16_t result = cpu->regs.a - *op;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (*op & 0xF) < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  return 4;
}

// TODO: Remove macro and make INC_W
#define DEF_INC_W(A, B)\
static uint8_t INC_##A##B(CPU *cpu)\
{\
  uint16_t result = (cpu->regs.A << 8) + cpu->regs.B + 1;\
  cpu->regs.A = result >> 8;\
  cpu->regs.B = result;\
  return 8;\
}

static uint8_t INC(CPU *cpu, uint8_t *ref)
{
  (*ref)++;

  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, !(*ref & 0xF));

  return 4;
}

// TODO: Use DEF_OP_REG(INC_W, sp) instead
static uint8_t INC_sp(CPU *cpu)
{
  cpu->regs.sp++;
  return 8;
}

// TODO: Remove macro and make DEC_W
#define DEF_DEC_W(A, B)\
static uint8_t DEC_##A##B(CPU *cpu)\
{\
  uint16_t result = (cpu->regs.A << 8) + cpu->regs.B - 1;\
  cpu->regs.A = result >> 8;\
  cpu->regs.B = result;\
  return 8;\
}

static uint8_t DEC(CPU *cpu, uint8_t *ref)
{
  (*ref)--;

  cpuSetFlag(cpu, FLAG_Z, !*ref);
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (*ref & 0xF) == 0xF);

  return 4;
}

// TODO: Use DEF_OP_REG(DEC_W, sp) instead
static uint8_t DEC_sp(CPU *cpu)
{
  cpu->regs.sp--;
  return 8;
}

/**
 * Bitwise operations
 */
static uint8_t AND(CPU *cpu, uint8_t *op)
{
  cpu->regs.a &= *op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, true);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

static uint8_t OR(CPU *cpu, uint8_t *op)
{
  cpu->regs.a |= *op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

static uint8_t XOR(CPU *cpu, uint8_t *op)
{
  cpu->regs.a ^= *op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

/**
 * Definitions
 */
#define DEF_OP_REG(OP, A)\
static uint8_t OP##_##A(CPU *cpu)\
{\
  return OP(cpu, &cpu->regs.A);\
}

#define DEF_OP_$HL(OP)\
static uint8_t OP##_$hl(CPU *cpu)\
{\
  uint8_t ref = mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l);\
  uint8_t cycles = OP(cpu, &ref);\
\
  mmuWriteByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l, ref);\
\
  return cycles + 4;\
}

#define DEF_OP_B(OP, A)\
static uint8_t OP##_##A(CPU *cpu)\
{\
  return OP(cpu, A);\
}

#define DEF_OP_D8(OP)\
static uint8_t OP##_d8(CPU *cpu)\
{\
  uint8_t ref = mmuReadByte(cpu->mmu, cpu->regs.pc);\
  uint8_t cycles = OP(cpu, &ref);\
  cpu->regs.pc++;\
  return cycles + 4;\
}

#define DEF_OP_B_REG(OP, A, B)\
static uint8_t OP##_##A##_##B(CPU *cpu)\
{\
  return OP(cpu, A, &cpu->regs.B);\
}

#define DEF_OP_B_$HL(OP, A)\
static uint8_t OP##_##A##_$hl(CPU *cpu)\
{\
  uint8_t ref = mmuReadByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l);\
  uint8_t cycles = OP(cpu, A, &ref);\
\
  mmuWriteByte(cpu->mmu, (cpu->regs.h << 8) + cpu->regs.l, ref);\
\
  return cycles + 4;\
}

#define DEF_OP_Z(OP)\
static uint8_t OP##_z(CPU *cpu)\
{\
  if (cpuGetFlag(cpu, FLAG_Z)) {\
    return OP(cpu);\
  }\
  cpu->regs.pc++;\
  return 8;\
}

#define DEF_OP_NZ(OP)\
static uint8_t OP##_nz(CPU *cpu)\
{\
  if (!cpuGetFlag(cpu, FLAG_Z)) {\
    return OP(cpu);\
  }\
  cpu->regs.pc++;\
  return 8;\
}

#define DEF_OP_C(OP)\
static uint8_t OP##_c(CPU *cpu)\
{\
  if (cpuGetFlag(cpu, FLAG_C)) {\
    return OP(cpu);\
  }\
  cpu->regs.pc++;\
  return 8;\
}

#define DEF_OP_NC(OP)\
static uint8_t OP##_nc(CPU *cpu)\
{\
  if (!cpuGetFlag(cpu, FLAG_C)) {\
    return OP(cpu);\
  }\
  cpu->regs.pc++;\
  return 8;\
}

DEF_OP_B(RST, 0)
DEF_OP_B(RST, 0x8)
DEF_OP_B(RST, 0x10)
DEF_OP_B(RST, 0x18)
DEF_OP_B(RST, 0x20)
DEF_OP_B(RST, 0x28)
DEF_OP_B(RST, 0x30)
DEF_OP_B(RST, 0x38)

DEF_OP_Z(JR)
DEF_OP_NZ(JR)
DEF_OP_C(JR)
DEF_OP_NC(JR)

DEF_OP_REG(RLC, b)
DEF_OP_REG(RLC, c)
DEF_OP_REG(RLC, d)
DEF_OP_REG(RLC, e)
DEF_OP_REG(RLC, h)
DEF_OP_REG(RLC, l)
DEF_OP_$HL(RLC)
DEF_OP_REG(RLC, a)

DEF_OP_REG(RRC, b)
DEF_OP_REG(RRC, c)
DEF_OP_REG(RRC, d)
DEF_OP_REG(RRC, e)
DEF_OP_REG(RRC, h)
DEF_OP_REG(RRC, l)
DEF_OP_$HL(RRC)
DEF_OP_REG(RRC, a)

DEF_OP_REG(RL, b)
DEF_OP_REG(RL, c)
DEF_OP_REG(RL, d)
DEF_OP_REG(RL, e)
DEF_OP_REG(RL, h)
DEF_OP_REG(RL, l)
DEF_OP_$HL(RL)
DEF_OP_REG(RL, a)

DEF_OP_REG(RR, b)
DEF_OP_REG(RR, c)
DEF_OP_REG(RR, d)
DEF_OP_REG(RR, e)
DEF_OP_REG(RR, h)
DEF_OP_REG(RR, l)
DEF_OP_$HL(RR)
DEF_OP_REG(RR, a)

DEF_OP_REG(SLA, b)
DEF_OP_REG(SLA, c)
DEF_OP_REG(SLA, d)
DEF_OP_REG(SLA, e)
DEF_OP_REG(SLA, h)
DEF_OP_REG(SLA, l)
DEF_OP_$HL(SLA)
DEF_OP_REG(SLA, a)

DEF_OP_REG(SRA, b)
DEF_OP_REG(SRA, c)
DEF_OP_REG(SRA, d)
DEF_OP_REG(SRA, e)
DEF_OP_REG(SRA, h)
DEF_OP_REG(SRA, l)
DEF_OP_$HL(SRA)
DEF_OP_REG(SRA, a)

DEF_OP_REG(SWAP, b)
DEF_OP_REG(SWAP, c)
DEF_OP_REG(SWAP, d)
DEF_OP_REG(SWAP, e)
DEF_OP_REG(SWAP, h)
DEF_OP_REG(SWAP, l)
DEF_OP_$HL(SWAP)
DEF_OP_REG(SWAP, a)

DEF_OP_REG(SRL, b)
DEF_OP_REG(SRL, c)
DEF_OP_REG(SRL, d)
DEF_OP_REG(SRL, e)
DEF_OP_REG(SRL, h)
DEF_OP_REG(SRL, l)
DEF_OP_$HL(SRL)
DEF_OP_REG(SRL, a)

DEF_OP_B_REG(BIT, 0, b)
DEF_OP_B_REG(BIT, 0, c)
DEF_OP_B_REG(BIT, 0, d)
DEF_OP_B_REG(BIT, 0, e)
DEF_OP_B_REG(BIT, 0, h)
DEF_OP_B_REG(BIT, 0, l)
DEF_OP_B_$HL(BIT, 0)
DEF_OP_B_REG(BIT, 0, a)

DEF_OP_B_REG(BIT, 1, b)
DEF_OP_B_REG(BIT, 1, c)
DEF_OP_B_REG(BIT, 1, d)
DEF_OP_B_REG(BIT, 1, e)
DEF_OP_B_REG(BIT, 1, h)
DEF_OP_B_REG(BIT, 1, l)
DEF_OP_B_$HL(BIT, 1)
DEF_OP_B_REG(BIT, 1, a)

DEF_OP_B_REG(BIT, 2, b)
DEF_OP_B_REG(BIT, 2, c)
DEF_OP_B_REG(BIT, 2, d)
DEF_OP_B_REG(BIT, 2, e)
DEF_OP_B_REG(BIT, 2, h)
DEF_OP_B_REG(BIT, 2, l)
DEF_OP_B_$HL(BIT, 2)
DEF_OP_B_REG(BIT, 2, a)

DEF_OP_B_REG(BIT, 3, b)
DEF_OP_B_REG(BIT, 3, c)
DEF_OP_B_REG(BIT, 3, d)
DEF_OP_B_REG(BIT, 3, e)
DEF_OP_B_REG(BIT, 3, h)
DEF_OP_B_REG(BIT, 3, l)
DEF_OP_B_$HL(BIT, 3)
DEF_OP_B_REG(BIT, 3, a)

DEF_OP_B_REG(BIT, 4, b)
DEF_OP_B_REG(BIT, 4, c)
DEF_OP_B_REG(BIT, 4, d)
DEF_OP_B_REG(BIT, 4, e)
DEF_OP_B_REG(BIT, 4, h)
DEF_OP_B_REG(BIT, 4, l)
DEF_OP_B_$HL(BIT, 4)
DEF_OP_B_REG(BIT, 4, a)

DEF_OP_B_REG(BIT, 5, b)
DEF_OP_B_REG(BIT, 5, c)
DEF_OP_B_REG(BIT, 5, d)
DEF_OP_B_REG(BIT, 5, e)
DEF_OP_B_REG(BIT, 5, h)
DEF_OP_B_REG(BIT, 5, l)
DEF_OP_B_$HL(BIT, 5)
DEF_OP_B_REG(BIT, 5, a)

DEF_OP_B_REG(BIT, 6, b)
DEF_OP_B_REG(BIT, 6, c)
DEF_OP_B_REG(BIT, 6, d)
DEF_OP_B_REG(BIT, 6, e)
DEF_OP_B_REG(BIT, 6, h)
DEF_OP_B_REG(BIT, 6, l)
DEF_OP_B_$HL(BIT, 6)
DEF_OP_B_REG(BIT, 6, a)

DEF_OP_B_REG(BIT, 7, b)
DEF_OP_B_REG(BIT, 7, c)
DEF_OP_B_REG(BIT, 7, d)
DEF_OP_B_REG(BIT, 7, e)
DEF_OP_B_REG(BIT, 7, h)
DEF_OP_B_REG(BIT, 7, l)
DEF_OP_B_$HL(BIT, 7)
DEF_OP_B_REG(BIT, 7, a)

DEF_OP_B_REG(RES, 0, b)
DEF_OP_B_REG(RES, 0, c)
DEF_OP_B_REG(RES, 0, d)
DEF_OP_B_REG(RES, 0, e)
DEF_OP_B_REG(RES, 0, h)
DEF_OP_B_REG(RES, 0, l)
DEF_OP_B_$HL(RES, 0)
DEF_OP_B_REG(RES, 0, a)

DEF_OP_B_REG(RES, 1, b)
DEF_OP_B_REG(RES, 1, c)
DEF_OP_B_REG(RES, 1, d)
DEF_OP_B_REG(RES, 1, e)
DEF_OP_B_REG(RES, 1, h)
DEF_OP_B_REG(RES, 1, l)
DEF_OP_B_$HL(RES, 1)
DEF_OP_B_REG(RES, 1, a)

DEF_OP_B_REG(RES, 2, b)
DEF_OP_B_REG(RES, 2, c)
DEF_OP_B_REG(RES, 2, d)
DEF_OP_B_REG(RES, 2, e)
DEF_OP_B_REG(RES, 2, h)
DEF_OP_B_REG(RES, 2, l)
DEF_OP_B_$HL(RES, 2)
DEF_OP_B_REG(RES, 2, a)

DEF_OP_B_REG(RES, 3, b)
DEF_OP_B_REG(RES, 3, c)
DEF_OP_B_REG(RES, 3, d)
DEF_OP_B_REG(RES, 3, e)
DEF_OP_B_REG(RES, 3, h)
DEF_OP_B_REG(RES, 3, l)
DEF_OP_B_$HL(RES, 3)
DEF_OP_B_REG(RES, 3, a)

DEF_OP_B_REG(RES, 4, b)
DEF_OP_B_REG(RES, 4, c)
DEF_OP_B_REG(RES, 4, d)
DEF_OP_B_REG(RES, 4, e)
DEF_OP_B_REG(RES, 4, h)
DEF_OP_B_REG(RES, 4, l)
DEF_OP_B_$HL(RES, 4)
DEF_OP_B_REG(RES, 4, a)

DEF_OP_B_REG(RES, 5, b)
DEF_OP_B_REG(RES, 5, c)
DEF_OP_B_REG(RES, 5, d)
DEF_OP_B_REG(RES, 5, e)
DEF_OP_B_REG(RES, 5, h)
DEF_OP_B_REG(RES, 5, l)
DEF_OP_B_$HL(RES, 5)
DEF_OP_B_REG(RES, 5, a)

DEF_OP_B_REG(RES, 6, b)
DEF_OP_B_REG(RES, 6, c)
DEF_OP_B_REG(RES, 6, d)
DEF_OP_B_REG(RES, 6, e)
DEF_OP_B_REG(RES, 6, h)
DEF_OP_B_REG(RES, 6, l)
DEF_OP_B_$HL(RES, 6)
DEF_OP_B_REG(RES, 6, a)

DEF_OP_B_REG(RES, 7, b)
DEF_OP_B_REG(RES, 7, c)
DEF_OP_B_REG(RES, 7, d)
DEF_OP_B_REG(RES, 7, e)
DEF_OP_B_REG(RES, 7, h)
DEF_OP_B_REG(RES, 7, l)
DEF_OP_B_$HL(RES, 7)
DEF_OP_B_REG(RES, 7, a)

DEF_OP_B_REG(SET, 0, b)
DEF_OP_B_REG(SET, 0, c)
DEF_OP_B_REG(SET, 0, d)
DEF_OP_B_REG(SET, 0, e)
DEF_OP_B_REG(SET, 0, h)
DEF_OP_B_REG(SET, 0, l)
DEF_OP_B_$HL(SET, 0)
DEF_OP_B_REG(SET, 0, a)

DEF_OP_B_REG(SET, 1, b)
DEF_OP_B_REG(SET, 1, c)
DEF_OP_B_REG(SET, 1, d)
DEF_OP_B_REG(SET, 1, e)
DEF_OP_B_REG(SET, 1, h)
DEF_OP_B_REG(SET, 1, l)
DEF_OP_B_$HL(SET, 1)
DEF_OP_B_REG(SET, 1, a)

DEF_OP_B_REG(SET, 2, b)
DEF_OP_B_REG(SET, 2, c)
DEF_OP_B_REG(SET, 2, d)
DEF_OP_B_REG(SET, 2, e)
DEF_OP_B_REG(SET, 2, h)
DEF_OP_B_REG(SET, 2, l)
DEF_OP_B_$HL(SET, 2)
DEF_OP_B_REG(SET, 2, a)

DEF_OP_B_REG(SET, 3, b)
DEF_OP_B_REG(SET, 3, c)
DEF_OP_B_REG(SET, 3, d)
DEF_OP_B_REG(SET, 3, e)
DEF_OP_B_REG(SET, 3, h)
DEF_OP_B_REG(SET, 3, l)
DEF_OP_B_$HL(SET, 3)
DEF_OP_B_REG(SET, 3, a)

DEF_OP_B_REG(SET, 4, b)
DEF_OP_B_REG(SET, 4, c)
DEF_OP_B_REG(SET, 4, d)
DEF_OP_B_REG(SET, 4, e)
DEF_OP_B_REG(SET, 4, h)
DEF_OP_B_REG(SET, 4, l)
DEF_OP_B_$HL(SET, 4)
DEF_OP_B_REG(SET, 4, a)

DEF_OP_B_REG(SET, 5, b)
DEF_OP_B_REG(SET, 5, c)
DEF_OP_B_REG(SET, 5, d)
DEF_OP_B_REG(SET, 5, e)
DEF_OP_B_REG(SET, 5, h)
DEF_OP_B_REG(SET, 5, l)
DEF_OP_B_$HL(SET, 5)
DEF_OP_B_REG(SET, 5, a)

DEF_OP_B_REG(SET, 6, b)
DEF_OP_B_REG(SET, 6, c)
DEF_OP_B_REG(SET, 6, d)
DEF_OP_B_REG(SET, 6, e)
DEF_OP_B_REG(SET, 6, h)
DEF_OP_B_REG(SET, 6, l)
DEF_OP_B_$HL(SET, 6)
DEF_OP_B_REG(SET, 6, a)

DEF_OP_B_REG(SET, 7, b)
DEF_OP_B_REG(SET, 7, c)
DEF_OP_B_REG(SET, 7, d)
DEF_OP_B_REG(SET, 7, e)
DEF_OP_B_REG(SET, 7, h)
DEF_OP_B_REG(SET, 7, l)
DEF_OP_B_$HL(SET, 7)
DEF_OP_B_REG(SET, 7, a)

DEF_LD_B(b, c)
DEF_LD_B(b, d)
DEF_LD_B(b, e)
DEF_LD_B(b, h)
DEF_LD_B(b, l)
DEF_LD_B_HL(b)
DEF_LD_B(b, a)

DEF_LD_B(c, b)
DEF_LD_B(c, d)
DEF_LD_B(c, e)
DEF_LD_B(c, h)
DEF_LD_B(c, l)
DEF_LD_B_HL(c)
DEF_LD_B(c, a)

DEF_LD_B(d, b)
DEF_LD_B(d, c)
DEF_LD_B(d, e)
DEF_LD_B(d, h)
DEF_LD_B(d, l)
DEF_LD_B_HL(d)
DEF_LD_B(d, a)

DEF_LD_B(e, b)
DEF_LD_B(e, c)
DEF_LD_B(e, d)
DEF_LD_B(e, h)
DEF_LD_B(e, l)
DEF_LD_B_HL(e)
DEF_LD_B(e, a)

DEF_LD_B(h, b)
DEF_LD_B(h, c)
DEF_LD_B(h, d)
DEF_LD_B(h, e)
DEF_LD_B(h, l)
DEF_LD_B_HL(h)
DEF_LD_B(h, a)

DEF_LD_B(l, b)
DEF_LD_B(l, c)
DEF_LD_B(l, d)
DEF_LD_B(l, e)
DEF_LD_B(l, h)
DEF_LD_B_HL(l)
DEF_LD_B(l, a)

DEF_LD_HL_B(a)
DEF_LD_HL_B(b)
DEF_LD_HL_B(c)
DEF_LD_HL_B(d)
DEF_LD_HL_B(e)
DEF_LD_HL_B(h)
DEF_LD_HL_B(l)

DEF_LD_B(a, b)
DEF_LD_B(a, c)
DEF_LD_B(a, d)
DEF_LD_B(a, e)
DEF_LD_B(a, h)
DEF_LD_B(a, l)
DEF_LD_B_HL(a)

DEF_OP_REG(ADD, b)
DEF_OP_REG(ADD, c)
DEF_OP_REG(ADD, d)
DEF_OP_REG(ADD, e)
DEF_OP_REG(ADD, h)
DEF_OP_REG(ADD, l)
DEF_OP_$HL(ADD)
DEF_OP_REG(ADD, a)

DEF_OP_REG(ADC, b)
DEF_OP_REG(ADC, c)
DEF_OP_REG(ADC, d)
DEF_OP_REG(ADC, e)
DEF_OP_REG(ADC, h)
DEF_OP_REG(ADC, l)
DEF_OP_$HL(ADC)
DEF_OP_REG(ADC, a)

DEF_OP_REG(SUB, b)
DEF_OP_REG(SUB, c)
DEF_OP_REG(SUB, d)
DEF_OP_REG(SUB, e)
DEF_OP_REG(SUB, h)
DEF_OP_REG(SUB, l)
DEF_OP_$HL(SUB)
DEF_OP_REG(SUB, a)

DEF_OP_REG(SBC, b)
DEF_OP_REG(SBC, c)
DEF_OP_REG(SBC, d)
DEF_OP_REG(SBC, e)
DEF_OP_REG(SBC, h)
DEF_OP_REG(SBC, l)
DEF_OP_$HL(SBC)
DEF_OP_REG(SBC, a)

DEF_OP_REG(CP, b)
DEF_OP_REG(CP, c)
DEF_OP_REG(CP, d)
DEF_OP_REG(CP, e)
DEF_OP_REG(CP, h)
DEF_OP_REG(CP, l)
DEF_OP_$HL(CP)
DEF_OP_REG(CP, a)

DEF_OP_REG(INC, b)
DEF_OP_REG(INC, c)
DEF_OP_REG(INC, d)
DEF_OP_REG(INC, e)
DEF_OP_REG(INC, h)
DEF_OP_REG(INC, l)
DEF_OP_$HL(INC)
DEF_OP_REG(INC, a)

DEF_INC_W(b, c)
DEF_INC_W(d, e)
DEF_INC_W(h, l)

DEF_OP_REG(DEC, b)
DEF_OP_REG(DEC, c)
DEF_OP_REG(DEC, d)
DEF_OP_REG(DEC, e)
DEF_OP_REG(DEC, h)
DEF_OP_REG(DEC, l)
DEF_OP_$HL(DEC)
DEF_OP_REG(DEC, a)

DEF_DEC_W(b, c)
DEF_DEC_W(d, e)
DEF_DEC_W(h, l)

DEF_OP_REG(AND, b)
DEF_OP_REG(AND, c)
DEF_OP_REG(AND, d)
DEF_OP_REG(AND, e)
DEF_OP_REG(AND, h)
DEF_OP_REG(AND, l)
DEF_OP_$HL(AND)
DEF_OP_REG(AND, a)

DEF_OP_REG(OR, b)
DEF_OP_REG(OR, c)
DEF_OP_REG(OR, d)
DEF_OP_REG(OR, e)
DEF_OP_REG(OR, h)
DEF_OP_REG(OR, l)
DEF_OP_$HL(OR)
DEF_OP_REG(OR, a)

DEF_OP_REG(XOR, b)
DEF_OP_REG(XOR, c)
DEF_OP_REG(XOR, d)
DEF_OP_REG(XOR, e)
DEF_OP_REG(XOR, h)
DEF_OP_REG(XOR, l)
DEF_OP_$HL(XOR)
DEF_OP_REG(XOR, a)

DEF_OP_D8(ADD)
DEF_OP_D8(ADC)
DEF_OP_D8(SUB)
DEF_OP_D8(SBC)
DEF_OP_D8(AND)
DEF_OP_D8(XOR)
DEF_OP_D8(OR)
DEF_OP_D8(CP)

/**
 * Opcode definitions
 */
static uint8_t (*cpuOpsCB[256])(CPU*) = {
  &RLC_b,     // RLC B
  &RLC_c,     // RLC C
  &RLC_d,     // RLC D
  &RLC_e,     // RLC E
  &RLC_h,     // RLC H
  &RLC_l,     // RLC L
  &RLC_$hl,   // RLC (HL)
  &RLC_a,     // RLC A
  &RRC_b,     // RRC B
  &RRC_c,     // RRC C
  &RRC_d,     // RRC D
  &RRC_e,     // RRC E
  &RRC_h,     // RRC H
  &RRC_l,     // RRC L
  &RRC_$hl,   // RRC (HL)
  &RRC_a,     // RRC A
  &RL_b,      // RL B
  &RL_c,      // RL C
  &RL_d,      // RL D
  &RL_e,      // RL E
  &RL_h,      // RL H
  &RL_l,      // RL L
  &RL_$hl,    // RL (HL)
  &RL_a,      // RL A
  &RR_b,      // RR B
  &RR_c,      // RR C
  &RR_d,      // RR D
  &RR_e,      // RR E
  &RR_h,      // RR H
  &RR_l,      // RR L
  &RR_$hl,    // RR (HL)
  &RR_a,      // RR A
  &SLA_b,     // SLA B
  &SLA_c,     // SLA C
  &SLA_d,     // SLA D
  &SLA_e,     // SLA E
  &SLA_h,     // SLA H
  &SLA_l,     // SLA L
  &SLA_$hl,   // SLA (HL)
  &SLA_a,     // SLA A
  &SRA_b,     // SRA B
  &SRA_c,     // SRA C
  &SRA_d,     // SRA D
  &SRA_e,     // SRA E
  &SRA_h,     // SRA H
  &SRA_l,     // SRA L
  &SRA_$hl,   // SRA (HL)
  &SRA_a,     // SRA A
  &SWAP_b,    // SWAP B
  &SWAP_c,    // SWAP C
  &SWAP_d,    // SWAP D
  &SWAP_e,    // SWAP E
  &SWAP_h,    // SWAP H
  &SWAP_l,    // SWAP L
  &SWAP_$hl,  // SWAP (HL)
  &SWAP_a,    // SWAP A
  &SRL_b,     // SRL B
  &SRL_c,     // SRL C
  &SRL_d,     // SRL D
  &SRL_e,     // SRL E
  &SRL_h,     // SRL H
  &SRL_l,     // SRL L
  &SRL_$hl,   // SRL (HL)
  &SRL_a,     // SRL A
  &BIT_0_b,   // BIT 0,B
  &BIT_0_c,   // BIT 0,C
  &BIT_0_d,   // BIT 0,D
  &BIT_0_e,   // BIT 0,E
  &BIT_0_h,   // BIT 0,H
  &BIT_0_l,   // BIT 0,L
  &BIT_0_$hl, // BIT 0,(HL)
  &BIT_0_a,   // BIT 0,A
  &BIT_1_b,   // BIT 1,B
  &BIT_1_c,   // BIT 1,C
  &BIT_1_d,   // BIT 1,D
  &BIT_1_e,   // BIT 1,E
  &BIT_1_h,   // BIT 1,H
  &BIT_1_l,   // BIT 1,L
  &BIT_1_$hl, // BIT 1,(HL)
  &BIT_1_a,   // BIT 1,A
  &BIT_2_b,   // BIT 2,B
  &BIT_2_c,   // BIT 2,C
  &BIT_2_d,   // BIT 2,D
  &BIT_2_e,   // BIT 2,E
  &BIT_2_h,   // BIT 2,H
  &BIT_2_l,   // BIT 2,L
  &BIT_2_$hl, // BIT 2,(HL)
  &BIT_2_a,   // BIT 2,A
  &BIT_3_b,   // BIT 3,B
  &BIT_3_c,   // BIT 3,C
  &BIT_3_d,   // BIT 3,D
  &BIT_3_e,   // BIT 3,E
  &BIT_3_h,   // BIT 3,H
  &BIT_3_l,   // BIT 3,L
  &BIT_3_$hl, // BIT 3,(HL)
  &BIT_3_a,   // BIT 3,A
  &BIT_4_b,   // BIT 4,B
  &BIT_4_c,   // BIT 4,C
  &BIT_4_d,   // BIT 4,D
  &BIT_4_e,   // BIT 4,E
  &BIT_4_h,   // BIT 4,H
  &BIT_4_l,   // BIT 4,L
  &BIT_4_$hl, // BIT 4,(HL)
  &BIT_4_a,   // BIT 4,A
  &BIT_5_b,   // BIT 5,B
  &BIT_5_c,   // BIT 5,C
  &BIT_5_d,   // BIT 5,D
  &BIT_5_e,   // BIT 5,E
  &BIT_5_h,   // BIT 5,H
  &BIT_5_l,   // BIT 5,L
  &BIT_5_$hl, // BIT 5,(HL)
  &BIT_5_a,   // BIT 5,A
  &BIT_6_b,   // BIT 6,B
  &BIT_6_c,   // BIT 6,C
  &BIT_6_d,   // BIT 6,D
  &BIT_6_e,   // BIT 6,E
  &BIT_6_h,   // BIT 6,H
  &BIT_6_l,   // BIT 6,L
  &BIT_6_$hl, // BIT 6,(HL)
  &BIT_6_a,   // BIT 6,A
  &BIT_7_b,   // BIT 7,B
  &BIT_7_c,   // BIT 7,C
  &BIT_7_d,   // BIT 7,D
  &BIT_7_e,   // BIT 7,E
  &BIT_7_h,   // BIT 7,H
  &BIT_7_l,   // BIT 7,L
  &BIT_7_$hl, // BIT 7,(HL)
  &BIT_7_a,   // BIT 7,A
  &RES_0_b,   // RES 0,B
  &RES_0_c,   // RES 0,C
  &RES_0_d,   // RES 0,D
  &RES_0_e,   // RES 0,E
  &RES_0_h,   // RES 0,H
  &RES_0_l,   // RES 0,L
  &RES_0_$hl, // RES 0,(HL)
  &RES_0_a,   // RES 0,A
  &RES_1_b,   // RES 1,B
  &RES_1_c,   // RES 1,C
  &RES_1_d,   // RES 1,D
  &RES_1_e,   // RES 1,E
  &RES_1_h,   // RES 1,H
  &RES_1_l,   // RES 1,L
  &RES_1_$hl, // RES 1,(HL)
  &RES_1_a,   // RES 1,A
  &RES_2_b,   // RES 2,B
  &RES_2_c,   // RES 2,C
  &RES_2_d,   // RES 2,D
  &RES_2_e,   // RES 2,E
  &RES_2_h,   // RES 2,H
  &RES_2_l,   // RES 2,L
  &RES_2_$hl, // RES 2,(HL)
  &RES_2_a,   // RES 2,A
  &RES_3_b,   // RES 3,B
  &RES_3_c,   // RES 3,C
  &RES_3_d,   // RES 3,D
  &RES_3_e,   // RES 3,E
  &RES_3_h,   // RES 3,H
  &RES_3_l,   // RES 3,L
  &RES_3_$hl, // RES 3,(HL)
  &RES_3_a,   // RES 3,A
  &RES_4_b,   // RES 4,B
  &RES_4_c,   // RES 4,C
  &RES_4_d,   // RES 4,D
  &RES_4_e,   // RES 4,E
  &RES_4_h,   // RES 4,H
  &RES_4_l,   // RES 4,L
  &RES_4_$hl, // RES 4,(HL)
  &RES_4_a,   // RES 4,A
  &RES_5_b,   // RES 5,B
  &RES_5_c,   // RES 5,C
  &RES_5_d,   // RES 5,D
  &RES_5_e,   // RES 5,E
  &RES_5_h,   // RES 5,H
  &RES_5_l,   // RES 5,L
  &RES_5_$hl, // RES 5,(HL)
  &RES_5_a,   // RES 5,A
  &RES_6_b,   // RES 6,B
  &RES_6_c,   // RES 6,C
  &RES_6_d,   // RES 6,D
  &RES_6_e,   // RES 6,E
  &RES_6_h,   // RES 6,H
  &RES_6_l,   // RES 6,L
  &RES_6_$hl, // RES 6,(HL)
  &RES_6_a,   // RES 6,A
  &RES_7_b,   // RES 7,B
  &RES_7_c,   // RES 7,C
  &RES_7_d,   // RES 7,D
  &RES_7_e,   // RES 7,E
  &RES_7_h,   // RES 7,H
  &RES_7_l,   // RES 7,L
  &RES_7_$hl, // RES 7,(HL)
  &RES_7_a,   // RES 7,A
  &SET_0_b,   // SET 0,B
  &SET_0_c,   // SET 0,C
  &SET_0_d,   // SET 0,D
  &SET_0_e,   // SET 0,E
  &SET_0_h,   // SET 0,H
  &SET_0_l,   // SET 0,L
  &SET_0_$hl, // SET 0,(HL)
  &SET_0_a,   // SET 0,A
  &SET_1_b,   // SET 1,B
  &SET_1_c,   // SET 1,C
  &SET_1_d,   // SET 1,D
  &SET_1_e,   // SET 1,E
  &SET_1_h,   // SET 1,H
  &SET_1_l,   // SET 1,L
  &SET_1_$hl, // SET 1,(HL)
  &SET_1_a,   // SET 1,A
  &SET_2_b,   // SET 2,B
  &SET_2_c,   // SET 2,C
  &SET_2_d,   // SET 2,D
  &SET_2_e,   // SET 2,E
  &SET_2_h,   // SET 2,H
  &SET_2_l,   // SET 2,L
  &SET_2_$hl, // SET 2,(HL)
  &SET_2_a,   // SET 2,A
  &SET_3_b,   // SET 3,B
  &SET_3_c,   // SET 3,C
  &SET_3_d,   // SET 3,D
  &SET_3_e,   // SET 3,E
  &SET_3_h,   // SET 3,H
  &SET_3_l,   // SET 3,L
  &SET_3_$hl, // SET 3,(HL)
  &SET_3_a,   // SET 3,A
  &SET_4_b,   // SET 4,B
  &SET_4_c,   // SET 4,C
  &SET_4_d,   // SET 4,D
  &SET_4_e,   // SET 4,E
  &SET_4_h,   // SET 4,H
  &SET_4_l,   // SET 4,L
  &SET_4_$hl, // SET 4,(HL)
  &SET_4_a,   // SET 4,A
  &SET_5_b,   // SET 5,B
  &SET_5_c,   // SET 5,C
  &SET_5_d,   // SET 5,D
  &SET_5_e,   // SET 5,E
  &SET_5_h,   // SET 5,H
  &SET_5_l,   // SET 5,L
  &SET_5_$hl, // SET 5,(HL)
  &SET_5_a,   // SET 5,A
  &SET_6_b,   // SET 6,B
  &SET_6_c,   // SET 6,C
  &SET_6_d,   // SET 6,D
  &SET_6_e,   // SET 6,E
  &SET_6_h,   // SET 6,H
  &SET_6_l,   // SET 6,L
  &SET_6_$hl, // SET 6,(HL)
  &SET_6_a,   // SET 6,A
  &SET_7_b,   // SET 7,B
  &SET_7_c,   // SET 7,C
  &SET_7_d,   // SET 7,D
  &SET_7_e,   // SET 7,E
  &SET_7_h,   // SET 7,H
  &SET_7_l,   // SET 7,L
  &SET_7_$hl, // SET 7,(HL)
  &SET_7_a    // SET 7,A
};

uint8_t cpuExec(CPU *cpu)
{
  uint8_t op = mmuReadByte(cpu->mmu, cpu->regs.pc);
  cpu->regs.pc++;

  if (cpu->cb) {
    cpu->cb = false;
    return cpuOpsCB[op](cpu);
  }

  return cpuOpcode(cpu, op);
}

uint8_t cpuOpcode(CPU *cpu, uint8_t opcode)
{
  switch (opcode) {
    case 0: /* NOP */
      return NOP(cpu);

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
      return INC_bc(cpu);

    case 0x4: /* INC B */
      return INC_b(cpu);

    case 0x5: /* DEC B */
      return DEC_b(cpu);

    case 0x6: /* LD B,d8 */
      cpu->regs.b = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x7: /* RLCA */
      return RLCA(cpu);

    case 0x8: /* LD (a16),SP */
      mmuWriteWord(cpu->mmu, mmuReadWord(cpu->mmu, cpu->regs.pc), cpu->regs.sp);
      cpu->regs.pc += 2;
      return 20;

    case 0x9: /* ADD HL,BC */
    {
      uint16_t ref = (cpu->regs.b << 8) + cpu->regs.c;
      return ADD_W(cpu, &ref);
    }

    case 0xA: /* LD A,(BC) */
      cpu->regs.a = mmuReadByte(cpu->mmu, (cpu->regs.b << 8) + cpu->regs.c);
      return 8;

    case 0xB: /* DEC BC */
      return DEC_bc(cpu);

    case 0xC: /* INC C */
      return INC_c(cpu);

    case 0xD: /* DEC C */
      return DEC_c(cpu);

    case 0xE: /* LD C,d8 */
      cpu->regs.c = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0xF: /* RRCA */
      return RRCA(cpu);

    case 0x10: /* STOP 0 */
      return NOP(cpu);

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
      return INC_de(cpu);

    case 0x14: /* INC D */
      return INC_d(cpu);

    case 0x15: /* DEC D */
      return DEC_d(cpu);

    case 0x16: /* LD D,d8 */
      cpu->regs.d = mmuReadByte(cpu->mmu, cpu->regs.pc); cpu->regs.pc++;
      return 8;

    case 0x17: /* RLA */
      return RLA(cpu);

    case 0x18: /* JR r8 */
      return JR(cpu);

    case 0x19: /* ADD HL,DE */
    {
      uint16_t ref = (cpu->regs.d << 8) + cpu->regs.e;
      return ADD_W(cpu, &ref);
    }

    case 0x1A: /* LD A,(DE) */
      cpu->regs.a = mmuReadByte(cpu->mmu, (cpu->regs.d << 8) + cpu->regs.e);
      return 8;

    case 0x1B: /* DEC DE */
      return DEC_de(cpu);

    case 0x1C: /* INC E */
      return INC_e(cpu);

    case 0x1D: /* DEC E */
      return DEC_e(cpu);

    case 0x1E: /* LD E,d8 */
      cpu->regs.e = mmuReadByte(cpu->mmu, cpu->regs.pc); cpu->regs.pc++;
      return 8;

    case 0x1F: /* RRA */
      return RRA(cpu);

    case 0x20: /* JR NZ,r8 */
      return JR_nz(cpu);

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
      return INC_hl(cpu);

    case 0x24: /* INC H */
      return INC_h(cpu);

    case 0x25: /* DEC H */
      return DEC_h(cpu);

    case 0x26: /* LD H,d8 */
      cpu->regs.h = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x27: /* DAA */
      return DAA(cpu);

    case 0x28: /* JR Z,r8 */
      return JR_z(cpu);

    case 0x29: /* ADD HL,HL */
    {
      uint16_t ref = (cpu->regs.h << 8) + cpu->regs.l;
      return ADD_W(cpu, &ref);
    }

    case 0x2A: /* LD A,(HL+) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      cpu->regs.a = mmuReadByte(cpu->mmu, addr);
      cpu->regs.h = (++addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x2B: /* DEC HL */
      return DEC_hl(cpu);

    case 0x2C: /* INC L */
      return INC_l(cpu);

    case 0x2D: /* DEC L */
      return DEC_l(cpu);

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
      return JR_nc(cpu);

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
      return INC_sp(cpu);

    case 0x34: /* INC (HL) */
      return INC_$hl(cpu);

    case 0x35: /* DEC (HL) */
      return DEC_$hl(cpu);

    case 0x36: /* LD (HL),d8 */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      mmuWriteByte(cpu->mmu, addr, mmuReadByte(cpu->mmu, cpu->regs.pc));
      cpu->regs.pc++;
      return 12;
    }

    case 0x37: /* SCF */
      return SCF(cpu);

    case 0x38: /* JR C,r8 */
      return JR_c(cpu);

    case 0x39: /* ADD HL,SP */
      return ADD_W(cpu, &cpu->regs.sp);

    case 0x3A: /* LD A,(HL-) */
    {
      uint16_t addr = (cpu->regs.h << 8) + cpu->regs.l;
      cpu->regs.a = mmuReadByte(cpu->mmu, addr);
      cpu->regs.h = (--addr) >> 8;
      cpu->regs.l = addr;
      return 8;
    }

    case 0x3B: /* DEC SP */
      return DEC_sp(cpu);

    case 0x3C: /* INC A */
      return INC_a(cpu);

    case 0x3D: /* DEC A */
      return DEC_a(cpu);

    case 0x3E: /* LD A,d8 */
      cpu->regs.a = mmuReadByte(cpu->mmu, cpu->regs.pc);
      cpu->regs.pc++;
      return 8;

    case 0x3F: /* CCF */
      return CCF(cpu);

    case 0x40: /* LD B,B */
      return NOP(cpu);

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
      return NOP(cpu);

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
      return NOP(cpu);

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
      return NOP(cpu);

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
      return NOP(cpu);

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
      return NOP(cpu);

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
      return NOP(cpu);

    case 0x80: /* ADD A,B */
      return ADD_b(cpu);

    case 0x81: /* ADD A,C */
      return ADD_c(cpu);

    case 0x82: /* ADD A,D */
      return ADD_d(cpu);

    case 0x83: /* ADD A,E */
      return ADD_e(cpu);

    case 0x84: /* ADD A,H */
      return ADD_h(cpu);

    case 0x85: /* ADD A,L */
      return ADD_l(cpu);

    case 0x86: /* ADD A,(HL) */
      return ADD_$hl(cpu);

    case 0x87: /* ADD A,A */
      return ADD_a(cpu);

    case 0x88: /* ADC A,B */
      return ADC_b(cpu);

    case 0x89: /* ADC A,C */
      return ADC_c(cpu);

    case 0x8A: /* ADC A,D */
      return ADC_d(cpu);

    case 0x8B: /* ADC A,E */
      return ADC_e(cpu);

    case 0x8C: /* ADC A,H */
      return ADC_h(cpu);

    case 0x8D: /* ADC A,L */
      return ADC_l(cpu);

    case 0x8E: /* ADC A,(HL) */
      return ADC_$hl(cpu);

    case 0x8F: /* ADC A,A */
      return ADC_a(cpu);

    case 0x90: /* SUB A,B */
      return SUB_b(cpu);

    case 0x91: /* SUB A,C */
      return SUB_c(cpu);

    case 0x92: /* SUB A,D */
      return SUB_d(cpu);

    case 0x93: /* SUB A,E */
      return SUB_e(cpu);

    case 0x94: /* SUB A,H */
      return SUB_h(cpu);

    case 0x95: /* SUB A,L */
      return SUB_l(cpu);

    case 0x96: /* SUB A,(HL) */
      return SUB_$hl(cpu);

    case 0x97: /* SUB A,A */
      return SUB_a(cpu);

    case 0x98: /* SBC A,B */
      return SBC_b(cpu);

    case 0x99: /* SBC A,C */
      return SBC_c(cpu);

    case 0x9A: /* SBC A,D */
      return SBC_d(cpu);

    case 0x9B: /* SBC A,E */
      return SBC_e(cpu);

    case 0x9C: /* SBC A,H */
      return SBC_h(cpu);

    case 0x9D: /* SBC A,L */
      return SBC_l(cpu);

    case 0x9E: /* SBC A,(HL) */
      return SBC_$hl(cpu);

    case 0x9F: /* SBC A,A */
      return SBC_a(cpu);

    case 0xA0: /* AND B */
      return AND_b(cpu);

    case 0xA1: /* AND C */
      return AND_c(cpu);

    case 0xA2: /* AND D */
      return AND_d(cpu);

    case 0xA3: /* AND E */
      return AND_e(cpu);

    case 0xA4: /* AND H */
      return AND_h(cpu);

    case 0xA5: /* AND L */
      return AND_l(cpu);

    case 0xA6: /* AND (HL) */
      return AND_$hl(cpu);

    case 0xA7: /* AND A */
      return AND_a(cpu);

    case 0xA8: /* XOR B */
      return XOR_b(cpu);

    case 0xA9: /* XOR C */
      return XOR_c(cpu);

    case 0xAA: /* XOR D */
      return XOR_d(cpu);

    case 0xAB: /* XOR E */
      return XOR_e(cpu);

    case 0xAC: /* XOR H */
      return XOR_h(cpu);

    case 0xAD: /* XOR L */
      return XOR_l(cpu);

    case 0xAE: /* XOR (HL) */
      return XOR_$hl(cpu);

    case 0xAF: /* XOR A */
      return XOR_a(cpu);

    case 0xB0: /* OR B */
      return OR_b(cpu);

    case 0xB1: /* OR C */
      return OR_c(cpu);

    case 0xB2: /* OR D */
      return OR_d(cpu);

    case 0xB3: /* OR E */
      return OR_e(cpu);

    case 0xB4: /* OR H */
      return OR_h(cpu);

    case 0xB5: /* OR L */
      return OR_l(cpu);

    case 0xB6: /* OR (HL) */
      return OR_$hl(cpu);

    case 0xB7: /* OR A */
      return OR_a(cpu);

    case 0xB8: /* CP B */
      return CP_b(cpu);

    case 0xB9: /* CP C */
      return CP_c(cpu);

    case 0xBA: /* CP D */
      return CP_d(cpu);

    case 0xBB: /* CP E */
      return CP_e(cpu);

    case 0xBC: /* CP H */
      return CP_h(cpu);

    case 0xBD: /* CP L */
      return CP_l(cpu);

    case 0xBE: /* CP (HL) */
      return CP_$hl(cpu);

    case 0xBF: /* CP A */
      return CP_a(cpu);

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
        return 16;
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
        return 24;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xC5: /* PUSH BC */
      PUSH(cpu, (cpu->regs.b << 8) + cpu->regs.c);
      return 16;

    case 0xC6: /* ADD A,d8 */
      return ADD_d8(cpu);

    case 0xC7: /* RST 00H */
      return RST_0(cpu);

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
      return CB(cpu);

    case 0xCC: /* CALL Z,a16 */
      if (cpuGetFlag(cpu, FLAG_Z)) {
        PUSH(cpu, cpu->regs.pc + 0x2);
        cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
        return 24;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xCD: /* CALL a16 */
      PUSH(cpu, cpu->regs.pc + 0x2);
      cpu->regs.pc = mmuReadWord(cpu->mmu, cpu->regs.pc);
      return 24;

    case 0xCE: /* ADC A,d8 */
      return ADC_d8(cpu);

    case 0xCF: /* RST 08H */
      return RST_0x8(cpu);

    case 0xD0: /* RET NC */
      if (!cpuGetFlag(cpu, FLAG_C)) {
        cpu->regs.pc = POP(cpu);
        return 20;
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
        return 24;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xD5: /* PUSH DE */
      PUSH(cpu, (cpu->regs.d << 8) + cpu->regs.e);
      return 16;

    case 0xD6: /* SUB A,d8 */
      return SUB_d8(cpu);

    case 0xD7: /* RST 10H */
      return RST_0x10(cpu);

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
        return 24;
      }
      cpu->regs.pc += 2;
      return 12;

    case 0xDE: /* SBC A,d8 */
      return SBC_d8(cpu);

    case 0xDF: /* RST 18H */
      return RST_0x18(cpu);

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
      return AND_d8(cpu);

    case 0xE7: /* RST 20H */
      return RST_0x20(cpu);

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
      return XOR_d8(cpu);

    case 0xEF: /* RST 28H */
      return RST_0x28(cpu);

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
      return DI(cpu);

    case 0xF5: /* PUSH AF */
      PUSH(cpu, (cpu->regs.a << 8) + cpu->regs.f);
      return 16;

    case 0xF6: /* OR d8 */
      return OR_d8(cpu);

    case 0xF7: /* RST 30H */
      return RST_0x30(cpu);

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
      return EI(cpu);

    case 0xFE: /* CP d8 */
      return CP_d8(cpu);

    case 0xFF: /* RST 38H */
      return RST_0x38(cpu);
  }

  return 0;
}
