.include "partition.s"

        .text
quicksort:
        # a0 stores array
        # a1 stores start
        # a2 stores end

        # ADD CODE HERE
        addi sp, sp, -20		# allocate 6 words on stack
        mv t1, a2			# move end to t1
 	sw ra, 16(sp)			# store ra
 	sw a0, 12(sp)			# store array
 	sw a1, 8(sp)			# store start
 	sw t1, 4(sp)			# store end
 	sub t2, a2, a1 			# t2 = end-start
 	li t3, 2			# t3 = 2
 	blt t2, t3, return		# if end-start < 2 return
 	jal ra, partition		# partition(array, start, end), a0 = q
 	mv t4, a0			# t4 = q
 	lw t1, 4(sp)			# pop end
 	lw a1, 8(sp)			# pop start
 	lw a0, 12(sp)			# pop array
 	lw ra, 16(sp)			# pop ra
 	mv a2, t4			# a2 = q
 	sw ra, 16(sp)			# store ra
 	sw a0, 12(sp)			# store array
 	sw t1, 4(sp)			# store end
 	sw t4, 0(sp)			# store q
 	jal ra, quicksort		# call quicksort(array, start, q)
 	lw a1, 0(sp)			# a1 = q
 	lw a2, 4(sp)			# a2 = end
 	lw a0, 12(sp)			# pop array
 	lw ra, 16(sp)			# pop ra
 	jal ra, quicksort		# quicksort(array, q, end)
 	
 return:
 	lw ra, 16(sp)			# pop ra
 	addi sp, sp, 20			# move stack pointer back
 	jalr zero, ra, 0		# return
