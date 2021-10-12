/* @(#)35	1.6  src/bos/usr/bin/csh/proc.h, cmdcsh, bos411, 9428A410j 11/12/92 13:37:14 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS:
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */
/* flag values for p_flags */
#define	PRUNNING	0x0001		/* running */
#define	PSTOPPED	0x0002		/* stopped */
#define	PNEXITED	0x0004		/* normally exited */
#define	PAEXITED	0x0008		/* abnormally exited */
#define	PSIGNALED	0x0010		/* terminated by a signal != SIGINT */
#define	PNOTIFY		0x0020		/* notify async when done */
#define	PTIME		0x0040		/* job times should be printed */
#define	PAWAITED	0x0080		/* top level is waiting for it */
#define	PFOREGND	0x0100		/* started in shells pgrp */
#define	PDUMPED		0x0200		/* process dumped core */
#define	PDIAG		0x0400		/* diagnostic output also piped out */
#define	PPOU		0x0800		/* piped output */
#define	PREPORTED	0x1000		/* status has been reported */
#define	PINTERRUPTED	0x2000		/* job stopped via interrupt signal */
#define	PPTIME		0x4000		/* time individual process */
#define	PNEEDNOTE	0x8000		/* notify as soon as practical */

#define	PNULL		(struct process *)0
#define	PMAXLEN		80

#define	PALLSTATES	\
		(PRUNNING|PSTOPPED|PNEXITED|PAEXITED|PSIGNALED|PINTERRUPTED)

/* defines for arguments to pprint */
#define	NUMBER		01
#define	NAME		02
#define	REASON		04
#define	AMPERSAND	010
#define	FANCY		020
#define	SHELLDIR	040		/* print shell's dir if not the same */
#define	JOBDIR		0100		/* print job's dir if not the same */
#define	AREASON		0200

struct	process	proclist;		/* list head of all processes */
bool	pnoprocesses;			/* pchild found nothing to wait for */

struct	process *pholdjob;		/* one level stack of current jobs */

struct	process *pcurrjob;		/* current job */
struct	process	*pcurrent;		/* current job in table */
struct	process *pprevious;		/* previous job in table */

short	pmaxindex;			/* current maximum job index */

bool	timesdone;			/* shtimes buffer full ? */
