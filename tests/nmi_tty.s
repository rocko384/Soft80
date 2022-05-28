#define NMI_PAD .defs $0066 - $, 0
#define IN_BUF_BEGIN $2000
#define IN_BUF_END $2100


MAIN:
	LD SP, $FFFF

	LD HL, TEXT
    CALL STRLEN
    
    LD HL, TEXT
    LD B, A
	CALL PRINT
    
    CALL REQ_READ
    
    HALT
    
    LD HL, IN_BUF_BEGIN
    CALL STRLEN
    
    LD HL, IN_BUF_BEGIN
    LD B, A
    CALL PRINT
    
    HALT
	
NMI_PAD

NMI:
	LD C, 80
	IN B, (C)
    LD HL, IN_BUF_BEGIN
    INIR
	RETN

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
    
REQ_READ:
	LD C, 80
    LD B, $FF
    OUT (C), B
    RET
    
TEXT:
	.db "Hello, world!\n", 0
    