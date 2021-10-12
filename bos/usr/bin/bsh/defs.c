static char sccsid[] = "@(#)91	1.15  src/bos/usr/bin/bsh/defs.c, cmdbsh, bos411, 9428A410j 9/1/93 17:30:51";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 *
 * OSF/1 1.1
 */

#include	<sys/limits.h>
#include	<sys/types.h>
#include 	<setjmp.h>
#include	"mode.h"
#include	"name.h"

int	_open_max;

/* temp files and io */

int		output = 2;
int		ioset;
struct ionod	*iotemp;	/* files to be deleted sometime */
struct ionod	*fiotemp;	/* function files to be deleted sometime */
struct ionod	*iopend;	/* documents waiting to be read at NL */
struct fdsave	*fdmap;

/* substitution */
int		dolc;
uchar_t		**dolv;
struct dolnod	*argfor;
struct argnod	*gchain;


/* name tree and words */
int		wdval;
int		wdnum;
int		fndef;
int		nohash;
struct argnod	*wdarg;
int		wdset;
BOOL		reserv;

/* special names */
uchar_t		*pcsadr;
uchar_t		*pidadr;
uchar_t		*cmdadr;

/* transput */ 
uchar_t 	*tempname;
unsigned int 	serial; 
unsigned int 	peekc;
unsigned int 	peekn;
uchar_t 	*comdiv;

long		flags;
int		rwait;	/* flags read waiting */

/* error exits from various parts of shell */
jmp_buf		subshell;
jmp_buf		errshell;

/* fault handling */
BOOL		trapnote;
BOOL		mailalarm;

/* execflgs */
int		exitval;
int		retval;
BOOL		execbrk;
int		loopcnt;
int		breakcnt;
int 		funcnt;

int		wasintr;	/* used to tell if break or delete is hit
	   			   while executing a wait
				*/

int		eflag;

/* The following stuff is from stak.h	*/

uchar_t		*stakbas;
uchar_t		*staktop;
uchar_t		*stakbot;
uchar_t		*stakbsy;
uchar_t		*brkend;
