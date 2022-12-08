#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

// types of opcodes
#define LUI 0x37
#define AUIPC 0x17
#define JAL 0x6f
#define JALR 0x67
#define B 0x63
#define L 0x3
#define S 0x23
#define I 0x13
#define R 0x33
#define ECALL 0x73

// types of ALU control actions
#define AND 0x0
#define OR 0x1
#define ADD 0x2
#define SUB 0x6

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
