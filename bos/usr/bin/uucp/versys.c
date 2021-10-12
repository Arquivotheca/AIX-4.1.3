static char sccsid[] = "@(#)44	1.5  src/bos/usr/bin/uucp/versys.c, cmduucp, bos411, 9428A410j 6/17/93 14:28:21";
/* 
 * COMPONENT_NAME: CMDUUCP versys.c
 * 
 * FUNCTIONS: versys 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.versys.c
	versys.c	1.1	7/29/85 16:34:26
*/
#include "uucp.h"
/* VERSION( versys.c	5.2 -  -  ); */

#include <stdlib.h>   
extern nl_catd catd;
/*
 * verify system name
 * input:
 *	name	-> system name
 * returns:  
 *	0	-> success
 *	FAIL	-> failure
 */
versys(name)
char *name;
{
	register char *iptr;
	char line[300];

	setlocale(LC_ALL,"");
	if (MB_CUR_MAX == 1) { 
	/* check for illegal chars in site name */
		char *namep;
		namep = name;
		while (*namep)
	  	     if (! (isascii(*namep++))) { 
	 	 	   fprintf (stderr, MSGSTR(MSG_VER_1, 
      "A non-ascii character was detected in system name %s\n"), name);
		    	   exit( 2 );
		}
        } 
	if (EQUALS(name, Myname))
		return(0);

	
	while (getsysline(line, sizeof(line))) {
		if((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t') || 
			(line[0] == '\n'))
			continue;

		if ((iptr=strpbrk(line, " \t")) == NULL)
		    continue;	/* why? */
		*iptr = '\0';
		if (EQUALS(name, line)) {
			sysreset();
			return(0);
		}
	}
	sysreset();
	return(FAIL);
}
