DAT ONE 0x01
DAT TEMP

; Get user input and calculate loop start value.
INP
STA TEMP
INP
ADD TEMP

; While value != loop
WHILE_LOOP_1:
	BRZ END_WHILE_LOOP_1
	OUT
	SUB ONE
	BRA WHILE_LOOP_1

; End program
END_WHILE_LOOP_1:
	HLT
