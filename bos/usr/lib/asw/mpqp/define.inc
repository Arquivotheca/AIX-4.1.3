	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
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
;	"@(#)69	1.5  src/bos/usr/lib/asw/mpqp/define.inc, ucodmpqp, bos411, 9428A410j 5/26/92 11:18:07"
;
;  Common length definitions.  Many of the regions of shared memory have
; their lengths defined here.
;
NUM_PORT EQU	4

;  The number of priority levels the scheduler manages

NUM_LEV	EQU	7

;  Queue structure sizes

QHDR	EQU	4		; Byte and word queue header sizes
QLGT	EQU	48		; Queue length in items for a single queue
				; All single length queues have this item lgt
Q2LGT	EQU	(QLGT*2)	; Queue length in items for a double queue
NUM_CMD	EQU	48		; Number of commands = number of Tx Buffers

;  Offsets to pertinent fields inside regular queue structures
;
LGT	EQU	0
DNE	EQU	1
OUTP	EQU	2
INP	EQU	3
QUEUE	EQU	4

;  Since the Rx and Tx buffer sizes and base addresses are subject to change,
; this information was placed into the shared data area at init. time.
;
RXNBUF	EQU	48		; Number of receive buffers
RXBSIZE	EQU	4096		; Receive data buffer size
RXPARA	EQU	2000H		; Receive buffer base paragraph
TXNBUF	EQU	48		; Number of transmit buffers
TXBSIZE	EQU	4096		; Transmit data buffer size
TXPARA	EQU	5000H		; Transmit buffer base paragraph
;
;  Transmit extensions are used for mapping of Bisync data on 1 Meg
;  and 2 Meg cards, allowing full 4096-byte frames to be processed.
;  At least 8631 bytes is needed to accomodate maximum character
;  insertion for a 4096 byte frame.  Since eight extensions will not
;  fit in a single segment, four are placed in segment 8 and four are
;  in segment 9; hence, each extension is 16K in size.
;
TXESIZE EQU	4000H		; Transmit extension size
TXEPARA EQU	8000H		; Transmit extension base paragraph

RXHDR	EQU	10		; Each Rx Buffer has this size header

LOADSEG	EQU	1000h		; Load Segment (ip starts at 100h)
;
; Port Work masks to OR or ~AND into a scheduler level
;
MASK_P0	EQU	01
MASK_P1	EQU	02
MASK_P2	EQU	04
MASK_P3	EQU	08

; Each and every e_block is composed of the following fields

E_TYPE	EQU	0			; Type of interrupt causing eblock
E_RSR	EQU	2			; SCC Rx Status Register, TYPE = SCC
E_TRSR	EQU	3			; SCC Tx/Rx Status Register, TYPE = SCC
E_ICTSR	EQU	4			; SCC Input, C/T Status Reg.
E_DREG	EQU	5			; CIO Data Register, TYPE = CIO
E_X21	EQU	6			; X.21 PAL interrupt data
;
RT_SI	EQU	1			; eblock type: Rx, Rx/Tx Status
EC_SI	EQU	2			; eblock type: External, C/T Status
CI_SI	EQU	4			; eblock type: CIO...
X21_SI	EQU	8			; eblock type: X.21 Pal
RX_SI	EQU	20h			; eblock type: RxRdy interrupt

FAILSAFE EQU	4000h			; eblock type: failsafe timer (1)
TIMER_P	EQU	8000h			; eblock type: timer (0)
;
; The global "Frame Type" storage bytes, _txftype, are restricted to the
; following values.
;
FT_GENERIC	EQU	00h	; Frame type indeterminate/insignificant

FT_X		EQU	01h	; X.21 data frame
FT_B_ASCII	EQU	02h	; BiSync frame has ASCII data
FT_B_EXT	EQU	04h	; BiSync frame with external clocking.
FT_B_ITB	EQU	08h	; BiSync frame ends with ITB character
FT_B		EQU	10h	; A BiSync frame at all.

FT_A		EQU	20h	; Async data frame
FT_S		EQU	40h	; SDLC data frame
FT_FINAL	EQU	80h	; Final data frame
;
