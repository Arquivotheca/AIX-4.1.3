	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS:
;	_in08		: Get a byte from I/O space
;	_in16		: Get a short (word) from I/O space
;	_out08		: Put a byte into I/O space
;	_out16		: Put a short (word) into I/O space
;	_disable	: Disable maskable processor interrupts
;	_enable		: Re-enable processor interrupts after _disable
;	_halt		: Stop the CPU until an interrupt occurs
;	_Log2Seg	: Convert real address to segmented address
;	_AddrInc2	: Increment a segmented address by two (cross seg)
;	_MemVfy		: Write and verify a pattern in memory block
;	_host_intr	: Interrupt the system unit via TASKREG
;	_SetPDMA	: Setup a port DMA channel, never starts it
;	_InitPDMA	: Start a port DMA channel.  Reads/writes CCW.
;	_xmt_24bits	: Send 24 bit times of a character (X.21)
;	_RECV_BUFFER	: Convert a receive buffer index to a FAR address
;	_XMIT_BUFFER	: Convert a transmit buffer index to a FAR address
;	_XMIT_EXTENSION : Convert a transmit extension index to a FAR address
;	_next_bcc	: Calculate next block check character (CRC/LRC)
;	_set_FAST	: Setup an SCC for Counter/Timer clocked transfers
;	_Ascii_To_Ep	: Convert a data block to Even parity
;	_Ascii_To_Op	: Convert a data block to Odd parity
;	_Ascii_Fr_Ep	: Convert a data block from Even parity
;	_Ascii_Fr_Op	: Convert a data block from Odd parity
;	_start_bm_dma	: Start Bus Master DMA transfer.
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
;  FUNCTION: Utility services for both "C" and assembly language
;
;**********************************************************************
;
	TITLE	asmutil
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)52	1.8  src/bos/usr/lib/asw/mpqp/asmutil.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:58:53'
	db	0
	.XLIST
	include	define.inc
	include	mpqp.inc
	include	iapx186.inc
	include	duscc.inc
	include	cio.inc
	include	portdma.inc
	.LIST
	PAGE
;
; In 8 bits, parameter: I/O address (DX,[bp+4])
;
	public	_in08
_in08	proc	near
	enter	0,0			;
	mov	dx,word ptr [bp+4]	; I/O Address
	in	al,dx
	xor	ah,ah			; Always returns 0-255
	leave				;
	ret
_in08	endp
;
; In 16 bits, parameter: I/O address (DX,[bp+4])
;
	public	_in16
_in16	proc	near
	enter	0,0			;
	mov	dx,word ptr [bp+4]	; I/O Address
	in	ax,dx
	leave				;
	ret
_in16	endp
;
; Out 8 bits, parameters: I/O address (DX,[bp+4]) and value (AX,[bp+6])
;
	public	_out08
_out08	proc	near
	enter	0,0			;
	mov	dx,word ptr [bp+4]	; I/O Address
	mov	ax,word ptr [bp+6]	; value
	out	dx,al
	leave				;
	ret
_out08	endp
;
; Out 16 bits, parameters: I/O address (DX,[bp+4]) and value (AX,[bp+6])
;
	public	_out16
_out16	proc	near
	enter	0,0			;
	mov	dx,word ptr [bp+4]	; I/O Address
	mov	ax,word ptr [bp+6]	; value
	out	dx,ax
	leave				;
	ret
_out16	endp
;
;
; Clear X.21 - turn off T and C in a very short period
;
	public	_clrTC
_clrTC	proc	near
	enter	0,0			;
	mov	dx,ENREG		; Set dx to Enable Reg
	mov	al,04h			; Turn C off
	out	dx,al			;
	mov	dx,CIO_0+P_A_DATA	;
	mov	al,1Fh			; Disable 422 drivers
	out	dx,al			;
	leave				;
	ret
_clrTC	endp
;
; Disable interrupts to the 80186 processor
;
;	public	_disable
;_disable proc	near
;	pop	ax	; Save return address from stack
;	mov	bx,ax	; Store in bx
;	pushf		; Push CPU flags onto stack
;	cli		; Clear interrupt enable flag
;	push	bx	; Put return address back on stack;
;	ret		; return to that address.
;_disable endp

;	public	_enable
;_enable	proc	near
;	pop	ax	; Save return address from stack
;	mov	bx,ax	; Store in bx
;	popf		; Restore CPU flags from stack (allows nesting)
;	push	bx	; Put return address back on stack;
;	ret		; return to that address.
;_enable endp



;******************************************************
	extrn 	_disables:BYTE
	public	_disable
