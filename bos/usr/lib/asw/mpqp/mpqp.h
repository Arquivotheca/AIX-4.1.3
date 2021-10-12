/*
static char sccsid[] = "@(#)33  1.7  src/bos/usr/lib/asw/mpqp/mpqp.h, ucodmpqp, bos411, 9428A410j 7/20/93 13:14:17";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	mpqp.h		: Defines for Typhoon base card hardware specific
 *			  registers such as dma offsets, ccw bits, scc release
 *			  interrupt offsets, etc.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/****************************************************************************
 This is the master I/O Space definition file for Typhoon adapters.
 Significant I/O addresses are defined symbolically here as are
 relevant bits within many of them.  The type definitions for, for
 example, DMA channel control blocks are found in separate files, as
 are the definitions for fields within them (i.e. SCC, CIO, and all
 3 types of DMA).

 053089 : File updated to Echo 3 standards
 ****************************************************************************/

#define	TRANBASE	0xC000		/* Base of the 64 TRAN Registers */

#define	TRAN_WP		(unsigned short)0x0001	/* Write Protect */
#define	TRAN_RWP	(unsigned short)0x0002	/* Read/Write Protect */

/****************************************************************************/

#define	DMADISABLE	0x8214		/* Port DMA Master Disable Register */
#define	DMAASSIGN	0x8212		/* DMA Channel Assignment Register */
#define	DMAVECTOR	0x8210		/* DMA Vector Register */

/* DMA Channel Interrupt Status Registers */

#define	DMA_STAT_F	0x820F
#define	DMA_STAT_E	0x820E
#define	DMA_STAT_D	0x820D
#define	DMA_STAT_C	0x820C
#define	DMA_STAT_B	0x820B
#define	DMA_STAT_A	0x820A
#define	DMA_STAT_9	0x8209
#define	DMA_STAT_8	0x8208
#define	DMA_STAT_7	0x8207
#define	DMA_STAT_6	0x8206
#define	DMA_STAT_5	0x8205
#define	DMA_STAT_4	0x8204
#define	DMA_STAT_3	0x8203
#define	DMA_STAT_2	0x8202
#define	DMA_STAT_1	0x8201
#define	DMA_STAT_0	0x8200

/* Two significant bits exist in the DMA Interrupt Status Registers */

#define	DMA_ISR_CM	0x01		/* Character Match occurred */
#define	DMA_ISR_TC	0x02		/* Transfer Count expired (->0) */
#define DMA_ISR_EOM	0x04		/* End of Message from DUSCC */

/* DMA Channel Descriptor Table Base I/O Addresses */

#define	DMA_CDT_F	0x81E0
#define	DMA_CDT_E	0x81C0
#define	DMA_CDT_D	0x81A0
#define	DMA_CDT_C	0x8180
#define	DMA_CDT_B	0x8160
#define	DMA_CDT_A	0x8140
#define	DMA_CDT_9	0x8120
#define	DMA_CDT_8	0x8100
#define	DMA_CDT_7	0x80E0
#define	DMA_CDT_6	0x80C0
#define	DMA_CDT_5	0x80A0
#define	DMA_CDT_4	0x8080
#define	DMA_CDT_3	0x8060
#define	DMA_CDT_2	0x8040
#define	DMA_CDT_1	0x8020
#define	DMA_CDT_0	0x8000

/* DMA Channel Descriptor Table Internal Field Offsets */

#define	PDMA_CCW	0x00		/* Channel Control Word */
#define	PDMA_CA		0x02		/* Target Address bits 0-15 */
#define	PDMA_CM		0x04		/* Character Match Bytes */
#define	PDMA_IOA	0x06		/* I/O Address bits 0-15 */
#define	PDMA_TC		0x08		/* Transfer Count */
#define	PDMA_CAE	0x0A		/* Target Address bits 16-20 */
#define	PDMA_LAE	0x0C		/* List Address bits 16-20 */
#define	PDMA_LA		0x0E		/* List Address bits 0-15 */

/* Bit Definitions within the DMA Channel Control Word */

#define	PDMA_CCW_CM_BOTH     0x6000	/* Match on C1 AND C2 is sequence */
#define	PDMA_CCW_CM_EITHER   0x4000	/* Match on C1 OR C2 in data */
#define	PDMA_CCW_CM_C1ONLY   0x2000	/* Match on C1 only */
#define	PDMA_CCW_CM_OFF      0x1FFF	/* No Character Matching.  AND mask */

