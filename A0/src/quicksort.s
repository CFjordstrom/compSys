.include "partition.s"

        .text
quicksort:
        # a0 stores array
        # a1 stores start
        # a2 stores end

        # ADD CODE HERE
        addi sp, sp, -24		# allocate 6 words on stack
        mv a6, a2			# move end to a6
        sw a6, 20(sp)			# store end
 	sw ra, 16(sp)			# store ra
 	sw a0, 12(sp)			# store array
 	sw a1, 8(sp)			# store start
 	sub a3, a2, a1 			# a3 = end-start
 	li a4, 2			# a4 = 2
 	blt a3, a4, return		# if end-start < 2 return
 	jal ra, partition		# partition(array, start, end), a0 = q
 	mv a5, a0			# a5 = q
 	lw a1, 8(sp)			# pop start
 	lw a0, 12(sp)			# pop array
 	lw ra, 16(sp)			# pop ra
 	lw a6, 20(sp)			# pop end
 	mv a2, a5			# a2 = q
 	sw a6, 20(sp)			# store end
 	sw ra, 16(sp)			# store ra
 	sw a0, 12(sp)			# store array
 	sw a5, 0(sp)			# store q
 	jal ra, quicksort		# call quicksort(array, start, q)
 	lw a5, 0(sp)			# pop q
 	lw a0, 12(sp)			# pop array
 	lw ra, 16(sp)			# pop ra
 	lw a6, 20(sp)			# pop end
 	mv a1, a5			# a1 = q
 	mv a2, a6			# a2 = end
 	jal ra, quicksort		# quicksort(array, q, end)
 	
 return:
 	lw ra, 16(sp)			# pop ra
 	addi sp, sp, 24			# move stack pointer back
 	jalr zero, ra, 0		# return