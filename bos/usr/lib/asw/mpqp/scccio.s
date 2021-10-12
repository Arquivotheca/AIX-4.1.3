	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: 
;	iSCC0A, iSCC0A_TXR, iSCC0A_RT, iSCC0A_EC : Port 0 SCC ISRs
;	iSCC0B, iSCC0B_TXR, iSCC0B_RT, iSCC0B_EC : Port 1 SCC ISRs
;	iSCC1A, iSCC1A_TXR, iSCC1A_RT, iSCC1A_EC : Port 2 SCC ISRs
;	iSCC1B, iSCC1B_TXR, iSCC1B_RT, iSCC1B_EC : Port 3 SCC ISRs
;	iCIO0A		: Port 0 CIO ISR
;	iCIO0B		: Port 1 CIO ISR
;	iCIO1A		: Port 2 CIO ISR
;	iCIO1B		: Port 3 CIO ISR
;	intX21		: X.21 PAL interrupt service
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
;  FUNCTION: Module contains all communications controller and input/
;	     output control unit interrupts.  Tx, Rx and line status
;	     are all reported upwards to the offlevels from here.
;	     Also, MPQP EIB-specific X.21 Pal pattern detect logic
;	     interrupts are serviced and reset.
;
;**********************************************************************
;
	TITLE	sccserve
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
	.SALL				; Suppress macro expansion code
sccsid	db	'@(#)64        1.7  src/bos/usr/lib/asw/mpqp/scccio.s, ucodmpqp, bos41J, 9519A_all 5/5/95 10:36:38'
	db	0
	.XLIST
	include	define.inc
	include	iapx186.inc
	include	mpqp.inc
	include	portdma.inc
	include	duscc.inc
	include	cio.inc
	.LIST
	PAGE
;
	extrn	_errsts:word		; scheduler work table
	extrn	_estat:BYTE		; eblock status tables
	extrn	_estat0:BYTE,_estat1:BYTE,_estat2:BYTE,_estat3:BYTE
;
;****************************************************************************
; Notes on the SCC Interrupt Service Routines
;
;  1) The Receiver Available (_RX) interrupt is neither used or enabled.
;     Since Port DMA channels are latched to the DUSCCs for RxData, the
;     availability of receive characters in the FIFO is not significant.
;  2) The Transmitter Available (_TX) interrupt is not used.  The Tx Port
;     DMA channels are set to interrupt on counter expiration and provide
;     the "available" mechanism.
;  3) Rx and Tx status conditions, significant line changes included, and
;     channel timer expiration are utilized.  All cases are considered
;     errors.  These interrupts copy DUSCC status registers into port-
;     specific memory (the eblocks) and signal the offlevel with ERR STS.
;****************************************************************************
;
	public	$BEG_SCC,$END_SCC
$BEG_SCC	EQU THIS BYTE

extIntr	proc	near
	in	al,dx			;  8	Read SCC General Status
	and	al,ah			;  3	Mask off insignificant bit
	out	dx,al			;  7	and clear these SCC sources
	mov	dx,bx			;  2	SCC Interrupt Release address
	out	dx,al			;  7	Write SCC Release Interrupt
	$$NSEOI				; 15	CPU non-specific EOInterrupt
	pop	dx			; 10
	pop	bx			; 10
	pop	ax			; 10
	iret				; 28
extIntr	endp
;
iSCCX	macro	IOADDR,IOMASK,RELADDR
	push	ax			; 10
	push	bx			; 10
	push	dx			; 10
	mov	ah,IOMASK		;  3
	mov	bx,RELADDR		;  4	SCC Interrupt Release address
	mov	dx,IOADDR		;  4	SCC Address, General Status Reg.
	jmp	extIntr			; 14
	endm
;
iSCCTXR	macro	PORT,IOADDR		; Tx Ready Interrupt
	pusha				;
	mov	cx,PORT			;
	mov	dx,IOADDR		;  4	SCC Base Address
	jmp	iTXRDY			;
	endm
