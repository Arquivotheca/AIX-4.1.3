/*
static char sccsid[] = "@(#)28	1.5  src/bos/usr/lib/asw/mpqp/duscc.h, ucodmpqp, bos411, 9428A410j 1/29/91 13:50:24";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 	dussc.h - Defines for DUSCC bit masks and register offsets.
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

/*
   Signetics DUSCC Definitions.
   Notice that the DUSCC type definition cannot be used, per se, because
   the DUSCC exists in I/O address space and not memory.
*/

#define	byte		unsigned char

typedef struct
       {
	 unsigned char		cm1;		/* channel mode register 1 */
	 unsigned char		cm2;		/* channel mode register 2 */
	 unsigned char		s1;		/* SYN1/Second. Addr. 1 */
	 unsigned char		s2;		/* SYN2/Second. Addr. 2 */
	 unsigned char		tp;		/* tx parameter register */
	 unsigned char		tt;		/* tx timing register */
	 unsigned char		rp;		/* rx parameter register */
	 unsigned char		rt;		/* rx timing register */
	 unsigned char		ctph;		/* c/t preset register hi */
	 unsigned char		ctpl;		/* c/t preset register lo */
	 unsigned char		ctc;		/* c/t control register */
	 unsigned char		om;		/* output & misc. register */
	 unsigned char		cth;		/* c/t hi */
	 unsigned char		ctl;		/* c/t lo */
	 unsigned char		pc;		/* pin config. register */
	 unsigned char		cc;		/* channel command register */
	 unsigned char		txfifo [4];	/* tx fifo */
	 unsigned char		rxfifo [4];	/* rx fifo */
	 unsigned char		rs;		/* rx status register */
	 unsigned char		trs;		/* tx & rx status register */
	 unsigned char		icts;		/* input & c/t status reg. */
	 unsigned char		gs;		/* General Status Register */
	 unsigned char		ie;		/* interrupt enable register */
	 unsigned char		fill0;
	 unsigned char		iv;		/* interrupt vector register */
	 unsigned char		ivm;		/* modified ivr */
	 unsigned char		ic;		/* interrupt control reg. */
	 unsigned char		mr;		/* master reset register */
       } t_scc_reg;

#define	SCC_CM1		(int)0
#define	SCC_CM2		(int)2
#define	SCC_S1		(int)4
#define	SCC_S2		(int)6
#define	SCC_TP		(int)8
#define	SCC_TT		(int)10
#define	SCC_RP		(int)12
#define	SCC_RT		(int)14
#define	SCC_CTPH	(int)16
#define	SCC_CTPL	(int)18
#define	SCC_CTC		(int)20
#define	SCC_OM		(int)22
#define	SCC_CTH		(int)24
#define	SCC_CTL		(int)26
#define	SCC_PC		(int)28
#define	SCC_CC		(int)30
#define	SCC_TXFIFO	(int)32
#define	SCC_RXFIFO	(int)40
#define	SCC_RS		(int)48
#define	SCC_TRS		(int)50
#define	SCC_ICTS	(int)52
#define	SCC_GS		(int)54
#define	SCC_IE		(int)56
#define	SCC_IV		(int)60
#define	SCC_IVM		(int)124
#define	SCC_IC		(int)62
#define	SCC_MR		(int)126

/* Channel Mode Register 1 */

#define	C1_NRZ		(byte)0xC0	/* NRZ, Not an OR Mask! */
#define	C1_NRZI		(byte)0x40
#define	C1_FM0		(byte)0x80
#define	C1_FM1		(byte)0xC0

#define	C1_EC		(byte)0x20	/* B: Extended Control */
#define	C1_PODD		(byte)0x20	/* Odd Parity */
#define	C1_PEVEN	(byte)0xDF	/* even parity, and mask */

#define	C1_8BIT		(byte)0x18	/* B: 8 bit address mode */
					/* Above Not an OR Mask! */
