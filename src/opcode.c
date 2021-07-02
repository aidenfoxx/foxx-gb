#include "opcode.h"

static uint8_t cpuOpcode(CPU*, uint8_t);
static uint8_t cpuOpcodeCB(CPU*, uint8_t);

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

/**
 * Bitshift operations
 */
static uint16_t JR(CPU *cpu, int8_t a)
{
  cpu->regs.pc += a;
  cpu->regs.pc++; // TODO: Should JR handle this? Maybe macro this function

  return 12;
}


static uint8_t RLC(CPU *cpu, uint8_t a)
{
  uint8_t r = (a & 0x80) >> 7;

  cpuSetFlag(cpu, FLAG_C, r);
  a = (a << 1) + r;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t RRC(CPU *cpu, uint8_t a)
{
  uint8_t r = a & 0x1;

  cpuSetFlag(cpu, FLAG_C, r);
  a = (a >> 1) + (r << 7);
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t RL(CPU *cpu, uint8_t a)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);

  cpuSetFlag(cpu, FLAG_C, a & 0x80);
  a = (a << 1) + c;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t RR(CPU *cpu, uint8_t a)
{
  uint8_t c = cpuGetFlag(cpu, FLAG_C);

  cpuSetFlag(cpu, FLAG_C, a & 0x1);
  a = (a >> 1) + (c << 7);
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t SLA(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, a & 0x80);
  a <<= 1;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t SRA(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, a & 0x1);
  a >>= 1;
  a |= (a & 0x40) << 1;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t SWAP(CPU *cpu, uint8_t a)
{
  uint8_t result = (a >> 4) | (a << 4);

  cpuSetFlag(cpu, FLAG_Z, !result);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return result;
}


static uint8_t SRL(CPU *cpu, uint8_t a)
{
  cpuSetFlag(cpu, FLAG_C, a & 0x1);
  a >>= 1;
  cpuSetFlag(cpu, FLAG_Z, !a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);

  return a;
}

static uint8_t BIT(CPU *cpu, uint8_t a, uint8_t b) {
  cpuSetFlag(cpu, FLAG_Z, !(b & (1 << a)));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, true);

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
#define DEF_LD_B_B(A, B)\
static uint8_t LD_##A##_##B(CPU *cpu)\
{\
  cpu->regs.A = cpu->regs.B;\
  return 4;\
}

#define DEF_LD_B_P(A, B, C)\
static uint8_t LD_##A##_##B##C(CPU *cpu)\
{\
  cpu->regs.A = mmuReadByte(cpu->mmu, (cpu->regs.B << 8) + cpu->regs.C);\
  return 8;\
}

#define DEF_LD_P_B(A, B, C)\
static uint8_t LD_##A##B##_##C(CPU *cpu)\
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

/**
 * Arithmetic operations
 */
/* TODO: Do these after macro */
static uint8_t INC(CPU *cpu, uint8_t op)
{
  cpuSetFlag(cpu, FLAG_Z, !(++op));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, !(op & 0xF));

  return op;
}

static uint8_t DEC(CPU *cpu, uint8_t op)
{
  uint8_t result = op - 1;

  cpuSetFlag(cpu, FLAG_Z, !result);
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (result & 0xF) == 0xF);

  return result;
}
/* END TODO: Do these after macro */

static uint8_t ADD_W(CPU *cpu, uint16_t op)
{
  uint16_t hl = (cpu->regs.h << 8) + cpu->regs.l;
  uint32_t result = hl + op;

  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, (hl & 0xFFF) + (op & 0xFFF) > 0xFFF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFFFF);

  cpu->regs.h = result >> 8;
  cpu->regs.l = result;

  return 8;
}

static uint8_t ADD(CPU *cpu, uint8_t op, uint8_t carry)
{
  uint16_t result = cpu->regs.a + op + carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) + (op & 0xF) + carry > 0xF);
  cpuSetFlag(cpu, FLAG_C, result > 0xFF);

  cpu->regs.a = result;

  return 4;
}

static uint8_t SUB(CPU *cpu, uint8_t op, uint8_t carry, bool store)
{
  int16_t result = cpu->regs.a - op - carry;

  cpuSetFlag(cpu, FLAG_Z, !(result & 0xFF));
  cpuSetFlag(cpu, FLAG_N, true);
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.a & 0xF) - (op & 0xF) - carry < 0);
  cpuSetFlag(cpu, FLAG_C, result < 0);

  if (store) {
    cpu->regs.a = result;
  }

  return 4;
}

#define DEF_INC_B(A)\
static uint8_t INC_##A(CPU *cpu)\
{\
  cpu->regs.A++;\
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.A);\
  cpuSetFlag(cpu, FLAG_N, false);\
  cpuSetFlag(cpu, FLAG_H, !(cpu->regs.A & 0xF));\
  return 4;\
}

#define DEF_INC_W(A, B)\
static uint8_t INC_##A##B(CPU *cpu)\
{\
  uint16_t result = (cpu->regs.A << 8) + cpu->regs.B + 1;\
  cpu->regs.A = result >> 8;\
  cpu->regs.B = result;\
  return 8;\
}

