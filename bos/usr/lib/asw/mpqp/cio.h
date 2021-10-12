/*
static char sccsid[] = "@(#)26  1.3  src/bos/usr/lib/asw/mpqp/cio.h, ucodmpqp, bos411, 9428A410j 1/30/91 17:17:42";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: cio.h - Include file for CIO bit mask defines 
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

typedef struct S_CIO_REGS
{
	unsigned char		data;
	unsigned char		reserved_1;
} t_cio_reg;

/* CIO register offset */
#define MIC		(unsigned int)0		/* Master interrupt control */
#define MCR		(unsigned int)2		/* Master configuration */
#define P_A_IV		(unsigned int)4		/* Port A Interrupt vector */
#define P_B_IV		(unsigned int)6		/* Port B interrupt vector */
#define CT_IV		(unsigned int)8		/* C/T Interrupt vector */
#define P_C_DPP		(unsigned int)10	/* Port C data path polarity */
#define P_C_DDR		(unsigned int)12	/* Port C data direction */
#define P_C_SRC		(unsigned int)14	/* Port C special IO control */
#define P_A_CS		(unsigned int)16	/* Port A Command/Status */
#define P_B_CS		(unsigned int)18	/* Port B Command/Status */

#define	CS_CLR_I	(unsigned char)0x20	/* Clear IP, IUS bits */
#define	CS_EN_I		(unsigned char)0xC0	/* Enable interrupts */
#define	CS_DIS_I	(unsigned char)0xE0	/* Disable interrupts */
#define	CS_ERR_I	(unsigned char)0x01	/* Interrupt on error? */

#define T_1_CS		(unsigned int)20	/* C/T 1 Command/Status */
#define T_2_CS		(unsigned int)22	/* C/T 2 Command/Status */
#define T_3_CS		(unsigned int)24	/* C/T 3 Command/Status */
#define P_A_DATA	(unsigned int)26	/* Port A Data */
#define P_B_DATA	(unsigned int)28	/* Port B Data */
#define P_C_DATA	(unsigned int)30	/* Port C Data */

/* Port data register offsets */

#define	CIOCH_A		(unsigned int)64
#define	CIOCH_B		(unsigned int)80

#define	P_MODE		(unsigned int)0		/* Mode Specification */
#define	P_SHAKE		(unsigned int)2		/* Handshaking Specification */
#define	P_DPPOL		(unsigned int)4		/* Data Path Polarity */
#define	P_DDIR		(unsigned int)6		/* Data Direction */
#define	P_SPCTL		(unsigned int)8		/* Special I/O Control */
#define	P_PPOL		(unsigned int)10	/* Pattern Polarity */
#define	P_PTRAN		(unsigned int)12	/* Pattern Transition */
#define	P_PMASK		(unsigned int)14	/* Pattern Mask */

#define	CIOD_DSR	(unsigned char)0x40
#define	CIOD_RI		(unsigned char)0x80

/* CIO I/O register assignments */

#define RING_INDICATE	(unsigned char)0x7F	/* Input active low */
#define DATA_SET_READY	(unsigned char)0xBF	/* Input active low */
#define ENABLE_422	(unsigned char)0x20	/* Output active high */
#define DISABLE_422	(unsigned char)0xDF	/* Output active high */
#define ENABLE_V35	(unsigned char)0xEF	/* Output active low */
#define DISABLE_V35	(unsigned char)0x10	/* Output active low */

#define	ENABLE_DTE_CLK	(unsigned char)0x08	/* DTE Clocking */
#define	DISABLE_DTE_CLK	(unsigned char)0xF7	/* DCE Clocking */
#define	ENABLE_DCE_CLK	(unsigned char)0xF7	/* DCE Clocking */
#define	DISABLE_DCE_CLK	(unsigned char)0x08	/* DTE Clocking */
#define ENABLE_T232	(unsigned char)0xF7	/* Output active low */
#define DISABLE_T232	(unsigned char)0x08	/* Output active low */

#define ENABLE_232	(unsigned char)0xFB	/* Output active low */
#define DISABLE_232	(unsigned char)0x04	/* Output active low */
#define ENABLE_HRS	(unsigned char)0xFD	/* Output active low */
#define DISABLE_HRS	(unsigned char)0x02	/* Output active low */
#define ENABLE_DTR	(unsigned char)0xFE	/* Output inactive high */
#define DISABLE_DTR	(unsigned char)0x01	/* Output inactive high */
#define ENABLE_BLK	(unsigned char)0x04	/* Output active high */
#define DISABLE_BLK	(unsigned char)0xFB	/* Output active high */

#define	CIO_X21_PAT	(unsigned char)0x03	/* AND mask for pattern */
#define	CIO_X21_00	(unsigned char)0x00	/* PAL detect bits 00 */
#define	CIO_X21_01	(unsigned char)0x01	/* PAL detect bits 01 */
#define	CIO_X21_10	(unsigned char)0x02	/* PAL detect bits 10 */
#define	CIO_X21_11	(unsigned char)0x03	/* PAL detect bits 11 */

#define	CIO_X21_IA	(unsigned char)0x08	/* INDICATE line */

/* The X.21 enable bit and CONTROL line selection are made through the
   ENABLE register. */

#define	CIO_S_X21_CA	(unsigned char)0x01
#define	CIO_C_X21_CA	(unsigned char)0xFE
#define	CIO_S_X21	(unsigned char)0x02
#define	CIO_C_X21	(unsigned char)0xFD