#define	C1_EXTADR	(byte)0x08	/* B: extended address mode */
#define	C1_16BIT	(byte)0x10	/* B: 16 bit address mode */
#define	C1_16GRP	(byte)0x18	/* B: 16 bit with group mode */

#define	C1_NOPAR	(byte)0xE7	/* A/C: No parity AND mask */
					/* Above Not an OR Mask! */
#define	C1_YESPAR	(byte)0x10	/* A/C: With parity */
#define	C1_FRCPAR	(byte)0x18	/* A/C: Force parity */

#define	C1_PRI		(byte)0x07	/* B: Primary.  Not an OR Mask! */
#define	C1_SEC		(byte)0x01	/* B: Secondary */
#define	C1_LOOP		(byte)0x02	/* B: Loop */
#define	C1_LOOPNO	(byte)0x03	/* B: Loop, No address compare */
#define	C1_2SYN		(byte)0x04	/* C: Dual SYN */
#define	C1_BISYNC	(byte)0x05	/* C: Dual SYN (BISYNC) */
#define	C1_1SYN		(byte)0x06	/* C: Single SYN */
#define	C1_ASYNC	(byte)0x07	/* C: async */

/* Channel Mode Register 2 */

#define	C2_NORM		(byte)0xC0	/* Normal.  Not an OR Mask! */
#define	C2_AUTOECHO	(byte)0x40	/* Auto Echo mode */
#define	C2_LOCALLOOP	(byte)0x80	/* Local Loop mode */

#define	C2_HDX1		(byte)0x38	/* HDX Single Address DMA */
					/* Warning: Above not an OR Mask! */
#define	C2_HDX2		(byte)0x08	/* HDX Dual Address DMA */
#define	C2_FDX1		(byte)0x10	/* FDX Single Address DMA */
#define	C2_FDX2		(byte)0x18	/* FDX Dual Address DMA */
#define	C2_RXWAIT	(byte)0x20	/* Wait on RX Only */
#define	C2_TXWAIT	(byte)0x28	/* Wait on TX Only */
#define	C2_WAITBOTH	(byte)0x30	/* Wait on RX or TX */
#define	C2_PORI		(byte)0x38	/* Polled or Interrupt */

#define	C2_NOFCS	(byte)0x07	/* None.  Not an OR Mask! */
#define	C2_0LRC8	(byte)0x02	/* LRC 8 Preset to 0 */
#define	C2_1LRC8	(byte)0x03	/* LRC 8 Preset to 1 */
#define	C2_0CRC16	(byte)0x04	/* CRC 16 Preset to 0 */
#define	C2_1CRC16	(byte)0x05	/* CRC 16 Preset to 1 */
#define	C2_0CCITT	(byte)0x06	/* CRC CCITT Preset 0 */
#define	C2_1CCITT	(byte)0x07	/* CRC CCITT Preset 1 */

/* Pin Configuration Register */

#define	PC_GPO2		(byte)0xBF	/* General Purpose.  Not OR Mask! */
#define	PC_RTS		(byte)0x40	/* RTS Output. */

#define	PC_SYNOUT	(byte)0xDF	/* SYNOUT Select.  Not OR Mask! */
#define	PC_SYN_RTS	(byte)0x20	/* RTS Output. */

#define	PC_R_INP	(byte)0xE7	/* RTxC Pin, input.  !OR Mask */
#define	PC_R_CT		(byte)0x08	/* RTxC Pin, output from C/T */
#define	PC_R_Tx1	(byte)0x10	/* RTxC Pin, output from TXclk * 1 */
#define	PC_R_Rx1	(byte)0x18	/* RTxC Pin, output from RXclk * 1 */
#define	PC_T_INP	(byte)0xF8	/* TRxC Pin, input.  !OR Mask */

