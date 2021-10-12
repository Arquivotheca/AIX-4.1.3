/* @(#)45     1.47  src/bos/kernel/sys/POWER/mpqp.h, sysxmpqp, bos411, 9435B411a 8/31/94 18:17:04 */

#ifndef	_H_MPQP
#define	_H_MPQP
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqp.h - general header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#include <sys/comio.h>
#include <sys/types.h>
#include <sys/errids.h>
#include <sys/mbuf.h>

#define	MAX_CHAN    1		/* maximum number of channels per port*/
#define MPQP_RESOURCE_NAME "mpq"
#define ADAPTER_LCK 0xff
#define boolean unsigned char

#define RX_BUF_LEN	4090		/* size of receive buffer */
#define TX_BUF_LEN	4096		/* size of transmit buffer */

/**************************************************************************
 *   MPQP IOCTL	OPERATIONS						  *
 **************************************************************************/
#define	MP_SET_DELAY	0x00e1	/* Set NDELAY bit		*/
#define	Q_SDELAY	0x00e1	/* Set NDELAY bit		*/
#define	MP_CDELAY	0x00e2	/* Clear NDELAY	bit		*/
#define	Q_CDELAY	0x00e2	/* Clear NDELAY	bit		*/
#define	MP_START_DEV	0x00e3	/* Set parms and start port	*/
#define	Q_START_DEV	0x00e3	/* Set parms and start port	*/
#define	MP_HALT		0x00e4	/* Halt	port			*/
#define	Q_HALT		0x00e4	/* Halt	port			*/
#define	MP_START_AR	0x00e5	/* Start auto response		*/
#define	Q_START_AR	0x00e5	/* Start auto response		*/
#define	MP_STOP_AR	0x00e6	/* Stop	auto response		*/
#define	Q_STOP_AR	0x00e6	/* Stop	auto response		*/
#define	MP_CHG_PARMS	0x00e7	/* Change parameters		*/
#define	Q_CHG_PARMS	0x00e7	   /* Change parameters		   */
#define	MP_FLUSH_PORT	0x00e8	/* Flush queue			*/
#define	MP_GETSTAT	0x00e9	/* Query status			*/
#define	Q_GETSTAT	0x00e9	/* Query status			*/
#define	MP_RMEM		0x00ea	/* Read adapter memory		*/
#define	Q_RMEM		0x00ea	/* Write adapter memory		*/
#define	MP_WMEM		0x00eb	/* Read	adapter	memory		*/
#define	Q_WMEM		0x00eb	/* Read	adapter	memory		*/
#define	MP_RASW		0x00ec	/* Reload adapter software	*/
#define	Q_RASW		0x00ec	/* Reload adapter software	*/
#define	MP_QPCMD	0x00ed	/* Perform ioctl command	*/
#define	Q_QPCMD		0x00ed	/* Perform ioctl command	*/
#define	MP_TRACEON	0x00be	/* Turn	on port	activity trace	*/
#define	Q_TRACEON	0x00ee	/* Turn	on port	activity trace	*/
#define	MP_TRACEOFF	0x00bf	/* Turn	off port activity trace	*/
#define	Q_TRACEOFF	0x00ef	/* Turn	off port activity trace	*/
#define	MP_TRC_ON_ALL	0x00f0	/* Turn	on port	activity trace */
#define	Q_TRC_ON_ALL	0x00f0	/* Turn	on port	activity trace */
#define	MP_TRC_OFF_ALL	0x00f1	/* Turn	off port activity trace	*/
#define	Q_TRC_OFF_ALL	0x00f1	/* Turn	off port activity trace	*/
#define	MP_PQUERY	0x00f2	/* Retrieve port trace data	*/
#define	Q_PQUERY	0x00f2	/* Retrieve port trace data	*/
#define	MP_AQUERY	0x00f3	/* Retrieve adapter trace data	*/
#define	Q_AQUERY	0x00f3	/* Retrieve adapter trace data	*/
#define	MP_GETIC	0x00f4	/* Get interrupt count		*/
#define	Q_GETIC		0x00f4	/* Get interrupt count		*/
/**************************************************************************
 *   MPQP IOCTL	COMMANDS						  *
 **************************************************************************/

/* Read	and write ioctls... */

#define	DIAG_XMT_LNG	0x11		/* Diagnostic transmit long */
#define	DIAG_XMT_GTHR	0x12		/* Diagnostic transmit gather */
#define	DIAG_READ	0x30		/* Diagnostic read by ioctl */
#define	DIAG_WRITE	0x31		/* Diagnostic write by ioctl */

/**************************************************************************
 *   MPQP IOCTL	DIAGNOSTIC/SETUP COMMANDS				  *
 **************************************************************************/
/* Adapter Diagnostic Commands */

#define	ROS_QUIK_TEST	0xf0		/* ROS type quick test		*/
#define	RAM_EXT_TEST	0xe0		/* RAM extended	test		*/
#define	MEMORY_CKSUM	0xf1		/* Memory checksum test		*/
#define	ROS_CPU_AU_TEST	0xf2		/* ROS CPU auto	test		*/
#define	CIO_ROS_AU_TEST	0xf4		/* CIO ROS Auto	Test		*/
					/*  Extended Test		*/
