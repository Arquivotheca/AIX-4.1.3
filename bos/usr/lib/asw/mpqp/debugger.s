	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	int01		: Single Step interrupt service routine
;	int03		: Breakpoint interrupt service routine
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
;  FUNCTION: Provide debugging facilities from the host system unit.
;
;**********************************************************************
;
	TITLE	debugger
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
	.SALL				; Suppress macro expansion code
sccsid	db	'@(#)54	1.4  src/bos/usr/lib/asw/mpqp/debugger.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:01'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	include	duscc.inc
	include	cio.inc
	include	portdma.inc
	.LIST
	PAGE

	extrn	_cmdblk:BYTE
;
; This module handles two CPU interrupts which provide debugging hooks.
; They are Int01: Single Step and Int03: Breakpoint.  The breakpointing
; services and single stepping are closely related because setting a
; breakpoint involves instruction replacement.  When the breakpoint opcode
; is executed and the int03 service vector executes, the breakpoint opcode
; is replaced with the original opcode byte and the processor single-steps
; across it.  Once on the other side, the exchange occurs again and the
; single step flag is cleared.
;
; When a breakpoint or manual single step is detected, a data block is placed
; into the debugger command area (command block zero) and the host system is
; interrupted.  The program suspends until a command is received from the host
; system in the same command area.  The first byte is zero when the debugger
; is telling the host something, non-zero when it contains a host command.
; The second byte, when coming from the debugger to the host, is what type of
; data is contained in the block, i.e. GP regs, DMA regs, Timer regs, etc..
;
; The breakpoint table has the following structure.  Byte 0 is the original
; opcode.  Byte two is the port number, 0 - NUM-PORT, or either 0xFE
; (all ports) or 0xFF (not in use).  The last four bytes are the address
; of the breakpoint, which allows the table to be indexed for the proper
; replacement opcode and port information.  The address is stored so that
; an LDS or LES from a DWORD pointer will create a working address.
;
NUM_BRK	EQU	32

bpoint	db	NUM_BRK*8 DUP(0FFh)

OPCODE	EQU	BYTE PTR 0		; Offset, Swapped instruction OpCode

PORTID	EQU	BYTE PTR 1		; Offset, Breakpoint port number
NOTUSED	EQU	0FFh			; Breakpoint entry not in use
ANYPORT	EQU	0FEh			; Breakpoint on any port running

SKIPCNT	EQU	WORD PTR 2		; Offset, skip count before hitting
ADDRESS	EQU	DWORD PTR 4		; Offset, address for LDS/LES, etc.
ADDR_LO	EQU	WORD PTR 4		; Offset, address for low word fetch
ADDR_HI	EQU	WORD PTR 6		; Offset, address for high word fetch

; The field offsets in debugger generated command blocks and possible values
;
OWNER	EQU	BYTE PTR 0
BLKTYP	EQU	BYTE PTR 1
DATAVAL	EQU	2

C_CPUREG EQU	01h			; Possible Block Types (BLKTYP)
C_CPUDMA EQU	02h
C_CPUTMR EQU	03h
C_PRTDMA EQU	04h
C_BMREGS EQU	05h
C_SCCREG EQU	06h
C_CIOREG EQU	07h
C_BRKPT  EQU	0Ah
C_STEP   EQU	0Bh
C_EXIT   EQU	0Ch			; ALWAYS the last entry

; The single step flag (sstep) is used to tell the debugger Single Step
; routine that a breakpoint is being skipped across.  In this case, the
; sstep flag contains the index (0-NumBP) of the breakpoint for which
; instruction replacement must re-occur.  The value 0xFF means not single
; stepping across a breakpoint.

sstep	db	0FFh

; The common debugger entry routine uses the following word to save its
; return address, jumped to later.

deb_ret	dw	0

;
; The command service functions are vectored to by function table lookup.

v_tab	dw	cpuregs
	dw	dmaregs
	dw	tmrregs
	dw	ret_imm
	dw	ret_imm
	dw	ret_imm
	dw	ret_imm
	dw	ret_imm
	dw	breakpt
	dw	ret_imm
	page
;
; * START OF CODE * START OF CODE * START OF CODE * START OF CODE *
;
;
; On entry to and exit from the debugging facilities, the CPU timers and DMA
; channels are stopped.  We don't want any significant events to mess up the
; user operating environment.
;
d0ctl	dw	STOP_D
d1ctl	dw	STOP_D
t0ctl	dw	STOP_T
t1ctl	dw	STOP_T
t2ctl	dw	STOP_T

