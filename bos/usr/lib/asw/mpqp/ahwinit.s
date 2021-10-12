	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: ahwinit - Initialize the adapter hardware resources
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
;  FUNCTION: Initialize the Port DMA channels, CIO and SCC chips,
;            186 internal timers, Bus Master DMA channel and set
;            translate table protection to secure our code area.
;
;**********************************************************************
;
	TITLE	ahwinit
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)50	1.4  src/bos/usr/lib/asw/mpqp/ahwinit.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:58:50'
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

	extrn	_intreset:NEAR
;
; * START OF CODE * START OF CODE * START OF CODE * START OF CODE *
;
	public	ahwinit
ahwinit	proc	near
	pusha
;
; Setup the SSTIC4 Segment Mapping to provide address checking and interrupt
; us if non-existent memory is ever referenced.
;
	call	SetProt
;
; Initialize the Bus Master DMA Channel.
; Bus Master DMA is multiplexed onto INT0 with INTCOM (channel) interrupts.
;
	call	InitBMDMA
;
; Initialize the Port DMA chip for 4 ports, vector includes status, etc.
;
	call	InitPDMA
;
; Initialize the Serial Communications Controllers (SCC)
;
	call	PorScc0
	call	PorScc1
;
; Initialize the Counter/Timer and Parallel I/O Units (CIO)
;
	call	PorCio0
	call	PorCio1
;
	call	InitTimer
;
	mov	dx,ENREG		; Enables register
	mov	al,00000110b		; Disable X.21, WRPDIAG
	out	dx,al			; Execute

	popa
	ret
ahwinit	endp

;
; The protection feature of the adapter is used only for catching software
; errors and allowing them to be reported on NMI.  The low 512K of memory
; have unlimited access, i.e. R/W.  The next 512K-PROM are protected from
; both reading and writing.
;
SetProt	proc	near
	mov	dx,TRANBASE		; Base of translation registers
	mov	cx,4			; 4 entries with no protection
	xor	ax,ax			;
noprot:					;
	out	dx,ax			; Write translation value
	add	ax,4			; Increment address bit 14
	add	dx,2			;
	loop	noprot			;
	or	ax,TRAN_WP		; Enable Write Protection
	mov	cx,2			; 2 entries with writing disabled
codeseg:				;
	out	dx,ax			; Write translation value
	add	ax,4			; Increment address bit 14
	add	dx,2			;
	loop	codeseg			;
	and	ax,NOT TRAN_WP		; Enable R/W Access
	mov	cx,34			; 34 entries with no protection
dataseg:				;
	out	dx,ax			; Write translation value
	add	ax,4			; Increment address bit 14
	add	dx,2			;
	loop	dataseg			;
	or	ax,TRAN_RWP		; Enable R/W Protection
	mov	cx,22			; Up to but excluding PROM
rwprot:					;
	out	dx,ax			; Write translation value
	add	dx,2			;
	add	ax,4			;
	loop	rwprot			;
	mov	dx,TTPROTON		; Translate Protection Enable
	out	dx,al			; and execute
	ret
SetProt	endp

;
; Initialize the Bus Master DMA channel to a known state, i.e. OFF
;
InitBMDMA proc	near
	IFNDEF	TYP_E
	mov	al,1			;
	out	BMCH1RESET,al		; Write the one bit, start reset
	jmp	$+2			; allow the channel to stabilize
	xor	ax,ax			;
	out	BMCH1RESET,al		; Clear the one bit, end reset
	ENDIF
	ret
InitBMDMA endp

