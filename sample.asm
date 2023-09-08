;This program counts from 10 to 0
	.ORIG x3000   	
	LEA R0, TEN		;This instruction will be loaded into memory location x3000
	LDW r7, r7, #0
	rshfl r0, r0, #2
START		ADD R1, R1, #-1
	stb r1, r5, #5
	not r0 r5
	ret
	rti
	BRp DONE
	BR START
				;blank line
DONE	TRAP x25		;The last executable instruction
TEN	.FILL x000A		;This is 10 in 2's comp, hexadecimal
	.END			;The pseudo-op, delimiting the source program