#define	PDMA_CCW_ST_EOM	0x1000		/* Stop Channel on EOM (Rx) */
#define	PDMA_CCW_ST_ACM	0x0800		/* Stop Assocaited Tx Chan. on CM */
#define	PDMA_CCW_ST_CM	0x0400		/* Stop Channel on Character Match */
#define	PDMA_CCW_ST_TC	0x0200		/* Stop Channel on Terminal Count */
#define	PDMA_CCW_IE_EOM	0x0100		/* Interrupt ENABLE/disable, EOM */
#define	PDMA_CCW_IE_CM	0x0080		/* Interrupt ENABLE/disable, CM  */
#define	PDMA_CCW_IE_TC	0x0040		/* Interrupt ENABLE/disable, TC  */
#define	PDMA_CCW_LE_EOM	0x0020		/* List Chaining ENABLE/disable, EOM */
#define	PDMA_CCW_LE_CM	0x0010		/* List Chaining ENABLE/disable, CM  */
#define	PDMA_CCW_LE_TC	0x0008		/* List Chaining ENABLE/disable, TC  */
#define	PDMA_CCW_BW	0x0004		/* WORD/byte indicator.  Always 0 */
#define	PDMA_CCW_RT	0x0002		/* TRANSMIT/receive channel */
#define	PDMA_CCW_EN	0x0001		/* DMA channel master enable */

#define	CM_CCW	(PDMA_CCW_ST_CM|PDMA_CCW_LE_CM|PDMA_CCW_IE_CM)
#define	TC_CCW	(PDMA_CCW_ST_TC|PDMA_CCW_LE_TC|PDMA_CCW_IE_TC)
#define	EOM_CCW	(PDMA_CCW_ST_EOM|PDMA_CCW_LE_EOM|PDMA_CCW_IE_EOM)

#define	LE_CCW	(PDMA_CCW_LE_EOM|PDMA_CCW_LE_CM|PDMA_CCW_LE_TC)
#define	ST_CCW	(PDMA_CCW_ST_EOM|PDMA_CCW_ST_CM|PDMA_CCW_ST_TC)
#define	IE_CCW	(PDMA_CCW_IE_EOM|PDMA_CCW_IE_CM|PDMA_CCW_IE_TC)

/* defines for receive channel control words */

#define	_CCW_SDLC_RX (TC_CCW|CM_CCW|EOM_CCW)

#define	_CCW_ASY_RX (PDMA_CCW_LE_TC|PDMA_CCW_LE_EOM|PDMA_CCW_LE_CM|PDMA_CCW_IE_EOM|PDMA_CCW_IE_TC|PDMA_CCW_IE_CM|PDMA_CCW_CM_EITHER)

#define _CCW_BSC_RX (TC_CCW|CM_CCW|EOM_CCW)

#define	_CCW_X21_RX (PDMA_CCW_LE_TC|PDMA_CCW_LE_EOM|PDMA_CCW_IE_EOM|PDMA_CCW_IE_TC|PDMA_CCW_CM_EITHER)

#define	_CCW_X21_CM (CM_CCW | TC_CCW | EOM_CCW | PDMA_CCW_CM_EITHER)

#define	PD_ABSC_CM	0x8510		/* ASCII DLE/ENQ (with odd parity)  */
#define	PD_EBSC_CM	0x2D10		/* EBCDIC DLE/ENQ */
#define	PD_X21_CM	0x07AB		/* X.21, PLUS (with parity) or BEL  */
#define	PD_CM_ASY_N	0x0A13		/* Async, LF and XOFF (No parity)   */
#define	PD_CM_ASY_E	0x0A93		/* Async, LF and XOFF (Even parity) */
#define	PD_CM_ASY_O	0x8A13		/* Async, LF and CR (Odd parity)  */

/* Transmit channel Echo CCW Caveats:
   Echo is strange.  To get automatic TEOM handshaking between the DUSCC
   and Echo, List Chaining must be enabled.  This is accomodated in the
   assembler SetPDMA routine, which automatically links Transmit channels
   to a null (stopped) CCW.
   These definitions are VERY sensitive!  Read all the bit definitions again
   before altering them.  Many EOM bits are defined on Rx channels only.

   PDMA_CCW_LE_EOM, when set, tells echo to send a signal to DUSCC upon 
   terminal count to instruct DUSCC to send out EOM flag after transmission
   of this last byte.
*/

