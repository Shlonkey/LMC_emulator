DAT ONE 0x01 ;Defines a variable ONE initalized with value 0x01. Assembler will decide Address
DAT TEMP ;Defines a variable with no initial value. Assembler will decide address and value

INP
STA TEMP
INP
ADD TEMP
LOOP:
	OUT
	BRZ END
	SUB ONE
	BRA LOOP
END:HLT
DAT TEMP_2

 ;Defines a variable with no initial value. Assembler will decide address and value
