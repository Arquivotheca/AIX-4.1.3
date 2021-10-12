static char sccsid[] = "@(#)64	1.26.1.1  src/bos/usr/ccs/lib/libs/getgroupattr.c, libs, bos411, 9428A410j 7/22/92 17:40:03";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: getgroupattr, putgroupattr, IDtogroup, grouptoID, nextgroup
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
 
#include <sys/stat.h>
#include <sys/errno.h>
#include <usersec.h>
#include <userpw.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include "libs.h"

extern	char	*calloc();

extern int	errno;

static	int	loadgroups (void);
char	*nextgroup(int,int);

/*
 * NAME: getgroupattr
 *                                                                    
 * FUNCTION: get group attributes
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
getgroupattr (register char *group, register char *atnam, 
		register void *val, register int type)
{
	/* check group's name length */
#ifdef	_STRICT_NAMES
	if (strlen(group) > (PW_NAMELEN-1))
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

	/* the GROUP_TABLE flag indicates the group static linked list */
	if (getuattr(group,atnam,val,type,GROUP_TABLE))
		return (-1);
	else
		return (0);
}
/*
 * NAME: putgroupattr
 *                                                                    
 * FUNCTION: put the specified group attribute
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  
int
putgroupattr (register char *group, register char *atnam, 
		register void *val, register int type)
{
	/* check group's name length */
#ifdef	_STRICT_NAMES
	if (strlen(group) > (PW_NAMELEN-1))
	{
		errno = ENAMETOOLONG;
		return (-1);
	}
#endif
		
	if (chksessions() == 0)
	{
		if (setuserdb ( S_READ | S_WRITE ))
			return (-1);
	}

	/* the GROUP_TABLE flag indicates the group static linked list */
	if (putuattr(group,atnam,val,type,GROUP_TABLE))
		return (-1);
	else
		return (0);
}

/* Objects for the grouptoID and IDtogroup routines: */  
#define	MAXGROUPS	500 /* first 500 ids will be indexes into the glist */

struct	grplist
{
	gid_t	id;
#ifdef	_STRICT_NAMES
	char	name[PW_NAMELEN];
#else
	char	name[PW_NAMELEN*2];
#endif
};

static  struct	grplist *glist = (struct grplist *)NULL; /* head of table */
static  int	count = 0;   /* marks the count the group table */
static	int	total = 0;	/* total count of groups */

int
endgroups ()
{
	if (glist)
		free (glist);

	glist = (struct grplist *)NULL;
	total = count = 0;
	return (0);
}

/*
 * NAME: addgroup
 *                                                                    
 * FUNCTION: adds a glist struct to the local cache and returns index of it
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: -1 if fail, index of glist element if ok
 */  

static	int
addgroup(gr)
struct group *gr;
{
register int	siz;

	/* allocate enough for MAXGROUPS structures */
	if (!glist) {
		siz = sizeof (struct grplist);
		if ((glist = (struct grplist *)calloc(MAXGROUPS,siz)) == NULL)
			return (-1);
	}

#ifdef	_STRICT_NAMES
	if (strlen(gr->gr_name) > (PW_NAMELEN-1))
		return(0);
#endif
	count++;
	/* see if we still have room */
	if (count >= MAXGROUPS)
	{
		siz = (((total+1)/MAXGROUPS)+1) * 
			sizeof (struct grplist) * MAXGROUPS;
		if((glist=(struct grplist *)realloc(glist,siz)) == NULL)
			return (-1);
		count = 0;
	}
	/*
	 * copy the group info into the
	 * glist structure
	 */
	strncpy (glist[total].name, gr->gr_name, sizeof glist[0].name);
	glist[total].name[sizeof(glist[0].name) - 1] = '\0';
	glist[total++].id = gr->gr_gid;

