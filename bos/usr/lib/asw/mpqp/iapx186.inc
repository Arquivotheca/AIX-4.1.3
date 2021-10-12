	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
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
;  FUNCTION: 
;
;**********************************************************************
;
;	"@(#)71	1.3  src/bos/usr/lib/asw/mpqp/iapx186.inc, ucodmpqp, bos411, 9428A410j 1/30/91 17:11:56"
;
;	This file contains the internal register offsets for 186 internal
;	DMA and Timers.
;
PER_CB		EQU	0FF00h		; All references made		051989
;
;*****************************************************************************
;	Internal Timer Register Store
;
;	+----------+-----+-----+-----+
;	| Reg Name | T 0 | T 1 | T 2 |
;	+----------+-----+-----+-----+
;	| Mode/Ctl | 56h | 5Eh | 66h |
;	| MaxCnt B | 54h | 5Ch | --- |
;	| MaxCnt A | 52h | 5Ah | 62h |
;	| Count    | 50h | 58h | 60h |
;	+----------+-----+-----+-----+
;
TMR0_BASE	EQU	PER_CB+50h	; Timer 0 base address
TMR1_BASE	EQU	PER_CB+58h	; Timer 1 base address
TMR2_BASE	EQU	PER_CB+60h	; Timer 2 base address

TMR_CNT		EQU	0h		; Count Register offset
TMR_MAX_A	EQU	2h		; Maximum Count, A
TMR_MAX_B	EQU	4h		; Maximum Count, B
TMR_MCTL	EQU	6h		; Mode/Control Register offset

TMR_M_CONT	EQU	0001h	; Timer Continuous (1), Halt on Max Count (0)
TMR_M_ALT	EQU	0002h	; Use MAX A (0), Alternate MAX A,B (1)
TMR_M_EXT	EQU	0004h	; External (1) or Internal (0) Clock Source
TMR_M_PRE	EQU	0008h	; Prescale from timer 2? (1 = Yes)
TMR_M_RTG	EQU	0010h	; Retrigger on Input Pin (1 = Yes)
TMR_M_MC	EQU	0020h	; Maximum Count Reached? (1 = Yes) for Polling

TMR_M_RIU	EQU	1000h	; Max Count Register In Use (1 = MAX B)
TMR_M_INT	EQU	2000h	; Terminal Count Interrupt Enable (1 = Enabled)
TMR_M_INH	EQU	4000h	; Inhibit writing of EN (0 = Inhibit)
TMR_M_EN	EQU	8000h	; Enable Counting from selected Input (1)

TMR_C_10MS	EQU	7A12h	; 31250 * 320ns = 10ms
TMR_C_5MS	EQU	3D09h	; 15625 * 320ns =  5ms
TMR_100MS	EQU	10h	;  10 @ 10ms = 0.1s ( 100ms) Prescaled
TMR_1S		EQU	100h	; 100 @ 10ms = 1.0s (1000ms) Prescaled

;
;*****************************************************************************
;	Internal DMA Register Store
;
;	+-------------+-----+-----+
;	| Reg Name    | D 0 | D 1 |
;	+-------------+-----+-----+
;	| Control     | CAh | DAh |
;	| Xfer Count  | C8h | D8h |
;	| Dest. Ext.  | C6h | D6h |
;	| Dest.       | C4h | D4h |
;	| Source Ext. | C2h | D2h |
;	| Source Ptr. | C0h | C0h |
;	+-------------+-----+-----+
;
DMA0_BASE	EQU	PER_CB+0C0h	; DMA Channel 0 Base Address
DMA1_BASE	EQU	PER_CB+0D0h	; DMA Channel 1 Base Address
;
DMA_SRC		EQU	00h	; Source Address
DMA_SRCE	EQU	02h	; Source Address Extension
DMA_DST		EQU	04h	; Destination Address
DMA_DSTE	EQU	06h	; Destination Address Extension
DMA_CNT		EQU	08h	; Transfer Count
DMA_CTL		EQU	0Ah	; Control Word

DMA_C_WORD	EQU	0001h	; byte/WORD Transfers
DMA_C_START	EQU	0002h	; stop/START the Channel
DMA_C_CHANGE	EQU	0004h	; no change/CHANGE the stop/START bit on write
DMA_C_TDRQ	EQU	0010h	; disable/ENABLE DMA Requests from Timer 2
DMA_C_PRI	EQU	0020h	; low/HIGH Priority relative to other channel
DMA_C_NOSYN	EQU	0000h	; DMA Transfer unsynchronized
DMA_C_SRC_SYN	EQU	0040h	; DMA Transfer Source Synchronized
DMA_C_DST_SYN	EQU	0080h	; DMA Transfer Destination Synchronized
DMA_C_INT	EQU	0100h	; disable/ENABLE Terminal Count Interrupt
DMA_C_TERM	EQU	0200h	; continue/STOP DMA on Terminal Count Reached
DMA_C_S_INC	EQU	0400h	; Increment Source Address
DMA_C_S_DEC	EQU	0800h	; Decrement Source Address
DMA_C_S_MEM	EQU	1000h	; io/MEMORY Source Address
DMA_C_D_INC	EQU	2000h	; Increment Destination Address
DMA_C_D_DEC	EQU	4000h	; Decrement Destination Address
DMA_C_D_MEM	EQU	8000h	; io/MEMORY Destination Address

;
;*****************************************************************************
;	Internal Timer Register Store
;
NONSPEOI EQU	8000h			; Non-specific EOI Command to 80C186
EOIREG   EQU	PER_CB+22h		; 80C186 Interrupt Clear location

$$NSEOI	macro
	mov	dx,EOIREG		;
	mov	ax,NONSPEOI		;
	out	dx,ax			;
	endm

$$IRET	macro
	push	ax			;
	push	dx			;
	mov	dx,EOIREG		;
	mov	ax,NONSPEOI		;
	out	dx,ax			;
	pop	dx			;
	pop	ax			;
	iret				;
	endm

;
;*****************************************************************************
;	CPU internals
;
OF_ON	EQU	800h			; OVERflow flag ON
OF_OFF	EQU	NOT OF_ON		; OVERflow flag OFF
DF_ON	EQU	400h			; DIRection flag ON
DF_OFF	EQU	NOT DF_ON		; DIRection flag OFF
IF_ON	EQU	200h			; INTerrupt enable flag ON
IF_OFF	EQU	NOT IF_ON		; INTerrupt enable flag OFF
TF_ON	EQU	100h			; TRAP flag ON
TF_OFF	EQU	NOT TF_ON		; TRAP flag OFF
SF_ON	EQU	80h			; SIGN flag ON
SF_OFF	EQU	NOT SF_ON		; SIGN flag OFF
ZF_ON	EQU	40h			; ZERO flag ON
ZF_OFF	EQU	NOT ZF_ON		; ZERO flag OFF
AF_ON	EQU	10h			; AUXilliary Carry flag ON
AF_OFF	EQU	NOT AF_ON		; AUXilliary Carry flag OFF
PF_ON	EQU	4h			; Parity flag ON
PF_OFF	EQU	NOT PF_ON		; Parity flag OFF
CF_ON	EQU	1h			; Carry flag ON
CF_OFF	EQU	NOT CF_ON		; Carry flag OFF
