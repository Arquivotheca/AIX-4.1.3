static char sccsid[] = "@(#)19	1.17.1.11  src/bos/usr/ccs/lib/libs/libs_read.c, libs, bos411, 9428A410j 7/4/94 18:13:28";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: 	rdst 
 *		readattr
 *		rdbase
 *		rdpwd
 *		rdgrps
 *		rdgrpsids
 *		rdgroup
 *		rdaudit
 *		rdst
 *		readattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <pwd.h>
#include <usersec.h>
#include <userpw.h>	/* for PW_NAMELEN */
#include <grp.h>
#include "libs.h"

extern struct	xlat	xtab[];
extern	char		*getstr ();		/* get a string from caller */
extern	char		*getlist ();		/* get a list from caller */

int	rdbase (struct eval *,char *,int,struct attr *,int);
int	rdpwd (struct eval *,char *,int,struct attr *,int);
int	rdgrps (struct eval *,AFILE_t,int,struct attr *,int);
int	rdgrpsids (struct eval *,AFILE_t,int,struct attr *,int);
int	rdgroup (struct eval *,char *,int,struct attr *,int);
int	rdaudit (struct eval *,AFILE_t,int,struct attr *,int);
int	rdst (struct eval *,AFILE_t,int,struct attr *,int);
int	readattr (struct ehead *,struct attr *,int,
		struct atfile *,int,struct eval *);
int	setintattr (struct atval *,uid_t );

/*
 * NAME: rdbase
 *                                                                    
 * FUNCTION: reads the passwd file and loads info into eval struct
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *		errno is set to ENOENT if member is not in data base.
 * PASSED:
 *	struct	eval 	*eval;		 where final info goes 
 *	char		*af;		 not used 
 *	int		fi;		 not used 
 *	struct	attr	*atab;		 attribute file table 
 *	int		nattr;		 number of possible attributes 
 */  

int
rdbase (struct eval *eval,char *af,int fi,struct attr *atab,int nattr)
{
struct	passwd	*pw;		/* return from getpwent() */
struct	atval	*aval;		/* pointer to attribute value struct */
char		*pgrp;		/* pointer to primary group string */
int		rc = 1;		/* return code initially set to fail */

	/* 
	 * attributes to this file are hard coded 
	 * in the attribute table because they won't change
	 */

	setpwent ();

	if ((pw = _getpwnam_shadow (eval->name,0)) != NULL)
	{
		/* get and set flags for id field */
		setintattr (&(eval->atvals[IDX_ID]), pw->pw_uid);

		/* translate the pgrp id to the appropriate string */
		aval = &(eval->atvals[IDX_PGRP]);
		if ((pgrp = IDtogroup (pw->pw_gid)) != NULL)
		{
			aval->val = getstr (pgrp, &(aval->flgs));
			aval->flgs |= CACHED;
		}
		else
		{
			aval->val = '\0';
			aval->flgs |= NOT_FOUND;
		}

		/* copy the "home" string */
		aval = &(eval->atvals[IDX_HOME]);
		if (pw->pw_dir && *pw->pw_dir)
		{
			aval->val = getstr (pw->pw_dir, &(aval->flgs));
			aval->flgs |= CACHED;
		}
		else
		{
			aval->val = '\0';
			aval->flgs |= NOT_FOUND;
		}

		/* copy the "shell" string */
		aval = &(eval->atvals[IDX_SHELL]);
		if (pw->pw_shell && *pw->pw_shell)
		{
			aval->val = getstr (pw->pw_shell, &(aval->flgs));
			aval->flgs |= CACHED;
		}
		else
		{
			aval->val = '\0';
			aval->flgs |= NOT_FOUND;
		}

		/* copy the "gecos" string */
		aval = &(eval->atvals[IDX_GECOS]);
		if (pw->pw_gecos && *pw->pw_gecos)
		{
			aval->val = getstr (pw->pw_gecos, &(aval->flgs));
			aval->flgs |= CACHED;
		}
		else
		{
			aval->val = '\0';
			aval->flgs |= NOT_FOUND;
		}
		rc = 0;

		/* copy the encrypted password string */
		aval = &(eval->atvals[IDX_PWD]);
		if (pw->pw_passwd && *pw->pw_passwd)
		{
			aval->val = getstr (pw->pw_passwd, &(aval->flgs));
			aval->flgs |= CACHED;
		}
		else
		{
			aval->val = '\0';
			aval->flgs |= NOT_FOUND;
		}
		rc = 0;
	}
	else
		errno = ENOENT;

	endpwent();
	return (rc);
}

