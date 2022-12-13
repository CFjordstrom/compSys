#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>

// create registers as struct or array of 32 ints
int x[32];

enum bitsize{byte, halfword, word};

// defines the ALU control signal that is output to the ALU
enum ALU_action {
    ALU_ADD = 0,
    ALU_SUB = 1,
    ALU_AND = 2,
    ALU_OR = 3,
    ALU_SLL = 4,
    ALU_SLT = 5,
    ALU_SLTU = 6,
    ALU_XOR = 7,
    ALU_SRL = 8,
    ALU_SRA = 9,
    ALU_MUL = 10,
    ALU_DIV = 11,
    ALU_REM = 12,
    BRANCH_BEQ = 13,
    BRANCH_BNE = 14,
    BRANCH_BLT = 15,
    BRANCH_BGE = 16,
    BRANCH_BLTU = 17,
    BRANCH_BGEU = 18,
};

// returns base to the power of exponent
int power(int base, int exponent) {
    int product = 1;

    for (int i = 0; i < exponent; i++) {
        product *= base;
    }
    return product;
}

int ecall(int* registers){
    int syscall = registers[7];
    if(syscall == (3 || 93)){
        return -1;
    }
    else if(syscall == 1){
        registers[7] = getchar();
    } else if(syscall == 2){
        putchar(registers[6]);
    }
    return 0;
}

// returns instruction field from end bit to start bit including both end and start
// example: call to get_ins_field(ins, 6, 0) will get opcode
int get_insn_field(int insn, int end, int start) {
    int length = end-start+1;
    int mask = power(2, length)-1;
    return mask & (insn >> start);
}

int sign_extend(int num, int len_bits) {
    int mask = power(2, len_bits+1)-1;
    int mask2 = power(2, len_bits);
    int rest = 0xFFFFFFFF-mask;
    int sign = ((num & mask) | (num & mask2) ? rest : 0);
    return (sign | num);
}

// ALUOp 00 = add, ALUOp 01 = sub, ALUOp 10 = funct
void set_signals(int opcode, int funct3, int* Branch, int* MemRead, int* MemToReg, int* ALUOp0, int* ALUOp1, int* MemWrite, int* ALUSrc, int* RegWrite, enum bitsize* size) {
    switch(opcode) {
        case LUI:
            *RegWrite = 1;
            *ALUSrc = 1;
            break;
        
        case AUIPC:
            *ALUSrc = 1;
            *RegWrite = 1;
            break;

        case JAL:
            *ALUSrc = 1;
            *RegWrite = 1;
            break;

        case JALR:
            *Branch = 1;
            *RegWrite = 1;
            break;

        case B:
            *Branch = 1;
            *ALUOp1 = 1;
            break;

        case L:
            switch (funct3)
            {
            case 0x0:
                *size = byte;
                break;
            case 0x1:
                *size = halfword;
                break;
            case 0x2:
                *size = word;
                break;
            }
            *MemRead = 1;
            *ALUSrc = 1;
            *MemToReg = 1;
            *RegWrite = 1;
            break;

        case S:
            switch (funct3)
            {
            case 0x0:
                *size = byte;
                break;
            case 0x1:
                *size = halfword;
                break;
            case 0x2:
                *size = word;
                break;
            }
            *ALUSrc = 1;
            *MemWrite = 1;
            break;

        case I:
            *ALUSrc = 1;
            *ALUOp1 = 1;
            *RegWrite = 1;
            break;
        
        case RM:
            *ALUOp1 = 1;
            *RegWrite = 1;
            break;

        case ECALL:
            break;

        default:
            printf("Malformed signal\n");
    }
}

// some immediates need first bit set to 30
int get_imm_gen(int insn, int opcode){
    int imm;
    switch (opcode) {
        case LUI:
            return get_insn_field(insn, 31, 12);
        
        case AUIPC:
            return get_insn_field(insn, 31, 12);

        case JAL:
            imm = (get_insn_field(insn, 31, 31) << 20) | (get_insn_field(insn, 30, 21) << 1) | (get_insn_field(insn, 20, 20) << 11) | (get_insn_field(insn, 19, 12) << 12);
            return sign_extend(imm, 20);

        case JALR:
            return get_insn_field(insn, 31, 20);

        case B:
            return (get_insn_field(insn, 31, 31) << 11) | (get_insn_field(insn, 7, 7) << 10) | (get_insn_field(insn, 30, 25) << 4) | get_insn_field(insn, 11, 8);

        case L:
            return get_insn_field(insn, 31, 20);

        case S:
            return get_insn_field(insn, 31, 20);

        case I:
            return get_insn_field(insn, 31, 25);

        default:
            printf("ImmGen error\n");
            return -1;
    }
    return -1;
}

