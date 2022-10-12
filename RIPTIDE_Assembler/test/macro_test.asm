;macro test for 8xasm

INCLUDE "RIPTIDE-III.INC"

ham equ r1
cheese equ r2

macro nothingburger nothing, burger
    nop
    move nothing, 8, burger
    nop
endmacro nothingburger

	ORG 0
	JMP INT_RESET
	RET	;INT_HSYNC
	RET ;INT_VSYNC
	RET	;INT_UART_RX
	RET	;INT_UART_TX
	RET	;INT_KB_RX
	RET	;INT_TIMER
	RET	;INT_I2C
	
foo
    nothingburger ham, cheese
    ret

int_reset
    xmit `high foo, aux
    call foo
    jmp int_reset
    
