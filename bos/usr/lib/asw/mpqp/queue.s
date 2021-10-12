	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;=========================================================================
;
;        DANGER!!!	DANGER!!!	DANGER!!!	DANGER!!!
;
;  This file contains extremely brain-damaged code!  Use of these routines
;  will achieve unpredictable and often fatal consequences such as random
;  corruption of adapter data structures, poor performance, and complete
;  lockup (requiring a reboot of the host machine).  The routines in this file
;  are no longer used by the adapter software (queue.s has been removed from
;  the Makefile); all queues are now managed by the routines in the file 
;  "queues.c".  This file is preserved for historical reasons and as a toxic 
;  waste disposal area of sorts.  If anything, it should serve as a prime 
;  example of how NOT to design and code adapter software.   The following 
;  erroneous assembler directive is deliberately inserted to prevent use 
;  of this file:
;
;=========================================================================
	endp
;
;  FUNCTIONS: 
;	_q_init		: Initialize a queue structure
;	_QueueEmpty	: Return boolean status, is a queue empty?
;	_QueueBackup	: Return an item to the head (versus tail)
;			:  of a queue.
;	_q_add_bi	: Add an entry to a byte queue
;	_q_add_wi	: Add an entry to a word queue
;	_q_rem_bi	: Remove an entry from a byte queue
;	_q_rem_wi	: Remove an entry from a word queue
;	_q_rem_rqe	: Remove an entry from an RQE queue
;	_q_rem_b	: Remove an entry from a byte queue, non-interlocked
;	_cmd_rem	: Remove a command from a command (byte) queue
;	_trcqadd	: Place an entry on both function trace queues
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
;  FUNCTION: The adapter software contains a plethora of queues to hold
;	     both used and free resources.  Since at least four differ-
;	     ent sized items are manipulated, numerous functions exist.
;
;
;**********************************************************************
;
	TITLE	queue
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)63	1.3  src/bos/usr/lib/asw/mpqp/queue.s, ucodmpqp, bos411, 9428A410j 1/30/91 17:19:44'
	.XLIST
	include	define.inc
	.LIST
	PAGE

;=========================================================================
;
; q_init	Initialize a queue.  It functions for both byte and word qs
;
; Entry:	[bp+4]	= &queue
;		[bp+6]	= length
; Returns:	AX	= &queue
; Alters:	Nothing outside of queue
; Stack:	2 words
;
	public	_q_init
_q_init	proc	near
	push	bp
	mov	bp,sp
	push	di
	mov	di,WORD PTR [bp+4]
;
;=========================================================================
;  Pitfall #1	Lock the queue?  Why?  Since all queues are initialized
;		at init time, there is no need for a lock -- who else is
;		going to access it?  More on locks in Pitfall #5 below.

	mov	BYTE PTR INP[di],0FFh		; Lock the queue
	mov	al,BYTE PTR [bp+6]		;
	xor	ah,ah				;

;=========================================================================
;  Pitfall #2:	Note that the length and end pointers are set to the
;		same value!  Why have two fields then?  The end pointer
;		(which could have been referenced as "ENDP" rather than
;		"DNE" -- indicative of the overall backward thinking here)
;		could be set to length - 1 so that the queue routines
;		don't have to recalculate this value (or ptr+1) every time.

	mov	BYTE PTR LGT[di],al		; Set LGT (length)
	mov	BYTE PTR DNE[di],al		; Set END pointer
	mov	BYTE PTR OUTP[di],0		; Set OUTPointer
	mov	BYTE PTR INP[di],0		; Set INPointer, unlock queue
;
	mov	ax,di				; Set return value
	pop	di
	pop	bp
	ret
_q_init	endp
;
; QueueEmpty	Is the passed queue empty?
;
; Entry:	[bp+4]	= &queue
; Returns:	AX	= !0 = Queue is Empty
;		AX	=  0 = Queue is Non-Empty
; Alters:	Nothing
; Stack:	2 words
;
	public	_QueueEmpty
_QueueEmpty proc near
	push	bp			;
	mov	bp,sp			;
	push	bx			;
	mov	bx,WORD PTR [bp+4]	;
	xor	ax,ax			;

;=========================================================================
;  Pitfall #3:  Note the use of "lock" -- this instruction is designed
;		for locking of the bus between hardware processors; NOT
;		as a semaphore between multiple processes on the same
;		processor!  For that matter, why even lock it if you are 
;		just looking at it?

	lock mov bx,[bx+2]		; Get head, tail
	cmp	bl,bh			; Are they the same?
	jnz	qExit			;  No.
	dec	ax			;