_disable proc	near
;Start of test code
	cli			; Clear interrupt enable flag
	inc	_disables	; increment int_stack pointer
	ret			; return
_disable endp
;
	extrn 	_disables:BYTE
	public	_enable
_enable	proc	near
;Start of test code
	xor	ah,ah		; clear ah
	mov	al,_disables	; mov disables into al
	cmp	al,0		; Already 0?
	jne	cntdown		; jump if ok
	hlt			; halt the CPU if disables out of wack
cntdown:
	dec	ax	 	; decrement used count
	jnz	return		; if disables are nested, return
	sti			; Set interrupt enable flag
return:
	mov	_disables,al	; Restore disables count
	ret			; Return
_enable endp
	page
;
; PORTID : Turn a bit masked value into a port number.  First bit in from the
;          right is the one we count until.
;   entry: [bp+4] - bit field, a 16 bit word
;    exit:  ax    - port number (0-15) OR -1 (FFFF) if called with 0 
;
	public	_port
_port	proc	near
	enter	0,0			; 15
	push	cx			; 10+
;
	mov	ax,word ptr[bp+4]	; 12+	bit masked port number
	mov	cx,0FFFFh		;  4	pre-increment, so set to -1
	cmp	ax,0			;  4	code would loop forever on 0
	jz	portend			;  4/13	return -1 if called w/0
portloop:
	inc	cx			;  3
	shr	ax,1			;  2
	jnc	portloop		;  4/13
portend:
	mov	ax,cx			;  2	store return value
	pop	cx			; 10+
	leave				;  8
	ret				; 16
_port	endp
;
; PORTBIT: Turn a port number into a bit mask, the opposite of PORT
;   entry: [bp+4] - port number (0-15), higher bits ignored
;    exit:  ax    - bit mask to manipulate a scheduler level with
;    NOTE: Function depends on sch_lev being 16 bits wide (return value)
;        : To expand requires ax/dx retval and shift with carry to high half
;
	public	_portbit
_portbit proc	near
	enter	0,0			;
	push	cx			; 10+
	mov	ax,1			;     Preset return value
;
	mov	cx,word ptr[bp+4]	;     Fetch parameter
	and	cx,000Fh		;     Only 0-15 are meaningful
	jz	bit_ex			;     We're finished
bit_loop:
	shl	ax,1			;
	loop	bit_loop		;
;
bit_ex:
	pop	cx			; 10+
	leave				;
	ret				; 16
_portbit endp

	public	$$retf
$$retf	proc	near
	pop	ax			; discard near return location
	db	0CBh			; Hardcoded Far Return
$$retf	endp

	public	$$retn
$$retn	proc	near
	pop	ax			; discard near return location
	db	0C3h			; Hardcoded Near Return
$$retn	endp

	public	_halt
_halt	proc	near
	pushf				;
	cli				;
	hlt				;
	popf				;
	ret				;
_halt	endp

	public	_Log2Seg
_Log2Seg proc	near
	enter	0,0			;
	mov	dx,[bp+6]		; Get segment information
	and	dx,000Fh		; Mask to addressable RAM
	ror	dx,4			; Move low nibble to high
	mov	ax,[bp+4]		; Get offset
	leave				;
	ret				;
_Log2Seg endp

	public	_AddrInc2
_AddrInc2 proc	near
	enter	0,0			;
	xor	dx,dx			;
	mov	bx,[bp+6]		; Segment part of address
	rol	bx,4			; Segment to Logical
	mov	ax,[bp+8]		; Get number of words to increment by
	shl	ax,1			; Times two = word size
	rcl	dx,1			; Maintain significant bit
	add	ax,[bp+4]		; Add low
	adc	dx,bx			; Add high and carry from low
	and	dx,000Fh		; Sanity check
	ror	dx,4			; Logical to Segment
	leave				;
	ret				;
_AddrInc2 endp

; Test memory with a pattern.
; unsigned char far *MemVfy( unsigned char far *, unsigned, unsigned );
;                            Test Region Ptr,     Pattern,  Test Length
; Returns 0 on success, address of failure on error.

	public	_MemVfy
_MemVfy	proc	near
	push	bp
	mov	bp,sp
	push	di
	push	es
	les	di,[bp+4]
	mov	ax,[bp+8]
	mov	cx,[bp+10]
	rep  stos WORD PTR [di]
	les	di,[bp+4]
	mov	cx,[bp+10]
	repe scas WORD PTR [di]
	jcxz	MemOk
	sub	di,2
	mov	dx,di
	mov	ax,es
MemExit:
	pop	es
	pop	di
	pop	bp
	ret
MemOk:
	xor	ax,ax
	mov	dx,ax
	jmp	MemExit
