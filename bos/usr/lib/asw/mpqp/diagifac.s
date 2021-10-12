	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	_ramtst, _chktst, _cputst, _ciotst, _scctst, _ssttst : ROS tests
;	_gmsize, _getiid, _geteid, _setcio, _setscc, _setdma,
;	_settmr, _intwdt, _priswc : ROS services and query commands
;	_getvpd, _CpuRegs, _CpuTmr, _CpuDma, _SegReg, _SccRegs,
;	_CioRegs, _DmaRegs : Extended query commands
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
;  FUNCTION: Entry points into the ROS built-in tests are provided, as
;	     are vital product data and hardware component register dumps.
;
;**********************************************************************
;
	TITLE	diagifac
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)55	1.5  src/bos/usr/lib/asw/mpqp/diagifac.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:05'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	.LIST
	PAGE

	extrn	_edrr_0:BYTE
;
; This module interfaces the MPQP on-board ROS diagnostic functions.
; 
; Basically, all it does is turn stack parameters (like "C" provides)
; into register parameters for the ROS functions.  Actually, it also
; saves the registers and turns the ridiculous and insignificant "Carry"
; flag returned by the ROS into something meaningful to "C" like AX/DX
; register pairs.  With the exception of the memory tests, whose return
; values are filtered at a higher level, any function here must return
; simply a zero/non-zero value for RQE type determination.  Actual
; return data must be placed into EDRR[0].
;
; See the diagnostic function prototype definition file for the order of
; the parameters.
;
;****************************************************************************
;
; RAM TEST: [bp+4] : Logical address to start test, LSW
;           [bp+6] : Logical address to start test, MSW
;	    [bp+8] : Number of paragraphs
;
; Returns:   ax:dx : -1 = OK, else & of error
;
	public	_ramtst
_ramtst	proc	near
	enter	0,0			;
	push	ds			;
	push	es			;
	mov	ax,[bp+4]		; Test address, low
	mov	dx,[bp+6]		; Test address, high
	mov	cl,4			;
ramlp:
	shr	dx,1			;
	rcr	ax,1			;
	loop	ramlp			;

	mov	es,ax			; Start -> ES
	pusha				;
	mov	cx,word ptr [bp+8]	; Count -> CX
;
	xor	ax,ax			;
;	mov	ah,00h			; select RAM test
	int	0FEh			; Invoke diagnostic
;
	mov	ax,di			; save offset where POPA won't get it
	mov	ds,ax			;
	popa				;
	mov	ax,es			; segment -> DX
	mov	dx,ax			;
	mov	ax,ds			; offset  -> AX
	pop	es			;
	pop	ds			;
	jc	ramfail			; error
	mov	ax,0FFFFh		;
	cwd				; No error
ramfail:
	leave				;
	ret				;
_ramtst	endp
;
;****************************************************************************
;
; CHECKSUM TEST: [bp+4] : Logical address to start test, LSW
;                [bp+6] : Logical address to start test, MSW
;		 [bp+8] : Number of bytes (last 2 are checksum)
;
; Returns:      ax : 0 = OK, -1 = error
;
	public	_chktst
_chktst	proc	near
	push	bp
	mov	bp,sp
	push	es
	mov	ax,[bp+4]		; Test address, low
	mov	dx,[bp+6]		; Test address, high
	mov	cl,4			;
chklp:
	shr	dx,1			;
	rcr	ax,1			;
	loop	chklp			;

	mov	es,ax			; Start -> ES
	push	cx
	mov	cx,word ptr [bp+8]	; Count -> CX
;
	xor	ax,ax
	mov	ah,01h			; select checksum test
	int	0FEh			; Invoke diagnostic
;
	mov	ax,0h			; can't XOR			062789
	jc	chkfail			; Failed, return (char *)0	062789
	mov	ax,0FFFFh		; Passed, return (char *)-1	062789
chkfail:
	pop	cx
	pop	es
	pop	bp
	cwd
	ret
_chktst	endp
;
;****************************************************************************
;
; CPU TEST: [bp+4] : Pointer to the command block (Not Used)
;	    [bp+6] : Offset of 128 byte DMA work area
;	    [bp+8] : Segment of 128 byte DMA work area
;
; Returns:      ax : 0 = OK, -1 = error
;
	public	_cputst
