; MSC Address map
; 0x0 program space control register
; 	bit0	reset bit (LSB)
; 	bit1	reserved
; 	bit2	reserved
; 	bit3	control enable bit
; 0x1 program space page register
; 0x2 data space control register
; 	bit0	reset bit (LSB)
; 	bit1	flush bit
; 	bit2	reserved
; 	bit3	control enable bit
; 0x3 data space page register

; RS-232 module address map
; 0 data register
; 1 status register
;	bit0	tx_overwrite bit (LSB)
;	bit1	rx_overwrite bit
;	bit2	tx_done bit
;	bit3	rx_done bit

; Address map for right bank:
; 0x0000 to 0xFFFF	active data memory page (cached)

; Address map for program space:
; 0x0000 to 0xFFFF	active program memory page (cached)

; REMEMBER: PROGRAM MEMORY IS WORD ADDRESSABLE WHILE DATA MEMORY IS BYTE ADDRESSABLE!
; THE TWO MEMORY SPACES ARE SHARED THROUGH THE CACHE CONTROLLERS, BOTH CACHES MUST BE FLUSHED FOR CHANGES TO THE DATA SPACE TO SHOW UP IN THE PROGRAM SPACE.

INCLUDE "RIPTIDE-II.INC"
; THESE ARE ADDRESSES OF MEMORY LOCATIONS USED FOR SPECIAL PURPOSES IN THE PROGRAM
DUMP_START_LOW		EQU $F000
DUMP_START_HIGH		EQU $F001
DUMP_END_LOW		EQU $F002
DUMP_END_HIGH		EQU $F003

DISPLAY_COUNT_L	EQU $F004
DISPLAY_COUNT_H	EQU $F005

PREV_VSYNC	EQU $F006
FRAME_COUNT EQU $F007

RND_L EQU $F008
RND_H EQU $F009

HEX_BUF_L EQU $F00A
HEX_BUF_H EQU $F00B

	ORG 0
