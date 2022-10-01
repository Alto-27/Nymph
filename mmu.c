#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "mmu.h"

#define CPU_MEM_SIZE 0xFFFF // 64 KiB for now, 2 KiB WRAM later
#define PPU_MEM_SIZE 0x4000  // 16384 for now, 2 KiB VRAM later
#define OAM_MEM_SIZE 256

memory_map mmu;
bool pbc = false;

void loadROM(char * filename) {
    // load the rom
}

void loadOpcodes(uint8_t * opcodes, int length) {
    // load opcodes for snake6502
    // set reset vector to 0x8000 so program counter starts in the correct place
    writeRAM(0xFFFC, 0x00);
    writeRAM(0xFFFD, 0x80);
    for (size_t i = 0; i < length; i++) {
        writeRAM(0x8000 + i, opcodes[i]);
    } 
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

uint16_t indirect_X_index(nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc + 1);
    uint16_t address = (rel_addr + cpu->x) & 0xff;
    uint16_t address2 = (rel_addr + cpu->x + 1) & 0xff;
    uint8_t high = readRAM(address2);
    uint8_t low = readRAM(address);
    uint16_t final_addr = (high << 8) | low;
    return final_addr;
}

uint16_t indirect_Y_index(nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    uint8_t low = readRAM(rel_addr);
    uint8_t high = readRAM((rel_addr + 1) & 0xff);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->y) & 0xff00);
    return (address + cpu->y) & 0xffff;
}

uint16_t zero_page(nesCPU * cpu) {
    return readRAM(cpu->pc+1);
}

uint16_t zero_page_X(nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    return (rel_addr + cpu->x) & 0xff;
}

uint16_t zero_page_Y(nesCPU * cpu) {
    uint8_t rel_addr = readRAM(cpu->pc+1);
    return (rel_addr + cpu->y) & 0xff;
}

uint16_t absolute_X(nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc+1);
    uint8_t high = readRAM(cpu->pc+2);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->x) & 0xff00);
    return (address + cpu->x) & 0xffff;
}

uint16_t absolute_Y(nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc+1);
    uint8_t high = readRAM(cpu->pc+2);
    uint16_t address = (high << 8) | low;
    pbc = (address & 0xff00) != ((address + cpu->y) & 0xff00);
    return (address + cpu->y) & 0xffff;
}

uint16_t absolute(nesCPU * cpu) {
    uint8_t low = readRAM(cpu->pc + 1);
    uint8_t high = readRAM(cpu->pc + 2);
    return (high << 8) | low;
}

bool pageBoundaryCross(void) {
    return pbc;
}

void writeRAM(uint16_t address, uint8_t value) {
    // Need to handle mirroring
    mmu.cpu_mem[address] = value;
}

uint8_t readRAM(uint16_t address) {
    return mmu.cpu_mem[address];
}