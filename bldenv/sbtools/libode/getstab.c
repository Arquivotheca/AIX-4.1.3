/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: getstab
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
 * $Log: getstab.c,v $
 * Revision 1.8.5.4  1993/04/29  15:45:15  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:23  damon]
 *
 * Revision 1.8.5.3  1993/04/27  20:55:48  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:55:28  damon]
 * 
 * Revision 1.8.5.2  1993/04/08  16:30:06  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  16:28:10  damon]
 * 
 * Revision 1.8.2.4  1992/12/03  17:20:56  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:14  damon]
 * 
 * Revision 1.8.2.3  1992/12/02  20:26:13  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:18  damon]
 * 
 * Revision 1.8.2.2  1992/06/15  18:10:23  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:27  damon]
 * 
 * Revision 1.8.5.2  1992/06/15  16:30:40  damon
 * 	Taken from 2.1.1
 * 
 * Revision 1.8.2.2  1992/03/24  19:27:58  damon
 * 	Changed strings.h to string.h
 * 	[1992/03/24  19:27:25  damon]
 * 
 * Revision 1.8  1991/12/05  21:05:02  devrcs
 * 	Merged some includes into include of ode/odedefs.h
 * 	[91/09/04  13:10:27  damon]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:11  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:11:58  randyb]
 * 
 * Revision 1.6  90/10/07  20:03:28  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:09:16  gm]
 * 
 * Revision 1.5  90/08/09  14:23:12  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:47:04  gm]
 * 
 * Revision 1.4  90/07/17  12:36:43  devrcs
 * 	More changes for gcc.
 * 	[90/07/08  21:28:01  gm]
 * 
 * Revision 1.3  90/06/29  14:38:43  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:21:11  gm]
 * 
 * Revision 1.2  90/01/02  19:26:59  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:14:38  gm
 * 	Current version from CMU.
 * 	[89/12/23            gm]
 * 
 * 	Modified for 4.2 BSD.  Now uses stderr for output.
 * 	[85/04/28            sas]
 * 
 * 	Fixed by adding missing "return" statement!  Somehow, it
 * 	worked on the VAX anyway ...
 * 	[83/03/08            sas]
 * 
 * 	Added code to use default value on error or EOF in standard input.
 * 	[82/10/23            sas]
 * 
 * 	Rewritten for VAX.  Now lets stablk() do the error-message printing.
 * 	[79/11/20            sas]
 * 
 * $EndLog$
 */
/*  getstab  --  ask user for string in table
 *
 *  Usage:  i = getstab (prompt,table,defalt)
 *	int i;
 *	char *prompt, **table, *defalt;
 *
 *  Getstab prints the messge:	prompt  [defalt]
 *  and asks the user to type in a line.  This input text
 *  is compared to all the strings in the table to see which
 *  (if any) it matches; the stablk() routine is used
 *  for the matching.  If the string is ambiguous or invalid
 *  (i.e. matches zero strings, or more than one), the cycle
 *  is repeated.  When a valid string is typed, the index
 *  of the string it matches is returned.  If the user just
 *  types carriage return, the default string is used for matching.
 *  The default is also used on error or EOF in standard input.
 *  The string table may be declared in this way:
 * 	char *table[] = {"string1","string2",...,0};
 *
 *  Originally by klg (Ken Greer) on IUS/SUS UNIX.
 */

#ifndef lint
static char sccsid[] = "@(#)84  1.1  src/bldenv/sbtools/libode/getstab.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:09";
#endif /* not lint */

#include <stdio.h>
#include <string.h>
#include <ode/odedefs.h>
#include <ode/interface.h>
#include <ode/util.h>

int
getstab ( const char *prompt, const char *table[], const char *defalt )
{
	char input[200];
	register int ix;

	fflush (stdout);
	do {
		ui_print ( VALWAYS, "%s  [%s]  ", prompt, defalt);
		fflush (stdout);
		if (gets (input) == NULL)
		  strcpy (input,defalt);
		if (*input == '\0')
		  strcpy (input,defalt);
		ix = stablk (input,table,0);
	} 
	while (ix < 0);

	return (ix);
}
