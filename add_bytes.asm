.orig x3000
lea r0, num_address
ldw r0, r0, #0
lea r7, bit_mask
ldw r7, r7, #0

ldb r1, r0, #0
brn neg_r1 
brp pos_r1
brz no_overflow

neg_r1 ldb r2, r0, #1
brn neg_neg
brp no_overflow
brz no_overflow

pos_r1 ldb r2, r0, #1
brn no_overflow
brp pos_pos
brz no_overflow

neg_neg add r4, r1, r2
and r4, r4, r7
brz overflow
br no_overflow

pos_pos add r4, r1, r2
and r4, r4, r7
brz no_overflow
br overflow

no_overflow and r6, r6, #0
stb r6, r0, #3
br sum_bytes

overflow and r6, r6, #0
add r6, r6, #1
stb r6, r0, #3
br sum_bytes

sum_bytes and r3, r3, #0
add r3, r1, r2
stb r3, r0, #2
br done

done halt
num_address .fill x3100
bit_mask .fill x0080
.end