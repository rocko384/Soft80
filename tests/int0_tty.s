#define RST6_PAD .defs $0030 - $, 0
#define IN_BUF_BEGIN $2000
#define IN_BUF_END $2100

MAIN:
	LD SP, $FFFF
	IM 0
    EI

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
	
RST6_PAD

INT:
	LD C, 80
	IN B, (C)
    LD HL, IN_BUF_BEGIN
    INIR
	RETI

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
    