/*
 * NAME: rdpwd
 *                                                                    
 * FUNCTION: reads the passwd file and loads all users that have 
 *		this group as the primary.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *		errno is set to ENOENT if member is not in data base.
 * PASSED:
 *	struct	eval 	*eval;		 points to specified name 
 *	char		*af;		 file handle 
 *	int		fi;		 file table index 
 *	struct	attr	*atab;		 attribute table 
 *	int		nattr;		 number of attributes 
 */  

int
rdpwd (struct eval *eval,char *af,int fi,struct attr *atab,int nattr)
{
struct	attr	*attr;		/* this attribute */
struct	atval	*av;		/* this attribute value */
char		*np, *bp, *buf;
int		bufsize;
gid_t		gid;		/* principal group id */
struct	passwd	*p;		/* return from getpwent() */
int		rc = 1;		/* return code initially set to error */
int		members = 0;	/* count of members checked */
int		offset;		/* tmp to save offset when realloc */

	/* find the group attribute at the group index offset */
	attr = atab + IDX_ID;

	/* set attribute value to value in cach at the attribute index */
	av = &(eval->atvals[attr->ai]);

	/* if group is not in cash get group name from the id */
	if (!(av->flgs & CACHED))
		grouptoID (eval->name, &gid);
	else
		gid = (gid_t) av->val;

	/* get some memory to hold this group's primary users */
	bufsize = BUFSIZ;
	if ((bp = buf = malloc (bufsize)) == NULL)
		return(1);

	/* get the primary users value structure */
	attr = atab + IDX_PUSERS;
	av = &(eval->atvals[attr->ai]);

	setpwent();

	while ((p = getpwent ()) != NULL)
	{
		members++;
		if (p->pw_gid == gid)
		{
			/* if we run out of memory then get some more */
			/*     add 2 for double-null */
			if (bp - buf > (bufsize - (strlen (p->pw_name) + 2)))
			{
				offset = bp - buf;
				bufsize += BUFSIZ;
				if ((buf = realloc (buf, bufsize)) == NULL)
				{
					endpwent();
					return(1);
				}
				bp = buf + offset;
			}
			/* add user name */
			np = p->pw_name;
			while (*bp++ = *np++);
			rc = 0;
		}
	}
	*bp = '\0';		/* end with double-null */
	av->val = buf;
	av->flgs |= CACHED | DYNAMIC;

	endpwent();

	/*
	 * if getpwent() succeeded at least once (ie. no read error etc.)
	 * and this member wasn't found set errno and return 
	 */

	if (members && rc)
		errno = ENOENT;

	return (rc);
}

/*
 * NAME: rdgrps
 *                                                                    
 * FUNCTION: build a list of group names of which this user is a member.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS:  0  : success
 *	    -1	: failure
 */  