#define DEF_INC_SP()\
static uint8_t INC_sp(CPU *cpu)\
{\
  cpu->regs.sp += 1;\
  return 8;\
}

#define DEF_DEC_B(A)\
static uint8_t DEC_##A(CPU *cpu)\
{\
  cpu->regs.A--;\
  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.A);\
  cpuSetFlag(cpu, FLAG_N, true);\
  cpuSetFlag(cpu, FLAG_H, (cpu->regs.A & 0xF) == 0xF);\
  return 4;\
}

#define DEF_DEC_W(A, B)\
static uint8_t DEC_##A##B(CPU *cpu)\
{\
  uint16_t result = (cpu->regs.A << 8) + cpu->regs.B - 1;\
  cpu->regs.A = result >> 8;\
  cpu->regs.B = result;\
  return 8;\
}

#define DEF_DEC_SP()\
static uint8_t DEC_sp(CPU *cpu)\
{\
  cpu->regs.sp--;\
  return 8;\
}

#define DEF_ADD_B(A)\
static uint8_t ADD_##A(CPU *cpu)\
{\
  return ADD(cpu, cpu->regs.A, 0);\
}

#define DEF_ADD_P(A, B)\
static uint8_t ADD_##A##B(CPU *cpu)\
{\
  return ADD(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B), 0) + 4;\
}

#define DEF_ADC_B(A)\
static uint8_t ADC_##A(CPU *cpu)\
{\
  return ADD(cpu, cpu->regs.A, cpuGetFlag(cpu, FLAG_C));\
}

#define DEF_ADC_P(A, B)\
static uint8_t ADC_##A##B(CPU *cpu)\
{\
  return ADD(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B), cpuGetFlag(cpu, FLAG_C)) + 4;\
}

#define DEF_SUB_B(A)\
static uint8_t SUB_##A(CPU *cpu)\
{\
  return SUB(cpu, cpu->regs.A, 0, true);\
}

#define DEF_SUB_P(A, B)\
static uint8_t SUB_##A##B(CPU *cpu)\
{\
  return SUB(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B), 0, true) + 4;\
}

#define DEF_SDC_B(A)\
static uint8_t SDC_##A(CPU *cpu)\
{\
  return SUB(cpu, cpu->regs.A, cpuGetFlag(cpu, FLAG_C), true);\
}

#define DEF_SDC_P(A, B)\
static uint8_t SDC_##A##B(CPU *cpu)\
{\
  return SUB(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B), cpuGetFlag(cpu, FLAG_C), true) + 4;\
}

#define DEF_CP_B(A)\
static uint8_t CP_##A(CPU *cpu)\
{\
  return SUB(cpu, cpu->regs.A, 0, false);\
}

#define DEF_CP_P(A, B)\
static uint8_t CP_##A##B(CPU *cpu)\
{\
  return SUB(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B), 0, false) + 4;\
}

#define DEF_CP_D8()\
static uint8_t CP_d8(CPU *cpu)\
{\
  uint8_t cycles = SUB(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc), 0, false);\
  cpu->regs.pc++;\
  return cycles + 4;\
}

DEF_INC_W(b, c)
DEF_INC_W(d, e)
DEF_INC_W(h, l)
DEF_INC_SP()

DEF_INC_B(b)
DEF_INC_B(c)
DEF_INC_B(d)
DEF_INC_B(e)
DEF_INC_B(h)
DEF_INC_B(l)
DEF_INC_B(a)

DEF_DEC_W(b, c)
DEF_DEC_W(d, e)
DEF_DEC_W(h, l)
DEF_DEC_SP()

DEF_DEC_B(b)
DEF_DEC_B(c)
DEF_DEC_B(d)
DEF_DEC_B(e)
DEF_DEC_B(h)
DEF_DEC_B(l)
DEF_DEC_B(a)

DEF_ADD_B(b)
DEF_ADD_B(c)
DEF_ADD_B(d)
DEF_ADD_B(e)
DEF_ADD_B(h)
DEF_ADD_B(l)
DEF_ADD_P(h, l)
DEF_ADD_B(a)

DEF_ADC_B(b)
DEF_ADC_B(c)
DEF_ADC_B(d)
DEF_ADC_B(e)
DEF_ADC_B(h)
DEF_ADC_B(l)
DEF_ADC_P(h, l)
DEF_ADC_B(a)

DEF_SUB_B(b)
DEF_SUB_B(c)
DEF_SUB_B(d)
DEF_SUB_B(e)
DEF_SUB_B(h)
DEF_SUB_B(l)
DEF_SUB_P(h, l)
DEF_SUB_B(a)

DEF_SDC_B(b)
DEF_SDC_B(c)
DEF_SDC_B(d)
DEF_SDC_B(e)
DEF_SDC_B(h)
DEF_SDC_B(l)
DEF_SDC_P(h, l)
DEF_SDC_B(a)

DEF_CP_B(b)
DEF_CP_B(c)
DEF_CP_B(d)
DEF_CP_B(e)
DEF_CP_B(h)
DEF_CP_B(l)
DEF_CP_P(h, l)
DEF_CP_B(a)

DEF_CP_D8()

/**
 * Bitwise operations
 */