#define	_CCW_TX      (PDMA_CCW_RT|PDMA_CCW_ST_TC|PDMA_CCW_LE_TC|PDMA_CCW_LE_EOM)
#define	_CCW_TX_BSC  (PDMA_CCW_RT|PDMA_CCW_ST_TC|PDMA_CCW_LE_TC|PDMA_CCW_LE_EOM)
#define	_CCW_TX_SDLC (PDMA_CCW_RT|PDMA_CCW_ST_TC|PDMA_CCW_LE_TC|PDMA_CCW_LE_EOM)
#define	_CCW_TX_ASY  (PDMA_CCW_RT|PDMA_CCW_ST_TC|PDMA_CCW_LE_TC|PDMA_CCW_IE_TC)
 
#define	SCCRELINT5	0x3225		/* SCC Release Register, CIO 1 */
#define	SCCRELINT4	0x3224		/* SCC Release Register, CIO 0 */
#define	SCCRELINT3	0x3223		/* SCC Release Register, SCC 3 */
#define	SCCRELINT2	0x3222		/* SCC Release Register, SCC 2, X.21 */
#define	SCCRELINT1	0x3221		/* SCC Release Register, SCC 1 */
#define	SCCRELINT0	0x3220		/* SCC Release Register, SCC 0 */

#define	SCC_3B_BASE	0x1B20		/* SCC 3 Channel B Base Address */
#define	SCC_3A_BASE	0x1B00		/* SCC 3 Channel A Base Address */
#define	SCC_2B_BASE	0x1A20		/* SCC 2 Channel B Base Address */
#define	SCC_2A_BASE	0x1A00		/* SCC 2 Channel A Base Address */
#define	SCC_1B_BASE	0x1920		/* SCC 1 Channel B Base Address */
#define	SCC_1A_BASE	0x1900		/* SCC 1 Channel A Base Address */
#define	SCC_0B_BASE	0x1820		/* SCC 0 Channel B Base Address */
#define	SCC_0A_BASE	0x1800		/* SCC 0 Channel A Base Address */

/* Enable register IO address  */
#define ENREG      	0x0800		/* Enable Register Address      */
/* Enable register bit map */
#define ENR_S_X21_CA	(unsigned char)0x01	/* X21 CONTROL line, high */
#define ENR_C_X21_CA	(unsigned char)0xFE	/* X21 CONTROL line, low  */
#define ENR_S_X21	(unsigned char)0x02	/* X21 PAL "Disable" OR mask */
#define ENR_C_X21	(unsigned char)0xFD	/* X21 PAL "Enable" AND mask */
/* Enables register Default setting, everthing is disabled */
#define	ENREG_DISABLE	(unsigned char)0x06
                                                                          
#define	CIO_1B_SRB	0x550		/* CIO 1 Port B Spec. Registers */
#define	CIO_1A_SRB	0x540		/* CIO 1 Port A Spec. Registers */
#define	CIO_0B_SRB	0x1D0		/* CIO 0 Port B Spec. Registers */
#define	CIO_0A_SRB	0x1C0		/* CIO 0 Port A Spec. Registers */

#define	CIO_1_BASE	0x500		/* CIO 1 Base Address */
#define	CIO_0_BASE	0x180		/* CIO 0 Base Address */

#define	PCPAR2		0x009A		/* PC Parity Error Register 2 */
#define	PCPAR1		0x0098		/* PC Parity Error Register 1 */
#define	PCPAR0		0x0096		/* PC Parity Error Register 0 */
#define	IOCHCK		0x0094		/* I/O Channel Check Register */
#define	INTIDREG	0x0092		/* Interrupt ID Reg, uC Masters */

/****************************************************************************/

#define	INT0STAT	0x0090		/* CPU INT0 Status Register */

#define	IS0_BMCH2	(unsigned char)0x04	/* BM Ch. 2 Terminal Count */
#define	IS0_BMCH1	(unsigned char)0x02	/* BM Ch. 1 Terminal Count */
#define	IS0_INTCOM	(unsigned char)0x01	/* INTCOM Identifier */

/****************************************************************************/

#define	PESTAT		0x008C		/* PESTAT Register */
#define	PCCSTAT		0x008A		/* PCCSTAT Register */
#define	CARDID		0x0088		/* Card ID Register */

