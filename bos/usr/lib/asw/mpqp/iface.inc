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
;	"@(#)73	1.3  src/bos/usr/lib/asw/mpqp/iface.inc, ucodmpqp, bos411, 9428A410j 1/30/91 17:17:47"
;
RQE_TX_ACK	EQU	0000h		; Transmit Complete RQE
RQE_TX_NAK	EQU	8000h		; Transmit Error RQE
RQE_RX_ACK	EQU	1000h		; Receive Data Available RQE
RQE_RX_NAK	EQU	9000h		; Receive Data Error RQE
RQE_ST_RSP	EQU	2000h		; Port Status Response RQE (Solic.)
RQE_ST_ERR	EQU	0A000h		; Port Status Response RQE (Unsol.)

RQE_AD_ERR	EQU	6000h		; Adapter Error Response RQE, No EDRR
RQE_AD_EDR	EQU	0E000h		; Adapter Error Response RQE, EDRR
RQE_DI_RSP	EQU	7000h		; Diagnostic Response RQE, No EDRR
RQE_DI_EDR	EQU	0F000h		; Diagnostic Response RQE, EDRR

RQE_PORT0	EQU	0000h		; Port 0 indicator
RQE_PORT1	EQU	0100h		; Port 1 indicator
RQE_PORT2	EQU	0200h		; Port 2 indicator
RQE_PORT3	EQU	0300h		; Port 3 indicator
;
; log_error call parameters
;
RXDATA_LOST	EQU	1700h		;
					;
BUSMSTR_RNG	EQU	1800h		; Bus Master DMA start attempted
					; while Bus Master DMA in progress.

; Response Queue Elements for NMI Interrupts.  The contents of the second
; 16 bit word are dependent on the generating condition.

NMI_CHRESET	EQU	01E0h		; Channel Reset
NMI_WATCHDOG	EQU	02E0h		; Watchdog timer (disabled) expired
NMI_PARITY	EQU	03E0h		; On board Parity error
NMI_UCPARITY	EQU	04E0h		; Host access Parity error
NMI_HOSTNMI	EQU	05E0h		; Host (COMREG) Generated NMI
NMI_TTPROT	EQU	06E0h		; Translate Table Protection error
NMI_ADDRCMP	EQU	07E0h		; Diagnostic Address Compare NMI
NMI_CHCHECK	EQU	08E0h		; External Channel Check
NMI_SFDBKRTN	EQU	09E0h		; Missing -SFDBKRTN during BM DMA
NMI_LOSTARB	EQU	0AE0h		; Host changed ARB via POS Access
NMI_LOSTCHAN	EQU	0BE0h		; Host either changed sleep/ENABLE or
					; expelled us via ARB/-GNT

INT_ILLEGAL	EQU	0A0E0h		; RQE is E0Ax with x=interrupt no.
