/* @(#)42	1.2  src/bos/kernel/sys/strstat.h, sysxpse, bos411, 9428A410j 5/22/91 09:59:19 */
/*
 *   COMPONENT_NAME: SYSXPSE strstat.h
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

#ifndef _STRSTAT_
#define _STRSTAT_

/* module statistics structure */
struct	module_stat {
	long	ms_pcnt;	/* count of calls to put proc */
	long	ms_scnt;	/* count of calls to service proc */
	long	ms_ocnt;	/* count of calls to open proc */
	long	ms_ccnt;	/* count of calls to close proc */
	long	ms_acnt;	/* count of calls to admin proc */
	char	* ms_xptr;	/* pointer to private statistics */
	short	ms_xsize;	/* length of private statistics buffer */
};

#endif
