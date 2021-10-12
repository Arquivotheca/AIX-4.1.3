	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS:
;	_GateUser	: Scheduler vectors to service function or wakeup
;	_ProcSleep	: Put a scheduler service function to sleep,
;			:  preserving the stack frame for awakening
;	_ExitUser	: Return to scheduler after completing service fn
;	_StopTimer	: Stop either general-purpose CPU timer
;	_StartTimer	: Set and start either general purpose CPU timer
;	int08		: CPU Timer 0 interrupt service routine
;	int12		: CPU Timer 1 interrupt service routine
;	_AdapReset	: Reset the adapter.  Not used.
;	_WaitBM		: Wait for Bus Master DMA to complete.  Diagnostic
;			:  use only.
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
;  FUNCTION: Hardware-specific services which are common utilities.
;	     Includes the CPU (General Purpose) timers, Bus Master
;	     DMA channel, and low level process services (scheduler).
;
;**********************************************************************
;
	TITLE	hardutil
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)57	1.4  src/bos/usr/lib/asw/mpqp/hardutil.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:16'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	include	cio.inc
	.LIST
	PAGE
;
;*****************************************************************************
; * START OF CODE * START OF CODE * START OF CODE * START OF CODE *
;*****************************************************************************
;
	extrn	_StackPtr:WORD
;
; GateUser switches context from System (scheduler) to User (Port).
; The state of the executing context is saved, Registers SP, SI, DI and BP.
;
; [BP+6] - Port Number                    [BP+4] - Scheduler service function
; [BP+2] - Scheduler return address       [BP]   - Pushed BP
;
	public	_GateUser
	even
_GateUser proc	near
	push	bp			;
	mov	bp,sp			;
	mov	ax,[bp+6]		; Obtain port number
	shl	ax,3			; Scale to array offset
	add	ax,OFFSET _StackPtr	; Create pointer to Process Save Area
	mov	bx,ax			;
	mov	ax,[bp+4]		; Fetch User Function address
	mov	dx,[bp+6]		; Refetch Port Number
	cli				; Prohibit interruption
	xchg	[bx],sp			; Exchange New/Old new Stack Pointers
	xchg	[bx+2],si		; Save SI
	xchg	[bx+4],di		; Save DI
	xchg	[bx+6],bp		; Save BP
	sti				; Allow interruption
	or	ax,ax			; Wakeup (ax=0) or Vector (ax!=0)
	jz	_WakeFn			;
_GateFn:				; Vector to the user supplied fn
	push	dx			; Store port number
	push	OFFSET _ExitUser	; Push function exit fixup address
	jmp	ax			; No need to RETurn here...
_WakeFn:				; Awaken the suspended process
	mov	bx,sp			;
	mov	ax,ss:[bx+4]		; Retrieve port control block pointer
	ret				; Resume processing within this frame
_GateUser endp

;
; The ProcSleep function is called to stop an offlevel state machine in a
; restartable way, i.e. preserving the Call/Return and variable stack.
; Port # is only parm.  Notice that the CALLER also passes the port control
; block pointer, which GateUser returns when awakening the process, at
; address [bp+6], but that is transparent to both ProcSleep and ExitUser.
;
	public	_ProcSleep
	even
_ProcSleep proc	near
	mov	bx,sp			;
	push	ss:[bx+2]		; Push port number for ExitUser
_ProcSleep endp

;
; Change contexts from user to scheduler, so the scheduler can continue
; regardless of where the "user" stack is.  Never called, just returned to.
;
	public	_ExitUser
_ExitUser proc	near
	pop	bx			; Get back the port number
	shl	bx,3			; and convert into a process state ptr
	add	bx,OFFSET _StackPtr	;
	cli				; Prohibit interruption
	xchg	sp,[bx]			; Exchange New/Old new Stack Pointers
	xchg	si,[bx+2]		; Restore SI
	xchg	di,[bx+4]		; Restore DI
	xchg	bp,[bx+6]		; Restore BP
	sti				; Allow interruption
	pop	bp			;
	ret	 			;
