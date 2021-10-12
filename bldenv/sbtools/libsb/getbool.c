static char sccsid[] = "@(#)39  1.1  src/bldenv/sbtools/libsb/getbool.c, bldprocess, bos412, GOLDA411a 4/29/93 12:21:06";
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
/*  getbool -- ask user a yes/no question
 *
 *  Usage:  i = getbool (prompt, defalt);
 *
 *  Example:  do {...} while (getbool ("More?",1));
 *
 *  Prints prompt string, asks user for response.  Defalt is
 *  0 (no) or 1 (yes), and is used if user types just carriage return,
 *  or on end-of-file or error in the standard input.
 */

#include <stdio.h>
#include <ode/interface.h>

int getbool (prompt, defalt)
char *prompt;
int defalt;
{
	register int valu;
	register char ch;
	char input [100];

	fflush (stdout);
	if (defalt != 1 && defalt != 0)  defalt = 1;
	valu = 2;				/* meaningless value */
	do {
		ui_print ( VALWAYS, "%s  [%s]  ",
			   prompt, (defalt ? "yes" : "no"));
		fflush (stdout);		/* in case it's buffered */

		if (gets (input) == NULL) {
			valu = defalt;
		}
		else {
			ch = *input;			/* first char */
			if (ch == 'y' || ch == 'Y')
			  valu = 1;
			else if (ch == 'n' || ch == 'N')
			  valu = 0;
			else if (ch == '\0')
			  valu = defalt;
			else 
			  ui_print ( VALWAYS,
			    "Must begin with 'y' (yes) or 'n' (no).\n");
		}
	} 
	while (valu == 2);			/* until correct response */
	return (valu);
}
