static char sccsid[] = "@(#)71	1.2  src/bos/usr/bin/pax/mem.c, cmdarch, bos411, 9428A410j 6/26/91 13:09:52";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/* $Source: /u/mark/src/pax/RCS/mem.c,v $
 *
 * $Revision: 1.2 $
 *
 * mem.c - memory allocation and manipulation functions
 *
 * DESCRIPTION
 *
 *	These routines are provided for higher level handling of the UNIX
 *	memory allocation functions.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log:	mem.c,v $
 * Revision 1.2  89/02/12  10:04:53  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:17  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: mem.c,v 1.2 89/02/12 10:04:53 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* mem_get - allocate memory
 *
 * DESCRIPTION
 *
 *	Mem_get attempts to allocate a block of memory using the malloc
 *	function call.  In the event that the memory is not available, 
 *	mem_get will display an "Out of memory" message for the user
 *	the first time it encounters the an out of memory situation.
 *	Subsequent calls to mem_get may fail, but no message will be
 *	printed.
 *
 * PARAMETERS
 *
 *	uint len	- The amount of memory to allocate
 *
 * RETURNS
 *
 *	Normally returns the pointer to the newly allocated memory.  If
 *	an error occurs, NULL is returned, and an error message is
 *	printed.
 *
 * ERRORS
 *
 *	ENOMEM	No memory is available 
 */


char *mem_get(uint len)

{
    char           *mem;
    static short    outofmem = 0;

    if ((mem = (char *)malloc(len)) == (char *)NULL && !outofmem) {
	outofmem++;
	warn("mem_get()", MSGSTR(NOMEM, "Out of memory"));
    }
    return (mem);
}


/* mem_str - duplicate a string into dynamic memory
 *
 * DESCRIPTION
 *
 *	Mem_str attempts to make a copy of string.  It allocates space for
 *	the string, and if the allocation was successfull, copies the old
 *	string into the newly allocated space.
 *
 * PARAMETERS
 *
 *	char *str 	- string to make a copy of 
 *
 * RETURNS
 *
 *	Normally returns a pointer to a new string at least as large
 *	as strlen(str) + 1, which contains a copy of the the data 
 *	passed in str, plus a null terminator.  Returns (char *)NULL 
 *	if enough memory to make a copy of str is not available.
 */


char *mem_str(char *str)

{
    char           *mem;

    if (mem = mem_get((uint) strlen(str) + 1)) {
	strcpy(mem, str);
    }
    return (mem);
}