int
rdgrps (struct eval *eval,AFILE_t af,int fi,struct attr *atab,int nattr)
{
#define	GRPSTORAGE	L_cuserid * NGROUPS_MAX	
	struct	group  *grp;		/* Group structure found via gid     */
	char 	*	buf = (char *)NULL; /* Storage for group names 	     */
	char 	*	bufptr;		/* Current point in "buf"   	     */	
	char	*	group;		/* Group name			     */
	char	*	grset;		/* User's group set		     */
	char	*	gsp;		/* User's group set pointer          */	
	char	*	end;		/* Ending gid delimeter ',' or '\0'  */	
	int		totsize = 1;	/* Total space used in buffer	     */
	gid_t		gid;		/* Group id			     */
	int		offset;		/* tmp to save offset when realloc   */
	int		bufsize = 0;	/* Current "buf" size		     */
	struct	atval  *av;		/* Attribute value		     */
#ifndef	_STRICT_NAMES
	char		maxmember[PW_NAMELEN];
	char		maxgroup[PW_NAMELEN];
#endif

	/* 
	 * Return the groupset and allocate local storage for it.  
	 * This is a comma separated list of group ids. 
	 */
	if (grset = getgrset(eval->name))
	{
		if ((grset = (char *)strdup(grset)) == (char *)NULL)
		{
			endgrent();
			return(-1);
		}
	}
	gsp = grset;
	
	while (gsp && *gsp)
	{
		gid = strtoul(gsp, &end, 10);
		
		/* 
		 * If an integer cannot be formed, the "end" pointer is set
		 * to the beginning of the string.
	   	 */
		if (end == gsp)
			break;

		gsp = end;	/* Move to the next group id (skip comma) */
		if (*gsp == ',')
			gsp++;
		
		if ((grp = getgrgid(gid)) == (struct group *)NULL)
			continue;
		
		/*
		 * Enforce AIX limits on group name lengths.  Currently 
		 * eight characters long.
		 */ 
		if (strlen(grp->gr_name) > (PW_NAMELEN-1))
#ifdef	_STRICT_NAMES
		{
			errno = ENAMETOOLONG;
			continue;
		}
#else
		{
			_chrncpy(maxgroup, grp->gr_name, PW_NAMELEN - 1);
        		maxgroup[PW_NAMELEN - 1] = (char)NULL;
			group = maxgroup;
		}
#endif
		else
			group = grp->gr_name;

		totsize += strlen(group) + 1;

		/* dynamic growth of group buffer */
		if (!bufsize)
		{
			bufsize = GRPSTORAGE;
			if ((bufptr = buf = (char *)
			     malloc(bufsize)) == (char *)NULL)
			{
				free(grset);
				endgrent();
				return (-1);
			}
		}
		else
		{
			if(totsize > bufsize)
			{
				offset   = bufptr - buf;
				bufsize += GRPSTORAGE;
				if((buf = (char *)
				    realloc(buf, bufsize)) == (char *)NULL)
				{
					free(grset);
					endgrent();
					return(-1);
				}
				bufptr = buf + offset;
			}
		}
		/* build a SEC_LIST of group names */
		while (*bufptr++ = *group++);
		*bufptr = '\0';
	}

	endgrent();

	if (grset) free(grset);	/* Free temporary storage */
		
	av = &(eval->atvals[IDX_GROUPS]);
	if (buf)
	{
		av->val = buf;
		av->flgs |= (CACHED | DYNAMIC);
	}
	else 
	{
		av->flgs |= NOT_FOUND;
		errno = ENOENT;
		return(-1);
	}
	return(0);
}

/*
 * NAME: rdgrpsids
 *                                                                    
 * FUNCTION: build a list of group ids of which this user is a member.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS:  0 	: success
 *	    -1  : failure
 */  
int
rdgrpsids (struct eval *eval,AFILE_t af,int fi,struct attr *atab,int nattr)
{
	char		*buf = (char *)NULL,
					/* Return storage of group ids  */
			*bufptr, 	/* Current position in "buf"	*/
			*gsp;		/* User's group set		*/
	struct	atval	*av;		/* Attribute value structure	*/


        if ((gsp = getgrset(eval->name)) && *gsp)
	{
		if ((bufptr = buf = (char *)
		     malloc(strlen(gsp) + 2)) == (char *)NULL)
			return(-1);

		while (*gsp)
		{
			while (*gsp && *gsp != ',')
				*bufptr++ = *gsp++;

			*bufptr++ = (char)NULL;

			if (*gsp == ',')
				gsp++;
		}
		*bufptr = (char)NULL;
	}

	av = &(eval->atvals[IDX_GROUPSIDS]);
	if (buf)
	{
		av->val = buf;
		av->flgs |= (CACHED | DYNAMIC);
	}
	else 
	{
		av->flgs |= NOT_FOUND;
		errno = ENOENT;
		return (-1);
	}
	return (0);
}

/*
 * NAME: rdgroup
 *                                                                    
 * FUNCTION: reads the group name info from /etc/group.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 - successful 
 *	    1 - failure
 */  