INIT
	XMIT $00, AUX
	XMIT $00, R1
	XMIT $00, R2
	XMIT $00, R3
	XMIT $00, R4
	XMIT $00, R5
	XMIT $00, R6
	XMIT $00, R11	;INITIALIZE REGISTERS

	XMIT `HIGH MSC_D_CONTROL, ADDR_HIGH
	XMIT `LOW MSC_D_CONTROL, ADDR_LOW
	XMIT $08, IO7, 8	;ENABLE DATA CACHE CONTROL

	XMIT `LOW MSC_D_PAGE, ADDR_LOW
	XMIT $02, IO7, 8	;SWITCH TO DATA PAGE 2

	XMIT `LOW MSC_D_CONTROL, ADDR_LOW
	XMIT $01, IO7, 8	;RESET DATA CACHE AND DISABLE CONTROL

	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	MOVE RIV7, 8, AUX	;HOLD UNTIL RESET IS DONE

	XMIT `HIGH HEX_BUF_H, ADDR_HIGH
	XMIT `LOW HEX_BUF_H, ADDR_LOW
	XMIT $00, DATA7, 8	;CLEAR HEX HIGH
	XMIT `HIGH HEX_BUF_L, ADDR_HIGH
	XMIT `LOW HEX_BUF_L, ADDR_LOW
	XMIT $00, DATA7, 8	;CLEAR HEX LOW

	XMIT `HIGH RND_L, ADDR_HIGH
	XMIT `LOW RND_L, ADDR_LOW
	XMIT $00, DATA7, 8
	XMIT `HIGH RND_H, ADDR_HIGH
	XMIT `LOW RND_H, ADDR_LOW
	XMIT $00, DATA7, 8

	XMIT `HIGH HEX_INC, AUX
	CALL HEX_INC	;01

	XMIT `HIGH MINI_DUMP_REPLACER, AUX
	CALL MINI_DUMP_REPLACER

	XMIT `HIGH HEX_INC, AUX
	CALL HEX_INC	;02

	;INITIALIZE DISPLAY IN SEMI-GRAPHICS 4 MODE
	XMIT `HIGH DISPLAY_INIT_SG4, AUX
	CALL DISPLAY_INIT_SG4

	XMIT `HIGH HEX_INC, AUX
	CALL HEX_INC	;03

	JMP MAIN

HEX_INC
	XMIT `HIGH HEX_BUF_L, ADDR_HIGH
	XMIT `LOW HEX_BUF_L, ADDR_LOW
	XMIT $01, AUX
	ADD DATA7, 8, AUX
	MOVE AUX, 8, DATA7
	XMIT `HIGH HEX_LOW, ADDR_HIGH
	XMIT `LOW HEX_LOW, ADDR_LOW
	MOVE AUX, 8, IO7
	XMIT `HIGH HEX_BUF_H, ADDR_HIGH
	XMIT `LOW HEX_BUF_H, ADDR_LOW
	MOVE OVF, AUX
	ADD DATA7, 8, AUX
	MOVE AUX, 8, DATA7
	XMIT `HIGH HEX_HIGH, ADDR_HIGH
	XMIT `LOW HEX_HIGH, ADDR_LOW
	MOVE AUX, 8, IO7
	RET

BIG_DUMP
; LOAD END ADDRESS AND MAKE IT NEGATIVE
	XMIT `HIGH DUMP_END_HIGH, IVL
	XMIT `LOW DUMP_END_HIGH, IVR
	MOVE RIV7, 8, R1

	XMIT `HIGH DUMP_END_LOW, IVL
	XMIT `LOW DUMP_END_LOW, IVR
	MOVE RIV7, 8, R2

	XMIT $FF, AUX
	XOR R1, R1
	XOR R2, R2

	XMIT $01, AUX	;WE MAKE THE HIGH AND LOW PARTS NEGATIVE, NOT THE WHOLE THING.
	ADD R2, R2		;THIS IS BECAUSE WE COMPARE IT BY PARTS.
	ADD R1, R1
; LOAD START ADDRESS
	XMIT `HIGH DUMP_START_HIGH, IVL
	XMIT `LOW DUMP_START_HIGH, IVR
	MOVE RIV7, 8, R5

	XMIT `HIGH DUMP_START_LOW, IVL
	XMIT `LOW DUMP_START_LOW, IVR
	MOVE RIV7, 8, R6
BD_LOOP
; READ DATA FROM MEMORY
	MOVE R5, IVL
	MOVE R6, IVR
	MOVE RIV7, 8, R4
; CHECK THAT UART IS READY
	XMIT $FF, IVL
	XMIT $FF, IVR
BD_WAIT
	XMIT $01, AUX
	MOVE LIV5, 1, R3
	XOR R3, R3
	NZT R3, BD_WAIT
	XMIT $FF, IVL
	XMIT $FE, IVR
	MOVE R4, 8, LIV7
; ADD 1 TO ADDRESS
	ADD R6, R6
	MOVE OVF, AUX
	ADD R5, R5
; COMPARE HIGH ADDRESS
	MOVE R5, AUX
	ADD R1, R3
	NZT R3, BD_LOOP
; COMPARE LOW ADDRESS
	MOVE R6, AUX
	ADD R2, R3
	NZT R3, BD_LOOP
	RET

MINI_DUMP_REPLACER
; CONFIGURE DUMP
	XMIT `HIGH DUMP_END_HIGH, IVL
	XMIT `LOW DUMP_END_HIGH, IVR
	XMIT $00, RIV7, 8
	XMIT `HIGH DUMP_END_LOW, IVL
	XMIT `LOW DUMP_END_LOW, IVR
	XMIT $02, RIV3, 4
	XMIT $00, RIV7, 4	;DUMP END VARS NOW CONTAIN $0020

	XMIT `HIGH DUMP_START_HIGH, IVL
	XMIT `LOW DUMP_START_HIGH, IVR
	XMIT $00, RIV7, 8
	XMIT `HIGH DUMP_START_LOW, IVL
	XMIT `LOW DUMP_START_LOW, IVR
	XMIT $00, RIV7, 8	;DUMP START VARS NOW CONTAIN $0000
; CALL BIG DUMP AND GO TO NEXT TARGET
	XMIT `HIGH BIG_DUMP, AUX
	CALL BIG_DUMP
	RET

