static char sccsid[] = "@(#)25	1.7  src/bos/usr/ccs/lib/libs/acl_set.c, libs, bos411, 9428A410j 6/16/90 01:08:07";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: acl_set, acl_fset 
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
#include <sys/mode.h>

/*                                                                   
*
* EXTERNAL ROUTINES: acl_get, chacl
*
*/

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_set
*
* PURPOSE: Set the base access control information of a file.
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


int	acl_set(path, owner_mode, group_mode, default_mode)
char	*path;
int	owner_mode;
int	group_mode;
int	default_mode;
{

	struct	acl	*new_acl;
	char	*acl_get();

	/* 
	   new_acl is the existing acl of the file whose size we will 
	   truncate down to a base acl. 
	*/

	/* get the ACL of the file. */
	if ((new_acl = (struct acl *)acl_get(path)) == NULL)
		return(-1);

	/* Define the ACL length for a "base" ACL as the size of an ACL
	   without any (extended) ACL entries. */

	new_acl->acl_len = sizeof(struct acl) - sizeof(struct acl_entry);

	/* disable ACLs	*/
	new_acl->acl_mode &= ~S_IXACL; 

	/* Set the new base modes */
	new_acl->u_access = (ushort)owner_mode;
	new_acl->g_access = (ushort)group_mode;
	new_acl->o_access = (ushort)default_mode;

	if (chacl(path, new_acl, (int)(new_acl->acl_len)) != 0)
	{
		free(new_acl);
		return(-1);	/* failure */
	}

	free(new_acl);
	return(0);	/* success */
}


/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_fset
*
* PURPOSE: Set the base access control information of a file.
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

int	acl_fset(fildes, owner_mode, group_mode, default_mode)
int	fildes;
int	owner_mode;
int	group_mode;
int	default_mode;
{

	struct	acl	*new_acl;
	char	*acl_fget();

	/* 
	   new_acl is the existing acl of the file whose size we will 
	   truncate down to a base acl. 
	*/

	/* get the ACL of the file. */
	if ((new_acl = (struct acl *)acl_fget(fildes)) == NULL)
		return(-1);

	/* Define the ACL length for a "base" ACL as the size of an ACL
	   without any (extended) ACL entries. */

	new_acl->acl_len = sizeof(struct acl) - sizeof(struct acl_entry);

	/* disable ACLs	*/
	new_acl->acl_mode &= ~S_IXACL; 

	/* Set the new base modes */
	new_acl->u_access = (ushort)owner_mode;
	new_acl->g_access = (ushort)group_mode;
	new_acl->o_access = (ushort)default_mode;

	if (fchacl(fildes, new_acl, (int)(new_acl->acl_len)) != 0)
	{
		free(new_acl);
		return(-1);	/* failure */
	}

	free(new_acl);
	return(0);	/* success */
}
