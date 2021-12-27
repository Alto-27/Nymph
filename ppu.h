#ifndef PPU_H
#define PPU_H

#include <inttypes.h>

typedef struct nymphPPU {
    uint8_t vram[2048];
    uint8_t graphics[8192];
    uint8_t palettes;
} ppu;

void initPPU(char * filename);

#endif