static char sccsid[] = "@(#)97  1.2  src/bldenv/pkgtools/warn.c, pkgtools, bos412, GOLDA411a 1/29/93 17:26:34";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: defined
 *		fatal
 *		warn
 *		
 *
 *   ORIGINS: 18,27,71
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
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
/* $Source: /u/mark/src/pax/RCS/warn.c,v $
 *
 * $Revision: 1.2 $
 *
 * warn.c - miscellaneous user warning routines 
 *
 * DESCRIPTION
 *
 *	These routines provide the user with various forms of warning
 *	and informational messages.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
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
 * $Log:	warn.c,v $
 * Revision 1.2  89/02/12  10:06:15  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:40  mark
 * Initial revision
 * 
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: warn.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 91/10/01 15:55:56 $";
#endif
/* 
 * warn.c - miscellaneous user warning routines 
 *
 * DESCRIPTION
 *
 *	These routines provide the user with various forms of warning
 *	and informational messages.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
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
 * Revision 1.2  89/02/12  10:06:15  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:40  mark
 * Initial revision
 * 
 */

/* Headers */

#include <stdio.h>

/* Function Prototypes */

extern char *myname;
extern int exit_status;

/* fatal - print fatal message and exit
 *
 * DESCRIPTION
 *
 *	Fatal prints the program's name along with an error message, then
 *	exits the program with a non-zero return code.
 *
 * PARAMETERS
 *
 *	char 	*why	- description of reason for termination 
 *		
 * RETURNS
 *
 *	Returns an exit code of 1 to the parent process.
 */


void fatal(char *why)

{
    fprintf(stderr, "%s: %s\n", myname, why);
    exit(1);
}



/* warn - print a warning message
 *
 * DESCRIPTION
 *
 *	Print an error message listing the program name, the actual error
 *	which occurred and an informational message as to why the error
 *	occurred on the standard error device.  The standard error is
 *	flushed after the error is printed to assure that the user gets
 *	the message in a timely fasion.
 *
 *	The exit status the program return is set to 1.
 *
 * PARAMETERS
 *
 *	char *what	- Pointer to string describing what failed.
 *	char *why	- Pointer to string describing why did it failed.
 */


void warn(char *what, char *why)

{
    fprintf(stderr, "%s: %s : %s\n", myname, what, why);
    fflush(stderr);
    exit_status = 1;
}
