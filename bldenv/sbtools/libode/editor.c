/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: editor
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
 * $Log: editor.c,v $
 * Revision 1.8.5.2  1993/04/27  22:28:39  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  22:28:29  damon]
 *
 * Revision 1.8.5.1  1993/04/27  22:28:38  damon
 * *** Initial Branch Revision ***
 *
 * Revision 1.8.2.4  1992/12/11  16:52:11  damon
 * 	CR 359. cleanup
 * 	[1992/12/11  16:51:54  damon]
 * 
 * Revision 1.8.2.3  1992/12/03  17:20:40  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:01  damon]
 * 
 * Revision 1.8.2.2  1992/12/02  20:25:51  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/02  20:23:09  damon]
 * 
 * Revision 1.8  1991/12/05  21:04:36  devrcs
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:46:01  randyb]
 * 
 * 	Correct copyright; clean up lint input; added ui_print function.
 * 	[91/01/08  12:10:45  randyb]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:23:10  dwm]
 * 
 * Revision 1.6  90/10/07  20:02:53  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:08:24  gm]
 * 
 * Revision 1.5  90/08/09  14:22:49  devrcs
 * 	Moved here from usr/local/sdm/lib/libsb.
 * 	[90/08/05  12:46:19  gm]
 * 
 * Revision 1.4  90/07/17  12:36:37  devrcs
 * 	More changes for gcc.
 * 	[90/07/08  21:27:50  gm]
 * 
 * Revision 1.3  90/06/29  14:38:13  devrcs
 * 	Moved here from defunct libcs library.
 * 	[90/06/23  14:20:35  gm]
 * 
 * Revision 1.2  90/01/02  19:26:38  gm
 * 	Fixes for first snapshot.
 * 
 * Revision 1.1  89/12/26  10:13:22  gm
 * 	Current version from CMU.  Changed default editor to vi.
 * 	[89/12/23            gm]
 * 
 * 	Rewritten for 4.2 BSD UNIX.
 * 	[85/11/22            gm0w]
 * 
 * $EndLog$
 */
/*
 *  editor  --  fork editor to edit some text file
 *
 *  Usage:
 *	i = editor(file, prompt);
 *	char *file, *prompt;
 *	int i;
 *
 *  The editor() routine is used to fork the user's favorite editor.
 *  There is assumed to be an environment variable named "EDITOR" whose
 *  value is the name of the favored editor.  If the EDITOR parameter is
 *  missing, some default (see DEFAULTED below) is assumed.  The runp()
 *  routine is then used to find this editor on the searchlist specified
 *  by the PATH variable (or the default path).  "file" is the name of
 *  the file to be edited and "prompt" is a string (of any length) which
 *  will be printed in a such a way that the user can see it at least at
 *  the start of the editing session.  editor() returns the value of the
 *  runp() call.
 */

#ifndef lint
static char sccsid[] = "@(#)77  1.1  src/bldenv/sbtools/libode/editor.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:53";
#endif /* not lint */

#include <stdio.h>
#include <stdlib.h>
#include <ode/interface.h>
#include <ode/run.h>
#include <ode/util.h>

#define DEFAULTED "vi"

int
editor( const char *file, const char *prompt )
{
	const char *editor_name;

	if ((editor_name = getenv("EDITOR")) == NULL)
		editor_name = DEFAULTED;
	if ( prompt != NULL ) 
	  ui_print ( VNORMAL, "%s\n", prompt );
	return(runp(editor_name, editor_name, file, 0));
}
