static char sccsid[] = "@(#)85	1.20  src/bos/usr/ccs/lib/libs/getuserattr.c, libs, bos411, 9428A410j 1/3/92 10:46:21";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: getuserattr, putuserattr, IDtouser , nextuser
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
 
#include <sys/errno.h>
#include <ctype.h>
#include <usersec.h>
#include <pwd.h>
#include <fcntl.h>
#include <userpw.h>
#include "libs.h"

extern	int	chksessions();

/*
 * NAME: getuserattr
 *                                                                    
 * FUNCTION: get user attributes
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library routine. It returns the requested attribute values
 *	in malloc'd memory. A call to enduserdb() will free all the memory.
 *                                                                   
 * DATA STRUCTURES:
 *	
 * RETURNS: 
 *
 */  
int
getuserattr (char *user,char *atnam,void *val,int type)
{
	/* check user name's length */
#ifdef	_STRICT_NAMES
	if (strlen(user) > (PW_NAMELEN-1))
	{
		errno = ENAMETOOLONG;
		return (-1);
	}
#endif
		
	if (chksessions() == 0)
	{
		if (setuserdb (S_READ))
			return (-1);
	}

	if(getuattr(user,atnam,val,type,USER_TABLE))
		return (-1);
	else
		return (0);
}
/*
 * NAME: putuserattr
 *                                                                    
 * FUNCTION: put the specified user attribute
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  
int
putuserattr (char *user,char *atnam,void *val,int type)
{
	/* check user name's length */
#ifdef	_STRICT_NAMES
	if (strlen(user) > (PW_NAMELEN-1))
	{
		errno = ENAMETOOLONG;
		return (-1);
	}
#endif
		
	if (chksessions() == 0)
	{
		if(setuserdb (S_READ|S_WRITE))
			return (-1);
	}

	if (putuattr(user,atnam,val,type,USER_TABLE))
		return (-1);
	else 
		return (0);
}
/*
 * NAME: IDtouser
 *                                                                    
 * FUNCTION: get the user name associated with the specified user ID
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: pointer to a user name or NULL if not found
 */  

char *
IDtouser (uid_t id)
{
static	struct	passwd	*bp = NULL;
#ifdef	_STRICT_NAMES
static	char	user[PW_NAMELEN];
#else
static	char	user[PW_NAMELEN*2];
#endif
static	uid_t	id_save;			

	if ((!bp) || (id != id_save))			
	{	
		setpwent ();
		if ((bp = getpwuid (id)) != NULL)
		{
#ifdef	_STRICT_NAMES
			if (strlen (bp->pw_name) > (PW_NAMELEN-1))
			{
				errno = ENAMETOOLONG;
				endpwent();
				return (NULL);
			}
#endif
			id_save = id;				
			strncpy (user, bp->pw_name, sizeof user);
			user[(sizeof user) - 1] = '\0';
			endpwent();
			return (user);
		}
		errno = ENOENT;
		endpwent();
		return (NULL);
	}
	return (user);
}

/*
 * NAME: nextuser
 *                                                                    
 * FUNCTION: get next user
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library routine. It returns the next user in a linear
 *	search of the user database (/etc/passwd).
 *                                                                   
 * DATA STRUCTURES:
 *	
 * RETURNS: 
 *
 */  
char	*
nextuser(int mode,int arg)
{

static	FILE	*local_pwfile=NULL;
struct	passwd	*pw;
static	int	operation=0;
static	char	buf[BUFSIZ+1];
char	*s;
int	c;
int 	flags;
	
	/* arg must be 0 */
	if (arg != 0)
	{
		errno=EINVAL;
		return((char *)NULL);
	}
		
	/* cannot specify both modes */
	if((mode&S_SYSTEM) && (mode&S_LOCAL))
	{
		errno=EINVAL;
		return(NULL);
	}

	if(mode & S_SYSTEM)
	{
		operation = mode; /* save the mode */
		setpwent();
		mode = 0;
	}

	if(mode & S_LOCAL)
	{
		operation = mode; 	/* save the mode */
		/* keep the password file open */
		if(local_pwfile==NULL) 	/* open it initially */
		{
			if((local_pwfile=fopen(PWD_FILENAME,"r")) == NULL)
				return(NULL);

			/* open these files with a close on exec */
			flags = fcntl ((int)local_pwfile->_file,F_GETFD,0);
			flags |= FD_CLOEXEC;
			if (fcntl ((int)local_pwfile->_file,F_SETFD,flags))
				return (NULL);

		}
		else 			/* already open, just rewind it */
			rewind(local_pwfile);	
		mode = 0; 		/* now go and get the info	*/
	}

	if(mode == (int)NULL)
	{
		switch(operation)
		{
		  case	S_SYSTEM:
			if ((pw=getpwent()) == NULL)
			{
				errno = ENOENT;
				return(NULL);
			}
			else
				return(pw->pw_name);

		  case	S_LOCAL:
			/* loop until we get a valid entry */
			while(1)
			{
				if((fgets(buf,BUFSIZ,local_pwfile)) == NULL)
				{
					errno = ENOENT;
					return(NULL);
				}

				/* get only local entries */
				if(isalpha((int)buf[0]))
				{
					s=buf;
					c=0;

					/* loop until we get a colon */
					while((c<=BUFSIZ) && (*s!='\n'))
					{
						if(*s == ':')
						{
							*s='\0';
							return(&buf[0]);
						}
						c++;
						s++;
					}
					/* there's no colon */
					errno = ENOENT;
					return(NULL);
				}
			}	

		  default:
			errno=EINVAL;
			return(NULL);
		}
			
	}

	/* 'mode' is invalid */
	errno=EINVAL;
	return(NULL);
}