#define	PC_T_X2		(byte)0x01	/* TRxC Pin, output XTAL/2 */
#define	PC_T_DPLL	(byte)0x02	/* TRxC Pin, output from DPLL */
#define	PC_T_CT		(byte)0x03	/* TRxC Pin, output from C/T */
#define	PC_T_Tx16	(byte)0x04	/* TRxC Pin, output from TXclk * 16 */
#define	PC_T_Rx16	(byte)0x05	/* TRxC Pin, output from RXclk * 16 */
#define	PC_T_Tx1	(byte)0x06	/* TRxC Pin, output from TXclk * 1 */
#define	PC_T_Rx1	(byte)0x07	/* TRxC Pin, output from RXclk * 1 */

/* Transmitter Parameter Register */

#define	TP_A_STOP1	(byte)0x70	/* Note!  Set for 1 Stop Bit */
#define	TP_A_STOP15	(byte)0x80	/* Note!  Set for 1.5 Stop Bits */
#define	TP_A_STOP2	(byte)0xF0	/* Note!  Set for 2 Stop Bits */

#define	TP_B_EFI	(byte)0x3F	/* Underrun: FCS/FLAG/Idle  !OR */
#define	TP_B_AMARK	(byte)0x80	/* Underrun: Abort/Marks */
#define	TP_B_AFLAG	(byte)0xC0	/* Underrun: Abort/Flags */

#define	TP_B_IMARK	(byte)0xDF	/* Idle with MARK, !OR Mask */
#define	TP_B_IFLAG	(byte)0x20	/* Idle with FLAG */
#define	TP_B_ISYN	(byte)0x20	/* Idle with SYN's */

#define	TP_B_0TEOM	(byte)0xEF	/* Don't TEOM.  Not an OR Mask! */
#define	TP_B_1TEOM	(byte)0x10	/* TEOM on Zero Cnt or Done */

#define	TP_C_EI		(byte)0x3F	/* Underrun: FCS/Idle  Not an OR */
#define	TP_C_MARK	(byte)0x80	/* Underrun: Send MARK */
#define	TP_C_SYN	(byte)0xC0	/* Underrun: Send SYN */

#define	TP_C_IMARK	(byte)0xDF	/* Idle with MARK, !OR Mask */
#define	TP_C_ISYN	(byte)0x20	/* Idle with SYN */

#define	TP_C_0TEOM	(byte)0xEF	/* Don't TEOM.  Not an OR Mask! */
#define	TP_C_1TEOM	(byte)0x10	/* TEOM on Zero Cnt or Done */

#define	TP_0TXRTS	(byte)0xF7	/* TX RTS Control?  No.  !OR Mask */
#define	TP_1TXRTS	(byte)0x08	/* TX RTS Control?  Yes. */
#define	TP_0TXCTS	(byte)0xFB	/* TX CTS Enable?  No.  !OR Mask */
#define	TP_1TXCTS	(byte)0x04	/* TX CTS Enable?  Yes. */
#define	TP_5BITS	(byte)0xFC	/* Select 5 data bits.  Not OR Mask */
#define	TP_6BITS	(byte)0x01	/* Select 6 data bits. */
#define	TP_7BITS	(byte)0x02	/* Select 7 data bits. */
#define	TP_8BITS	(byte)0x03	/* Select 8 data bits. */

/* Transmitter Timing Register */

#define	TT_RTxC		(byte)0x7F	/* Ext. Source: RTxC.  !an OR Mask */
#define	TT_TRxC		(byte)0x80	/* Ext. Source: TRxC */

#define	TT_Ex1		(byte)0x8F	/* Clock Sel., 1 * External.  !OR */
#define	TT_Ex16		(byte)0x10	/* Clock Sel., 16 * External */
#define	TT_DPLL		(byte)0x20 	/* Clock Sel., DPLL */
#define	TT_BRG		(byte)0x30	/* Clock Sel., Bit Rate Generator */
#define	TT_ACTx2	(byte)0x40	/* Other Channel C/T * 2 */
#define	TT_ACTx32	(byte)0x50	/* Other Channel C/T * 32 */
#define	TT_PCTx2	(byte)0x60	/* Own Channel C/T * 2 */
#define	TT_PCTx32	(byte)0x70	/* Own Channel C/T * 32 */