static uint8_t AND(CPU *cpu, uint8_t op)
{
  cpu->regs.a &= op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, true);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

static uint8_t OR(CPU *cpu, uint8_t op)
{
  cpu->regs.a |= op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

static uint8_t XOR(CPU *cpu, uint8_t op)
{
  cpu->regs.a ^= op;

  cpuSetFlag(cpu, FLAG_Z, !cpu->regs.a);
  cpuSetFlag(cpu, FLAG_N, false);
  cpuSetFlag(cpu, FLAG_H, false);
  cpuSetFlag(cpu, FLAG_C, false);

  return 4;
}

#define DEF_AND_B(A)\
static uint8_t AND_##A(CPU *cpu)\
{\
  return AND(cpu, cpu->regs.A);\
}

#define DEF_AND_P(A, B)\
static uint8_t AND_##A##B(CPU *cpu)\
{\
  return AND(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B)) + 4;\
}

#define DEF_OR_B(A)\
static uint8_t OR_##A(CPU *cpu)\
{\
  return OR(cpu, cpu->regs.A);\
}

#define DEF_OR_P(A, B)\
static uint8_t OR_##A##B(CPU *cpu)\
{\
  return OR(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B)) + 4;\
}

#define DEF_XOR_B(A)\
static uint8_t XOR_##A(CPU *cpu)\
{\
  return XOR(cpu, cpu->regs.A);\
}

#define DEF_XOR_P(A, B)\
static uint8_t XOR_##A##B(CPU *cpu)\
{\
  return XOR(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B)) + 4;\
}

DEF_AND_B(b)
DEF_AND_B(c)
DEF_AND_B(d)
DEF_AND_B(e)
DEF_AND_B(h)
DEF_AND_B(l)
DEF_AND_P(h, l)
DEF_AND_B(a)

DEF_OR_B(b)
DEF_OR_B(c)
DEF_OR_B(d)
DEF_OR_B(e)
DEF_OR_B(h)
DEF_OR_B(l)
DEF_OR_P(h, l)
DEF_OR_B(a)

DEF_XOR_B(b)
DEF_XOR_B(c)
DEF_XOR_B(d)
DEF_XOR_B(e)
DEF_XOR_B(h)
DEF_XOR_B(l)
DEF_XOR_P(h, l)
DEF_XOR_B(a)

/**
 * Set operations.
 */
#define DEF_BIT_B_B(A, B)\
static uint8_t BIT_##A##_##B(CPU *cpu)\
{\
  return BIT(cpu, A, cpu->regs.B);\
}

#define DEF_BIT_B_P(A, B, C)\
static uint8_t BIT_##A##_##B##C(CPU *cpu)\
{\
  return BIT(cpu, A, mmuReadByte(cpu->mmu, (cpu->regs.B << 8) + cpu->regs.C)) + 8;\
}

DEF_BIT_B_B(0, b)
DEF_BIT_B_B(0, c)
DEF_BIT_B_B(0, d)
DEF_BIT_B_B(0, e)
DEF_BIT_B_B(0, h)
DEF_BIT_B_B(0, l)
DEF_BIT_B_P(0, h, l)
DEF_BIT_B_B(0, a)

DEF_BIT_B_B(1, b)
DEF_BIT_B_B(1, c)
DEF_BIT_B_B(1, d)
DEF_BIT_B_B(1, e)
DEF_BIT_B_B(1, h)
DEF_BIT_B_B(1, l)
DEF_BIT_B_P(1, h, l)
DEF_BIT_B_B(1, a)

DEF_BIT_B_B(2, b)
DEF_BIT_B_B(2, c)
DEF_BIT_B_B(2, d)
DEF_BIT_B_B(2, e)
DEF_BIT_B_B(2, h)
DEF_BIT_B_B(2, l)
DEF_BIT_B_P(2, h, l)
DEF_BIT_B_B(2, a)

DEF_BIT_B_B(3, b)
DEF_BIT_B_B(3, c)
DEF_BIT_B_B(3, d)
DEF_BIT_B_B(3, e)
DEF_BIT_B_B(3, h)
DEF_BIT_B_B(3, l)
DEF_BIT_B_P(3, h, l)
DEF_BIT_B_B(3, a)

DEF_BIT_B_B(4, b)
DEF_BIT_B_B(4, c)
DEF_BIT_B_B(4, d)
DEF_BIT_B_B(4, e)
DEF_BIT_B_B(4, h)
DEF_BIT_B_B(4, l)
DEF_BIT_B_P(4, h, l)
DEF_BIT_B_B(4, a)

DEF_BIT_B_B(5, b)
DEF_BIT_B_B(5, c)
DEF_BIT_B_B(5, d)
DEF_BIT_B_B(5, e)
DEF_BIT_B_B(5, h)
DEF_BIT_B_B(5, l)
DEF_BIT_B_P(5, h, l)
DEF_BIT_B_B(5, a)

DEF_BIT_B_B(6, b)
DEF_BIT_B_B(6, c)
DEF_BIT_B_B(6, d)
DEF_BIT_B_B(6, e)
DEF_BIT_B_B(6, h)
DEF_BIT_B_B(6, l)
DEF_BIT_B_P(6, h, l)
DEF_BIT_B_B(6, a)

