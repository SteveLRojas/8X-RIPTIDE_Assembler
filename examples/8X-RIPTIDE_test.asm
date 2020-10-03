INCLUDE "8X-RIPTIDE.INC"
	ORG 0
	JMP XEC_TEST
XEC_IV_TEST
	XMIT $AA, R0
	XMIT $00, LIV3, 4
	XMIT $00, LIV7, 4	;CLEAR LIV
	XEC CLR_R0 (LIV7, 4)
	XMIT $01, LIV7, 4	;SET LIV TO 1
	XEC CLR_R0 (LIV7, 4)
	JMP CALL_TEST
CLR_R0
	XMIT $00, R0
SET_R0
	XMIT $FF, R0

XEC_TEST
	XMIT $55, IVL
	XMIT $00, R0
	XEC CLR_LFT_ADDR (R0)
	XMIT $01, R0
	XEC CLR_LFT_ADDR (R0)
	XMIT $02, R0
	XEC CLR_LFT_ADDR (R0)
	JMP XEC_TEST
XEC_BRANCH_RETURN
	JMP XEC_IV_TEST
CLR_LFT_ADDR
	XMIT $00, R7
SET_LFT_ADDR
	XMIT $FF, R7
XEC_BRANCH
	JMP XEC_BRANCH_RETURN
	
NOP_TEST
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	JMP NOP_TEST

CALL_TEST
	XMIT HIGH SUB_NOP, AUX
	CALL SUB_NOP
	NOP
	NOP
	CALL SUB_SHORT
	XMIT 8, R1
	CALL SUB_NESTED
SUB_NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	RET
SUB_SHORT
	RET
SUB_NESTED
	MOVE AUX, R2	;COPY AUX TO R2
	XMIT $FF, AUX	;SET AUX TO -1
	ADD R1, R1	;DECREMENT R1
	MOVE R2, AUX	;RESTORE AUX
	NZT R1, SN_NEXT	;IF R1 NOT ZERO CALL SUB_NESTED AGAIN
	RET
SN_NEXT
	CALL SUB_NESTED
	JMP NOP_TEST