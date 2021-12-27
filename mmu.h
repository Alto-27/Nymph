#ifndef MMU_H
#define MMU_H

#include <inttypes.h>
#include "cpu.h"

struct memory_map {
    uint8_t * cpu_mem;
    uint8_t * ppu_mem;
    uint8_t * oam;
};

extern struct memory_map mmu;

typedef enum { false, true } bool;

void loadROM(char * filename);
void init_mmu(void);
void clean_mem(void);
uint16_t indirect_X_index(struct nesCPU * cpu);
uint16_t indirect_Y_index(struct nesCPU * cpu);
uint16_t zero_page(struct nesCPU * cpu);
uint16_t zero_page_X(struct nesCPU * cpu);
uint16_t zero_page_Y(struct nesCPU * cpu);
uint16_t absolute(struct nesCPU * cpu);
uint16_t absolute_X(struct nesCPU * cpu);
uint16_t absolute_Y(struct nesCPU * cpu);
bool pageBoundaryCross(void);



static inline void writeRAM(uint16_t address, uint8_t value) {
    // Need to handle mirroring
    mmu.cpu_mem[address] = value;
}

static inline uint8_t readRAM(uint16_t address) {
    return mmu.cpu_mem[address];
}

#endif