DEF_BIT_B_B(7, b)
DEF_BIT_B_B(7, c)
DEF_BIT_B_B(7, d)
DEF_BIT_B_B(7, e)
DEF_BIT_B_B(7, h)
DEF_BIT_B_B(7, l)
DEF_BIT_B_P(7, h, l)
DEF_BIT_B_B(7, a)

#define DEF_RES_B_B(A, B)\
static uint8_t RES_##A##_##B(CPU *cpu)\
{\
  cpu->regs.B = cpu->regs.B & ~(1 << A);\
  return 8;\
}

#define DEF_RES_B_P(A, B, C)\
static uint8_t RES_##A##_##B##C(CPU *cpu)\
{\
  uint16_t addr = (cpu->regs.B << 8) + cpu->regs.C;\
  mmuWriteByte(cpu->mmu, addr, mmuReadByte(cpu->mmu, addr) & ~(1 << A));\
  return 16;\
}

DEF_RES_B_B(0, b)
DEF_RES_B_B(0, c)
DEF_RES_B_B(0, d)
DEF_RES_B_B(0, e)
DEF_RES_B_B(0, h)
DEF_RES_B_B(0, l)
DEF_RES_B_P(0, h, l)
DEF_RES_B_B(0, a)

DEF_RES_B_B(1, b)
DEF_RES_B_B(1, c)
DEF_RES_B_B(1, d)
DEF_RES_B_B(1, e)
DEF_RES_B_B(1, h)
DEF_RES_B_B(1, l)
DEF_RES_B_P(1, h, l)
DEF_RES_B_B(1, a)

DEF_RES_B_B(2, b)
DEF_RES_B_B(2, c)
DEF_RES_B_B(2, d)
DEF_RES_B_B(2, e)
DEF_RES_B_B(2, h)
DEF_RES_B_B(2, l)
DEF_RES_B_P(2, h, l)
DEF_RES_B_B(2, a)

DEF_RES_B_B(3, b)
DEF_RES_B_B(3, c)
DEF_RES_B_B(3, d)
DEF_RES_B_B(3, e)
DEF_RES_B_B(3, h)
DEF_RES_B_B(3, l)
DEF_RES_B_P(3, h, l)
DEF_RES_B_B(3, a)

DEF_RES_B_B(4, b)
DEF_RES_B_B(4, c)
DEF_RES_B_B(4, d)
DEF_RES_B_B(4, e)
DEF_RES_B_B(4, h)
DEF_RES_B_B(4, l)
DEF_RES_B_P(4, h, l)
DEF_RES_B_B(4, a)

DEF_RES_B_B(5, b)
DEF_RES_B_B(5, c)
DEF_RES_B_B(5, d)
DEF_RES_B_B(5, e)
DEF_RES_B_B(5, h)
DEF_RES_B_B(5, l)
DEF_RES_B_P(5, h, l)
DEF_RES_B_B(5, a)

DEF_RES_B_B(6, b)
DEF_RES_B_B(6, c)
DEF_RES_B_B(6, d)
DEF_RES_B_B(6, e)
DEF_RES_B_B(6, h)
DEF_RES_B_B(6, l)
DEF_RES_B_P(6, h, l)
DEF_RES_B_B(6, a)

DEF_RES_B_B(7, b)
DEF_RES_B_B(7, c)
DEF_RES_B_B(7, d)
DEF_RES_B_B(7, e)
DEF_RES_B_B(7, h)
DEF_RES_B_B(7, l)
DEF_RES_B_P(7, h, l)
DEF_RES_B_B(7, a)

#define DEF_SET_B_B(A, B)\
static uint8_t SET_##A##_##B(CPU *cpu)\
{\
  cpu->regs.B = cpu->regs.B | (1 << A);\
  return 8;\
}

#define DEF_SET_B_P(A, B, C)\
static uint8_t SET_##A##_##B##C(CPU *cpu)\
{\
  uint16_t addr = (cpu->regs.B << 8) + cpu->regs.C;\
  mmuWriteByte(cpu->mmu, addr, mmuReadByte(cpu->mmu, addr) | (1 << A));\
  return 16;\
}

DEF_SET_B_B(0, b)
DEF_SET_B_B(0, c)
DEF_SET_B_B(0, d)
DEF_SET_B_B(0, e)
DEF_SET_B_B(0, h)
DEF_SET_B_B(0, l)
DEF_SET_B_P(0, h, l)
DEF_SET_B_B(0, a)

DEF_SET_B_B(1, b)
DEF_SET_B_B(1, c)
DEF_SET_B_B(1, d)
DEF_SET_B_B(1, e)
DEF_SET_B_B(1, h)
DEF_SET_B_B(1, l)
DEF_SET_B_P(1, h, l)
DEF_SET_B_B(1, a)

DEF_SET_B_B(2, b)
DEF_SET_B_B(2, c)
DEF_SET_B_B(2, d)
DEF_SET_B_B(2, e)
DEF_SET_B_B(2, h)
DEF_SET_B_B(2, l)
DEF_SET_B_P(2, h, l)
DEF_SET_B_B(2, a)

