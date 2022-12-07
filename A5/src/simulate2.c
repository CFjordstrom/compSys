#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>

// create registers as struct or array of 32 ints
int x[32];

// returns base to the power of exponent
int power(int base, int exponent) {
    int product = 1;

    for (int i = 0; i < exponent; i++) {
        product *= base;
    }
    return product;
}

// returns instruction field from end bit to start bit including both end and start
// example: call to get_ins_field(ins, 6, 0) will get opcode
int get_insn_field(int insn, int end, int start) {
    int length = end-start+1;
    int mask = power(2, length)-1;
    return mask & (insn >> start);
}

int get_type(int opcode) {
    switch(opcode) {
        case 0x33:
            return R;

        case (0x3 || 0x13 || 0x67):
            return I;

        case 0x23:
            return S;

        case 0x63:
            return B;

        case 0x37:
            return U;
        
        case 0x6f:
            return J;

        default:
            printf("Malformed opcode\n");
            exit(1);
    }
}

void set_signals(int type, int Branch, int MemRead, int MemToReg, int ALUOp0, int ALUOp1, int MemWrite, int ALUSrc, int RegWrite) {
    switch(type) {
        case R:
            Branch = 0;
            MemRead = 0;
            MemToReg = 0;
            ALUOp0 = 0;
            ALUOp1 = 1;
            MemWrite = 0;
            ALUSrc = 0;
            RegWrite = 1;
            break;

        case I:
            Branch = 0;
            MemRead = 1;
            MemToReg = 1;
            ALUOp0 = 0;
            ALUSrc = 1;
            MemWrite = 0;
            
            break;

        case S:
            MemWrite = 1;
            ALUSrc = 1;
            break;

        case B:
            Branch = 1;
            ALUOp0 = 1;
            break;

        case U:
            break;

        case J:
            break;

        default:
            printf("Malformed signal\n");
            exit(1);
    }
}

void set_imm_get(int type, int insn){

    return 0;
}

void set_ALU_ctrl(int ALUOp0, int ALUOp1, int insn, int funct3, int ALU_control){
    if(ALUOp0 == 0 && ALUOp1 == 0){
        ALU_control = 2;
    }
    else if(ALUOp0 == 0 && ALUOp1 == 1){
        ALU_control = 6;
    }
    else if (ALUOp0 == 1 && ALUOp1 == 0)
    {
        int funct7_30 = get_insn_field(insn, 30, 30);
        if (funct7_30 == 0){
            switch (funct3)
            {
            case 0:
                ALU_control = 2;
                break;
            case 7:
                ALU_control = 0;
                break;
            case 6:
                ALU_control = 1;
                break;
            default:
                printf("Invalid ALUOp.");
                break;
            }
        }else{
            ALU_control = 6;
        }
    }
    return 0;
}

void set_pc(int Branch, int ALU, int ImmGen, int PC){
    if(Branch == 1 && ALU == 0){
        PC = ImmGen + PC;
    }else{
        PC += 4;
    }
}


long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

    // fetch instruction
    int PC = start_addr;
    int insn = memory_rd_w(mem, PC);
    const char* insn_a = assembly_get(as, PC);
    printf("instruction = %s\n", insn_a);
    int opcode = get_insn_field(insn, 6, 0);
    int rd = get_insn_field(insn, 11, 7);
    int funct3 = get_insn_field(insn, 14, 12);
    int rs1 = get_insn_field(insn, 19, 15);
    int rs2 = get_insn_field(insn, 24, 20);
    int funct7 = get_insn_field(insn, 31, 25);
    printf("PC = 0x%x\ninsn = 0x%x\nopcode = 0x%x\nrd = 0x%x\nfunct3 = 0x%x\nrs1 = 0x%x\nrs2 = 0x%x\nfunct7 = 0x%x\n", PC, insn, opcode, rd, funct3, rs1, rs2, funct7);

    int type = get_type(opcode);
    printf("type = %i\n", type);
    int Branch = 0;
    int MemRead = 0;
    int MemToReg = 0;
    int ALUOp0 = 0;
    int ALUOp1 = 0;
    int MemWrite = 0;
    int ALUSrc = 0;
    int RegWrite = 0;

    int ImmGen = 0;
    int ALU_control = 0;
    int ALU = 0;

    // set PC
    set_pc(Branch, ALU, ImmGen, PC);

    // decode instruction, set signals
    set_signals(type, Branch, MemRead, MemToReg, ALUOp0, ALUOp1, MemWrite, ALUSrc, RegWrite);

    // generate immediates
    set_imm_get(type, insn);

    // set ALU control
    set_ALU_ctrl(ALUOp0, ALUOp1, insn, funct3, ALU_control);
    
    // execute ALU

    // memory access

    // register write


    return 0;
}