SERIAL_GET_BYTE
; CHECK THAT UART IS READY
	XMIT $FF, IVL
	XMIT $FF, IVR
SGB_WAIT
	XMIT $01, AUX
	MOVE LIV4, 1, R11 	;READ RX DONE BIT
	XOR R11, R11 			;INVERT RX DONE BIT
	NZT R11, SGB_WAIT	;IF TX NOT READY KEEP WAITING
	XMIT $FE, IVR
	MOVE LIV7, 8, R11
	RET

SERIAL_SEND_BYTE
; CHECK THAT UART IS READY
	XMIT $FF, IVL
	XMIT $FF, IVR
SSB_WAIT
	XMIT $01, AUX
	MOVE LIV5, 1, R6
	XOR R6, R6
	NZT R6, SSB_WAIT
	XMIT $FE, IVR
	MOVE R11, 8, LIV7
	RET

DISPLAY_INIT_SG4
;SET ALL CHARS TO 0X60
;32 * 16 = 512 CHARS
	XMIT $00, R1	;LOW COUNTER
	XMIT $00, R2	;HIGH COUNTER
	XMIT $01, AUX	;IMPLICIT OPERAND FOR INCREMENTS
DI_LOOP
	MOVE R2, IVL
	MOVE R1, IVR
	XMIT $06, LIV3, 4
	XMIT $00, LIV7, 4
	ADD R1, R1
	NZT R1, DI_LOOP
	NZT R2, DI_DONE
	ADD R2, R2
	JMP DI_LOOP
DI_DONE
	RET

MAIN
	XMIT $00, R1	;LOW COUNTER
	XMIT $00, R2	;HIGH COUNTER
	XMIT `HIGH DISPLAY_COUNT_L, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_L, ADDR_LOW
	MOVE R1, 8, DATA7
	XMIT `HIGH DISPLAY_COUNT_H, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_H, ADDR_LOW
	MOVE R2, 8, DATA7
	XMIT `HIGH PREV_VSYNC, ADDR_HIGH
	XMIT `LOW PREV_VSYNC, ADDR_LOW
	XMIT $01, DATA7, 8
	XMIT `LOW FRAME_COUNT, ADDR_LOW
	XMIT $00, DATA7, 8
	XMIT `HIGH HEX_INC, AUX
	CALL HEX_INC	;04
MAIN_LOOP
	XMIT `HIGH PREV_VSYNC, ADDR_HIGH
	XMIT `LOW PREV_VSYNC, ADDR_LOW
	MOVE DATA7, 8, AUX
	NZT AUX, M_PREV		;IF PREV_VSYNC JUMP TO M_PREV
	XMIT `HIGH TIMER_STATUS, ADDR_HIGH
	XMIT `LOW TIMER_STATUS, ADDR_LOW
	MOVE IO3, 1, AUX
	NZT AUX, M_VSYNC	;ELSE IF CURRENT VSYNC JUMP TO M_VSYNC
	;JMP M_RINGS		;ELSE JUMP TO M_RINGS
	JMP MAIN_LOOP	;THIS SHOULD JUMP TO HSYNC DETECTION
