static char sccsid[] = "@(#)41  1.1  src/bldenv/sbtools/libsb/getstr.c, bldprocess, bos412, GOLDA411a 4/29/93 12:21:24";
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 *  Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 *  School of Computer Science 
 *  Carnegie Mellon University 
 *  Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * ODE 2.1.1
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getstr.c,v $ $Revision: 1.7 $ (OSF) $Date: 91/12/05 21:05:06 $";
#endif
/*  getstr --  prompt user for a string
 *
 *  Usage:  p = getstr (prompt,defalt,answer);
 *	char *p,*prompt,*defalt,*answer;
 *
 *  Getstr prints this message:  prompt  [defalt]
 *  and accepts a line of input from the user.  This line is
 *  entered into "answer", which must be a big char array;
 *  if the user types just carriage return, then the string
 *  "defalt" is copied into answer.
 *  Value returned by getstr is just the same as answer,
 *  i.e. pointer to result string.
 *  The default value is used on error or EOF in the standard input.
 */

#include <stdio.h>

char *getstr (prompt,defalt,answer)
char *prompt,*defalt,*answer;
{
	char defbuf[4000];
	register char *retval;

	fflush (stdout);
	fprintf (stderr,"%s  [%s]  ",prompt,defalt);
	fflush (stderr);
	strcpy (defbuf,defalt);
	retval = (char *) gets (answer);
	if (retval == NULL || *answer == '\0')  strcpy (answer,defbuf);
	if (retval == NULL)
	    return (retval);
	else
	    return (answer);
}
