	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: none
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
;  FUNCTION: Global data declaration for the linker-created copy of
;	     the adapter software interrupt vector table.  Used once
;	     during initialization only.
;
;**********************************************************************
;
	TITLE	ivt
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	PAGE
sccsid	db	'@(#)60	1.4  src/bos/usr/lib/asw/mpqp/ivt.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:34'
	db	0

	extrn	int00:FAR,int01:FAR,intNMI:FAR,int03:FAR,int04:FAR
	extrn	int05:FAR,int06:FAR,int07:FAR,int08:FAR,int09:FAR
	extrn	int0A:FAR,int0B:FAR,intINT0:FAR,intINT1:FAR,intINT2:FAR
	extrn	intINT3:FAR,Null_ISR:FAR,int12:FAR,int13:FAR
	extrn	iCIO0A:FAR,iCIO0B:FAR,iCIO1A:FAR,iCIO1B:FAR
	extrn	iSCC0A:FAR,iSCC0A_TXR:FAR,iSCC0A_RT:FAR,iSCC0A_EC:FAR
	extrn	iSCC0B:FAR,iSCC0B_TXR:FAR,iSCC0B_RT:FAR,iSCC0B_EC:FAR
	extrn	iSCC1A:FAR,iSCC1A_TXR:FAR,iSCC1A_RT:FAR,iSCC1A_EC:FAR
	extrn	iSCC1B:FAR,iSCC1B_TXR:FAR,iSCC1B_RT:FAR,iSCC1B_EC:FAR
	extrn	iPDMA_0:FAR,iPDMA_1:FAR,iPDMA_2:FAR,iPDMA_3:FAR
	extrn	iPDMA_4:FAR,iPDMA_5:FAR,iPDMA_6:FAR,iPDMA_7:FAR
	extrn	iPDMA_8:FAR,iPDMA_9:FAR,iPDMA_A:FAR,iPDMA_B:FAR
	extrn	iPDMA_C:FAR,iPDMA_D:FAR,iPDMA_E:FAR,iPDMA_F:FAR
	extrn	False_ISR:FAR
	even
	public	ivt
ivt	EQU	$
	dw	OFFSET int00		; Divide Error Exception interrupt
	dw	OFFSET int01		; Instruction Trace interrupt (NU)
	dw	OFFSET intNMI		; NMI interrupt (Multiplexed)
	dw	OFFSET int03		; Debug Instruction interrupt (NU)
	dw	OFFSET int04		; INT0 Overflow interrupt
	dw	OFFSET int05		; Array Bounds Exception interrupt (NU)
	dw	OFFSET int06		; Illegal Opcode interrupt
					; Warning: Adapter State is Corrupted!
	dw	OFFSET int07		; Illegal Escape Opcode interrupt
					; Warning: Adapter State is Corrupted!
	dw	OFFSET int08		; 186 Timer 0 interrupt
	dw	OFFSET int09		; Reserved - (NU)
	dw	OFFSET int0A		; 186 DMA Channel 0 interrupt
	dw	OFFSET int0B		; 186 DMA Channel 1 interrupt
	dw	OFFSET intINT0		; INT0 Interrupt (Multiplexed)
	dw	OFFSET intINT1		; INT1 Interrupt
	dw	OFFSET intINT2		; INT2 Interrupt
	dw	OFFSET intINT3		; INT3 Interrupt
	dw	OFFSET Null_ISR		; interrupt 10h
	dw	OFFSET Null_ISR		; interrupt 11h
	dw	OFFSET int12		; 186 Timer 1 interrupt
	dw	OFFSET int13		; 186 Timer 2 interrupt
	dw	OFFSET Null_ISR		; interrupt 14h
	dw	OFFSET Null_ISR		; interrupt 15h
	dw	OFFSET Null_ISR		; interrupt 16h
	dw	OFFSET Null_ISR		; interrupt 17h
	dw	OFFSET Null_ISR		; interrupt 18h
	dw	OFFSET Null_ISR		; interrupt 19h
	dw	OFFSET iCIO0A		; CIO Number 0 Channel A
	dw	OFFSET iCIO0B		; CIO Number 0 Channel B
	dw	OFFSET Null_ISR		; CIO Number 0 Counter/Timers
	dw	OFFSET iCIO1A		; CIO Number 1 Channel A
	dw	OFFSET iCIO1B		; CIO Number 1 Channel B
	dw	OFFSET Null_ISR		; CIO Number 1 Counter/Timers
					;
i20h	dw	OFFSET iSCC0A		; SCC Number 0 Channel A Receiver
	dw	OFFSET iSCC0A_TXR	; SCC Number 0 Channel A Transmitter
	dw	OFFSET iSCC0A_RT	; SCC Number 0 Channel A TX/RX Status
	dw	OFFSET iSCC0A_EC	; SCC Number 0 Channel A External C/T
	dw	OFFSET iSCC0B		; SCC Number 0 Channel B Receiver
	dw	OFFSET iSCC0B_TXR	; SCC Number 0 Channel B Transmitter
	dw	OFFSET iSCC0B_RT	; SCC Number 0 Channel B TX/RX Status
	dw	OFFSET iSCC0B_EC	; SCC Number 0 Channel B External C/T
	dw	OFFSET iSCC1A		; SCC Number 1 Channel A Receiver
	dw	OFFSET iSCC1A_TXR	; SCC Number 1 Channel A Transmitter
	dw	OFFSET iSCC1A_RT	; SCC Number 1 Channel A TX/RX Status
	dw	OFFSET iSCC1A_EC	; SCC Number 1 Channel A External C/T
	dw	OFFSET iSCC1B		; SCC Number 1 Channel B Receiver
	dw	OFFSET iSCC1B_TXR	; SCC Number 1 Channel B Transmitter
	dw	OFFSET iSCC1B_RT	; SCC Number 1 Channel B TX/RX Status
	dw	OFFSET iSCC1B_EC	; SCC Number 1 Channel B External C/T
					;
	dw	OFFSET Null_ISR		; 30
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
	dw	OFFSET Null_ISR		;
					;
i40h	dw	OFFSET iPDMA_0		; Port DMA Channel 0
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_1		; Port DMA Channel 1
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_2		; Port DMA Channel 2
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_3		; Port DMA Channel 3
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_4		; Port DMA Channel 4
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_5		; Port DMA Channel 5
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_6		; Port DMA Channel 6
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_7		; Port DMA Channel 7
	dw	OFFSET Null_ISR		;
i50h	dw	OFFSET iPDMA_8		; Port DMA Channel 8
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_9		; Port DMA Channel 9
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_A		; Port DMA Channel A
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_B		; Port DMA Channel B
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_C		; Port DMA Channel C
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_D		; Port DMA Channel D
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_E		; Port DMA Channel E
	dw	OFFSET Null_ISR		;
	dw	OFFSET iPDMA_F		; Port DMA Channel F
	dw	OFFSET Null_ISR		;
					;
i60h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
i70h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
i80h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
i90h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iA0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iB0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iC0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iD0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iE0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
					;
iF0h	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET Null_ISR		; 
	dw	OFFSET False_ISR	; 
_TEXT	ends
	end