M_VSYNC
	XMIT `HIGH HEX_INC, AUX
	CALL HEX_INC	;05
	XMIT `HIGH FRAME_COUNT, ADDR_HIGH
	XMIT `LOW FRAME_COUNT, ADDR_LOW
	XMIT $01, AUX
	XMIT 60, R5
	ADD DATA7, 8, R4	;GET AND INCREMENT FRAME COUNTER
	XMIT `HIGH DIV8, AUX
	CALL DIV8	;COMPUTE FRAME_COUNT % 60
	MOVE R4, 8, DATA7	;STORE RESULT IN FRAME COUNTER
	NZT R4, M_PREV	;IF FRAME_COUNT NOT ZERO JUMP TO M_PREV
	XMIT `HIGH M_RING_2, AUX
	CALL M_RING_2	;ELSE CALL M_RING_2
M_PREV
	XMIT `HIGH TIMER_STATUS, ADDR_HIGH
	XMIT `LOW TIMER_STATUS, ADDR_LOW
	MOVE IO3, 1, AUX	;GET CURRENT VSYNC
	XMIT `HIGH PREV_VSYNC, ADDR_HIGH
	XMIT `LOW PREV_VSYNC, ADDR_LOW
	MOVE AUX, 8, DATA7	;UPDATE PREV_VSYNC

	JMP MAIN_LOOP

M_RING_2
	XMIT `HIGH RND_BUILD_BYTE, AUX
	CALL RND_BUILD_BYTE
	;XMIT `HIGH RND_GET_WORD, AUX
	;CALL RND_GET_WORD

	XMIT `HIGH RND_L, ADDR_HIGH
	XMIT `LOW RND_L, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	;XMIT `HIGH HEX_INC, AUX
	;CALL HEX_INC
	RET

M_PUT_CHARS
	XMIT $02, R3	;END ADDRESS HIGH
	XMIT `HIGH DISPLAY_COUNT_L, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_L, ADDR_LOW
	MOVE DATA7, 8, R1
	XMIT `HIGH DISPLAY_COUNT_H, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_H, ADDR_LOW
	MOVE DATA7, 8, R2
	XMIT `HIGH BYTE_TO_HEX, AUX
	CALL BYTE_TO_HEX
	MOVE R6, R5	;COPY LOW CHAR TO R5
M_PUT_FIRST
	XMIT `HIGH SERIAL_SEND_BYTE, AUX
	CALL SERIAL_SEND_BYTE	;SEND CHAR

	XMIT $01, AUX
	MOVE R2, IVL
	MOVE R1, IVR	;SET DISPLAY ADDRESS
	MOVE R11, 8, LIV7	;OUTPUT DATA

	ADD R1, R1	;INCREMENT LOW ADDRESS
	NZT R1, M_PUT_SECOND	;IF NOT ZERO CONTINUE TO NEXT CHAR

	ADD R2, AUX
	MOVE AUX, R2	;ELSE INCREMENT HIGH ADDRESS (PLACE RESULT IN BOTH R2 AND AUX)

	XOR R3, AUX	;COMPARE HIGH ADDRESS WITH HIGH END ADDRESS
	NZT AUX, M_PUT_SECOND	;IF NOT END ADDRESS CONTINUE TO NEXT CHAR
	XMIT $00, R1	;LOW COUNTER
	XMIT $00, R2	;HIGH COUNTER
M_PUT_SECOND
	MOVE R5, R11	;PUT LOW CHAR IN R11
	XMIT `HIGH SERIAL_SEND_BYTE, AUX
	CALL SERIAL_SEND_BYTE	;SEND CHAR

	XMIT $01, AUX
	MOVE R2, IVL
	MOVE R1, IVR	;SET DISPLAY ADDRESS
	MOVE R11, 8, LIV7	;OUTPUT DATA

	ADD R1, R1	;INCREMENT LOW ADDRESS
	NZT R1, MPS_DONE	;IF NOT ZERO LOOP BACK

	ADD R2, AUX
	MOVE AUX, R2	;ELSE INCREMENT HIGH ADDRESS (PLACE RESULT IN BOTH R2 AND AUX)

	XOR R3, AUX	;COMPARE HIGH ADDRESS WITH HIGH END ADDRESS
	NZT AUX, MPS_DONE
	XMIT $00, R1	;LOW COUNTER
	XMIT $00, R2	;HIGH COUNTER
MPS_DONE
	XMIT `HIGH DISPLAY_COUNT_L, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_L, ADDR_LOW
	MOVE R1, 8, DATA7
	XMIT `HIGH DISPLAY_COUNT_H, ADDR_HIGH
	XMIT `LOW DISPLAY_COUNT_H, ADDR_LOW
	MOVE R2, 8, DATA7
	RET

