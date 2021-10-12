	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	iPDMA_0, iPDMA_1, iPDMA_2, iPDMA_3, iPDMA_4, iPDMA_5, iPDMA_6,
;	iPDMA_7, iPDMA_8, iPDMA_9, iPDMA_A, iPDMA_B, iPDMA_C, iPDMA_D,
;	iPDMA_E, iPDMA_F : Port DMA channel interrupt service routines
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
;  FUNCTION: All sixteen port DMA channels are serviced in this module.
;	     Each is a "FAR" procedure and resets the DMA channel, as
;	     required, for subsequent use.  Interrupts are re-enabled,
;	     if applicable.
;
;**********************************************************************
;
	TITLE	dmaserve
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
	.SALL				; Suppress macro expansion code
sccsid	db	'@(#)56	1.9  src/bos/usr/lib/asw/mpqp/dmaserve.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:11'
	db	0
	.XLIST
	include	iapx186.inc
	include	mpqp.inc
	include	define.inc
	include	portdma.inc
	include	duscc.inc
	.LIST
	PAGE

	public	$BEG_PDMA,$END_PDMA
$BEG_PDMA	EQU THIS BYTE
;
;****************************************************************************
; 40 - 5E (Even) : Port DMA Interrupts
;
	public	iPDMA_0,iPDMA_1,iPDMA_2,iPDMA_3
	public	iPDMA_4,iPDMA_5,iPDMA_6,iPDMA_7
	public	iPDMA_8,iPDMA_9,iPDMA_A,iPDMA_B
	public	iPDMA_C,iPDMA_D,iPDMA_E,iPDMA_F
;
iRXDMA	macro	NUM,PRIDMA,AUXDMA	; start macro directive
	pusha				; 36
	mov	dx,PRIDMA		;  4
	mov	bx,AUXDMA		;  4
	mov	cx,NUM			;  4
	jmp	P_RXDMA			; 15
	endm				; end macro directive
;       Straight Line Execution Time      63    0.000 005 040 S
;
iTXDMA	macro	NUM,PRIDMA		; start macro directive
	pusha				; 36
	mov	dx,PRIDMA		;  4
	mov	cx,NUM			;  4
	jmp	P_TXDMA			; 15
	endm				; end macro directive
;       Straight Line Execution Time      59    0.000 000 000 S
;
iPDMA_0	proc	far
	iRXDMA	00h,PD0BASE,PD8BASE
iPDMA_0	endp
;
iPDMA_1	proc	far
	iTXDMA	01h,PD1BASE
iPDMA_1	endp
;
iPDMA_2	proc	far
	iRXDMA	02h,PD2BASE,PDABASE
iPDMA_2	endp
;
iPDMA_3	proc	far
	iTXDMA	03h,PD3BASE
iPDMA_3	endp
;
iPDMA_4	proc	far
	iRXDMA	04h,PD4BASE,PDCBASE
iPDMA_4	endp
;
iPDMA_5	proc	far
	iTXDMA	05h,PD5BASE
iPDMA_5	endp
;
iPDMA_6	proc	far
	iRXDMA	06h,PD6BASE,PDEBASE
iPDMA_6	endp
;
iPDMA_7	proc	far
	iTXDMA	07h,PD7BASE
iPDMA_7	endp
;
iPDMA_8	proc	far
	iRXDMA	08h,PD8BASE,PD0BASE
iPDMA_8	endp
;
iPDMA_9	proc	far
	iTXDMA	09h,PD9BASE
iPDMA_9	endp
;
iPDMA_A	proc	far
	iRXDMA	0Ah,PDABASE,PD2BASE
iPDMA_A	endp
;
iPDMA_B	proc	far
	iTXDMA	0Bh,PDBBASE
iPDMA_B	endp
;
iPDMA_C	proc	far
	iRXDMA	0Ch,PDCBASE,PD4BASE
iPDMA_C	endp
;
iPDMA_D	proc	far
	iTXDMA	0Dh,PDDBASE
iPDMA_D	endp
;
iPDMA_E	proc	far
	iRXDMA	0Eh,PDEBASE,PD6BASE
iPDMA_E	endp
;
iPDMA_F	proc	far
	iTXDMA	0Fh,PDFBASE
iPDMA_F	endp
	page
