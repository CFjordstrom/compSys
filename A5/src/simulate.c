#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>

// create registers as struct or array of 32 ints
int x[32];

enum bitsize{byte, halfword, word, ubyte, uhalfword, uword};

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
    ALU_LUI = 19,
};

// returns base to the power of exponent
int power(int base, int exponent) {
    int product = 1;

    for (int i = 0; i < exponent; i++) {
        product *= base;
    }
    return product;
}

int ecall(int* registers, FILE *log_file){
    int syscall = registers[17];
    if((syscall == 3 || syscall == 93)){
        fprintf(log_file, "Syscall made with x[17] = 3 or 93 - exiting.\n");
        return -1;
    }
    else if(syscall == 1){
        fprintf(log_file, "Syscall made with x[17] = 1 - getting character.\n");
        char c = getchar();
        fprintf(log_file, "Got character: %c", c);
        registers[17] = c;
    } else if(syscall == 2){
        fprintf(log_file, "Syscall made with x[17] = 2 - putting character: %c\n", registers[16]);
        putchar(registers[16]);
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
void set_signals(int opcode, int funct3, int* Branch, int* MemRead, int* MemToReg, int* ALUOp0, int* ALUOp1, int* MemWrite, int* ALUSrc, int* RegWrite, enum bitsize* size, FILE *log_file) {
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
            case 0x4:
                *size = ubyte;
                break;
            case 0x5:
                *size = uhalfword;
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
            fprintf(log_file, "Malformed signal - could not recognize opcode.\n");
    }
}

// some immediates need first bit set to 30
int get_imm_gen(int insn, int opcode, FILE *log_file){
    int imm;
    unsigned int imm_0 = 0;
    unsigned int imm_10_to_1 = 0;
    unsigned int imm_11 = 0;
    unsigned int imm_19_to_12 = 0;
    unsigned int imm_20 = 0;
    unsigned int imm_4_to_1_11 = 0;
    unsigned int imm_12_to_10 = 0;
    unsigned int imm_12 = 0;
    unsigned int imm_10_to_5 = 0;
    unsigned int imm_4_to_1 = 0;
    unsigned int imm_11_to_5 = 0;
    unsigned int imm_4_to_0 = 0;



    switch (opcode) {
        case LUI:
            return get_insn_field(insn, 31, 12);
        
        case AUIPC:
            return get_insn_field(insn, 31, 12);

        case JAL:
            unsigned int imm2 = 0;
            imm_0 = 0;
            imm_10_to_1 = (insn & 0b01111111111000000000000000000000) >> 20;
            imm_11 = (insn & 0b00000000000100000000000000000000) >> 9;
            imm_19_to_12 = (insn & 0b00000000000011111111000000000000);
            imm_20 = (insn & 0x80000000) >> 11;
            imm2 = imm_20 | imm_19_to_12 | imm_11 | imm_10_to_1 | imm_0;
            if ((imm2 >> 20) != 0){
                imm2 = imm2 | 0xfff00000; // add sign extension
            }
            return imm2;
            //imm = (get_insn_field(insn, 31, 31) << 20) | (get_insn_field(insn, 30, 21) << 1) | (get_insn_field(insn, 20, 20) << 11) | (get_insn_field(insn, 19, 12) << 12);
            //return sign_extend(imm, 20);

        case JALR:
            imm = get_insn_field(insn, 31, 20);
            if((imm >> 11) == 1){
                return (imm | 0b11111111111111111111000000000000);
            }else{
                return imm;
            }

        case B:
            imm_0 = 0;
            imm_11 = (get_insn_field(insn, 7, 7) << 11);
            imm_4_to_1 = (get_insn_field(insn, 11, 8) << 1);
            imm_10_to_5 = (get_insn_field(insn, 30, 25) << 5);
            imm_12 = (get_insn_field(insn, 31, 31) << 12);
            imm = (imm_12 | imm_11 | imm_10_to_5 | imm_4_to_1 | imm_0);
            if((imm_12 >> 12) == 1){
                return imm = (imm | 0b11111111111111111111000000000000);
            }else{
                return imm;
            }
            /*
            imm_0 = 0;
            imm_4_to_1_11 = get_insn_field(insn, 11, 7);
            imm_12_to_10 = get_insn_field(insn, 31, 25);
            imm_11 = (imm_4_to_1_11 & 0b00000001) << 11;
            imm_12 = (imm_12_to_10 & 0b00111111) << 6;
            imm_10_to_5 = (imm_12_to_10 & 0b00111111) <<5;
            imm_4_to_1 = (imm_4_to_1_11 & 0b00011110);
            imm = (imm_12 | imm_11 | imm_10_to_5 | imm_4_to_1 | imm_0);
            if (((imm_12) >> 12) != 0){
                imm = imm | 0xffffe000;
            }
            return imm;
            */
            //return (get_insn_field(insn, 31, 31) << 11) | (get_insn_field(insn, 7, 7) << 10) | (get_insn_field(insn, 30, 25) << 4) | get_insn_field(insn, 11, 8);

        case L:
            return get_insn_field(insn, 31, 20);

        case S:
            imm_11_to_5 = (get_insn_field(insn, 31, 25) << 5);
            imm_4_to_0 = get_insn_field(insn, 11, 7);
            imm = ((imm_11_to_5) | imm_4_to_0);
            if((imm >> 11) == 1){
                return imm = (imm | 0b11111111111111111111000000000000);
            }
            return imm;

        case I:
            imm = ((insn & 0b11111111111100000000000000000000) >> 20);
            if((imm >> 11) == 1){
                return (imm | 0b11111111111111111111000000000000);
            }else{
                return imm;
            }

        default:
            fprintf(log_file, "ImmGen error\n");
            return -1;
    }
    return -1;
}

// gets the output of the ALU control based on the ALUOp and the funct3 and funct7 fields, also get addressing size (byte, halfword, word)
int ALU_control(int opcode, int ALUOp1, int ALUOp0, int funct7, int funct3, FILE *log_file){
    int bit25 = get_insn_field(funct7, 0, 0);
    int bit30 = get_insn_field(funct7, 5, 5);
    if(ALUOp1 == 0 && ALUOp0 == 0){
        if(opcode == LUI){
            fprintf(log_file, "ALU control signal: LUI\n");
            return ALU_LUI;
        } else{
            fprintf(log_file, "ALU control signal: ADD\n");
            return ALU_ADD;
        }
    }
    else if(ALUOp1 == 0 && ALUOp0 == 1){
        fprintf(log_file, "ALU control signal: SUB\n");
        return ALU_SUB;
    }
    else if (ALUOp1 == 1 && ALUOp0 == 0)
    {
        if(opcode == I){
                fprintf(log_file, "I-format OPCODE.\n");
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
                        if(bit30 == 0){
                            return ALU_SRL;
                        } else{
                            return ALU_SRA;
                        }

                    case 0x6:
                        return ALU_OR;

                    case 0x7:
                        return ALU_AND;

                    default:
                        return 0;
                }
        }
        else if (opcode == B) { // if branch check funct3
            fprintf(log_file, "B-format opcode.\n");
            switch (funct3) {
                case 0x0:
                    fprintf(log_file, "ALU control signal: BEQ\n");
                    return BRANCH_BEQ;
                    break;

                case 0x1:
                    fprintf(log_file, "ALU control signal: BNE\n");
                    return BRANCH_BNE;
                    break;

                case 0x4:
                    fprintf(log_file, "ALU control signal: BLT\n");
                    return BRANCH_BLT;
                    break;

                case 0x5:
                    fprintf(log_file, "ALU control signal: BGE\n");
                    return BRANCH_BGE;
                    break;

                case 0x6:
                    fprintf(log_file, "ALU control signal: BLTU\n");
                    return BRANCH_BLTU;
                    break;

                case 0x7:
                    fprintf(log_file, "ALU control signal: BGEU\n");
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
                fprintf(log_file, "R format opcode\n");
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

int ALU_execute(int input1, int input2, enum ALU_action ALU_action, FILE *log_file){
    switch (ALU_action) {
        case ALU_LUI:
            return (input2 << 12) & 0xfffff000;

        case ALU_ADD:
            fprintf(log_file, "Adding input1: %d and input 2: %d\n", input1, input2);
            return input1 + input2;

        case ALU_SUB:
            fprintf(log_file, "Subtracting input1: %d and input 2: %d\n", input1, input2);
            return input1 - input2;

        case ALU_OR:
            return input1 | input2;
        
        case ALU_AND:
            fprintf(log_file, "Performing bitwise AND on: 0x%x and 0x%x \n", input1, input2);
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
            fprintf(log_file, "ALU executing: BEQ sub:(%d - %d)\nExpected result: %d\n", input1, input2, (input1 - input2));
            return input1 - input2;
        
        case BRANCH_BNE:
            fprintf(log_file, "ALU executing: BNE\n");
            return !(input1 - input2);

        case BRANCH_BLT:
            fprintf(log_file, "ALU executing: BLT\n");
            return !(input1 < input2);

        case BRANCH_BGE:
            fprintf(log_file, "ALU executing: BGE - checking if %d >= %d\n", input1, input2);
            return !(input1 >= input2);

        case BRANCH_BLTU:
            fprintf(log_file, "ALU executing: BLTU\n");
            return !((unsigned int) input1 < (unsigned int) input2);

        case BRANCH_BGEU:
            fprintf(log_file, "ALU executing: BGEU\n");
            return !((unsigned int) input1 >= (unsigned int) input2);

        default:
            break;
    }
    return -1;
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int PC = start_addr;
    long int num_insn = 0;
    int looping = 0;
    fprintf(log_file, "Beginning of log.\n");
    while (1) {
        fprintf(log_file, "\n Instruction %d.\nPC = 0x%x\n", looping, PC);
        // fetch instruction
        int insn = memory_rd_w(mem, PC);
        int opcode = get_insn_field(insn, 6, 0);
        int rd = get_insn_field(insn, 11, 7);
        int funct3 = get_insn_field(insn, 14, 12);
        int rs1 = get_insn_field(insn, 19, 15);
        int rs2 = get_insn_field(insn, 24, 20);
        int funct7 = get_insn_field(insn, 31, 25);
        const char* insn_a = assembly_get(as, PC);
        fprintf(log_file, "Instruction: %s\n", insn_a);
        
        if(opcode == ECALL){
            fprintf(log_file, "ECALL made with x[17] = %d\n", x[17]);
            looping++;
            if(ecall(&x, log_file) == -1){
                return 0;
            }else{
                PC += 4;
                continue;
            }
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
        set_signals(opcode, funct3, &Branch, &MemRead, &MemToReg, &ALUOp0, &ALUOp1, &MemWrite, &ALUSrc, &RegWrite, &s, log_file);

        // generate immediate
        int ImmGen = get_imm_gen(insn, opcode, log_file);
        fprintf(log_file, "ImmGen result = %i\n", ImmGen);

        // get ALU action depending on control signals and instruction
        int ALU_action = ALU_control(opcode, ALUOp1, ALUOp0, funct7, funct3, log_file);

        // execute ALU
        int ALU_result;
        if (ALUSrc) {
            if (opcode == JAL) {
                ALU_result = ALU_execute(PC, ImmGen, ALU_action, log_file);
            }
            else {
                fprintf(log_file, "Calling ALU execute with inputs x[%d] and Imm.\n", rs1, ImmGen, ALU_action);
                ALU_result = ALU_execute(x[rs1], ImmGen, ALU_action, log_file);
            }
        }
        else {
            fprintf(log_file ,"Calling ALU execute with inputs: x[%d] and x[%d].\n", rs1, rs2);
            ALU_result = ALU_execute(x[rs1], x[rs2], ALU_action, log_file);
        }
        fprintf(log_file, "ALU result = %d\n", ALU_result);

        // data memory
        int address = ALU_result;
        int write_data;

        // if MemToReg is set write_data is read from memory
        if (MemToReg) {
            fprintf(log_file, "Loading mem to register x[%d] = %d\n", rd, x[rd]);
            switch (s)
            {
            case byte:
                write_data = memory_rd_b(mem, address);
                fprintf(log_file, "Loaded byte (0x%x) from mem[0x%x] into write data\n", write_data, address);
                break;
            case halfword:
                write_data = memory_rd_h(mem, address);
                fprintf(log_file, "Loaded halfword (0x%x) from mem[0x%x] into write data\n", write_data, address);
                break;
            case word:
                write_data = memory_rd_w(mem, address);
                fprintf(log_file, "Loaded word (0x%x) from mem[0x%x] into write data\n", write_data, address);
                break;
            case ubyte:
                write_data = (unsigned int) memory_rd_b(mem, address);
                fprintf(log_file, "Loaded unsigned byte (0x%x) from mem[0x%x] into write data\n", write_data, address);
                break;
            case uhalfword:
                write_data = (unsigned int) memory_rd_h(mem, address);
                fprintf(log_file, "Loaded unsigned halfword (0x%x) from mem[0x%x] into write data\n", write_data, address);
                break;
            }
        }
        else { // if MemToReg is not set write_data is ALU result, except if MemWrite = 1; then write_data is the value at x[rs2]
            if(MemWrite){
                write_data = x[rs2];
                fprintf(log_file, "MemWrite = 1. Fetching write data from x[%d]. Write_data = %i\n", rs2, write_data);
            }else{
                write_data = ALU_result;
                fprintf(log_file, "MemWrite = 0. Fetching write data from ALU. Write_data = %i\n", rs2, write_data);
            }
        }

        // if memwrite is set write result to memory
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
            fprintf(log_file, "Wrote (0x%x) to mem[0x%x]\n", write_data, address);
        }

        // if RegWrite is set write result to register
        if (RegWrite) {
            if(opcode == JALR){
                fprintf(log_file, "RegWrite for JALR\n");
                x[rd] = PC + 4;
                fprintf(log_file, "Wrote PC to x[%i] = %d\n", rd, (PC + 4));
            } else if(opcode == JAL){
                fprintf(log_file, "RegWrite for JAL\n");
                x[rd] = PC + 4;
                fprintf(log_file, "Wrote PC to x[%i] = %d\n", rd, (PC + 4));
            }
            else{
                fprintf(log_file, "RegWrite for non JAL/JALR: wrote %d to register x[%d]\n", write_data, rd);
                x[rd] = write_data;
                fprintf(log_file, "Wrote write_data to x[%i] = %d\n", rd, write_data);
                if (rd == 0) {
                    x[rd] = 0;
                }
            }
            
        }
        x[0] = 0;
        
        // update PC depending on branch and ALU result
        if (opcode == JAL) {
            fprintf(log_file, "JAL jump.\n");
            PC = write_data - 4;
        } else if(opcode == JALR){
            PC = write_data - 4;
            fprintf(log_file, "JALR jump.\n");
        }
        if (Branch == 1){
            if(ALU_result == 0){
                fprintf(log_file, "Branch = 1, ALU = 0. Taking branch. PC += ImmGen.\n");
                PC += ImmGen;
            }else{
                fprintf(log_file, "Branch = 1, ALU != 0. Skipping branch. PC += 4.\n");
                PC += 4;
            }
        }
        else {
            PC += 4;
        }
        fprintf(log_file, "New PC = 0x%x\n", PC);
        int memorytest = memory_rd_b(mem, x[8]);
        fprintf(log_file, "Value at -1(s0) (mem[0x%x]): %d\n", x[8], memorytest);
        num_insn++;
        looping++;
        fprintf(log_file, "Registers: ");
        for(int i = 0; i < 32; i++){
            fprintf(log_file, "x[%d] = %d, ", i, x[i]);
            if(i%5 == 0 && i != 0){
                fprintf(log_file, "\n");
            }
        }
        fprintf(log_file, "\n");
    }
return num_insn;
}