;
iSCCRT	macro	NUM,SCCBASE		; Rx/Tx Status Int. macro
	pusha				; 36
	mov	cl,NUM			;  4
	mov	dx,SCCBASE		;  4
	jmp	SCCRT			;
	endm
;
iSCCEC	macro	NUM,SCCBASE		; External and C/T Status Int. macro
	pusha				; 36
	mov	cx,NUM			;  4
	mov	dx,SCCBASE		;  4
	jmp	SCCEC			;
	endm
;
;****************************************************************************
; 20 - 27 : SCC 0 Interrupt
;
	public	iSCC0A,iSCC0A_TXR,iSCC0A_RT,iSCC0A_EC
	public	iSCC0B,iSCC0B_TXR,iSCC0B_RT,iSCC0B_EC
;
iSCC0A	proc	far
	iSCCX	SCC_0A+GENSTAT,03h,SCCREL_D0
iSCC0A	endp
;
iSCC0A_TXR proc	far
	iSCCTXR	00h,SCC_0A
iSCC0A_TXR endp
;
iSCC0A_RT proc	far
	iSCCRT	00h,SCC_0A
iSCC0A_RT endp
;
iSCC0A_EC proc	far
	iSCCEC	00h,SCC_0A
iSCC0A_EC endp
;
iSCC0B	proc	far
	iSCCX	SCC_0B+GENSTAT,30h,SCCREL_D0
iSCC0B	endp
;
iSCC0B_TXR proc	far
	iSCCTXR	01h,SCC_0B
iSCC0B_TXR endp
;
iSCC0B_RT proc	far
	iSCCRT	01h,SCC_0B
iSCC0B_RT endp
;
iSCC0B_EC proc	far
	iSCCEC	01h,SCC_0B
iSCC0B_EC endp
;
;****************************************************************************
; 28 - 2F : SCC 1 Interrupt
;
	public	iSCC1A,iSCC1A_TXR,iSCC1A_RT,iSCC1A_EC
	public	iSCC1B,iSCC1B_TXR,iSCC1B_RT,iSCC1B_EC
;
iSCC1A	proc	far
	iSCCX	SCC_1A+GENSTAT,03h,SCCREL_D1
iSCC1A	endp
;
iSCC1A_TXR proc	far
	iSCCTXR	02h,SCC_1A
iSCC1A_TXR endp
;
iSCC1A_RT proc	far
	iSCCRT	02h,SCC_1A
iSCC1A_RT endp
;
iSCC1A_EC proc	far
	iSCCEC	02h,SCC_1A
iSCC1A_EC endp
;
iSCC1B	proc	far
	iSCCX	SCC_1B+GENSTAT,30h,SCCREL_D1
iSCC1B	endp
;
iSCC1B_TXR proc	far
	iSCCTXR	03h,SCC_1B
iSCC1B_TXR endp
;
iSCC1B_RT proc	far
	iSCCRT	03h,SCC_1B
iSCC1B_RT endp
;
iSCC1B_EC proc	far
	iSCCEC	03h,SCC_1B
iSCC1B_EC endp
;
;*****
; Interrupt common header
;*****
scc_com:
	pop	bx			;	Return address		083089
	push	ds			;				083089
	push	es			;				083089
	mov	ax,LOADSEG		;				083089
	mov	ds,ax			;				083089
	mov	es,ax			;				083089
					;
	mov	si,OFFSET _estat	;	Base of eblock table
	mov	al,cl			;	port number
	xor	ah,ah			;
	mov	di,ax			;  2	Save port number	082989
	shl	ax,3			;	times sizeof estat (8)
	add	si,ax			;	Port estat pointer
					;
	jmp	bx			;	Return to caller	083089

;********************************************************************** ******
	extrn	_txioa:WORD,_txftype:BYTE
	public	iTXRDY
iTXRDY	proc	near
	call	scc_com			;	setup stuff
