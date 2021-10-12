
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
;	"@(#)76	1.5  src/bos/usr/lib/asw/mpqp/mpqp.inc, ucodmpqp, bos411, 9428A410j 4/16/91 14:40:09"

; How many bytes are allowed on a DMA read operation from the DUSCC?
; Well, start with 4096 - 6 bytes to reserve storage for the header.

RX_LGT	EQU	4096-6

;
; The offsets for items inside a Command Block
;
CMDTYP	EQU	0
RSVD1	EQU	2
CMDPORT	EQU	4
RSVD2	EQU	5
CMDLGT	EQU	6
CMDCTL	EQU	8

;
; ACMDREG values
;

ACQINT	EQU	0
TXFLMT	EQU	1

TXFL_THR	EQU	4		; Threshhold for TxFL !empty interrupt

;
; Transmit State Machine states
;
TXINT	EQU	40h

;****************************
; Interrupt service constants
;
	 IFDEF	TYP_E
GEOI     EQU	16h			; SSTIC Interrupt Clear location
	 ENDIF
;****************************
; Hardware resources
;
LOCREG0		EQU	0000h		;
LOCREG1		EQU	0002h		;
;****************************
INITREG0	EQU	0004h		;

IR0_SLEEP	EQU	08h		; SLEEP/enable adapter to channel
IR0_CHCKTYP	EQU	80h		; SYNCHRONOUS/continuous Channel Check
;****************************
INITREG1	EQU	0006h		;

IR1_PAREN	EQU	04h		; Channel Data Parity ENABLE/disable
IR1_MEM_LO	EQU	08h		; Encoded Memory Size, Low bit
IR1_DMAPGEN	EQU	10h		; DMAPG Window mapping ENABLE/disable
IR1_MEM_HI	EQU	20h		; Encoded Memory Size, High bit
IR1_PROMRDY	EQU	40h		; PROM READY/not ready, POST complete
IR1_REFRESH	EQU	80h		; DOUBLE/single refresh time 13.4/6.7
;****************************
; NMI Status and Mask register equates
;

NMIMASK		EQU	0008h		; NMI Interrupt Mask Register
NMISTAT		EQU	000Ah		; NMI Interrupt Status Register

NMI_CRST	EQU	0001h		; Channel Reset
NMI_WD		EQU	0002h		; Watchdog Timer
NMI_PE		EQU	0004h		; Parity Error
NMI_PCC		EQU	0008h		; uC I/O Channel Check
NMI_NC		EQU	0010h		; uC Generated NMI
NMI_PROT	EQU	0020h		; W, R/W Protect Error

NMI_M_DG	EQU	0040h		; (Mask) Degate RAM from uC
NMI_S_IE	EQU	0040h		; (Stat) uC Interrupts Enabled
NMI_M_FP	EQU	0080h		; (Mask) Force Bad Memory Parity
NMI_S_IP	EQU	0080h		; (Stat) uC Interrupt Pending

NMI_DAC		EQU	0100h		; Diagnostic Address Comp.
NMI_BMCC	EQU	0200h		; Bus Master I/O Channel Check
NMI_SFDBK	EQU	0400h		; Missing -SFDBKRTN
NMI_PAREN	EQU	0800h		; Parity Latch Enable
NMI_ARBLEV	EQU	1000h		; Change in Arbitration Levels
NMI_CHLOST	EQU	2000h		; Microchannel lost via ARB/-GNT

NMI_M_OFF	EQU	373eh	; Allowed NMIs when NMI ISR runs.	083189
				; Allow channel reset NMI only.		030691
NMI_M_ON	EQU	0d02h	; NMIs allowed during normal operation.	030691
				; Don't allow card selected feedback indicator
				; NMI (CSFDBKRTN) because of H/W bug.
NMI_S_MASK	EQU	3b3fh	; NMIs allowed intNMI services.		030691
				; Don't check for CSFDBKRTN since it is bogus 
				; NMI and bus master DMA still completes
				; even if this NMI is generated

;****************************

RICPAR0		EQU	000Ch		;
RICPAR1		EQU	000Eh		;
RICPAR2		EQU	0010h		;
;****************************
TASKREG		EQU	0012h		;

TREG_ARQ	EQU	00h		; Adapter Response Queue went non-empty
TREG_TXFREE	EQU	01h		; Tx Free List went non-empty
TREG_ARQOVER	EQU	02h		; Adapter Response Queue Overflowed
TREG_DEBUG	EQU	03h		; Special debugger synchronization
TREG_DMA0ACK	EQU	80h		; Bus Master Request ACK, Port 0
TREG_DMA1ACK	EQU	81h		; Bus Master Request ACK, Port 1
TREG_DMA2ACK	EQU	82h		; Bus Master Request ACK, Port 2
TREG_DMA3ACK	EQU	83h		; Bus Master Request ACK, Port 3
TREG_WDT	EQU	0FEh		; Watchdog Timer Expired
TREG_NOI	EQU	0FFh		; No interrupt pending
;****************************

