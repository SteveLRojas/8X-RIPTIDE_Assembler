; Address map for left bank:
; 0x0000 to 0x0FFF video memory (read write)
; 0x1000 to 0xFFF3	unused
; 0xFFF4 to 0xFFF5 keyboard module (read write)
; 0xFFF6 to 0xFFF7 video mode register (write only)
; 0xFFF8 to 0xFFFB memory subsystem control registers	(write only)
; 0xFFFC to 0xFFFD	HEX display registers	(write only)
; 0xFFFE to 0xFFFF	RS-232 module	(read write)

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

R3_SAVE_LOC		EQU $F007

BUF_0 EQU $F015
BUF_1 EQU $F016
BUF_2 EQU $F017
BUF_3 EQU $F018
BUF_4 EQU $F019
BUF_5 EQU $F01A
BUF_6 EQU $F01B

	ORG 0
INIT
	XMIT $FF, IVL
	XMIT $FC, IVR
	XMIT $00, LIV3, 4
	XMIT $01, LIV7, 4	;SET HEX LOW
	XMIT $FF, IVL
	XMIT $FD, IVR
	XMIT $00, LIV7, 8	;SET HEX HIGH

	XMIT $00, AUX
	XMIT $00, R1
	XMIT $00, R2
	XMIT $00, R3
	XMIT $00, R4
	XMIT $00, R5
	XMIT $00, R6
	XMIT $00, R11	;INITIALIZE REGISTERS

	;LEAVE THE PROGRAM CACHE ALONE FOR NOW
	;XMIT $FF, IVL
	;XMIT $F8, IVR
	;XMIT $08, LIV7, 8	;ENABLE PROGRAM CACHE CONTROL

	XMIT $FF, IVL
	XMIT $FA, IVR
	XMIT $08, LIV7, 8	;ENABLE DATA CACHE CONTROL

	XMIT $FB, IVR
	XMIT $02, LIV7, 8	;SWITCH TO DATA PAGE 2

	XMIT $FA, IVR
	XMIT $01, LIV7, 8	;RESET DATA CACHE AND DISABLE CONTROL

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

	XMIT $FF, IVL
	XMIT $FC, IVR
	XMIT $00, LIV3, 4
	XMIT $02, LIV7, 4	;SET HEX LOW

	XMIT `HIGH MINI_DUMP_REPLACER, AUX
	;CALL MINI_DUMP_REPLACER
	NOP

	XMIT $FF, IVL
	XMIT $FC, IVR
	XMIT $00, LIV3, 4
	XMIT $03, LIV7, 4	;SET HEX LOW

	;INITIALIZE DISPLAY IN COLOR GRAPHICS 3 MODE
	;XMIT `HIGH DISPLAY_INIT_CG3, AUX
	;CALL DISPLAY_INIT_CG3
	;INITIALIZE DISPLAY IN SEMI-GRAPHICS 4 MODE
	XMIT `HIGH DISPLAY_INIT_SG4, AUX
	CALL DISPLAY_INIT_SG4

	JMP MAIN

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

DISPLAY_INIT_CG3
;SET ALL DISPLAY ELEMENTS TO 3 (RED)
;4 ELEMENTS PER BYTE; SET ALL BYTES TO 0XFF
;128 * 96 / 4 = 3072 BYTES
;TWO LOOPS; 12 * 256 = 3072
	XMIT $FF, IVL
	XMIT $F7, IVR	;VIDEO MODE REGISTER ADDRESS
	XMIT $01, LIV7, 1	;SET VIDEO MODE TO COLOR GRAPHICS 3
	XMIT $00, R1	;LOW COUNTER
	XMIT 12, R2		;HIGH COUNTER
	XMIT $FF, AUX	;IMPLICIT OPERAND FOR DECREMENTS
DI_CG3_LOOP
	ADD R2, IVL	;R2 - 1 -> IVL
	ADD R1, IVR	;R1 - 1 -> IVR
	XMIT $0F, LIV3, 4
	XMIT $0F, LIV7, 4
	ADD R1, R1
	NZT R1, DI_CG3_LOOP
	ADD R2, R2
	NZT R2, DI_CG3_LOOP
	RET

MAIN
	XMIT $00, R1	;LOW COUNTER
	XMIT $00, R2	;HIGH COUNTER
	XMIT $02, R3	;END ADDRESS HIGH
	XMIT $FF, IVL
	XMIT $FC, IVR
	XMIT $00, LIV3, 4
	XMIT $04, LIV7, 4	;SET HEX LOW

MAIN_LOOP
	XMIT `HIGH R3_SAVE_LOC, ADDR_HIGH
	XMIT `LOW R3_SAVE_LOC, ADDR_LOW
	MOVE R3, 8, DATA7	;SAVE R3

	XMIT `HIGH SERIAL_GET_BYTE, AUX
	CALL SERIAL_GET_BYTE	;GET FIRST ARG HIGH

	XMIT `HIGH BUF_0, ADDR_HIGH
	XMIT `LOW BUF_0, ADDR_LOW
	MOVE R11, 8, DATA7	;STORE DATA IN BUF_0
	XMIT `HIGH SERIAL_SEND_BYTE, AUX
	CALL SERIAL_SEND_BYTE

	XMIT `HIGH SERIAL_GET_BYTE, AUX
	CALL SERIAL_GET_BYTE	;GET FIRST ARG LOW

	XMIT `HIGH BUF_1, ADDR_HIGH
	XMIT `LOW BUF_1, ADDR_LOW
	MOVE R11, 8, DATA7	;STORE DATA IN BUF_1
	MOVE R11, R4	;STORE DATA IN NUMERATOR
	XMIT `HIGH SERIAL_SEND_BYTE, AUX
	CALL SERIAL_SEND_BYTE

	XMIT `HIGH SERIAL_GET_BYTE, AUX
	CALL SERIAL_GET_BYTE	;GET SECOND ARG

	XMIT `HIGH BUF_2, ADDR_HIGH
	XMIT `LOW BUF_2, ADDR_LOW
	MOVE R11, 8, DATA7	;STORE DATA IN BUF_2
	MOVE R11, R5	;STORE DATA IN DENOMINATOR
	XMIT `HIGH SERIAL_SEND_BYTE, AUX
	CALL SERIAL_SEND_BYTE

	XMIT `HIGH DIV8, AUX
	CALL DIV8	;DO THE DIVISION

	XMIT `HIGH BUF_3, ADDR_HIGH
	XMIT `LOW BUF_3, ADDR_LOW
	MOVE R11, 8, DATA7	;STORE RESULT IN BUF_3
	XMIT `HIGH BUF_4, ADDR_HIGH
	XMIT `LOW BUF_4, ADDR_LOW
	MOVE R4, 8, DATA7	;STORE REMAINDER IN BUF_4

	XMIT `HIGH BUF_2, ADDR_HIGH
	XMIT `LOW BUF_2, ADDR_LOW
	MOVE DATA7, 8, R3	;STORE SECOND ARG IN MUL_IN_A
	XMIT `HIGH BUF_0, ADDR_HIGH
	XMIT `LOW BUF_0, ADDR_LOW
	MOVE DATA7, 8, R4	;STORE FIRST ARG HIGH IN MUL_IN_B_H
	XMIT `HIGH BUF_1, ADDR_HIGH
	XMIT `LOW BUF_1, ADDR_LOW
	MOVE DATA7, 8, R5	;STORE FIRST ARG LOW IN MUL_IN_B_L

	XMIT `HIGH MUL_8_16, AUX
	CALL MUL_8_16	;DO THE MULTIPLICATION

	XMIT `HIGH BUF_5, ADDR_HIGH
	XMIT `LOW BUF_5, ADDR_LOW
	MOVE R6, 8, DATA7	;STORE MUL_OUT_H IN BUF_5
	XMIT `HIGH BUF_6, ADDR_HIGH
	XMIT `LOW BUF_6, ADDR_LOW
	MOVE R11, 8, DATA7	;STORE MUL_OUT_L IN BUF_6

	XMIT `HIGH BUF_0, ADDR_HIGH
	XMIT `LOW BUF_0, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_1, ADDR_HIGH
	XMIT `LOW BUF_1, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_2, ADDR_HIGH
	XMIT `LOW BUF_2, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_3, ADDR_HIGH
	XMIT `LOW BUF_3, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_4, ADDR_HIGH
	XMIT `LOW BUF_4, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_5, ADDR_HIGH
	XMIT `LOW BUF_5, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT `HIGH BUF_6, ADDR_HIGH
	XMIT `LOW BUF_6, ADDR_LOW
	MOVE DATA7, 8, R11
	XMIT `HIGH M_PUT_CHARS, AUX
	CALL M_PUT_CHARS

	XMIT $FF, IVL
	XMIT $FC, IVR
	XMIT $00, LIV3, 4
	XMIT $05, LIV7, 4	;SET HEX LOW

	JMP MAIN_LOOP

M_PUT_CHARS
	XMIT `HIGH R3_SAVE_LOC, ADDR_HIGH
	XMIT `LOW R3_SAVE_LOC, ADDR_LOW
	MOVE DATA7, 8, R3	;RESTORE R3

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
	RET

; PADDING FOR ALIGNING BRANCH BOUNDARIES
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
	NOP
	NOP
	NOP
	NOP

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