#define	SCC_ROS_AU_TEST	0xf5		/* SCC ROS Auto	Test		*/
#define	ROS_IFACE_TEST	0xf6		/* ROS SSTIC4 auto test		*/
#define	PORT_DMA_TST	0xe8		/* Port	DMA Test		*/
#define	BUS_MSTR_DMA	0xec		/* INT1	Interrupt Test		*/
#define	STRT_CARD_RST	0xff		/* Start Card Reset		*/

/* Adapter Setup and Status Commands */

#define	RTN_MEM_SIZE	0xd0		/* Return Memory Size		*/
#define	GET_INTF_ID	0xd1		/* Get Interface ID		*/
#define	GET_EIB_ID	0xd2		/* Get EIB ID			*/
#define	CFG_CIO_PORT	0xd3		/* Configure CIO Port		*/
#define	CFG_SCC_CHNL	0xd4		/* Configure SCC Channel	*/
#define	CFG_DMA_CHNL	0xd5		/* Configure DMA Channel	*/
#define	CFG_HDW_TMR	0xd6		/* Configure Hardware		*/
					/*   Timer			*/
#define	INIT_WD_TMR	0xd7		/* Initialize Watchdog		*/
					/*   Timer			*/
#define	PRIORITY_SWTCH	0xd8		/* Priority Switch		*/
#define	GET_ROS_VRSN	0xd9		/* Get ROS Version		*/

#define	CPU_TMR_QRY	0xb1		/* CPU Timer Query	 */
#define	CPU_DMA_QRY	0xb2		/* CPU DMA Query	 */
#define	DATA_BLASTER	0xbb		/* Data	Blaster	Test	 */
#define	SCC_DIAG_SETUP	0xbc		/* SCC Diagnostic Setup	 */
#define	SER_REG_ACC	0xbd		/* Serial Register Access */
#define	TRACE_ENBL	0xbe		/* Enable Trace	Facility */
#define	TRACE_DISABL	0xbf		/* Disable Trace Facility */
#define	IO_READ		0xde		/* IO Read		  */
#define	IO_WRITE	0xdf		/* IO Write		  */

/* Port	Specific Diagnostic Commands */

#define	DUSCC_REG_QRY	0xc0		/* DUSCC Register Query	 */
#define	CIO_REG_QRY	0xc1		/* CIO Register	Query	 */
#define	DMA_REG_QRY	0xc2		/* DMA Register	Query	 */
#define	QUERY_MDM_INT	0x2a		/* Query Modem Interrupts */

/**************************************************************************
 *   MPQP Trace	Indicator Flags						  *
 **************************************************************************/
#define	Q_T_RQE		0		/* Entry is a response queue elt */
#define	Q_T_CMD		1		/* Entry is a port command	 */
/**************************************************************************
 *   MPQP SPECIFIC ERRNO VALUES						  *
 **************************************************************************/
/* Note: these two values for errno are	currently in use but not	  *
 *	 in the	context	of diagnostics.	 We chose them because		  *
 *	 we needed a method of passing specific	adapter	related		  *
 *	 information to	the user task within the context of		  *
 *	 fairly	standard AIX V3	system call interface.			  *
 **************************************************************************/
#define	ETSTFL	       25		/* diagnostic test failed */
#define	EBADRRP	       26		/* diagnostic test failed */
					/* invalid EDRR	pointer	  */
/**************************************************************************
 *   MPQP ERROR	SYMBOLIC CONSTANTS					  *
 **************************************************************************/
#define	M_E_NOADAPTER	1	/* no MPQP adapters present    */
#define	M_E_BADADAINIT	2	/* adapter initialization failed */
#define	M_E_BADPRTINIT	3	/* port	initialization failed  */
#define	M_E_IMINDEVNO	100	/* illegal minor device	number */
/**********************************************************************
 *    Miscellaneous Ioctl Defines				      *
 **********************************************************************/
