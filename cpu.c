/*
    Sam's 6502 CPU

    Uses the following docs as reference guides (many thanks to them):

    -> www.masswerk.at/6502/6502_instruction_set.html
    -> web.archive.org/web/20210803072316/http://www.obelisk.me.uk/6502/reference.html
    -> users.telenet.be/kim1-6502/6502/proman.html
    -> archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
    -> archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
    -> wiki.nesdev.org/w/index.php

    Also uses many online tutorials/blogs to figure out how to even get started:

    -> Emudev reddit/discord
    -> emudev.org
    -> emudev.de
    -> yizhang82.dev/blog/nes/
    -> bugzmanov.github.io/nes_ebook/chapter_1.html
    -> fms.komkon.org/EMUL8/HOWTO.html
    -> codeslinger.co.uk
    -> emulator101.com
    -> javidx9's YouTube NES Emulator from scratch series


    + Finished all legal opcodes! Woo! (Not tested yet tho...)
    + Finished a good majority of the stable illegal opcodes! (Also not tested...)

    TODO:
    - Finish up the writeRAM memory map so CPU testing can begin
    - Need to start testing against nestest, which will require debug logs
    ? Optimize/Refactor to speed things up if necessary
    ? Experiment with different methods of dispatch: currently uses a switch, maybe try function/jump table or something wild like computed gotos + X-macros
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cpu.h"
#include "mmu.h"

struct nesCPU cpu;

void resetCPU(struct nesCPU * cpu) {
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    uint8_t high = readRAM(0xffd);
    uint8_t low = readRAM(0xffc);
    cpu->pc = (high << 8) | low;     // start pc at reset vector
    cpu->sp = 0xfd;                  // start sp here b/c of nestest
    cpu->status = 0x24;              // set unused and irq disable to true
}

int interpret(struct nesCPU * cpu) {
    int cycles = 0;
    // get next instruction
    // decode instruction
    // execute instruction
    // update pc and cycle count
    // return cycles
    uint8_t opcode = readRAM(cpu->pc);
    switch(opcode) {
        case 0x00:                                  // BRK
            BRK(cpu);
            cycles = 7;
            printf("Opcode: BRK | cycles: %d | PC: %X | flags: %X\n", cycles, cpu->pc, cpu->status);
            break;
        case 0x01:                                  // ORA ind, X
            ORA(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x02:
            JAM();
            break;
        case 0x03:                                  // SLO ind, X
            SLO(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x04:                                  // NOP
            NOP(cpu->pc);
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x05:                                   // ORA zpg
            ORA(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x06:                                  // ASL zpg
            ASL(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x07:                                  // SLO zpg
            SLO(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x08:                                  // PHP impl
            PHP(cpu);
            cycles = 3;
            cpu->pc += 1;
            break;
        case 0x09:                                  // ORA #
            ORA(cpu, cpu->pc+1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x0A:                                  // ASL A
            ASL_A(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x0B:
            // ANC #
            break;
        case 0x0C:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x0D:                                  // ORA abs
            ORA(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x0E:                                  // ASL abs
            ASL(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x0F:                                  // SLO abs
            SLO(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x10:                                  // BPL Branch on Result Plus relative
            cycles = BPL(cpu);
            break;
        case 0x11:                                  // ORA ind, Y
            ORA(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0x12:
            JAM();
            break;
        case 0x13:                                  // SLO ind, Y
            SLO(cpu, indirect_Y_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x14:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x15:                                  // ORA zpg, X
            ORA(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x16:                                  // ASL zpg, X
            ASL(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x17:                                  // SLO zpg, X
            SLO(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x18:                                  // CLC clear carry flag
            CLC(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x19:                                  // ORA abs, Y
            ORA(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5: 4;
            cpu->pc += 3;
            break;
        case 0x1A:                                  // NOP implied
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x1B:                                  // SLO abs, Y
            SLO(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x1C:                                  // NOP
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x1D:                                  // ORA abs, X
            ORA(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x1E:                                  // ASL abs, X
            ASL(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x1F:                                  // SLO abs, X
            SLO(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x20:
            break;
        case 0x21:                                  // AND ind, X
            AND(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x22:
            JAM();
            break;
        case 0x23:                                  // RLA ind, X
            RLA(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x24:                                  // BIT zpg
            BIT(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x25:                                  // AND zpg
            AND(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x26:                                  // ROL zpg
            ROL(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x27:                                  // RLA zpg
            RLA(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x28:                                  // PLP Pull Processor status from stack
            PLP(cpu);
            cycles = 4;
            cpu->pc += 1;
            break;
        case 0x29:                                  // AND imm
            AND(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x2A:                                  // ROL Accumulator
            ROL_A(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x2B:
            break;
        case 0x2C:                                  // BIT abs
            BIT(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x2D:                                  // AND abs
            AND(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x2E:                                  // ROL abs
            ROL(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x2F:                                  // RLA abs
            RLA(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x30:                                  // BMI Branch if minus
            cycles = BMI(cpu);
            break;
        case 0x31:                                  // AND ind, Y
            AND(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0x32:
            JAM();
            break;
        case 0x33:                                  // RLA ind, Y
            RLA(cpu, indirect_Y_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x34:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x35:                                  // AND zpg, X
            AND(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x36:                                  // ROL zpg, X
            ROL(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x37:                                  // RLA zpg, X
            RLA(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x38:                                  // SEC Set Carry Flag
            SEC(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x39:                                  // AND abs, Y
            AND(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x3A:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x3B:                                  // RLA abs, Y
            RLA(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x3C:                                  // NOP
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x3D:                                  // AND abs, X
            AND(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x3E:                                  // ROL abs, X
            ROL(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x3F:                                  // RLA abs, X
            RLA(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x40:                                  // RTI Return from subroutine
            RTI(cpu);
            cycles = 6;
            break;
        case 0x41:                                  // EOR ind, X
            EOR(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x42:
            JAM();
            break;
        case 0x43:                                  // SRE ind, X
            SRE(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x44:                                  // NOP
            NOP(cpu->pc);
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x45:                                  // EOR zpg
            EOR(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x46:                                  // LSR zpg
            LSR(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x47:                                  // SRE zpg
            SRE(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x48:                                  // PHA Push accumulator onto stack
            PHA(cpu);
            cycles = 3;
            cpu->pc += 1;
            break;
        case 0x49:                                  // EOR imm
            EOR(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x4A:                                  // LSR Accumulator
            LSR_A(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x4B:
            break;
        case 0x4C:                                  // JMP abs
            JMP_ABS(cpu, absolute(cpu));
            cycles = 3;
            break;
        case 0x4D:                                  // EOR abs
            EOR(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x4E:                                  // LSR abs
            LSR(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x4F:                                  // SRE abs
            SRE(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x50:                                  // BVC Branch if overflow clear
            cycles = BVC(cpu);
            break;
        case 0x51:                                  // EOR ind, Y
            EOR(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0x52:
            JAM();
            break;
        case 0x53:                                  // SRE ind, Y
            SRE(cpu, indirect_Y_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x54:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x55:                                  // EOR zpg, X
            EOR(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x56:                                  // LSR zpg, X
            LSR(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x57:                                  // SRE zpg, X
            SRE(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x58:                                  // CLI clear interrupt disable
            CLI(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x59:                                  // EOR abs, Y
            EOR(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x5A:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x5B:                                  // SRE abs, Y
            SRE(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x5C:                                  // NOP
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x5D:                                  // EOR abs, X
            EOR(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x5E:                                  // LSR abs, X
            LSR(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x5F:                                  // SRE abs, X
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x60:                                  // RTS Return from Subroutine
            RTS(cpu);
            cycles = 6;
            break;
        case 0x61:                                  // ADC ind, X
            ADC(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x62:
            JAM();
            break;
        case 0x63:                                  // RRA ind, X
            RRA(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x64:                                  // NOP
            NOP(cpu->pc);
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x65:                                  // ADC zpg
            ADC(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x66:                                  // ROR zpg
            ROR(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x67:                                  // RRA zpg
            RRA(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0x68:                                  // PLA Pull accumulator from stack
            PLA(cpu);
            cycles = 4;
            cpu->pc += 1;
            break;
        case 0x69:                                  // ADC imm
            ADC(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x6A:                                  // ROR Accumulator
            ROR_A(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x6B:
            break;
        case 0x6C:                                  // JMP ind
            JMP_IND(cpu);
            cycles = 5;
            break;
        case 0x6D:                                  // ADC abs
            ADC(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x6E:                                  // ROR abs
            ROR(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x6F:                                  // RRA abs
            RRA(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0x70:                                  // BVS Branch if overflow set
            cycles = BVS(cpu);
            break;
        case 0x71:                                  // ADC ind, Y
            ADC(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0x72:
            JAM();
            break;
        case 0x73:                                  // RRA ind, Y
            RRA(cpu, indirect_Y_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0x74:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x75:                                  // ADC zpg, X
            ADC(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x76:                                  // ROR zpg, X
            ROR(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x77:                                  // RRA zpg, X
            RRA(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x78:                                  // SEI Set Interrupt Disable
            SEI(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x79:                                  // ADC abs, Y
            ADC(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x7A:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x7B:                                  // RRA abs, Y
            RRA(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x7C:                                  // NOP
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x7D:                                  // ADC abs, X
            ADC(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0x7E:                                  // ROR abs, X
            ROR(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x7F:                                  // RRA abs, X
            RRA(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0x80:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x81:                                  // STA ind, X
            STA(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x82:                                  // NOP imm
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x83:                                  // SAX ind, X
            SAX(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x84:                                  // STY zpg
            STY(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x85:                                  // STA zpg
            STA(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x86:                                  // STX spg
            STX(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x87:                                  // SAX zpg
            SAX(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0x88:                                  // DEY Decrement Y by one
            DEY(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x89:                                  // NOP imm
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0x8A:                                  // TXA Transfer X to Accumulator
            TXA(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x8B:
            break;
        case 0x8C:                                  // STY abs
            STY(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x8D:                                  // STA abs
            STA(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x8E:                                  // STX abs
            STX(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x8F:                                  // SAX abs
            SAX(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0x90:                                  // BCC Branch if Carry clear
            cycles = BCC(cpu);
            break;
        case 0x91:                                  // STA ind, Y
            STA(cpu, indirect_Y_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0x92:
            JAM();
            break;
        case 0x93:
            break;
        case 0x94:                                  // STY zpg, X
            STY(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x95:                                  // STA zpg, X
            STA(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x96:                                  // STX zpg, Y
            STX(cpu, zero_page_Y(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x97:                                  // SAX zpg, Y
            SAX(cpu, zero_page_Y(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0x98:                                  // TYA Transfer Y to Accumulator
            TYA(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x99:                                  // STA abs, Y
            STA(cpu, absolute_Y(cpu));
            cycles = 5;
            cpu->pc += 3;
            break;
        case 0x9A:                                  // TXS Transfer X to Stack Pointer
            TXS(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0x9B:
            break;
        case 0x9C:
            break;
        case 0x9D:                                  // STA abs, X
            STA(cpu, absolute_X(cpu));
            cycles = 5;
            cpu->pc += 3;
            break;
        case 0x9E:
            break;
        case 0x9F:
            break;
        case 0xA0:                                  // LDY imm
            LDY(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xA1:                                  // LDA X, ind
            LDA(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xA2:                                  // LDX imm
            LDX(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xA3:                                  // LAX ind, X
            LAX(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xA4:                                  // LDY zpg
            LDY(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xA5:                                  // LDA zpg
            LDA(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xA6:                                  // LDX zpg
            LDX(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xA7:                                  // LAX zpg
            LAX(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xA8:                                  // TAY Transfer Accumulator to Y
            TAY(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xA9:                                  // LDA $xx Immediate
            LDA(cpu, cpu->pc+1);
            cycles = 2;
            cpu->pc += 2;
            printf("Opcode: LDA | cycles: %d | PC: %X | flags: %X\n", cycles, cpu->pc, cpu->status);
            break;
        case 0xAA:                                  // TAX Transfer Accumlator to Index X
            TAX(cpu);
            cycles = 2;
            cpu->pc += 1;
            printf("Opcode: TAX | cycles: %d | PC: %X | flags: %X\n", cycles, cpu->pc, cpu->status);
            break;
        case 0xAB:
            break;
        case 0xAC:                                  // LDY abs
            LDY(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xAD:                                  // LDA abs
            LDA(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xAE:                                  // LDX abs
            LDX (cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xAF:                                  // LAX abs
            LAX(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xB0:                                  // BCS Branch if Carry Set
            cycles = BCS(cpu);
            break;
        case 0xB1:                                  // LDA ind, Y
            LDA(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0xB2:
            JAM();
            break;
        case 0xB3:                                  // LAX ind, Y
            LAX(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0xB4:                                  // LDY zpg, X
            LDY(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xB5:                                  // LDA zpg, X
            LDA(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xB6:                                  // LDX zpg, Y
            LDX(cpu, zero_page_Y(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xB7:                                  // LAX zpg, Y
            LAX(cpu, zero_page_Y(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xB8:                                  // CLV Clear Overflow flag
            CLV(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xB9:                                  // LDA abs, Y
            LDA(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xBA:                                  // TSX Transfer Stack Pointer to X
            TSX(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xBB:                                  // LAS abs, Y
            LAS(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xBC:                                  // LDY abs, X
            LDY(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xBD:                                  // LDA abs, X
            LDA(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xBE:                                  // LDX abs, Y
            LDX(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xBF:                                  // LAX abs, Y
            LAX(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xC0:                                  // CPY imm
            CPY(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xC1:                                  // CMP ind, X
            CMP(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xC2:                                  // NOP imm
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xC3:                                  // DCP ind, X
            DCP(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0xC4:                                  // CPY zpg
            CPY(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xC5:                                  // CMP zpg
            CMP(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xC6:                                  // DEC zpg
            DEC(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0xC7:                                  // DCP zpg
            DCP(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0xC8:                                  // INY Increment Y
            INY(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xC9:                                  // CMP imm
            CMP(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xCA:                                  // DEX Decrement X by One
            DEX(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xCB:                                  // SBX imm
            SBX(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xCC:                                  // CPY abs
            CPY(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xCD:                                  // CMP abs
            CMP(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xCE:                                  // DEC abs
            DEC(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0xCF:                                  // DCP abs
            DCP(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0xD0:                                  // BNE Branch if not equal
            cycles = BNE(cpu);
            break;
        case 0xD1:                                  // CMP ind, Y
            CMP(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0xD2:
            JAM();
            break;
        case 0xD3:                                  // DCP ind, Y
            DCP(cpu, indirect_Y_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0xD4:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xD5:                                  // CMP zpg, X
            CMP(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xD6:                                  // DEC zpg, X
            DEC(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xD7:                                  // DCP zpg, X
            DCP(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xD8:                                  // CLD clear decimal mode
            CLD(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xD9:                                  // CMP abs, Y
            CMP(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xDA:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xDB:                                  // DCP abs, Y
            DCP(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0xDC:                                  // NOP
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xDD:                                  // CMP abs, X
            CMP(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xDE:                                  // DEC abs, X
            DEC(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0xDF:                                  // DCP abs, X
            DCP(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0xE0:                                  // CPX imm
            CPX(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xE1:                                  // SBC ind, X
            SBC(cpu, indirect_X_index(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xE2:                                  // NOP imm
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xE3:                                  // ISC ind, X
            ISC(cpu, indirect_X_index(cpu));
            cycles = 8;
            cpu->pc += 2;
            break;
        case 0xE4:                                  // CPX zpg
            CPX(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xE5:                                  // SBC zpg
            SBC(cpu, zero_page(cpu));
            cycles = 3;
            cpu->pc += 2;
            break;
        case 0xE6:                                  // INC zpg
            INC(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0xE7:                                  // ISC zpg
            ISC(cpu, zero_page(cpu));
            cycles = 5;
            cpu->pc += 2;
            break;
        case 0xE8:                                  // INX Increment X
            INX(cpu);
            cycles = 2;
            cpu->pc += 1;
            printf("Opcode: INX | cycles: %d | PC: %X | flags: %X\n", cycles, cpu->pc, cpu->status);
            break;
        case 0xE9:                                  // SBC imm
            SBC(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 2;
            break;
        case 0xEA:                                  // NOP No Operation
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xEB:                                  // USBC imm
            USBC(cpu, cpu->pc + 1);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xEC:                                  // CPX abs
            CPX(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xED:                                  // SBC abs
            SBC(cpu, absolute(cpu));
            cycles = 4;
            cpu->pc += 3;
            break;
        case 0xEE:                                  // INC abs
            INC(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0xEF:                                  // ISC abs
            ISC(cpu, absolute(cpu));
            cycles = 6;
            cpu->pc += 3;
            break;
        case 0xF0:                                  // BEQ Branch if equal
            cycles = BEQ(cpu);
            break;
        case 0xF1:                                  // SBC ind, Y
            SBC(cpu, indirect_Y_index(cpu));
            cycles = (pageBoundaryCross()) ? 6 : 5;
            cpu->pc += 2;
            break;
        case 0xF2:
            JAM();
            break;
        case 0xF3:                                  // ISC ind, Y
            ISC(cpu, indirect_Y_index(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xF4:                                  // NOP
            NOP(cpu->pc);
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xF5:                                  // SBC zpg, X
            SBC(cpu, zero_page_X(cpu));
            cycles = 4;
            cpu->pc += 2;
            break;
        case 0xF6:                                  // INC zpg, X
            INC(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xF7:                                  // ISC zpg, X
            ISC(cpu, zero_page_X(cpu));
            cycles = 6;
            cpu->pc += 2;
            break;
        case 0xF8:                                  // SED Set Decimal Flag
            SED(cpu);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xF9:                                  // SBC abs, Y
            SBC(cpu, absolute_Y(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xFA:                                  // NOP impl
            NOP(cpu->pc);
            cycles = 2;
            cpu->pc += 1;
            break;
        case 0xFB:                                  // ISC abs, Y
            ISC(cpu, absolute_Y(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0xFC:
            NOP(absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xFD:                                  // SBC abs, X
            SBC(cpu, absolute_X(cpu));
            cycles = (pageBoundaryCross()) ? 5 : 4;
            cpu->pc += 3;
            break;
        case 0xFE:                                  // INC abs, X
            INC(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        case 0xFF:                                  // ISC abs, X
            ISC(cpu, absolute_X(cpu));
            cycles = 7;
            cpu->pc += 3;
            break;
        default:
            printf("Unknown opcode!\n");
            cpu->pc += 1;
            break;
    }
    return cycles;
}

void pushStack(struct nesCPU * cpu, uint8_t value) {
    writeRAM(0x100 + cpu->sp, value);
    cpu->sp -= 1;
    cpu->sp &= 0xff;         // probably not necessary
}

uint8_t popStack(struct nesCPU * cpu) {
    cpu->sp += 1;
    cpu->sp &= 0xff;         // probably not necessary
    return readRAM((0x100 + cpu->sp) & 0x1ff);   // to prevent overflow?
}

void updateFlag(struct nesCPU * cpu, uint8_t condition, uint8_t mask) {
    if(condition) {
        cpu->status |= mask;
    } else {
        cpu->status &= ~mask;
    }
}

void updateNegZero(struct nesCPU * cpu, uint8_t value) {
    updateFlag(cpu, value >> 7, NEGATIVE_MASK);
    updateFlag(cpu, value == 0, ZERO_MASK);
}

void ADD(struct nesCPU * cpu, uint8_t value) {
    uint16_t sum = cpu->a + value + (cpu->status & CARRY_MASK);
    uint8_t old_a = cpu->a;
    cpu->a = (uint8_t) sum;
    updateFlag(cpu, (~(old_a ^ value) & (old_a ^ cpu->a) & 0x80), OVERFLOW_MASK);
    updateFlag(cpu, sum > 0xff, CARRY_MASK);
    updateNegZero(cpu, cpu->a);
}

void ADC(struct nesCPU * cpu, uint16_t addr) {
    ADD(cpu, readRAM(addr));
}

void AND(struct nesCPU * cpu, uint16_t addr) {
    cpu->a &= readRAM(addr);
    updateNegZero(cpu, cpu->a);
}

void ASL_A(struct nesCPU * cpu) {
    updateFlag(cpu, cpu->a >> 7, CARRY_MASK);
    cpu->a = cpu->a << 1;
    updateNegZero(cpu, cpu->a);
}

void ASL(struct nesCPU * cpu, uint16_t addr) {
    uint8_t shift = readRAM(addr);
    updateFlag(cpu, shift >> 7, CARRY_MASK);
    shift = shift << 1;
    updateNegZero(cpu, shift);
    writeRAM(addr, shift);
}

int BCC(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(!(cpu->status & CARRY_MASK)) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

int BCS(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(cpu->status & CARRY_MASK) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

int BEQ(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(cpu->status & ZERO_MASK) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

void BIT(struct nesCPU * cpu, uint16_t addr) {
    uint8_t bit_test = readRAM(addr);
    updateFlag(cpu, bit_test & OVERFLOW_MASK, OVERFLOW_MASK);
    updateFlag(cpu, bit_test & NEGATIVE_MASK, NEGATIVE_MASK);
    updateFlag(cpu, (bit_test & cpu->a) == 0, ZERO_MASK);
}

int BMI(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(cpu->status & NEGATIVE_MASK) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

int BNE(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(!(cpu->status & ZERO_MASK)) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

int BPL(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(!(cpu->status & NEGATIVE_MASK)) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

void BRK(struct nesCPU * cpu) {
    pushStack(cpu, (uint8_t) (((cpu->pc + 2) & 0xff00) >> 8));    // need to handle 16 bit push
    pushStack(cpu, (uint8_t) ((cpu->pc + 2) & 0xff));
    pushStack(cpu, cpu->status | 0x30);       // push brk and unused flags set for some reason
    uint8_t low = readRAM(0xFFFE);      // need to handle irq vectors
    uint8_t high = readRAM(0xFFFF);
    cpu->pc = (high << 8) | low;        
    cpu->status |= BRK_MASK;               // however only brk flag is set globally
}

int BVC(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(!(cpu->status & OVERFLOW_MASK)) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

int BVS(struct nesCPU * cpu) {
    int cycles = 2;
    uint16_t rel_addr = 2 + readRAM(cpu->pc + 1) + cpu->pc;
    if(cpu->status & OVERFLOW_MASK) {
        ++cycles;
        if((rel_addr & 0xff00) != (cpu->pc & 0xff00)) {
            ++cycles;
        }
        cpu->pc = rel_addr;
    }
    return cycles;
}

void CLC(struct nesCPU * cpu) {
    cpu->status &= ~CARRY_MASK;
}

void CLD(struct nesCPU * cpu) {
    cpu->status &= ~DECIMAL_MASK;
}

void CLI(struct nesCPU * cpu) {
    cpu->status &= ~IRQ_MASK;
}

void CLV(struct nesCPU * cpu) {
    cpu->status &= ~OVERFLOW_MASK;
}

void CMP(struct nesCPU * cpu, uint16_t addr) {
    uint8_t compare = cpu->a - readRAM(addr);
    updateFlag(cpu, compare >= 0, CARRY_MASK);
    updateNegZero(cpu, compare);
}

void CPX(struct nesCPU * cpu, uint16_t addr) {
    uint8_t compare = cpu->x - readRAM(addr);
    updateFlag(cpu, compare >= 0, CARRY_MASK);
    updateNegZero(cpu, compare);
}

void CPY(struct nesCPU * cpu, uint16_t addr) {
    uint8_t compare = cpu->y - readRAM(addr);
    updateFlag(cpu, compare >= 0, CARRY_MASK);
    updateNegZero(cpu, compare);
}

void DEC(struct nesCPU * cpu, uint16_t addr) {
    uint8_t decrement = readRAM(addr);
    decrement -= 1;
    writeRAM(addr, decrement);
    updateNegZero(cpu, decrement);
}

void DEX(struct nesCPU * cpu) {
    cpu->x -= 1;
    updateNegZero(cpu, cpu->x);
}

void DEY(struct nesCPU * cpu) {
    cpu->y -= 1;
    updateNegZero(cpu, cpu->y);
}

void EOR(struct nesCPU * cpu, uint16_t addr) {
    cpu->a ^= readRAM(addr);
    updateNegZero(cpu, cpu->a);
}

void INC(struct nesCPU * cpu, uint16_t addr) {
    uint8_t newVal = readRAM(addr) + 1;
    writeRAM(addr, newVal);
    updateNegZero(cpu, newVal);
}

void INX(struct nesCPU * cpu) {
    cpu->x += 1;
    updateNegZero(cpu, cpu->x);
}

void INY(struct nesCPU * cpu) {
    cpu->y += 1;
    updateNegZero(cpu, cpu->y);
}

void JMP_IND(struct nesCPU * cpu) {
    uint16_t ind_addr = (readRAM(cpu->pc + 2) << 8) | readRAM(cpu->pc + 1);
    uint8_t lsb = readRAM(ind_addr);
    uint8_t msb = readRAM(ind_addr + 1);
    if(ind_addr & 0x00ff == 0x00ff) {       // For indirect jmp bug on page boundary
        msb = readRAM(ind_addr & 0xff00);
    }
    cpu->pc = (msb << 8) | lsb;
}

void JMP_ABS(struct nesCPU * cpu, uint16_t addr) {
    cpu->pc = addr;
}

void JSR(struct nesCPU * cpu, uint16_t addr) {
    pushStack(cpu, (uint8_t) (((cpu->pc - 1) & 0xff00) >> 8));      // need to handle little endian 16 bit stack push
    pushStack(cpu, (uint8_t) ((cpu->pc - 1) & 0xff));

    cpu->pc = addr;
}

void LDA(struct nesCPU * cpu, uint16_t addr) {
    cpu->a = readRAM(addr);
    updateNegZero(cpu, cpu->a);
}

void LDX(struct nesCPU * cpu, uint16_t addr) {
    cpu->x = readRAM(addr);
    updateNegZero(cpu, cpu->x);
}

void LDY(struct nesCPU * cpu, uint16_t addr) {
    cpu->y = readRAM(addr);
    updateNegZero(cpu, cpu->y);
}

void LSR_A(struct nesCPU * cpu) {
    updateFlag(cpu, cpu->a & 0x1, CARRY_MASK);
    cpu->a = cpu->a >> 1;
    updateNegZero(cpu, cpu->a);
}

void LSR(struct nesCPU * cpu, uint16_t addr) {
    uint8_t shift = readRAM(addr);
    updateFlag(cpu, shift & 0x1, CARRY_MASK);
    shift = shift >> 1;
    updateNegZero(cpu, shift);
    writeRAM(addr, shift);
}

void NOP(uint16_t addr) {}

void ORA(struct nesCPU * cpu, uint16_t addr) {
    cpu->a |= readRAM(addr);
    updateNegZero(cpu, cpu->a);
}

void PHA(struct nesCPU * cpu) {
    pushStack(cpu, cpu->a);
}

void PHP(struct nesCPU * cpu) {
    pushStack(cpu, cpu->status | 0x30);
}

void PLA(struct nesCPU * cpu) {
    cpu->a = popStack(cpu);
    updateNegZero(cpu, cpu->a);
}

void PLP(struct nesCPU * cpu) {
    cpu->status = popStack(cpu);
}

void ROL_A(struct nesCPU * cpu) {
    uint8_t old_carry = cpu->status & CARRY_MASK;
    updateFlag(cpu, cpu->a >> 7, CARRY_MASK);
    cpu->a = cpu->a << 1;
    cpu->a |= old_carry;
    updateNegZero(cpu, cpu->a);
}

void ROL(struct nesCPU * cpu, uint16_t addr) {
    uint8_t old_carry = cpu->status & CARRY_MASK;
    uint8_t rotate = readRAM(addr);
    updateFlag(cpu, rotate >> 7, CARRY_MASK);
    rotate = rotate << 1;
    rotate |= old_carry;
    writeRAM(addr, rotate);
    updateNegZero(cpu, rotate);
}

void ROR_A(struct nesCPU * cpu) {
    updateFlag(cpu, cpu->status & CARRY_MASK, NEGATIVE_MASK);
    uint8_t old_carry = (cpu->status & CARRY_MASK) << 7;
    updateFlag(cpu, cpu->a & 0x1, CARRY_MASK);
    cpu->a = cpu->a >> 1;
    cpu->a |= old_carry;
    updateFlag(cpu, cpu->a == 0, ZERO_MASK);
}

void ROR(struct nesCPU * cpu, uint16_t addr) {
    updateFlag(cpu, cpu->status & CARRY_MASK, NEGATIVE_MASK);
    uint8_t old_carry = (cpu->status & CARRY_MASK) << 7;
    uint8_t rotate = readRAM(addr);
    updateFlag(cpu, rotate & 0x1, CARRY_MASK);
    rotate = rotate >> 1;
    rotate |= old_carry;
    updateFlag(cpu, rotate == 0, ZERO_MASK);
    writeRAM(addr, rotate);
}

void RTI(struct nesCPU * cpu) {
    cpu->status = popStack(cpu);
    uint8_t low = popStack(cpu);
    uint8_t high = popStack(cpu);
    cpu->pc = (high << 8) | low;
}

void RTS(struct nesCPU * cpu) {
    uint8_t low = popStack(cpu);
    uint8_t high = popStack(cpu);
    cpu->pc = (high << 8) | low;
}

void SBC(struct nesCPU * cpu, uint16_t addr) {
    ADD(cpu, ~readRAM(addr));
}

void SEC(struct nesCPU * cpu) {
    cpu->status |= CARRY_MASK;
}

void SED(struct nesCPU * cpu) {
    cpu->status |= DECIMAL_MASK;
}

void SEI(struct nesCPU * cpu) {
    cpu->status |= IRQ_MASK;
}

void STA(struct nesCPU * cpu, uint16_t addr) {
    writeRAM(addr, cpu->a);
}

void STX(struct nesCPU * cpu, uint16_t addr) {
    writeRAM(addr, cpu->x);
}

void STY(struct nesCPU * cpu, uint16_t addr) {
    writeRAM(addr, cpu->y);
}

void TAX(struct nesCPU * cpu) {
    cpu->x = cpu->a;
    updateNegZero(cpu, cpu->x);
}

void TAY(struct nesCPU * cpu) {
    cpu->y = cpu->a;
    updateNegZero(cpu, cpu->y);
}

void TSX(struct nesCPU * cpu) {
    cpu->x = cpu->sp;
    updateNegZero(cpu, cpu->x);
}

void TXA(struct nesCPU * cpu) {
    cpu->a = cpu->x;
    updateNegZero(cpu, cpu->a);
}

void TXS(struct nesCPU * cpu) {
    cpu->sp = cpu->x;
}

void TYA(struct nesCPU * cpu) {
    cpu->a = cpu->y;
    updateNegZero(cpu, cpu->a);
}

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

void JAM() {}

// DEC + CMP
void DCP(struct nesCPU * cpu, uint16_t addr) {
    DEC(cpu, addr);
    CMP(cpu, addr);
}

// INC + SBC
void ISC(struct nesCPU * cpu, uint16_t addr) {
    INC(cpu, addr);
    SBC(cpu, addr);
}

// LDA/TSX
// M AND SP -> A, X, SP
void LAS(struct nesCPU * cpu, uint16_t addr) {
    cpu->sp &= readRAM(addr);
    cpu->a = cpu->sp;
    TSX(cpu);
}

// LDA + TAX
// M -> A -> X
void LAX(struct nesCPU * cpu, uint16_t addr) {
    cpu->a = readRAM(addr);
    cpu->x = cpu->a;
    updateNegZero(cpu, cpu->x);
}

// RLA + AND
void RLA(struct nesCPU * cpu, uint16_t addr) {
    ROL(cpu, addr);
    AND(cpu, addr);
}

// ROR + ADC
void RRA(struct nesCPU * cpu, uint16_t addr) {
    ROR(cpu, addr);
    ADC(cpu, addr);
}

// A & X -> M
void SAX(struct nesCPU * cpu, uint16_t addr) {
    writeRAM(addr, cpu->a & cpu->x);
}

// CMP and DEX, flags set by CMP
// (A AND X) - oper -> X
void SBX(struct nesCPU * cpu, uint16_t addr) {
    uint8_t compare = (cpu->a & cpu->x) - readRAM(addr);
    updateFlag(cpu, compare >= 0, CARRY_MASK);
    updateNegZero(cpu, compare);
    cpu->x = compare;
}

// ASL + ORA
// Do ASL on M, update carry using M, A OR M -> A, update neg and zero using A
void SLO(struct nesCPU * cpu, uint16_t addr) {
    uint8_t shift = readRAM(addr);
    updateFlag(cpu, shift >> 7, CARRY_MASK);
    shift = shift << 1;
    writeRAM(addr, shift);
    cpu->a |= shift;
    updateNegZero(cpu, cpu->a);
}

// LSR + EOR
// DO LSR on M, update carry using M, A XOR M -> A, update neg and zero using A
void SRE(struct nesCPU * cpu, uint16_t addr) {
    uint8_t shift = readRAM(addr);
    updateFlag(cpu, shift & 0x1, CARRY_MASK);
    shift = shift >> 1;
    writeRAM(addr, shift);
    cpu->a ^= shift;
    updateNegZero(cpu, cpu->a);
}

// Same as SBC immediate
void USBC(struct nesCPU * cpu, uint16_t addr) {
    SBC(cpu, addr);
}