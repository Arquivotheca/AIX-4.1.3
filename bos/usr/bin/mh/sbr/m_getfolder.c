static char sccsid[] = "@(#)72	1.3  src/bos/usr/bin/mh/sbr/m_getfolder.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:15";
/* 
 * COMPONENT_NAME: CMDMH m_getfolder.c
 * 
 * FUNCTIONS: m_getfolder 
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


/* m_getfolder.c - get the current folder */

#include "mh.h"
#include <stdio.h>


char *m_getfolder()
{
    register char  *folder;

    if ((folder = m_find (pfolder)) == NULL || *folder == 0)
	folder = defalt;
    return folder;
}