#define	NUM_TRACE_ELEM		256	/* number of port trace	elements */
#define	SELECT_SIG_LEN		256	/* num of selection signals allowed */
#define	PHYS_V25		0x01	/* V25 mask    */
#define	PHYS_SM			0x02	/* Smart modem */
#define	PHYS_X21		0x04	/* X21 mask    */
#define	PHYS_V35		0x08	/* V35 mask    */
#define	PHYS_422		0x10	/* 422 mask    */
#define	PHYS_232		0x20	/* 232 mask    */
#define	BSC_MASK		0x01
#define	SDLC_MASK		0x02
#define	ASYNC_MASK		0x04
#define	PAR_ENBL		0x80	/* parity enable */
#define	PAR_SELECT		0x40	/* parity select */
#define	BSC_ASCII		0x01	/* Bisync ASCII	*/
#define	SDLC_NRZI		0x01	/* SDLC	NRZI mode mask */
#define	FD_MASK			0x01	/* full	duplex mask	*/
#define	DR_MASK			0x02	/* data	rate select mask */
#define	CR_MASK			0x04	/* control RTS mask	*/
#define	DD_MASK			0x08	/* dial	data mask	*/
#define	CDSTL_MASK		0x10	/* clear data set to line mask */
#define	AUTO_MASK		0x20	/* auto	for switched only */
#define	CALL_MASK		0x40	/* call	switched only	  */
#define	SWITCHED		0x00	/* switched line */
#define	LEASED_MASK		0x80	/* leased		*/
#define	STOP_BIT		0x30	/* for checking	stop bit */
#define	FIELD_SELECT_RESET	0x00    /* initial value for field select */
#define FIELD_SELECT_SET	0xbf    /* set value for field select */
#define FS_RCV_TMR		0x80	/* receive timer bit mask 	*/
#define	FS_MDM_INT_MSK		0x40    /* Modem interrupt mask      	*/
#define FS_PL			0x20	
#define FS_POLL_ADDR		0x10	/* Poll address bit mask	*/
#define FS_SEL_ADDR		0x08	/* Select address bit mask-bisync only*/
#define FS_AP			0x04	
#define FS_DP			0x02	
#define FS_BR			0x01	
#define	BAUD_RATE_MAX		0x99e8  /* max baud rate allow 39400 */

/* The following definitions are used by chg_mask in change parameters and 
   should have the same values as their field select counterparts above.  */

#define	CP_RCV_TMR	  (int)	0x00000080    /* receive timer bit mask	*/
#define	CP_POLL_ADDR	  (int)	0x00000010    /* Poll address bit mask	*/
#define	CP_SEL_ADDR	  (int)	0x00000008    /* Select	addr mask-bisync only*/

/*****************************************************************************/
/* tracing macros 							     */
/*****************************************************************************/
void MPQSaveTrace (int, char *, int, int, int, int);

#define DD_MPQ_HOOK         0x21
#define MPQTRACE1(a1) \
        MPQSaveTrace(1, a1, 0, 0, 0, 0)
#define MPQTRACE2(a1,a2) \
        MPQSaveTrace(2, a1, (int)a2, 0, 0, 0)
#define MPQTRACE3(a1,a2,a3) \
        MPQSaveTrace(3, a1, (int)a2, (int)a3, 0, 0)
#define MPQTRACE4(a1,a2,a3,a4) \
        MPQSaveTrace(4, a1, (int)a2, (int)a3, (int)a4 , 0)
#define MPQTRACE5(a1,a2,a3,a4,a5) \
        MPQSaveTrace(5, a1, (int)a2, (int)a3, (int)a4, (int)a5)

/*****************************************************************************/
/* Mbuf tracing macros                                                       */ 
/*****************************************************************************/
#define MBUFTRACE1(a1)       MPQSaveTrace(1, a1, 0, 0, 0, 0)
#define MBUFTRACE2(a1,a2)    MPQSaveTrace(2, a1, (int)a2, 0, 0, 0)
#define MBUFTRACE3(a1,a2,a3) MPQSaveTrace(3, a1, (int)a2, (int)a3, 0, 0)
#define MBUFTRACE4(a1,a2,a3,a4) \
        MPQSaveTrace(4, a1, (int)a2, (int)a3, (int)a4 , 0)
#define MBUFTRACE5(a1,a2,a3,a4,a5) \
        MPQSaveTrace(5, a1, (int)a2, (int)a3, (int)a4, (int)a5)

/*************************************************************************
 *    MPQP Status Block	Code Values					 *
 *************************************************************************/

#define	 MP_ERR_THRESHLD_EXC		0x82
#define  MP_END_OF_AUTO_RESP		0x2C
#define	 MP_RDY_FOR_MAN_DIAL		0x2210

/**********************************************************************
 *    MPQP Status Block	Option[0] Values			      *
 **********************************************************************/

/* Option [0] values when code = CIO_START_DONE or MP_ERR_THRESHLD_EXC */