;
; Initialize the Port DMA channels.  Set the operating mode and base vector.
; Even channels (0, 2, 4, ..., E) are Receive channels on MPQP, their odd
; counterparts (1, 3, 5, ..., F) must be Transmit channels.  The I/O addresses
; the channels service are also fixed.  The assignment of DMA channels
; for a particular port is also fixed.
;
;	 Chan	 Serves			 Chan	 Serves
;	(0, 8) = SCC 0 Port A Rx	(1, 9) = SCC 0 Port A Tx
;	(2, A) = SCC 0 Port B Rx	(3, B) = SCC 0 Port B Tx
;	(4, C) = SCC 1 Port A Rx	(5, D) = SCC 1 Port A Tx
;	(6, E) = SCC 1 Port B Rx	(7, F) = SCC 1 Port B Tx
;
InitPDMA proc	near
	IFNDEF	TYP_E
	mov	dx,DMADISABLE		; Port DMA Master Disable register
	mov	al,1			; Write mode: stop DMA immediately
	out	dx,al			;
	ENDIF
	mov	dx,DMAASSIGN		; Port DMA Device configuration reg.
	mov	al,PDMA_RXPRI+PDMA_4P	; Receive channel priority, 4 DMA/P
	out	dx,al			;
	mov	dx,DMAVECTOR		; Port DMA Base interrupt vector
	mov	al,PDMA_VEC		; and value
	out	dx,al			;
;
; On the MPQP, certain hard channel requirements exist.  Setup the STATIC
; fields in each of the 16 channel descriptors.
;
	mov	cx,100h			; Set up loop count
	mov	dx,PD0BASE		; Set up Start Addr
	xor	ax,ax			; Zero ax
pdinit:
	out	dx,ax			; Init all CDT's of Port DMA Chnls to 0
	add	dx,2			;
	loop	pdinit			;
					;
	mov	dx,PD0STAT		; Port DMA Ch 0 Status Register
	mov	cx,16			;
ch_clr:					; Init all DMA Stat Regs to 0
	in	al,dx			;
	inc	dx			;
	loop	ch_clr			;
					;
	IFNDEF	TYP_E
	mov	dx,DMADISABLE		; Port DMA Master Disable register
	xor	al,al			; Write mode: re-enable Port DMA
	out	dx,al			;
	ENDIF
	$$NSEOI				; Just because
	ret
InitPDMA endp

PorScc0	proc	near
	mov	dx,SCC_0A+MR		; Address, Master Reset Register
	out	dx,al			; Reset part, data ignored
	jmp	$+2			; wait...
	in	al,dx			; Terminate reset condition
	mov	dx,SCC_0A+IVECT		; SCC0 Interrupt Vector Register
	mov	al,20h			; Base interrupt #, unmodified
	out	dx,al			;
	mov	dx,SCC_0A+INTCTL	; Interrupt Control Register
	mov	al,10000111b		; Interleaved priority between A/B,
					; Port A, B master interrupt enables,
	out	dx,al			; vector includes status
					;
	mov	dx,SCC_0A+PINCFG	; Port A Pin Configuration Register
	mov	al,20h			; FDX Dual Address DMA, RTS output,
					;  TxClk and RxClk inputs
	out	dx,al			; Setup port A
	add	dx,SCC_0B-SCC_0A	;
	out	dx,al			; Setup port B

	mov	dx,SCC_0A+OUTMISC	; Port A Output/Misc. Register
	mov	al,0E0h			; Use Tx Resid Lgt, RTS Low
	out	dx,al			; Setup port A
	add	dx,SCC_0B-SCC_0A	;
	out	dx,al			; Setup port B
					;
	mov	dx,SCCREL_D0		; Point to SCC Interrupt Release
	out	dx,al			; Re-enable SCC interrupt, data ignored
	ret				;
PorScc0	endp

PorScc1	proc	near
	mov	dx,SCC_1A+MR		; Address, Master Reset Register
	out	dx,al			; Reset part, data ignored
	jmp	$+2			; wait...
	in	al,dx			; Terminate reset condition
	mov	dx,SCC_1A+IVECT		; SCC1 Interrupt Vector Register
	mov	al,28h			; Base interrupt #, unmodified
	out	dx,al			;
	mov	dx,SCC_1A+INTCTL	; Interrupt Control Register
	mov	al,10000111b		; Interleaved priority between A/B,
					; Port A, B master interrupt enables,
	out	dx,al			; vector includes status
					;
	mov	dx,SCC_1A+PINCFG	; Port A Pin Configuration Register
	mov	al,20h			; FDX Dual Address DMA, RTS output,
					;  TxClk and RxClk inputs
	out	dx,al			; Setup port A
	add	dx,SCC_1B-SCC_1A	;
	out	dx,al			; Setup port B

	mov	dx,SCC_1A+OUTMISC	; Port A Output/Misc. Register
	mov	al,0E0h			; Use Tx Resid Lgt, RTS Low
	out	dx,al			; Setup port A
	add	dx,SCC_1B-SCC_1A	;
	out	dx,al			; Setup port B
					;
	mov	dx,SCCREL_D1		; Point to SCC Interrupt Release
	out	dx,al			; Re-enable SCC interrupt, data ignored
	ret				;