STOP_D	EQU	DMA_C_CHANGE AND (NOT DMA_C_START)
STOP_T	EQU	TMR_M_INH AND (NOT TMR_M_EN)

CTcontrol proc	near
	mov	dx,TMR2_BASE+TMR_MCTL	; Do the prescaler timer first
	in	ax,dx			;
	xchg	t2ctl,ax		;
	out	dx,ax			;
	mov	dx,TMR1_BASE+TMR_MCTL	; Then timer 1
	in	ax,dx			;
	xchg	t1ctl,ax		;
	out	dx,ax			;
	mov	dx,TMR0_BASE+TMR_MCTL	; The multifunction port event timer
	in	ax,dx			;
	xchg	t0ctl,ax		;
	out	dx,ax			;
	mov	dx,DMA1_BASE+DMA_CTL	; CPU DMA channel 1
	in	ax,dx			;
	xchg	d1ctl,ax		;
	out	dx,ax			;
	mov	dx,DMA0_BASE+DMA_CTL	; CPU DMA channel 0
	in	ax,dx			;
	xchg	d0ctl,ax		;
	out	dx,ax			;
	ret				;
CTcontrol endp

	page
;
; Offsets within the Command Block return for CPU Registers
;
REGAX	EQU	2
REGBX	EQU	REGAX+2
REGCX	EQU	REGBX+2
REGDX	EQU	REGCX+2
REGCS	EQU	REGDX+2
REGDS	EQU	REGCS+2
REGES	EQU	REGDS+2
REGSS	EQU	REGES+2
REGSI	EQU	REGSS+2
REGDI	EQU	REGSI+2
REGBP	EQU	REGDI+2
REGSP	EQU	REGBP+2
REGIP	EQU	REGSP+2
REGFL	EQU	REGIP+2
IP_B0	EQU	32			; Data from CS:IP starts here
FRAME	EQU	48			; Data from [bp-8]->[bp+6] starts here

;
; Offsets on the current stack frame for all of the process registers.
;
SFR_SS	EQU	WORD PTR 0
SFR_ES	EQU	WORD PTR 2
SFR_DS	EQU	WORD PTR 4
SFR_DI	EQU	WORD PTR 6
SFR_SI	EQU	WORD PTR 8
SFR_BP	EQU	WORD PTR 10
SFR_SP	EQU	WORD PTR 12
SFR_BX	EQU	WORD PTR 14
SFR_DX	EQU	WORD PTR 16
SFR_CX	EQU	WORD PTR 18
SFR_AX	EQU	WORD PTR 20
SFR_IP	EQU	WORD PTR 22
SFR_CS	EQU	WORD PTR 24
SFR_FL	EQU	WORD PTR 26
SFR_CSIP EQU	DWORD PTR 22

	page
;
; Create a CPU registers command block for passage to the host system
; On entry, DS:DI must point to the place to put the CPUBLK structure.
; DS:BX must point to the SS register pushed on procedure entry.
; Registers AX, CX, and SI destroyed
;
$mvmemw	macro	dst,src
	mov	ax,src
	mov	dst,ax
	endm

cpuregs	proc	near
	mov	BLKTYP[di],C_CPUREG	;
	$mvmemw	REGAX[di],SFR_AX[bp]	; AX
	$mvmemw	REGBX[di],SFR_BX[bp]	; BX
	$mvmemw	REGCX[di],SFR_CX[bp]	; CX
	$mvmemw	REGDX[di],SFR_DX[bp]	; DX
	$mvmemw	REGCS[di],SFR_CS[bp]	; CS
	$mvmemw	REGDS[di],SFR_DS[bp]	; DS
	$mvmemw	REGES[di],SFR_ES[bp]	; ES
	$mvmemw	REGSS[di],SFR_SS[bp]	; SS
	$mvmemw	REGSI[di],SFR_SI[bp]	; SI
	$mvmemw	REGDI[di],SFR_DI[bp]	; DI
	$mvmemw	REGBP[di],SFR_BP[bp]	; BP
	$mvmemw	REGSP[di],SFR_SP[bp]	; SP
	$mvmemw	REGIP[di],SFR_IP[bp]	; IP
	$mvmemw	REGFL[di],SFR_FL[bp]	; FL

	push	ds			;
	mov	ax,di			;
					; Increment si and di in the movs
	add	di,IP_B0		; Point ES:DI to code storage
	lds	si,SFR_CSIP[bp]		; Get trapped fn CS:IP into DS:SI
	mov	cx,16			; Move 16 bytes as bytes, no swap
	rep movsb			; Prepare instruction stream preview
					;
	mov	ds,SFR_SS[bp]		; Provide function domain view
	mov	si,SFR_SP[bp]		; from SS, all ascending words for 8
	mov	cx,8			; di is magically setup from above
	rep movsw			;
	mov	di,ax			;
	pop	ds			;
	ret				;
