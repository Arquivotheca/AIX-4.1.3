static char sccsid[] = "@(#)52	1.5.1.1  src/bos/usr/ccs/lib/libs/acl_chg.c, libs, bos411, 9428A410j 2/5/93 10:51:52";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: acl_chg, acl_fchg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1983
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/access.h>
#include <sys/acl.h>
#include <sys/errno.h>

/*                                                                   
*
* EXTERNAL ROUTINES: acl_get, chacl
*
*/

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_chg
*
* PURPOSE: Change the access rights to a file.
*
* NOTES: 
* Version 3.1 "real" routine.
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a value of 0 is returned.
* Otherwise, a value of -1 is returned.
*
*/  

int acl_chg(path, how, mode, who)
char *path;	/* file to alter */
int how;	/* how to alter access rights */
int mode;	/* what alterations to make */
int who;	/* who is affected */
{

	int 	base_ACL = 0;		/* flag indicating base ACL only */
	struct acl_entry	*end;	/* pointer to end of ACL */
	struct	acl		*aclp;	/* ACL pointer */
	struct	acl_entry	*acep;  /* ACL entry pointer */
	char	*acl_get();


	/* Get the ACL of the file. */
	if ((aclp = (struct acl *)acl_get(path)) == NULL)
	{
		return(-1);
	}
	

	/* Determine existence of ANY (extended) ACL entries. */
	if (aclp->acl_len <= (sizeof(struct acl) - sizeof(struct acl_entry)))
		base_ACL = 1;


	/* perform the specified operation */
	switch (how)
	{
		case ACC_PERMIT:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access |= mode; /* add permission */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access |= mode; /* add permission */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access |= mode;	/* add permission */
				aclp->o_access |= mode; /* add permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access &= ~mode; /* remove restriction */
						else
							acep->ace_access |= mode;  /* add permission     */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access |= mode; /* add permission */
				aclp->g_access |= mode; /* add permission */
				aclp->o_access |= mode; /* add permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access &= ~mode; /* remove restriction */
						else
							acep->ace_access |= mode;  /* add permission     */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
		break;

		case ACC_DENY:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access &= ~mode; /* remove permission */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access &= ~mode; /* remove permission */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access &= ~mode; /* remove permission */
				aclp->o_access &= ~mode; /* remove permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access |= mode;  /* add restriction   */
						else
							acep->ace_access &= ~mode; /* remove permission */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access &= ~mode; /* remove permission */
				aclp->g_access &= ~mode; /* remove permission */
				aclp->o_access &= ~mode; /* remove permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access |= mode;  /* add restriction   */
						else
							acep->ace_access &= ~mode; /* remove permission */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
			break;

		case ACC_SPECIFY:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access = mode; /* set permissions */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access = mode; /* set permissions */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access = mode; /* set permissions */
				aclp->o_access = mode; /* set permissions */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access = ~mode; /* set restrictions */
						else
							acep->ace_access = mode;  /* set permissions  */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access = mode; /* set permissions */
				aclp->g_access = mode; /* set permissions */
				aclp->o_access = mode; /* set permissions */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						if (acep->ace_type == ACC_DENY)
							acep->ace_access = ~mode; /* set restrictions */
						else
							acep->ace_access = mode;  /* set permissions  */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
			break;

		default:
			/* return -1 for failure */
			free(aclp);
			errno = EINVAL;
			return(-1);

	}

	/* chacl input file */
	if (chacl(path, aclp, (int)(aclp->acl_len)) == 0)
	{
		/* return 0 for success */
		free(aclp);
		return(0);
	}
	else
	{
		/* return -1 for failure */
		free(aclp);
		return(-1);
	}

}

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: acl_fchg
*
* PURPOSE: Change the access rights to a file.
*
* NOTES: 
* Version 3.1 "real" routine.
* acl_fchg uses ttyname() to get the filename, and passes
*
* RETURN VALUE DESCRIPTION: 
* Upon successful completion, a value of 0 is returned.
* Otherwise, a value of -1 is returned.
*
*/  


