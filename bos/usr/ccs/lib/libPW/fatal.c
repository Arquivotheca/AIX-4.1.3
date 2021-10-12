static char sccsid[] = "@(#)20	1.10  src/bos/usr/ccs/lib/libPW/fatal.c, libPW, bos411, 9428A410j 3/7/94 15:42:12";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: fatal
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

# include	"sys/types.h"
# include	"macros.h"
# include	"fatal.h"

#include "pw_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)

/*
	General purpose error handler.
	Typically, low level subroutines which detect error conditions
	(an open or create routine, for example) return the
	value of calling fatal with an appropriate error message string.
	E.g.,	return(fatal("can't do it"));
	Higher level routines control the execution of fatal
	via the global word Fflags.
	The macros FSAVE and FRSTR in <fatal.h> can be used by higher
	level subroutines to save and restore the Fflags word.
 
	The argument to fatal is a pointer to an error message string.
	The action of this routine is driven completely from
	the "Fflags" global word (see <fatal.h>).
	The following discusses the interpretation of the various bits
	of Fflags.
 
	The FTLMSG bit controls the writing of the error
	message on file descriptor 2.  The message is preceded
	by the string "ERROR: ", unless the global character pointer
	"Ffile" is non-zero, in which case the message is preceded
	by the string "ERROR [<Ffile>]: ".  A newline is written
	after the user supplied message.
 
	If the FTLCLN bit is on, clean_up is called with an
	argument of 0 (see clean.c).
 
	If the FTLFUNC bit is on, the function pointed to by the global
	function pointer "Ffunc" is called with the user supplied
	error message pointer as argument.
	(This feature can be used to log error messages).
 
	The FTLACT bits determine how fatal should return.
	If the FTLJMP bit is on longjmp(Fjmp,1) is
	called (Fjmp is declared in macros.h and must be set elsewhere
	via setjmp).
 
	If the FTLEXIT bit is on the value of userexit(1) is
	passed as an argument to exit(II)
	(see userexit.c).
 
	If none of the FTLACT bits are on
	(the default value for Fflags is 0), the global word
	"Fvalue" (initialized to -1) is returned.
 
	If all fatal globals have their default values, fatal simply
	returns -1.
*/

int	Fcnt;
int	Fflags;
char	*Ffile;
int	Fvalue = -1;
int	(*Ffunc)();
jmp_buf Fjmp;


fatal(msg)
char *msg;
{
	nl_catd catd;
	++Fcnt;
	if (Fflags & FTLMSG) {
		catd = catopen(MF_PW, NL_CAT_LOCALE);
		catclose(catd); 
		if (Ffile) {
			write(2,Ffile,length(Ffile));
			write(2,": ",2);
		}
		write(2,msg,length(msg));
		write(2, "\n", 1);
	}
	if (Fflags & FTLCLN)
		clean_up(0);
	if (Fflags & FTLFUNC)
		(*Ffunc)(msg);
	switch (Fflags & FTLACT) {
	case FTLJMP:
		longjmp(Fjmp,1);
	case FTLEXIT:
		exit(userexit(1));
	}
	return(Fvalue);
}