DEF_SET_B_B(3, b)
DEF_SET_B_B(3, c)
DEF_SET_B_B(3, d)
DEF_SET_B_B(3, e)
DEF_SET_B_B(3, h)
DEF_SET_B_B(3, l)
DEF_SET_B_P(3, h, l)
DEF_SET_B_B(3, a)

DEF_SET_B_B(4, b)
DEF_SET_B_B(4, c)
DEF_SET_B_B(4, d)
DEF_SET_B_B(4, e)
DEF_SET_B_B(4, h)
DEF_SET_B_B(4, l)
DEF_SET_B_P(4, h, l)
DEF_SET_B_B(4, a)

DEF_SET_B_B(5, b)
DEF_SET_B_B(5, c)
DEF_SET_B_B(5, d)
DEF_SET_B_B(5, e)
DEF_SET_B_B(5, h)
DEF_SET_B_B(5, l)
DEF_SET_B_P(5, h, l)
DEF_SET_B_B(5, a)

DEF_SET_B_B(6, b)
DEF_SET_B_B(6, c)
DEF_SET_B_B(6, d)
DEF_SET_B_B(6, e)
DEF_SET_B_B(6, h)
DEF_SET_B_B(6, l)
DEF_SET_B_P(6, h, l)
DEF_SET_B_B(6, a)