int acl_fchg(fildes, how, mode, who)
int fildes;	/* file to alter */
int how;	/* how to alter access rights */
int mode;	/* what alterations to make */
int who;	/* who is affected */
{
	int 	base_ACL = 0;		/* flag indicating base ACL only */
	struct acl_entry	*end;	/* pointer to end of ACL */
	struct	acl		*aclp;	/* ACL pointer */
	struct	acl_entry	*acep;  /* ACL entry pointer */
	char	*acl_fget();

	/* Get the ACL of the file. */
	if ((aclp = (struct acl *)acl_fget(fildes)) == NULL)
	{
		return(-1);
	}
	

	/* Determine existence of ANY (extended) ACL entries. */
	if (aclp->acl_len <= (sizeof(struct acl) - sizeof(struct acl_entry)))
		base_ACL = 1;


	/* perform the specified operation */
	switch (how)
	{
		case ACC_PERMIT:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access |= mode; /* add permission */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access |= mode; /* add permission */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access |= mode;	/* add permission */
				aclp->o_access |= mode; /* add permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access &= ~mode; /* remove restriction */
						else
							acep->ace_access |= mode;  /* add permission     */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access |= mode; /* add permission */
				aclp->g_access |= mode; /* add permission */
				aclp->o_access |= mode; /* add permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access &= ~mode; /* remove restriction */
						else
							acep->ace_access |= mode;  /* add permission     */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
		break;

		case ACC_DENY:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access &= ~mode; /* remove permission */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access &= ~mode; /* remove permission */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access &= ~mode; /* remove permission */
				aclp->o_access &= ~mode; /* remove permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access |= mode;  /* add restriction   */
						else
							acep->ace_access &= ~mode; /* remove permission */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access &= ~mode; /* remove permission */
				aclp->g_access &= ~mode; /* remove permission */
				aclp->o_access &= ~mode; /* remove permission */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access |= mode;  /* add restriction   */
						else
							acep->ace_access &= ~mode; /* remove permission */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
			break;

		case ACC_SPECIFY:
			switch (who)
			{
			  case ACC_OBJ_OWNER:
				aclp->u_access = mode; /* set permissions */
				break;

			  case ACC_OBJ_GROUP:
				aclp->g_access = mode; /* set permissions */
				break;

			  case ACC_OTHERS:
				/* Everyone other than the owner. */
				aclp->g_access = mode; /* set permissions */
				aclp->o_access = mode; /* set permissions */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						/* restrictive entries are stored as negatives */
						if (acep->ace_type == ACC_DENY)
							acep->ace_access = ~mode; /* set restrictions */
						else
							acep->ace_access = mode;  /* set permissions  */
					}
				}
				break;

			  case ACC_ALL:
				aclp->u_access = mode; /* set permissions */
				aclp->g_access = mode; /* set permissions */
				aclp->o_access = mode; /* set permissions */
				if (!base_ACL)
				{
					/* loop through the ACL to every extended entry */
					for(acep=aclp->acl_ext,end=acl_last(aclp);acep<end;acep=acl_nxt(acep))
					{
						if (acep->ace_type == ACC_DENY)
							acep->ace_access = ~mode; /* set restrictions */
						else
							acep->ace_access = mode;  /* set permissions  */
					}
				}
				break;

			  default:
				/* return -1 for failure */
				free(aclp);
				errno = EINVAL;
				return(-1);
			}
			break;

		default:
			/* return -1 for failure */
			free(aclp);
			errno = EINVAL;
			return(-1);

	}

	/* fchacl input file */
	if (fchacl(fildes, aclp, (int)(aclp->acl_len)) == 0)
	{
		/* return 0 for success */
		free(aclp);
		return(0);
	}
	else
	{
		/* return -1 for failure */
		free(aclp);
		return(-1);
	}
}