qExit:					;
	pop	bx			;
	pop	bp			;
	ret				;
_QueueEmpty endp

;				
; QueueBackup	Is the passed queue empty?
;
; Entry:	[bp+4]	= &queue
; Returns:	AX	= 0 in all cases
; Alters:	The queue OUT pointer is backed up by 1, wrapped as necessary
; Stack:	2 words
;
	public	_QueueBackup
_QueueBackup proc near
	push	bp			;
	mov	bp,sp			;
	push	si			;
	mov	si,WORD PTR [bp+4]	; Get queue pointer, only parameter
	mov	ax,[si+2]		; Get head, tail
	or	al,al			; OUTP == 0?
	jnz	decTail			;  not a snowball's chance
	mov	al,[si+1]		; Get END (i.e. length)
decTail:
	dec	al			; Dec. in all cases, END is one high
	mov	BYTE PTR [si+2],al	; Write back the OUTPointer
	xor	ax,ax			; Indicate success
	pop	si			;
	pop	bp			;
	ret				;
_QueueBackup endp
;
;*****************************************************************************
; Common logic for the Queue Add Byte/Word Interlocked routines
;
qai_entry:				; Common entry logic, Interlocked Add
	pop	ax			; near return location (IP)
	push	bp
	mov	bp,sp
	push	di
	push	si
	mov	di,WORD PTR [bp+4]
	mov	si,di
	add	si,QUEUE		; Add offset to actual queue data
	push	dx
	mov	dx,WORD PTR [bp+6]
	push	cx
	xor	cx,cx			; Preliminary return value
	pushf				;				070589
	cli				;				070589
	push	ax			; Restore our return location
;
	mov	ax,0FFFFh

;=========================================================================
;  Pitfall #4:	Look at this retry code again -- an INFINITE SPIN LOOP!
;		Should something go wrong with the queue (as is often the 
;		case when the code in this file is used), the adapter will 
;		hang here forever, ignoring any commands -- including adapter  
;		reset commands!  Only a complete hardware reset of the 
;		adapter (and, therefore, the host) can rescue it from this 
;		disaster.
;
qai_rtry:				; Queue Add Retry
	xchg	al,INP[di]		; Lock the list
	cmp	al,0FFh			; Already locked?
	jz	qai_rtry		; Yes, retry
	mov	ah,OUTP[di]		; OUT pointer for later
	ret
;
;*****************************************************************************
; q_add_bi	Queue Add, Byte - Interlocked
;		Items are inserted at IN.  Note that the OUTPointer is
;		ignored and no previously empty response is possible.
;
; Entry:	[bp+4]	= &queue
;		[bp+6]	= Item to add
; Returns:	void
; Alters:	Nothing outside of queue
; Stack:	6 words
;
	public	_q_add_bi
_q_add_bi proc	near

;=========================================================================
;  Pitfall #5:	Here is the first example of an "interlocked" queue
;		routine; it first calls "qai_retry" to wait for access to
;		the queue -- who are we competing against here?  A simple
;		disabling of interrupts before accessing the queue and
;		reenabling afterwards will guarantee that no other entities
;		will call "q_xxx_xi" during this access.  If the queue is
;		shared between the device driver and adapter, then there is
;		no need to implement a semaphore as long as the rules for
;		accessing the queue are followed: the reader of the
;		queue accesses ONLY the output of the queue while the
;		writer accesses ONLY its input (even if both access it
;		simultaneously).  These are very simple and straightforward 
;		software engineering concepts (successfully implemented 
;		in "queues.c").  In summary, the locks are COMPLETELY 
;		UNNECESSARY and are in fact, unreliable and destructive 
;		as implemented here.

	call	qai_entry
	inc	al			; Point to next sequential element
	cmp	al,DNE[di]		; Wrap the list?
	jl	q_abi_1			;  no...
	xor	al,al			;  yes..
q_abi_1:
	xor	ah,ah			;				062689
	add	si,ax			; [si] = IN
	mov	[si],dl			; Store new item
	mov	INP[di],al		; And store updated IN pointer
	popf				;				070589
	pop	cx
	pop	dx
	pop	si
	pop	di
	pop	bp
	ret
