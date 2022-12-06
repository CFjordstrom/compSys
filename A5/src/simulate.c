#include "memory.h"
#include "assembly.h"
#include <stdio.h>

// create registers as struct or array of 32 ints

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

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    int PC = start_addr;
    int ins = memory_rd_w(mem, PC);
    int opcode = get_ins_field(ins, 6, 0);
    int rd = get_ins_field(ins, 11, 7);
    int funct3 = get_ins_field(ins, 14, 12);
    int rs1 = get_ins_field(ins, 19, 15);
    int rs2 = get_ins_field(ins, 24, 20);
    int funct7 = get_ins_field(ins, 31, 25);
    printf("opcode = 0x%x\n", opcode);

    switch(opcode){
        case 3:
            switch(funct3){
                case 0:
                    printf("instruction: lb");
                    break;
                
                case 1:
                    printf("instruction: lh");
                    break;

                case 2:
                    printf("instruction: lw");
                    break;

                case 4:
                    printf("instruction: lbu");
                    break;
                
                case 5:
                    printf("instruction: lhu");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;

        case 19:
            switch(funct3){
                case 0:
                    printf("instruction: addi");
                    break;

                case 1:
                    printf("instruction: slli");
                    break;

                case 2:
                    printf("instruction: slti");
                    break;

                case 3:
                    printf("instruction: sltu");
                    break;

                case 4:
                    printf("instruction: xor");
                    break;

                case 5:
                    if(funct7 == 0){
                        printf("instruction: srli");
                    }else{
                        printf("instruction: srai");
                    }
                    break;

                case 6:
                    printf("instruction: ori");
                    break;

                case 7:
                    printf("instruction: andi");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;
        
        case 23:
            printf("instruction: auipc");
        
        case 35:
            switch(funct3){
                case 0:
                    printf("instruction: sb");
                    break;
                case 1:
                    printf("instruction: sh");
                    break;
                case 2:
                    printf("instruction: sw");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;
        
        case 51:
            switch(funct3){
                case 0:
                    if (funct7 == 0){
                        printf("instruction: add");
                    }else{
                        printf("instruction: sub");
                    }
                    break;
                case 1:
                    printf("instruction: sll");
                    break;
                case 2:
                    printf("instruction: slt");
                    break;
                case 3:
                    printf("instruction: sltu");
                    break;
                case 4:
                    printf("instruction: xor");
                    break;
                case 5:
                    if (funct7 == 0){
                        printf("instruction: srl");
                    }else{
                        printf("instruction: sra");
                    }
                    break;
                case 6:
                    printf("instruction: or");
                    break;
                case 7:
                    printf("instruction: and");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;

        case 55:
            printf("instruction: lui");
            break;

        case 99:
            switch(funct3){
                case 0:
                    printf("instruction: beq");
                    break;
                case 1:
                    printf("instruction: bne");
                    break;
                case 4:
                    printf("instruction: blt");
                    break;
                case 5:
                    printf("instruction: bge");
                    break;
                case 6:
                    printf("instruction: bltu");
                    break;
                default:
                    printf("invalid funct3: 0x%x given opcode: 0x%x\n", funct3, opcode);
                    return 1;
            }
            break;

        case 103:
            printf("instruction: jalr");
            break;
        
        case 111:
            printf("instruction: jal");
            break;
            
        default:
            printf("Invalid opcode.");
            return 1;

    }

    return 0;
}
