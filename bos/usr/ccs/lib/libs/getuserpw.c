static char sccsid[] = "@(#)88	1.17  src/bos/usr/ccs/lib/libs/getuserpw.c, libs, bos411, 9428A410j 4/25/91 10:35:45";
/*
 * COMPONENT_NAME: (LIBS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: getuserpw, putuserpw 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/errno.h>
#include <sys/stat.h>
#include <usersec.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <userpw.h>
#include "libs.h"

extern  int	errno;

static	void	setflags(char *flags);
static	int	getflags(ulong pwflgs,char **flags);

/* routines defined in commonattr.c */
extern	char		*getstr ();		/* get a string from caller */
extern	char		*getlist ();		/* get a list from caller */
extern	char		*putlist ();		/* put a list into the cache */
extern	char		*nextrec ();		/* parse next record */
extern	char		*nextattr ();		/* parse to next attribute */
extern  struct	ehead	*setattr();		/* set the session handle */

/*
 * NAME: getuserpw
 *                                                                    
 * FUNCTION: get user password information                                                                    
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library routine. It returns the requested attribute values
 *	in malloc'd memory. A call to enduserdb() will free all the memory.
 *                                                                   
 * DATA STRUCTURES:
 *	
 *	Static userpw structure is kept as a one user cache.
 *
 * RETURNS: 
 *
 *	If successful a pointer to a userpw structure 
 *	otherwise a NULL
 *
 */  

static	struct	userpw	upw;

struct	userpw *
getuserpw (register char *user)
{
char		*flags;
register int	err;	/* place to save the previous errno */
#ifndef	_STRICT_NAMES
char	dummy[PW_NAMELEN];
#endif
		
	/* save previous errno to pass it on */
	err = errno;

	/* check user name's length */
	if (strlen(user) > (PW_NAMELEN-1))
	{
#ifdef	_STRICT_NAMES
		errno = ENAMETOOLONG;
		return (NULL);
#else
		strncpy (dummy, user, sizeof dummy);
		dummy[sizeof dummy - 1] = '\0';
		user = dummy;
#endif
	}
		
	/* open for read only */
	if (chkpsessions() == 0)
	{
		if (setpwdb (S_READ))
			return (NULL);
	}

	/* initialize the userpw struct */
	strcpy (upw.upw_name, user);
	upw.upw_passwd = NULL;
	upw.upw_lastupdate = 0;
	upw.upw_flags = 0;

	/* get the user's passwd from /etc/security/passwd */
	if (getuattr(user,SEC_PASSWD,&(upw.upw_passwd),SEC_CHAR,PASSWD_TABLE))
	{
		/* if there is an entry but no password */
		if (errno == ENOATTR)
			upw.upw_passwd = NULL;

		/* if there isn't an entry or if any other error */
		else
		{
			return (NULL);
		}
	}

	/*
	 * get the lastupdate field. Not checking for errors here 
	 * because we would have failed on the call above if 
	 * there were any major problems (we are just retrieving from
	 * cached memory at this point). We don't care about ENOATTR.
	 */

	getuattr(user,SEC_LASTUP,&(upw.upw_lastupdate),SEC_INT,PASSWD_TABLE);

	/* get the flags entry and convert from string to hex flags.  */
	getuattr(user,SEC_FLAGS,&(flags),SEC_LIST,PASSWD_TABLE);

	if (flags)
		setflags(flags);

	/* reset errno back to its original value */
	errno = err;

	return (&upw);
}

/*
 * NAME: setflags()
 *                                                                    
 * FUNCTION: converts the flags string to hex flags.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: none.
 *
 */  

static void
setflags(char *flags)
{
char	*ptr;
char	*val;

	ptr = flags;
	val = ptr;

	while (1)
	{
		if (*ptr == '\0')
		{
			*ptr++ = '\0';
			if (!strcmp(val,"ADMCHG"))
				upw.upw_flags |= PW_ADMCHG;

			if (!strcmp(val,"ADMIN"))
				upw.upw_flags |= PW_ADMIN;

			if (!strcmp(val,"NOCHECK"))
				upw.upw_flags |= PW_NOCHECK;

			if (*ptr == '\0')
				return;
			else
				val = ptr;
		}
		ptr++;
	}
}

/*
 * NAME: putuserpw
 *                                                                    
 * FUNCTION: put the specified user password attribute
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 *		0 if successful and non-zero on failure
 */  

int
putuserpw (register struct userpw *up)
{
char	*flags;

	/* open for read and write */
	if (chkpsessions() == 0)
	{
		if (setpwdb (S_READ | S_WRITE))
			return (-1);
	}

	/* put the password in /etc/security/passwd */
	if (putuattr(up->upw_name,SEC_PASSWD,up->upw_passwd,
				SEC_CHAR,PASSWD_TABLE) == -1)
		return (-1);

	/* put the lastupdate in /etc/security/passwd */
	if (putuattr(up->upw_name,SEC_LASTUP,up->upw_lastupdate,
				SEC_INT,PASSWD_TABLE) == -1)
		return (-1);

	/* set the admin flags */
	if (up->upw_flags)
	{
		if(getflags(up->upw_flags,&flags))
			return(-1);

		/* put the flags in /etc/security/passwd */
		if(putuattr(up->upw_name,SEC_FLAGS,(flags),
				SEC_LIST,PASSWD_TABLE) == -1)
		{
			free(flags);
			return (-1);
		}
		free(flags);
	}
	/* clear the admin flags */
	else
	{
		if(putuattr(up->upw_name,SEC_FLAGS,NULL,
				SEC_LIST,PASSWD_TABLE) == -1)
			return (-1);
	}

	/* write the data out to the files */
	if (putuattr(up->upw_name,NULL,NULL,SEC_COMMIT,PASSWD_TABLE) == -1)
		return (-1);

	return (0);
}


/*
 * NAME: getflags()
 *                                                                    
 * FUNCTION: converts the hex flags to strings.
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: none.
 *
 */  

static int
getflags(ulong pwflgs,char **flags)
{
register char	*ptr;
register int	siz;

	/* allocate space for flags string */
	siz = strlen("NOCHECK ") + strlen("ADMCHG ") + strlen("ADMIN ") +  5;

	if ((*flags = (char *)malloc(siz)) == NULL)
		return(-1);

	/* initialize string to NULL */
	**flags = '\0';

	/* parse string */
	if (pwflgs & PW_NOCHECK)
		strcpy(*flags,"NOCHECK ");

	if (pwflgs & PW_ADMCHG)
	{
		if (**flags)
			strcat(*flags,"ADMCHG ");
		else
			strcpy(*flags,"ADMCHG ");
	}

	if (pwflgs & PW_ADMIN)
	{
		if (**flags)
			strcat(*flags,"ADMIN ");
		else
			strcpy(*flags,"ADMIN ");
	}

	ptr = *flags;

	/* turn the space separated string to NULL separated */
	while (*ptr)
	{
		if (*ptr == ' ')
			*ptr = '\0';
		ptr++;
	}

	return(0);
}