;
_q_add_bi endp
;
;*****************************************************************************
; q_add_wi	Queue Add, Word - Interlocked
;		Items are inserted at IN, removed from OUT
;
; Entry:	[bp+4]	= &queue
;		[bp+6]	= Item to add
; Returns:	AX	=  0 if OK, queue was not empty
;		AX	= -2 if OK, queue was empty when called
;		AX	= -1 if queue full, item not added
; Alters:	Nothing outside of queue
; Stack:	6 words
;
	public	_q_add_wi
_q_add_wi proc	near
	call	qai_entry		;
	inc	al			; Point to next sequential element
	cmp	al,DNE[di]		; Wrap the list? (in(al)=END)
	jl	q_awi_2			;  no...
	xor	al,al			;  yes..
q_awi_2:
	cmp	al,ah			; Is the list full? (new in=out)
	je	q_awi_3			;  Yes, return -1 immediately
	mov	ch,al			; Save new IN pointer value
	xor	ah,ah			;
	shl	ax,1			; Convert to word offset
	add	si,ax			; Pointer to new tail item
	mov	[si],dx			; Store new item
	mov	INP[di],ch		; And store updated IN pointer
	mov	al,cl			; Move return value
q_awi_4:
	popf				;				070589
	pop	cx
	pop	dx
	pop	si
	pop	di
	pop	bp
	cbw				;
	ret
q_awi_3:
	dec	al			;				062689
	mov	INP[di],al		; Restore prior IN pointer (unlock)
	mov	al,0FFh			; Indicate error, list full
	jmp	q_awi_4			; and exit procedure
;
_q_add_wi endp
;
;*****************************************************************************
; The Remove Queue Byte/Word Interlocked Common Code
;
qri_entry:
	pop	ax
	push	bp
	mov	bp,sp
	push	di
	push	si
	mov	di,WORD PTR [bp+4]
	mov	si,di
	add	si,QUEUE		; Add offset to actual queue data
	push	dx
	pushf				;				070589
	cli				;				070589
	push	ax			; Restore our return location
;
	xor	ax,ax			;
	dec	ax			;


;=========================================================================
;  Pitfall #6	Another infinite loop hazard (see Pitfall #4)

qri_rtry:				; Queue Remove Retry
     lock xchg	al,INP[di]		; Lock the list, get in pointer
	cmp	al,0FFh			; Already locked?
	jz	qri_rtry		; Yes, retry
	mov	ah,OUTP[di]		; Get the out pointer
;
	xchg	al,ah			; Rearrange (al=out, ah=in)
	cmp	al,ah			; List Empty?
	jnz	q_ri_1			;  no, continue processing
	mov	INP[di],ah		; Unlock the list
	mov	al,0FFh			; Indicate the queue is empty to caller
	ret
q_ri_1:
	inc	al			; Create updated out pointer
	cmp	al,DNE[di]		; Did the list wrap?
	jnz	q_ri_2			;  no...
	xor	al,al			;  yes..
q_ri_2:
	mov	dx,ax			; Save the in and out pointers
	xor	ah,ah			; Current (out) extended in AX
	ret
;
;*****************************************************************************
; q_rem_bi	Queue Remove, Byte - Interlocked
;		Items are removed from OUT
;
; Entry:	[bp+4]	= &queue
; Returns:	AL	= Item if OK, -1 if queue empty
; Alters:	Nothing outside of queue
; Stack:	5 words
;
	public	_q_rem_bi
_q_rem_bi proc	near
	call	qri_entry
	cmp	al,0FFh			; List empty?
	je	q_rbi_4			;  yes, return the (uchar)-1
	add	si,ax			; Point to OUTP pointed to item
	mov	OUTP[di],dl		; Save the updated OUT pointer
	mov	al,[si]			; Fetch head pointed to item
	mov	INP[di],dh		; Unlock the list (restore INP)
q_rbi_4:
	popf				;				070589
	pop	dx
	pop	si
	pop	di
	pop	bp
	xor	ah,ah			; Clear high half of return value
	ret
_q_rem_bi endp
;
;*****************************************************************************
; q_rem_wi	Queue Remove, Word - Interlocked
;		Items are removed from OUT
;
; Entry:	[bp+4]	= &queue
; Returns:	AX	= Item if OK, -1 if queue empty
; Alters:	Nothing outside of queue
; Stack:	5 words
;
	public	_q_rem_wi
