#include <stdint.h>

#include "cpu.h"

/*inline static uint8_t ADD(CPU *cpu, uint8_t op) {
  return 0x41;
}

#define DEF_OP_B(OP, A)                \
static inline uint8_t OP##_##A(CPU *cpu)      \
{                                           \
  return OP(cpu, cpu->regs.A); \
}


#define DEF_OP_REF(OP, A, B)                                                         \
static inline uint8_t OP##_REF_##A##B(CPU *cpu)                                                 \
{                                                                                       \
  return OP(cpu, mmuReadByte(cpu->mmu, (cpu->regs.A << 8) + cpu->regs.B)); \
}

#define DEF_OP_D8(OP)              \
static inline uint8_t OP##_d8(CPU *cpu) \
{                                       \
  return OP(cpu, 8);       \
}

DEF_OP_B(ADD, a)

DEF_OP_B(ADD, a)
DEF_OP_B(ADD, b)
DEF_OP_B(ADD, c)
DEF_OP_B(ADD, d)
DEF_OP_B(ADD, e)
DEF_OP_B(ADD, h)
DEF_OP_B(ADD, l)

DEF_OP_REF(ADD, h, l)

DEF_OP_D8(ADD)*/
