static char sccsid[] = "@(#)32	1.5  src/bos/usr/ccs/lib/libsrc/srcstattxt.c, libsrc, bos411, 9428A410j 2/26/91 14:54:59";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcstattxt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcstattxt
**    Title:	Get SRC Status Text For a Status Code
** PURPOSE:
**	To return a text representation of a status code.
** 
** SYNTAX:
**    chart * srcstattxt(statcd)
**    Parameters:
**	i short statcd - statcode to be transated into text
**
** RETURNS:
**	(char *) to status text
**
**/
#include "src.h"
#include <nl_types.h>
#include <limits.h>
char *src_get_msg();
extern char *src_def_stat[];
char *srcstattxt(statcd)
short statcd;
{
	char *stattxt;


	/* no status code supplied */
	if (statcd == 0)
		return("");
	
	if(statcd < FIRST_STATUS_SRC || statcd > LAST_STATUS_SRC)
		return("unknown status");

	/* get status code from the msgtable */
	stattxt=src_get_msg(SRCSTATSET,(int)statcd,src_def_stat[statcd-(FIRST_STATUS_SRC)]);
	return(stattxt);
}
