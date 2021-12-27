#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "mmu.h"

#define CPU_MEM_SIZE 0x10000
#define PPU_MEM_SIZE 0x4000
#define OAM_MEM_SIZE 256

struct memory_map mmu;
bool pbc = false;

void loadROM(char * filename) {
    // load the rom
}

void init_mmu(void) {
    pbc = false;
    mmu.cpu_mem = malloc(sizeof(uint8_t) * CPU_MEM_SIZE);
    mmu.ppu_mem = malloc(sizeof(uint8_t) * PPU_MEM_SIZE);
    mmu.oam = malloc(sizeof(uint8_t) * OAM_MEM_SIZE);
    memset(mmu.cpu_mem, 0, sizeof(*mmu.cpu_mem));
    memset(mmu.ppu_mem, 0, sizeof(*mmu.ppu_mem));
    memset(mmu.oam, 0xff, sizeof(*mmu.oam));
}

void clean_mem(void) {
    free(mmu.cpu_mem);
    free(mmu.ppu_mem);
    free(mmu.oam);
}

uint16_t indirect_X_index(struct nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc + 1);
    uint16_t address = (rel_addr + cpu->x) & 0xff;
    uint16_t address2 = (rel_addr + cpu->x + 1) & 0xff;
    uint8_t high = readRAM(address2);
    uint8_t low = readRAM(address);
    uint16_t final_addr = (high << 8) | low;
    return final_addr;
}

uint16_t indirect_Y_index(struct nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    uint8_t low = readRAM(rel_addr);
    uint8_t high = readRAM((rel_addr + 1) & 0xff);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->y) & 0xff00);
    return (address + cpu->y) & 0xffff;
}

uint16_t zero_page(struct nesCPU * cpu) {
    return readRAM(cpu->pc+1);
}

uint16_t zero_page_X(struct nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    return (rel_addr + cpu->x) & 0xff;
}

uint16_t zero_page_Y(struct nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    return (rel_addr + cpu->y) & 0xff;
}

uint16_t absolute_X(struct nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc+1);
    uint8_t high = readRAM(cpu->pc+2);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->x) & 0xff00);
    return (address + cpu->x) & 0xffff;
}

uint16_t absolute_Y(struct nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc+1);
    uint8_t high = readRAM(cpu->pc+2);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->y) & 0xff00);
    return (address + cpu->y) & 0xffff;
}

uint16_t absolute(struct nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc + 1);
    uint8_t high = readRAM(cpu->pc + 2);
    return (high << 8) | low;
}

bool pageBoundaryCross(void) {
    return pbc;
}