;	test	_txftype[di],FT_B	; 10	Better be a BiSync one	101089
;	jnz	txRdyOk			;  4/13	Jumps for BiSync	101089
txRdyOk:				;
	jmp	scc_exit		; 14	Exit without scheduling
iTXRDY	endp
;
;****************************************************************************
; The Rx/Tx Status Change SCC Interrupts come here.  We copy out signif.
; SCC registers and signal the off-level.
;
	extrn	_txwork:WORD,_txccw:WORD
	extrn	_trsMask:BYTE,_rsMask:BYTE
	extrn	_SCCbase:WORD
	extrn   _get_rsr:NEAR
SCCRT	proc	near
	call	scc_com			;	setup E_STAT pointer, read GSR
;
; Even channels are on SCC Ports `A', Odd on Ports `B'.  From this, the
; Rx/Tx Status bit mask can be determined as can the clearing write mask.
;
	add	dx,RXSTAT		;  4	Move to the Rx Status Reg.
	in	al,dx			;  8	Get actual RSR
	out	dx,al			;  7	clear interrupts in actual RSR
	mov	ah,al			;	Move RXSTAT to ah

	add	dx,TRSTAT-RXSTAT	;  4	Move to Tx & Rx Status Reg.
	in	al,dx			;  8
	out	dx,al			;  7	clear any interrupt
	sub	dx,TRSTAT		;  4
					;
	and	ah,_rsMask[di]		; 10	Ignore bits, Rx stat
	and	al,_trsMask[di]		; 10	Ignore bits, Tx/Rx stat
					;
	mov	bx,ax			;  3	Save Rx, Tx/Rx status	082989
	or	ax,ax			;  3	Anything concerning us?	082989
	jnz	test_trs		;  4/13	There are signif. bits	111789
	jmp	scc_exit		; 14	Exit without scheduling
test_trs:
	or	bl,bl			;	Anything in TSR?	020591
	jz	ErrSched		;	Save status reg bits	020591
pro_sel:				;	Proto-specific branch	111789
	mov	al,_txftype[di]		;  8	Fetch the frame type	111789
	mov	ch,al			;  2	Save for safety...	111789
	test	ch,FT_B			;  4	Is it a BiSync frame?	111789
	jz	TrySOM			; 13/4	Absolutely not.  Good.	111789
	jmp	Bsc_tsi			; 14	Unfortunately so.	111789
;
; This ISR was taught to start the Port DMA for transmit operations when the
; Tx SOM Ack interrupt (Tx/Rx Status) arrives.
;
TrySOM:					;
	test	bl,TRS_SENDACK		;  4	SOM Ack?		090889
	jz	TryEOF			; 13/4	No, try Tx/Rx stat EOF	090889
	call	SOMserv			;
;
; When the txwork scheduling was moved from Port DMA terminal count to
; error/status Frame Complete (Tx/Rx Status Register), the following code
; was added so that the ASW will not run errsts if only txwork need run.
;
TryEOF:					;	Does TRS indicate EOF?
	mov	dx,_SCCbase[di]		;    Get SCC base address
	add	dx,CH_CMD;		;    Move to channel command reg
	mov	al,RESET_TX		;    Disable transmitter
	test	bl,TRS_EOFRAME		;  4	Frame Complete?		082989
	jz	ErrSched		;  4/13	No, continue error path	082989
	call	EOFserv			;
ErrSched:				;
	or	bx,bx			;  3	Compares RS & TRS	090889
	jz	scc_exit		;  4/13	Yes, no error/status	082989
	mov	ax,bx			;  2	Recover Rx, Tx/Rx stat	090889
	or	E_RSR[si],ah		;  9	Save for error/status
	or	E_TRSR[si],al		;  9	 offlevel processing
	or	WORD PTR E_TYPE[si],RT_SI	; 16  Tx/Rx Status Int.
	jmp	scc_end			; 14	Exit, schedule offlevel
