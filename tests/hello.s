MAIN:
	LD HL, TEXT
    CALL STRLEN
    
    LD HL, TEXT
    LD B, A
	CALL PRINT
    
    HALT
	
STRLEN:
	LD A, 0
	LD BC, 0
    CPIR
    SUB C
    SUB 1
	RET
	
PRINT:
    LD C, 80
    OTIR
	RET
    
TEXT:
	.db "Hello, world!\n", 0
    