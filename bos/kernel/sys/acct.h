/* @(#)39	1.10  src/bos/kernel/sys/acct.h, sysproc, bos411, 9428A410j 3/30/94 15:43:05 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27,3
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_ACCT
#define _H_ACCT

#include "sys/types.h"
#include "sys/param.h"

/*
 * Accounting structures
 * these use a comp_t type which is a 3 bit base 8 
 * exponent, 13 bit fraction "floating point" number.
 * Units are 1/AHZ seconds.
 */

typedef	ushort comp_t;		/* "floating point" */
		/* 13-bit fraction, 3-bit exponent  */

struct	acct
{
	char	ac_flag;		/* Accounting flag */
	char	ac_stat;		/* Exit status */
	uid_t	ac_uid;			/* Accounting user ID */
	gid_t	ac_gid;			/* Accounting group ID */
	dev_t	ac_tty;			/* control typewriter */
	time_t	ac_btime;		/* Beginning time */
	comp_t	ac_utime;		/* acctng user time in seconds */
	comp_t	ac_stime;		/* acctng system time in seconds */
	comp_t	ac_etime;		/* acctng elapsed time in seconds */
	comp_t	ac_mem;			/* memory usage */
	comp_t	ac_io;			/* chars transferred */
	comp_t	ac_rw;			/* blocks read or written */
	char	ac_comm[8];		/* command name */
};	

#define	AFORK	0001		/* has executed fork, but no exec */
#define	ASU	0002		/* used super-user privileges */
#define	ACOMPAT	0004		/* used compatibilty mode */
#define	ACORE	0010		/* dumped core */
#define	AXSIG	0020		/* killed by signal */
#define	ACCTF	0300		/* record type: 00 = acct */

/*
 * 1/AHZ is the granularity of the data encoded in the various
 * comp_t fields. This is not necessarily equal to hz.
 */

#define AHZ 64
#endif	/* _H_ACCT */
