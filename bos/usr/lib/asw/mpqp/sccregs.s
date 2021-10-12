	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: None
;
;  ORIGINS: 27
;
;  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
;  combined with the aggregated modules for this product)
;                   SOURCE MATERIALS
;  (C) COPYRIGHT International Business Machines Corp. 1988, 1989
;  All Rights Reserved
;
;  US Government Users Restricted Rights - Use, duplication or
;  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;
;  FUNCTION: Module contains data definitions for default SCC register
;	     setups for all supported data transfer and dial protocols.
;
;**********************************************************************
;
	TITLE	sccregs
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)66	1.6  src/bos/usr/lib/asw/mpqp/sccregs.s, ucodmpqp, bos411, 9428A410j 5/17/94 09:00:06'
	db	0
	.XLIST
	include	define.inc
	include	duscc.inc
	.LIST

; These are the tables which setup a Signetics DUSCC to do any of a number
; of functions.

	PAGE
;
; The normal SDLC data protocol setup, External clocking by default.
;
	public	_scc_def_sdlc
_scc_def_sdlc equ $
	db	00000000b	; CMR1 : BOP Primary, single octet address,
				;      : no parity.  Select NRZ
	db	00011111b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : CRC CCITT Preset 1
	db	0		; SYN1 : Address compare, first byte
	db	0		; SYN2 : Address compare, second byte
	db	11111111b	; TPR  : Abort/Flag on underrun, Idle sync, RTS
				;      : Ctl, CTS enable Tx, 8 bit char, TEOM
	db	10000000b	; TTR  : TRxC clocking at 1x
	db	00001011b	; RPR  : All Party Address, 8 bit character
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	11100000b	; OMR  : Residual Length 8 bits, RTS off
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; The X.21 SDLC data protocol setup, External clocking by default.
;
	public	_scc_x21_sdlc
_scc_x21_sdlc equ $
	db	00000000b	; CMR1 : BOP Primary, single octet address,
				;      : no parity.  Select NRZ
	db	00011111b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : CRC CCITT Preset 1
	db	0		; SYN1 : Address compare, first byte
	db	0		; SYN2 : Address compare, second byte
	db	11110011b	; TPR  : Abort/Flag on underrun, Idle sync,
				;      : 8 bit char, TEOM
	db	00000000b	; TTR  : RTxC clocking at 1x
	db	00001011b	; RPR  : All Party Address, 8 bit character
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	11100001b	; OMR  : Residual Length 8 bits, RTS on
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; BISYNC - EBCDIC default register settings (External clock)
;
	public	_scc_def_bi_e
_scc_def_bi_e equ $
	db	00000101b	; CMR1 : COP Bisync, dual syn,
				;      : EBCDIC.  Select NRZ
	db	00011100b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : CRC 16 Preset 0s
	db	032h            ; SYN1 : Address compare, first byte
	db	032h  		; SYN2 : Address compare, second byte
	db	00111111b	; TPR  : FCS Idle, Idle sync, RTS/CTS control
				;      : TEOM, 8-bit characters
	db	10000000b	; TTR  : TRxC clocking at 1x
	db	10100011b	; RPR  : 8 bit character,
				;      : SYN strip, Auto Hunt and chk pad
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000000b	; OMR  : RTS off
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; BISYNC - ASCII/Odd Parity default register settings (External clock)
;
	public	_scc_def_bi_a
_scc_def_bi_a equ $
	db	00100101b	; CMR1 : COP Bisync, dual syn,
				;      : ASCII Odd parity.  Select NRZ
	db	00011010b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : LRC 8 Preset 0s
	db	016h            ; SYN1 : Address compare, first byte
	db	016h  		; SYN2 : Address compare, second byte
	db	00111111b	; TPR  : FCS Idle, Idle sync, RTS/CTS control
				;      : TEOM, 8-bit characters
	db	10000000b	; TTR  : TRxC clocking at 1x
	db	11100011b	; RPR  : 8 bit character, don't parity strip,
				;      : SYN strip, Auto Hunt and chk pad
				;      : Pass FCS to FIFO
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000000b	; OMR  : RTS off
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; X.21 BISYNC - EBCDIC default register settings (External clock)
;
	public	_scc_x21_bi_e