cpuregs	endp

	page
;  ******************* CPU DMA REGISTER MANIPULATION *********************

SP_L_0	EQU	2		; Source Pointer, Low, Channel 0
SP_H_0	EQU	4		; Source Pointer, High, Channel 0
DP_L_0	EQU	6		; Destination Pointer, Low, Channel 0
DP_H_0	EQU	8		; Destination Pointer, High, Channel 0
TC_0	EQU	10		; Transfer Count, Channel 0
CTL_0	EQU	12		; Control Word, Channel 0
SP_L_1	EQU	14		; Source Pointer, Low, Channel 1
SP_H_1	EQU	16		; Source Pointer, High, Channel 1
DP_L_1	EQU	18		; Destination Pointer, Low, Channel 1
DP_H_1	EQU	20		; Destination Pointer, High, Channel 1
TC_1	EQU	22		; Transfer Count, Channel 1
CTL_1	EQU	24		; Control Word, Channel 1

;
; Create a DMA registers command block for passage to the host system.
; On entry, DS:DI must point to the place to put the CPUBLK structure.
; ES must equal DS, as the INS instruction uses ES:DI automatically.
;
dmaregs	proc	near
	mov	BLKTYP[di],C_CPUDMA	;
	push	di			;
	add	di,DATAVAL		; Point to data return region
	mov	dx,DMA0_BASE		;
	mov	cx,5			; Get DMA 0 Registers
dma0get:				;
	ins	WORD PTR[di],dx		; DMA 0 Source, Low 16 bits
	add	dx,2			;
	loop	dma0get			;
	mov	ax,d0ctl		;
	stosw				; Real DMA 0 Control Word was stolen
					;
	mov	dx,DMA1_BASE		;
	mov	cx,5			; Get DMA 1 Registers
dma1get:				;
	ins	WORD PTR[di],dx		; DMA 1 Source, Low 16 bits
	add	dx,2			;
	loop	dma1get			;
	mov	ax,d1ctl		;
	stosw				; Real DMA 1 Control Word was stolen
					;
	pop	di			;
	ret				;
dmaregs	endp

	page
;  ****************** CPU TIMER REGISTER MANIPULATION ********************

CC_T0	EQU	2		; Current Count Register, Timer 0
MCA_T0	EQU	4		; Max Count A Register, Timer 0
MCB_T0	EQU	6		; Max Count B Register, Timer 0
CTL_T0	EQU	8		; Mode/Control Word, Timer 0
CC_T1	EQU	10		; Current Count Register, Timer 1
MCA_T1	EQU	12		; Max Count A Register, Timer 1
MCB_T1	EQU	14		; Max Count B Register, Timer 1
CTL_T1	EQU	16		; Mode/Control Word, Timer 1
CC_T2	EQU	18		; Current Count Register, Timer 2
MCA_T2	EQU	20		; Max Count A Register, Timer 2
MCB_T2	EQU	22		; Not Used
CTL_T2	EQU	24		; Mode/Control Word, Timer 2

tmrregs	proc	near
	mov	BLKTYP[di],C_CPUTMR	;
	push	di			;
	add	di,DATAVAL		; Point to data return region
	mov	dx,TMR0_BASE		; Get the registers for timer 0
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	ins	WORD PTR[di],dx		;
	mov	ax,t0ctl		; Real timer control was removed
	stosw				;
	mov	dx,TMR1_BASE		; Get the registers for timer 1
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	ins	WORD PTR[di],dx		;
	add	dx,2			;
	ins	WORD PTR[di],dx		;
	mov	ax,t1ctl		; Real timer control was removed
	stosw				;
	mov	dx,TMR2_BASE		; Get the registers for timer 2
	ins	WORD PTR[di],dx		;
	add	dx,4			;
	xor	ax,ax			;
	stosw				; Timer 2 has no Max Count B
	ins	WORD PTR[di],dx		;
	mov	ax,t2ctl		; Real timer control was removed
	stosw				;

	pop	di			;
	ret				;