_ExitUser endp

;*****************************************************************************
; Timer service functions for the ports.  These use CPU timer 0 and assume
; CPU timer 2 has been setup as a 10mS prescaler.  It has.  In ahwinit.

	extrn	_ptimer0:WORD,_ptimer1:WORD
	extrn	_t_port0:BYTE,_t_port1:BYTE

STOP_TMR  EQU	 4000h		; 01xx xxxx xxxx xxxx Stop the CPU timer
START_TMR EQU	0E008h		; 111x xxxx xxxx 1xxx Start the CPU timer
				; Bits are ENABLE, -INH, INTERRUPT, PRESCALE

;
; Stop timer kills timer 0 and returns the Current Count.  Very important:
; the timer could reach maximum count and clear Count to zero on the way
; here.  If this case, Count = 0, the Max Count A register is returned
; instead.
;
	public	_StopTimer
_StopTimer proc	near
	push	bp			;				060589
	mov	bp,sp			;				060589
	mov	dx,TMR0_BASE		;				060589
	mov	ax,[bp+4]		; Timer number flag		060589
	or	ax,ax			; Timer 0?			060589
	jz	cStop			; Yes, continue			060589
	mov	dx,TMR1_BASE		; Timer base address		060589
cStop:					;
	pushf				;
	cli				;
	add	dx,TMR_MCTL		; Timer Mode/Control		060589
	mov	ax,STOP_TMR		;
	out	dx,ax			; Stop the timer
	sub	dx,TMR_MCTL-TMR_CNT	; Timer Count			060589
	in	ax,dx			; Retrieve Current Count
	or	ax,ax			; If non-zero, it was counting	052089
	jnz	StopExit		;  and AX is return value	052089
	add	dx,TMR_MAX_A-TMR_CNT	; Else, expired, we must return	060589
	in	ax,dx			;  Max Count A for #of ticks	052089
StopExit:				;				052089
	popf				;
	pop	bp			;				060589
	ret				;
_StopTimer endp

; Entry :  [bp+4]  : Timer number, either 0 or 1 (non-zero)
; 	:  [bp+6]  : Timer count, in 100ms (0.1S) increments)
; Returns:    AX  : void, is actually the Mode/Control used to start the timer

	public	_StartTimer
_StartTimer proc near
	push	bp			;
	mov	bp,sp			;
	mov	dx,TMR0_BASE		;				060589
	mov	ax,[bp+4]		;				060589
	or	ax,ax			; Timer zero or one?		060589
	jz	cStart			; Jumps for timer zero		060589
	mov	dx,TMR1_BASE		;				060589
cStart:					;				060589
	mov	ax,[bp+6]		;
	or	ax,ax			; Be sure value is !0		052089
	jz	StartExit		;				052089
	add	dx,TMR_MAX_A		; Timer Max Count		051989
	out	dx,ax			; Set new Count			051989
	xor	ax,ax			;				052189
	sub	dx,TMR_MAX_A-TMR_CNT	; Clear the current count	052189
	out	dx,ax			;				052189
	mov	ax,START_TMR		; Enable counting		052089
	add	dx,TMR_MCTL-TMR_CNT	; Timer 0 Mode/Control		051989
	out	dx,ax			; Start the timer		051989
StartExit:				;				052089
	pop	bp			;
	ret				;
_StartTimer endp

; When the 80186 Timer 0 expires, the CPU vectors here (to interrupt 8)
; automatically.  The MAX_A count for timer 0 is passed as a parameter to
; the C language interrupt hanlder.

	extrn	_timer_exp:NEAR		; 				052089
	public	int08
