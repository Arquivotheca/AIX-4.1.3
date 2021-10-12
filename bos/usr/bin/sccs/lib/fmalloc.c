static char sccsid[] = "@(#)89 1.8 src/bos/usr/bin/sccs/lib/fmalloc.c, cmdsccs, bos411, 9428A410j 2/6/94 10:36:01";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: fmalloc, ffree, ffreeall
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

# include       "defines.h"

char    *ptrlist;

/* allocate asize bytes;
 * chain to ptrlist for later ffreeall
 */
char *
fmalloc(asize)
unsigned asize;
{
	char *ptr, *malloc();

	asize += sizeof(char *);

	if (!(ptr = malloc(asize)))
		fatal(MSGCO(OTOFSPC, "\nThere is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (ut9)\n"));  /* MSG */

	*(char **)ptr = ptrlist;
	ptrlist = ptr;
	return ptr + sizeof(char *);
}

/* free space allocated by fmalloc; unlink from ptrlist */
ffree(aptr)
char *aptr;
{
	register char **pp, *ptr;

	aptr -= sizeof(char *);
	for (pp = &ptrlist;; pp = (char **)ptr) {
		if (!(ptr = *pp))
			fatal(MSGCO(FFREE, "\nAn attempt to free memory failed.\n\
\tUse local problem reporting procedures.\n"));  /* MSG */
		if (aptr == ptr)
			break;
	}
	*pp = *(char **)ptr;
	free(aptr);
}

/* free all memory on ptrlist */
ffreeall()
{
	register char *ptr;

	while (ptr = ptrlist) {
		ptrlist = *(char **)ptr;
		free(ptr);
	}
}