_cputst	proc	near
	push	bp
	mov	bp,sp
	push	ds
	push	si
	mov	ax,word ptr [bp+8]
	mov	ds,ax
	mov	si,word ptr [bp+6]
;
	xor	ax,ax
	mov	ah,03h			; Select CPU (processor) test
	int	0FEh			; Invoke diagnostic
;
	mov	ax,0FFFFh
	jc	cpufail
	xor	ax,ax
cpufail:
	pop	si
	pop	ds
	pop	bp
	ret
_cputst	endp
;
;****************************************************************************
;
; CIO TEST: [bp+4] : CIO Number.  Only Zero (0) and One (1) are valid.
;
; Returns:      ax : 0 = OK, -1 = CIO Failed, -2 = Bad CIO Number
;
	public	_ciotst
_ciotst	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	dx,word ptr [bp+4]	; CIO Number
	and	dx,7FFFh		; Be sure its positive
	mov	ax,0FFFEh
	cmp	dx,1
	jg	ciofail			; Bad CIO Number, Direction (JG) OK?
;
	mov	al,dl			; CIO Number, again
	mov	ah,04h			; Test number
	int	0FEh			; Invoke diagnostic
;
	mov	ax,0FFFFh
	jc	ciofail			; Bad CIO Unit
	xor	ax,ax
ciofail:
	pop	dx
	pop	bp
	ret
_ciotst	endp
;
;****************************************************************************
;
; SCC TEST: [bp+4] : SCC Number.  Only Zero (0) and One (1) are valid.
;
; Returns:      ax : 0 = OK, -1 = SCC Failed, -2 = Bad SCC Number
;
	public	_scctst
_scctst	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	dx,word ptr [bp+4]	; SCC Number
	and	dx,7FFFh		; Be sure its positive
	mov	ax,0FFFEh
	cmp	dx,1
	jg	sccfail			; Bad SCC Number, Direction (JG) OK?
;
	mov	al,dl			; SCC Number, again
	mov	ah,04h			; Test number
	int	0FEh			; Invoke diagnostic
;
	mov	ax,0FFFFh
	jc	sccfail			; Bad SCC Unit
	xor	ax,ax
sccfail:
	pop	dx
	pop	bp
	ret
_scctst	endp
;
;****************************************************************************
;
; SSTIC TEST:      : No Input Parameters
;
; Returns:      ax : 0 = OK, -1 = error
;
	public	_ssttst
_ssttst	proc	near
	xor	ax,ax
	mov	ah,06h			; Select SSTIC test
	int	0FEh			; Invoke diagnostic
;
	mov	ax,0FFFFh
	jc	sstfail
	xor	ax,ax
sstfail:
	ret
_ssttst	endp
;
;****************************************************************************
;
; Get Memory Size  : No Input Parameters
;
; Returns:      Memory size returned in the first two words of EDRR0
;
	public	_gmsize
_gmsize	proc	near
	xor	ax,ax
	mov	ah,02h			; Select Get Memory Size
	int	0FEh			; Invoke diagnostic

	push	si			;
	mov	si,OFFSET _edrr_0	; Adapter diagnostics use edrr0
	mov	BYTE PTR 0[si],2	; Set length to two bytes
	mov	WORD PTR 2[si],ax	; Insert return value, reversed
	pop	si			;
	xor	ax,ax			;
	ret
_gmsize	endp
;
;****************************************************************************
;
; Get Interface ID : No Input Parameters
;
; Returns:      ah : hardware interface of SCC Port 0
;		al : hardware interface of SCC Port 1
;
	public	_getiid
_getiid	proc	near
	push	cx
;
	xor	ax,ax
	mov	ah,07h			; Select Get Interface ID
	int	0FEh			; Invoke diagnostic
	mov	ax,cx
;
	push	si			;
	mov	si,OFFSET _edrr_0	; Adapter diagnostics use edrr0
	mov	BYTE PTR 0[si],2	; Set length to two bytes
	mov	WORD PTR 2[si],ax	; Insert return value, reversed
	pop	si			;
	pop	cx
	xor	ax,ax			;
	ret
