/* aix_sccsid[] = "src/bos/usr/lib/sendmail/conf.h, cmdsend, bos411, 9428A410j AIX 10/29/91 18:10:06" */
/* 
 * COMPONENT_NAME: CMDSEND conf.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
**	@(#)conf.h	5.7 (Berkeley) 1/5/86
**
*/

/*
**  CONF.H -- All user-configurable parameters for sendmail
*/

#include <sys/limits.h>			/* needed for PATH_MAX */

# define MAXFDESC       200             /* max open file descriptors */

/*
**  Table sizes, etc....
**	There shouldn't be much need to change these....
*/

# define MAXLINE	1024		/* max line length */
# define MAXNAME	256		/* max length of a name */
# define MAXFNAME       PATH_MAX        /* max distinguishable file name */
# define MAXFIELD	4096		/* max total length of a hdr field */
# define MAXPV		40		/* max # of parms to mailers */
# define MAXHOP		17		/* max value of HopCount */
# define MAXATOM	100		/* max atoms per address */
# define MAXMAILERS	25		/* maximum mailers known to system */
# define MAXRWSETS	30		/* max # of sets of rewriting rules */
# define MAXPRIORITIES	25		/* max values for Precedence: field */
# define MAXTRUST	30		/* maximum number of trusted users */
# define MAXUSERENVIRON	250		/* max # of items in user environ */
# define QUEUESIZE	600		/* max # of jobs per queue run */
# define MAXMXHOSTS	10		/* max # of MX records */

/*
**  Compilation options.
**
**	#define these if they are available; comment them out otherwise.
*/

# ifndef DEBUG
# define DEBUG		1	/* enable debugging */
# endif

# ifndef LOG
# define LOG		1	/* enable logging */
# endif