/* Receiver Parameter Register */

#define	RP_A_0RTS	(byte)0xEF	/* RX RTS Control?  No.  !OR Mask */
#define	RP_A_1RTS	(byte)0x10	/* RX RTS Control?  Yes. */
#define	RP_A_0PS	(byte)0xF7	/* Strip Parity?  No.  !OR Mask */
#define	RP_A_1PS	(byte)0x08	/* Strip Parity?  Yes. */

#define	RP_C_0SYNS	(byte)0x7F	/* No SYN Strip.  !OR Mask */
#define	RP_C_1SYNS	(byte)0x80	/* Yes SYN Strip. */
#define	RP_C_0FCSF	(byte)0xBF	/* No FCS to FIFO.  !OR Mask */
#define	RP_C_1FCSF	(byte)0x40	/* Yes FCS to FIFO. */
#define	RP_C_0AHP	(byte)0xDF	/* No Auto Hunt/Pad Chk.  !OR Mask */
#define	RP_C_1AHP	(byte)0x20	/* Yes Auto Hunt/Pad Chk. */
#define	RP_C_0ESYN	(byte)0xEF	/* No External Sync.  !OR Mask */
#define	RP_C_1ESYN	(byte)0x10	/* Yes External Sync. */
#define	RP_C_0PS	(byte)0xF7	/* No Parity Strip.  !OR Mask */
#define	RP_C_1PS	(byte)0x08	/* Yes Parity Strip. */

#define	RP_B_0FCSF	(byte)0xBF	/* No FCS to FIFO.  !OR Mask */
#define	RP_B_1FCSF	(byte)0x40	/* Yes FCS to FIFO. */
#define	RP_B_HOVRM	(byte)0xDF	/* Overrun Mode: HUNT.  !OR Mask */
#define	RP_B_COVRM	(byte)0x20	/* Overrun Mode: CONT. */
#define	RP_C_0APA	(byte)0xF7	/* No All Party Address.  !OR Mask */
#define	RP_C_1APA	(byte)0x08	/* Yes All Party Address. */

#define	RP_0RXDCD	(byte)0xFB	/* RX DCD Enable?  No.  !OR Mask */
#define	RP_1RXDCD	(byte)0x04	/* RX DCD Enable?  Yes. */
#define	RP_5BITS	(byte)0xFC	/* Select 5 data bits.  Not OR Mask */
#define	RP_6BITS	(byte)0x01	/* Select 6 data bits. */
#define	RP_7BITS	(byte)0x02	/* Select 7 data bits. */
#define	RP_8BITS	(byte)0x03	/* Select 8 data bits. */

/* Receiver Timing Register */

#define	RT_RTxC		(byte)0x7F	/* Ext. Source: RTxC.  !an OR Mask */
#define	RT_TRxC		(byte)0x80	/* Ext. Source: TRxC */

#define	RT_Ex1		(byte)0x8F	/* Clock Sel., 1 * External.  !OR */
#define	RT_A_Ex16	(byte)0x10	/* Clock Sel., 16 * External */
#define	RT_A_BRG	(byte)0x20	/* Clock Sel., Bit Rate Generator */
#define	RT_A_CT		(byte)0x30	/* Clock Sel., Bit Rate Generator */
#define	RT_DPLL64	(byte)0x40	/* DPLL, 64 * X1/CLK */
#define	RT_DPLL32E	(byte)0x50	/* DPLL, 32 * External */
#define	RT_DPLL32B	(byte)0x60	/* DPLL, 32 * Bit Rate Generator */
#define	RT_DPLL32C	(byte)0x70	/* DPLL, 32 * C/T */

/* Output and Miscellaneous Register */