PorScc1	endp

;
; All CIO initialization other than reset and interrupt vector initialization
; was added 052289.  Table driven on 052589.
;
c0Areg	dw	CIO_0+P_A_CS,CIO_0+P_A_CS
	dw	CIO_0A+P_MODE,CIO_0A+P_SHAKE,CIO_0A+P_DPPOL,CIO_0A+P_DDIR
	dw	CIO_0A+P_SPCTL,CIO_0A+P_PPOL,CIO_0A+P_PTRAN,CIO_0A+P_PMASK
	dw	CIO_0+P_A_DATA
c0Breg	dw	CIO_0+P_B_CS,CIO_0+P_B_CS
	dw	CIO_0B+P_MODE,CIO_0B+P_SHAKE,CIO_0B+P_DPPOL,CIO_0B+P_DDIR
	dw	CIO_0B+P_SPCTL,CIO_0B+P_PPOL,CIO_0B+P_PTRAN,CIO_0B+P_PMASK
	dw	CIO_0+P_B_DATA
c1Areg	dw	CIO_1+P_A_CS,CIO_1+P_A_CS
	dw	CIO_1A+P_MODE,CIO_1A+P_SHAKE,CIO_1A+P_DPPOL,CIO_1A+P_DDIR
	dw	CIO_1A+P_SPCTL,CIO_1A+P_PPOL,CIO_1A+P_PTRAN,CIO_1A+P_PMASK
	dw	CIO_1+P_A_DATA
c1Breg	dw	CIO_1+P_B_CS,CIO_1+P_B_CS
	dw	CIO_1B+P_MODE,CIO_1B+P_SHAKE,CIO_1B+P_DPPOL,CIO_1B+P_DDIR
	dw	CIO_1B+P_SPCTL,CIO_1B+P_PPOL,CIO_1B+P_PTRAN,CIO_1B+P_PMASK
	dw	CIO_1+P_B_DATA

c0reg	dw	CIO_0+P_A_IV,CIO_0+P_B_IV,CIO_0+CT_IV,CIO_0+MCC,CIO_0+MIC
c1reg	dw	CIO_1+P_A_IV,CIO_1+P_B_IV,CIO_1+CT_IV,CIO_1+MCC,CIO_1+MIC

cioPval	db	CIO_PCS_CLRIE,CIO_PCS_CBOTH
	db	PMS_INIT,PHS_INIT,0,0C0h,0,0,0C0h,0
	db	00011111b

c0val	db	1Ah,1Bh,1Ch
	db	MCC_B_EN+MCC_1_EN+MCC_2_EN+MCC_C3_EN+MCC_A_EN,MIC_MIE
c1val	db	1Dh,1Eh,1Fh
	db	MCC_B_EN+MCC_1_EN+MCC_2_EN+MCC_C3_EN+MCC_A_EN,MIC_MIE
;
PorCio0	proc	near
	mov	dx,CIO_0		; CIO 0 Base address, Master Int. Ctl.
	in	al,dx			;
	mov	al,0FFh			;
	out	dx,al			; Reset the device
	jmp	$+2			; wait...
	xor	al,al			;
	out	dx,al			; Clear reset
					;
	mov	di,OFFSET c0Areg	; Source of I/O Addresses
	mov	si,OFFSET cioPval	; Source of I/O Values
	mov	cx,11			;
