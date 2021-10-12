static char sccsid[] = "@(#)26	1.4  src/bos/usr/ccs/lib/libs/acl_put.c, libs, bos411, 9428A410j 6/16/90 01:08:03";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: acl_put, acl_fput 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/acl.h>


/*                                                                   
*
* EXTERNAL ROUTINES: chacl
*
*/

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_put
*
* PURPOSE: Set the access control information of a file.
*
* NOTES: 
* Version 3.1 "real" routine.
*
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a value of 0 is returned.
* Otherwise, a value of -1 is returned.
*
*/

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */


int	acl_put(path, aclp, release)
char	*path;
char	*aclp;
int	release;
{
	if (chacl(path, (struct acl *)aclp, ((struct acl *)aclp)->acl_len) != 0)
		return(-1);	/* failure */

	/* if release == 0, then retain the pointer for later use. */
	if (release != 0)
		free(aclp);

	return(0);	/* success */
}

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_fput
*
* PURPOSE: Set the access control information of a file.
*
* NOTES: 
* Version 3.1 "real" routine.
*
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a value of 0 is returned.
* Otherwise, a value of -1 is returned.
*
*/

int	acl_fput(fildes, aclp, release)
int	fildes;
char	*aclp;
int	release;
{
	if (fchacl(fildes, (struct acl *)aclp, ((struct acl *)aclp)->acl_len) != 0)
		return(-1);	/* failure */

	/* if release == 0, then retain the pointer for later use. */
	if (release != 0)
		free(aclp);

	return(0);	/* success */
}
