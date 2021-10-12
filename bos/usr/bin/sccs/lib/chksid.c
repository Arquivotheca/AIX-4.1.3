static char sccsid[] = "@(#)75 1.7 src/bos/usr/bin/sccs/lib/chksid.c, cmdsccs, bos411, 9428A410j 2/6/94 10:35:17";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: chksid
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
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

# include	"defines.h"

chksid(p,sp)
char *p;
register struct sid *sp;
{
	if (*p ||
		(sp->s_rel == 0 && sp->s_lev) ||
		(sp->s_lev == 0 && sp->s_br) ||
		(sp->s_br == 0 && sp->s_seq))
			fatal(MSGCO(INVSID, "\nThe specified SID is not valid.  Use the sact command\n\
\tto check the p-file for valid SID numbers. (co8)\n"));
}
