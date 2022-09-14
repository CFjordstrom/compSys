        .text
partition:
        # array in a0
        # p in a1
        # r in a2

        # ADD CODE HERE
        addi t0, a1, -1			# t0(i) = p-1
        slli a1, a1, 2			# a1 = p*4
        add a1, a0, a1			# a1 = &array[p]
        lw a1, 0(a1)			# a1(pivot) = array[p]
	mv t1, a2 			# t1(j) = r
	mv t6, ra			# t6 = ra
	
partition_loop:
	jal ra, partition_j_loop	# go to j-loop
	jal ra, partition_i_loop	# go to i-loop
	blt t0, t1, partition_loop_continue		# if i < j
	addi a0, t1, 1			# a0 = j + 1
	jalr zero, t6, 0
	
partition_loop_continue:
	slli t2, t0, 2			# t2 = i*4
	slli t3, t1, 2			# t3 = j*4
	add t2, a0, t2			# t2 = &array[i]
	add t3, a0, t3			# t3 = &array[j]
	lw t4, 0(t2)			# t4 = array[i]
	lw t5, 0(t3)			# t5 = array[j]
	sw t5, 0(t2)			# array[i] = array[j]
	sw t4, 0(t3)			# array[j] = tmp
	jal zero, partition_loop	# loop again
	
partition_j_loop:
	addi t1, t1, -1			# t1(j)--
	slli t2, t1, 2			# t2 = j*4
	add t2, a0, t2			# t2 = &array[j]
	lw t2, 0(t2)			# t2 = array[j]
	blt a1, t2, partition_j_loop	# if pivot < array[j] loop again
	jalr zero, ra, 0		# else return to partition_loop

partition_i_loop:
	addi, t0, t0, 1			# t0(i)++
	slli t2, t0, 2			# t2 = i*4
	add t2, a0, t2			# t2 = &array[i]
	lw t2, 0(t2)			# t2 = array[i]
	blt t2, a1, partition_i_loop	# if array[i] < pivot loop again
	jalr zero, ra, 0		# else return to partition_loop