SCCRT	endp

;
; To clear the interrupt from an SCC, you must:
;  1) Write the bits back to the status register (already done when read)
;  2) At Least 2 instructions later, write the SCCRELINT location to
;     free the interrupt daisy chaining
;  3) Issue the 80186 Non-Specific EOI
;
scc_end:				;	Signal the off-level
	mov	ax,1			;
	shl	ax,cl			;
	or	_errsts,ax		;	Schedule ERR_STS offlevel
scc_exit:				;	Interrupt was insignificant
	mov	al,cl			;	Port number
	xor	ah,ah			;
	shr	ax,1			;	Port number to SCC Number
	mov	dx,SCCREL_D0		;	Base, SCCRELINT in I/O space
	add	dx,ax			;
	out	dx,al			;	Re-enable Echo daisy-chain
	pop	es			;				083089
	pop	ds			;				083089
	$$NSEOI				; 15
	popa				; 51
	iret				; 28

;********************************************************************** ******
SOMserv	proc	near			;	NEAR proc for near ret	111789
	test	ch,FT_X			;	X.21 frame?
	jz	txtime			;	jump to test timing reg
	mov	dx,CIO_0+P_A_DATA	;	Get port 0 cio
	in	al,dx			;	get cio contents
	or	al,CIO_S_422		;	Turn on 422 drivers
	out	dx,al			;	write out to CIO
	jmp	startDMA		;	jmp to finish SOM processing
txtime:
	shl	di,1			;  2	Convert to Word index	
	mov	dx,_SCCbase[di]		;	Point to scc base
	add	dx,TXTIM		;	Add tx timing reg offset
	in	al,dx			;	Get tx timing reg
	and	al,0fh			;	Get low nibble
	or	al,al			;	Check if internally clocked
	jz	SOMexit			;	jmp if external
startDMA:
	mov	dx,_txioa[di]		;	Set up DMA io addr
	in	ax,dx			;	Read current ccw
	or	ax,PDMA_CCW_EN		;  4	Turn on Enable bit	
	mov	_txccw[di],ax		;  9	Save in global memory	
	out	dx,ax			;  7	Start the DMA channel	
SOMexit:				;
	and	bl,NOT TRS_SENDACK	;  4	Clear the SOM Ack bit	
	mov	dx,_SCCbase[di]		; 12
	shr	di,1			;  2
	ret				;
SOMserv	endp				;				111789
;********************************************************************** ******
EOFserv	proc	near			;	NEAR proc for near ret	111789
	mov	ax,1			;  4				
	mov	cx,di			;  2	Get the port number	
	shl	ax,cl			;  5-8  Make scheduler OR mask	
	or	WORD PTR _txwork,ax	; 10	Indicate offlevel work	
	and	bl,NOT TRS_EOFRAME	;  4	Clear End-Of-Frame bit	
	and	bl,NOT TRS_TXEMPTY	;  4	Ditto Tx Empty flag	
	ret				;
EOFserv	endp				;				111789

;********************************************************************* *********
;	
;	BISYNC Transmit Status Interrupt Handler			15AUG90
;
;	This interrupt handler is used exclusively for handling
;	events due to changes in the DUSCC receiver/transmitter 
;	status registers that occur at the beginning and end of 
;	Bisync frame transmissions.
;	

Bsc_tsi proc	near
	mov	cx,di			;    Save the port number
	mov	ch,_txftype[di]		;    Fetch the frame type
	shl	di,1			;    Convert to word index

