;                     /\         /\__
;                   // \       (  0 )_____/\            __
;                  // \ \     (vv          o|          /^v\
;                //    \ \   (vvvv  ___-----^        /^^/\vv\
;              //  /     \ \ |vvvvv/               /^^/    \v\
;             //  /       (\\/vvvv/              /^^/       \v\
;            //  /  /  \ (  /vvvv/              /^^/---(     \v\
;           //  /  /    \( /vvvv/----(O        /^^/           \v\
;          //  /  /  \  (/vvvv/               /^^/             \v|
;        //  /  /    \( vvvv/                /^^/               ||
;       //  /  /    (  vvvv/                 |^^|              //
;      //  / /    (  |vvvv|                  /^^/            //
;     //  / /   (    \vvvvv\          )-----/^^/           //
;    // / / (          \vvvvv\            /^^^/          //
;   /// /(               \vvvvv\        /^^^^/          //
;  ///(              )-----\vvvvv\    /^^^^/-----(      \\
; //(                        \vvvvv\/^^^^/               \\
;/(                            \vvvv^^^/                 //
;                                \vv^/         /        //
;                                             /<______//
;                                            <<<------/
;                                             \<
;                                              \
;**************************************************
;* RIPTIDE-II.INC     HEADER FILE                 *
;* Copyright (C) 2021 Esteban Looser-Rojas.       *
;* Contains register defines the RIPTIDE-II CPU   *
;* and address defines for platform peripherals.  *
;* Addresses are for the RIPTIDE-II test platform.*
;**************************************************

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

; keyboard module address map
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

IVL EQU R7
IVR EQU R17
AUX EQU R0
OVF EQU R10

DATA0 EQU RIV0
DATA1 EQU RIV1
DATA2 EQU RIV2
DATA3 EQU RIV3
DATA4 EQU RIV4
DATA5 EQU RIV5
DATA6 EQU RIV6
DATA7 EQU RIV7

IO0 EQU LIV0
IO1 EQU LIV1
IO2 EQU LIV2
IO3 EQU LIV3
IO4 EQU LIV4
IO5 EQU LIV5
IO6 EQU LIV6
IO7 EQU LIV7

ADDR_HIGH EQU IVL
ADDR_LOW EQU IVR

KB_DATA EQU $FFF4
KB_STATUS EQU $FFF5

V_MODE EQU $FFF6
VGA_MODE EQU $FFF7	;THESE TWO ARE THE SAME REGISTER

MSC_P_CONTROL EQU $FFF8
MSC_P_PAGE EQU $FFF9
MSC_D_CONTROL EQU $FFFA
MSC_D_PAGE EQU $FFFB

HEX_LOW EQU $FFFC
HEX_HIGH EQU $FFFD

UART_DATA EQU $FFFE
UART_STATUS EQU $FFFF