tmrregs	endp

	page
;
; The host has requested that we set a breakpoint.  Find an empty one, fill it
; and exchange instructions as required, then indicate acknowledgement in the
; command block.  If the break table is full, we return failure.
;
;  Register Usage:
;  Entry: ES = DS = CS = User Segment      DI = &command block
;  Inside: SI = Breakpoint Pointer         CX, others = scratch
;
;  Command Block Parameters:
;  00 = Cmd Type  01 = Port No  02-03 = Skip Count  04-07 = Breakpoint address
;
BRK_ACK	EQU	80h			; BLKTYP for successful break set
BRK_NAK	EQU	81h			; BLKTYP for unsuccessful break set

BRKPNT	EQU	0CCh			; The elusive INT 3 opcode

breakpt	proc	near
	mov	si,OFFSET bpoint	;
	mov	cx,NUM_BRK		;
try_bp:					;
	cmp	PORTID[si],NOTUSED	; Is this entry free?
	je	setbreak		; yes, set the breakpoint
	loop	try_bp			; no, try the next
	mov	BLKTYP[di],BRK_NAK	; Indicate command failure
	ret				;
setbreak:				;
	push	ds			;
	mov	BLKTYP[di],BRK_ACK	; Indicate command success
	mov	al,BYTE PTR 1[di]	; Port Number indicator
	mov	PORTID[si],al		; Save Port Number in the break table
	mov	ax,WORD PTR 2[di]	; Breakpoint skip count
	mov	SKIPCNT[si],ax		; Store in breakpoint table

	mov	bx,WORD PTR 4[di]	; Breakpoint address, low word
	mov	ax,WORD PTR 6[di]	; Breakpoint address, high word
	ror	ax,4			; Convert to SEG:OFF from long
	mov	ADDR_LO[si],bx		; Save Low Address into break table
	mov	ADDR_HI[si],ax		; Save Low Address into break table
	mov	ds,ax			; Complete break address in DS:BX
	mov	al,BRKPNT		; Get the breakpoint handy
	xchg	[bx],al			; Swap User Opcode and Breakpoint
	mov	OPCODE[si],al		; Save User Opcode into break table
	pop	ds			;
	ret				;
breakpt	endp

	page
;
; If the user either hits a breakpoint or manually single steps, different
; from the automatic single step required to restart the program after hitting
; a breakpoint, execution comes here.  From here, the user may enter commands.
;
cmdproc	proc	near
	mov	di,OFFSET _cmdblk	;
	mov	OWNER[di],C_CPUREG	; Simulate the automatic CPU Reg dump
cmdloop:
	cmp	OWNER[di],0		; Wait for driver to kick us
	je	cmdloop			;
	mov	bl,OWNER[di]		; Get request into faster memory
	xor	bh,bh			;
	cmp	bl,C_EXIT		; Out of range?
	jge	cmdexit			; yes.
;
; Vector to the command processor
;
	dec	bx			; Make this zero relative
	shl	bx,1			; Create vector table offset
	call	WORD PTR v_tab[bx]	; Invoke the service function
	mov	al,TREG_DEBUG		; Every command generates a response
	out	TASKREG,al		; so interrupt the system unit
	mov	di,OFFSET _cmdblk	; Reset cmdblk pointer just in case
	jmp	cmdloop			; and accept another command
cmdexit:				;
	mov	OWNER[di],0		; Clear command block 0 ownership
	jnz	cmdloop			; Command was garbage
	ret				;
cmdproc	endp

	page
;
; Exchange the instruction in the OPCODE field of the breakpoint with the
; value stored in memory at the ADDRESS.
;
instsub	proc	near
	mov	al,sstep		; Get the breakpoint number
	cbw				; Extend into all of AX
	shl	ax,3			; Multiply by the size of a breakpoint
	add	ax,OFFSET bpoint	; Add in the breakpoint table base
	mov	ax,si			;
	lds	bx,ADDRESS[si]		; Pointer to the exch. opcode
	mov	al,OPCODE[si]		;
	xchg	BYTE PTR[bx],al		;
	ret				;
instsub	endp

