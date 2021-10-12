	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	intINT0		: The interrupt service routine for Bus Master
;			:  DMA completion and host system generated	
;			:  interrupts.
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
;  FUNCTION: Processor INT0 interrupt source determination, service
;	     vectoring, and clearing.
;
;**********************************************************************
;
	TITLE	int0serv
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)58	1.4  src/bos/usr/lib/asw/mpqp/int0serv.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:21'
	db	0
	.XLIST
	include	iapx186.inc
	include	mpqp.inc
	include	cio.inc
	include	iface.inc
	.LIST
	PAGE
;
	extrn	_acqueue:BYTE
	extrn	_acmdreg:WORD
	extrn	_tx_int_en:WORD
	extrn	_pcq_tab:WORD
	extrn	_cmdblk:BYTE
	extrn	deb_cmd:NEAR
;
	extrn	_host_command:NEAR	; C Language INTCOM ISR
	extrn	_dma_complete:NEAR	; C Language Bus Master TC ISR
;
;*****************************************************************************
; The INT0 interrupt service routine.  This interrupt is multiplexed on the
; adapter card, so the true source must be determined.
;
; Note: It has been recommended by HW that INTCOM interrupt service occur
; before the Bus Master Terminal Count interrupt.
;
	public	intINT0
intINT0	proc	far
	pusha				; 36
					;
	in	al,INT0STAT		; 10   What caused the INT0?
	test	al,IS0_BMCH1		;  3   Was it the Bus Master?
	jz	try_INTCOM		; 13/4  no, certainly not.
	push	ax			; 10   Save INT0STAT
	mov	al,NOT IS0_BMCH1	;  3   Re-enable Bus Master Int.
	out	INT0STAT,al		;  9   
					;
	mov	dx,CIO_0+P_C_DPP	;  4	Prepare to fire LED	091089
	mov	al,0F0h			;  3	Data Pattern Polarity	091089
	out	dx,al			;  8	for Port C/CIO 0	091089
					;
	call	_dma_complete		; 15   Invoke C Language TC ISR
	pop	ax			; 10   Restore INT0STAT
try_INTCOM:
	test	al,IS0_INTCOM		;  3   Was it an INTCOM interrupt?
	jz	int0eoi			; 13/4  no, unfortunately not.
					;
	in	ax,INTIDREG		; 10   Which master is bothering us?
	not	ax			;  3   Prepare to re-enable them all
	out	INTIDREG,ax		;  9   And execute
	not	ax			;  3   Back to the int0 source mask
	test	ax,8000h		;  4   Was it channel 15 (Host INTCOM)?
	jz	debentry		; 13/4  no, wasting our time.
					;
	cmp	_acmdreg,ACQINT		; 10   Adapter Command Queue non-empty?
	jne	tryTXF			; 13/4 No, try Tx Free List empty
;
; The command is both INTCOM and not for Tx Free List Empty
;
	call	_host_command		; 15   Let C Language do some work
	jmp	int0eoi			; 14   interrupt service complete
;
; Were we interrupted because the TX Free List went empty?
;
tryTXF:
	cmp	_acmdreg,TXFLMT		; 10   Tx Free List Empty
	jnz	int0eoi			; 13/4 System error!  Bad ACMDREG value
;
; Set the global flag so that appropriate addition to TX FL causes interrupt
; Notice that a defined threshold value is placed into the global interrupt
; enable flag for decrementing.  This determines how many TX command blocks
; must be freed before the interrupt goes out.
;
	mov	_tx_int_en,TXFL_THR	; 13   Enable the !empty TX Free int.
	jmp	int0eoi			; 14
debentry:				;      User requesting debugger entry?
	test	ax,1			;  4   Signalled by bit 0
	jz	int0eoi			; 13/4 No, go away.
	pushf				;  9   Simulate Interrupt Service
	push	cs			;  9   as above
	call	deb_cmd			; 15   Yes, call debugger service
	add	sp,4			;  4   Restore over call simulation
	jmp	int0eoi			; 14
int0eoi:				;
	$$NSEOI				; 15
	popa				; 51
	iret				; 28
intINT0	endp

_TEXT	ends
	end
