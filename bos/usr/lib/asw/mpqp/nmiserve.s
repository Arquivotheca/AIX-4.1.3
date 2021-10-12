	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	_bm_dump	: Dump the contents of the Bus Master DMA channel
;			:  to global memory (dump area).
;	_pe_dump	: Dump parity error/capture and status registers
;			:  to global memory (dump area).
;	intNMI		: Main dispatcher for NMI interrupts, of which
;			:  at least ten sources exist, all of which are
;			:  errors.
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
;  FUNCTION: Handle reporting of each differentiable source of a non-
;	     maskable interrupt.  Separate services exist for each and
;	     a different RQE is generated for each.  Cleanup is performed
;	     as applicable for the error type.
;
;**********************************************************************
;
	TITLE	nmiserve
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)61	1.6  src/bos/usr/lib/asw/mpqp/nmiserve.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:41'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	include	iface.inc
	.LIST
	PAGE

	extrn	_host_intr:NEAR,_queue_writel:NEAR,_dma_complete:NEAR
	extrn	_arqueue:BYTE,_dma_err:WORD,_dma_tmp:WORD
;
;****************************************************************************
; The following is a global service function which reads the contents of
; the Bus Master DMA channel registers into global data area.
;
	extrn	_bm_area:WORD

	public	_bm_dump
_bm_dump proc	near
	push	es			;  9
	push	di			; 10
	mov	ax,LOADSEG		;  4	Prepare data segment
	mov	es,ax			;  2
	mov	di,OFFSET _bm_area	;  4
					;
	mov	dx,BMDMA_BASE		;  4	Transfer Count
	ins	WORD PTR [di],dx	; 14
	add	dx,2			;  4	Card Address Extension
	ins	WORD PTR [di],dx	; 14
	add	dx,2			;  4	Card Address
	ins	WORD PTR [di],dx	; 14
	add	dx,2			;  4	System Address Extension
	ins	WORD PTR [di],dx	; 14
	add	dx,2			;  4	System Address
	ins	WORD PTR [di],dx	; 14
	mov	ax,_dma_tmp		;  8	Channel Control
	stos	WORD PTR [di]		; 10
	add	dx,4			;  4	List Address Extension
	ins	WORD PTR [di],dx	; 14
	add	dx,2			;  4	List Address Pointer
	ins	WORD PTR [di],dx	; 14
	mov	dx,PCCSTAT		;  4	Fetch PCCSTAT register
	ins	BYTE PTR [di],dx	; 14
					;
	pop	di			; 10
	pop	es			;  8
	ret				; 16	Near return
_bm_dump endp
	page
;****************************************************************************
; The following is a global service to handle Parity Errors.  It reads
; PESTAT and RICPARs 0,1, and 2 into a global data storage area, auto-
; matically resetting parity capture logic when RICPAR2 is read.
;
	extrn	_pe_area:WORD

	public	_pe_dump
_pe_dump proc	near
	push	dx			; 10	Save entry conditions
	push	es			;  9	Routine callable from offlevel
	push	di			; 10
	mov	ax,LOADSEG		;  4	Prepare data segment
	mov	es,ax			;  2
	mov	di,OFFSET _pe_area	;  4
	mov	dx,PESTAT		;  4
	ins	WORD PTR [di],dx	; 14	PESTAT Error Source Indicator
	mov	dx,RICPAR0		;  4
	ins	BYTE PTR [di],dx	; 14	Address Capture, Low 8 bits
	add	dx,RICPAR1-RICPAR0	;  4
	ins	BYTE PTR [di],dx	; 14	Address Capture, Middle 8 bits
	add	dx,RICPAR2-RICPAR1	;  4
	ins	BYTE PTR [di],dx	; 14	Address Capture, Upper 5 bits
	xor	al,al			;  3
	stos	BYTE PTR [di]		; 10	Zero final byte
	pop	di			; 10
	pop	es			;  8
	pop	dx			; 10	Restore entry conditions
	ret				; 16	Near return
_pe_dump endp
;
;****************************************************************************
; Note: fnc_rqe generates an RQE from the value passed on the stack.
;	[sp+4] : sequence
;	[sp+2] : status/port/type

fnc_rqe	proc	near
	pop	cx			; 10	Our return address
	pop	bx			; 10
	pop	ax			; 10
	push	cx			; 10
	push	ax			; 10
	push	bx			; 10
	push	OFFSET _arqueue		; 10
	call	_queue_writel		; 15
	add	sp,6			;  4	This removes our caller's parm
	call	_host_intr		; 15	Interrupt the host (channel)
	ret				; 16
fnc_rqe	endp
	page
;****************************************************************************
; Due to the multiplexed nature of the NMI interrupt on MPQP, numerous
; services may be required on any particular interrupt.
; Certain causes imply that additional registers should be read, many also
; require diagnostic error responses to the Driver because of HW anomalies.
;****************************************************************************
; 02 - NMI (Non-Maskable Interrupt)
;
	even