_q_rem_wi proc	near
	call	qri_entry
	cmp	al,0FFh			; List Empty?
	jz	q_rwi_3			;  yes, return a -1
	shl	ax,1			; Convert to word offset
	add	si,ax			; Point to head item
	mov	OUTP[di],dl		; Save the updated OUT pointer
	mov	ax,[si]			; Fetch head pointed to item
	mov	INP[di],dh		; Unlock the list (restore INP)
q_rwi_4:
	popf				;				070589
	pop	dx
	pop	si
	pop	di
	pop	bp
	ret
q_rwi_3:
	cbw				; Extend AL to AX (ushort)-1
	jmp	q_rwi_4
;
_q_rem_wi endp

;*****************************************************************************
; q_rem_rqe	Queue Remove, Response Queue Element (doubleword)
;		Items are removed from OUT
;
; Entry:	[bp+4]	= &queue
; Returns:	AX	= Item LSW if OK, -1 if queue empty
; 		DX	= Item MSW if OK, -1 if queue empty
; Alters:	Nothing outside of queue
; Stack:	x words
;
	public	_q_rem_rqe
_q_rem_rqe proc	near
	call	qri_entry
	cmp	al,0FFh			; List Empty?
	jz	q_rri_4			;  yes, return a -1
	shl	ax,1			;
	shl	ax,1			;
	add	si,ax			; Point to return data
	mov	OUTP[di],dl		; Write updated OUTP
	mov	ax,[si]			; Low (LS) word
	mov	si,2[si]		; High (MS) word
	mov	INP[di],dh		; Unlock list
q_rri_3:
	popf				;				070589
	pop	dx			; note! value lost, we return long
	mov	dx,si
	pop	si
	pop	di
	pop	bp
	ret
q_rri_4:
	cbw				; Signal -1 error exit value
	mov	si,ax
	jmp	q_rri_3
_q_rem_rqe endp

;
;*****************************************************************************
; q_rem_b	Queue Remove, Byte
;		Items are removed from OUT
;
; Entry:	[bp+4]	= &queue
; Returns:	AL	= Item if OK, -1 if queue empty
; Alters:	Nothing outside of queue
; Stack:	2 words
;
;=========================================================================
;  Pitfall #7	Non-interlocked version (that's good), but what if this
;		routine is called simultaneously for the same queue by 
;		two different offlevels?  Interrupts should be disabled
;		around access to the out pointer (and its slot) or data 
;		corruption could result.  
;		    Note the efficiency pretensions: each line indicates 
;		the number of cycles used by that instruction (just in case 
;		we don't know how to read a data book and/or operate a 
;		calculator).  Comments describing what the code is doing 
;		(or attempting to do) would have been infinitely more 
;		useful.  Saving a microsecond here and there won't help
;		a sloppy and poorly thought out design, and pretending to 
;		save cycles impresses no one.
;
	public	_q_rem_b
_q_rem_b proc	near
	push	bp			; 10
	pushf				;  9
	cli				;  2
	mov	bp,sp			;  2
	mov	ax,[bp+4]		;  8
	mov	si,ax			;  2
	mov	ax,OUTP[si]		;  8
	cmp	al,ah			;  3
	je	mt_1			;  4/13
	xor	ah,ah			;  3
	add	si,ax			;  3
	xchg	al,ah			;  4
	inc	ah			;  3
	cmp	ah,DNE[si]		; 10
	jnz	ok_1			;  4/13
	xor	ah,ah			;  3
ok_1:					;
	mov	al,[si]			;  8
	mov	OUTP[si],ah		;  9
lv_1:					;
	popf				;  8
	xor	ah,ah			;  3
	pop	bp			; 10
	ret				; 16
mt_1:					;
	xor	ax,ax			;  3
	dec	ax			;  3
	jmp	lv_1			; 14
;					 ----- ---------------
;			   Front End     152    0.000 012 160 S
;       Straight Line Execution Time       0    0.000 024 400 S
;
;
_q_rem_b endp

;
;*****************************************************************************
; cmd_rem	Remove a command from a byte queue and replace the command
;		block index returned with NO_BUFNO (0xF0).
;
;=============================================================================
;  Pitfall #8	Why?  What is so special about the value "F0h" ?  As long
;		as the queues are correctly accessed (see above), then we 
;		are concerned only with queue slots that contain valid
;		information (command block pointers) -- we shouldn't care
;		about what's in the unused slots . . . unless the queues are
;		somehow being corrupted?  In other words, if your design 
;		is broken, FIX IT; don't attempt to bandage it with an even 
;		worse kludge.
;		
;
; Entry:	[bp+4]	= &queue
; Returns:	AL	= Item if OK, -1 if queue empty
; Alters:	Nothing outside of queue
; Stack:	2 words
;
	public	_cmd_rem
