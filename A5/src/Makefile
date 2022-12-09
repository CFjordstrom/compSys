GCC=gcc -g -Wall -Wextra -pedantic -std=gnu11
LDFLAGS=-lm

sim: main.c simulate.o memory.o assembly.o read_exec.o
	$(GCC) $< *.o -o $@ $(LD_FLAGS)

simulate.o: simulate3.c simulate.h
	$(GCC) -c $< -o $@

memory.o: memory.c memory.h
	$(GCC) -c $< -o $@

assembly.o: assembly.c assembly.h
	$(GCC) -c $< -o $@

read_exec.o: read_exec.c read_exec.h
	$(GCC) -c $< -o $@

clean:
	rm -rf *.o sim log

rebuild: clean sim