#define	MP_CTS_TIMEOUT		0x15
#define	MP_RX_ABORT		0x18
#define	MP_RX_DMA_BFR_ERR	0x1C
#define MP_RESET_CMPL 		0x20 	/* Reset Completed */ 
#define	MP_X21_TIMEOUT		0x21	/* X.21	timer expired	  */
#define	MP_ADAP_NOT_FUNC	0x30	/* Adapter not functioning  */
#define MP_TOTAL_TX_ERR         0x31
#define MP_TOTAL_RX_ERR         0x32
#define MP_TX_PERCENT           0x33
#define MP_RX_PERCENT           0x34
#define MP_DSR_ALRDY_ON		0x40
#define	MP_DSR_DROPPED		0x41
#define MP_ASY_LOST_RTS         0x42
#define	MP_TX_UNDERRUN		0x89
#define	MP_CTS_UNDERRUN		0x88
#define	MP_DSR_ON_TIMEOUT	0xA1	/* DSR fails to come on	*/
#define	MP_RCV_TIMEOUT		0xA7
#define	MP_AR_RCV_TIMEOUT	0xA8
#define	MP_TX_FAILSAFE_TIMEOUT	0xB1 	/* Transmit command did not complete */
#define	MP_X21_RETRIES_EXC	0xCE	/* Retries exceeded call not compl.  */
#define	MP_X21_CLEAR		0xD2	/* Unexpected Clear received from DCE*/
#define MP_BUF_STAT_OVFLW       0x1001
#define	MP_RX_FRAME_ERR		0xC001
#define MP_RX_BSC_FRAME_ERR     0xA001
#define MP_RX_BSC_PAD_ERR       0xA002
#define	MP_RX_OVERRUN		0x8001
#define	MP_RX_PARITY_ERR	0x8002
#define MP_FRAME_CRC            0x8003
#define MP_LOST_SYNC            0x8004
#define MP_SDLC_ABORT		0x9001
#define MP_SDLC_SHRT_FRM        0x9002
#define MP_SDLC_RESIDUAL        0x9003
#define MP_X21_INVALID_CPS      0xB001
#define MP_ASW_OVERWRITE        0xD001   /* ASW overwitten      */
#define MP_BAD_RX_DMA_Q_AD      0xD002   /* bad address on DMA Q*/
#define MP_LOST_COMMAND         0xD003   /* lost PCQ command    */

/**********************************************************************
 *    MPQP Status Block	Option[1] Values			      *
 **********************************************************************/

/* The following timer indicators will be passed in Option[1] when */
/* the code is CIO_START_DONE and Option[0] = MP_X21_TIMEOUT	   */

#define MP_XT1_TIMER		0xc1	/* X.21 Timer that expired */
#define MP_XT2_TIMER		0xc2	/* X.21 Timer that expired */
#define MP_XT3_TIMER		0xc3	/* X.21 Timer that expired */
#define MP_XT4_TIMER		0xc4	/* X.21 Timer that expired */
#define MP_XT5_TIMER		0xc5	/* X.21 Timer that expired */
#define MP_XT6_TIMER		0xc6	/* X.21 Timer that expired */
#define MP_X_RETRY1_TIMER	0xc7	/* X.21 Timer that expired */
#define MP_X_RETRY2_TIMER	0xc8	/* X.21 Timer that expired */
#define MP_X_GR0_TIMER		0xc9	/* X.21 Timer that expired */
#define MP_X_LSD_TIMER		0xca	/* X.21 Timer that expired */
#define MP_X_DCE_READY_TIMER	0xcb	/* X.21 Timer that expired */
#define MP_X_24B_TIMER		0xCC	/* X.21 Timer that expired */

/* end of X.21 timer indicators */

#define	MP_FORCED_HALT		0x08	/* Halt	due adapter or network errors */
#define	MP_NORMAL_HALT		0x09	/* Normal Halt */

/**********************************************************************
 *    MPQP Status Block	Option[2] Values			      *
 **********************************************************************/

#define	MP_NETWORK_FAILURE	0x0a	/* Network failure */
#define	MP_HW_FAILURE		0x0b	/* Hardware failure */

/***************************************************************************
 *		   MPQP	Physical link					   *
 ***************************************************************************/

#define	 PL_232D	0x20	  /*   EIA232-D		   */
#define	 PL_422A	0x10	  /*   EIA422-A		   */
#define	 PL_V35		0x08	  /*   V.35		   */
#define	 PL_X21		0x04	  /*   X.21		   */
#define	 PL_SMART_MODEM	0x02	  /*   Smart modem	   */
#define	 PL_V25		0x01	  /*   V25bis		   */

/*************************************************************************
 *		 MPQP STATUS CONSTANTS					 *
 *************************************************************************/

#define	MP_BUF_OVERFLOW		0x8001	  /* Receive buffer overflow */
#define	MP_X21_CPS		0x8002	  /* X.21 call progress	signal*/
#define	MP_X21_DPI		0x8003	  /*X.21 DCE Provided Information
						 (For Network data)  */
#define	MP_MODEM_DATA		0x8004	  /* Modem data	(ie. autodial
						   modem data */
#define	MP_AR_DATA_RCVD		0x8005	  /* Data receive while	in
						   Auto-response */
#define	MP_AR_END		0x8006	  /* Auto-response terminated */
#define	MP_AR_RX_TIMEOUT	0x8007	  /* Auto-response rx. timeout */

/***************************************************************************
 *			BISYNC Errors and Message types			   *
 ***************************************************************************/

