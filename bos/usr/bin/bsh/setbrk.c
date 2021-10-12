static char sccsid[] = "@(#)15	1.14  src/bos/usr/bin/bsh/setbrk.c, cmdbsh, bos411, 9428A410j 9/1/93 17:36:08";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: setbrk
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.8  com/cmd/sh/sh/setbrk.c, cmdsh, bos324 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"

char 	*sbrk();

uchar_t *
setbrk(incr)
{

	register uchar_t *a = (uchar_t *)sbrk(incr);

	if((int) a == -1) 
	{ 
# if DEBUG
		error("AARGH! setbrk failed");
		abort();
# endif
		error (MSGSTR(M_NOSPACE,(char *)nospace));
	}
	else
		brkend = a + incr;
	return(a);
}
