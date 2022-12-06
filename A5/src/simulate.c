#include "memory.h"
#include "assembly.h"
#include <stdio.h>

// create registers as struct or array of 32 ints
int x[32];

// alu signals and muxes
struct aluControl{
    int branch;
    int MemRead;
    int MemtoReg;
    int ALUOp;
    int MemWrite;
    int ALUSrc;
    int RegWrite;

    int ALUSrc_mux;
    int MemtoReg_mux;
    int branch_mux;
};

// get instruction -> get opcode -> depending on opcode format is R, I, S, SB or 5th whatever that is

// to get parts of instruction shift right x bits and AND with 2^y-1. for opcode don't need to shift, just AND with 2^7-1. rd = shift right 7 bits and AND with 2^5-1

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
int get_ins_field(int ins, int end, int start) {
    int length = end-start+1;
    int mask = power(2, length)-1;
    return mask & (ins >> start);
}

// sets alu control signals and mux'es based on opcode and funct fields
// actually also sets registers for now
// print statements purely for convenience

void set_signals(struct aluControl *signals,  int opcode, int funct3, int funct7, int ins, int rd, int rs1, int rs2){
    int immediate = 0;
    int shamt;
    switch(opcode){
        case 3:
            switch(funct3){
                case 0:
                    printf("instruction: lb\n");
                    break;
                
                case 1:
                    printf("instruction: lh\n");
                    break;

                case 2:
                    printf("instruction: lw\n");
                    break;

                case 4:
                    printf("instruction: lbu\n");
                    break;
                
                case 5:
                    printf("instruction: lhu\n");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;

        case 19: 
            // I-format; we calculate immediate [11:0] from bits [31:20].
            // and we recall that shamt[4:0] equals bits [24:20] (rs2)
            immediate = get_ins_field(ins, 31, 20);
            shamt = rs2;
            //remember to sign-extend immediate
            switch(funct3){
                case 0:
                    printf("instruction: addi\n");
                    x[rd] = x[rs1] + immediate;
                    break;

                case 1:
                    printf("instruction: slli\n");
                    x[rd] = x[rs1] << shamt;
                    break;

                case 2:
                    printf("instruction: slti\n");
                    if(x[rs1] < immediate){
                        x[rd] = 1;
                    }else{
                        x[rd] = 0;
                    }
                    break;

                case 3:
                    printf("instruction: sltu\n");
                    if(x[rs1] < x[rs2]){
                        x[rd] = 1;
                    }else{
                        x[rd] = 0;
                    }
                    break;

                case 4:
                    printf("instruction: xori\n");
                    x[rd] = x[rs1] ^ immediate;
                    break;

                case 5:
                    if(funct7 == 0){
                        printf("instruction: srli\n");
                        // implement logical right shift for signed int
                    }else{
                        printf("instruction: srai\n");
                        x[rd] = x[rs1] >> shamt;
                    }
                    break;

                case 6:
                    printf("instruction: ori\n");
                    x[rd] = x[rs1] | immediate;
                    break;

                case 7:
                    printf("instruction: andi\n");
                    x[rd] = x[rs1] & immediate;
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;
        
        case 23:
            printf("instruction: auipc\n");
        
        case 35:
            switch(funct3){
                case 0:
                    printf("instruction: sb\n");
                    break;
                case 1:
                    printf("instruction: sh\n");
                    break;
                case 2:
                    printf("instruction: sw\n");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;
        
        case 51:
            if(funct7 == 1){
                switch (funct3){
                    case 0:
                        printf("instruction: mul\n");
                        break;
                    case 1:
                        printf("instruction: mulh\n");
                        break;
                    case 2:
                        printf("instruction: mulhsu\n");
                        break;
                    case 3:
                        printf("instruction: mulhu\n");
                        break;
                    case 4:
                        printf("instruction: div\n");
                        break;
                    case 5:
                        printf("instruction: divu\n");
                        break;
                    case 6:
                        printf("instruction: rem\n");
                        break;
                    case 7:
                        printf("instruction: remu\n");
                        break;
                    default:
                        printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                        break;
                }
            }else{
                switch(funct3){
                    case 0:
                        if (funct7 == 0){
                            printf("instruction: add\n");
                        }else{
                            printf("instruction: sub\n");
                        }
                        break;
                    case 1:
                        printf("instruction: sll\n");
                        break;
                    case 2:
                        printf("instruction: slt\n");
                        break;
                    case 3:
                        printf("instruction: sltu\n");
                        break;
                    case 4:
                        printf("instruction: xor\n");
                        break;
                    case 5:
                        if (funct7 == 0){
                            printf("instruction: srl\n");
                        }else{
                            printf("instruction: sra\n");
                        }
                        break;
                    case 6:
                        printf("instruction: or\n");
                        break;
                    case 7:
                        printf("instruction: and\n");
                        break;
                    default:
                        printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                        return 1;
                }
            }
            break;

        case 55:
            printf("instruction: lui\n");
            // set x[rd] = immediate[31:12] << 12
            immediate = get_ins_field(ins, 31, 12);
            x[rd] = immediate<<12;
            break;

        case 59:
            switch (funct3){
                case 0:
                    printf("instruction: mulw\n");
                    break;
                case 4:
                    printf("instruction: divw\n");
                    break;
                case 5:
                    printf("instruction: divuw\n");
                    break;
                case 6:
                    printf("instruction: remw\n");
                    break;
                case 7:
                    printf("instruction: remuw\n");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    break;
            }
            printf("instruction: lui\n");
            break;

        case 99:
            switch(funct3){
                case 0:
                    printf("instruction: beq\n");
                    break;
                case 1:
                    printf("instruction: bne\n");
                    break;
                case 4:
                    printf("instruction: blt\n");
                    break;
                case 5:
                    printf("instruction: bge\n");
                    break;
                case 6:
                    printf("instruction: bltu\n");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;

        case 103:
            printf("instruction: jalr\n");
            break;
        
        case 111:
            printf("instruction: jal\n");
            break;

        case 115:
            printf("instruction: ecall\n");
            break;

        default:
            printf("Invalid opcode.\n");
            return 1;

    }

}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    struct aluControl *signals = malloc(sizeof(struct aluControl));
    memset(&signals, 0, sizeof(signals));
    int PC = start_addr;


    // idk how we terminate, counter for convenience, will remove
    int ins_ctr = 0;
    while(ins_ctr++ < 20){
        // fetch instruction
        int ins = memory_rd_w(mem, PC);
        int opcode = get_ins_field(ins, 6, 0);
        int rd = get_ins_field(ins, 11, 7);
        int funct3 = get_ins_field(ins, 14, 12);
        int rs1 = get_ins_field(ins, 19, 15);
        int rs2 = get_ins_field(ins, 24, 20);
        int funct7 = get_ins_field(ins, 31, 25);

        // decode instruction, set control signals
        printf("Instruction %d:\n", ins_ctr);
        set_signals(signals, opcode, funct3, funct7, ins, rd, rs1, rs2);
        printf("opcode = 0x%x\n", opcode);
        printf("address: 0x%x\n", PC);
        printf("rd: %d\n", rd);
        printf("rs1: x[%d]\n", rs1);
        printf("rs2: x[%d]\n", rs2);
        printf("\n");

        // set registers, compute branch address

        // execute ALU, execute load/store

        // increment PC, clear temp registers?
        PC += 4;
    }

    return 0;
}