_scc_x21_bi_e equ $
	db	00000101b	; CMR1 : COP Bisync, dual syn,
				;      : EBCDIC.  Select NRZ
	db	00011100b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : CRC 16 Preset 0s
	db	032h            ; SYN1 : Address compare, first byte
	db	032h  		; SYN2 : Address compare, second byte
	db	00110011b	; TPR  : FCS Idle, Idle sync, 
				;      : TEOM, 8-bit characters
	db	00000000b	; TTR  : RTxC clocking at 1x
	db	10100011b	; RPR  : 8 bit character,
				;      : SYN strip, Auto Hunt and chk pad
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000001b	; OMR  : RTS on
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; X.21 BISYNC - ASCII/Odd Parity default register settings (External clock)
;
	public	_scc_x21_bi_a
_scc_x21_bi_a equ $
	db	00100101b	; CMR1 : COP Bisync, dual syn,
				;      : ASCII Odd parity.  Select NRZ
	db	00011010b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : LRC 8 Preset 0s
	db	016h            ; SYN1 : Address compare, first byte
	db	016h  		; SYN2 : Address compare, second byte
	db	00110011b	; TPR  : FCS Idle, Idle sync, 
				;      : TEOM, 8-bit characters
	db	00000000b	; TTR  : RTxC clocking at 1x
	db	11100011b	; RPR  : 8 bit character, don't parity strip,
				;      : SYN strip, Auto Hunt and chk pad
				;      : Pass FCS to FIFO
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000001b	; OMR  : RTS on
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; Asynchronous setup, 2400 Baud, 7 bit Odd parity
;
	public	_scc_def_asy
_scc_def_asy equ $
	db	00110111b	; CMR1 : Async, odd parity
	db	00011000b	; CMR2 : Normal mode, FDX/Dual Address DMA,
	db	00h             ; SYN1 : No syn/addr compare value
	db	00h  		; SYN2 : No syn/addr compare value
	db	01111010b	; TPR  : 1 Stop bit, 7 bit char
	db	00111011b	; TTR  : BRG, 2400 baud
	db	00000010b	; RPR  : 7 bit character
	db	00101011b	; RTR  : RTxC from BRG, 2400 baud
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000001b	; OMR  : RTS on
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

	PAGE
;
; X.21 Call Establishment default settings.
;
	public	_scc_def_x21
_scc_def_x21 equ $
	db	00000100b	; CMR1 : COP Dual syn, no parity
	db	00011000b	; CMR2 : Normal mode, FDX/Dual Address DMA,
				;      : No CRC generation
	db	016h            ; SYN1 : IA5 SYN
	db	016h  		; SYN2 : IA5 SYN
	db	11010011b	; TPR  : SYN on underrun
				;      : 8 bit char, mark on Idle
	db	00000000b	; TTR  : RTxC, 1xExternal 
	db	10001011b	; RPR  : 8 bit character, Strip parity & SYN
	db	00000000b	; RTR  : RTxC Clocking at 1x
	db	16h,80h		; CTPH, CTPL : Counter/Timer Presets, 0.1 Sec
	db	10011010b	; CTC  : C/T Source is Crystal/4, prescale 64
				;      : Interrupt, Preset on zero count
	db	00000001b	; OMR  : RTS on
	db	0,0		; CTH, CTL : Counter/Timer current value Hi/Lo
	db	00100000b	; PCR  : FDX DMA, RTS output, Rx, TxClk inputs
	db	10000001b	; CCR  : Stop the counter/timer
;
	db	0,0,0,0,0,0,0,0	; TxFIFO, RxFIFO
;
	db	0,0		; RSR, TRSR : Cleared via CCR commands
	db	0FFh		; ICTSR : Clear all bits
	db	7 DUP(0)	; others

_TEXT	ends
	end