_MemVfy	endp

;
; WARNING: In the real code, an interrupt failsafe timer must be put around
; this code because there are probably timing windows in the host software.
; If we time out again, the host is ignoring us and we can presumably clean
; up as we see fit.
;
	extrn	_lost_intr:WORD
	public	_host_intr
_host_intr proc	near
	enter	0,0			;
	in	ax,NMISTAT		;
	test	ax,NMI_S_IP		; Is an interrupt pending to the host?
	jz	intreq			;  no, get on with this interrupt
	inc	_lost_intr		; Increment lost interrupt count
	in	al,TASKREG		; Clear previous interrupt type data
intreq:					;
	mov	al,[bp+4]		; TASKREG value, only parameter
	out	TASKREG,al		;
	leave				;
	ret
_host_intr endp

; Setup a Port (echo) DMA channel.  Channels are dedicated to inbound
; or outbound data service already, so the only significant bit in the
; channel control word in Start/Stop.
; Entry Parameters:
;	[bp+4] : I/O Base address of desired DMA channel (CCW address)
;
CA	EQU	WORD PTR 0		; Card Address, Least significant word
CAE	EQU	WORD PTR 2		; Card Address, Most significant word
TC	EQU	WORD PTR 4		; Transfer Count
IOA	EQU	WORD PTR 6		; I/O Address, Tx or Rx FIFO
CMB	EQU	WORD PTR 8		; Character Match Bytes, Rx Only
CCW	EQU	WORD PTR 10		; Channel Control Word, Enable bit is
					;  never written out.
LA	EQU	WORD PTR 12		; List Address, LSW (Rx Only)
LAE	EQU	WORD PTR 14		; List Address, MSW (Rx Only)

	extrn	_pdma_cdt:WORD
	public	_SetPDMA
_SetPDMA proc	near
	enter	0,0			;
	push	si			;
	push	dx			;
	mov	si,OFFSET _pdma_cdt	;
	mov	dx,[bp+4]		;      Get Echo channel I/O Base Addr
;
	add	dx,PDMA_CCW		;      Channel Control Word
	mov	ax,CCW[si]		;
	and	ax,NOT PDMA_CCW_EN	;      Ensure channel is not started
	out	dx,ax			;
					;
	add	dx,PDMA_CA-PDMA_CCW	;      Card Address
	mov	ax,CA[si]		;
	out	dx,ax			;
	add	dx,PDMA_CM-PDMA_CA	;      Character Match bytes
	mov	ax,CMB[si]		;
	out	dx,ax			;
	add	dx,PDMA_IOA-PDMA_CM	;      I/O Address
	mov	ax,IOA[si]		;
	out	dx,ax			;
	add	dx,PDMA_TC-PDMA_IOA	;      Transfer Count
	mov	ax,TC[si]		;
	out	dx,ax			;
	add	dx,PDMA_CAE-PDMA_TC	;      Card Address Extension
	mov	ax,CAE[si]		;
	rol	ax,4			;
;	and	ax,0Fh			;
	out	dx,ax			;
;
	add	dx,PDMA_LAE-PDMA_CAE	;      List Address Extension
	mov	ax,LAE[si]		;
	rol	ax,4			;
	out	dx,ax			;
	add	dx,PDMA_LA-PDMA_LAE	;      List Address
	mov	ax,LA[si]		;
	out	dx,ax			;
setExit:				;
	pop	dx			;
	pop	si			;
	leave				;
	ret				;
_SetPDMA endp

; InitPDMA accepts the Echo channel base address as its only parameter.
; It reads the current CCW, ORs in the ENABLE bit and rewrites it.

	public	_InitPDMA
_InitPDMA proc	near
	enter	0,0			;
	mov	dx,[bp+4]		; Echo channel I/O base address
	in	ax,dx			; CCW is first word of CDT
	test	ax,PDMA_CCW_EN		; Already running?	
	jnz	running			;  yes.
	or	ax,PDMA_CCW_EN		; Add the enable bit
	out	dx,ax			;
running:				;
	leave				;
	ret				;
_InitPDMA endp

;-------------------------------------------------------------------------
;	XMIT_BUFFER
;	
;	Input:	Index of transmit buffer (sp + 4)
;	Output:	Address of transmit buffer (DX:AX)

	public	_XMIT_BUFFER
_XMIT_BUFFER proc near			; 27 AUG 90  MGM

	push	bp			; Save current frame pointer
	mov	bp,sp			; Get new frame pointer
	mov	dx,TXBSIZE		; Size of transmit buffer
	mov	ax,[bp+4]		; passed buffer number
	xor	ah,ah			; remove any garbage
	mul	dx			; Multiply to get offset in DX,AX
	ror	dx,4			; shift DX to make it DX:AX
	add	dx,TXPARA		; add in TX Buffer base paragraph

	pop	bp			; Restore Frame Pointer
	ret				; Return Result in DX:AX
