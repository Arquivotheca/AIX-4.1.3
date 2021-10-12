static char sccsid[] = "@(#)88	1.1  src/bos/usr/ccs/lib/libs/getportattr.c, libs, bos411, 9428A410j 10/4/93 11:17:34";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: getportattr, putportattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/errno.h>
#include <ctype.h>
#include <usersec.h>
#include <pwd.h>
#include <fcntl.h>
#include <userpw.h>
#include "libs.h"

extern	int	chksessions();

/*
 * NAME: getportattr
 *
 * FUNCTION: get port attributes
 *
 * EXECUTION ENVIRONMENT:
 *	This is a library routine. It returns the requested attribute values
 *	in malloc'd memory. A call to enduserdb() will free all the memory.
 *
 * RETURNS:
 *	zero on success, non-zero on failure.
 */
int
getportattr (char *port,char *atnam,void *val,int type)
{
	if (chksessions() == 0)
	{
		if (setuserdb (S_READ))
			return (-1);
	}

	if(getuattr(port,atnam,val,type,PORT_TABLE))
		return (-1);
	else
		return (0);
}
/*
 * NAME: putportattr
 *
 * FUNCTION: put the specified port attribute
 *
 * EXECUTION ENVIRONMENT: library
 *
 * RETURNS:
 *	zero on success, non-zero on failure.
 */
int
putportattr (char *port,char *atnam,void *val,int type)
{
	if (chksessions() == 0)
	{
		if(setuserdb (S_READ|S_WRITE))
			return (-1);
	}

	if (putuattr(port,atnam,val,type,PORT_TABLE))
		return (-1);
	else 
		return (0);
}