;-------------------------------------------------------------------------------
;
;	Start of Message Processing:  The DUSCC has sent the first 
;	two SYN's and is ready for transmit data.  The DMA channel
;	control word (for this port) in Echo is modified to enable 
;	DMA -- this causes Echo to begin writing transmit data to 
;	the DUSCC transmit FIFO.
;
	test	bl,TRS_SENDACK		;    Start of Message Ack?
	jz	Bsc_eof			;    If not, goto EOF check.
					;    Enable Transmit DMA:
	mov	dx,_txioa[di]		;    	Port DMA channel base
	in	ax,dx			;    	Read channel control word
	or	ax,PDMA_CCW_EN		;    	turn on Enable bit
	mov	_txccw[di],ax		;    	Save in global memory
	out	dx,ax			;    	Start the DMA channel
	and	bl,NOT TRS_SENDACK	;    Clear the SOM ack bit
	jmp	Bsc_exit		;    End of SOM processing

;-------------------------------------------------------------------------------
;
;	End of Frame Processing:  The DUSCC has begun to transmit the
;	end of message sequence (the CRC or LRC for Bisync).  The DMA
;	channel must be disabled at this point to prevent garbage
;	characters from going to the TX FIFO (this should be done
;	automatically by Echo at transfer count, but it delays the
;	disable because of list-chaining operations).
;
Bsc_eof:
	test	bl,TRS_EOFRAME		;    End of frame interrupt?
	jz	Bsc_exit		;    if not, exit.
					;    Disable Transmit DMA:
	mov	dx,_txioa[di]		;    	Get port DMA channel base
	in	ax,dx			;    	Read Channel Control Word
	and	ax,NOT PDMA_CCW_EN	;    	Turn off enable DMA bit
	out	dx,ax			;    	Write back to DMA channel
	mov	_txccw[di],ax		;    	Save in global memory
;
;	Since this transmit is finished (as far as the adapter SW is
;	concerned), the next transmit can be started, so the Scheduler
;	must be notified of more transmit work to do:
;
					;    Schedule Transmit Offlevel:
	mov	ax,1			; 	Make scheduler OR mask
	shl	ax,cl			;	from port number.
	or	WORD PTR _txwork,ax	;	Indicate offlevel work
	and	bl,NOT TRS_EOFRAME	;    Clear end-of-frame flag
	and	bl,NOT TRS_TXEMPTY	;    Clear TX-empty flag
;
;	If the line is to be turned around for receive (i.e., this was
;	a non-ITB frame), restore the DUSCC back to  Bisync mode, with
;	CRC-16 frame check sequence for EBCDIC, LRC-8 for ASCII.  If in
;	ASCII mode, restore odd parity.  The counterpart to this code
;	resides in bsc_xmit_frame (bscoffl.c), which places the DUSCC in
;	COP_DUAL_SYN mode for transmits.  The bit values for these 
;	registers can be found in the Signetics SCN26562 documentation.
;
	test	ch,FT_B_ITB		;    Was this an ITB frame?
	jnz	Bsc_exit		;    if so, skip DUSCC reconfig
	mov	dx,_SCCbase[di]		;    Get SCC base address
	add	dx,CH_CMD;		;    Move to channel command reg
	mov	al,DISABLE_TX		;    Disable transmitter
	out	dx,al			;

	sub	dx,CH_CMD-TXPARM;	;    Move to TPR register
	in	al,dx			;    Read its contents
	and	al,0DFh			;    Turn off idle SYN's
	out	dx,al			;    Write to TPR

	sub	dx,TXPARM-CHMODE2	;    Move to CMR2 register
	in	al,dx			;    Read its contents
					;    Select Frame Check Sequence:
	and	al,0F8h			;        Clear low three bits
	test	ch,FT_B_ASCII		;        ASCII mode?
	jz	Bsc_crc			; 	 If not, use CRC    
	or	al,02h			;        If so, use LRC-8 preset 0
	jmp	Bsc_fcs 		;
Bsc_crc:				;
	or	al,04h			;        CRC-16 preset 0
Bsc_fcs:
	out	dx,al			;        Write FCS to CMR2

	sub	dx,CHMODE2-CHMODE1	;    Move to CMR1 register
	in	al,dx			;    Read its contents
	and	al,0F8h			;    Clear low three bits
	or	al,05h			;    Restore to Bisync mode
	test	ch,FT_B_ASCII		;    Ascii mode?
	jz	Bsc_mode		;    If not, that's all
	or	al,20h			;    else, restore to odd parity
