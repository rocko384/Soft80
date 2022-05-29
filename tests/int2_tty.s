#define INT_TABLE_PAD .defs $1F00 - $, 0
#define IN_BUF_BEGIN $2000
#define IN_BUF_END $2100

MAIN:
	LD SP, $FFFF
    LD A, INT_TABLE >> 8
    LD I, A
	IM 2
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
    
    EI
    
    JP STOP

INT0:
	LD C, 80
	IN B, (C)
    LD HL, IN_BUF_BEGIN
    INIR
	RETI

INT1:
	LD HL, TEXT2
    CALL STRLEN
    
    LD HL, TEXT2
    LD B, A
    CALL PRINT
    
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

STOP:
	HALT
    JP STOP

TEXT:
	.db "Hello, world!\n", 0

TEXT2:
	.db "Interrupt!\n", 0

INT_TABLE_PAD

INT_TABLE:
	.dw INT0
    .dw INT1