#include "debug.h"

void debugCPU(CPU *cpu, MMU *mmu, uint8_t opcode)
{
	printf("\n");
	printf("%x: OPERATION: %x (%s)\n", cpu->r->pc, opcode, !cpu->cb ? debugCPUOpcode(opcode) : debugCPUOpcodeCB(opcode));
	printf("\n");
	printf("AF: %x\n", (cpu->r->a << 8) + cpu->r->f);
	printf("BC: %x\n", (cpu->r->b << 8) + cpu->r->c);
	printf("DE: %x\n", (cpu->r->d << 8) + cpu->r->e);
	printf("HL: %x\n", (cpu->r->h << 8) + cpu->r->l);
	printf("SP: %x\n", cpu->r->sp);
	printf("PC: %x\n", cpu->r->pc);
	printf("\n");
	printf("EI: %i ", cpu->ei);
	printf("IME: %i\n", cpu->ime);
	printf("IE: %i ", mmuReadByte(mmu, 0x0FFFF));
	printf("IF: %i\n", mmuReadByte(mmu, 0x0FF0F));
	printf("CB: %i\n", cpu->cb);
	printf("HALT: %i\n", cpu->halt);
	printf("STOP: %i\n", cpu->stop);
	getchar();
}

char *debugCPUOpcode(uint8_t opcode)
{
	if (opcode == 0x00) return "NOP";
	else if (opcode == 0x01) return "LD BC,d16";
	else if (opcode == 0x02) return "LD (BC),A";
	else if (opcode == 0x03) return "INC BC";
	else if (opcode == 0x04) return "INC B";
	else if (opcode == 0x05) return "DEC B";
	else if (opcode == 0x06) return "LD B,d8";
	else if (opcode == 0x07) return "RLCA";
	else if (opcode == 0x08) return "LD (a16),SP";
	else if (opcode == 0x09) return "ADD HL,BC";
	else if (opcode == 0x0A) return "LD A,(BC)";
	else if (opcode == 0x0B) return "DEC BC";
	else if (opcode == 0x0C) return "INC C";
	else if (opcode == 0x0D) return "DEC C";
	else if (opcode == 0x0E) return "LD C,d8";
	else if (opcode == 0x0F) return "RRCA";
	else if (opcode == 0x10) return "STOP 0";
	else if (opcode == 0x11) return "LD DE,d16";
	else if (opcode == 0x12) return "LD (DE),A";
	else if (opcode == 0x13) return "INC DE";
	else if (opcode == 0x14) return "INC D";
	else if (opcode == 0x15) return "DEC D";
	else if (opcode == 0x16) return "LD D,d8";
	else if (opcode == 0x17) return "RLA";
	else if (opcode == 0x18) return "JR r8";
	else if (opcode == 0x19) return "ADD HL,DE";
	else if (opcode == 0x1A) return "LD A,(DE)";
	else if (opcode == 0x1B) return "DEC DE";
	else if (opcode == 0x1C) return "INC E";
	else if (opcode == 0x1D) return "DEC E";
	else if (opcode == 0x1E) return "LD E,d8";
	else if (opcode == 0x1F) return "RRA";
	else if (opcode == 0x20) return "JR NZ,r8";
	else if (opcode == 0x21) return "LD HL,d16";
	else if (opcode == 0x22) return "LD (HL+),A";
	else if (opcode == 0x23) return "INC HL";
	else if (opcode == 0x24) return "INC H";
	else if (opcode == 0x25) return "DEC H";
	else if (opcode == 0x26) return "LD H,d8";
	else if (opcode == 0x27) return "DAA";
	else if (opcode == 0x28) return "JR Z,r8";
	else if (opcode == 0x29) return "ADD HL,HL";
	else if (opcode == 0x2A) return "LD A,(HL+)";
	else if (opcode == 0x2B) return "DEC HL";
	else if (opcode == 0x2C) return "INC L";
	else if (opcode == 0x2D) return "DEC L";
	else if (opcode == 0x2E) return "LD L,d8";
	else if (opcode == 0x2F) return "CPL";
	else if (opcode == 0x30) return "JR NC,r8";
	else if (opcode == 0x31) return "LD SP,d16";
	else if (opcode == 0x32) return "LD (HL-),A";
	else if (opcode == 0x33) return "INC SP";
	else if (opcode == 0x34) return "INC (HL)";
	else if (opcode == 0x35) return "DEC (HL)";
	else if (opcode == 0x36) return "LD (HL),d8";
	else if (opcode == 0x37) return "SCF";
	else if (opcode == 0x38) return "JR C,r8";
	else if (opcode == 0x39) return "ADD HL,SP";
	else if (opcode == 0x3A) return "LD A,(HL-)";
	else if (opcode == 0x3B) return "DEC SP";
	else if (opcode == 0x3C) return "INC A";
	else if (opcode == 0x3D) return "DEC A";
	else if (opcode == 0x3E) return "LD A,d8";
	else if (opcode == 0x3F) return "CCF";
	else if (opcode == 0x40) return "LD B,B";
	else if (opcode == 0x41) return "LD B,C";
	else if (opcode == 0x42) return "LD B,D";
	else if (opcode == 0x43) return "LD B,E";
	else if (opcode == 0x44) return "LD B,H";
	else if (opcode == 0x45) return "LD B,L";
	else if (opcode == 0x46) return "LD B,(HL)";
	else if (opcode == 0x47) return "LD B,A";
	else if (opcode == 0x48) return "LD C,B";
	else if (opcode == 0x49) return "LD C,C";
	else if (opcode == 0x4A) return "LD C,D";
	else if (opcode == 0x4B) return "LD C,E";
	else if (opcode == 0x4C) return "LD C,H";
	else if (opcode == 0x4D) return "LD C,L";
	else if (opcode == 0x4E) return "LD C,(HL)";
	else if (opcode == 0x4F) return "LD C,A";
	else if (opcode == 0x50) return "LD D,B";
	else if (opcode == 0x51) return "LD D,C";
	else if (opcode == 0x52) return "LD D,D";
	else if (opcode == 0x53) return "LD D,E";
	else if (opcode == 0x54) return "LD D,H";
	else if (opcode == 0x55) return "LD D,L";
	else if (opcode == 0x56) return "LD D,(HL)";
	else if (opcode == 0x57) return "LD D,A";
	else if (opcode == 0x58) return "LD E,B";
	else if (opcode == 0x59) return "LD E,C";
	else if (opcode == 0x5A) return "LD E,D";
	else if (opcode == 0x5B) return "LD E,E";
	else if (opcode == 0x5C) return "LD E,H";
	else if (opcode == 0x5D) return "LD E,L";
	else if (opcode == 0x5E) return "LD E,(HL)";
	else if (opcode == 0x5F) return "LD E,A";
	else if (opcode == 0x60) return "LD H,B";
	else if (opcode == 0x61) return "LD H,C";
	else if (opcode == 0x62) return "LD H,D";
	else if (opcode == 0x63) return "LD H,E";
	else if (opcode == 0x64) return "LD H,H";
	else if (opcode == 0x65) return "LD H,L";
	else if (opcode == 0x66) return "LD H,(HL)";
	else if (opcode == 0x67) return "LD H,A";
	else if (opcode == 0x68) return "LD L,B";
	else if (opcode == 0x69) return "LD L,C";
	else if (opcode == 0x6A) return "LD L,D";
	else if (opcode == 0x6B) return "LD L,E";
	else if (opcode == 0x6C) return "LD L,H";
	else if (opcode == 0x6D) return "LD L,L";
	else if (opcode == 0x6E) return "LD L,(HL)";
	else if (opcode == 0x6F) return "LD L,A";
	else if (opcode == 0x70) return "LD (HL),B";
	else if (opcode == 0x71) return "LD (HL),C";
	else if (opcode == 0x72) return "LD (HL),D";
	else if (opcode == 0x73) return "LD (HL),E";
	else if (opcode == 0x74) return "LD (HL),H";
	else if (opcode == 0x75) return "LD (HL),L";
	else if (opcode == 0x76) return "HALT";
	else if (opcode == 0x77) return "LD (HL),A";
	else if (opcode == 0x78) return "LD A,B";
	else if (opcode == 0x79) return "LD A,C";
	else if (opcode == 0x7A) return "LD A,D";
	else if (opcode == 0x7B) return "LD A,E";
	else if (opcode == 0x7C) return "LD A,H";
	else if (opcode == 0x7D) return "LD A,L";
	else if (opcode == 0x7E) return "LD A,(HL)";
	else if (opcode == 0x7F) return "LD A,A";
	else if (opcode == 0x80) return "ADD A,B";
	else if (opcode == 0x81) return "ADD A,C";
	else if (opcode == 0x82) return "ADD A,D";
	else if (opcode == 0x83) return "ADD A,E";
	else if (opcode == 0x84) return "ADD A,H";
	else if (opcode == 0x85) return "ADD A,L";
	else if (opcode == 0x86) return "ADD A,(HL)";
	else if (opcode == 0x87) return "ADD A,A";
	else if (opcode == 0x88) return "ADC A,B";
	else if (opcode == 0x89) return "ADC A,C";
	else if (opcode == 0x8A) return "ADC A,D";
	else if (opcode == 0x8B) return "ADC A,E";
	else if (opcode == 0x8C) return "ADC A,H";
	else if (opcode == 0x8D) return "ADC A,L";
	else if (opcode == 0x8E) return "ADC A,(HL)";
	else if (opcode == 0x8F) return "ADC A,A";
	else if (opcode == 0x90) return "SUB A,B";
	else if (opcode == 0x91) return "SUB A,C";
	else if (opcode == 0x92) return "SUB A,D";
	else if (opcode == 0x93) return "SUB A,E";
	else if (opcode == 0x94) return "SUB A,H";
	else if (opcode == 0x95) return "SUB A,L";
	else if (opcode == 0x96) return "SUB A,(HL)";
	else if (opcode == 0x97) return "SUB A,A";
	else if (opcode == 0x98) return "SBC A,B";
	else if (opcode == 0x99) return "SBC A,C";
	else if (opcode == 0x9A) return "SBC A,D";
	else if (opcode == 0x9B) return "SBC A,E";
	else if (opcode == 0x9C) return "SBC A,H";
	else if (opcode == 0x9D) return "SBC A,L";
	else if (opcode == 0x9E) return "SBC A,(HL)";
	else if (opcode == 0x9F) return "SBC A,A";
	else if (opcode == 0xA0) return "AND B";
	else if (opcode == 0xA1) return "AND C";
	else if (opcode == 0xA2) return "AND D";
	else if (opcode == 0xA3) return "AND E";
	else if (opcode == 0xA4) return "AND H";
	else if (opcode == 0xA5) return "AND L";
	else if (opcode == 0xA6) return "AND (HL)";
	else if (opcode == 0xA7) return "AND A";
	else if (opcode == 0xA8) return "XOR B";
	else if (opcode == 0xA9) return "XOR C";
	else if (opcode == 0xAA) return "XOR D";
	else if (opcode == 0xAB) return "XOR E";
	else if (opcode == 0xAC) return "XOR H";
	else if (opcode == 0xAD) return "XOR L";
	else if (opcode == 0xAE) return "XOR (HL)";
	else if (opcode == 0xAF) return "XOR A";
	else if (opcode == 0xB0) return "OR B";
	else if (opcode == 0xB1) return "OR C";
	else if (opcode == 0xB2) return "OR D";
	else if (opcode == 0xB3) return "OR E";
	else if (opcode == 0xB4) return "OR H";
	else if (opcode == 0xB5) return "OR L";
	else if (opcode == 0xB6) return "OR (HL)";
	else if (opcode == 0xB7) return "OR A";
	else if (opcode == 0xB8) return "CP B";
	else if (opcode == 0xB9) return "CP C";
	else if (opcode == 0xBA) return "CP D";
	else if (opcode == 0xBB) return "CP E";
	else if (opcode == 0xBC) return "CP H";
	else if (opcode == 0xBD) return "CP L";
	else if (opcode == 0xBE) return "CP (HL)";
	else if (opcode == 0xBF) return "CP A";
	else if (opcode == 0xC0) return "RET NZ";
	else if (opcode == 0xC1) return "POP BC";
	else if (opcode == 0xC2) return "JP NZ,a16";
	else if (opcode == 0xC3) return "JP a16";
	else if (opcode == 0xC4) return "CALL NZ,a16";
	else if (opcode == 0xC5) return "PUSH BC";
	else if (opcode == 0xC6) return "ADD A,d8";
	else if (opcode == 0xC7) return "RST 00H";
	else if (opcode == 0xC8) return "RET Z";
	else if (opcode == 0xC9) return "RET";
	else if (opcode == 0xCA) return "JP Z,a16";
	else if (opcode == 0xCB) return "PREFIX CB";
	else if (opcode == 0xCC) return "CALL Z,a16";
	else if (opcode == 0xCD) return "CALL a16";
	else if (opcode == 0xCE) return "ADC A,d8";
	else if (opcode == 0xCF) return "RST 08H";
	else if (opcode == 0xD0) return "RET NC";
	else if (opcode == 0xD1) return "POP DE";
	else if (opcode == 0xD2) return "JP NC,a16";
	else if (opcode == 0xD4) return "CALL NC,a16";
	else if (opcode == 0xD5) return "PUSH DE";
	else if (opcode == 0xD6) return "SUB A,d8";
	else if (opcode == 0xD7) return "RST 10H";
	else if (opcode == 0xD8) return "RET C";
	else if (opcode == 0xD9) return "RETI";
	else if (opcode == 0xDA) return "JP C,a16";
	else if (opcode == 0xDC) return "CALL C,a16";
	else if (opcode == 0xDE) return "SBC A,d8";
	else if (opcode == 0xDF) return "RST 18H";
	else if (opcode == 0xE0) return "LDH (a8),A";
	else if (opcode == 0xE1) return "POP HL";
	else if (opcode == 0xE2) return "LD (C),A";
	else if (opcode == 0xE5) return "PUSH HL";
	else if (opcode == 0xE6) return "AND d8";
	else if (opcode == 0xE7) return "RST 20H";
	else if (opcode == 0xE8) return "ADD SP,r8";
	else if (opcode == 0xE9) return "JP (HL)";
	else if (opcode == 0xEA) return "LD (a16),A";
	else if (opcode == 0xEE) return "XOR d8";
	else if (opcode == 0xEF) return "RST 28H";
	else if (opcode == 0xF0) return "LDH A,(a8)";
	else if (opcode == 0xF1) return "POP AF";
	else if (opcode == 0xF2) return "LD A,(C)";
	else if (opcode == 0xF3) return "DI";
	else if (opcode == 0xF5) return "PUSH AF";
	else if (opcode == 0xF6) return "OR d8";
	else if (opcode == 0xF7) return "RST 30H";
	else if (opcode == 0xF8) return "LD HL,SP+r8";
	else if (opcode == 0xF9) return "LD SP,HL";
	else if (opcode == 0xFA) return "LD A,(a16)";
	else if (opcode == 0xFB) return "EI";
	else if (opcode == 0xFE) return "CP d8";
	else if (opcode == 0xFF) return "RST 38H";

	return "";
}