#define MP_ETB		(unsigned short)0x2001	/* --- ETB : 0 1 */
#define MP_ETX		(unsigned short)0x2002	/* --- ETX : 0 2 */
#define MP_ACK0		(unsigned short)0x2008	/* --- AK0 : 0 8 */
#define MP_ACK1		(unsigned short)0x2009	/* --- AK1 : 0 9 */
#define MP_WACK		(unsigned short)0x200A	/* --- WAK : 0 A */
#define MP_NAK		(unsigned short)0x200B	/* --- NAK : 0 B */
#define MP_ENQ 		(unsigned short)0x200C	/* --- ENQ : 0 C */
#define MP_EOT 		(unsigned short)0x200D	/* --- EOT : 0 D */
#define MP_RVI 		(unsigned short)0x200E	/* --- RVI : 0 E */
#define MP_DISC		(unsigned short)0x200F	/* --- DSC : 0 F */
#define MP_STX_ITB	(unsigned short)0x2020	/* STX ITB : 2 0 */
#define MP_STX_ETB	(unsigned short)0x2021	/* STX ETB : 2 1 */
#define MP_STX_ETX	(unsigned short)0x2022	/* STX ETX : 2 2 */
#define MP_STX_ENQ	(unsigned short)0x202C	/* STX ENQ : 2 C */
#define MP_SOH_ITB	(unsigned short)0x2030	/* SOH ITB : 3 0 */
#define MP_SOH_ETB	(unsigned short)0x2031	/* SOH ETB : 3 1 */
#define MP_SOH_ETX	(unsigned short)0x2032	/* SOH ETX : 3 2 */
#define MP_SOH_ENQ	(unsigned short)0x203C	/* SOH ENQ : 3 C */
#define MP_DATA_ACK0	(unsigned short)0x2088	/* dat AK0 : 8 8 */
#define MP_DATA_ACK1	(unsigned short)0x2089	/* dat AK1 : 8 9 */
#define MP_DATA_NAK	(unsigned short)0x208B	/* dat ETB : 8 B */
#define MP_DATA_ENQ	(unsigned short)0x208C	/* dat ENQ : 8 C */

/**************************************************************************
 *		   MPQP	Auto-dial Protocol				  *
 **************************************************************************/
#define	DIAL_PRO_BSC	0x01	       /* same as BSC_MASK */
#define	DIAL_PRO_SDLC	0x02	       /* same as SDLC_MASK */
#define	DIAL_PRO_ASYNC	0x08	       /* same as ASYNC_MASK */

/****************************************************************************
 *		   MPQP	dial flags  for	ASCII				    *
 ****************************************************************************/

#define	DIAL_FLG_PAR_EN		0x80	  /* Parity enable/Parity Select   */
#define	DIAL_FLG_PAR_ODD	0x40	  /* Odd parity	for ASCII */
					  /* This value	is logical OR with
					     dial_flags	to set odd parity */
#define	DIAL_FLG_PAR_EVEN	0xbf	  /* even parity for ASCII */
					  /* This value	is logical AND	with
					   dial_flags to set even parity bit */
#define	DIAL_FLG_BSC_ASC	0x01	  /* ASCII Bisync	   */

/****************************************************************************
 *		   MPQP	dial flags  for	SDLC				    *
 ****************************************************************************/

#define	 DIAL_FLG_NRZ		0xef	  /* Dial flag for SDLC	NRZ	     */
					  /* This value	must be	logical	AND
					     with dial_flags to	set NRZ	bit */
#define	 DIAL_FLG_NRZI		0x01	  /* Dial flag for SDLC	NRZI	     */
					  /* This value	must be	logical	OR with
					     dial_flags	to set NRZI bit	on */


/****************************************************************************
 *		   MPQP	dial flags  for	ASYNC				    *
 ****************************************************************************/

#define	 DIAL_FLG_STOP_CLEAR	0xf3
#define	 DIAL_FLG_STOP_0	0x00	  /* Zero stop bit    */
#define	 DIAL_FLG_STOP_1	0x04	  /* One stop bit     */
#define	 DIAL_FLG_STOP_15	0x08	  /* 1.5 stop bits    */
#define	 DIAL_FLG_STOP_2	0x0c	  /* 2 stop bits      */
#define	 DIAL_FLG_CHAR_CLEAR	0xfc
#define	 DIAL_FLG_CHAR_5	0x00	  /* 5 bits/char      */
#define	 DIAL_FLG_CHAR_6	0x01	  /* 6 bits/char      */
#define	 DIAL_FLG_CHAR_7	0x02	  /* 7 bits/char      */
#define	 DIAL_FLG_CHAR_8	0x03	  /* 8 bits/char      */
#define	 DIAL_FLG_C_CARR_ON	0x20	  /* Continuous	Carrier	RTS always ON */
#define	 DIAL_FLG_C_CARR_OFF	0xdf	  /* RTS diabled between transmission */
					  /* This value	must be	AND with dial
					     flags to turn it off */
#define	 DIAL_FLG_TX_NO_CTS	0xef	  /* Transmit without waiting for CTS */
#define	 DIAL_FLG_TX_CTS	0x10	  /* Wait for CTS to transmit	      */

/**************************************************************************
 *		   MPQP	data protocol					  *
 **************************************************************************/
#define	 DATA_PROTO_BSC			0x01
#define	 DATA_PROTO_SDLC_FDX		0x02
#define	 DATA_PROTO_SDLC_HDX		0x04
#define	 DATA_FLG_BSC_ASC		0x01
#define	 DATA_FLG_BSC_TRANSP		0x04  /* BSC transparent mode */
#define	 DATA_FLG_NRZI			0x01
#define	 DATA_FLG_NRZ			0xfe
#define	 DATA_FLG_ADDR_CHK		0x02
#define	 DATA_FLG_RST_TMR		0x10
#define	 DATA_FLG_C_CARR_ON		0x20	  /* Continuous	Carrier */
#define	 DATA_FLG_C_CARR_OFF		0xdf	  /* Controlled Carrier */