#define OM_TXRES_LEN    (byte)0xE0       /* Tx residual char len (AND mask)  */
#define OM_TXRDY        (byte)0x10       /* Tx FIFO empty        (AND mask)  */
#define OM_RXRDY        (byte)0x08       /* Rx FIFO full         (AND mask)  */
#define OM_GPO2_LOW     (byte)0x04       /* General purpose output 2, OR     */
#define OM_GPO2_HIGH    (byte)0xFB       /* General purpose output 2, AND    */
#define OM_GPO1_LOW     (byte)0x02       /* General purpose output 1, OR     */
#define OM_GPO1_HIGH    (byte)0xFD       /* General purpose output 1, AND    */
#define OM_RTS_ACTIVE   (byte)0x01       /* RTS control, active low (OR mask)*/
#define OM_RTS_OFF      (byte)0xFE       /* RTS control, disable high, AND   */

/* Baud Rates for the BRG Encoded fields */

#define	BR_50		(byte)0xF0	/* Not an OR Mask */
#define	BR_75		(byte)0x01
#define	BR_110		(byte)0x02
#define	BR_134		(byte)0x03
#define	BR_150		(byte)0x04
#define	BR_200		(byte)0x05
#define	BR_300		(byte)0x06
#define	BR_600		(byte)0x07
#define	BR_1050		(byte)0x08
#define	BR_1200		(byte)0x09
#define	BR_2000		(byte)0x0A
#define	BR_2400		(byte)0x0B
#define	BR_4800		(byte)0x0C
#define	BR_9600		(byte)0x0D
#define	BR_19200	(byte)0x0E
#define	BR_38400	(byte)0x0F

/* Counter/Timer Control Register */

#define	CTC_DZDI	(byte)0x7F	/* Disable Zero Detect Interrupt */
					/* Above Not an OR Mask */
#define	CTC_EZDI	(byte)0x80	/* Enable Zero Detect Interrupt */
#define	CTC_PZDC	(byte)0xBF	/* Preset Zero Detect Control */
					/* Above Not an OR Mask */
#define	CTC_CZDC	(byte)0x40	/* Continue Zero Detect Control */
#define	CTC_OCSQ	(byte)0xDF	/* Square Output Control */
					/* Above Not an OR Mask */
#define	CTC_OCPUL	(byte)0x20	/* Pulse Output Control */
#define	CTC_PRE_1	(byte)0xE7	/* Prescaler = 1, Not an OR Mask */
					/* Above Not an OR Mask */
#define	CTC_PRE_16	(byte)0x08	/* Prescaler = 16 */
#define	CTC_PRE_32	(byte)0x10	/* Prescaler = 32 */
#define	CTC_PRE_64	(byte)0x18	/* Prescaler = 64 */

/* Clock Source */

#define	CTC_CS_RTxC	(byte)0xF8	/* RTxC, Not an OR Mask */
#define	CTC_CS_TRxC	(byte)0x01	/* TRxC */
#define	CTC_CS_XDIV4	(byte)0x02	/* X1/CLK divided by 4 */
#define	CTC_CS_XGRxD	(byte)0x03	/* X1/CLK / 4 gated by RxD */
#define	CTC_CS_RxBRG	(byte)0x04	/* Rx Bit Rate Generator */
#define	CTC_CS_TxBRG	(byte)0x05	/* Tx Bit Rate Generator */
#define	CTC_CS_RxCHAR	(byte)0x06	/* Rx Characters */
#define	CTC_CS_TxCHAR	(byte)0x07	/* Tx Characters */

/* Channel Command Register */

