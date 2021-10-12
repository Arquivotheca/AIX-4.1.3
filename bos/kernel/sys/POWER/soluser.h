/* @(#)38	1.4  src/bos/kernel/sys/POWER/soluser.h, sysxsol, bos411, 9428A410j 6/2/92 13:25:48 */
#ifndef _H_SOLUSER
#define _H_SOLUSER
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: soluser.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/comio.h>
#include <sys/devinfo.h>

/*
 *  Miscellaneous constants
 */

#define SOL_MAX_OPENS	(6)		/* Maximum opens allowed	*/
#define SOL_MAX_NETIDS	(32)		/* Maximum CIO_STARTS 		*/
#define SOL_MAX_XMIT	(60 * 1024)	/* Maximum packet size		*/
#define SOL_MIN_XMIT	(1)		/* Minimum packet size		*/
#define SOL_OFF_LEVEL	INTOFFL1	/* off-level level		*/
#define SOL_PACKET_CNT	(1)		/* #packets fastwrt can handle	*/

/*
 *  ioctl command definitions
 */

#define SOL_IOCTL		(DD_SOL<<8)

#define	SOL_GET_PRIDS		(SOL_IOCTL | 0x01)
#define	SOL_CHECK_PRID		(SOL_IOCTL | 0x02)
#define	SOL_BUFFER_ACCESS	(SOL_IOCTL | 0x03)
#define	SOL_SYNC_OTP		(SOL_IOCTL | 0x04)
#define	SOL_ACTIVATE		(SOL_IOCTL | 0x05)
#define	SOL_SCR			(SOL_IOCTL | 0x06)
#define	SOL_OLS			(SOL_IOCTL | 0x07)
#define	SOL_RHR			(SOL_IOCTL | 0x08)
#define	SOL_CRC			(SOL_IOCTL | 0x09)
#define SOL_LOCK_TO_XTAL	(SOL_IOCTL | 0x0a)
#define SOL_CARD_PRESENT	(SOL_IOCTL | 0x0b)
#define SOL_LOOPBACK_TEST	(SOL_IOCTL | 0x0c)

/*
 *  SOL Async Status Blocks
 */
#define SOL_PREFIX		(DD_SOL<<16)

#define SOL_NEW_PRID		(SOL_PREFIX | 0x0001)
#define SOL_PRID_CONFLICT	(SOL_PREFIX | 0x0002)

/*
 *  Hardware Failure Status Blocks (CIO_HARD_FAIL)
 */
#define SOL_FATAL_ERROR				1
#define SOL_RCVRY_THRESH			2

/*
 *  TX_DONE codes (option[3] field)
 */
#define SOL_ACK					0
#define SOL_NACK_NB				3
#define SOL_NACK_NR				4
#define SOL_NACK_NS				5
#define SOL_NEVER_CONN				7
#define SOL_NO_CONN				8
#define SOL_DOWN_CONN				9

/*
 *  DIAGNOSTICS ERROR RETURN CODES
 */

#define SOL_SUCCESS				0
#define SOL_SLA_NOT_STOPPED			1
#define SOL_DMA_UNHANG_FAILED			2
#define SOL_SLA_START_FAILED			3
#define SOL_SLA_IO_EXCEPTION			4
#define SOL_SYNC_OTP_FAILED			5
#define SOL_SCR_NOT_COMPLETE			6
#define SOL_OFFL_SEQ_NOT_REC			7
#define SOL_CRC_CHK_BIT_NOT_SET			8
#define SOL_CANT_STOP_SLA			9
#define SOL_NO_LINK_CHK_BIT			10
#define SOL_SLA_ALREADY_STARTED			11
#define SOL_TIMEOUT				12
#define SOL_LINK_CHK_BIT_SET			13
#define SOL_UNEXPD_FRAME_NOT_SET		14
#define SOL_ADDR_MIS_NOT_SET			15
#define SOL_RHR_COMPARE_FAILED			16

/*
 *  Diagnostic ioctl structures
 */