nmi_vec	dw	v_nmi_crst		; Channel Reset
	dw	v_nmi_wd		; Watchdog timer
	dw	v_nmi_pe		; Parity Error
	dw	v_nmi_pcc		; uChannel Channel Check
	dw	v_nmi_nc		; uChannel Generated NMI
	dw	v_nmi_prot		; W, R/W Protect Violation
	dw	$return			; placeholder (masked, never called)
	dw	$return			; placeholder (masked, never called)
	dw	v_nmi_dac		; Diagnostic Address Compare
	dw	v_nmi_bmcc		; Bus Master Channel Check
	dw	$return			; Missing Card Select Feedback	  030391
					; Don't process since there is a h/w bug
					; Dma's actually complete even when set
	dw	$return			; placeholder (rsrvd, don't check)030391
	dw	v_nmi_arb		; Lost Arbitration Level
	dw	v_nmi_chan		; Lost Channel via ARB/-GNT

	public	intNMI
intNMI	proc	far			; 	ISRs are called FAR
	pusha				; 36
	mov	ax,NMI_M_OFF		;  4	Errors still allowed	083189
	out	NMIMASK,ax		;  9	Silence most interrupts	083189
					;
	mov	dx,BMDMA_BASE+BM_CC	;  4	Bus Master Chan. Ctl	083189
	in	ax,dx			;  8	Bus Master Chan. Ctl	083189
	mov	bx,ax			;  2				090789
	mov	_dma_tmp,bx		;  9	Save, may restart it	090389
	xor	ax,ax			;  3	Prepare halted CCW	083189
	out	dx,ax			;  7	Stop Bus Master Channel	083189
	push	ds			;  9				090789
	push	es			;  9				090789
	mov	bx,LOADSEG		;  4				090789
	mov	ds,bx			;  2				090789
	mov	es,bx			;  2				090789
					;
	in	ax,NMISTAT		; 10
	and	ax,NMI_S_MASK		;  4				083189
	mov	bx,-2			;  4	Start at -2
nmiloop:				;
	cmp	ax,0			;  4	
	jz	nmiexit			;  4/13	No bits left to process
	add	bx,2			;  4
	shr	ax,1			;  2
	jnc	nmiloop			;  4/13	
	push	ax			; 10	Save rotated NMISTAT
	push	bx			; 10	Save fn table offset	090789
	call	WORD PTR nmi_vec[bx]	; 19	Do the function
	pop	bx			; 10	Restore fn table offset	090789
	pop	ax			; 10	Restore rotated NMISTAT
	jmp	nmiloop			; 14
nmiexit:				;
	mov	ax,_dma_tmp		;  8	Retrieve entry BM CCW	090389
	test	ax,BM_CC_START		;  4	Was BM DMA running?	083189
	jz	nmiret			;  4/13	No, end interrupt now	083189
	out	BMDMA_BASE+BM_CC,ax	;  9	Restart channel		083189
nmiret:					;
	$$NSEOI				; 15
	mov	ax,NMI_M_ON		;  4	Reenable NMIs		031591
	out	NMIMASK,ax		;  9				031591
	pop	es			;  8				090789
	pop	ds			;  8				090789
	popa				; 51
	iret				; 28
intNMI	endp
	page
;****************************************************************************
; v_nmi_crst : Host Channel Reset from a warm start
;
; The world has come to an end, invoke the firmware startup routine by
; cheating and vectoring directly to the POR entry point.
;
v_nmi_crst proc	near
	push	0			; 10
	push	NMI_CHRESET		; 10	Indicate Channel Reset
	call	fnc_rqe			; 15	Make RQE
	pop	ax			; 10	local (near) return location
	push	0h			; 10	Reset at FFFF:0
	push	0FFFFh			; 10	POR the chip
	cli				;  2	Disable interrupts
	db	0CBh			; 22	Hardcoded FAR return
v_nmi_crst endp
;
;****************************************************************************
; v_nmi_wd   : Watchdog Timer Error/Expiration
;
; The host system is automatically notified of Watchdog timer errors through
; the TASKREG value of 0xFE and, thus, no particular action is required here.
;
v_nmi_wd proc	near
	push	0			; 10
	push	NMI_WATCHDOG		; 10	Indicate Watchdog Expired
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_wd endp
;
;****************************************************************************
; v_nmi_prot	: SSTIC Detected Protection Violation
;
; When the adapter generates a protection violation, execution transfers here.
; A response queue element is generated to signal the condition to the driver.
; RQE.RTYPE = 0110b  RQE.PORTID = 0000b  RQE.STATUS = 0xA0
;
v_nmi_prot proc	near
	push	0			; 10
	push	NMI_TTPROT		; 10	Translate Table Protection
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_prot endp
;
;****************************************************************************
; v_nmi_nc      : Host system generated NMI detected.
;
v_nmi_nc proc	near
	push	0			; 10
	push	NMI_HOSTNMI		; 10	Host Generated NMI
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_nc endp
;
;****************************************************************************
; v_nmi_pe      : Parity error detected.  This error is caused by many
;		  different, differentiable sources and is served by the
;		  PESTAT register for additional problem determination.
;
;	extrn	_nmi_pe:NEAR		;       PE Service Function in "C"
v_nmi_pe proc	near
	call	_pe_dump		; 15	Call parity error dumper
	push	WORD PTR[_pe_area]	; 10	Push PESTAT register value
	push	NMI_PARITY		; 10	DRAM access parity error
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_pe endp
	page
;****************************************************************************
; A Bus Master related error should only dump the channel registers once and
; should clear the saved channel control word so that NMI ISR exit doesn't
; restart the operation.  Also, the error count should only be incremented
; once.  This routine is a collection on once-only Bus Master error code.
;
	extrn	_Dma_Count:WORD		; DMA Operation Pending Count
bm_err:
	mov	cx,_dma_tmp		;				030391
	test	cx,BM_CC_START		;  4	Was BM DMA running?	083189
	jz	bm_addr			;  4/13 No, exit w/o function	083189
	inc	_dma_err		; 15	Increment error count	083189
	call	_bm_dump		; 15	Dump BMCH Registers	083189
	xor	ax,ax			;  3	Bus Master was involved	090789
	xchg	_dma_tmp,ax		; 17	leave channel stopped	090789
	cmp	_Dma_Count,0		; 10	Application DMA going?	100489
	jz	bm_addr			;  4/13	It probably was		100489
	call	_dma_complete		; 15	Let Bus Master recovery	083189
					;	run.  Calls "C".	083189
;
;****************************************************************************
; When a Bus Master related error occurs, passing the page address in
; system memory being referenced would be useful.  Performed here, address
; aaaa bbbb cccc dddd eeee ffff gggg hhhh (System address bits 31-0) becomes
; bbbb cccc dddd eeee			  (System address bits 27-12)
;
bm_addr:
	mov	ax,[_bm_area+06h]	;  8	Sys. Addr. bits 31-16
	mov	dx,[_bm_area+08h]	; 12	Sys. Addr. bits 15-0
	shl	ax,4			;  9	 bits "bbbb cccc dddd 0000"
	shr	dx,4			;  9	 bits "0000 eeee XXXX XXXX"
	or	al,dh			;  3	 bits "bbbb cccc dddd eeee"
	ret				; 16	Return value in AX
;
;****************************************************************************
; v_nmi_pcc     : Host system Channel Check occurred.  The PCCSTAT register
;		: allows detailed diagnosis of what particular condition,
;		: whether the MPQP was a Master or Slave, was occurring when
;		: the error was detected.
;
v_nmi_pcc proc	near
	call	bm_err			; 15	Output BMCH Register contents
	push	ax			; 10	Store host address for RQE
	push	NMI_UCPARITY		; 10	Indicate Host DRAM Parity Error
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_pcc endp
;
;****************************************************************************
; v_nmi_bmcc    : An External channel check occurred on the channel.
;		: The MPQP adapter was Bus Master and received -CHCHK.
;
v_nmi_bmcc proc	near
	call	bm_err			; 15	Output BMCH Register contents
	push	ax			; 10	Store host address for RQE
	push	NMI_CHCHECK		; 10	Indicate Channel Check
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_bmcc endp
;
;****************************************************************************
; v_nmi_fdbk    : No Card Select Feedback indicated on a channel operation.
;		: The MPQP adapter was Bus Master and received -CHCHK.
;
v_nmi_fdbk proc	near
	call	bm_err			; 15	Output BMCH Register contents
	push	ax			; 10	Store host address for RQE
	push	NMI_SFDBKRTN		; 10	No -SFDBKRTN on Master cycle
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_fdbk endp
;
;****************************************************************************
; v_nmi_arb	: Lost Arbitration Level via Host POS Register write
;
v_nmi_arb  proc	near
	call	bm_err			; 15	Output BMCH Register contents
	push	ax			; 10	Store host address for RQE
	push	NMI_LOSTARB		; 10	Host changed our ARB level
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_arb  endp
;
;****************************************************************************
; v_nmi_chan	: Lost Channel via ARB/-GNT
;		: Note: we were thrown from the bus, presumably because we
;		:  violated the 6.8uS -PREEMPT timeout.
;		: Optionally, the Host cleared our -sleep/+ENABLE bit in POS 2
;
v_nmi_chan proc	near
	call	bm_err			; 15	Output BMCH Register contents
	push	ax			; 10	Store host address for RQE
	push	NMI_LOSTCHAN		; 10	No -SFDBKRTN on Master cycle
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_chan endp
;
;****************************************************************************
; v_nmi_dac     : The diagnostic address compare function matched addresses.
;
v_nmi_dac proc	near
	push	0			; 10
	push	NMI_ADDRCMP		; 10	Diagnostic Address Compare
	call	fnc_rqe			; 15	Make RQE
	ret				; 16
v_nmi_dac endp
;
;****************************************************************************
; $return       : This function is used only to allow simple multibit
;		: nmistat registers to be handled with timy amounts of code.
;
$return	proc	near
	ret
$return	endp

_TEXT	ends
	end