_XMIT_BUFFER endp
	
;-------------------------------------------------------------------------
;	XMIT_EXTENSION
;	
;	Input:	Index of transmit extension (sp + 4)
;	Output:	Address of transmit extension (DX:AX)

	public	_XMIT_EXTENSION
_XMIT_EXTENSION proc near		; 27 AUG 90  MGM

	push	bp			; Save current frame pointer
	mov	bp,sp			; Get new frame pointer
	mov	dx,TXESIZE		; Size of transmit extension
	mov	ax,[bp+4]		; passed extension number
	xor	ah,ah			; remove any garbage
	mul	dx			; Multiply to get offset in DX,AX
	ror	dx,4			; shift DX to make it DX:AX
	add	dx,TXEPARA		; add in TX Extension base paragraph

	pop	bp			; Restore Frame Pointer
	ret				; Return Result in DX:AX
_XMIT_EXTENSION endp
	
;-------------------------------------------------------------------------
;	RECV_BUFFER
;	
;	Input:	Index of receive buffer (sp + 4)
;	Output:	Address of receive buffer (DX:AX)

	public	_RECV_BUFFER
_RECV_BUFFER proc near			; 27 AUG 90  MGM

	push	bp			; Save current frame pointer
	mov	bp,sp			; Get new frame pointer
	mov	dx,RXBSIZE		; Size of receive buffer
	mov	ax,[bp+4]		; passed buffer number
	xor	ah,ah			; remove any garbage
	mul	dx			; Multiply to get offset in DX,AX
	ror	dx,4			; shift DX to make it DX:AX
	add	dx,RXPARA		; add in RX Buffer base paragraph

	pop	bp			; Restore frame pointer
	ret				; Return Result in DX:AX
_RECV_BUFFER endp

;-------------------------------------------------------------------------
;	NEXT_BCC
;
;	Calculates the next block check character for the given bcc and
;	the next byte of data.  If CRC is selected, a bytewise CRC-16
;	Cyclic Redundancy Check is performed; otherwise a Longitudinal
;	Redundancy Check (vertical parity) is performed.
;	
;	Input:	Next byte to accumulate in block check character (sp + 4)
;		Current block check character (sp + 6)
;		CRC flag: TRUE for CRC accumulation, FALSE for LRC (sp + 8)
;	Output:	Value of new block check character (AX)

	extrn	_Crctbl:BYTE
	public	_next_bcc		; 27 APR 90  MGM
_next_bcc proc near
	
	push	bp			; Save current frame pointer
	mov	bp,sp			; Get new frame pointer
	push	si			; and index register
	mov	ax,[bp+8]		; Get CRC flag.
	mov	dx,[bp+6]		; Get current bcc
	cmp	ax,0			; CRC flag set?
	je	lrc			; If not, do LRC

					; CRC-16 CALCULATION:
	mov	al,BYTE PTR [bp+4]	;     Get new byte
	xor	ax,dx			;     Get CRC table index
	xor	ah,ah			;     Zero high byte
	mov	si,ax			;     Move to index register
	shl	si,1			;     Make it word-based
	mov	ax,WORD PTR _Crctbl[si]	;     Look up from table
	shr	dx,8			;     Shift bcc right 8 bits
	xor	ax,dx			;     Then xor for result in AX

	pop	si			; Restore index register
	pop	bp			; and previous frame pointer
	ret
lrc:					; LRC-8 CALCULATION:
	mov	al,BYTE PTR [bp+4]	;     Get new byte
	xor	ax,dx			;     Then xor for result in AX
	and	ax,007Fh		;     Mask off parity bit
	
	pop	si			; Restore index register
	pop	bp			; and previous frame pointer
	ret
_next_bcc endp

;-------------------------------------------------------------------------
;	START_BM_DMA
;
;	Transfers the Bus Master Microchannel Channel Descriptor Block
;	image to I/O address space and initiates the DMA transfer (either
;	from or to the host) by writing the start bit to the Channel 
;	Control Register.
;	Input:	Address of Channel Descriptor Block image to be transferred
;		to I/O space (sp + 4).
;	Output: None.

	public	_start_bm_dma		; 21 SEP 90  MGM
_start_bm_dma proc near

	push	bp			; Save current frame pointer
	mov	bp,sp			; Get new frame pointer
	push	si			; and save registers
	push	ax
	push	dx
	mov	ax,[bp+4]		; Get CCB Address.
	mov	si,ax

