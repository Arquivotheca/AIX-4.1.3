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
;	"@(#)70	1.4  src/bos/usr/lib/asw/mpqp/duscc.inc, ucodmpqp, bos411, 9428A410j 5/3/91 18:30:05"
;
SCC_0A	EQU	1800h		; SCC 0 Channel A Base Address
SCC_0B	EQU	1840h		; SCC 0 Channel B Base Address
SCC_1A	EQU	1900h		; SCC 1 Channel A Base Address
SCC_1B	EQU	1940h		; SCC 1 Channel B Base Address

SCCREL_D0 EQU	3220h		; SCC 0 Release Interrupt Register
SCCREL_D1 EQU	3221h		; SCC 1 Release Interrupt Register

CHMODE1	EQU	0		; Channel Mode Register 1
CHMODE2	EQU	2		; Channel Mode Register 2
S1	EQU	4		; SYN1/Secondary Address 1
S2	EQU	6		; SYN2/Secondary Address 2
TXPARM	EQU	8		; Transmitter Parameter
TXTIM	EQU	10		; Transmitter Timing
RXPARM	EQU	12		; Receiver Parameter
RXTIM	EQU	14		; Receiver Timing
CTPR_HI	EQU	16		; Counter/Timer Preset, High
CTPR_LO	EQU	18		; Counter/Timer Preset, Low
CTCTL	EQU	20		; Counter/Timer Control
OUTMISC	EQU	22		; Output/Miscellaneous
CT_HI	EQU	24		; Counter/Timer, High (R)
CT_LO	EQU	26		; Counter/Timer, Low (R)
PINCFG	EQU	28		; Pin Configuration
CH_CMD	EQU	30		; Channel Command
TXFIFO	EQU	32		; Tx Fifo (4 bytes, no individual access)
RXFIFO	EQU	40		; Rx Fifo (4 bytes, no individual access)
RXSTAT	EQU	48		; Receiver Status
TRSTAT	EQU	50		; Transmitter/Receiver Status
ICTSTAT	EQU	52		; Input, Counter/Timer Status
GENSTAT	EQU	54		; General Status
INT_EN	EQU	56		; Interrupt Enable
IVECT	EQU	60		; Base Interrupt Vector (Unmodified)
IVECT_M	EQU	124		; Interrupt Vector (Modified)
INTCTL	EQU	62		; Interrupt Control
MR	EQU	126		; Master Reset

; General Status Register Fields

GS_A_RXRDY	EQU	 1h	; Channel A Rx Ready
GS_A_TXRDY	EQU	 2h	; Channel A Tx Ready
GS_A_RTSTS	EQU	 4h	; Channel A Rx/Tx Status/Error
GS_A_ECTSTS	EQU	 8h	; Channel A External, Counter/Timer Status
GS_B_RXRDY	EQU	10h
GS_B_TXRDY	EQU	20h
GS_B_RTSTS	EQU	40h
GS_B_ECTSTS	EQU	80h

; Receiver Status Register

RS_EOM		EQU	80h	; End-Of-Message
RS_B_ABORT	EQU	40h	; Abort detect - BOP
RS_A_LOSTRTS	EQU	40h	; RTS Negated - Async
RS_C_PAD	EQU	40h	; PAD Error - COP
RS_OVERRUN	EQU	20h	; Receiver Overran (DMA Overflow)
RS_SHORT	EQU	10h	; Short Frame detected (SDLC only)
RS_DETECT	EQU	0Ch	; Receiver detect logic (ignored)
				;  Note: cannot be ignored if ASYNC break is
				;  considered significant.

RS_IGNORE	EQU	RS_EOM+RS_DETECT

; Transmitter/Receiver Status Register

TRS_TXEMPTY	EQU	80h	; Transmitter Empty (DMA Underrun)
TRS_CTSUNDER	EQU	40h	; CTS Underrun (Lost CTS)
TRS_EOFRAME	EQU	20h	; Frame Complete.  Who cares?
TRS_SENDACK	EQU	10h	; Break/SOM/Abort Ack
TRS_DPLL	EQU	 8h	; DPLL Error
TRS_RESID	EQU	 7h	; Rx Residual Character Length (0-7 bits)

TRS_IGNORE	EQU	TRS_EOFRAME+TRS_SENDACK+TRS_RESID

ICS_CRUN	EQU	80h	; Counter/Timer Running
ICS_CZERO	EQU	40h	; Counter/Timer went to Zero
ICS_dDCD	EQU	20h	; Delta Carrier Detect
ICS_dCTS	EQU	10h	; Delta Clear to Send
ICS_DCD		EQU	8h	; Carrier Detect line
ICS_CTS		EQU	4h	; Clear to Send line

; Interrupt Enables

IE_DELTA	EQU	80h	; Delta DCD/CTS enable
IE_TXRDY	EQU	40h	; Tx Ready enable
IE_TXERR	EQU	20h	; Transmit status, Tx Empty, CTS Underrun,
				; Frame Complete, Send acknowledgements, PLL
IE_RXRDY	EQU	10h	; Rx Ready enable
IE_RXEOM	EQU	 8h	; EOM detect and Pad errors, ABORT (BOP)
IE_RXERR_1	EQU	 4h	; Rx Overrun, Short Frame detect
IE_RXSTS	EQU	 2h	; Rx SYN/Flag detect, Idle detect
IE_RXERR_0	EQU	 1h	; Rx Framing/CRC/Parity, Residual Lgt errors

;  DUSCC Commands
RESET_TX	EQU	 0h
RESET_TXCRC	EQU	 1h
ENABLE_TX	EQU	 2h
DISABLE_TX	EQU	 3h
TSOM		EQU	 4h
TSOMP		EQU	 5h
TEOM		EQU	 6h
TABRK		EQU	 7h
TDLE		EQU	 8h
EXCLUDE_CRC	EQU	0Dh

RESET_RX	EQU	40h
ENABLE_RX	EQU	42h
DISABLE_RX	EQU	43h

START_CT	EQU	80h
STOP_CT		EQU	81h
PRESET_F	EQU	82h
PRESET_CTP	EQU	83h

DPLL_SEARCH	EQU	0C0h
DPLL_DISABLE	EQU	0C1h
DPLL_MODE_FM	EQU	0C2h
DPLL_MODE_NRZI	EQU	0C3h
