/* @(#)51	1.1  src/bos/usr/include/pse/echo.h, sysxpse, bos411, 9428A410j 5/7/91 14:30:37 */
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _ECHO_
#define _ECHO_

/** Copyright (c) 1990  Mentat Inc.
 ** echo.c 2.4, last change 4/9/91
 **/

#define	ECHO_IOCACK		1	/** Do an iocack reply with no data */
#define	ECHO_IOCNAK		2	/** Do an iocnak reply */
#define	ECHO_NOREPLY		3	/** Let the ioctl timeout, no reply */
#define	ECHO_DATA		4	/** Do an iocack with data */
#define	ECHO_BIGDATA		5	/** Do an iocack with > ic_len data */
#define	ECHO_GENMSG		6	/** Gen a message from the eblks in data */
#define	ECHO_GENMSG_NOREPLY	7	/** Gen a message from the eblks, but don't reply to the M_IOCTL */
#define	ECHO_FEED_ME		8	/** Send infinite messages upstream. */
#define	ECHO_RVAL		0x8000	/** Add return value */
#define	ECHO_RERROR		0x4000	/** Error return */

typedef struct iecho_s {
	int	ie_error;
	int	ie_rval;
	char	* ie_buf;	/* buffer address for transparent ioctls */
	int	ie_len;		/* buffer length for transparent ioctls */
} iecho_t;

typedef	struct echo_blk {
	int	eb_type;	/* type of genned mblk */
	int	eb_len;		/* len of data to put in mblk; data follows immediately (if any) */
	int	eb_flag;	/* flag word to be copied into b_flag */
} eblk_t;

#endif
