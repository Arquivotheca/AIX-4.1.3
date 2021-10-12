static char sccsid[] = "@(#)70  1.10  src/bos/usr/bin/que/libque/tcp.c, cmdque, bos411, 9428A410j 6/3/91 12:02:10";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: localhost, remote
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "../common.h"
#include <time.h>
#include <sys/types.h>
#include <IN/standard.h>
#include <ctype.h>

#include "libq_msg.h"

char * localhost()
{
	static char lcalhost[HOST_SZ];
	static boolean known=FALSE;
 
	if (!known)
		if (gethostname(lcalhost,HOST_SZ) == (-1))
			syserr((int)EXITFATAL,GETMESG(MSGGETH,"Could not get host name."));
	if ( 0 == strlen(lcalhost) )
		strcpy(lcalhost,"localhost");
	return(lcalhost);
}


boolean remote(host)
char * host;
{

	/* if blank input, assume it's local and return false*/
	if (!host)
		return(FALSE);

	if (!host[0])
		return(FALSE);


	if (strncmp(localhost(),host,HOST_SZ)==0)
		return(FALSE);
	else
		return(TRUE);
}