;	The Watchdog LED is turned off at the initiation of Bus Master
;	DMA.  The BM DMA interrupt handler then turns it back on -- if
;	we don't get an interrupt back, we are dead (LED off).

	mov	dx,CIO_0+P_C_DPP	; Turn off LED
	mov	al,0F5h			; Port C of CIO 0
	out	dx,al

;	List chaining is not used for Bus Master DMA, so the List
;	Address is set to zero.

	xor	ax,ax			; Clear AX
	mov	dx,BMDMA_BASE+BM_LAE	; List Address Extension
	out	dx,ax
	mov	dx,BMDMA_BASE+BM_LA	; List Address
	out	dx,ax

;	The remainder of the Channel Descriptor Block is transferred to
;	I/O space, writing of the final word (Channel Control Register) 
;	causes Contender to begin the DMA transfer.

	mov	dx,BMDMA_BASE+BM_TC	; Transfer Count
	mov	ax,WORD PTR CDB_TC[si]
	out	dx,ax	

	mov	dx,BMDMA_BASE+BM_CAE	; Card Address Extension
	mov	ax,WORD PTR CDB_CA_HI[si]
	rol	ax,4			; segment -> physical address
	out	dx,ax	
	
	mov	dx,BMDMA_BASE+BM_CA	; Card Address
	mov	ax,WORD PTR CDB_CA_LO[si]
	out	dx,ax	

	mov	dx,BMDMA_BASE+BM_SAE	; System (Host) Address Extension
	mov	ax,WORD PTR CDB_SA_HI[si]
	out	dx,ax	

	mov	dx,BMDMA_BASE+BM_SA	; System (Host) Address
	mov	ax,WORD PTR CDB_SA_LO[si]
	out	dx,ax	

	mov	dx,BMDMA_BASE+BM_CC	; Channel Control Word
	mov	ax,WORD PTR CDB_CC[si]
	out	dx,ax	
	
	pop	dx			; Restore registers
	pop	ax
	pop	si
	pop	bp			; and previous frame pointer
	ret
_start_bm_dma endp


;-------------------------------------------------------------------------
;
; Assembler service routine to setup high speed DUSCC Counter/Timer interval
; based transmitter clocking.  Notice that the receiver is set to use the
; DPLL, i.e. remains "externally" clocked.
;
; Pass SCC Base address at [BP+4], first parameter
; C/T time constant (preset) is a word at [BP+6]
;
	public	_set_FAST
_set_FAST proc	near
	enter	0,0

	mov	dx,[bp+4]		; SCC Base address		090789
	add	dx,TXTIM		; Transmitter timing		080689
	mov	al,0F0h			; TRxC Source, 32x Own C/T	080689
	out	dx,al			;				080689
	add	dx,RXTIM-TXTIM		; Receiver timing		080689
	mov	al,70h			; RTxC Source, DPLL @ 32x C/T	091189
	out	dx,al			;				080689
	add	dx,CTPR_HI-RXTIM	; Counter/Timer Preset, High	080689
	mov	al,[bp+7]		; User-supplied time constant	080689
	out	dx,al			;				080689
	add	dx,CTPR_LO-CTPR_HI	; Counter/Timer Preset, Low	080689
	mov	al,[bp+6]		; User-supplied time constant	080689
	out	dx,al			;				080689
	add	dx,CTCTL-CTPR_LO	; Counter/Timer Control		091589
	mov	al,2			; xtal/4, preset, square	091589
	out	dx,al			;				091589
	add	dx,PINCFG-CTCTL		; Pin Configuration		091589
	mov	al,26h			; RTS pin, TRxC from 1x TxClk	080689
	out	dx,al			;				080689
	add	dx,CH_CMD-PINCFG	; Channel Command		080689
	mov	al,83h			; Preset from CTPR HI/LO	091589
	out	dx,al			;				091589
	mov	al,80h			; Start Counter/Timer		080689
	out	dx,al			;				080689

	leave
	ret
_set_FAST endp

; High Level Interface:
; extern Ascii_To_Ep ( unsigned char far  *p_data, int  lgt )
; extern Ascii_To_Op ( unsigned char far  *p_data, int  lgt )
;
; Translate from ASCII (00-7F) data to data with parity.  Calling sequences:
;	[bp+4] (parm1) : Translate data address, offset
;	[bp+6] (parm1) : Translate data address, segment
;	[bp+8] (parm2) : Translate operation length
; Returns:
;	 0  : Always returns zero
;
	public	_Ascii_To_Ep
