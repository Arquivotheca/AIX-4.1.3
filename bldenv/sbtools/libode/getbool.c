/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: getbool
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: getbool.c,v $
 * Revision 1.8.5.2  1993/04/27  20:55:42  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:55:25  damon]
 *
 * Revision 1.8.5.1  1993/04/27  20:55:40  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.8.2.3  1992/12/03  17:20:53  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:13  damon]
 * 
 * Revision 1.8.2.2  1992/12/02  20:26:11  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:17  damon]
 * 
 * Revision 1.8  1991/12/05  21:04:59  devrcs
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:06  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:11:23  randyb]
 * 
 * Revision 1.6  90/10/07  20:03:24  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:09  gm]
 * 
 * Revision 1.5  90/08/09  14:23:08  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:58  gm]
 * 
 * Revision 1.4  90/07/17  12:36:40  devrcs
 * 	More changes for gcc.
 * 	[90/07/08  21:27:56  gm]
 * 
 * Revision 1.3  90/06/29  14:38:40  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:07  gm]
 * 
 * Revision 1.2  90/01/02  19:26:48  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:56  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Modified for 4.2 BSD.  Now uses stderr for output.
 * 	[85/04/28            sas]
 * 
 * 	Added code to return default if gets returns NULL.
 * 	[82/10/23            sas]
 * 
 * 	Rewritten for VAX.  Possible changes for the future:  accept "t" (true)
 * 	and "f" (false), 0 and 1, etc.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
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

#ifndef lint
static char sccsid[] = "@(#)83  1.1  src/bldenv/sbtools/libode/getbool.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:06";
#endif /* not lint */

#include <stdio.h>
#include <ode/interface.h>
#include <ode/util.h>

int
getbool ( const char *prompt, int defalt )
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
