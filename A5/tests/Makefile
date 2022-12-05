%.riscv: %.c lib.c
	riscv32-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O1 $< lib.c -static -nostartfiles -nostdlib -o $@

%.dis: %.riscv
	riscv32-unknown-elf-objdump -s -w $< > $@
	riscv32-unknown-elf-objdump -S $< >> $@
