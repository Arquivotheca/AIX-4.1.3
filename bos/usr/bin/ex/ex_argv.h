/* @(#)10	1.7  src/bos/usr/bin/ex/ex_argv.h, cmdedit, bos411, 9428A410j 11/23/93 13:35:02 */
/*
 * COMPONENT_NAME: (CMDEDIT) ex_argv.c
 *
 * FUNCTION: none
 *
 * ORIGINS: 3, 10, 13, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ex_argv.h	1.3  com/cmd/edit/vi,3.1,9013 10/6/89 17:15:38 
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */
/*  ex_argv.h,v  Revision: 2.4  (OSF) Date: 90/10/07 16:28:57  */

/*
 * The current implementation of the argument list is poor,
 * using an argv even for internally done "next" commands.
 * It is not hard to see that this is restrictive and a waste of
 * space.  The statically allocated glob structure could be replaced
 * by a dynamically allocated argument area space.
 */
var char	**argv;
var char	**argv0;
var char	*args;
var char	*args0;
var short	argc;
var short	argc0;
var short	morargc;		/* Used with "More files to edit..." */
var wchar_t	*firstpat = (wchar_t *)NULL;/* From +/pat	*/
struct	glob {
	short	argc;			/* Index of current file in argv */
	short	argc0;			/* Number of arguments in argv */
	char	*argv[NARGS + 1];	/* WHAT A WASTE! */
	char	argspac[NCARGS + sizeof (int)];
};
var struct	glob frob;
