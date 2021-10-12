static char sccsid[] = "@(#)24	1.5  src/bos/usr/ccs/lib/libs/acl_get.c, libs, bos411, 9428A410j 6/16/90 01:07:58";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: acl_get, acl_fget 
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
#include <sys/errno.h>

/*                                                                   
*
* EXTERNAL ROUTINES: statacl
*
*/

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_get
*
* PURPOSE: Get the access control information of a file.
*
* NOTES: 
* Version 3.1 "real" routine.
*
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a pointer to the ACL structure is 
* returned.  Otherwise, a NULL pointer is returned. am
*
*/  


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern	int	errno;

char 	*acl_get(path)
char 	*path;
{
	struct	acl	*aclp;	/* ACL pointer */
	int	acl_length;	/* total length of ACL */

	/* allocate a page for the acl. we'll re-allocate more if necessary */
	acl_length = BUFSIZ;

	while(1)	/* loop around in case we need more space */
	{
		if ( (aclp = (struct acl *)malloc(acl_length)) == NULL)
		{
			return(NULL);
		}

		/* if statacl succeeds, return the ACL pointer */
		if (statacl(path, 0, aclp, acl_length) == 0)
			return( (char *)aclp );

		if (errno != ENOSPC)	/* error other than 'no space left' */
		{
			free(aclp);
			return(NULL);
		}

		/* we need more space for ACL's, allocate aclp->acl_len bytes */
		acl_length = aclp->acl_len;
		free(aclp); 		/* free the pointer to re-allocate */
	}
}

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_fget
*
* PURPOSE: Get the access control information of a file.
*
* NOTES: 
* Version 3.1 "real" routine.
*
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a pointer to the acl structure is 
* returned.  Otherwise, a NULL pointer is returned. am
*
*/  

char 	*acl_fget(fildes)
int 	fildes;
{
	struct	acl	*aclp;	/* ACL pointer */
	int	acl_length;	/* total length of ACL */

	/* allocate a page for the acl. we'll re-allocate more if necessary */
	acl_length = BUFSIZ;

	while(1)	/* loop around in case we need more space */
	{
		if ( (aclp = (struct acl *)malloc(acl_length)) == NULL)
		{
			return(NULL);
		}

		/* if fstatacl succeeds, return the ACL pointer */
		if (fstatacl(fildes, 0, aclp, acl_length) == 0)
			return( (char *)aclp );

		if (errno != ENOSPC)	/* error other than 'no space left' */
		{
			free(aclp);
			return(NULL);
		}

		/* we need more space for ACL's, allocate aclp->acl_len bytes */
		acl_length = aclp->acl_len;
		free(aclp); 		/* free the pointer to re-allocate */
	}
}
