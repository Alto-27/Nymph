#include <stdio.h>
#include <inttypes.h>
#include "mmu.h"
#include "cpu.h"

int main(int argc, char * argv[]) {

    uint8_t opcodes[5] = {0xa9, 0xc0, 0xaa, 0xe8, 0x00};
    /*
        LDA #$c0    ; a9 c0
        TAX         ; aa
        INX         ; e8
        BRK         ; 00
    */
    
    init_mmu();
    resetCPU(&cpu);
    
    for(int i = 0; i < 5; i++) {
        writeRAM(cpu.pc + i, opcodes[i]);
    }

    for(int j = 0; j < 4; j++) {
        interpret(&cpu);
    }    
    
    clean_mem();
    return 0;
}