_cmd_rem proc	near
	push	bp			; 10
	mov	bp,sp			;  2
	push	si			; 10
	mov	ax,[bp+4]		;  8	Get queue pointer
	mov	si,ax			;  2
	mov	al,OUTP[si]		;  8	Get list "out" pointer
	xor	ah,ah			;  3	Clear MSB
	inc	al			;  3	Pre-increment out
	cmp	al,DNE[si]		; 10	Wrap list?
	jnz	rk_1			;  4/13 No, jump around
	xor	al,al			;  3	Yes, wrap "out"
rk_1:					;
	add	si,ax			;  3	Pointer, item to take
	mov	ah,al			;  2	Save incremented out
	add	si,QUEUE		;  4
	mov	al,[si]			;  8	Acquire pointed to item
	cmp	al,0F0h			;  3	No command indicated?
	je	re_1			; 13/4	True, return -1
	mov	BYTE PTR [si],0F0h	; 12	Clear removed item
	mov	si,[bp+4]		; 12	Get back queue pointer
	mov	OUTP[si],ah		;  9	Store new out pointer
rx_1:					;
	xor	ah,ah			;  3	Return an unsigned char
	pop	si			; 10
	pop	bp			; 10
	ret				; 16
re_1:					;	Pointed to item wasn't a cmd
	xor	ax,ax			;  3
	dec	ax			;  3
	jmp	rx_1			;	Rejoin standard code path
_cmd_rem endp

;=============================================================================
;  Pitfall #9	Yet another queue -- for traces?  Why not simply wrap trace
;		data in a single block of memory designated for that purpose
;		(see the function "char_trace" in the file "tracefn.c")?
;		Note the indecision here -- if tq_fnc is "allocated" (whatever
;		that means), then put the next trace element somewhere else;
;		oh let's put it at . . . how about FA00h?  Would you trust the
;		trace generated by this code?
;
;*****************************************************************************
; trcqadd - Add an element to the adapter software flow trace queue
;
; Entry:	[bp+4]	= Value to add, two bytes (two characters)
; Returns:	AX	= Nothing significant (void)
; Alters:	Nothing outside of queue
; Stack:	2 words
;
	extrn	_tq_fnc:WORD,_tq_ret:WORD
	public	_trcqadd
_trcqadd proc	near
	ret
	mov	bx,[bp+2]		; Caller's return address
	push	bp			;
	mov	bp,sp			;
	push	di			;
	mov	di,_tq_fnc		; Get address of trace queue
	or	di,di			; Allocated?
	jnz	add_ele			; Yes, add the element now
					;
	mov	di,0FA00h		; Temporary fixed value
	mov	_tq_ret,di		;
	mov	al,0F0h			; Temporary fixed size
	mov	BYTE PTR LGT[di],al	; Set LGT (length)
	mov	BYTE PTR DNE[di],al	; Set END pointer
	mov	BYTE PTR OUTP[di],0	; Set OUTPointer
	mov	BYTE PTR INP[di],0	; Set INPointer, unlock queue
	mov	di,0F800h		; Temporary fixed value
	mov	_tq_fnc,di		;
	mov	al,0F0h			; Temporary fixed size
	mov	BYTE PTR LGT[di],al	; Set LGT (length)
	mov	BYTE PTR DNE[di],al	; Set END pointer
	mov	BYTE PTR OUTP[di],0	; Set OUTPointer
	mov	BYTE PTR INP[di],0	; Set INPointer, unlock queue
add_ele:				;
	xor	ah,ah			;
	pushf				;				091089
	cli				; Prohibit interruption
	mov	al,INP[di]		;
	inc	al			; Preincrement In pointer
	cmp	al,DNE[di]		; Wrap the list?
	jb	ta_cont			;  no...
	xor	al,al			;  yes..
ta_cont:				;
	mov	INP[di],al		; Store new value
	shl	ax,1			; Times size of queue element
	add	di,QUEUE		; Point to base of queue data
	add	di,ax			; Add in pointer
	mov	ax,[bp+4]		; Get trace data word
	mov	WORD PTR [di],ax	; Save trace caller's parameter
	sub	di,_tq_fnc		;
	add	di,_tq_ret		;
	mov	WORD PTR [di],bx	; Save caller's return address
	popf				;				091089
	pop	di			;
	pop	bp			;
	ret				;