CPUPAGE		EQU	0014h		;
TREG		EQU	0016h		; Special non-clearing TASKREG
GAID		EQU	0018h		; Gate Array ID byte
;****************************
INITREG3	EQU	001Ah		;

IR3_FAIRNESS	EQU	08h		; Channel Fairness ENABLE/disable
;****************************
; The Bus Master DMA Channel definitions, equates, so on and so forth.
;
BMDMA_BASE	EQU	0020h

BM_TC		EQU	0000h		; Transfer Count, (2)
BM_CAE		EQU	0002h		; Card Address, Extension (2)
BM_CA		EQU	0004h		; Card Address (2)
BM_SAE		EQU	0006h		; System Address, Extension (2)
BM_SA		EQU	0008h		; System Address (2)
BM_CC		EQU	000Ah		; Channel Control (2)
BM_LAE		EQU	000Ch		; List Address, Extension (2)
BM_LA		EQU	000Eh		; List Address (2)
;
;  The following CDB definitions are matched to the "C" Channel
;  descriptor block "bm_dma_ccb".
;
CDB_TC		EQU	0000h		; Transfer Count
CDB_CA_LO	EQU	0002h		; Card Address, Low Word
CDB_CA_HI	EQU	0004h		; Card Address, High Word
CDB_SA_LO	EQU	0006h		; System Address, Low Word
CDB_SA_HI	EQU	0008h		; System Address, High Word
CDB_CC		EQU	000Ah		; Channel Control Word
CDB_LA_LO	EQU	000Ch		; List Address, Low Word
CDB_LA_HI	EQU	000Eh		; List Address, High Word
;
;  Bits of the Channel Control Word:
;
BM_CC_START	EQU	0001H		; Start(1)/Stop
BM_CC_IN	EQU	0002H		; Read channel(1)/Write channel
BM_CC_TCINT	EQU	0004H		; Terminal Count Interrupt (1/0)
BM_CC_LE	EQU	0008H		; List Chaining Enable (1/0)
BM_CC_SAINC	EQU	0010H		; System Address Increment(1)/Decrement
BM_CC_CAINC	EQU	0020H		; Card Address Increment(1)/Decrement
BM_CC_MEM	EQU	0040H		; Memory Cycle(1)/I-O Cycle
BM_CC_TCHLT	EQU	0080H		; Stop Channel on Terminal Count (1/0)
;****************************
; DMAPAGE0 to 15 are omitted, Card/Card DMA is prohibited.

DBID		EQU	0086h		; EIB (Daughterboard) ID (1)
CARDID		EQU	0088h		; Card ID Register (2)
PCCSTAT		EQU	008Ah		; Processor Channel Check Status (1)
PESTAT		EQU	008Ch		; Parity Error Status (2)
PROCSYNC	EQU	008Eh		; Processor Sync. Register (1)
;****************************
; INT0 Status registers and affiliated definitions
;
INT0STAT	EQU	0090h		; INT0 Status Register (1)

IS0_BMCH2	EQU	04h		; BM Ch. 2 Terminal Count
IS0_BMCH1	EQU	02h		; BM Ch. 1 Terminal Count
IS0_INTCOM	EQU	01h		; INTCOM Identifier

INTIDREG	EQU	0092h		; INT0 Identifier Register (2)
;****************************

IOCHCK		EQU	0094h		; I/O Channel Check Register (1)
PCPAR0		EQU	0096h		; Processor Channel Parity Reg. (1)
PCPAR1		EQU	0098h		; Processor Channel Parity Reg. (1)
PCPAR2		EQU	009Ah		; Processor Channel Parity Reg. (1)
TTPROTON	EQU	009Ch		; Translate Table Protection On  (1)
TTPROTOFF	EQU	009Eh		; Translate Table Protection Off (1)
BMCH1RESET	EQU	00A0h		; Bus Master Ch. 1 Reset (1)

ENREG		EQU	0800h		; Enables Register (1)
SIMMCONFIG	EQU	0802h		; SIMM Configuration Register (1)

SCCRELINT0	EQU	3220h		; SCC Release Interrupt, SCC 0
SCCRELINT1	EQU	3221h		; SCC Release Interrupt, SCC 1
SCCRELINT2	EQU	3222h		; SCC Release Interrupt, SCC 2/X.21 PAL
SCCRELINT3	EQU	3223h		; SCC Release Interrupt, SCC 3
SCCRELINT4	EQU	3224h		; SCC Release Interrupt, CIO 0
SCCRELINT5	EQU	3225h		; SCC Release Interrupt, CIO 1

;****************************
; MPQP Protection/Translation Feature Equates
;
TRANBASE	EQU	0C000h		; Base of the 64 Trans/Prot Regs

TRAN_WP		EQU	1		; Write Protect indicator
TRAN_RWP	EQU	2		; Read/Write Protect indicator

TTPROTOFF	EQU	9Eh		; Turn Translation/Protection OFF
TTPROTON	EQU	9Ch		; Turn Translation/Protection ON
;****************************