/**************************************************************************
 *	      MPQP Modem Flags						  *
 **************************************************************************/
#define	 MF_LEASED			0x80  /* this value must be logical OR
					  with modem flags to set leased line */
#define	 MF_SWITCHED			0x7f  /* This value must be logical AND
					with modem flags to set	switched line */
#define	 MF_LISTEN			0xbf  /* This value must be logical AND
					with modem flags to set	listen option */
#define	 MF_CALL			0x40  /* This value must be logical OR
					with modem flags to set	call option */
#define	 MF_AUTO			0xdf /*	This value must	be logical AND
					with modem flags to set	auto mode */
#define	 MF_MANUAL			0x20 /*	This value must	be logical OR
					with modem flags to set	manual mode */
#define	 MF_DIAL_DATA			0x08 /*	this value must	be logical OR
					with modem flags to indicate buffer has
					dial data */
#define	 MF_NO_DIAL_DATA		0xf7 /*	This value must	be logical ANDed
						with modem flags to turn DD bit
						off */
#define	 MF_CDSTL_ON			0x10  /* Enable	DTR without waiting for
					       for Ring	Indicator (RI)	    */
					      /* this value must be logical OR
					with modem flags to CDSTL on */
#define	 MF_CDSTL_OFF			0xef  /* Enabel	DTR after Ring Indicator
					       occurs	   */
					      /* This value must be logical AND
					with modem flags for CDSTL off */
#define	 MF_DRS_ON			0x02	  /* Enable data rate select */
					    /* This value must be logical OR
					with modem flags to select rate	*/
#define	 MF_DRS_OFF			0xfd /*	Do not enable data rate	select*/
					     /*	This value must	be logical AND
					with modem flags to Set	DRS off	*/


/******* these defines are for trace hooks   *******************/

#define	ADAPT_TOO_BIG		0x000000f0   /*	adapter	number is too big */
#define	NO_ACB			0x000000f1   /*	acb is NULL		  */
#define	NO_OFFL_INTR		0x000000f2   /*	there is no offlevel intr
						  structure		   */
#define	NO_INTR_REG		0x000000f3   /*	no interrupt registration */
#define	NO_PORT_DDS		0x000000f4   /*	no dds for requested port */
#define	CHAN_TOO_BIG		0x000000f5   /*	channel	> CHAN_MAX	  */
#define	CHAN_BUSY		0x000000f6   /*	channed	is used	by kernel */
#define	NO_MBUF_AVAIL		0x000000f7   /*	no mbuf	available	  */
#define	NO_XMIT_CHAIN		0x000000f8   /*	no transmit chain	  */
#define	ADAPT_ALRDY_OPEN	0x000000f9   /*	adapter	already	opened	  */
#define	UIO_MOVE_ERR		0x000000fd   /*	error in uiomove	  */
#define	PHYS_LINK_INV		0x000000e0   /*	physical link is invalid  */
#define	DATA_PROTO_INV		0x000000e1   /*	data protocol is invalid  */
#define	BAUD_RATE_INV		0x000000e2   /*	baud rate is invalid	  */
#define	NO_ERROR		0x000000e3
#define	PARM1			0x00000001   /*	parameter # 1	 */
#define	PARM2			0x00000002   /*	parameter # 2	 */
#define	PARM3			0x00000003   /*	parameter # 3	 */
#define	PARM4			0x00000004   /*	parameter # 4	 */

/**************************************************************************
 *	      MPQP values for retry groups				  *
 **************************************************************************/
#define	 CG_SIG_0	0x8000
#define	 CG_SIG_1	0x4000
#define	 CG_SIG_2	0x2000
#define	 CG_SIG_3	0x1000
#define	 CG_SIG_4	0x0800
#define	 CG_SIG_5	0x0400
#define	 CG_SIG_6	0x0200
#define	 CG_SIG_7	0x0100
#define	 CG_SIG_8	0x0080
#define	 CG_SIG_9	0x0040


/******************************************************************************
 *   MPQP Select Queue Info Structure Template				   *
 ***************************************************************************/
typedef	struct SELQUEUE
{

    struct SELQUEUE	*p_sel_que;		/* link	pointer	*/
    struct status_block	stat_block;		/* status block	structre */
						/* from	comio.h		 */
    unsigned long	rqe_value;

} t_sel_que;

/***************************************************************************
 *   MPQP Channel Information Structure	Template			   *
 ***************************************************************************/