int08	proc	far
	pusha				;				052089
	mov	dx,TMR0_BASE+TMR_MCTL	; Timer 0 Mode/Control		052089
	mov	ax,STOP_TMR		; Note this clears other bits	052089
	out	dx,ax			; Stop the timer (unnecessary)	052089
	mov	dx,TMR0_BASE+TMR_MAX_A	; Timer 0 Count from Max A	052089
	in	ax,dx			;				052089
	push	OFFSET _t_port0		; Running timer indicator	060589
	push	OFFSET _ptimer0		; Timer value table		060589
	push	TIMER_P			; Eblock type mask		060589
	push	ax			; Prep. parameter to timer_exp	052089
	xor	ax,ax			; Clear Max A in case 		052189
	out	dx,ax			;  StopTimer gets called	052189
	call	_timer_exp		; Timer 0 Service Routine, "C"	052089
	add	sp,8			; Restore stack frame		052089
	$$NSEOI				;				052489
	popa				;				052089
	iret				; Issue EOI, Return from Intr.	052489
int08	endp

; When the 80186 Timer 1 expires, the CPU vectors here (to interrupt 0x12)
; automatically.  The MAX_A count for timer 1 is passed as a parameter to
; the C language interrupt hanlder.

	public	int12
int12	proc	far
	pusha				;				060589
	mov	dx,TMR1_BASE+TMR_MCTL	; Timer 1 Mode/Control		060589
	mov	ax,STOP_TMR		; Note this clears other bits	060589
	out	dx,ax			; Stop the timer (unnecessary)	060589
	mov	dx,TMR1_BASE+TMR_MAX_A	; Timer 1 Count from Max A	060589
	in	ax,dx			;				060589
	push	OFFSET _t_port1		; Running timer indicator	060589
	push	OFFSET _ptimer1		; Timer value table		060589
	push	FAILSAFE		; Eblock type mask		060589
	push	ax			; Prep. parameter to timer_exp	060589
	xor	ax,ax			; Clear Max A in case 		060589
	out	dx,ax			;  StopTimer gets called	060589
	call	_timer_exp		; Timer 1 Service Routine, "C"	060589
	add	sp,8			; Restore stack frame		060589
	$$NSEOI				;				060589
	popa				;				060589
	iret				; Issue EOI, Return from Intr.	060589
int12	endp

; Reset the adapter.  This no longer actually pulls the plug on the card
; but, instead, relies on the host computer resetting us via COMREG.
; This is executed by the host system Reset Adapter command after the
; command block has been verified.
;
	public	_AdapReset
_AdapReset proc	near
	in	al,INITREG1		;
	and	al,NOT IR1_PROMRDY	;	Clear Prom Ready
	out	INITREG1,al		;
					;
	mov	dx,CIO_0+P_C_DPP	;  4	Prepare to kill LED	091089
	mov	al,0F5h			;  3	Data Pattern Polarity	091089
	out	dx,al			;  8	for Port C/CIO 0	091089
					;
	ret				;
	hlt				;	On real cards, issue HLT
_AdapReset endp

	page
;*****************************************************************************
; The WaitBM function waits until the Bus Master DMA channel completes the
; current transfer.  The fact that the service essentially polls the INT0
; status register is necessary because this call is ONLY ALLOWED when CPU
; Interrupts Are DISABLED.  This function cannot be called by a real applic-
; ation, or the INT0 ISR and, eventually, bm_int_tc services would run and
; WaitBM would run forever.

	public	_WaitBM
_WaitBM	proc	near
	push	cx			; 10
restart:
	mov	cx,800h			;  4
	in	ax,BMDMA_BASE+BM_CC	; 10
	test	ax,BM_CC_START		;  4
	jnz	wabt			; 13/4
	pop	cx			; 10
	in	al,INT0STAT		; 10
	mov	al,NOT IS0_BMCH1	;  3
	out	INT0STAT,al		;  9
					;
	mov	dx,CIO_0+P_C_DPP	;  4	Prepare to fire LED	091089
	mov	al,0F0h			;  3	Data Pattern Polarity	091089
	out	dx,al			;  8	for Port C/CIO 0	091089
					;
	xor	ax,ax			;  3
	ret				; 16
wabt:
;	shr	ax,15			; 20
	loop	wabt			; 17/5
	jmp	restart			; 14
_WaitBM	endp

;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end