Bsc_mode:
	out	dx,al			;    Write mode to CMR1
	
	add	dx,CH_CMD-CHMODE1;	;    Move to channel command reg
	mov	al,RESET_RX		;    Reset receiver 
	out	dx,al			;
	mov	al,ENABLE_RX		;    Then enable
	out	dx,al			;
Bsc_exit:
	mov	dx,_SCCbase[di]		;    Restore DUSCC base address
	shr	di,1			;    Restore port no. to byte index
	jmp	ErrSched		; 
Bsc_tsi	endp

;
;****************************************************************************
; The External and Counter/Timer Interrupts come here.  We copy out signif.
; SCC registers and signal the off-level.
;
	extrn	_tscc_val:WORD
SCCEC	proc	near
	call	scc_com			;
	add	dx,ICTSTAT		;  4	Input and C/T Status
	in	al,dx			;  8
	out	dx,al			;  7	Clear any interrupt
	mov	bl,al			;  2	Save register value	110989
	test	al,ICS_CZERO		;  3	Counter reached Zero?	110989
	jz	LCint			; 13/4	No, check line changes	110989
	shl	di,1			;  2	Convert to word index	110989
	mov	ax,_tscc_val[di]	;  8	Get 1/10 second count	110989
	or	ax,ax			;  3	Running?		110989
	jz	ClrTmr			; 13/4	No, extraneous intr.	110989
	dec	ax			;  3	Decrease by 1/10 second	110989
	mov	_tscc_val[di],ax	;  9	Store decremented count	111489
	jnz	ClrTmr			; 13/4	Continue if not 0	110989
	sub	dx,ICTSTAT-CH_CMD	;  4	Move to channel command	110989
	mov	al,STOP_CT		;  3	And stop the counter	110989
	out	dx,al			;  7	 via SCC command	110989
ECTsig:					;
	mov	E_ICTSR[si],bl		;  9	Save for the offlevel	110989
	or	WORD PTR E_TYPE[si],EC_SI ; 16
	jmp	scc_end			; 14
ClrTmr:
	and	bl, NOT ICS_CZERO	;	if tscc_val not 0, clr sts
LCint:					;	Line change interrupt?	111489
	test	bl,ICS_dDCD OR ICS_dCTS	;  3	Either line changed?	110989
	jnz	ECTsig			; 13/4	Yes, save status, sched	110989
	jmp	scc_exit		; 14	Leave without sceduling	111489
SCCEC	endp
;
$END_SCC	EQU THIS BYTE
;
;****************************************************************************
;
	public	$BEG_CIO,$END_CIO
$BEG_CIO	EQU THIS BYTE
	.LALL				; List macro expansion
;
; CIO Data (Line status) Interrupts and other similar nonsense.  Notice that
; the Port C, CIO1 interrupt is not fielded here.
;
; Entry:  DX = CIO Base Address
;         AL = Port Number
;
CIOD	proc	near
	xor	ah,ah			;	Clear high half of port number
	push	cx			; 10
	push	si			;
	mov	cl,al			;	Save the port number
	mov	si,1			;	Prepare scheduler level OR mask
	shl	si,cl			;
	or	_errsts,si		; 16	Indicate work on level errsts
					;
	test	al,1			;	Odd (CIO Channel B) ?
	jz	cioEven			;	No.
	add	dx,2			;	Yes, add 2 byte offset