int
rdgroup (struct eval *eval,char *af,int fi,struct attr *atab,int nattr)
{
	struct	group	*gr;
	struct	atval	*aval;
	char	*	member;
	int		offset;		/* tmp to save offset when realloc */
	int		rc	= 0;
#ifndef	_STRICT_NAMES
	char		maxmember[PW_NAMELEN];
#endif

	setgrent();

	if ((gr = (struct group *)getgrnam(eval->name)) != (struct group *)NULL)
	{
		struct	attr * 	attr;
		char	**	mem = gr->gr_mem;
		char	*	np, 
			*	bp, 
			*	buf;
		int		bufsize;
		int		i;

		/* group id */
		attr = atab + IDX_ID;
		setintattr (&(eval->atvals[attr->ai]), gr->gr_gid);

		/* build group member list */
		attr = atab + IDX_USERS;

		bufsize = BUFSIZ;
		if ((bp = buf = malloc (bufsize)) == NULL)
		{
			endgrent();
			return(1);
		}

		for (i = 0; mem[i]; i++)
		{
			if ((strlen(mem[i]) + 1) > PW_NAMELEN)
			{
#ifdef	_STRICT_NAMES
				continue;
#else
				_chrncpy(maxmember, mem[i], PW_NAMELEN - 1);
        			maxmember[PW_NAMELEN - 1] = (char)NULL;
				member = maxmember;
#endif
			}
			else
				member = mem[i];

			if(bp - buf > (bufsize - (strlen (member) + 2)))
			{
				offset = bp - buf;
				bufsize += BUFSIZ;
				if ((buf = realloc (buf, bufsize)) == NULL)
				{
					endgrent();
					return(1);
				}
				bp = buf + offset;
			}
			/* add member name */
			np = member;
			while (*bp++ = *np++);
		}
		*bp = '\0';
		eval->atvals[attr->ai].val = buf;
		eval->atvals[attr->ai].flgs |= CACHED | DYNAMIC;
	}
	else
	{
		errno = ENOENT;
		rc = 1;
	}
	endgrent();

	return(rc);
}

static char DEFAULT[] = "default";

/*
 * NAME: rdaudit
 *                                                                    
 * FUNCTION: reads the audit name info from /etc/security/audit/config.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *
 */  

int
rdaudit (struct eval *eval,AFILE_t af,int fi,struct attr *atab,int nattr)
{
ATTR_t		at;
struct	attr	*a, *findattr ();
struct	atval	*av;
char		*val;

	if ((at = afgetrec (af, "users")) == NULL)
		return (1);

	a = atab + IDX_AUDIT;
	av = &(eval->atvals[a->ai]);
	if ((val = afgetatr (at, eval->name)) == NULL &&
	    (val = afgetatr (at, DEFAULT)) == NULL)
	{
		av->val = NULL;
		av->flgs |= NOT_FOUND;
		return (1);
	}
	av->val = getlist (val, &(av->flgs), av->siz);
	return (0);
}


/*
 * NAME: rdst
 *                                                                    
 * FUNCTION: reads the stanza files.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *
 */  

int
rdst (struct eval *eval,AFILE_t af,int fi,struct attr *atab,int nattr)
{
ATTR_t		at;
int		i;
struct	attr	*a;

	/* get the stanza of eval->name */
	if ((at = afgetrec (af, eval->name)) == NULL)
		if ((at = afgetrec (af,DEFAULT)) == NULL)
			return (-1);

	/* read in all attributes for this 'name' in this file */
	for (a = atab, i = 0; i < nattr; a++, i++)
	{
		/* is this the right file? */
		if (a->fi == fi)
		{
		char	*(*get)();

			if (get = xtab[a->xi].get)
			{
			struct	atval 	*aval = &(eval->atvals[i]);
			char		*val;

				/* last table entry is wild card */
				if (i == (nattr - 1))
				{
				/* another wild card so free the previous */
					if (aval->flgs & DYNAMIC)
					{
						free (aval->val);
						aval->flgs &= ~DYNAMIC;
					}
				}
				/* could get here with mult wild card attrs */
				if (aval->flgs & CACHED)
					continue;

				if ((val = afgetatr (at, a->atnam)))
				{
					aval->flgs |= a->qwerks;
					aval->val = (*(get))(val,&(aval->flgs));
				}

				else
					aval->flgs |= NOT_FOUND;
			}
		}
	}
	return (0);
}

/*
 * NAME: readattr
 *                                                                    
 * FUNCTION: reads the attribute from the data base files.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *
 */  

int
readattr (struct ehead *head,struct attr *atab,int nattr,
		struct atfile *ftab,int fi,struct eval *eval)
{
struct	atfile  *f = ftab + fi;
int		rc = 0;

	/* open or rewind */
	if ((f->fhdle = (*(f->open))(f->org, f->fhdle, head->mode)) == NULL)
	{
		/* can't open file */
		errno = EPERM;
		return (-1);
	}
	/* read (caches all attributes that are in this file) */
	rc = (*(f->read))(eval, f->fhdle, fi, atab, nattr);

	return (rc);
}

/*
 * NAME: setintattr
 *                                                                    
 * FUNCTION: sets the int values in the atval struct.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from libs routines.
 *                                                                   
 * RETURNS: 0 if successful 1 otherwise.
 *
 */  

int
setintattr (struct atval *at,uid_t val)
{
	at->val = (char *)val;
	at->siz = 1;
	at->flgs |= CACHED;
	at->flgs &= ~DYNAMIC;
}