#define CC_RESET_TX     (byte)0x00      /* Reset Transmitter */
#define CC_RESET_TXCRC  (byte)0x01      /* Reset Transmit CRC */
#define CC_ENABLE_TX    (byte)0x02      /* Enable Transmitter */
#define CC_DISABLE_TX   (byte)0x03      /* Disable Transmitter */
#define CC_TX_SOM       (byte)0x04      /* Transmit Start of Msg */
#define CC_TX_SOMP      (byte)0x05      /* Transmit Start of Msg w/ Pad */
#define CC_TX_EOM       (byte)0x06      /* Transmit End of Msg */
#define CC_TX_ABORT     (byte)0x07      /* Transmit Abort */
#define CC_TX_DLE	(byte)0x08      /* Transmit DLE */
#define CC_GAOP		(byte)0x09      /* Go Active on Poll */
#define CC_RESET_GAOP	(byte)0x0A      /* Reset Go Active on Poll */
#define CC_GO_ON_LP	(byte)0x0B      /* Go on Loop */
#define CC_GO_OFF_LP	(byte)0x0C      /* Go off Loop */
#define CC_EXCL_CRC	(byte)0x0D      /* Exclude From CRC */
#define CC_RESET_RX     (byte)0x40      /* Reset Transmitter */
#define CC_ENABLE_RX    (byte)0x42      /* Enable Transmitter */
#define CC_DISABLE_RX   (byte)0x43      /* Disable Transmitter */
#define CC_START_CT	(byte)0x80	/* start counter/timer */
#define CC_STOP_CT	(byte)0x81	/* stop counter/timer */
#define CC_PRESET_FF_CT	(byte)0x82	/* preset counter/timer from ff */
#define CC_PRESET_PR_CT	(byte)0x83	/* start counter/timer from parm reg */
#define CC_DPLL_SEARCH	(byte)0xC0	/* enter search mode */
#define CC_DPLL_DISABLE	(byte)0xC1	/* disable DPLL */
#define CC_DPLL_FM	(byte)0xC2	/* set FM mode */
#define CC_DPLL_NRZI	(byte)0xC3	/* set NRZI mode */


/* Receiver Status Register */

#define	RS_A_CMPCH	(byte)0x80	/* Character Compare */
#define	RS_A_LOSTRTS	(byte)0x40	/* RTS Negated */
#define	RS_A_OVERRUN	(byte)0x20	/* Overrun Error */
#define	RS_A_BRKEND	(byte)0x08	/* Break End Detect */
#define	RS_A_BRKSTART	(byte)0x04	/* Break Start Detect */
#define	RS_A_FRAMING	(byte)0x02	/* Framing Error */
#define	RS_A_PARITY	(byte)0x01	/* Parity Error */

#define	RS_B_EOM	(byte)0x80	/* EOM Detect */
#define	RS_B_ABORT	(byte)0x40	/* Abort Detect */
#define	RS_B_OVERRUN	(byte)0x20	/* Overrun Error */
#define	RS_B_SHORT	(byte)0x10	/* Short Frame Detect */
#define	RS_B_IDLE	(byte)0x08	/* Idle Detect */
#define	RS_B_FLAG	(byte)0x04	/* Flag Detect */
#define	RS_B_CRC	(byte)0x02	/* CRC Error */
#define	RS_B_RCL	(byte)0x01	/* RCL Not Zero */

#define	RS_C_EOM	(byte)0x80	/* EOM Detect */
#define	RS_C_PAD	(byte)0x40	/* PAD Error */
#define	RS_C_OVERRUN	(byte)0x20	/* Overrun Error */
#define	RS_C_RSVD	(byte)0x18	/* Reserved Bits - Not used */
#define	RS_C_SYN	(byte)0x04	/* SYN Detect */
#define	RS_C_CRC	(byte)0x02	/* CRC Error */
#define	RS_C_PARITY	(byte)0x01	/* Parity Error */

#define	RS_L_EOM	(byte)0x80	/* EOM Detect */
#define	RS_L_ABORT	(byte)0x40	/* Abort/EOP Detect */
#define	RS_L_OVERRUN	(byte)0x20	/* Overrun Error */
#define	RS_L_SHORT	(byte)0x10	/* Short Frame Detect */
#define	RS_L_TURN	(byte)0x08	/* Turn-around Detect */
#define	RS_L_FLAG	(byte)0x04	/* Flag Detect */
#define	RS_L_CRC	(byte)0x02	/* CRC Error */
#define	RS_L_RCL	(byte)0x01	/* RCL Not Zero */

/* Transmitter and Receiver Status Register */