// gets the output of the ALU control based on the ALUOp and the funct3 and funct7 fields, also get addressing size (byte, halfword, word)
int ALU_control(int opcode, int ALUOp1, int ALUOp0, int funct7, int funct3){
    int bit25 = get_insn_field(funct7, 0, 0);
    int bit30 = get_insn_field(funct7, 5, 5);
    if(ALUOp1 == 0 && ALUOp0 == 0){
        return ALU_ADD;
    }
    else if(ALUOp1 == 0 && ALUOp0 == 1){
        return ALU_SUB;
    }
    else if (ALUOp1 == 1 && ALUOp0 == 0)
    {
        if (opcode == B) { // if branch check funct3
            switch (funct3) {
                case 0x0:
                    return BRANCH_BEQ;
                    break;

                case 0x1:
                    return BRANCH_BNE;
                    break;

                case 0x4:
                    return BRANCH_BLT;
                    break;

                case 0x5:
                    return BRANCH_BGE;
                    break;

                case 0x6:
                    return BRANCH_BLTU;
                    break;

                case 0x7:
                    return BRANCH_BGEU;
                    break;
            }
        }
        else if (opcode == RM) { // if R or M check bit 25 or bit 30 to check which one
            if (bit25) { // if M check funct3
                switch (funct3) {
                    case 0x0:
                        return ALU_MUL;

                    case 0x1:
                        return ALU_MUL;

                    case 0x2:
                        return ALU_MUL;

                    case 0x3:
                        return ALU_MUL;

                    case 0x4:
                        return ALU_DIV;

                    case 0x5:
                        return ALU_DIV;

                    case 0x6:
                        return ALU_REM;

                    case 0x7:
                        return ALU_REM;
                }
            }
            else if (bit30){ // else if R with bit30 set check funct3
                switch (funct3) {
                    case 0x0:
                        return ALU_SUB;

                    case 0x5:
                        return ALU_SRA;
                }
            }
            else {  // else R with bit 30 not set check funct3
                switch (funct3) {
                    case 0x0:
                        return ALU_ADD;

                    case 0x1:
                        return ALU_SLL;

                    case 0x2:
                        return ALU_SLT;

                    case 0x3:
                        return ALU_SLTU;

                    case 0x4:
                        return ALU_XOR;
                    
                    case 0x5:
                        return ALU_SRL;

                    case 0x6:
                        return ALU_OR;

                    case 0x7:
                        return ALU_AND;

                    default:
                        return 0;
                }
            }
        }
    }
    return -1;
}

