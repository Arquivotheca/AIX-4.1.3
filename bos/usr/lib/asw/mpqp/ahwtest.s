;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
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
;
	page	60,132
	title	ahwtest
;
	.XLIST
	include	segments.s
sccsid	db	'@(#)51	1.3  src/bos/usr/lib/asw/mpqp/ahwtest.s, ucodmpqp, bos411, 9428A410j 1/30/91 17:05:50'
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	.LIST

	extrn	_intreset:NEAR
;
; * START OF CODE * START OF CODE * START OF CODE * START OF CODE *
;
;
; Test the On-board CPU DMA Channels, albeit quickly
;

d0t1	dw	88h,0,			; Source is CARDID, 0x88 I/O
		0,3,			; Destination address 30000 (CS:0)
		128,			; Count = 128 words
		0A307h			; Control
d0t2	dw	18h,0,			; Source is GAID, 0x18 I/O
		0FFh,3,			; Destination address
		256,			; Count = 256 bytes
		0C306h			; Control
d1t1	dw	0,3,			;
		800,0,			; Destination is
		128,			; Count = 126 words
		01707h			; Control
d1t2	dw	0FFH,3,			;
		800,0,			; Destination is ENREG, 0x800h I/O
		256,			; Count = 256 bytes
		01B06h			; Control

dmaset:
	mov	cx,6			; Six control registers to write
t_init:
	outs	dx,WORD PTR[si]		;
	add	dx,2			;
	loop	t_init			;
	ret				;
cpu186t:
	mov	bx,dx			;
	mov	si,OFFSET d0t1		;
	call	dmaset			;
	hlt
	xor	si,si			; Prepare to compare!
	mov	dx,87F0h		; Compare value
	mov	cx,128			; loop count
clp01:
	lodsw				;
	cmp	ax,dx			;
	jne	t_died			; error, pattern mismatch
	loop	clp01			; No, continue pattern match
;
; Fall through, everthing is fine.  CX emptied.
; Continue tests.
;
t_died:
	ret
	
	public	_cpudma
_cpudma	proc	near
	push	dx
	push	cx
	push	bx
	mov	dx,DMA0BASE		; CPU DMA Channel 0 base address
	call	cpu186t			; Call common test logic
	or	ax,ax			; Did an error (ax!=0) occur?
	jnz	cdmaerr			;  yes.
	mov	dx,DMA1BASE		; CPU DMA Channel 1 base address
	call	cpu186t			; Call common test logic
cdmaerr:				; Note: returns AX=0 for OK, else...
	pop	bx
	pop	cx
	pop	dx
	ret
_cpudma	endp

;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end
