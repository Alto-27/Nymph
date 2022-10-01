#ifndef CPU_H
#define CPU_H

#include <inttypes.h>

#define CARRY_MASK 0x01
#define ZERO_MASK 0x02
#define IRQ_MASK 0x04
#define DECIMAL_MASK 0x08
#define BRK_MASK 0x10
#define UNUSED_MASK 0x20
#define OVERFLOW_MASK 0x40
#define NEGATIVE_MASK 0x80

typedef struct nes {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t pc;
    uint8_t status;
    uint8_t up;
    uint8_t down;
    uint8_t left;
    uint8_t right;
    uint8_t opcode;
    uint64_t cycle;
} nesCPU;

extern nesCPU cpu;

//enum addr_mode { imm, zpg, zpg_X, abs, abs_X, abs_Y, ind_X, ind_Y  };

int interpret(nesCPU * cpu);
void resetCPU(nesCPU * cpu);
void pushStack(nesCPU * cpu, uint8_t value);
uint8_t popStack(nesCPU * cpu);

void updateFlag(nesCPU * cpu, uint8_t condition, uint8_t mask);

void updateNegZero(nesCPU * cpu, uint8_t value);

void ADD(nesCPU * cpu, uint8_t value);

void ADC(nesCPU * cpu, uint16_t addr);

void AND(nesCPU * cpu, uint16_t addr);

void ASL_A(nesCPU * cpu);

void ASL(nesCPU * cpu, uint16_t addr);

int BCC(nesCPU * cpu);

int BCS(nesCPU * cpu);

int BEQ(nesCPU * cpu);

void BIT(nesCPU * cpu, uint16_t addr);

int BMI(nesCPU * cpu);

int BNE(nesCPU * cpu);

int BPL(nesCPU * cpu);

void BRK(nesCPU * cpu);

int BVC(nesCPU * cpu);

int BVS(nesCPU * cpu);

void CLC(nesCPU * cpu);

void CLD(nesCPU * cpu);

void CLI(nesCPU * cpu);

void CLV(nesCPU * cpu);

void CMP(nesCPU * cpu, uint16_t addr);

void CPX(nesCPU * cpu, uint16_t addr);

void CPY(nesCPU * cpu, uint16_t addr);

void DEC(nesCPU * cpu, uint16_t addr);

void DEX(nesCPU * cpu);

void DEY(nesCPU * cpu);

void EOR(nesCPU * cpu, uint16_t addr);

void INC(nesCPU * cpu, uint16_t addr);

void INX(nesCPU * cpu);

void INY(nesCPU * cpu);

void JMP_IND(nesCPU * cpu);

void JMP_ABS(nesCPU * cpu, uint16_t addr);

void JSR(nesCPU * cpu, uint16_t addr);

void LDA(nesCPU * cpu, uint16_t addr);

void LDX(nesCPU * cpu, uint16_t addr);

void LDY(nesCPU * cpu, uint16_t addr);

void LSR_A(nesCPU * cpu);

void LSR(nesCPU * cpu, uint16_t addr);

void NOP(uint16_t addr);

void ORA(nesCPU * cpu, uint16_t addr);

void PHA(nesCPU * cpu);

void PHP(nesCPU * cpu);

void PLA(nesCPU * cpu);

void PLP(nesCPU * cpu);

void ROL_A(nesCPU * cpu);

void ROL(nesCPU * cpu, uint16_t addr);

void ROR_A(nesCPU * cpu);

void ROR(nesCPU * cpu, uint16_t addr);

void RTI(nesCPU * cpu);

void RTS(nesCPU * cpu);

void SBC(nesCPU * cpu, uint16_t addr);

void SEC(nesCPU * cpu);

void SED(nesCPU * cpu);

void SEI(nesCPU * cpu);

void STA(nesCPU * cpu, uint16_t addr);

void STX(nesCPU * cpu, uint16_t addr);

void STY(nesCPU * cpu, uint16_t addr);

void TAX(nesCPU * cpu);

void TAY(nesCPU * cpu);

void TSX(nesCPU * cpu);

void TXA(nesCPU * cpu);

void TXS(nesCPU * cpu);

void TYA(nesCPU * cpu);

/*
    Illegal (but stable) Opcodes

    There are other opcodes that need to be implemented, but it seems lots of semi-functional NES emulators don't include any of the
    illegal opcodes, let alone some of these missing ones

    For the forseeable future, the unstable opcodes will remain unimplemented simply due to the fact
    I'm not trying to make anything super accurate, just something for educational and recreational purposes.

    I might change my mind about implementing the rest of the missing opcodes, but for now I just want to see something work.
*/

// 0x4B ALR
// 0x0B ANC
// 0x2B ANC
// 0x6B ARR

void JAM();

// DEC + CMP
void DCP(nesCPU * cpu, uint16_t addr);

// INC + SBC
void ISC(nesCPU * cpu, uint16_t addr);

// LDA/TSX
// M AND SP -> A, X, SP
void LAS(nesCPU * cpu, uint16_t addr);

// LDA + TAX
// M -> A -> X
void LAX(nesCPU * cpu, uint16_t addr);

// RLA + AND
void RLA(nesCPU * cpu, uint16_t addr);

// ROR + ADC
void RRA(nesCPU * cpu, uint16_t addr);

// A & X -> M
void SAX(nesCPU * cpu, uint16_t addr);

// CMP and DEX, flags set by CMP
// (A AND X) - oper -> X
void SBX(nesCPU * cpu, uint16_t addr);

// ASL + ORA
// Do ASL on M, update carry using M, A OR M -> A, update neg and zero using A
void SLO(nesCPU * cpu, uint16_t addr);

// LSR + EOR
// DO LSR on M, update carry using M, A XOR M -> A, update neg and zero using A
void SRE(nesCPU * cpu, uint16_t addr);

// Same as SBC immediate
void USBC(nesCPU * cpu, uint16_t addr);
#endif