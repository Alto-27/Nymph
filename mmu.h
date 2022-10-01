#ifndef MMU_H
#define MMU_H

#include <inttypes.h>
#include "cpu.h"

typedef struct bus {
    uint8_t * cpu_mem;
    uint8_t * ppu_mem;
    uint8_t * oam;
} memory_map;

typedef enum { false, true } bool;

void loadROM(char * filename);
void init_mmu(void);
void clean_mem(void);
uint16_t indirect_X_index(nesCPU * cpu);
uint16_t indirect_Y_index(nesCPU * cpu);
uint16_t zero_page(nesCPU * cpu);
uint16_t zero_page_X(nesCPU * cpu);
uint16_t zero_page_Y(nesCPU * cpu);
uint16_t absolute(nesCPU * cpu);
uint16_t absolute_X(nesCPU * cpu);
uint16_t absolute_Y(nesCPU * cpu);
bool pageBoundaryCross(void);

void loadOpcodes(uint8_t * opcodes, int length);
void writeRAM(uint16_t address, uint8_t value);
uint8_t readRAM(uint16_t address);

/*
static inline void writeRAM(uint16_t address, uint8_t value) {
    // Need to handle mirroring
    mmu.cpu_mem[address] = value;
}

static inline uint8_t readRAM(uint16_t address) {
    return mmu.cpu_mem[address];
}
*/
#endif