	/* see if we need space for last NULL */
	if (count == MAXGROUPS)
	{
		siz=((total+1) * MAXGROUPS) + (2*(sizeof(struct grplist)));
		if((glist=(struct grplist *)realloc(glist,siz)) == NULL)
			return (-1);
	}

	glist [total].name[0] = '\0';
	glist [total].id = (gid_t)NULL;

	return(total - 1);
}

/*
 * NAME: IDtogroup
 *                                                                    
 * FUNCTION: get the group name associated with the specified group ID
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: pointer to a group name or NULL if not found
 */  
char *
IDtogroup (gid_t id)
{
int		i;
struct group *gr;

	/* check for it in the local cache */
	for (i = 0;i<total;i++)
	{
		if (id == glist[i].id)
		{
			return (glist[i].name);
		}
	}

	/* query for it and add to cache if found */
	if ((gr = getgrgid(id)) && (i = addgroup(gr)) != -1)
		return(glist[i].name);

	if (!gr)
		errno = ENOENT;
	return (NULL);
}

/*
 * NAME: 	grouptoID
 *                                                                    
 * FUNCTION: 	Convert a group name to a group ID
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *
 *		The caller passes a pointer to a gid_t
 *                                                                   
 * RETURNS: 	The number of group names converted to IDs (1)
 */  
int
grouptoID (char *name, gid_t *group)
{
int	i;
struct group *gr;

	/* check for it in the local cache */
	for (i = 0;i<total; i++)
	{
		if (strncmp (name, glist[i].name, sizeof glist[0].name) == 0)
		{
			*group = glist[i].id;
			return (1);
		}
	}

	/* query for it and add to cache if found */
	if ((gr = getgrnam(name)) && (i = addgroup(gr)) != -1) {
		*group = glist[i].id;
		return(1);
	}

	if (!gr)
		errno = ENOENT;
	return (0);
}

/*
 * NAME: nextgroup
 *                                                                    
 * FUNCTION: get next group
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library routine. It returns the next group in a linear
 *	search of the group database (/etc/group).
 *                                                                   
 * DATA STRUCTURES:
 *	
 * RETURNS: 
 *
 */  
char	*
nextgroup(int mode,int arg)
{

static	FILE	*local_grfile=NULL;
struct	group	*gr;
static	int	operation=0;
static	char	buf[BUFSIZ+1];
char	*s;
int	c;
register int flags;
	
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
		return((char *)NULL);
	}

	if(mode & S_SYSTEM)
	{
		operation = mode; /* save the mode */
		setgrent();
		mode = 0;
	}

	if(mode & S_LOCAL)
	{
		operation = mode; 	/* save the mode */
		/* keep the group file open */
		if(local_grfile==NULL) 	/* open it initially */
		{
			if((local_grfile=fopen(GROUP_FILENAME,"r")) == NULL)
				return((char *)NULL);

			/* open these files with a close on exec */
			flags = fcntl (local_grfile->_file,F_GETFD,0);
			flags |= FD_CLOEXEC;
			if (fcntl (local_grfile->_file,F_SETFD,flags))
				return ((char *)NULL);
		}
		else 			/* already open, just rewind it */
			rewind(local_grfile);	
		mode = 0; 		/* now go and get the info	*/
	}

	if(mode == 0)
	{
		switch(operation)
		{
		  case	S_SYSTEM:
			if ((gr=getgrent()) == NULL)
			{
				errno = ENOENT;
				return((char *)NULL);
			}
			else
				return(gr->gr_name);

		  case	S_LOCAL:
			/* loop until we get a valid entry */
			while(1)
			{
				if((fgets(buf,BUFSIZ,local_grfile)) == NULL)
				{
					errno = ENOENT;
					return((char *)NULL);
				}

				/* get only local entries */
				if(isalpha(buf[0]))
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
					return((char *)NULL);
				}
			}	

		  default:
			errno=EINVAL;
			return((char *)NULL);
		}
			
	}

	/* 'mode' is invalid */
	errno=EINVAL;
	return((char *)NULL);
}
