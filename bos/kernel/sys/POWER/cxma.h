/*
 * @(#)04       1.4.1.1  src/bos/kernel/sys/POWER/cxma.h, sysxtty, bos411, 9428A410j 2/24/94 15:13:14
 *
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FILE_NAME: cxma.h - System include file for CXMA driver.
 *
 * FUNCTIONS:
 *
 * ORIGINS: 80
 *
 * This software contains proprietary and confidential information of Digi
 * International Inc.  By accepting transfer of this copy, Recipient agrees
 * to retain this software in confidence, to prevent disclosure to others, and
 * to make no use of this software other than that for which it was delivered.
 * This is an unpublished copyright work Digi International Inc.  Execpt as
 * permitted by federal law, 17 USC 117, copying is strictly prohibited.
 *
 * Digi International Inc. CONFIDENTIAL - (Digi International Inc. Confidential
 * when combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT Digi International Inc. 1988-1992
 * All Rights Reserved
 *
 * US Government Users Restircted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with Digi International Inc.
 */

/*
 * This module provides application access to special CXMA driver
 * serial line enhancements which are not standard tty features.
 */

/************************************************************************
 * Ioctl command arguments for CXMA parameters.
 ************************************************************************/
#define CXMA_GETA	(('d'<<8) | 0x0)	/* Read params		*/

#define CXMA_SETA	(('d'<<8) | 0x1)	/* Set params		*/
#define CXMA_SETAW	(('d'<<8) | 0x2)	/* Drain & set params	*/
#define CXMA_SETAF	(('d'<<8) | 0x3)	/* Drain, flush & set params */

#define CXMA_KME	(('d'<<8) | 0x4)	/* Read/Write Host	*/
						/* Adapter Memory	*/

#define	CXMA_GETFLOW	(('d'<<8) | 0x5)	/* Get startc/stopc flow */
						/* control characters 	 */
#define	CXMA_SETFLOW	(('d'<<8) | 0x6)	/* Set startc/stopc flow */
						/* control characters	 */
#define	CXMA_GETAFLOW	(('d'<<8) | 0x7)	/* Get Aux. startc/stopc */
						/* flow control chars 	 */
#define	CXMA_SETAFLOW	(('d'<<8) | 0x8)	/* Set Aux. startc/stopc */
						/* flow control chars	 */
typedef struct cxmaflow_struct {
	uchar	startc;				/* flow cntl start char	*/
	uchar	stopc;				/* flow cntl stop char	*/
} cxmaflow_t;

#define	CXMA_F2200_GETA	 (('d'<<8) | 0x9)	/* Get 2x36 flow cntl flags */
#define	CXMA_F2200_SETAW (('d'<<8) | 0x10)	/* Set 2x36 flow cntl flags */
#define		F2200_MASK	0x03	/* 2200 flow cntl bit mask	*/
#define		FCNTL_2200	0x01	/* 2x36 terminal flow cntl	*/
#define		PCNTL_2200	0x02	/* 2x36 printer flow cntl	*/
#define	F2200_XON	0xf8
#define	P2200_XON	0xf9
#define	F2200_XOFF	0xfa
#define	P2200_XOFF	0xfb

#define	FXOFF_MASK	0x03		/* 2200 flow status mask	*/
#define	RCVD_FXOFF	0x01		/* 2x36 Terminal XOFF rcvd	*/
#define	RCVD_PXOFF	0x02		/* 2x36 Printer XOFF rcvd	*/

#define	CXMA_RESET	(('d'<<8) | 0x11)	/* Reset Fepos		*/

/************************************************************************
 * Values for cxma_flags 
 ************************************************************************/
#define CXMA_IXON	0x0001		/* Handle IXON in the FEP	*/
#define CXMA_FAST	0x0002		/* Fast baud rates		*/
#define RTSPACE		0x0004		/* RTS input flow control	*/
#define CTSPACE		0x0008		/* CTS output flow control	*/
#define DSRPACE		0x0010		/* DSR output flow control	*/
#define DCDPACE		0x0020		/* DCD output flow control	*/
#define DTRPACE		0x0040		/* DTR input flow control	*/
#define CXMA_COOK	0x0080		/* Cooked processing done in FEP */
#define CXMA_FORCEDCD	0x0100		/* Force carrier		*/
#define	CXMA_ALTPIN	0x0200		/* Alternate RJ-45 pin config	*/
#define	CXMA_AIXON	0x0400		/* Aux flow control in fep	*/

#define CXMA_PLEN	8		/* String length		*/
#define	CXMA_TSIZ	10		/* Terminal string len		*/

/************************************************************************
 * Structure used with ioctl commands for CXMA parameters.
 ************************************************************************/
struct cxma_struct {
	unsigned short	cxma_flags;		/* Flags (see above)	*/
	unsigned short	cxma_maxcps;		/* Max printer CPS	*/
	unsigned short	cxma_maxchar;		/* Max chars in print queue */
	unsigned short	cxma_bufsize;		/* Buffer size		*/
	unsigned short	cxma_edelay;		/* EDELAY Wakeup rate	*/
	unsigned char	cxma_onlen;		/* Length of ON string	*/
	unsigned char	cxma_offlen;		/* Length of OFF string	*/
	char		cxma_onstr[CXMA_PLEN];	/* Printer on string	*/
	char		cxma_offstr[CXMA_PLEN];	/* Printer off string	*/
	char		cxma_term[CXMA_TSIZ];	/* terminal string	*/
};

typedef struct cxma_struct cxma_t;

/************************************************************************
 * Digiboard KME definitions and structures.
 ************************************************************************/
#define	RW_IDLE		0	/* Operation complete			*/
#define	RW_READ		1	/* Read Concentrator Memory		*/
#define	RW_WRITE	2	/* Write Concentrator Memory		*/
#define RW_PAUSE        3	/* pause for config			*/

#define KME_DATASIZE	128

struct kme_struct {
	unsigned char	rw_req;		/* Request type			*/
	unsigned char	rw_board;	/* Host Adapter board number	*/
	unsigned char	rw_conc;	/* Concentrator number		*/
	unsigned char	rw_reserved;	/* Reserved for expansion	*/
	unsigned long	rw_addr;	/* Address in concentrator	*/
	unsigned short	rw_size;	/* Read/write request length	*/
	unsigned char	rw_data[KME_DATASIZE]; /* Data to read/write	*/
};

typedef struct kme_struct rw_t;


