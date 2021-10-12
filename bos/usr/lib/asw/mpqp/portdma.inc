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
;	"@(#)81	1.3  src/bos/usr/lib/asw/mpqp/portdma.inc, ucodmpqp, bos411, 9428A410j 1/30/91 17:19:31"
;
PDMA_BASE  EQU	8000h		;
ECHO_BASE  EQU	8000h		;

PD0BASE	EQU	8000h		;
PD1BASE	EQU	8020h		;
PD2BASE	EQU	8040h		;
PD3BASE	EQU	8060h		;
PD4BASE	EQU	8080h		;
PD5BASE	EQU	80A0h		;
PD6BASE	EQU	80C0h		;
PD7BASE	EQU	80E0h		;
PD8BASE	EQU	8100h		;
PD9BASE	EQU	8120h		;
PDABASE	EQU	8140h		;
PDBBASE	EQU	8160h		;
PDCBASE	EQU	8180h		;
PDDBASE	EQU	81A0h		;
PDEBASE	EQU	81C0h		;
PDFBASE	EQU	81E0h		;

PDMA_CCW	EQU	0000h	; Channel Control Word
PDMA_CA		EQU	0002h	; Card (Memory) Address, bits 0-15
PDMA_CM		EQU	0004h	; Character Match Bytes (Not Used)
PDMA_IOA	EQU	0006h	; I/O Address, all 16 bits (0-15)
PDMA_TC		EQU	0008h	; Transfer count
PDMA_CAE	EQU	000Ah	; Card (Memory) Address Extension, bits 16-20
PDMA_LAE	EQU	000Ch	; List Address Pointer Extension, bits 16-20
PDMA_LA		EQU	000Eh	; List Address Pointer, bits 0-15

PDMA_CCW_EN	EQU	0001h	; ENABLE/disable channel
PDMA_CCW_RT	EQU	0002h	; TRANSMIT/recieve channel (i.e I/O to memory?)
PDMA_CCW_BW	EQU	0004h	; Byte/Word indicator.  Always 0 = byte on MPQP

PDMA_CCW_LE_TC	EQU	0008h	; List Chaining ENABLE/disable on terminal cnt
PDMA_CCW_LE_CM	EQU	0010h	; List Chaining ENABLE/disable on char match
PDMA_CCW_LE_EOM	EQU	0020h	; List Chaining ENABLE/disable on EOM (DUSCC)
PDMA_CCW_LE	EQU	0038h	; Any List Chaining Enabled?

PDMA_CCW_IE_TC	EQU	0040h	; Interrupt ENABLE/disable on terminal cnt
PDMA_CCW_IE_CM	EQU	0080h	; Interrupt ENABLE/disable on char match
PDMA_CCW_IE_EOM	EQU	0100h	; Interrupt ENABLE/disable on EOM (DUSCC)
PDMA_CCW_IE	EQU	01C0h	; Any Interrupt Enabled?

PDMA_CCW_ST_TC	EQU	0200h	; Stop Channel on terminal cnt
PDMA_CCW_ST_CM	EQU	0400h	; Stop Channel on char match
PDMA_CCW_ST_ACM	EQU	0800h	; Stop Associated Tx Channel on char match
PDMA_CCW_ST_EOM	EQU	1000h	; Stop Channel on EOM (DUSCC)
PDMA_CCW_ST	EQU	1E00h	; Any Stop Channel conditions Enabled?

PDMA_CCW_CM1	EQU	2000h	; Character Match Option 1
PDMA_CCW_CM2	EQU	4000h	; Character Match Option 2
PDAM_CCW_CM	EQU	6000h	; Any Character Match Enabled?

; These are the locations of pertinent fields in both Buffer (B) and Linked
; List (L) chaining elements.

PDMA_B_OTC	EQU	0000h	; Old Transfer Count stored here
PDMA_B_ORS	EQU	0002h	; Old DUSCC Rx Status Byte
PDMA_B_TC	EQU	0004h	; New Transfer Count
PDMA_B_CA	EQU	0006h	; New Card (Memory) Address
PDMA_B_CAE	EQU	0008h	; New Card (Memory) Address Extension
PDMA_B_CCW	EQU	000Ah	; New Channel Control Word

PDMA_L_OTC	EQU	0000h	; Old Transfer Count stored here
PDMA_L_ORS	EQU	0002h	; Old DUSCC Rx Status Byte
PDMA_L_ZERO	EQU	0004h	; Zero indicates Linked List Chaining
PDMA_L_TC	EQU	0006h	; New Transfer Count
PDMA_L_CA	EQU	0008h	; New Card (Memory) Address
PDMA_L_CAE	EQU	000Ah	; New Card (Memory) Address Extension
PDMA_L_CCW	EQU	000Ch	; New Channel Control Word
PDMA_L_LAPE	EQU	000Eh	; New List Address Pointer Extension
PDMA_L_LAP	EQU	0010h	; New List Address Pointer

PDSRB	EQU	8200h		; DMA Interrupt Status Register I/O Base
PD0STAT	EQU	8200h		; Occupy byte addresses 8200 - 820F

PDSR_CM  EQU	01h		; Character Match Bit
PDSR_TC  EQU	02h		; Terminal Count Bit
PDSR_EOM EQU	04h		; End-Of-Message Bit

;
; The Port DMA Channel hardware control registers, et. al.
;

DMAVECTOR	EQU	8210h		; Port DMA Base Interrupt Vector
					; Only High 3, Low 1 bit have effect
PDMA_VEC	EQU	40h		; Vector in our memory map.  Reserves
					; 40,42,...,5C,5E (16 even vectors)

DMAASSIGN	EQU	8212h		; Port DMA Mode Select Reg.

PDMA_1VEC	EQU	4		; Echo uses multiple vectors if bit
					; 4 is clear
PDMA_RXPRI	EQU	2		; Rx takes priority to Tx
PDMA_4P		EQU	1		; Allocate DMA channels 4 per port

DMADISABLE	EQU	8214h		; Port DMA Master Disable Reg.