int ALU_execute(int input1, int input2, enum ALU_action ALU_action){
    switch (ALU_action) {
        case ALU_ADD:
            return input1 + input2;

        case ALU_SUB:
            return input1 - input2;

        case ALU_OR:
            return input1 | input2;
        
        case ALU_AND:
            return input1 & input2;

        case ALU_XOR:
            return input1 ^ input2;

        case ALU_SLL:
            return input1 << input2;

        case ALU_SLT:
            return input1 < input2 ? 1 : 0;
        
        case ALU_SRL:
            return (unsigned int)input1 >> input2;

        case ALU_SRA:
            return input1 >> input2;

        case ALU_SLTU:
            return (unsigned int)input1 < (unsigned int)input2 ? 1 : 0;

        case ALU_MUL:
            return input1 * input2;

        case ALU_DIV:
            return input1 / input2;
        
        case ALU_REM:
            return input1 % input2;

        case BRANCH_BEQ:
            return input1 - input2;
        
        case BRANCH_BNE:
            return !(input1 - input2);

        case BRANCH_BLT:
            return !(input1 < input2);

        case BRANCH_BGE:
            return !(input1 >= input2);

        case BRANCH_BLTU:
            return !((unsigned int) input1 < (unsigned int) input2);

        case BRANCH_BGEU:
            return !((unsigned int) input1 >= (unsigned int) input2);

        default:
            break;
    }
    return -1;
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int looping = 0;
    int PC = start_addr;
    while (looping < 3) {
        printf("PC = 0x%x\n", PC);
        // fetch instruction
        int insn = memory_rd_w(mem, PC);
        int opcode = get_insn_field(insn, 6, 0);
        int rd = get_insn_field(insn, 11, 7);
        int funct3 = get_insn_field(insn, 14, 12);
        int rs1 = get_insn_field(insn, 19, 15);
        int rs2 = get_insn_field(insn, 24, 20);
        int funct7 = get_insn_field(insn, 31, 25);
        const char* insn_a = assembly_get(as, PC);
        printf("instruction = %s\n", insn_a);
        //printf("PC = 0x%x\ninsn = 0x%x\nopcode = 0x%x\nrd = 0x%x\nfunct3 = 0x%x\nrs1 = %i\nrs2 = %i\nfunct7 = 0x%x\n", PC, insn, opcode, rd, funct3, rs1, rs2, funct7);

        if(opcode == ECALL){
            if(ecall(x) == -1){
                return 0;
            }
        }else{
            //continue;
        }

        int Branch = 0;
        int MemRead = 0;
        int MemToReg = 0;
        int ALUOp0 = 0;
        int ALUOp1 = 0;
        int MemWrite = 0;
        int ALUSrc = 0;
        int RegWrite = 0;
        enum bitsize s = word;

        // decode instruction, set signals
        set_signals(opcode, funct3, &Branch, &MemRead, &MemToReg, &ALUOp0, &ALUOp1, &MemWrite, &ALUSrc, &RegWrite, &s);

        // generate immediate
        int ImmGen = get_imm_gen(insn, opcode);
        printf("ImmGen = %i\n", ImmGen);

        // get ALU action depending on control signals and instruction
        int ALU_action = ALU_control(opcode, ALUOp0, ALUOp1, funct7, funct3);

        // execute ALU
        int ALU_result;
        if (ALUSrc) {
            if (opcode == JAL) {
                ALU_result = ALU_execute(PC, ImmGen, ALU_action);
            }
            else {
                ALU_result = ALU_execute(rs1, ImmGen, ALU_action);
            }
        }
        else {
            ALU_result = ALU_execute(rs1, rs2, ALU_action);
        }

        // data memory
        int address = ALU_result;
        printf("ALU_result = 0x%x\n", ALU_result);
        int write_data;

        // if MemToReg is set write_data is read from memory
        // TODO add support for byte, halfword etc.
        if (MemToReg) {
            switch (s)
            {
            case byte:
                write_data = memory_rd_b(mem, address);
                printf("(wrote byte) write data = %i\n", write_data);
                break;
            case halfword:
                write_data = memory_rd_h(mem, address);
                printf("(wrote halfword) write data = %i\n", write_data);
                break;
            case word:
                write_data = memory_rd_w(mem, address);
                printf("(wrote word) write data = %i\n", write_data);
                break; 
            }
        }
        else { // if MemToReg is not set write_data is ALU result
            write_data = ALU_result;
            printf("write data = %i\n", write_data);
        }

        // if memwrite is set write result to memory
        // TODO add support for byte, halfword etc.
        if (MemWrite) {
            switch (s)
            {
            case byte:
                memory_wr_b(mem, address, write_data);
                break;
            case halfword:
                memory_wr_h(mem, address, write_data);
                break;
            case word:
                memory_wr_w(mem, address, write_data);
                break; 
            }
        }

        // if RegWrite is set write result to register
        if (RegWrite) {
            x[rd] = write_data;
            if (rd == 0) {
                x[rd] = 0;
            }
            printf("x[%i] = 0x%x\n", rd, write_data);
        }
        printf("\n");
        
        // update PC depending on branch and ALU result
        if (opcode == JAL) {
            PC = write_data - 4;
        }
        if (Branch == 1 && ALU_result == 0) {
            PC = ImmGen;
        }
        else {
            PC += 4;
        }
        looping++;
    }
    return 0;
}