RND_GET_WORD
	XMIT `HIGH RND_L, ADDR_HIGH
	XMIT `LOW RND_L, ADDR_LOW
	XMIT $01, AUX
	XOR DATA7, 1, AUX
	XOR DATA0, 1, AUX	;COMPUTE NEW MSB AND PUT IT IN AUX
	XMIT `HIGH RND_H, ADDR_HIGH
	XMIT `LOW RND_H, ADDR_LOW
	MOVE DATA7, 1, R11	;STORE LSB OF RND_H IN R11
	MOVE DATA6, 7, DATA7	;RIGHT SHIFT RND_H
	MOVE AUX, 1, DATA0	;STORE NEW MSB IN RND_H
	XMIT `HIGH RND_L, ADDR_HIGH
	XMIT `LOW RND_L, ADDR_LOW
	MOVE DATA6, 7, DATA7	;RIGHT SHIFT RND_L
	MOVE R11, 1, DATA0	;STORE LSB OR RND_H IN RND_L
	RET

RND_BUILD_BYTE
	XMIT $08, R6
	JMP RND_BUILD_LOOP
RND_BUILD_WORD
	XMIT $10, R6
RND_BUILD_LOOP
	XMIT `HIGH RND_GET_WORD, AUX
	CALL RND_GET_WORD
	XMIT $FF, AUX
	ADD R6, R6
	NZT R6, RND_BUILD_LOOP
	RET

TEMP EQU R3
NUMERATOR EQU R4
DENOMINATOR EQU R5
INDEX EQU R6
RESULT EQU R11

DIV8
	NZT DENOMINATOR, DIV8_NZ
	RET	;IF DENOMINATOR IS ZERO RETURN
DIV8_NZ
	XMIT $00, RESULT
	XMIT $01, INDEX	;INITIALIZE RESULT AND INDEX
SHIFT_IT8
	XMIT $80, AUX
	AND DENOMINATOR, AUX
	NZT AUX, DIV8LOOP	;IF MSB OF DENOMINATOR IS SET GOTO DIV8LOOP
	MOVE INDEX, AUX
	ADD INDEX, INDEX
	MOVE DENOMINATOR, AUX
	ADD DENOMINATOR, DENOMINATOR	;LEFT-SHIFT DENOMINATOR AND INDEX
	JMP SHIFT_IT8
DIV8LOOP
	XMIT $FF, AUX
	XOR DENOMINATOR, TEMP
	XMIT $01, AUX
	ADD TEMP, TEMP	;MAKE DENOMINATOR NEGATIVE AND STORE IT IN TEMP
	MOVE NUMERATOR, AUX
	ADD TEMP, TEMP	;SUBTRACT DENOMINATOR FROM NUMERATOR AND STORE RESULT IN TEMP
	XMIT $01, AUX
	XOR OVF, AUX	;FLIP CARRY BIT
	NZT AUX, FINAL8	;IF RESULT IS NEGATIVE GOTO FINAL8
	MOVE TEMP, NUMERATOR	;MAKE THE RESULT THE NEW NUMERATOR
	MOVE INDEX, AUX
	ADD RESULT, RESULT	;ADD INDEX TO THE RESULT
FINAL8
	XMIT $7F, AUX
	AND DENOMINATOR(1), DENOMINATOR
	AND INDEX(1), INDEX
	NZT INDEX, DIV8LOOP
	RET

MUL_IN_A EQU R3
MUL_IN_B_H EQU R4
MUL_IN_B_L EQU R5
MUL_OUT_H EQU R6
MUL_OUT_L EQU R11

MUL_8_16
	XMIT $00, MUL_OUT_H
	XMIT $00, MUL_OUT_L
MACC_8_16
	XMIT $01, AUX
	AND MUL_IN_A, AUX	;CHECK IF LSB OF IN_A IS SET
	NZT AUX, MACC_8_16_ADD	;IF IT IS ADD IN_B TO RESULT
;THE FOLLOWING CODE IS DUPLICATED FOR PERFORMANCE REASONS
	XMIT $7F, AUX
	AND MUL_IN_A(1), MUL_IN_A	;RIGHT SHIFT IN_A
	MOVE MUL_IN_B_H, AUX
	ADD MUL_IN_B_H, MUL_IN_B_H
	MOVE MUL_IN_B_L, AUX
	ADD MUL_IN_B_L, MUL_IN_B_L
	MOVE OVF, AUX
	ADD MUL_IN_B_H, MUL_IN_B_H	;LEFT SHIFT IN_B
	NZT MUL_IN_A, MACC_8_16
	RET
MACC_8_16_ADD
	MOVE MUL_IN_B_H, AUX
	ADD MUL_OUT_H, MUL_OUT_H
	MOVE MUL_IN_B_L, AUX
	ADD MUL_OUT_L, MUL_OUT_L
	MOVE OVF, AUX
	ADD MUL_OUT_H, MUL_OUT_H	;ADD MUL_IN_B TO MUL_OUT
;THE FOLLOWING CODE IS DUPLICATED FOR PERFORMANCE REASONS
	XMIT $7F, AUX
	AND MUL_IN_A(1), MUL_IN_A	;RIGHT SHIFT IN_A
	MOVE MUL_IN_B_H, AUX
	ADD MUL_IN_B_H, MUL_IN_B_H
	MOVE MUL_IN_B_L, AUX
	ADD MUL_IN_B_L, MUL_IN_B_L
	MOVE OVF, AUX
	ADD MUL_IN_B_H, MUL_IN_B_H	;LEFT SHIFT IN_B
	NZT MUL_IN_A, MACC_8_16
	RET

MAIN_SIM
	XMIT `HIGH RND_GET_WORD, AUX
	CALL RND_GET_WORD
	JMP MAIN_SIM