typedef	struct MPQCHANINFO
{

    dev_t		devno;		/* device number */
    unsigned long	devflag;	/* device flags	from open */
    unsigned short	sync_flags;	/* selnotify flags for events */
    int			rcv_event_lst;	/* event list anchor for receive */
    int			xmt_event_lst;	/* event list anchor for transmit */
    struct kopen_ext	mpq_kopen;	/* struct kopen_ext passed to open */
    t_sel_que		*p_beg_page;	/* beginning of	select queue page */
    t_sel_que		*p_sel_avail;	/* head	of available linked list */
    t_sel_que		*p_rcv_head;	/* head	of receive available queue */
    t_sel_que		*p_rcv_tail;	/* tail	of receive available queue */
    t_sel_que		*p_stat_head;	/* head	of status available queue */
    t_sel_que		*p_stat_tail;	/* tail	of status available queue */
    t_sel_que		*p_lost_stat;	/* queue element for CIO_LOST_STATUS */
    t_sel_que		*p_lost_rcv;	/* queue element for LOST_DATA */

} t_chan_info;		/* end of channel information structure	*/

typedef	struct
{
	char	retry_cnt;
	char	rsv	 ;
	ushort	retry_delay;
	ushort	group_0;
	ushort	group_1;
	ushort	group_2;
	ushort	group_3;
	ushort	group_4;
	ushort	group_5;
	ushort	group_6;
	ushort	group_7;
	ushort	group_8;
	ushort	group_9;
	ushort	netlog_0;
	ushort	netlog_1;
	ushort	netlog_2;
	ushort	netlog_3;
	ushort	netlog_4;
	ushort	netlog_5;
	ushort	netlog_6;
	ushort	netlog_7;
	ushort	netlog_8;
	ushort	netlog_9;
	char	gr0_thresh;
	char	gr1_thresh;
	char	gr2_thresh;
	char	gr3_thresh;
	char	gr4_thresh;
	char	gr5_thresh;
	char	gr6_thresh;
	char	gr7_thresh;
	char	gr8_thresh;
	char	gr9_thresh;
} t_cps_data;

typedef struct
{
	unsigned short		len;
	char			sig[SELECT_SIG_LEN];
	t_cps_data		cps;
} t_x21_data;

typedef	struct
{
	unsigned short		len;
	char			sig[SELECT_SIG_LEN];  /*   dial string 	      */
				    /* when using v.25 bis: 		      */
				    /*   bisync requires STX and ETX,	      */
				    /*   SDLC requires address, control fields*/
	unsigned short		connect_timer;
	unsigned short		v25b_tx_timer;
} t_auto_data;

typedef	struct
{
	union
	{
		t_x21_data 		x21;
		t_auto_data 		autod;
	} t_dial;
} t_adap_dial_data;

#define	DIAL_LEN		sizeof(t_adap_dial_data)/* len of dial data */

/*
 * errmsg:
 *	error logger structure
 */
typedef struct errmsg {
	struct  err_rec0 err;
	char    file[32];
        int     data1;       /* use data1 and data2 to show detail  */ 
        int     data2;       /* data in the errlog report. Define   */
	                     /* these fields in the errlog template */
	                     /* These fields may not be used in all */
	                     /* cases.                              */
} errmsg;

/***************************************************************************
 *   MPQP Threshold Indicators						   *
 ***************************************************************************/
typedef	struct T_ERR_THRESH {
	unsigned long tx_err_thresh;
	unsigned long rx_err_thresh;
	unsigned long tx_err_percent;
	unsigned long rx_err_percent;
  	unsigned long tx_underrun_thresh;	/* tx_underrun threshold      */
  	unsigned long tx_cts_drop_thresh;	/* cts_underrun threshold     */
  	unsigned long tx_cts_timeout_thresh;	/* cts_timeout threshold      */
  	unsigned long tx_fs_timeout_thresh;	/* tx_timeout threshold       */
  	unsigned long rx_overrun_err_thresh;	/* rx_overrun threshold       */
  	unsigned long rx_abort_err_thresh;	/* abort_detect threshold     */
  	unsigned long rx_frame_err_thresh;	/* short_frame,asy_framing,   */
						/* bsc frame errors	      */
  	unsigned long rx_par_err_thresh;	/* asy_parity, bsc_parity     */
  	unsigned long rx_dma_bfr_err_thresh;	/* buffer_overflow threshold  */
} t_err_threshold;

/*
 *  Multi Protocol Quad	Port start device data structure Definition
 */
typedef	struct ST_DEV_PARM
{
    struct session_blk	mpqp_session;
    unsigned char	phys_link;		/* physical link	*/
    unsigned char	dial_proto;		/* dial	protocol	*/
    unsigned char	dial_flags;		/* dial	protocol	*/
    unsigned char	data_proto;		/* protocol in data transfer */
    unsigned char	data_flags;		/* protocol flags for data */
    unsigned char	modem_flags;		/* modem flags		*/
    unsigned char	poll_addr;		/* poll	address		*/
    unsigned char	select_addr;		/* select address	*/
    unsigned char	modem_intr_mask;	/* currently unused	*/
    unsigned short	baud_rate;		/* baud	rate		*/
    unsigned short	rcv_timeout;		/* receive time	out	*/
    unsigned short	rcv_data_offset;	/* receive data offset - */
						/* (reserved) 	 	 */
    union
    {
	t_x21_data	x21_data;
	t_auto_data	auto_data;
    } t_dial_data;
    t_err_threshold	*p_err_threshold;
} t_start_dev ;

