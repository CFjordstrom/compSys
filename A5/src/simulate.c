#include "memory.h"
#include "assembly.h"
#include <stdio.h>

// create registers as struct or array of 32 ints

// get instruction -> get opcode -> depending on opcode format is R, I, S, SB or 5th whatever that is

// to get parts of instruction shift right x bits and AND with 2^y-1. for opcode don't need to shift, just AND with 2^7-1. rd = shift right 7 bits and AND with 2^5-1

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {
    
    int ins = memory_rd_w(mem, start_addr);
    printf("instruction = %i\n", ins);

    return 0;
}