	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	aswinit		: Adapter software entry point, initialize code
;	_intreset	: Clear any INT0 manifested interrupt
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
;  FUNCTION: This module performs adapter software initialization and
;	     performs all handshaking with the system unit to signal
;	     the load status and location of common structures.  Calls
;	     ahwinit to setup hardware to our default values.
;
;**********************************************************************
;
	TITLE	aswinit
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)53	1.10  src/bos/usr/lib/asw/mpqp/aswinit.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:58:57'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	.LIST
	PAGE

	extrn	_queue_init:NEAR,ahwinit:NEAR,Null_ISR:FAR,intX21:NEAR
	extrn	_init_sched:NEAR,_init_pcb:NEAR,False_ISR:FAR
	extrn	_tx_free:BYTE,_rx_free:BYTE,_acqueue:BYTE,_arqueue:BYTE
	extrn	_p0cmdq:BYTE,_p1cmdq:BYTE,_p2cmdq:BYTE,_p3cmdq:BYTE
	extrn	_p0rspq:BYTE,_p1rspq:BYTE,_p2rspq:BYTE,_p3rspq:BYTE
	extrn	_tx_dma_q:BYTE
	extrn	_work_q:WORD
	extrn	ivt:WORD
	extrn	_disables:BYTE
;
	org	0
vectors	EQU	$
i_DZE_s	EQU	WORD PTR 0
i_DZE_o	EQU	WORD PTR 2
i_i0_s	EQU	WORD PTR 30h
i_x21_s	EQU	WORD PTR (vectors+(4*07Ch))
i_FE_o	EQU	WORD PTR (vectors+(4*0FEh))
i_FE_s	EQU	WORD PTR (vectors+(4*0FEh)+2)
	org	100H
;
MPQP_ID	EQU	08F70h
SSTIC41	EQU	80h			; Contender 3: No Data Parity
SSTIC42	EQU	81h			; Contender 4: No Data Parity
SSTIC43	EQU	82h			; Contender 5: Supports Data Parity

;
; * START OF CODE * START OF CODE * START OF CODE * START OF CODE *
;
	public	aswinit
aswinit	proc	near
	jmp	a_start			;
a_start:				;
	add	sp,6			; Repair caller's stack
	call	_intreset		; Reset INT0 and 80C186
;
; MPQP provides NMI service for parity-type errors, thus Parity Latch Enable
; is set.  The diagnostic address compare feature is disabled.
;
	mov	ax,NMI_M_ON		; Notice:  -SFDBKRTN Interrupt Disabled
	out	NMIMASK,ax		;
	in	ax,NMISTAT		;
	cli				; NO stack access until SS setup
	cld				; Set the direction flag to increment
test1:					;
	in	ax,CARDID		; Card ID
	cmp	ax,MPQP_ID		; Is this the correct type adapter?
	je	test2			;
	iret				; Wrong adapter, return to caller
test2:					;
	in	al,GAID			; Gate Array ID
	cmp	al,SSTIC41		; Bus Master Contender 3?
	je	ivt_setup		; Yes, but no Data Parity
	cmp	al,SSTIC42		; Bus Master Contender 4?	090389
	je	ivt_setup		; Yes, but no Data Parity	090389
	cmp	al,SSTIC43		; Bus Master Contender 5?	050690
	je	ivt_setup		; Yes, supports Data Parity	050690
	iret				; Wrong adapter, return to caller
ivt_setup:				;
;	mov	BYTE PTR cs:[0],0F4h	; Set a HALT opcode at CS:0
;	mov	BYTE PTR cs:[0],0C3h	; Place a RET opcode at CS:0
	call	LowSet			; Set low mem in CS to value	091589
	mov	ax,LOADSEG		; Prepare Data Segment
	mov	ds,ax			; Set Data Segment value	051689
	xor	ax,ax			; Prepare Stack Segment 	083189
	mov	ss,ax			; Set Stack Segment value	051689
	mov	es,ax			; Set Extra Segment value	051689
	mov	sp,0FFFEh		;  the Stack Pointer
	mov	bp,0FFFEh		;  the Frame Pointer		083189
					;
	mov	si,OFFSET ivt+2		; Copy source is ds:si, skip DZE
	mov	di,4			; Copy destination is es:di, 0:4
	mov	ax,LOADSEG		; All vectors in our CS		110789
	mov	cx,05Fh			; Vector count of transfer	032891
ivtmov:					;				110789
	movsw				; move the offset of vector
	stosw				; store the segment of vector	110789
	loop	ivtmov			;
	sti				;

	mov	si,es:i_FE_o		;				110789
	mov	dx,es:i_FE_s		;				110789

	mov	cx,09Fh			; Interrupts to null out
	mov	ax,OFFSET Null_ISR	; Not Used interrupt service
	mov	bx,LOADSEG		; Code Segment value