_trcqadd endp

;=============================================================================
;  Pitfall #10	This was moved here from asmutil.s (replaced by RECV_BUFFER
;		and XMIT_BUFFER).  Note the hardcoding of base segments for
;		the transmit and receive areas -- what if these change?
;		Lots of meaningless shuffling, and'ing and general arm-waving;
;		why not simply multiply the buffer number times the buffer
;		size and add to the respective base paragraph (as now done
;		by their replacements in "asmutil.s")?  Will the following
;		code work if the sizes of the buffers change (not = 1000h)?

; Compute RX ad TX Buffer addresses from passed "Buffer Number"
; Return DX:AX with dx = X000 ax = X000

bufserv	proc	near
	push	bp			;
	mov	bp,sp			;
	mov	ax,[bp+4]		; Passed buffer number
	add	al,dl			; Add segment base

	mov	dh,al			;
	and	dx,0F000h		;
	ror	ax,4			;
	and	ax,0F000h		;

	pop	bp			;
	ret				; Result in DX:AX
bufserv	endp

	public	_RX_BUFADDR,_TX_BUFADDR
_RX_BUFADDR proc near
	mov	dl,20h			; Rx Buffer Base Segment
	jmp	bufserv			;
_RX_BUFADDR endp
_TX_BUFADDR proc near
	mov	dl,50h			; Tx Buffer Base Segment
	jmp	bufserv			;
_TX_BUFADDR endp

;=============================================================================
;  Pitfall #11	This was moved here from hardutil.s (replaced by start_bm_dma
;		in asmutil.s).  This could have been done in about 40 lines
;		of straightforward assembly code (see start_bm_dma), but it 
;		seems that a lot of the extra code here exists to facilitate 
;		use of obscure or 'nifty' instructions like "cld", "outs", 
;		and "lodsw" -- a simpler and more efficient approach would 
;		be to let the assembler calculate offsets (as in start_bm_dma) 
;		rather than use complicated string instructions that perform 
;		calculations at run time.  In addition, one of the nastier 
;		side effects of this code is that it clobbers the Channel 
;		Descriptor Block image handed to it . . . sometimes.

;*****************************************************************************
; Bus Master DMA Utilities - Provide parameter reversal and manipulation to
; turn standard C language variables into the structure required by Contender.

	.SALL				; Suppress macro expansion code

CA_XCH	macro	C_OFF			; Exchange words [C_OFF], [C_OFF+2]

;=============================================================================
;	Why exchange with the caller's image when you can simply write the
;	value to the appropriate I/O register?

	mov	ax,C_OFF[si]		;( 8)
	xchg	ax,C_OFF+2[si]		;(17)
	mov	C_OFF[si],ax		;( 9)
	endm
CA_SEG	macro	C_OFF			; Turn Segment into Address Extension
	rol	WORD PTR C_OFF[si],4	;(21)
	endm
;
BL_LGT	EQU	12			; Length of a Buffer List Chain Entry
;
;                              =========                                       
; SetBM, as the name suggests, (really?) sets up and starts the Bus Master DMA 
; channel.  It understands Linked and Buffer List Chaining much like 
; Contender itself.
;
; Entry:  [bp+4]  :  Near pointer to transfer count of first CDB
;	  [bp+6]  :  Coalesce pointer parameters?  0=Yes, !0=No
;
	public	_SetBM
_SetBM	proc	near
	enter	0,0			; 15   Create local stack frame
	pusha				; 36   
	push	ds			; 10
	push	es			; 10
					;
	cmp	WORD PTR [bp+6],0	; 10   Should we reverse everything?
	jne	_BMagic			; 13/4
					;
	mov	ax,[bp+4]		;  8   "Near" CDB Pointer
	mov	si,ax			;  2
					;
	mov	cx,BM_CC_LE		;  4   Register access to immediate
	CA_XCH	BM_CAE			; 34   Exchange Card Address Words
	CA_SEG	BM_CAE			; 21   Convert Card Address Extension
	CA_XCH	BM_SAE			; 34   Exchange System Address Words

;=============================================================================
;	We do not use Buffer or Linked List Chaining.

	test	WORD PTR BM_CC[si],cx	; 10   List Chaining enabled?
	jz	_BMagic			; 14    no, arm the channel