#define	TRS_TXEMPTY	(byte)0x80	/* Transmitter Empty */
#define	TRS_CTSUNDER	(byte)0x40	/* CTS Underrun */
#define	TRS_EOFRAME	(byte)0x20	/* Frame Complete (Not Async) */
#define	TRS_SENDACK	(byte)0x10	/* A:Break C:SOM B:SOM/Abort Ack */
#define	TRS_DPLL	(byte)0x08	/* DPLL Error */

#define	TRS_C_HUNT	(byte)0x02	/* Rx HUNT Mode */
#define	TRS_C_XPNT	(byte)0x01	/* Rx Transparent Mode */

#define	TRS_B_RESIDUE	(byte)0xF8	/* AND Mask, BOP Residual char lgt */

/* These definitions are used with the IE (Interrupt Enable) bits in the
   Interrupt Enable register to control what Rx and Tx/Rx Status sources
   actually interrupt generate interrupts */

#define	RS_IE_76	(byte)(0x80|0x40)
#define	RS_IE_54	(byte)(0x20|0x10)
#define	RS_IE_32	(byte)(0x08|0x04)
#define	RS_IE_10	(byte)(0x02|0x01)

#define	TRS_IE		(byte)(0x80|0x40|0x20|0x10)

/* Input and Counter/Timer Status Register */

#define	ICS_CRUN	(byte)0x80	/* Counter Running */
#define	ICS_CZERO	(byte)0x40	/* Counter Zero Count */
#define	ICS_dDCD	(byte)0x20	/* Delta DCD */
#define	ICS_dCTS	(byte)0x10	/* Delta CTS */
#define	ICS_DCD		(byte)0x08	/* DCD */
#define	ICS_CTS		(byte)0x04	/* CTS */
#define	ICS_GPI2	(byte)0x02	/* General Purpose Input 2 */
#define	ICS_GPI1	(byte)0x01	/* General Purpose Input 1 */

/* Interrupt Enable Register */

#define	IE_DELTA	(byte)0x80	/* Delta DCD/CTS Enable */
#define	IE_TXRDY	(byte)0x40	/* TxRDY Enable */
#define	IE_TRSR		(byte)0x20	/* TRSR Bits 7:3 Enable */
#define	IE_RXRDY	(byte)0x10	/* RxRDY Enable */
#define	IE_RSR_76	(byte)0x08	/* RSR Bits 7:6 Enable */
#define	IE_RSR_54	(byte)0x04	/* RSR Bits 5:4 Enable */
#define	IE_RSR_32	(byte)0x02	/* RSR Bits 3:2 Enable */
#define	IE_RSR_10	(byte)0x01	/* RSR Bits 1:0 Enable */

/* General Status Register */

#define	GS_A_CTEXT	(byte)0x80	/* C/T or External Status */
#define	GS_A_RXTX	(byte)0x40	/* Rx/Tx Status */
#define	GS_A_TXRDY	(byte)0x20	/* */
#define	GS_A_RXRDY	(byte)0x10	/* */
#define	GS_B_CTEXT	(byte)0x08	/* C/T or External Status */
#define	GS_B_RXTX	(byte)0x04	/* Rx/Tx Status */
#define	GS_B_TXRDY	(byte)0x02	/* */
#define	GS_B_RXRDY	(byte)0x01	/* */

/* Interrupt Control Register */

#define	IC_CH_A		(byte)0x3F	/* Priority Channel A */
#define	IC_CH_B		(byte)0x40	/* Priority Channel B */
#define	IC_INT_A	(byte)0x80	/* Interleaved Priority A */
#define	IC_INT_B	(byte)0xC0	/* Interleaved Priority B */

#define	IC_VM_0		(byte)0xCF	/* Vectored Mode 0 */
#define	IC_VM_1		(byte)0x10	/* Vectored Mode 1 */
#define	IC_VM_2		(byte)0x20	/* Vectored Mode 2 */
#define	IC_VM_3		(byte)0x30	/* Non-Vectored Mode 3 */

