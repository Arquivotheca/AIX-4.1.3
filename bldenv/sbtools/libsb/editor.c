static char sccsid[] = "@(#)32  1.1  src/bldenv/sbtools/libsb/editor.c, bldprocess, bos412, GOLDA411a 4/29/93 12:20:03";
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

#include <stdio.h>
#include <ode/interface.h>

char *getenv();

#define DEFAULTED "vi"

int
editor(file, prompt)
register char *file, *prompt;
{
	register char *editor;

	if ((editor = getenv("EDITOR")) == NULL)
		editor = DEFAULTED;
	if (*prompt) 
	  ui_print ( VNORMAL, "%s\n", prompt );
	return(runp(editor, editor, file, 0));
}
