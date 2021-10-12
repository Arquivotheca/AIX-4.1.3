	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS:
;	int00, int04, int05, int06, int07, int09, int0A, int0B,
;	intINT1, intINT2, intINT3, int13 : Unexpected interrupt services
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
;  FUNCTION: This file catches unexpected (and unused) interrupts and
;	     generates an error RQE for the host system driver.  These
;	     interrupts are not fatal to operation of the adapter SW.
;
;**********************************************************************
;
	TITLE	intserve
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)59	1.4  src/bos/usr/lib/asw/mpqp/intserve.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:26'
	db	0
	.XLIST
	include	iapx186.inc
	include	iface.inc
	include	mpqp.inc
	.LIST
	PAGE

	extrn	_host_intr:NEAR,_queue_writel:NEAR
	extrn	_arqueue:BYTE
;****************************************************************************
;
; This module services certain of the magical/tragical CPU base interrupts.
;
;****************************************************************************
; Generate an RQE to indicate that an unused/disallowed interrupt occurred.
; On entry to $illegal, the stack contains the IP, CS, and flags in ascending
; order.  Put the interrupt type, from AH, into the RQE status field.

	public	$illegal
$illegal proc	near
	push	bp			;
	mov	bp,sp			;
	push	dx			;
	mov	dx,4[bp]		; Return instruction address
	push	dx			; Save as "sequence number"
	xor	al,al			;
	or	ax,INT_ILLEGAL		; RQE Type/Port/Status base value
	push	ax			;
	push	OFFSET _arqueue		;
	call	_queue_writel		; Add the RQE to the Adapter R.Q.
	add	sp,6			;
	pop	dx			;
	pop	bp			;
	pop	ax			;
; * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY *
	hlt				; Here stack frame is as on entry
; * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY * TEMPORARY *
	$$IRET				; Clear 186 interrupt controller
$illegal endp

;****************************************************************************
; 00 - Divide Error Exception
;
	public	int00
int00	proc	far
	push	ax
	mov	ah,0h				; Interrupt type
	jmp	$illegal
int00	endp
;****************************************************************************
; 01 - Instruction Trace - see debugger.s
;****************************************************************************
; 02 - NMI, see file NMISERVE.S for ISR
;****************************************************************************
; 03 - Debug Instruction - see debugger.s
;****************************************************************************
; 04 - INT0 Overflow
;
	public	int04
int04	proc	far
	push	ax
	mov	ah,4h				; Interrupt type
	jmp	$illegal
int04	endp
;****************************************************************************
; 05 - Array Bounds Exception
;
	public	int05
int05	proc	far
	push	ax
	mov	ah,5h				; Interrupt type
	jmp	$illegal
int05	endp
;****************************************************************************
; 06 - Illegal Opcode
;
;	Considered Fatal by RCM.
;
	public	int06
int06	proc	far
	push	ax
	mov	ah,6h				; Interrupt type
	jmp	$illegal
int06	endp
;****************************************************************************
; 07 - Illegal ESC Opcode
;
;	Considered Fatal by RCM.
;
	public	int07
int07	proc	far
	push	ax
	mov	ah,7h				; Interrupt type
	jmp	$illegal
int07	endp
;****************************************************************************
; 08 - Timer 0 Interrupt : See hardutil.s
;
;****************************************************************************
; 09 - Reserved
;
	public	int09
int09	proc	far
	push	ax
	mov	ah,9h				; Interrupt type
	jmp	$illegal
int09	endp
;****************************************************************************
; 0A - DMA 0 Interrupt
;
	public	int0A
int0A	proc	far
	push	ax
	mov	ah,0Ah				; Interrupt type
	jmp	$illegal
int0A	endp
;****************************************************************************
; 0B - DMA 1 Interrupt
;
	public	int0B
int0B	proc	far
	push	ax
	mov	ah,0Bh				; Interrupt type
	jmp	$illegal
int0B	endp
;****************************************************************************
; 0C - INT0 Interrupt, See file INT0SERV.S for ISR
;
;****************************************************************************
; 0D - INT1 Interrupt
;
	public	intINT1
intINT1	proc	far
	push	ax
	mov	ah,0Dh				; Interrupt type
	jmp	$illegal
intINT1	endp
;****************************************************************************
; 0E - INT2 Interrupt
;
	public	intINT2
intINT2	proc	far
	push	ax
	mov	ah,0Eh				; Interrupt type
	jmp	$illegal
intINT2	endp
;****************************************************************************
; 0F - INT3 Interrupt
;
	public	intINT3
intINT3	proc	far
	push	ax
	mov	ah,0Fh				; Interrupt type
	jmp	$illegal
intINT3	endp
;****************************************************************************
; 10,11 - Not Used
;
;****************************************************************************
; 12 - Timer 1 Interrupt : See hardutil.s
;
;****************************************************************************
; 13 - Timer 2
;
	public	int13
int13	proc	far
	push	ax
	mov	ah,13h				; Interrupt type
	jmp	$illegal
int13	endp
;****************************************************************************
; 14,15,16,17,18,19 : Not Used
;****************************************************************************
; 1A,1B,1C,1D,1E,1F : CIO0, CIO1
;****************************************************************************
; 20 - 27 : SCC 0 Interrupt, see scccio.s
;****************************************************************************
; 28 - 2F : SCC 1 Interrupt, see scccio.s
;****************************************************************************
; 30 - 3F : Not Used
;****************************************************************************
; 40 - 5E : Echo (Port) DMA Interrupts, all even
; 41 - 5F : Not Used, all odd
;****************************************************************************
; 60 - 7B : Not used
;****************************************************************************
; 7C : X.21 PAL interrupt - See scccio.s with other CIO type interrupts
;****************************************************************************

	 public	Null_ISR
Null_ISR proc	far
	 $$IRET			; Issue 186 EOI and IRET
Null_ISR endp

;****************************************************************************
;  False ISR - This routine has been added by the advice of the Boca
;engineers.  They have seen situations in which the status registers from
;the DUSCC get read by the echo so fast that when the 186 tries to process
;the interrupt, it gets a vector value of 0FFh.  The suggestion was to clear 
;the status registers on the DUSCCs and CIOs.
;****************************************************************************
	 public	False_ISR
False_ISR proc	far
	pusha
	push	di
	mov	dx,SCCRELINT0	;
	out	dx,al		;
	mov	dx,SCCRELINT1	;
	out	dx,al		;
	mov	dx,SCCRELINT4	;
	out	dx,al		;
	mov	dx,SCCRELINT5	;
	out	dx,al		;
	pop	di
	popa	
	$$IRET			; Issue 186 EOI and IRET
False_ISR endp
_TEXT	ends
	end