_Ascii_To_Ep proc near
	enter	0,0			;
	push	bx			; Translate table pointer
	mov	bx,OFFSET xlat2ep	; Set translate table address
	jmp	xlatTo			; Generic translation routine
_Ascii_To_Ep endp

	public	_Ascii_To_Op
_Ascii_To_Op proc near
	enter	0,0			;
	push	bx			; Translate table pointer
	mov	bx,OFFSET xlat2op	; Set translate table address
	jmp	xlatTo			; Generic translation routine
_Ascii_To_Op endp

xlatTo	proc	near
	push	cx			; Translate lookup count
	push	si			; String to be translated
	push	di			; Result of translation
;
	mov	di,word ptr[bp+4]	; Set translate string address
	mov	si,di			; out = in
	mov	ax,word ptr[bp+6]	; Translate string segment value
	push	es
	mov	es,ax
	mov	cx,word ptr[bp+8]	; Set translate iteration count
reTo:
	lods	es:BYTE PTR [si]	; al <- [si++]
	xlat				; al <- [bx+al]
	stos	es:BYTE PTR [di]	; [di++] <- al
	loop	reTo
	jmp	xlatRet
xlatTo	endp

;
; High Level Interface:
; extern Ascii_Fr_Ep ( unsigned char far  *p_data, int  lgt )
; extern Ascii_Fr_Op ( unsigned char far  *p_data, int  lgt )
;
; Translate FROM data with parity back into ASCII data.  Calling sequences:
;	[bp+4] (parm1) : Translate data address, offset
;	[bp+6] (parm1) : Translate data address, segment
;	[bp+8] (parm2) : Translate operation length
; Returns:
;	 0  : Successful translation
;	!0  : Error in lookup, parity error detected
;
	public	_Ascii_Fr_Ep
_Ascii_Fr_Ep proc near
	enter	0,0			;
	push	bx			; Translate table pointer
	mov	bx,OFFSET xlatFep	; Set translate table address
	jmp	xlatFrom		; Generic translation routine
_Ascii_Fr_Ep endp

	public	_Ascii_Fr_Op
_Ascii_Fr_Op proc near
	enter	0,0			;
	push	bx			; Translate table pointer
	mov	bx,OFFSET xlatFop	; Set translate table address
	jmp	xlatFrom		; Generic translation routine
_Ascii_Fr_Op endp
	
xlatFrom proc	near
	push	cx			; Translate lookup count
	push	si			; String to be translated
	push	di			; Result of translation
;
	mov	di,word ptr[bp+4]	; Set translate string address
	mov	si,di			; out = in
	mov	ax,word ptr[bp+6]	; Translate string segment value
	push	es
	mov	es,ax
	mov	cx,word ptr[bp+8]	; Set translate iteration count
reFrom:
	lods	es:BYTE PTR [si]	; al <- [si++]
	xlat				; al <- [bx+al]
	cmp	al,0FFh			; Parity error on lookup?
	je	exFrom			; Yes, indicate to caller
	stos	es:BYTE PTR [di]	; [di++] <- al
	loop	reFrom
	xor	ax,ax			; No error, report success
exFrom:
	jmp	xlatRet
xlatFrom endp

xlatRet	proc	near
	pop	es
	pop	di
	pop	si
	pop	cx
	pop	bx
	leave
	ret
xlatRet	endp
	
xlat2ep	equ	$			; Translate TO even parity from ASCII
	db	 00h, 81h, 82h, 03h, 84h, 05h, 06h, 87h
	db	 88h, 09h, 0Ah, 8Bh, 0Ch, 8Dh, 8Eh, 0Fh
	db	 90h, 11h, 12h, 93h, 14h, 95h, 96h, 17h
	db	 18h, 99h, 9Ah, 1Bh, 9Ch, 1Dh, 1Eh, 9Fh
	db	0A0h, 21h, 22h,0A3h, 24h,0A5h,0A6h, 27h
	db	 28h,0A9h,0AAh, 2Bh,0ACh, 2Dh, 2Eh,0AFh
	db	 30h,0B1h,0B2h, 33h,0B4h, 35h, 36h,0B7h
	db	0B8h, 39h, 3Ah,0BBh, 3Ch,0BDh,0BEh, 3Fh
	db	0C0h, 41h, 42h,0C3h, 44h,0C5h,0C6h, 47h
	db	 48h,0C9h,0CAh, 4Bh,0CCh, 4Dh, 4Eh,0CFh
	db	 50h,0D1h,0D2h, 53h,0D4h, 55h, 56h,0D7h
	db	0D8h, 59h, 5Ah,0DBh, 5Ch,0DDh,0DEh, 5Fh
	db	 60h,0E1h,0E2h, 63h,0E4h, 65h, 66h,0E7h
	db	0E8h, 69h, 6Ah,0EBh, 6Ch,0EDh,0EEh, 6Fh
	db	0F0h, 71h, 72h,0F3h, 74h,0F5h,0F6h, 77h
	db	 78h,0F9h,0FAh, 7Bh,0FCh, 7Dh, 7Eh,0FFh
