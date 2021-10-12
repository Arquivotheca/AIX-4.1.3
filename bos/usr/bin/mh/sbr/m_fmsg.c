static char sccsid[] = "@(#)68	1.3  src/bos/usr/bin/mh/sbr/m_fmsg.c, cmdmh, bos411, 9428A410j 6/15/90 22:12:58";
/* 
 * COMPONENT_NAME: CMDMH m_fmsg.c
 * 
 * FUNCTIONS: m_fmsg 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* m_fmsg.c - free a folder */

#include "mh.h"
#include <stdio.h>


void	m_fmsg (mp)
register struct msgs *mp;
{
    register int    i;

    if (mp == NULL)
	return;

    if (mp -> foldpath)
	free (mp -> foldpath);
#ifdef	MTR
    free ((char *) mp -> msgbase);
#endif	MTR
    for (i = 0; mp -> msgattrs[i]; i++)
	free (mp -> msgattrs[i]);
    free ((char *) mp);
}
