static char sccsid[] = "@(#)31	1.5  src/bos/usr/ccs/lib/libsrc/srcstathdr.c, libsrc, bos411, 9428A410j 7/16/91 14:53:57";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcstathdr
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
**    Name:	srcstathdr
**    Title:	Get SRC Status Header
** PURPOSE:
**	To retreve the status header from the SRC catalog of messages.
** 
** SYNTAX:
**    srcstathdr(hdr1,hdr2)
**    Parameters:
**	o char *hdr1 - objname field of statcode
**	o char *hdr2 - objtext field of statcode
**
**/
#include "src.h"
char *src_get_msg();
#include <nl_types.h>
#include <limits.h>
void srcstathdr(hdr1,hdr2)
char *hdr1;
char *hdr2;
{
	char *hdr;

	/* get status code from the msgtable */
	hdr=src_get_msg(SRCSTATHDR,1,"Subsystem         :Group            PID     Status");
	*hdr1='\0';

	strncat(hdr1,hdr,strcspn(hdr,":")-1);
	strcpy(hdr2,hdr+strcspn(hdr,":")+1);
}