xlat2op	equ	$			; Translate TO odd parity from ASCII
	db	 80h, 01h, 02h, 83h, 04h, 85h, 86h, 07h
	db	 08h, 89h, 8Ah, 0Bh, 8Ch, 0Dh, 0Eh, 8Fh
	db	 10h, 91h, 92h, 13h, 94h, 15h, 16h, 97h
	db	 98h, 19h, 1Ah, 9Bh, 1Ch, 9Dh, 9Eh, 1Fh
	db	 20h,0A1h,0A2h, 23h,0A4h, 25h, 26h,0A7h
	db	0A8h, 29h, 2Ah,0ABh, 2Ch,0ADh,0AEh, 2Fh
	db	0B0h, 31h, 32h,0B3h, 34h,0B5h,0B6h, 37h
	db	 38h,0B9h,0BAh, 3Bh,0BCh, 3Dh, 3Eh,0BFh
	db	 40h,0C1h,0C2h, 43h,0C4h, 45h, 46h,0C7h
	db	0C8h, 49h, 4Ah,0CBh, 4Ch,0CDh,0CEh, 4Fh
	db	0D0h, 51h, 52h,0D3h, 54h,0D5h,0D6h, 57h
	db	 58h,0D9h,0DAh, 5Bh,0DCh, 5Dh, 5Eh,0DFh
	db	0E0h, 61h, 62h,0E3h, 64h,0E5h,0E6h, 67h
	db	 68h,0E9h,0EAh, 6Bh,0ECh, 6Dh, 6Eh,0EFh
	db	 70h,0F1h,0F2h, 73h,0F4h, 75h, 76h,0F7h
	db	0F8h, 79h, 7Ah,0FBh, 7Ch,0FDh,0FEh, 7Fh

xlatFep	equ	$			; Translate FROM even parity ASCII
					; 0xFF values indicate PARITY ERRORs
	db	 00h,0FFh,0FFh, 03h,0FFh, 05h, 06h,0FFh
	db	0FFh, 09h, 0Ah,0FFh, 0Ch,0FFh,0FFh, 0Fh
	db	0FFh, 11h, 12h,0FFh, 14h,0FFh,0FFH, 17h
	db	 18h,0FFH,0FFh, 1Bh,0FFh, 1Dh, 1Eh,0FFh
	db	0FFh, 21h, 22h,0FFh, 24h,0FFh,0FFH, 27h
	db	 28h,0FFH,0FFh, 2Bh,0FFh, 2Dh, 2Eh,0FFh
	db	 30h,0FFh,0FFh, 33h,0FFh, 35h, 36h,0FFh
	db	0FFh, 39h, 3Ah,0FFh, 3Ch,0FFh,0FFh, 3Fh
	db	0FFh, 41h, 42h,0FFh, 44h,0FFh,0FFH, 47h
	db	 48h,0FFH,0FFh, 4Bh,0FFh, 4Dh, 4Eh,0FFh
	db	 50h,0FFh,0FFh, 53h,0FFh, 55h, 56h,0FFh
	db	0FFh, 59h, 5Ah,0FFh, 5Ch,0FFh,0FFh, 5Fh
	db	 60h,0FFh,0FFh, 63h,0FFh, 65h, 66h,0FFh
	db	0FFh, 69h, 6Ah,0FFh, 6Ch,0FFh,0FFh, 6Fh
	db	0FFh, 71h, 72h,0FFh, 74h,0FFh,0FFH, 77h
	db	 78h,0FFH,0FFh, 7Bh,0FFh, 7Dh, 7Eh,0FFh		; End   00-7F
	db	0FFh, 01h, 02h,0FFh, 04h,0FFh,0FFH, 07h		; Start 80-FF
	db	 08h,0FFH,0FFh, 0Bh,0FFh, 0Dh, 0Eh,0FFh
	db	 10h,0FFh,0FFh, 13h,0FFh, 15h, 16h,0FFh
	db	0FFh, 19h, 1Ah,0FFh, 1Ch,0FFh,0FFh, 1Fh
	db	 20h,0FFh,0FFh, 23h,0FFh, 25h, 26h,0FFh
	db	0FFh, 29h, 2Ah,0FFh, 2Ch,0FFh,0FFh, 2Fh
	db	0FFh, 31h, 32h,0FFh, 34h,0FFh,0FFH, 37h
	db	 38h,0FFH,0FFh, 3Bh,0FFh, 3Dh, 3Eh,0FFh
	db	 40h,0FFh,0FFh, 43h,0FFh, 45h, 46h,0FFh
	db	0FFh, 49h, 4Ah,0FFh, 4Ch,0FFh,0FFh, 4Fh
	db	0FFh, 51h, 52h,0FFh, 54h,0FFh,0FFH, 57h
	db	 58h,0FFH,0FFh, 5Bh,0FFh, 5Dh, 5Eh,0FFh
	db	0FFh, 61h, 62h,0FFh, 64h,0FFh,0FFH, 67h
	db	 68h,0FFH,0FFh, 6Bh,0FFh, 6Dh, 6Eh,0FFh
	db	 70h,0FFh,0FFh, 73h,0FFh, 75h, 76h,0FFh
	db	0FFh, 79h, 7Ah,0FFh, 7Ch,0FFh,0FFh, 7Fh