cioEven:
	shl	ax,3			;  8	"Multiply" by size of an estat
	mov	si,OFFSET _estat	;  4	estat table base address
	add	si,ax			;  3
	or	WORD PTR E_TYPE[si],CI_SI	; 12
					;
	add	dx,P_A_DATA		;  4	Add offset to Data register
	in	al,dx			;  8	Get the data register
	mov	E_DREG[si],al		;  9	Put in the estat block
					;
	sub	dx,P_A_DATA-P_A_CS	;	Move to Command/Status
	in	al,dx			;				072689
	jmp	$+2			;				072889
	mov	al,CIO_PCS_CBOTH	;	Clear IUS and IP at once
	out	dx,al			;
					;
	mov	al,cl			;	Recover long lost port number
	mov	dx,SCCREL_C0		;
	and	al,2			;	Which CIO, 0,1 = 0; 2,3 = 1
	jz	lowCio			;	Is CIO zero, Use SCCREL_C0
	inc	dx			;	Is CIO one, Use SCCREL_C1
lowCio:					;
	out	dx,al			;	Write Release Interrupt
	pop	si			; 10
	pop	cx			; 10
	jmp	$+2			;	Wait for the stupid chip
	$$NSEOI				; 15
	pop	dx			; 10
	pop	ax			; 10
	iret				; 28
CIOD	endp
;
;****************************************************************************
; 1C - CIO 0 Port A
;
	public	iCIO0A
iCIO0A	proc	far
	push	ax
	push	dx
	mov	dx,CIO_0
	mov	al,0
	jmp	CIOD
iCIO0A	endp
;****************************************************************************
; 1D - CIO 0 Port B
;
	public	iCIO0B
iCIO0B	proc	far
	push	ax
	push	dx
	mov	dx,CIO_0
	mov	al,1
	jmp	CIOD
iCIO0B	endp
;****************************************************************************
; 1E - CIO 1 Port A
;
	public	iCIO1A
iCIO1A	proc	far
	push	ax
	push	dx
	mov	dx,CIO_1
	mov	al,2
	jmp	CIOD
iCIO1A	endp
;****************************************************************************
; 1F - CIO 1 Port B
;
	public	iCIO1B
iCIO1B	proc	far
	push	ax
	push	dx
	mov	dx,CIO_1
	mov	al,3
	jmp	CIOD
iCIO1B	endp
;
;****************************************************************************
; 7C - CIO 1 Port C, X.21 State and Line Change Interrupt
;
	extrn	_x21_Skip:WORD
	public	intX21
intX21	proc	near
	pusha
	push	di
					;
	mov	ax,_x21_Skip		;	Is this the interrupt	081589
	or	ax,ax			;	we always get starting	081589
	jz	int_ok			;	the X.21 PALs?		081589
	mov	_x21_Skip,0		;	Yes, it is.  Bye now.	081589
	mov	di,0400h		;	Setup to init x.21 int cnt
	mov	al,0			;
	mov	ss:BYTE PTR [di],al	;
	mov	dx,CIO_1+P_B_DATA	;	Not done yet.  First    110689
	in	al,dx			;	PAL interrupt turn	110689	
	and	al,CIO_C_BLK		;	off blocking bit	110689
	out	dx,al			;				110689
	jmp	bad_int			;				081589
int_ok:					;				081589
	mov	dx,CIO_1+P_C_DATA	;
	in	al,dx			;	Retrieve PAL status
	mov	si,OFFSET _estat	;	estat table base addr
	or	WORD PTR E_TYPE[si],X21_SI	;
	mov	E_X21[si],al		;	Put in the estat block
	or	_errsts,1		;	Indicate work on errsts
	mov	di,0400h		;	Save count of x.21 ints
	mov	al,ss:BYTE PTR [di]	;	in 0:0400 for diagnostics
	inc	al			;	TU's.
	mov	ss:BYTE PTR [di],al	;
bad_int:				;
	mov	dx,SCCRELINT2		;	Release interrupt daisy	100289
	out	dx,al			;	chaining in Echo.  This	100289
					;	re-enables the PALs	100289
	$$NSEOI				;	Issue 186 EOI
	pop	di			;
	popa				;
	iret				;
intX21	endp
;
$END_CIO	EQU THIS BYTE

_TEXT	ends
	end
