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

struct nesCPU {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t pc;
    uint8_t status;
};

extern struct nesCPU cpu;

enum addr_mode { imm, zpg, zpg_X, abs, abs_X, abs_Y, ind_X, ind_Y  };

int interpret(struct nesCPU * cpu);
void resetCPU(struct nesCPU * cpu);
void pushStack(struct nesCPU * cpu, uint8_t value);
uint8_t popStack(struct nesCPU * cpu);

#endif