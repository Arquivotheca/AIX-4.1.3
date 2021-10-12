/* @(#)40	1.3  src/bos/kernel/sys/strlog.h, sysxpse, bos411, 9428A410j 5/27/91 12:56:14 */
/*
 *   COMPONENT_NAME: SYSXPSE strlog.h
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1990  Mentat Inc.
 *
 */

#ifndef _STRLOG_
#define	_STRLOG_

struct log_ctl {
	short	mid;
	short	sid;
	char	level;
	short	flags;
	long	ltime;
	long	ttime;
	int	seq_no;
};

#define	SL_FATAL	0x1	/* Fatal error */
#define	SL_NOTIFY	0x2	/* Notify the system administrator */
#define	SL_ERROR	0x4	/* Pass message to error logger */
#define	SL_TRACE	0x8	/* Pass message to tracer */
#define	SL_CONSOLE	0x10	/* Print the message on the console */
#define	SL_WARN		0x20	/* Warning */
#define	SL_NOTE		0x40	/* Notice this message */

struct trace_ids {
	short	ti_mid;
	short	ti_sid;
	char	ti_level;
};

#define	I_TRCLOG	1
#define	I_ERRLOG	2

#define	LOGMSGSZ	128
#define	WORDLEN		sizeof(int)
#define	NLOGARGS	4

#endif
