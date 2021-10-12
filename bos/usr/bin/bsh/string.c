static char sccsid[] = "@(#)18	1.13  src/bos/usr/bin/bsh/string.c, cmdbsh, bos411, 9428A410j 9/1/93 17:36:49";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: movstr any cf length movstrn
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
 * 1.7  com/cmd/sh/sh/string.c, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"


/* ========	general purpose string handling ======== */


uchar_t *
movstr(a, b)
register uchar_t	*a, *b;
{
	while (*b++ = *a++);
	return(--b);
}

any(c, s)
register uchar_t	c;
uchar_t	*s;
{
	register uchar_t d;

	while (d = *s++)
	{
		if (d == c)
			return(TRUE);
	}
	return(FALSE);
}

cf(s1, s2)
register uchar_t *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(0);
	return(*--s1 - *s2);
}

length(as)
uchar_t	*as;
{
	register uchar_t	*s;

	if (s = as)
		while (*s++);
	return(s - as);
}

uchar_t *
movstrn(a, b, n)
	register uchar_t *a, *b;
	register int n;
{
	while ((n-- > 0) && *a)
		*b++ = *a++;

	return(b);
}