#define	IC_MBITS	(byte)0x08	/* Bits to modify. Set=4:2, Clr=2:0 */
#define	IC_VIS		(byte)0x04	/* Vector Includes Status (1=true) */
#define	IC_A_MASTER	(byte)0x02	/* Channel A Master Enable (1=true) */
#define	IC_B_MASTER	(byte)0x01	/* Channel B Master Enable (1=true) */

/* Channel Command Register */

#define	RESET_TX	(byte)0x00	/* Reset the xmittr, clear FIFO, etc */
#define	RESET_TXCRC	(byte)0x01	/* Reset TX CRC */
#define	ENABLE_TX	(byte)0x02	/* Enable Transmitter */
#define	DISABLE_TX	(byte)0x03	/* Disable Transmitter */
#define	XMIT_SOM	(byte)0x04	/* Transmit SOM */
#define	XMIT_SOM_PAD	(byte)0x05	/* Transmit SOM with PAD */
#define	XMIT_EOM	(byte)0x06	/* Transmit EOM */
#define	XMIT_ABRK	(byte)0x07	/* Transmit Abort/Break */
#define	XMIT_DLE	(byte)0x08	/* Transmit DLE */
#define	ACT_ON_POLL	(byte)0x09	/* Go Active On Poll */
#define	RST_ACT_ON_POLL	(byte)0x0A	/* Reset Go Active On Poll */
#define	GO_ON_LOOP	(byte)0x0B	/* Go On-loop */
#define	GO_OFF_LOOP	(byte)0x0C	/* Go Off-loop */
#define	EXCLUDE_CRC	(byte)0x0D	/* Exclude from CRC */

#define	RESET_RX	(byte)0x40	/* Reset Receiver */
#define	ENABLE_RX	(byte)0x42	/* Enable Receiver */
#define	DISABLE_RX	(byte)0x43	/* Disable Receiver */

#define	CT_START	(byte)0x80	/* Start Counter/Timer */
#define	CT_STOP		(byte)0x81	/* Stop Counter/Timer */
#define	PRESET_FFFF	(byte)0x82	/* Preset Counter/Timer to FFFF */
#define	PRESET_CTPR	(byte)0x83	/* Preset C/T From CTPRH/CTPRL */

#define	ENTER_SRCH	(byte)0xC0	/* Enter Search Mode */
#define	DISABLE_DPLL	(byte)0xC1	/* Disable PLL */
#define	SET_FM		(byte)0xC2	/* Set FM/NRZ */
#define	SET_NRZ		(byte)0xC2	/* Set FM/NRZ */
#define	SET_NRZI	(byte)0xC3	/* Set NRZI */

/* DUSCC Register Accessors: */

# define SET_CMR1( val )	out08( p_pcb->scc_base + SCC_CM1, (val))
# define CMR1			in08 ( p_pcb->scc_base + SCC_CM1 )
# define SET_CMR2( val )	out08( p_pcb->scc_base + SCC_CM2, (val))
# define CMR2			in08 ( p_pcb->scc_base + SCC_CM2 )
# define SET_TPR( val )		out08( p_pcb->scc_base + SCC_TP, (val))
# define TPR			in08 ( p_pcb->scc_base + SCC_TP )
# define SET_OMR( val )		out08( p_pcb->scc_base + SCC_OM, (val))
# define OMR			in08 ( p_pcb->scc_base + SCC_OM )
# define RSR			in08 ( p_pcb->scc_base + SCC_RS )
# define WRITE_CCR( val )	out08( p_pcb->scc_base + SCC_CC, (val))
# define WRITE_TXFIFO( val )	out08( p_pcb->scc_base + SCC_TXFIFO, (val))
# define SET_CTPH( val )	out08( p_pcb->scc_base + SCC_CTPH, (val))
# define SET_CTPL( val )	out08( p_pcb->scc_base + SCC_CTPL, (val))
# define SET_CTC( val )		out08( p_pcb->scc_base + SCC_CTC, (val))
