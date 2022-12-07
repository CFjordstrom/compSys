#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define R 1
#define I 2
#define S 3
#define B 4
#define U 5
#define J 6

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