DEF_SET_B_B(7, b)
DEF_SET_B_B(7, c)
DEF_SET_B_B(7, d)
DEF_SET_B_B(7, e)
DEF_SET_B_B(7, h)
DEF_SET_B_B(7, l)
DEF_SET_B_P(7, h, l)
DEF_SET_B_B(7, a)

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
      cpu->regs.a = RLC(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

    case 0x8: /* LD (a16),SP */
      mmuWriteWord(cpu->mmu, mmuReadWord(cpu->mmu, cpu->regs.pc), cpu->regs.sp);
      cpu->regs.pc += 2;
      return 20;

    case 0x9: /* ADD HL,BC */
      return ADD_W(cpu, (cpu->regs.b << 8) + cpu->regs.c);

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
      cpu->regs.a = RRC(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, 0);
      return 4;

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
      cpu->regs.a = RL(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, false);
      return 4;

    case 0x18: /* JR r8 */
      return JR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc));

    case 0x19: /* ADD HL,DE */
      return ADD_W(cpu, (cpu->regs.d << 8) + cpu->regs.e);

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
      cpu->regs.a = RR(cpu, cpu->regs.a);
      cpuSetFlag(cpu, FLAG_Z, false);
      return 4;

    case 0x20: /* JR NZ,r8 */
      if (!cpuGetFlag(cpu, FLAG_Z)) {
        return JR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc));
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
        return JR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc));
      }
      cpu->regs.pc++;
      return 8;

    case 0x29: /* ADD HL,HL */
      return ADD_W(cpu, (cpu->regs.h << 8) + cpu->regs.l);

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
      if (!cpuGetFlag(cpu, FLAG_C)) {
        return JR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc));
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
      return INC_sp(cpu);

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
      return SCF(cpu);

    case 0x38: /* JR C,r8 */
      if (cpuGetFlag(cpu, FLAG_C)) {
        return JR(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc));
      }
      cpu->regs.pc++;
      return 8;

    case 0x39: /* ADD HL,SP */
      return ADD_W(cpu, cpu->regs.sp);

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
      return ADD_hl(cpu);

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
      return ADC_hl(cpu);

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
      return SUB_hl(cpu);

    case 0x97: /* SUB A,A */
      return SUB_a(cpu);

    case 0x98: /* SBC A,B */
      return SDC_b(cpu);

    case 0x99: /* SBC A,C */
      return SDC_c(cpu);

    case 0x9A: /* SBC A,D */
      return SDC_d(cpu);

    case 0x9B: /* SBC A,E */
      return SDC_e(cpu);

    case 0x9C: /* SBC A,H */
      return SDC_h(cpu);

    case 0x9D: /* SBC A,L */
      return SDC_l(cpu);

    case 0x9E: /* SBC A,(HL) */
      return SDC_hl(cpu);

    case 0x9F: /* SBC A,A */
      return SDC_a(cpu);

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
      return AND_hl(cpu);

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
      return XOR_hl(cpu);

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
      return OR_hl(cpu);

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
      return CP_hl(cpu);

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
      uint8_t cycles = ADD(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc), 0) + 4;
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
      return CB(cpu);

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
      uint8_t cycles = ADD(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc), cpuGetFlag(cpu, FLAG_C)) + 4;
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
      uint8_t cycles = SUB(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc), 0, true) + 4;
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
      uint8_t cycles = SUB(cpu, mmuReadByte(cpu->mmu, cpu->regs.pc), cpuGetFlag(cpu, FLAG_C), true) + 4;
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
      return DI(cpu);

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
      return EI(cpu);

    case 0xFE: /* CP d8 */
      return CP_d8(cpu);

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
      return BIT_0_b(cpu);

    case 0x41: /* BIT 0,C */
      return BIT_0_c(cpu);

    case 0x42: /* BIT 0,D */
      return BIT_0_d(cpu);

    case 0x43: /* BIT 0,E */
      return BIT_0_e(cpu);

    case 0x44: /* BIT 0,H */
      return BIT_0_h(cpu);

    case 0x45: /* BIT 0,L */
      return BIT_0_l(cpu);

    case 0x46: /* BIT 0,(HL) */
      return BIT_0_hl(cpu);

    case 0x47: /* BIT 0,A */
      return BIT_0_a(cpu);

    case 0x48: /* BIT 1,B */
      return BIT_1_b(cpu);

    case 0x49: /* BIT 1,C */
      return BIT_1_c(cpu);

    case 0x4A: /* BIT 1,D */
      return BIT_1_d(cpu);

    case 0x4B: /* BIT 1,E */
      return BIT_1_e(cpu);

    case 0x4C: /* BIT 1,H */
      return BIT_1_h(cpu);

    case 0x4D: /* BIT 1,L */
      return BIT_1_l(cpu);

    case 0x4E: /* BIT 1,(HL) */
      return BIT_1_hl(cpu);

    case 0x4F: /* BIT 1,A */
      return BIT_1_a(cpu);

    case 0x50: /* BIT 2,B */
      return BIT_2_b(cpu);

    case 0x51: /* BIT 2,C */
      return BIT_2_c(cpu);

    case 0x52: /* BIT 2,D */
      return BIT_2_d(cpu);

    case 0x53: /* BIT 2,E */
      return BIT_2_e(cpu);

    case 0x54: /* BIT 2,H */
      return BIT_2_h(cpu);

    case 0x55: /* BIT 2,L */
      return BIT_2_l(cpu);

    case 0x56: /* BIT 2,(HL) */
      return BIT_2_hl(cpu);

    case 0x57: /* BIT 2,A */
      return BIT_2_a(cpu);

    case 0x58: /* BIT 3,B */
      return BIT_3_b(cpu);

    case 0x59: /* BIT 3,C */
      return BIT_3_c(cpu);

    case 0x5A: /* BIT 3,D */
      return BIT_3_d(cpu);

    case 0x5B: /* BIT 3,E */
      return BIT_3_e(cpu);

    case 0x5C: /* BIT 3,H */
      return BIT_3_h(cpu);

    case 0x5D: /* BIT 3,L */
      return BIT_3_l(cpu);

    case 0x5E: /* BIT 3,(HL) */
      return BIT_3_hl(cpu);

    case 0x5F: /* BIT 3,A */
      return BIT_3_a(cpu);

    case 0x60: /* BIT 4,B */
      return BIT_4_b(cpu);

    case 0x61: /* BIT 4,C */
      return BIT_4_c(cpu);

    case 0x62: /* BIT 4,D */
      return BIT_4_d(cpu);

    case 0x63: /* BIT 4,E */
      return BIT_4_e(cpu);

    case 0x64: /* BIT 4,H */
      return BIT_4_h(cpu);

    case 0x65: /* BIT 4,L */
      return BIT_4_l(cpu);

    case 0x66: /* BIT 4,(HL) */
      return BIT_4_hl(cpu);

    case 0x67: /* BIT 4,A */
      return BIT_4_a(cpu);

    case 0x68: /* BIT 5,B */
      return BIT_5_b(cpu);

    case 0x69: /* BIT 5,C */
      return BIT_5_c(cpu);

    case 0x6A: /* BIT 5,D */
      return BIT_5_d(cpu);

    case 0x6B: /* BIT 5,E */
      return BIT_5_e(cpu);

    case 0x6C: /* BIT 5,H */
      return BIT_5_h(cpu);

    case 0x6D: /* BIT 5,L */
      return BIT_5_l(cpu);

    case 0x6E: /* BIT 5,(HL) */
      return BIT_5_hl(cpu);

    case 0x6F: /* BIT 5,A */
      return BIT_5_a(cpu);

    case 0x70: /* BIT 6,B */
      return BIT_6_b(cpu);

    case 0x71: /* BIT 6,C */
      return BIT_6_c(cpu);

    case 0x72: /* BIT 6,D */
      return BIT_6_d(cpu);

    case 0x73: /* BIT 6,E */
      return BIT_6_e(cpu);

    case 0x74: /* BIT 6,H */
      return BIT_6_h(cpu);

    case 0x75: /* BIT 6,L */
      return BIT_6_l(cpu);

    case 0x76: /* BIT 6,(HL) */
      return BIT_6_hl(cpu);

    case 0x77: /* BIT 6,A */
      return BIT_6_a(cpu);

    case 0x78: /* BIT 7,B */
      return BIT_7_b(cpu);

    case 0x79: /* BIT 7,C */
      return BIT_7_c(cpu);

    case 0x7A: /* BIT 7,D */
      return BIT_7_d(cpu);

    case 0x7B: /* BIT 7,E */
      return BIT_7_e(cpu);

    case 0x7C: /* BIT 7,H */
      return BIT_7_h(cpu);

    case 0x7D: /* BIT 7,L */
      return BIT_7_l(cpu);

    case 0x7E: /* BIT 7,(HL) */
      return BIT_7_hl(cpu);

    case 0x7F: /* BIT 7,A */
      return BIT_7_a(cpu);

    case 0x80: /* RES 0,B */
      return RES_0_b(cpu);

    case 0x81: /* RES 0,C */
      return RES_0_c(cpu);

    case 0x82: /* RES 0,D */
      return RES_0_d(cpu);

    case 0x83: /* RES 0,E */
      return RES_0_e(cpu);

    case 0x84: /* RES 0,H */
      return RES_0_h(cpu);

    case 0x85: /* RES 0,L */
      return RES_0_l(cpu);

    case 0x86: /* RES 0,(HL) */
      return RES_0_hl(cpu);

    case 0x87: /* RES 0,A */
      return RES_0_a(cpu);

    case 0x88: /* RES 1,B */
      return RES_1_b(cpu);

    case 0x89: /* RES 1,C */
      return RES_1_c(cpu);

    case 0x8A: /* RES 1,D */
      return RES_1_d(cpu);

    case 0x8B: /* RES 1,E */
      return RES_1_e(cpu);

    case 0x8C: /* RES 1,H */
      return RES_1_h(cpu);

    case 0x8D: /* RES 1,L */
      return RES_1_l(cpu);

    case 0x8E: /* RES 1,(HL) */
      return RES_1_hl(cpu);

    case 0x8F: /* RES 1,A */
      return RES_1_a(cpu);

    case 0x90: /* RES 2,B */
      return RES_2_b(cpu);

    case 0x91: /* RES 2,C */
      return RES_2_c(cpu);

    case 0x92: /* RES 2,D */
      return RES_2_d(cpu);

    case 0x93: /* RES 2,E */
      return RES_2_e(cpu);

    case 0x94: /* RES 2,H */
      return RES_2_h(cpu);

    case 0x95: /* RES 2,L */
      return RES_2_l(cpu);

    case 0x96: /* RES 2,(HL) */
      return RES_2_hl(cpu);

    case 0x97: /* RES 2,A */
      return RES_2_a(cpu);

    case 0x98: /* RES 3,B */
      return RES_3_b(cpu);

    case 0x99: /* RES 3,C */
      return RES_3_c(cpu);

    case 0x9A: /* RES 3,D */
      return RES_3_d(cpu);

    case 0x9B: /* RES 3,E */
      return RES_3_e(cpu);

    case 0x9C: /* RES 3,H */
      return RES_3_h(cpu);

    case 0x9D: /* RES 3,L */
      return RES_3_l(cpu);

    case 0x9E: /* RES 3,(HL) */
      return RES_3_hl(cpu);

    case 0x9F: /* RES 3,A */
      return RES_3_a(cpu);

    case 0xA0: /* RES 4,B */
      return RES_4_b(cpu);

    case 0xA1: /* RES 4,C */
      return RES_4_c(cpu);

    case 0xA2: /* RES 4,D */
      return RES_4_d(cpu);

    case 0xA3: /* RES 4,E */
      return RES_4_e(cpu);

    case 0xA4: /* RES 4,H */
      return RES_4_h(cpu);

    case 0xA5: /* RES 4,L */
      return RES_4_l(cpu);

    case 0xA6: /* RES 4,(HL) */
      return RES_4_hl(cpu);

    case 0xA7: /* RES 4,A */
      return RES_4_a(cpu);

    case 0xA8: /* RES 5,B */
      return RES_5_b(cpu);

    case 0xA9: /* RES 5,C */
      return RES_5_c(cpu);

    case 0xAA: /* RES 5,D */
      return RES_5_d(cpu);

    case 0xAB: /* RES 5,E */
      return RES_5_e(cpu);

    case 0xAC: /* RES 5,H */
      return RES_5_h(cpu);

    case 0xAD: /* RES 5,L */
      return RES_5_l(cpu);

    case 0xAE: /* RES 5,(HL) */
      return RES_5_hl(cpu);

    case 0xAF: /* RES 5,A */
      return RES_5_a(cpu);

    case 0xB0: /* RES 6,B */
      return RES_6_b(cpu);

    case 0xB1: /* RES 6,C */
      return RES_6_c(cpu);

    case 0xB2: /* RES 6,D */
      return RES_6_d(cpu);

    case 0xB3: /* RES 6,E */
      return RES_6_e(cpu);

    case 0xB4: /* RES 6,H */
      return RES_6_h(cpu);

    case 0xB5: /* RES 6,L */
      return RES_6_l(cpu);

    case 0xB6: /* RES 6,(HL) */
      return RES_6_hl(cpu);

    case 0xB7: /* RES 6,A */
      return RES_6_a(cpu);

    case 0xB8: /* RES 7,B */
      return RES_7_b(cpu);

    case 0xB9: /* RES 7,C */
      return RES_7_c(cpu);

    case 0xBA: /* RES 7,D */
      return RES_7_d(cpu);

    case 0xBB: /* RES 7,E */
      return RES_7_e(cpu);

    case 0xBC: /* RES 7,H */
      return RES_7_h(cpu);

    case 0xBD: /* RES 7,L */
      return RES_7_l(cpu);

    case 0xBE: /* RES 7,(HL) */
      return RES_7_hl(cpu);

    case 0xBF: /* RES 7,A */
      return RES_7_a(cpu);

    case 0xC0: /* SET 0,B */
      return SET_0_b(cpu);

    case 0xC1: /* SET 0,C */
      return SET_0_c(cpu);

    case 0xC2: /* SET 0,D */
      return SET_0_d(cpu);

    case 0xC3: /* SET 0,E */
      return SET_0_e(cpu);

    case 0xC4: /* SET 0,H */
      return SET_0_h(cpu);

    case 0xC5: /* SET 0,L */
      return SET_0_l(cpu);

    case 0xC6: /* SET 0,(HL) */
      return SET_0_hl(cpu);

    case 0xC7: /* SET 0,A */
      return SET_0_a(cpu);

    case 0xC8: /* SET 1,B */
      return SET_1_b(cpu);

    case 0xC9: /* SET 1,C */
      return SET_1_c(cpu);

    case 0xCA: /* SET 1,D */
      return SET_1_d(cpu);

    case 0xCB: /* SET 1,E */
      return SET_1_e(cpu);

    case 0xCC: /* SET 1,H */
      return SET_1_h(cpu);

    case 0xCD: /* SET 1,L */
      return SET_1_l(cpu);

    case 0xCE: /* SET 1,(HL) */
      return SET_1_hl(cpu);

    case 0xCF: /* SET 1,A */
      return SET_1_a(cpu);

    case 0xD0: /* SET 2,B */
      return SET_2_b(cpu);

    case 0xD1: /* SET 2,C */
      return SET_2_c(cpu);

    case 0xD2: /* SET 2,D */
      return SET_2_d(cpu);

    case 0xD3: /* SET 2,E */
      return SET_2_e(cpu);

    case 0xD4: /* SET 2,H */
      return SET_2_h(cpu);

    case 0xD5: /* SET 2,L */
      return SET_2_l(cpu);

    case 0xD6: /* SET 2,(HL) */
      return SET_2_hl(cpu);

    case 0xD7: /* SET 2,A */
      return SET_2_a(cpu);

    case 0xD8: /* SET 3,B */
      return SET_3_b(cpu);

    case 0xD9: /* SET 3,C */
      return SET_3_c(cpu);

    case 0xDA: /* SET 3,D */
      return SET_3_d(cpu);

    case 0xDB: /* SET 3,E */
      return SET_3_e(cpu);

    case 0xDC: /* SET 3,H */
      return SET_3_h(cpu);

    case 0xDD: /* SET 3,L */
      return SET_3_l(cpu);

    case 0xDE: /* SET 3,(HL) */
      return SET_3_hl(cpu);

    case 0xDF: /* SET 3,A */
      return SET_3_a(cpu);

    case 0xE0: /* SET 4,B */
      return SET_4_b(cpu);

    case 0xE1: /* SET 4,C */
      return SET_4_c(cpu);

    case 0xE2: /* SET 4,D */
      return SET_4_d(cpu);

    case 0xE3: /* SET 4,E */
      return SET_4_e(cpu);

    case 0xE4: /* SET 4,H */
      return SET_4_h(cpu);

    case 0xE5: /* SET 4,L */
      return SET_4_l(cpu);

    case 0xE6: /* SET 4,(HL) */
      return SET_4_hl(cpu);

    case 0xE7: /* SET 4,A */
      return SET_4_a(cpu);

    case 0xE8: /* SET 5,B */
      return SET_5_b(cpu);

    case 0xE9: /* SET 5,C */
      return SET_5_c(cpu);

    case 0xEA: /* SET 5,D */
      return SET_5_d(cpu);

    case 0xEB: /* SET 5,E */
      return SET_5_e(cpu);

    case 0xEC: /* SET 5,H */
      return SET_5_h(cpu);

    case 0xED: /* SET 5,L */
      return SET_5_l(cpu);

    case 0xEE: /* SET 5,(HL) */
      return SET_5_hl(cpu);

    case 0xEF: /* SET 5,A */
      return SET_5_a(cpu);

    case 0xF0: /* SET 6,B */
      return SET_6_b(cpu);

    case 0xF1: /* SET 6,C */
      return SET_6_c(cpu);

    case 0xF2: /* SET 6,D */
      return SET_6_d(cpu);

    case 0xF3: /* SET 6,E */
      return SET_6_e(cpu);

    case 0xF4: /* SET 6,H */
      return SET_6_h(cpu);

    case 0xF5: /* SET 6,L */
      return SET_6_l(cpu);

    case 0xF6: /* SET 6,(HL) */
      return SET_6_hl(cpu);

    case 0xF7: /* SET 6,A */
      return SET_6_a(cpu);

    case 0xF8: /* SET 7,B */
      return SET_7_b(cpu);

    case 0xF9: /* SET 7,C */
      return SET_7_c(cpu);

    case 0xFA: /* SET 7,D */
      return SET_7_d(cpu);

    case 0xFB: /* SET 7,E */
      return SET_7_e(cpu);

    case 0xFC: /* SET 7,H */
      return SET_7_h(cpu);

    case 0xFD: /* SET 7,L */
      return SET_7_l(cpu);

    case 0xFE: /* SET 7,(HL) */
      return SET_7_hl(cpu);

    case 0xFF: /* SET 7,A */
      return SET_7_a(cpu);
  }

  return 0;
}
