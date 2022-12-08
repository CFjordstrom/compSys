#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>

// create registers as struct or array of 32 ints
int x[32];

enum ALUOp {
    ALU_ADD = 0,
    ALU_SUB = 1,
    ALU_AND = 2,
    ALU_OR = 3,
};

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

/*int get_type(int opcode) {
    switch(opcode) {
        case 0x33:
            return R;

        case 0x3:
            return L;
        
        case 0x13:
            return I;

        case 0x67:
            return JALR;

        case 0x23:
            return S;

        case 0x63:
            return B;

        case 0x37:
            return LUI;
        
        case 0x6f:
            return J;
        
        case 0x73:
            return ECALL;

        default:
            printf("Malformed opcode\n");
            exit(1);
    }
}*/

void set_signals(int opcode, int Branch, int MemRead, int MemToReg, int ALUOp0, int ALUOp1, enum ALUOp ALUOp, int MemWrite, int ALUSrc, int RegWrite) {
    switch(opcode) {
        case LUI:
            RegWrite = 1;
            ALUSrc = 1;
            break;
        
        case AUIPC:
            break;

        case JAL:
            break;

        case JALR:
            break;

        case B:
            Branch = 1;
            ALUOp0 = 1;
            break;

        case L:
            MemRead = 1;
            ALUSrc = 1;
            MemToReg = 1;
            RegWrite = 1;
            break;

        case S:
            ALUSrc = 1;
            //ALUOp = ALU_SUB;
            MemWrite = 1;
            break;

        case I:
            ALUSrc = 1;
            MemRead = 1;
            MemToReg = 1;
            RegWrite = 1;
            break;
        
        case R:
            ALUOp1 = 1;
            //ALUOp = ALU_ADD;
            RegWrite = 1;
            break;

        case ECALL:
            break;

        default:
            printf("Malformed signal\n");
    }
}

void set_imm_get(int insn, int opcode, int rd, int rs1, int rs2, int funct3, int funct7, int ImmGen){
    switch (opcode) {
        case LUI:
            break;
        
        case AUIPC:
            break;

        case JAL:
            break;

        case JALR:
            break;

        case B:
            break;

        case L:
            break;

        case S:
            ImmGen = (funct7 << 5) | rd;
            break;

        case I:
            ImmGen = get_insn_field(insn, 31, 25);
            break;
        
        case R:
            break;

        case ECALL:
            break;

        default:
            break;
    }
}

// needs refactoring
int get_ALU_ctrl(int ALUOp0, int ALUOp1, int bit30, int funct3){
    if(ALUOp0 == 0 && ALUOp1 == 0){
        return ADD;
    }
    else if(ALUOp0 == 0 && ALUOp1 == 1){
        return SUB;
    }
    else if (ALUOp0 == 1 && ALUOp1 == 0)
    {
        if (bit30 == 0){
            switch (funct3)
            {
            case 0x0:
                return ADD;

            case 0x6:
                return OR;

            default:
                return AND;
            }
        }
        else {
            return SUB;
        }
    }
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
    int opcode = get_insn_field(insn, 6, 0);
    int rd = get_insn_field(insn, 11, 7);
    int funct3 = get_insn_field(insn, 14, 12);
    int rs1 = get_insn_field(insn, 19, 15);
    int rs2 = get_insn_field(insn, 24, 20);
    int funct7 = get_insn_field(insn, 31, 25);
    const char* insn_a = assembly_get(as, PC);
    printf("instruction = %s\n", insn_a);
    printf("PC = 0x%x\ninsn = 0x%x\nopcode = 0x%x\nrd = 0x%x\nfunct3 = 0x%x\nrs1 = 0x%x\nrs2 = 0x%x\nfunct7 = 0x%x\n", PC, insn, opcode, rd, funct3, rs1, rs2, funct7);

    int Branch = 0;
    int MemRead = 0;
    int MemToReg = 0;
    int ALUOp0 = 0;
    int ALUOp1 = 0;
    enum ALUOp ALUOp;
    int MemWrite = 0;
    int ALUSrc = 0;
    int RegWrite = 0;

    int ImmGen = 0;
    int ALU_control = 0;
    int ALU = 0;

    // set PC
    set_pc(Branch, ALU, ImmGen, PC);

    // decode instruction, set signals
    set_signals(opcode, Branch, MemRead, MemToReg, ALUOp0, ALUOp1, ALUOp, MemWrite, ALUSrc, RegWrite);

    // generate immediates
    set_imm_get(insn, opcode, rd, rs1, rs2, funct3, funct7, ImmGen);

    // set ALU control
    ALU_control = get_ALU_ctrl(ALUOp0, ALUOp1, get_insn_field(insn, 30, 30), funct3);
    
    // execute ALU

    // memory access

    // register write


    return 0;
}