empty:					;
	stosw				; Save interrupt function offset
	xchg	ax,bx			; Exchange offset, segment
	stosw				; Save interrupt function segment
	xchg	ax,bx			; Exchange segment, offset
	loop	empty			; Repeat indefinitely

	mov	ax,OFFSET False_ISR	; False interrupt service routine 032891
	stosw				; Save interrupt function offset  032891
	xchg	ax,bx			; Exchange offset, segment	  032891
	stosw				; Save interrupt function segment 032891

	mov	es:i_FE_o,si		;				110789
	mov	es:i_FE_s,dx		;				110789
	mov	es:i_x21_s,OFFSET intX21	; Set the X.21 PAL ISR	110789

	mov	ax,ds			; While the queues are changed, both
	mov	es,ax			; [si] and [di] should use our DS.

	sub	sp,4				;
	mov	bp,sp				;
	mov	WORD PTR [bp+2],QLGT		;
	mov	WORD PTR [bp],OFFSET _acqueue	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p0cmdq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p1cmdq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p2cmdq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p3cmdq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _tx_dma_q	;
	call	_queue_init			;

	mov	WORD PTR [bp+2],Q2LGT		;
	mov	WORD PTR [bp],OFFSET _arqueue	;
	call	_queue_init			;
	mov	WORD PTR [bp+2],QLGT		;
	mov	WORD PTR [bp],OFFSET _p0rspq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p1rspq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p2rspq	;
	call	_queue_init			;
	mov	WORD PTR [bp],OFFSET _p3rspq	;
	call	_queue_init			;
	add	sp,4				;

	mov	di,OFFSET _rx_free	; Do the Rx Free list first
	call	q_set
	mov	di,OFFSET _tx_free	; Do the Tx Free list next
	call	q_set

	push	0			; Port 0			051789
	mov	bp,sp			;				051789
	call	_init_pcb		;				051789
	mov	WORD PTR [bp],1		; Port 1			051789
	call	_init_pcb		;				051789
	mov	WORD PTR [bp],2		; Port 2			051789
	call	_init_pcb		;				051789
	mov	WORD PTR [bp],3		; Port 3			051789
	call	_init_pcb		;				051789
	mov	_disables,0		; Clear disable count
	add	sp,2			;				051789

	xor	ax,ax			; Set ES back to 0
	mov	es,ax			;
;
; Do the hardware initialization
;
	call	ahwinit			; Setup hardware interrupts, etc.
;
; Now place the address of the common tables and queues into the DZE at IVT 0
; When the driver has read the value, it interrupts us again on int0.
;
	mov	dx,es:i_i0_s		; Save real INT0 Service
	mov	ax,OFFSET restart	; Get temporary INT0 routing ptr
	mov	es:i_i0_s,ax		; and save in the IVT

	mov	ax,OFFSET _tx_free	; Tell the driver where the shared
	mov	bx,LOADSEG SHR 12	; data structures reside.  The low
	mov	es:i_DZE_o,bx		; half (0) must be written last
	mov	es:i_DZE_s,ax		; because that's what the driver
					; waits for.
	jmp	wabt

restart:
	add	sp,6			; Discard interrupt return
	mov	es:i_i0_s,dx		; Restore normal INT0 Service
	call	_intreset		; Reset INT0 and 80C186
	sti				; be sure interrupts are enabled

	xor	di,di			; Set IVT index pointer
	mov	si,OFFSET ivt		; Normal DZE Service vector
	movsw				; Note that both es and di are 0
	mov	ax,LOADSEG		;
	stosw				; which points conveniently to DZE
;
; Call the scheduler, which never returns
;
	mov	ax,ds			; code is done with segment 0	051689
	mov	es,ax			; references.			051689
	call	_init_sched		;

stop_cpu:
	sti
	xor	bx,bx			;
	mov	ds,bx			; 				083189
	mov	8[bx],OFFSET wabt	;				083189
	mov	10[bx],cs		;				083189
wabt:
	nop				; wait, possibly forever
	jmp	wabt			; Ouch!

q_set:
	mov	BYTE PTR LGT[di],QLGT+1	;				051091
	mov	BYTE PTR DNE[di],QLGT	;				051091
	mov	BYTE PTR OUTP[di],0	;
	mov	BYTE PTR INP[di],QLGT	;				051091
	add	di,QUEUE		; Point to the queue data
	mov	cx,QLGT+1		;				051091
	xor	ax,ax
	dec	al			;				051091
q_loop:					; Fill the queue
	stosb
	inc	al
	loop	q_loop
	ret

aswinit	endp

	public	_intreset
_intreset proc	near
	in	al,INT0STAT		; INT0 Interrupt source
	mov	ah,al			; Save source for later
	not	al			;
	out	INT0STAT,al		; Write one's complement to clear
	test	ah,IS0_INTCOM		; An INTCOM interrupt?
	jz	nseoi			; No, go directly to CPU EOI
	in	ax,INTIDREG		; Host identifier for INT0
	not	ax			; One's complement to re-enable int-
	out	INTIDREG,ax		; errupts from interrupting masters
nseoi:
	mov	dx,EOIREG		; Location in 80186 ICU
	mov	ax,NONSPEOI		; Non-specific EOI
	out	dx,ax			; and issue
	ret
_intreset endp

LowSet	proc	near
	push	es			;
	push	di			;
	push	cx			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	xor	di,di			;
	mov	cx,81h			;
	mov	ax,0F4F4h		;
	rep stosw			;
	pop	cx			;
	pop	di			;
	pop	es			;
	ret				;
LowSet	endp
;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end	aswinit			; Declare program entry point