struct sol_get_prids {		/* For SOL_GET_PRIDS ioctl		*/
	uchar	*bufptr;	/* Pointer to caller's buffer		*/
	uchar	buflen;		/* Length of buffer, in bytes		*/
	uchar	num_ids;	/* Number of IDs detected by DH		*/
	uchar	rsvd0;		/* Reserved field, should be set to 0	*/
	uchar	rsvd1;		/* Reserved field, should be set to 0	*/
	uint	rsvd2;		/* Reserved field, should be set to 0	*/
	uint	rsvd3;		/* Reserved field, should be set to 0	*/
};

struct sol_sla_status {		/* For SLA status on most diag ioctls	*/
	uint	status_1;	/* First status word in SLA		*/
	uint	status_2;	/* Second status word in SLA		*/
	uint	ccr;		/* Channel Control Register in SLA	*/
	uint	cfg_reg;	/* Configuration Register in SLA	*/
	uint	rhr_word0;	/* Receive Header Register - word 0	*/
	uint	rhr_word1;	/* Receive Header Register - word 1	*/
	uint	rhr_word2;	/* Receive Header Register - word 2	*/
	uint	rhr_word3;	/* Receive Header Register - word 3	*/
	uint	rhr_word4;	/* Receive Header Register - word 4	*/
	uint	thr_word0;	/* Transmit Header Register - word 0	*/
	uint	thr_word1;	/* Transmit Header Register - word 1	*/
	uint	thr_word2;	/* Transmit Header Register - word 2	*/
	uint	thr_word3;	/* Transmit Header Register - word 3	*/
	uint	thr_word4;	/* Transmit Header Register - word 4	*/
	uint	rsvd0;		/* Reserved field, should be set to 0	*/
	uint	rsvd1;		/* Reserved field, should be set to 0	*/
};

struct sol_buffer_access {	/* For SOL_BUFFER_ACCESS ioctl		*/
	uchar	*bufptr;	/* Pointer to caller's buffer		*/
	ushort	buflen;		/* Length of buffer, in bytes		*/
	uchar	flag;		/* Indicates a read or write operation	*/
#define	SOL_READ	1
#define	SOL_WRITE	2
	uchar	result;		/* Result of ioctl operation (if EIO)	*/
	struct sol_sla_status	status;	/* Returns SLA status to caller	*/
	uint	rsvd1;		/* Reserved field, should be set to 0	*/
	uint	rsvd2;		/* Reserved field, should be set to 0	*/
};

struct sol_diag_test {		/* For SOL_ACTIVATE, SOL_SCR, SOL_OLS,
				   SOL_RHR, and SOL_CRC ioctls		*/
	uchar	diag_mode;	/* Indicates what mode for the test	*/
#define	SOL_NORMAL	0	/* Normal mode - no wrap		*/
#define SOL_10_BIT_WRAP	1	/* Wrap within SLA			*/
#define SOL_1_BIT_WRAP	2	/* Wrap within Optic Two-Port Card	*/
#define	SOL_NBR_WRAP	4	/* Wrap between SLAs			*/
	uchar	result;		/* Result of ioctl operation (if EIO)	*/
	uchar	rsvd1;		/* Reserved field, should be set to 0	*/
	uchar	rsvd2;		/* Reserved field, should be set to 0	*/
	struct sol_sla_status	status;	/* Returns SLA status to caller	*/
	uint	rsvd3;		/* Reserved field, should be set to 0	*/
	uint	rsvd4;		/* Reserved field, should be set to 0	*/
};

/*
 *  SOL specific statistics structure
 */

typedef struct {
	ulong	collapsed_frame_mcnt;	/* (msb) count of frames collapsed   */
	ulong	collapsed_frame_lcnt;	/* (lsb) count of frames collapsed   */
	ulong	sta_que_overflow;	/* status lost due to full stat que  */
	ulong	rec_que_overflow;	/* rcv pkt lost due to full recv que */
} sol_stats_t;

/*
 *  Structure definition for CIO_QUERY ioctl
 */

typedef struct {
	cio_stats_t	cc;
	sol_stats_t	ds;
} sol_query_stats_t;

#endif /* _H_SOLUSER */
