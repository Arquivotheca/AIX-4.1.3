static char sccsid[] = "@(#)69 1.10 src/bos/usr/bin/sccs/lib/chkid.c, cmdsccs, bos411, 9428A410j 3/30/94 12:38:37";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: chkid
 *
 * ORIGINS: 3, 10, 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

#include  "defines.h"
#define   ISUPPER isupper

char *strchr();

chkid(line,idstr)

char *line;
char *idstr;

{
	register char *lp;
	register char *p;
	extern int Did_id;

	if (!Did_id && any('%',line))
		if (!idstr || *idstr == 0)
			for(lp=line; *lp != 0; lp++) {
				if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%')
					if (ISUPPER(lp[1]))
						switch (lp[1]) {
						case 'J':
							break;
						case 'K':
							break;
						case 'N':
							break;
						case 'O':
							break;
						case 'V':
							break;
						case 'X':
							break;
						default:
							return(Did_id++);
						}
			}
		else
			{
			 p=idstr;
			 lp=line;
			 while(lp=strchr(lp,*p))
				if(!(strncmp(lp,p,strlen(p))))
					return(Did_id++);
				else
					++lp;
			}

	return(Did_id);
}