char *debugCPUOpcodeCB(uint8_t opcode)
{
	if (opcode == 0x00) return "RLC B";
	else if (opcode == 0x01) return "RLC C";
	else if (opcode == 0x02) return "RLC D";
	else if (opcode == 0x03) return "RLC E";
	else if (opcode == 0x04) return "RLC H";
	else if (opcode == 0x05) return "RLC L";
	else if (opcode == 0x06) return "RLC (HL)";
	else if (opcode == 0x07) return "RLC A";
	else if (opcode == 0x08) return "RRC B";
	else if (opcode == 0x09) return "RRC C";
	else if (opcode == 0x0A) return "RRC D";
	else if (opcode == 0x0B) return "RRC E";
	else if (opcode == 0x0C) return "RRC H";
	else if (opcode == 0x0D) return "RRC L";
	else if (opcode == 0x0E) return "RRC (HL)";
	else if (opcode == 0x0F) return "RRC A";
	else if (opcode == 0x10) return "RL B";
	else if (opcode == 0x11) return "RL C";
	else if (opcode == 0x12) return "RL D";
	else if (opcode == 0x13) return "RL E";
	else if (opcode == 0x14) return "RL H";
	else if (opcode == 0x15) return "RL L";
	else if (opcode == 0x16) return "RL (HL)";
	else if (opcode == 0x17) return "RL A";
	else if (opcode == 0x18) return "RR B";
	else if (opcode == 0x19) return "RR C";
	else if (opcode == 0x1A) return "RR D";
	else if (opcode == 0x1B) return "RR E";
	else if (opcode == 0x1C) return "RR H";
	else if (opcode == 0x1D) return "RR L";
	else if (opcode == 0x1E) return "RR (HL)";
	else if (opcode == 0x1F) return "RR A";
	else if (opcode == 0x20) return "SLA B";
	else if (opcode == 0x21) return "SLA C";
	else if (opcode == 0x22) return "SLA D";
	else if (opcode == 0x23) return "SLA E";
	else if (opcode == 0x24) return "SLA H";
	else if (opcode == 0x25) return "SLA L";
	else if (opcode == 0x26) return "SLA (HL)";
	else if (opcode == 0x27) return "SLA A";
	else if (opcode == 0x28) return "SRA B";
	else if (opcode == 0x29) return "SRA C";
	else if (opcode == 0x2A) return "SRA D";
	else if (opcode == 0x2B) return "SRA E";
	else if (opcode == 0x2C) return "SRA H";
	else if (opcode == 0x2D) return "SRA L";
	else if (opcode == 0x2E) return "SRA (HL)";
	else if (opcode == 0x2F) return "SRA A";
	else if (opcode == 0x30) return "SWAP B";
	else if (opcode == 0x31) return "SWAP C";
	else if (opcode == 0x32) return "SWAP D";
	else if (opcode == 0x33) return "SWAP E";
	else if (opcode == 0x34) return "SWAP H";
	else if (opcode == 0x35) return "SWAP L";
	else if (opcode == 0x36) return "SWAP (HL)";
	else if (opcode == 0x37) return "SWAP A";
	else if (opcode == 0x38) return "SRL B";
	else if (opcode == 0x39) return "SRL C";
	else if (opcode == 0x3A) return "SRL D";
	else if (opcode == 0x3B) return "SRL E";
	else if (opcode == 0x3C) return "SRL H";
	else if (opcode == 0x3D) return "SRL L";
	else if (opcode == 0x3E) return "SRL (HL)";
	else if (opcode == 0x3F) return "SRL A";
	else if (opcode == 0x40) return "BIT 0,B";
	else if (opcode == 0x41) return "BIT 0,C";
	else if (opcode == 0x42) return "BIT 0,D";
	else if (opcode == 0x43) return "BIT 0,E";
	else if (opcode == 0x44) return "BIT 0,H";
	else if (opcode == 0x45) return "BIT 0,L";
	else if (opcode == 0x46) return "BIT 0,(HL)";
	else if (opcode == 0x47) return "BIT 0,A";
	else if (opcode == 0x48) return "BIT 1,B";
	else if (opcode == 0x49) return "BIT 1,C";
	else if (opcode == 0x4A) return "BIT 1,D";
	else if (opcode == 0x4B) return "BIT 1,E";
	else if (opcode == 0x4C) return "BIT 1,H";
	else if (opcode == 0x4D) return "BIT 1,L";
	else if (opcode == 0x4E) return "BIT 1,(HL)";
	else if (opcode == 0x4F) return "BIT 1,A";
	else if (opcode == 0x50) return "BIT 2,B";
	else if (opcode == 0x51) return "BIT 2,C";
	else if (opcode == 0x52) return "BIT 2,D";
	else if (opcode == 0x53) return "BIT 2,E";
	else if (opcode == 0x54) return "BIT 2,H";
	else if (opcode == 0x55) return "BIT 2,L";
	else if (opcode == 0x56) return "BIT 2,(HL)";
	else if (opcode == 0x57) return "BIT 2,A";
	else if (opcode == 0x58) return "BIT 3,B";
	else if (opcode == 0x59) return "BIT 3,C";
	else if (opcode == 0x5A) return "BIT 3,D";
	else if (opcode == 0x5B) return "BIT 3,E";
	else if (opcode == 0x5C) return "BIT 3,H";
	else if (opcode == 0x5D) return "BIT 3,L";
	else if (opcode == 0x5E) return "BIT 3,(HL)";
	else if (opcode == 0x5F) return "BIT 3,A";
	else if (opcode == 0x60) return "BIT 4,B";
	else if (opcode == 0x61) return "BIT 4,C";
	else if (opcode == 0x62) return "BIT 4,D";
	else if (opcode == 0x63) return "BIT 4,E";
	else if (opcode == 0x64) return "BIT 4,H";
	else if (opcode == 0x65) return "BIT 4,L";
	else if (opcode == 0x66) return "BIT 4,(HL)";
	else if (opcode == 0x67) return "BIT 4,A";
	else if (opcode == 0x68) return "BIT 5,B";
	else if (opcode == 0x69) return "BIT 5,C";
	else if (opcode == 0x6A) return "BIT 5,D";
	else if (opcode == 0x6B) return "BIT 5,E";
	else if (opcode == 0x6C) return "BIT 5,H";
	else if (opcode == 0x6D) return "BIT 5,L";
	else if (opcode == 0x6E) return "BIT 5,(HL)";
	else if (opcode == 0x6F) return "BIT 5,A";
	else if (opcode == 0x70) return "BIT 6,B";
	else if (opcode == 0x71) return "BIT 6,C";
	else if (opcode == 0x72) return "BIT 6,D";
	else if (opcode == 0x73) return "BIT 6,E";
	else if (opcode == 0x74) return "BIT 6,H";
	else if (opcode == 0x75) return "BIT 6,L";
	else if (opcode == 0x76) return "BIT 6,(HL)";
	else if (opcode == 0x77) return "BIT 6,A";
	else if (opcode == 0x78) return "BIT 7,B";
	else if (opcode == 0x79) return "BIT 7,C";
	else if (opcode == 0x7A) return "BIT 7,D";
	else if (opcode == 0x7B) return "BIT 7,E";
	else if (opcode == 0x7C) return "BIT 7,H";
	else if (opcode == 0x7D) return "BIT 7,L";
	else if (opcode == 0x7E) return "BIT 7,(HL)";
	else if (opcode == 0x7F) return "BIT 7,A";
	else if (opcode == 0x80) return "RES 0,B";
	else if (opcode == 0x81) return "RES 0,C";
	else if (opcode == 0x82) return "RES 0,D";
	else if (opcode == 0x83) return "RES 0,E";
	else if (opcode == 0x84) return "RES 0,H";
	else if (opcode == 0x85) return "RES 0,L";
	else if (opcode == 0x86) return "RES 0,(HL)";
	else if (opcode == 0x87) return "RES 0,A";
	else if (opcode == 0x88) return "RES 1,B";
	else if (opcode == 0x89) return "RES 1,C";
	else if (opcode == 0x8A) return "RES 1,D";
	else if (opcode == 0x8B) return "RES 1,E";
	else if (opcode == 0x8C) return "RES 1,H";
	else if (opcode == 0x8D) return "RES 1,L";
	else if (opcode == 0x8E) return "RES 1,(HL)";
	else if (opcode == 0x8F) return "RES 1,A";
	else if (opcode == 0x90) return "RES 2,B";
	else if (opcode == 0x91) return "RES 2,C";
	else if (opcode == 0x92) return "RES 2,D";
	else if (opcode == 0x93) return "RES 2,E";
	else if (opcode == 0x94) return "RES 2,H";
	else if (opcode == 0x95) return "RES 2,L";
	else if (opcode == 0x96) return "RES 2,(HL)";
	else if (opcode == 0x97) return "RES 2,A";
	else if (opcode == 0x98) return "RES 3,B";
	else if (opcode == 0x99) return "RES 3,C";
	else if (opcode == 0x9A) return "RES 3,D";
	else if (opcode == 0x9B) return "RES 3,E";
	else if (opcode == 0x9C) return "RES 3,H";
	else if (opcode == 0x9D) return "RES 3,L";
	else if (opcode == 0x9E) return "RES 3,(HL)";
	else if (opcode == 0x9F) return "RES 3,A";
	else if (opcode == 0xA0) return "RES 4,B";
	else if (opcode == 0xA1) return "RES 4,C";
	else if (opcode == 0xA2) return "RES 4,D";
	else if (opcode == 0xA3) return "RES 4,E";
	else if (opcode == 0xA4) return "RES 4,H";
	else if (opcode == 0xA5) return "RES 4,L";
	else if (opcode == 0xA6) return "RES 4,(HL)";
	else if (opcode == 0xA7) return "RES 4,A";
	else if (opcode == 0xA8) return "RES 5,B";
	else if (opcode == 0xA9) return "RES 5,C";
	else if (opcode == 0xAA) return "RES 5,D";
	else if (opcode == 0xAB) return "RES 5,E";
	else if (opcode == 0xAC) return "RES 5,H";
	else if (opcode == 0xAD) return "RES 5,L";
	else if (opcode == 0xAE) return "RES 5,(HL)";
	else if (opcode == 0xAF) return "RES 5,A";
	else if (opcode == 0xB0) return "RES 6,B";
	else if (opcode == 0xB1) return "RES 6,C";
	else if (opcode == 0xB2) return "RES 6,D";
	else if (opcode == 0xB3) return "RES 6,E";
	else if (opcode == 0xB4) return "RES 6,H";
	else if (opcode == 0xB5) return "RES 6,L";
	else if (opcode == 0xB6) return "RES 6,(HL)";
	else if (opcode == 0xB7) return "RES 6,A";
	else if (opcode == 0xB8) return "RES 7,B";
	else if (opcode == 0xB9) return "RES 7,C";
	else if (opcode == 0xBA) return "RES 7,D";
	else if (opcode == 0xBB) return "RES 7,E";
	else if (opcode == 0xBC) return "RES 7,H";
	else if (opcode == 0xBD) return "RES 7,L";
	else if (opcode == 0xBE) return "RES 7,(HL)";
	else if (opcode == 0xBF) return "RES 7,A";
	else if (opcode == 0xC0) return "SET 0,B";
	else if (opcode == 0xC1) return "SET 0,C";
	else if (opcode == 0xC2) return "SET 0,D";
	else if (opcode == 0xC3) return "SET 0,E";
	else if (opcode == 0xC4) return "SET 0,H";
	else if (opcode == 0xC5) return "SET 0,L";
	else if (opcode == 0xC6) return "SET 0,(HL)";
	else if (opcode == 0xC7) return "SET 0,A";
	else if (opcode == 0xC8) return "SET 1,B";
	else if (opcode == 0xC9) return "SET 1,C";
	else if (opcode == 0xCA) return "SET 1,D";
	else if (opcode == 0xCB) return "SET 1,E";
	else if (opcode == 0xCC) return "SET 1,H";
	else if (opcode == 0xCD) return "SET 1,L";
	else if (opcode == 0xCE) return "SET 1,(HL)";
	else if (opcode == 0xCF) return "SET 1,A";
	else if (opcode == 0xD0) return "SET 2,B";
	else if (opcode == 0xD1) return "SET 2,C";
	else if (opcode == 0xD2) return "SET 2,D";
	else if (opcode == 0xD3) return "SET 2,E";
	else if (opcode == 0xD4) return "SET 2,H";
	else if (opcode == 0xD5) return "SET 2,L";
	else if (opcode == 0xD6) return "SET 2,(HL)";
	else if (opcode == 0xD7) return "SET 2,A";
	else if (opcode == 0xD8) return "SET 3,B";
	else if (opcode == 0xD9) return "SET 3,C";
	else if (opcode == 0xDA) return "SET 3,D";
	else if (opcode == 0xDB) return "SET 3,E";
	else if (opcode == 0xDC) return "SET 3,H";
	else if (opcode == 0xDD) return "SET 3,L";
	else if (opcode == 0xDE) return "SET 3,(HL)";
	else if (opcode == 0xDF) return "SET 3,A";
	else if (opcode == 0xE0) return "SET 4,B";
	else if (opcode == 0xE1) return "SET 4,C";
	else if (opcode == 0xE2) return "SET 4,D";
	else if (opcode == 0xE3) return "SET 4,E";
	else if (opcode == 0xE4) return "SET 4,H";
	else if (opcode == 0xE5) return "SET 4,L";
	else if (opcode == 0xE6) return "SET 4,(HL)";
	else if (opcode == 0xE7) return "SET 4,A";
	else if (opcode == 0xE8) return "SET 5,B";
	else if (opcode == 0xE9) return "SET 5,C";
	else if (opcode == 0xEA) return "SET 5,D";
	else if (opcode == 0xEB) return "SET 5,E";
	else if (opcode == 0xEC) return "SET 5,H";
	else if (opcode == 0xED) return "SET 5,L";
	else if (opcode == 0xEE) return "SET 5,(HL)";
	else if (opcode == 0xEF) return "SET 5,A";
	else if (opcode == 0xF0) return "SET 6,B";
	else if (opcode == 0xF1) return "SET 6,C";
	else if (opcode == 0xF2) return "SET 6,D";
	else if (opcode == 0xF3) return "SET 6,E";
	else if (opcode == 0xF4) return "SET 6,H";
	else if (opcode == 0xF5) return "SET 6,L";
	else if (opcode == 0xF6) return "SET 6,(HL)";
	else if (opcode == 0xF7) return "SET 6,A";
	else if (opcode == 0xF8) return "SET 7,B";
	else if (opcode == 0xF9) return "SET 7,C";
	else if (opcode == 0xFA) return "SET 7,D";
	else if (opcode == 0xFB) return "SET 7,E";
	else if (opcode == 0xFC) return "SET 7,H";
	else if (opcode == 0xFD) return "SET 7,L";
	else if (opcode == 0xFE) return "SET 7,(HL)";
	else if (opcode == 0xFF) return "SET 7,A";

	return "";
}