;****************************************************************************
; Receive Interrupt Handler
;   The Rx DMA may terminate for two reasons, either Terminal Count (in
; which case the buffer would overflow) or End-Of-Message indication from
; the Signetics DUSCC.  The Terminal Count is an error, and the DUSCC
; receiver has to be reset and set in hunt mode again.  The End-Of-Message
; interrupt is the only acceptable way for a channel to complete, and the
; transfer count register from the completing channel is placed into the
; receive buffer header for the offlevel to handle.  Character matching
; isn't set to stop the channel.
;
; Entry:  bx : I/O Base address of port's alternate Rx channel, i.e.
;		port 0 and 8 are paired.  When 0 completes, 8 is the
;		alternate.
;	  cx : Completing DMA channel number, i.e. 0x0 - 0xF
;         dx : I/O Base address of completing channel
;
	extrn	_rxwork:WORD,_SCCbase:WORD
	extrn	_rxstat:WORD
	extrn	_rx_count:WORD
	page
	public	P_RXDMA
P_RXDMA	proc	near
	mov	di,cx			;  2	cx = channel #
	and	di,6h			;  4	di = port # * 2
					;
	xchg	dx,bx			;  4	Exchange channel addrs
	in	ax,dx			;  8	Next channel CCW
	or	ax,ax			;  3	Setup already?  If no,
	jz	rx_more			;  4/13	don't start Echo or SCC	080689
	or	ax,PDMA_CCW_EN		;  4	Yes, set the Enable bit
	out	dx,ax			;  7	and start the channel
	xor	bx,bx			;  3	Signal, enable SCC Rx	080689
rx_more:				;
	mov	dx,PD0STAT		;  4	Base, Echo status
	add	dx,cx			;  3	Add in port number
	in	al,dx			;  8	Clear Echo Status
	mov	ah,al			;  2	Move echo status high
					;
	mov	_rxstat[di],ax		;  9	status for offlevel
	test	ah,PDSR_EOM		;  3	Did Echo hit EOM?	080689
	jnz	rx_exit			; 13/4	Jumps if it did
	mov	dx,_SCCbase[di]		; 12	Port DUSCC base address

;For now, don't read the tx/rx status register since offlevel doesn't
;use it and it may be robbing transmit from getting transmit status if
;a receive interrupt is received while processing a transmit.

					;
	add	dx,CH_CMD		;  4	channel command reg.
	mov	al,RESET_RX		;  3	Force SCC to Hunt
	out	dx,al			;  7	Echo chip terminal counted
	or	bx,bx			;  3	Enable SCC Rx?		080689
	jnz	rx_exit			;  4/13	No if bx=0		080689
	mov	al,ENABLE_RX		;  3	Enable SCC Rx
	out	dx,al			;  7	Echo Channel is set up
rx_exit:
	mov	ax,1			;  3
	add	_rx_count[di],1 
	shr	di,1			;  2	convert to port number
	mov	cx,di			;  2	Get port number in cx/cl
	shl	ax,cl			;  5-8	Make scheduler OR mask
	or	WORD PTR _rxwork,ax	; 10	Indicate offlevel work
					;
	$$NSEOI				; 15				080289
	popa				; 51				080289
	iret				; 28				080289
;					 ----- ---------------
;       		   Front End	  63    0.000 005 040 S
;       	  ISR Execution Time     242    0.000 019 360 S
;       Straight Line Execution Time       0    0.000 024 400 S
;
P_RXDMA	endp
	page
;****************************************************************************
; Transmit Interrupt Handler
;
; Entry:  cx : Completing DMA channel number, i.e. 0x0 - 0xF
;         dx : I/O Base address of completing channel
;
	extrn	_txwork:WORD
	extrn	_txstat:WORD,_txlgt:WORD,_txinc:WORD
	extrn	_txccw:WORD
	extrn	_txftype:BYTE

	page
	public	P_TXDMA
P_TXDMA	proc	near
	mov	dx,PD0STAT		;  4	Echo Status Reg. Base	062589
	add	dx,cx			;  4	Add channel number	062589
	in	al,dx			;  8	Acknowledge Interrupt	062589
	test	al,PDSR_TC		;	Check for terminal count052990
	jz	txOk1			;	If so & ASYNC sched txwk052990
					;	Calculate sched Mask:	052990
	and	cx,6			;  	Get port # from chan #	052990
	shr	cx,1			;	Divide by 2		052990
	mov	ax,1			;  4				052990
	shl	ax,cl			;  5-8  Make scheduler OR mask	052990
	mov	di,cx			;				062390
	test	_txftype[di],FT_A	;   	If this is ASYNC,       062390
	jz	txOk1			;				062390
	or	WORD PTR _txwork,ax	; 10	Indicate offlevel work	052990
txOk1:					;
					;
	$$NSEOI				; 15				080289
	popa				; 51				080289
	iret				; 28				080289
P_TXDMA	endp

$END_PDMA	EQU THIS BYTE

_TEXT	ends
	end