nxt_llc:				;
	CA_XCH	BM_LAE			; 34   Exchange List Pointer Words
	mov	ax,BM_LA[si]		;  8   Save List Pointer in ES:DI
	mov	di,ax			;  2
	mov	ax,BM_LAE[si]		;  8
	mov	es,ax			;  2
	CA_SEG	BM_LAE			; 21   Convert List Pointer Extension
	mov	ax,es			;  2
	mov	ds,ax			;  2
	mov	si,di			;  2   Continue List from ES:DI
nxt_blc:				;
	mov	ax,[si]			;  8   Obtain the Transfer Count
	mov	bx,ax			;  2   
	or	ax,ax			;  3   Buffer or linked list chaining?
	jnz	blc_nxt			; 13/4 Buffer List if it jumps
	add	si,2			;  4   Point past length to real CDB
blc_nxt:				;
	CA_XCH	BM_CAE			; 34   Exchange Card Address Words
	CA_SEG	BM_CAE			; 21   Convert Card Address Extension
	CA_XCH	BM_SAE			; 34   Exchange System Address Words
	test	WORD PTR BM_CC[si],cx	; 10   List Chaining enabled?
	jz	_BMagic			; 14    no, arm the channel
	or	bx,bx			;  3   Buffer or linked list chaining?
	jz	nxt_llc			; 13/4 Linked List if it jumps
	add	si,BL_LGT		;  4   Increment to next Buffer List
	jmp	nxt_blc			; 14   exit is only via jump to BMagic
_SetBM	endp

;*****************************************************************************
; BMagic is always JMPed into when SetBM detects the end of the buffer and/or
; linked list chaining elements.  On entry, the entry parameter to SetBM is
; fetched and the I/O performed to start the first of the Scatter or Gather
; operations.  This code knows nothing about what Contender is being told, it
; simply performs the I/O to the master Channel Descriptor Block.
; Major code changes 083089, only functional additions are dated separately.

	extrn	_queue_writel:NEAR
	extrn	_arqueue:BYTE
_BMagic	proc	near			;

;=============================================================================
;	If a lock were implemented around the BM DMA channel (set here, free
;	in BM DMA interrupt handler), this check would be unnecessary and
;	BM DMA would run more reliably (no race condition).  This lock now
;	exists in the latest version of intzero.c

	in	ax,BMDMA_BASE+BM_CC	; 10				100389
	test	ax,BM_CC_START		;  4				100389
	jz	BMok			; 13/4				100389
	push	6001h			; 10				100389
	push	0DEADh			; 10				100389
	push	OFFSET _arqueue		; 10				100389
	call	_queue_writel		; 15				100389
	add	sp,6			;  4				100389
	jmp	BMend			; 14				100389
BMok:					;
	mov	ax,LOADSEG		;  4				083089
	mov	ds,ax			;  2				083089
	mov	es,ax			;  2				083089
	cld				;  2	Set dir flag (to increment)
	mov	ax,[bp+4]		;  8	Refetch the parameter,
	mov	si,ax			;  2
					;
	mov	dx,CIO_0+P_C_DPP	;  4	Prepare to kill LED	091089
	mov	al,0F5h			;  3	Data Pattern Polarity	091089
	out	dx,al			;  8	for Port C/CIO 0	091089
					;
	mov	dx,BMDMA_BASE		;  4	Start of CDB
					;
	outs	dx,WORD PTR[si]		; 14	Transfer count
	add	dx,2			;  4
	outs	dx,WORD PTR[si]		; 14	Card Address Extension
	add	dx,2			;  4
	outs	dx,WORD PTR[si]		; 14	Card Address
	add	dx,2			;  4
	outs	dx,WORD PTR[si]		; 14	System Address Extension
	add	dx,2			;  4
	outs	dx,WORD PTR[si]		; 14	System Address
	add	dx,4			;  4
	lodsw				; 10	Channel Control
	outs	dx,WORD PTR[si]		; 14	List Address Extension
	add	dx,2			;  4
	outs	dx,WORD PTR[si]		; 14	List Address
					;
	out	BMDMA_BASE+BM_CC,ax	;  9	Write Channel Control Word
					;	This starts Bus Master DMA.
BMend:					;
	pop	es			; 10
	pop	ds			; 10
	popa				; 51
	leave				;  8
	ret				; 16
_BMagic	endp

;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end