BYTE_TO_HEX
	XMIT $0F, AUX
	AND R11, R5	;GET LOW NIBBLE IN R5

	XMIT `HIGH S_HEX, AUX
	CALL S_HEX
	MOVE R5, R6	;STORE LOW CHAR IN R6

	XMIT $0F, AUX
	AND R11(4), R5	;GET HIGH NIBBLE IN R5

	XMIT `HIGH S_HEX, AUX
	CALL S_HEX
	MOVE R5, R11	;STORE HIGH CHAR IN R11
	RET

;NOTE: LABELS ON ORG STATEMENTS CANNOT BE REFERENCED IN THE CODE! THESE DO NOT BELONG TO ANY SEGMENT AND WILL NOT BE FOUND.
STRINGS	ORG $0400
S_HEX
	XEC HEX_0 (R5)	;CHARS 0 - F
	RET
HEX_0 XMIT $30, R5
	XMIT $31, R5
	XMIT $32, R5
	XMIT $33, R5
	XMIT $34, R5
	XMIT $35, R5
	XMIT $36, R5
	XMIT $37, R5
	XMIT $38, R5
	XMIT $39, R5
	XMIT $41, R5
	XMIT $42, R5
	XMIT $43, R5
	XMIT $44, R5
	XMIT $45, R5
	XMIT $46, R5

	ORG $0FFF	;PAD FILE TO 4K WORDS
	JMP INIT