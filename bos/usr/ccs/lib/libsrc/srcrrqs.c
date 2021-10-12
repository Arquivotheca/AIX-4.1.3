static char sccsid[] = "@(#)23	1.3  src/bos/usr/ccs/lib/libsrc/srcrrqs.c, libsrc, bos411, 9428A410j 6/16/90 02:36:35";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcrrqs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcsrrqs
**    Title:	Subsystem Request SRC Header Receiver
** PURPOSE:
**	To remember the SRCHEADER that contains the return address information
**	that the subsystem needs to reply to any request forwarded by SRC.
** 
** SYNTAX:
**    srcrrqs (p_pkt)
**    Parameters:
**	i char *p_pkt - pointer to packet received from SRC
**		
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**	copy return address into static srchdr buffer
**	return pointer to static srchdr buffer.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	For message queue systems the address passed in as p_pkt
**	must point past the mtype field.
**
**	The srchdr buffer will be replaced the next time this
**	function is called.
**
** RETURNS:
**	struct srchdr *
**
**/
#include "src.h"

static struct srchdr srchdr;

struct srchdr *srcrrqs (p_pkt)
char *p_pkt; 	/* pointer to socket packet or message queue message */
{
	/* copy srchdr info to save area */
	memcpy((void *)&srchdr,(void *)p_pkt,(size_t)sizeof(struct srchdr));

	return(&srchdr);
}