;
; The debugger is entered through the common setup services below.  Otherwise,
; this code would have to be in three places.
;
entrdeb	proc	near
	pop	deb_ret			; Save routine return address
	pusha				; Save all the 'normal' registers
	push	ds			; These setup the stack frame upon
	push	es			; which cpuregs operates (and relies)
	push	ss			;
	mov	ax,LOADSEG		; The user may have different DS/ES
	mov	ds,ax			; values, but SS is considered sacred.
	mov	es,ax			;
	mov	bp,sp			; Setup easy access to regs
	add	SFR_SP[bp],6		; Remove the interrupt entry words
	cld				; Set direction flag to increment
	call	CTcontrol		; Freeze cpu timers and dma channels
	jmp	deb_ret			; Return to caller via jump
entrdeb	endp

;
; Leave the debugger with the stack frame setup properly for the user process.
; Unless modified by user command, only the STEP bit in the flags register may
; possibly be different.
;
exitdeb	proc	near
	call	CTcontrol		; Restart cpu timers and dma channels
	sub	SFR_SP[bp],6		; Put back the interrupt entry words
	pop	ss			;
	pop	es			;
	pop	ds			;
	$$NSEOI				; Uses DX, AX
	popa				;
	iret				;
exitdeb	endp

	page
;
; The Single Step Trap (interrupt) service routine.
;
	public	int01
int01	proc	far
	call	entrdeb			; Invoke common debugger entry service
	cmp	sstep,0FFh		; Stepping across a breakpoint?
	je	int01a			; No
	and	SFR_FL[bp],TF_OFF	; Clear Trap (Single Step) Flag
	call	instsub			; Exchange instruction and continue
	jmp	exitdeb			; Exit debugger cleanly
int01a:					;
	call	cmdproc			; No, enter command processor
	jmp	exitdeb			; Exit debugger cleanly
int01	endp
	
;
; Breakpoint hit.  Put in the real instruction and single step via Trap flag.
; Also, set the breakpoint table index we hit into the global "sstep" variable.
; Then the single step interrupt will put the breakpoint back in and exit
; single step mode automagically.
;
	public	int03
int03	proc	far
	call	entrdeb			; Invoke common debugger entry service
	or	SFR_FL[bp],TF_ON	; Set Trap (Single Step) Flag

	mov	si,OFFSET bpoint	;
	xor	cx,cx			;

	mov	ax,SFR_CS[bp]		; CS
	mov	di,ax			; Convert any CS:IP into a value
	and	di,0F000h		; X000:XXXX, which is what the break
	rol	ax,4			; table contains
	and	ax,0FFF0h		;
	add	ax,SFR_IP[bp]		; IP
	jnc	int03a			; If no carry, CS is OK
	add	di,1000h		; Otherwise, increment our CS
int03a:					;
	cmp	ADDR_LO[si],ax		; Could this possibly be us?
	jne	next_bp			; Nope
	cmp	ADDR_HI[si],di		; Maybe, check the segment value
	je	getbreak		; yes, set the breakpoint
next_bp:				;
	inc	cx			; Increment the break table index
	cmp	cx,NUM_BRK		; Did we reach the number of breaks?
	jne	int03a			; no, try the next
	jmp	exitdeb			; Exit cleanly, not one of ours
getbreak:
	mov	sstep,cl		; Set magic one-time single step
	call	instsub			; And replace the instruction w/break
	call	cmdproc			;
	jmp	exitdeb			; Exit cleanly
int03	endp

	page
;
; Enter the debugger via command, as opposed to by single-step or breakpoint.
; Notice: the INT0 service routine invokes this procedure after simulating
; invokation by interrupt, i.e. it pushes the FLags and CS before the NEAR
; call.  This way, one can theoretically set the single-step flag and step
; back through the end of the INT0 ISR, if desired.  Usually, a breakpoint
; would be set and hit before tracing was started.  Don't issue 186 EOI.
;
	public	deb_cmd
deb_cmd	proc	near
	call	entrdeb			; Invoke common debugger entry service
	call	cmdproc			; Enter command processor loop
					;
					; Return manually, no EOI, IRET
	call	CTcontrol		; Restart cpu timers and dma channels
	sub	SFR_SP[bp],6		; Put back the interrupt entry words
	pop	ss			;
	pop	es			;
	pop	ds			;
	popa				;
	ret				;
deb_cmd	endp

ret_imm	proc	near
	ret				; Stub function
ret_imm	endp
;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end