_getiid	endp
;
;****************************************************************************
;
; Get EIB ID       : No Input Parameters
;
; Returns:      ax : EIB ID (Warning! Spec. is inconsistent on whether the
;			     EIB ID is in the High (CH) or Low (CL) half.
;
	public	_geteid
_geteid	proc	near
	push	cx
;
	xor	ax,ax
	mov	ah,0Eh			; Select Get Extended Interface ID
	int	0FEh			; Invoke diagnostic
	mov	ax,cx
;
	push	si			;
	mov	si,OFFSET _edrr_0	; Adapter diagnostics use edrr0
	mov	BYTE PTR 0[si],2	; Set length to two bytes
	mov	WORD PTR 2[si],ax	; Insert return value, reversed
	pop	si			;
	pop	cx
	xor	ax,ax			;
	ret
_geteid	endp
;
;****************************************************************************
;
; Setup CIO [bp+4] : CIO Number.  Only 0, 1, 2, and 3 are valid
;
; Returns:      ax : 0 = OK, -2 = Bad CIO Number
;
	public	_setcio
_setcio	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	ax,0FFFEh
	mov	dx,word ptr [bp+4]
	and	dx,7FFFh		; Be sure its positive
	cmp	dx,3
	jg	badcio
	mov	ax,dx
;
	mov	ah,08h			; Select Configure CIO
	int	0FEh			; Invoke diagnostic
	xor	ax,ax			;
;
badcio:
	pop	dx
	pop	bp
	ret
_setcio	endp
;
;****************************************************************************
;
; Setup SCC [bp+4] : SCC Port Number.  Only 0, 1, 2, and 3 are valid
;
; Returns:      ax : 0 = OK, -2 = Bad SCC Number
;
	public	_setscc
_setscc	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	ax,0FFFEh
	mov	dx,word ptr [bp+4]
	and	dx,7FFFh		; Be sure its positive
	cmp	dx,3
	jg	badscc
	mov	ax,dx
;
	mov	ah,09h			; Select Configure SCC
	int	0FEh			; Invoke diagnostic
	xor	ax,ax			;
;
badscc:
	pop	dx
	pop	bp
	ret
_setscc	endp
;
;****************************************************************************
;
; Setup DMA [bp+4] : CPU DMA Channel Number.  Only 0, and 1 are valid
;
; Returns:      ax : 0 = OK, -2 = Bad DMA Channel Number
;
	public	_setdma
_setdma	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	ax,0FFFEh
	mov	dx,word ptr [bp+4]
	and	dx,7FFFh		; Be sure its positive
	cmp	dx,1
	jg	baddma
	mov	ax,dx
;
	mov	ah,0Ah			; Select Configure DMA
	int	0FEh			; Invoke diagnostic
	xor	ax,ax			;
;
baddma:
	pop	dx
	pop	bp
	ret
_setdma	endp
;
;****************************************************************************
;
; Set Timer [bp+4] : CIO Timer Number.  Only 0, 1, 2, 3, and 4 are valid
;
; Returns:      ax : 0 = OK, -2 = Bad Timer Number
;
	public	_settmr
_settmr	proc	near
	push	bp
	mov	bp,sp
	push	dx
	mov	ax,0FFFEh
	mov	dx,word ptr [bp+4]
	and	dx,7FFFh		; Be sure its positive
	cmp	dx,4
	jg	badtmr
	mov	ax,dx
;
	mov	ah,0Bh			; Select Configure Hardware Timer
	int	0FEh			; Invoke diagnostic
	xor	ax,ax			;
;
badtmr:
	pop	dx
	pop	bp
	ret
_settmr	endp
;
;****************************************************************************
;
; Init. Watchdog   : No Input Parameters
;
; Returns:      ax : always 0
;
	public	_intwdt
_intwdt	proc	near
	xor	ax,ax
	mov	ah,0Ch			; Select Initialize Watchdog Timer
	int	0FEh			; Invoke diagnostic
	xor	ax,ax
	ret
_intwdt	endp
;
;****************************************************************************
;
; Priority Switch  : No Input Parameters
;
; Returns:      ax : always 0
;
	public	_priswc
_priswc	proc	near
	xor	ax,ax
	mov	ah,0Dh			; Select Priority Switch
	int	0FEh			; Invoke diagnostic
	xor	ax,ax
	ret
_priswc	endp

;
;****************************************************************************
;
; Get ROS Version  : Really returns the complete simulated VPD block contained
;		     in the ROS.
;
; Returns:      ax : always 0
;
ROSWORK	  EQU	400h			;
RETPTR	  EQU	ROSWORK+0		; 
O_ROS_VPD EQU	9h			; Offset of VPD in ROS segment
O_ROS_VER EQU	57h			;
VPD_LGT	  EQU	5Dh			; Length of Vital Product Data
ROS_LGT	  EQU	11			; Length of EC level and Version data

	public	_getvpd
_getvpd	proc	near
	push	bp			;
	mov	bp,sp			;
	push	si			;
	push	di			;
	push	ds			;
	push	es			;
	
	mov	ax,ds			;	Copy to ES:DI in our data seg.
	mov	es,ax			;
	mov	di,OFFSET _edrr_0	;	Adapter diagnostics use edrr0
	xor	bp,bp			;	You don't want to know
	mov	ax,RETPTR+2[bp]		;	Uses SS by default
	mov	ds,ax			;
	mov	si,O_ROS_VPD		;	ROS Version offset
	mov	cx,VPD_LGT		;	Length to copy
	mov	es:0[di],cx		;	Set EDRR length
	add	di,2			;	Move to data field
	rep movsw			;	blotto...
	xor	ax,ax			;	Always succeeds

	pop	es			;
	pop	ds			;
	pop	di			;
	pop	si			;
	pop	bp			;
	ret				;
_getvpd	endp

;
; The following functions answer the Soft Adapter Diagnostics commands, 0xB?.
; In all cases they are entered directly from the INTCOM (INT0) interrupt
; service sub-function.  That makes things a little more difficult,
; especially for CPU register query which has to stack backtrack to find the
; user's registers.  All started 052189b.
;
; Entry:  [bp+4] = &EDRR to use
;
;
	public	_CpuRegs
_CpuRegs proc	near
	push	di			;
	mov	di,[bp+4]		;
	mov	BYTE PTR 0[di],0	; Set length to zero bytes
	pop	di			;
	xor	ax,ax			;
	ret				;
_CpuRegs endp

;
; Entry:  [bp+4] = &EDRR to use
;         [bp+6] = CPU Timer Number
;
	public	_CpuTmr
_CpuTmr proc	near
	enter	0,0			;
	push	di			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	cli				;
	mov	di,[bp+4]		; Get the port EDRR address
	mov	ax,[bp+6]		; Get timer number
	shl	ax,3			; Scale to timer offset, *8
	mov	dx,TMR0_BASE		;
	add	dx,ax			; Completed timer pointer
	mov	BYTE PTR 0[di],8	; Set length to eight bytes
	add	di,2			; Increment to data section
	ins	WORD PTR[di],dx		; Get Current Count
	add	dx,2			;
	ins	WORD PTR[di],dx		; Get Maximum Count A
	add	dx,2			;
	ins	WORD PTR[di],dx		; Get Maximum Count B
	add	dx,2			;
	ins	WORD PTR[di],dx		; Get Mode/Control
	xor	ax,ax			;
	pop	di			;
	leave				;
	ret				;
_CpuTmr endp

;
; Entry:  [bp+4] = &EDRR to use
;         [bp+6] = CPU DMA Channel Number
;
	public	_CpuDma
_CpuDma proc	near
	enter	0,0			;
	push	di			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	cli				;
	mov	di,[bp+4]		; Get the port EDRR address
	mov	ax,[bp+6]		; Get timer number
	shl	ax,4			; Scale to timer offset, *16
	mov	dx,DMA0_BASE		;
	add	dx,ax			; Completed timer pointer
	mov	BYTE PTR 0[di],16	; Set length to sixteen bytes
	add	di,2			; Increment to data section
	mov	cx,6			; Word count to read
dmaloop:				;
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	loop	dmaloop			;
					;
	xor	ax,ax			;
	pop	di			;
	leave				;
	ret				;
_CpuDma endp

;
; Entry:  [bp+4] = &EDRR to use
;	  [bp+6] = Segment Register Number (0-63)
;
	public	_SegReg
_SegReg proc	near
	push	di			;
	mov	di,[bp+4]		;
	mov	BYTE PTR 0[di],2	; Set length to two bytes
	mov	dx,[bp+6]		; Get the segment register index
	add	dx,TRANBASE		; Base of Translation/Protection Regs
	in	ax,dx			; Get the register
	mov	WORD PTR 2[di],ax	; And save in the EDRR of choice
	pop	di			;
	xor	ax,ax			;
	ret				;
_SegReg endp

;
; Entry:  [bp+4] = &EDRR to use
;         [bp+6] = SCC I/O Base Address
;
	public	_SccRegs
_SccRegs proc	near
	enter	0,0			;
	push	di			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	cli				;
	mov	di,[bp+4]		;
	mov	BYTE PTR 0[di],32	;
	add	di,2			; Increment to data section
	mov	dx,[bp+6]		; Get this port's SCC base address
	mov	cx,32			; Number of bytes to read
sccloop:				;
	ins	BYTE PTR[di],dx		;
	add	dx,2			; NOTE: SCCs always use EVEN addresses
	loop	sccloop			;
					;
	xor	ax,ax			;
	pop	di			;
	leave				;
	ret				;
_SccRegs endp

;
; Entry:  [bp+4] = &EDRR to use
;         [bp+6] = CIO I/O Base Address
;         [bp+8] = CIO I/O Port Specific Base Address (what a joke)
;
CIOLGT	EQU	13
evn	db	0,2,4,10h,1Ah,40h,42h,44h,46h,48h,4Ah,4Ch,4Eh
odd	db	0,2,6,12h,1Ch,50h,52h,54h,56h,58h,5Ah,5Ch,5Eh

	public	_CioRegs
_CioRegs proc	near
	enter	0,0			;
	push	di			;
	push	bx			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	cli				;
	mov	di,[bp+4]		;
	mov	BYTE PTR 0[di],CIOLGT	;
	add	di,2			; Increment to data section
	mov	bx,OFFSET evn		;
	and	WORD PTR [bp+8],10h	; Odd channel?
	jz	cioCont			; No, go around
	mov	bx,OFFSET odd		; Yes, use odd channel index table
cioCont:				;
	mov	dx,[bp+6]		; Get this port's SCC base address
	mov	cx,CIOLGT		; Number of bytes to read
	xor	ah,ah			;
ciolp01:				;
	mov	al,[bx]			; get CIO index
	add	dx,ax			;
	ins	BYTE PTR[di],dx		;
	sub	dx,ax			;
	inc	bx			;
	loop	ciolp01			;
					;
	xor	ax,ax			;
	pop	bx			;
	pop	di			;
	leave				;
	ret				;
_CioRegs endp

;
; Entry:  [bp+4] = &EDRR to use
;         [bp+6] = Echo DMA channel index, we lookup the base address
;         [bp+8] = Port Number
;
	extrn	_PDMAbase:WORD
	public	_DmaRegs
_DmaRegs proc	near
	enter	0,0			;
	push	di			;
	push	bx			;
	mov	ax,LOADSEG		;
	mov	es,ax			;
	cli				;
	mov	di,[bp+4]		;
	mov	BYTE PTR 0[di],16	;
	add	di,2			; Increment to data section
	mov	bx,[bp+6]		; Get the Echo channel index
	mov	ax,[bp+8]		; Get Port Number
	shl	ax,1			; Multiply port # by 2
	add	bx,ax			; Add offset to get channel number
	and	bx,0Fh			; Modulo 16, i.e. 0-15
	shl	bx,1			; Word offset in table
	mov	dx,_PDMAbase[bx]	;
	mov	cx,8			; Number of bytes to read
dmalp01:				;
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	loop	dmalp01			;

	xor	ax,ax			;
	pop	bx			;
	pop	di			;
	leave				;
	ret				;
_DmaRegs endp
_TEXT	ends
	end