lp0A:	mov	dx,[di]			; Get address
	outs	dx,BYTE PTR [si]	; Write byte
	add	di,2			; Increment address pointer
	loop	lp0A			; until count exhausts

	mov	di,OFFSET c0Breg	;
	mov	si,OFFSET cioPval	;
	mov	cx,11			;
lp0B:	mov	dx,[di]			;
	outs	dx,BYTE PTR [si]	;
	add	di,2			;
	loop	lp0B			;

	mov	di,OFFSET c0reg		;
	mov	si,OFFSET c0val		;
	mov	cx,5			;
lp0:	mov	dx,[di]			;
	outs	dx,BYTE PTR [si]	;
	add	di,2			;
	loop	lp0			;

	ret				;
PorCio0	endp

PorCio1	proc	near
	mov	dx,CIO_1		; CIO 1 Base address
	in	al,dx			;
	mov	al,0FFh			;
	out	dx,al			; Reset the device
	jmp	$+2			; wait...
	xor	al,al			;
	out	dx,al			; Clear reset

	mov	di,OFFSET c1Areg	; Source of I/O Addresses
	mov	si,OFFSET cioPval	; Source of I/O Values
	mov	cx,11			;
lp1A:	mov	dx,[di]			; Get address
	outs	dx,BYTE PTR [si]	; Write byte
	add	di,2			; Increment address pointer
	loop	lp1A			; until count exhausts

	mov	di,OFFSET c1Breg	;
	mov	si,OFFSET cioPval	;
	mov	cx,11			;
lp1B:	mov	dx,[di]			;
	outs	dx,BYTE PTR [si]	;
	add	di,2			;
	loop	lp1B			;

	mov	di,OFFSET c1reg		;
	mov	si,OFFSET c1val		;
	mov	cx,5			;
lp1:	mov	dx,[di]			;
	outs	dx,BYTE PTR [si]	;
	add	di,2			;
	loop	lp1			;

	mov	dx,CIO_1+P_C_DPP	; Port C Data Pattern Polarity
	mov	al,00h			; 
	out	dx,al			;
	add	dx,P_C_DD-P_C_DPP	; Move to Data Direction Register
	mov	al,0Fh			; All pins are Inputs
	out	dx,al			;
	ret				;
PorCio1	endp

;
; Initialize the 186 internal timers with Timers 0, 1 stopped and Timer 2
; running as a 10ms (0.01S) prescaler available to Timers 0 and 1.
;
InitTimer proc	near
	mov	dx,TMR2_BASE+TMR_MCTL	; Top of Timer Control Block	051989
	xor	ax,ax			;
	out	dx,ax			; Timer 2 Mode/Control		051989
	sub	dx,4			; Skip Max Count B, not present	051989
	out	dx,ax			; Timer 2 Max Count A		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 2 Count			051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 1 Mode/Control		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 1 Max Count B		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 1 Max Count A		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 1 Count			051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 0 Mode/Control		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 0 Max Count B		051989
	sub	dx,2			;				051989
	out	dx,ax			; Timer 0 Max Count A		051989
					;
	mov	ax,TMR_C_10MS		; Select 10ms interval
	mov	dx,TMR2_BASE+TMR_MAX_A	; Maximum Count A		051989
	out	dx,ax			;				051989
	mov	ax,TMR_M_CONT+TMR_M_INH+TMR_M_EN	;		051989
	mov	dx,TMR2_BASE+TMR_MCTL	; Mode/Control			051989
	out	dx,ax			; Start the timer		051989
					;
	mov	ax,TMR_M_PRE+TMR_M_INT	; Prescale, Interrupt		051889
	mov	dx,TMR0_BASE+TMR_MCTL	;				051889
	out	dx,ax			; Write Mode/Control, halted	051989
	mov	dx,TMR1_BASE+TMR_MCTL	;				051889
	out	dx,ax			; Write Mode/Control, halted	051989
	ret				;
InitTimer endp

;
; *  END OF CODE  *  END OF CODE  *  END OF CODE  *  END OF CODE  *
;
_TEXT	ends
	end