xlatFop	equ	$			; Translate FROM odd parity ASCII
					; 0xFF values indicate PARITY ERRORs
	db	0FFh, 01h, 02h,0FFh, 04h,0FFh,0FFH, 07h
	db	 08h,0FFH,0FFh, 0Bh,0FFh, 0Dh, 0Eh,0FFh
	db	 10h,0FFh,0FFh, 13h,0FFh, 15h, 16h,0FFh
	db	0FFh, 19h, 1Ah,0FFh, 1Ch,0FFh,0FFh, 1Fh
	db	 20h,0FFh,0FFh, 23h,0FFh, 25h, 26h,0FFh
	db	0FFh, 29h, 2Ah,0FFh, 2Ch,0FFh,0FFh, 2Fh
	db	0FFh, 31h, 32h,0FFh, 34h,0FFh,0FFH, 37h
	db	 38h,0FFH,0FFh, 3Bh,0FFh, 3Dh, 3Eh,0FFh
	db	 40h,0FFh,0FFh, 43h,0FFh, 45h, 46h,0FFh
	db	0FFh, 49h, 4Ah,0FFh, 4Ch,0FFh,0FFh, 4Fh
	db	0FFh, 51h, 52h,0FFh, 54h,0FFh,0FFH, 57h
	db	 58h,0FFH,0FFh, 5Bh,0FFh, 5Dh, 5Eh,0FFh
	db	0FFh, 61h, 62h,0FFh, 64h,0FFh,0FFH, 67h
	db	 68h,0FFH,0FFh, 6Bh,0FFh, 6Dh, 6Eh,0FFh
	db	 70h,0FFh,0FFh, 73h,0FFh, 75h, 76h,0FFh
	db	0FFh, 79h, 7Ah,0FFh, 7Ch,0FFh,0FFh, 7Fh
	db	 00h,0FFh,0FFh, 03h,0FFh, 05h, 06h,0FFh		; End   00-7F
	db	0FFh, 09h, 0Ah,0FFh, 0Ch,0FFh,0FFh, 0Fh		; Start 80-FF
	db	0FFh, 11h, 12h,0FFh, 14h,0FFh,0FFH, 17h
	db	 18h,0FFH,0FFh, 1Bh,0FFh, 1Dh, 1Eh,0FFh
	db	0FFh, 21h, 22h,0FFh, 24h,0FFh,0FFH, 27h
	db	 28h,0FFH,0FFh, 2Bh,0FFh, 2Dh, 2Eh,0FFh
	db	 30h,0FFh,0FFh, 33h,0FFh, 35h, 36h,0FFh
	db	0FFh, 39h, 3Ah,0FFh, 3Ch,0FFh,0FFh, 3Fh
	db	0FFh, 41h, 42h,0FFh, 44h,0FFh,0FFH, 47h
	db	 48h,0FFH,0FFh, 4Bh,0FFh, 4Dh, 4Eh,0FFh
	db	 50h,0FFh,0FFh, 53h,0FFh, 55h, 56h,0FFh
	db	0FFh, 59h, 5Ah,0FFh, 5Ch,0FFh,0FFh, 5Fh
	db	 60h,0FFh,0FFh, 63h,0FFh, 65h, 66h,0FFh
	db	0FFh, 69h, 6Ah,0FFh, 6Ch,0FFh,0FFh, 6Fh
	db	0FFh, 71h, 72h,0FFh, 74h,0FFh,0FFH, 77h
	db	 78h,0FFH,0FFh, 7Bh,0FFh, 7Dh, 7Eh,0FFh
_TEXT	ends
	end
