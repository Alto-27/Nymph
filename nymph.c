#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "apu.h"
#include "ppu.h"
#include "io.h"

#define W_RES 256
#define H_RES 240
#define SCREEN_NAME "Nymph NES"

typedef enum { false, true } bool;

struct {
    bool running;
    struct {
        unsigned int w;
        unsigned int h;
        const char * name;
        SDL_Window * window;
        SDL_Renderer * renderer;
    } screen;
} Emu = {
    true,
    {
        W_RES,
        H_RES,
        SCREEN_NAME,
        NULL,
        NULL
    }
};

SDL_Event event;

char * test_rom = "nestest.nes";

int lastcyc = 0;
int ppus = 0;

int main(int argc, char * argv[]) {
    
    loadROM(test_rom);
    
    initPPU(test_rom);
    resetCPU();
    initAPU();

    for(;;) {
        if(Emu.running) {
            lastcyc = stepCPU();
            ppus = lastcyc * 3;
            while(ppus--) {
                stepPPU();
            }
            stepAPU(lastcyc);
        }

        handleWindowEvents(event);
    }

    return 1;
}

int getLastCyc(void) {
    return lastcyc;
}

void togglePause(void) {
    Emu.running ^= 1;
}

void setPause(void) {
    Emu.running = false;
}

void resetPause(void) {
    Emu.running = true;
}