#define	DMA_PAGE_F	0x005E		/* DMA Page Register 15 */
#define	DMA_PAGE_E	0x005C		/* DMA Page Register 14 */
#define	DMA_PAGE_D	0x005A		/* DMA Page Register 13 */
#define	DMA_PAGE_C	0x0058		/* DMA Page Register 12 */
#define	DMA_PAGE_B	0x0056		/* DMA Page Register 11 */
#define	DMA_PAGE_A	0x0054		/* DMA Page Register 10 */
#define	DMA_PAGE_9	0x0052		/* DMA Page Register 9 */
#define	DMA_PAGE_8	0x0050		/* DMA Page Register 8 */
#define	DMA_PAGE_7	0x004E		/* DMA Page Register 7 */
#define	DMA_PAGE_6	0x004C		/* DMA Page Register 6 */
#define	DMA_PAGE_5	0x004A		/* DMA Page Register 5 */
#define	DMA_PAGE_4	0x0048		/* DMA Page Register 4 */
#define	DMA_PAGE_3	0x0046		/* DMA Page Register 3 */
#define	DMA_PAGE_2	0x0044		/* DMA Page Register 2 */
#define	DMA_PAGE_1	0x0042		/* DMA Page Register 1 */
#define	DMA_PAGE_0	0x0040		/* DMA Page Register 0 */

#define	BM_CH_1_BASE	0x0030		/* Bus Master DMA Channel 1 Base */
#define	BM_CH_0_BASE	0x0020		/* Bus Master DMA Channel 0 Base */
	
#define	SCCSEL		0x001C		/* SCC Selection Register */

/****************************************************************************/

#define	INITREG3	0x001A		/* uC Initialization Register 3 */

#define	IR3_FEN		(unsigned char)0x10	/* Fairness Enable */

/****************************************************************************/

#define	GAID		0x0018		/* Gate Array ID Register */

#define	GAID_C1		(unsigned char)0x00	/* SSTIC1 */
#define	GAID_C2		(unsigned char)0x30	/* SSTIC2 */
#define	GAID_C3		(unsigned char)0xC0	/* SSTIC3 */
#define	GAID_C4		(unsigned char)0x80	/* SSTIC4 */

/****************************************************************************/

#define	CPUPAGE		0x0014		/* CPU Page Register */

/****************************************************************************/

#define	TASKREG		0x0012		/* Task Register (TREG) */

/****************************************************************************/

#define	RICPAR2		0x0010		/* Parity Error Address Latch 2 */
#define	RICPAR1		0x000E		/* Parity Error Address Latch 1 */
#define	RICPAR0		0x000C		/* Parity Error Address Latch 0 */

/****************************************************************************/

#define	NMISTAT		0x000A		/* NMI Interrupt Status Register */
#define	NMIMASK		0x0008		/* NMI Interrupt Mask Register */

#define	NMI_CRST	(unsigned short)0x0001	/* Channel Reset */
#define	NMI_WD		(unsigned short)0x0002	/* Watchdog Timer */
#define	NMI_PE		(unsigned short)0x0004	/* Parity Error */
#define	NMI_PCC		(unsigned short)0x0008	/* uC Channel Check */
#define	NMI_NC		(unsigned short)0x0010	/* uC Generated NMI */
#define	NMI_PROT	(unsigned short)0x0020	/* W, R/W Protect Error */

#define	NMI_M_DG	(unsigned short)0x0040	/* Degate RAM from uC */
#define	NMI_S_IE	(unsigned short)0x0040	/* uC Interrupts Enabled */
#define	NMI_M_FP	(unsigned short)0x0080	/* Force Bad uC Parity */
#define	NMI_S_IP	(unsigned short)0x0080	/* uC Interrupt Pending */

#define	NMI_DAC		(unsigned short)0x0100	/* Diagnostic Address Comp. */
#define	NMI_EPCC	(unsigned short)0x0200	/* External Channel Check */

#define	NMI_M_PAREN	(unsigned short)0x0400	/* Parity Latch Enable */

/****************************************************************************/

#define	INITREG1	0x0006		/* uC Initialization Register 1 */

#define	IR1_DRT		(unsigned char)0x80	/* Double Refresh Time */
#define	IR1_PRDY	(unsigned char)0x40	/* PROM Ready */
#define	IR1_PGEN	(unsigned char)0x10	/* DMAPG Enable */

# define INIT_REG1		in08( INITREG1 )
# define IR1_MSIZE		(unsigned char)0x28
# define M_1MEG			(unsigned char)0x00
# define M_128K			(unsigned char)0x08
# define M_512K			(unsigned char)0x20
# define M_2MEG			(unsigned char)0x28

# define MEMORY_SIZE		( INIT_REG1 & IR1_MSIZE )

/****************************************************************************/

#define	INITREG0	0x0004		/* uC Initialization Register 0 */
#define	LOCREG1		0x0002		/* uC Location Register 1 */
#define	LOCREG0		0x0000		/* uC Location Register 0 */

