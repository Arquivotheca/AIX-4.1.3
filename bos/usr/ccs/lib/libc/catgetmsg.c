static char sccsid[] = "@(#)98	1.12  src/bos/usr/ccs/lib/libc/catgetmsg.c, libcmsg, bos411, 9428A410j 4/28/91 11:00:15";
/*
 * COMPONENT_NAME: (LIBCMSG) LIBC Message Catalog Functions
 *
 * FUNCTIONS: catgetmsg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <nl_types.h>


/*
 * NAME: catgetmsg
 *                                                                    
 * FUNCTION: Get a message from a catalog and copies it into a buffer.
 *                                                                    
 * ARGUMENTS:
 *	catd		- catalog descriptor returned from catopen
 *	setno		- message set number
 *	msgno		- message number within set
 *	buf		- output message text buffer
 *	buflen		- length of buf
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	Executes under a process.
 *
 * RETURNS: Returns a pointer to the buffer on success or
 *	a pointer to a NULL string on failure.
 *
 */  

char *catgetmsg(nl_catd catd, int setno, int msgno, char *buf, int buflen)
{
	char *mp;	/* ptr to message text */

	mp = catgets(catd, setno, msgno, "");
	strncpy(buf, mp, buflen);
	buf[buflen-1] = '\0';
	return (buf);
}