/*
 *  Change parameters data structure definition
 */
typedef	struct T_CHG_PARMS
{
    int		   chg_mask	  ;	    /* mask for	change parms	  */
    unsigned short rcv_timer	  ;	    /* receive timer		  */
    unsigned char  poll_addr	  ;	    /* poll address		  */
    unsigned char  select_addr    ;	    /* select address		  */
} t_chg_parms ;

/*
 *   start auto	response parameter data	structure definition
 */

typedef	struct T_STRT_AR_PARMS
{
    short     rcv_timer		  ;	    /* receive timer		  */
    char      tx_rx_addr	  ;	    /* transmit	, receive address */
    char      tx_cntl		  ;	    /* transmit	control		  */
    char      rx_cntl		  ;	    /* receive control		  */
} t_start_ar;

/***************************************************************************
 *   MPQP Threshold Accumulators					   *
 ***************************************************************************/
typedef	struct T_CIO_STATS
{
    unsigned long tx_byte_mcnt;	        /* MSB of transmit byte	counter      */
    unsigned long tx_byte_lcnt;	        /* LSB of transmit byte	counter      */
    unsigned long rx_byte_mcnt;	        /* MSB of receive byte counter	     */
    unsigned long rx_byte_lcnt;	        /* LSB of receive byte counter       */
    unsigned long tx_frame_mcnt;	/* MSB of transmit frames counter    */
    unsigned long tx_frame_lcnt;	/* LSB of transmit frames counter    */
    unsigned long rx_frame_mcnt;	/* MSB of receive frames counter     */
    unsigned long rx_frame_lcnt;	/* LSB of receive frames counter     */

            /* MPQP device specific threshold counters */

    unsigned long tx_dma;               /* number of bus master dma transmit */
    unsigned long tx_dmabytes;          /* number of bytes of transmit dma   */
    unsigned long tx_short;             /* number of times of transmit short */
    unsigned long tx_shortbytes;        /* number of bytes of transmit short */
    unsigned long rx_dma;               /* number of receive dma             */
    unsigned long tx_err_cnt;           /* number of transmit errors         */
    unsigned long rx_err_cnt;           /* number of receive errors          */
    unsigned long tx_err_dcnt;          /* number of tx_err's since last     */
					/* tx_err threshold was met          */
    unsigned long rx_err_dcnt;          /* number of rx_err's since last     */
					/* rx_err threshold was met          */
    unsigned long tx_underrun;          /* number of bytes underrun          */
    unsigned long rx_overrun;           /* number of bytes overrun           */
    unsigned long tx_timeout;           /* numnber of timestx timeout error  */
    unsigned long rx_timeout;           /* number of times rx timeout        */
    unsigned long cts_timeout;          /* number of clear to send timeout   */
    unsigned long dsr_timeout;          /* number of data set ready timeout  */
    unsigned long cts_underrun;         /* number of clear to send dropped   */
    unsigned long dsr_dropped;          /* number of data set ready dropped  */
    unsigned long dsr_offtimeout;       /* number of data set ready timeout  */
    unsigned long buffer_overflow;      /* number of times buffer overflow   */
    unsigned long abort_detect;         /* number of abort detect	     */
    unsigned long short_frame;		/* number of SDLC short frames	     */
    unsigned long CRC_error;            /* number of CRC errors		     */
    unsigned long bsc_parity;           /* number of parity error ASCII only */
    unsigned long asy_framing;		/* number of asc or bsc frame errors */
    unsigned long asy_parity;		/* number of async parity errors     */
    unsigned long x21_stat;		/* number of x21 stat. errors        */
    boolean  clear_counters;            /* counter will be cleared           */
				        /* upon completion of call           */
} t_cio_stats ;

/***************************************************************************
 *   MPQP Threshold QUERY Structure (CIO_QUERY) 			   *
 ***************************************************************************/
typedef	struct T_QUERY_PARMS
{
    unsigned long  status;	/* out status */
    t_cio_stats	   *bufptr;	/* in -	destination buffer pointer */
    int		   buflen;	/* in -	destination buffer len ( bytes)	*/
    unsigned long  reserve[4];	/* reserved */
} t_query_parms	;

/***************************************************************************
 *   MPQP Threshold Pointers and Contants			           *
 ***************************************************************************/
#define DDS_STATS         p_dds->dds_ras.cio_stats
#define DDS_THRESH        p_dds->dds_ras.err_thresh
#define THRES_UNSOL_STAT  (unsigned short)0x0001 /* Unsol status from adapter */
#define THRES_TX_ERR      (unsigned short)0x0002 /* Transmission errors */
#define THRES_RX_ERR      (unsigned short)0x0003 /* Receive errors */

typedef	struct mp_write_extension
{
    struct write_extension	cio_write;   /* COMIO write extension */
    unsigned char		transparent; /* bisync transparent mode flag */
} t_write_ext;

#endif /* _H_MPQP */
