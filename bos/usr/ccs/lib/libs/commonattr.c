static char sccsid[] = "@(#)95	1.28.1.2  src/bos/usr/ccs/lib/libs/commonattr.c, libs, bos411, 9428A410j 11/29/93 17:10:28";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: setattr,endattr,getuattr,putuattr,enduserdb,setuserdb,endpwdb,
 *	      setpwdb,getgenericattr,putgenericattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <usersec.h>
#include <sysck.h>
#include "libs.h"	/* local library defines */
#include "tables.h"	/* static tables */

extern struct	xlat	xtab[];

int	rmufile(register char *name,register int flag,register int table);

struct eval *
neweval (name, atsiz)
char	*name;
int	atsiz;
{	
struct	eval	*eval;

	/* allocate new name structure (NOTE: calloc returns NULL'd space) */
	if ((eval = (struct eval *) calloc (1, sizeof (struct eval))) == NULL)
		return (NULL);

	/* name will be NULL if called from nextentry () */
	if (name)
	{
		if ((eval->name = malloc (strlen (name) + 1)) == NULL)
			return (NULL);

		strcpy (eval->name, name);
	}
	if((eval->atvals=(struct atval *)calloc(atsiz,
			sizeof(struct atval))) == NULL)
		return (NULL);

	return (eval);
}

struct	attr	*
findattr (atnam, type, atab, nattr)
char	*atnam;		/* the attribute name the caller is looking for */
int	type;		/* the data type (also index into translate table:
							(struct attr).xi */
struct	attr 	*atab;		/* the attribute table */
int		nattr;		/* size of attribute table */
{
register struct	attr	*a;

	for (a = atab; a < atab+nattr; a++)
	{
		if (!strcmp (atnam, a->atnam))
			break;
	}
	/* 
	 * If the search for the attribute name fails
	 *      then the caller can specify the attribute name. 
	 *	If type specified is 0 then the default type SEC_LIST is used.
	 */
	if (a == atab+nattr)
	{
		/* last table entry is a wild card */
		a--;

		/*
		 * if this entry is currently being used
		 * see if it contains the attribute value that
		 * we want.  if it doesn't mark it as updated.
		 */
		if (a->atnam && (*a->atnam != '*') && strcmp(atnam,a->atnam))
			a->qwerks |= REUSED;

		/* use caller's type and attribute name */
		a->xi = (type) ? type : SEC_LIST;
		a->atnam = atnam;
	}

	/* if we want to delete this attribute return the struct */
	if ((type == SEC_DELETE) || (type == SEC_NEW))
		return (a);

	/* 
	 * if attribute is not implemented or if type is 
	 * supplied and not the type we expect then fail
	 */
	if ((a->xi == NOT_IMPLEM) || (type && (a->xi != type)))
		a = NULL;

	return (a);
}

struct eval *
findeval (head, name)
struct	ehead	*head;
char		*name;
{
struct	eval   *eval;

	/* find the 'name' structure */
	for (eval = head->eval; eval; eval = eval->next)
	{
		if (strcmp (name, eval->name) == 0)
			return (eval);
	}
	return (NULL);	/* not found */
}

struct eval *
addeval (head, new)
struct	ehead	*head;
struct	eval   	*new;
{
struct  eval	*eval;

	/* first member of the list must be treated special case */
	if (!(head->eval))
		return (head->eval = new);	

	/* find the 'name' structure or last one */
	for (eval = head->eval; /* FOREVER */ ; eval = eval->next)
	{
		/* add to list */
		if (!(eval->next))
			break;
	}
	return (eval->next = new);
}

/*
 * NAME: setuserdb
 *                                                                    
 * FUNCTION: start the session
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
setuserdb(mode)
int	mode;	/* read/write ... */
{
	sessions++;
	if (!userhead)
	{
		if ((userhead = setattr(mode)) == NULL)
			return(-1);
	}
	else
		userhead->mode |= mode;
		
	if (!grouphead)
	{
		if ((grouphead = setattr(mode)) == NULL)
			return(-1);
	}
	else
		grouphead->mode |= mode;

	if (!sysckhead)
	{
		if ((sysckhead = setattr(mode)) == NULL)
			return(-1);
	}
	else
		sysckhead->mode |= mode;

	if (!porthead)
	{
		if ((porthead = setattr(mode)) == NULL)
			return(-1);
	}
	else
		porthead->mode |= mode;

	return (0);
}

/*
 * NAME: setpwddb
 *                                                                    
 * FUNCTION: start the session
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
setpwdb(mode)
int	mode;	/* read/write ... */
{
	psessions++;

	if (!pwdhead)
	{
		if ((pwdhead = setattr(mode)) == NULL)
			return(-1);
	}
	else
		pwdhead->mode |= mode;

	return (0);
}

/*
 * NAME: setattr
 *                                                                    
 * FUNCTION: setup the header structure 
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *                                                                   
 * RETURNS: if successful address of the header structure or if failure NULL
 *
 */  
struct	ehead *
setattr (mode)
int		mode;		/* read/write ... */
{
static struct	ehead	*head = NULL;

	/* initialize */
	if ((head = (struct ehead *) calloc (1, sizeof (struct ehead))) != NULL)
	{
		/* initialize header structure */
		head->mode = mode;
		head->eval = NULL;
	}
	return (head);
}

/*
 * NAME: endattr
 *                                                                    
 * FUNCTION: end the attribute get and put session
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *
 *	Free name (eval) structures.
 *	CLose all open files.
 *	Free header structure.
 *                                                                   
 * RETURNS: if successful address of the header structure or if failure NULL
 *
 */  
int
endattr (head, nattr, ftab, nftab)
struct	ehead	*head;
int		nattr;
struct	atfile	*ftab;
int		nftab;
{
struct	eval	*ua1;
struct  eval   *ua2;

	/* free name structures */
	for (ua1 = head->eval; ua1;)
	{
		ua2 = ua1->next;

		/* free all the attribute values */
		freeval (head, ua1, nattr);

		ua1 = ua2;
	}
	/* close all open files */
	{
	struct	atfile	*f;
	struct	atfile	*fend;

		for (f = ftab, fend = ftab + nftab; f < fend; f++)
		{
			if (f->fhdle)
			{
				(*(f->close))(f->fhdle);
				f->fhdle = NULL;
			}
		}
	}
	/* free session structure */
	free ((char *)head);
	return (0);
}

/*
 * NAME: getuattr
 *                                                                    
 * FUNCTION: retrieve the name's attribute value as specified
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *                                                                   
 *	Sanity check arguments.
 *	Find attribute in attribute table.
 *	Find name-value structure (eval) or create a new one.
 *	If requested attribute is already in CACHE then return it.
 *	Else read in the requested attributes (along with the other attributes 
 *	that are in the same file and then return it.
 *    	
 * RETURNS: 0 if successful otherwise:
 *
 *	EINVAL - name is not specified correctly or attribute not found
 *	EPERM  - caller doesn't have permissions to open the needed file.
 *	(failed system call errors will be bubbled up and returned)
 */  
int
getuattr (name, atnam, val, type, table)
char 		*name;
char		*atnam;
void		*val;
int		type;
int		table;
{
static struct ehead	*head;
register int		ai;
struct	attr		*a;
struct	eval		*eval;
int			nattr;
int			nftab;
struct	attr		*atab;
struct	atfile		*ftab;
unsigned short		*flgs;	/* indicates status of attribute value */

	/* check arguments */
	if (!name || !*name)
	{
		errno = EINVAL;
		return (-1);
	}

	if ((head = getableinfo(table,&nattr,&nftab,&atab,&ftab)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	if ((a = findattr (atnam, type, atab, nattr)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	ai = a->ai;

	if ((eval = findeval (head, name)) == NULL)
	{
		if ((eval = neweval (name, nattr)) == NULL)
			return (-1);

		(void) addeval (head, eval);

		/* set the flags for any qwerks */
		eval->atvals[ai].flgs |= a->qwerks;

		if(readattr(head,atab,nattr,ftab,(atab + ai)->fi,eval))
		{
		struct	atfile *f = ftab + (atab + ai)->fi;

			if (f->fhdle)
			{
				(*(f->close))(f->fhdle);
				f->fhdle = NULL;
			}
			*(void **) val = NULL;
 			return (-1);
		}
	}
	/* check the flags to see if this attribute was already searched for */
	flgs = &(eval->atvals[ai].flgs);

	/* set the flags for any qwerks */
	eval->atvals[ai].flgs |= a->qwerks;

	if (*flgs & NOT_FOUND)
	{
		eval->atvals[ai].val = NULL;
		errno = ENOATTR;
	}
	if (*flgs & NOT_VALID)
	{
		eval->atvals[ai].val = NULL;
		errno = EINVAL;
		return -1;
	}

	/*
	 * if requested value is not already in memory then read it in.
	 * if the attribute is a wildcard make sure that the one
	 * we are looking for is the one in the table.
	 */
	if(!(*flgs & CACHED) || (((ai + 1) == nattr) && (a->qwerks & REUSED)))
	{
		/* if this is a wild card attribute clear previous value */
		if (a->qwerks & REUSED)
		{
			/* clear the re-used flag */
			a->qwerks &= ~REUSED;

			/*
		 	 * if the previous value had been
		 	 * dynamically allocated, free it.
		 	 */
			if (*flgs & DYNAMIC)
			{
				free ((void *)eval->atvals[ai].val);
				eval->atvals[ai].val = (void *)NULL;
			}

			/* clear the attribute flags */
			eval->atvals[ai].flgs = 0;
		}
			
		/* read in the new value */
		if (readattr(head,atab,nattr,ftab,(atab + ai)->fi,eval))
		{
			*(void **) val = NULL;
 			return (-1);
		}
	}

	*(void **) val = (void *) eval->atvals[ai].val;

	if (*flgs & NOT_FOUND)
	{
		eval->atvals[ai].val = (char *)NULL;
		errno = ENOATTR;
		return (-1);
	}

	if (*flgs & NOT_VALID)
	{
		eval->atvals[ai].val = (char *)NULL;
		errno = EINVAL;
		return (-1);
	}

 	return (0);
}

/*
 * NAME: putuattr
 *                                                                    
 * FUNCTION: change the name's attribute value as specified
 *                                                                    
 * EXECUTION ENVIRONMENT: library 
 *                                                                   
 *	First, sanity check arguments. The 'head' variable must be set which 
 *	should be done in the above layer (i.e. putuserattr).
 *	Second, if the type is SEC_COMMIT then the new attributes are written
 *      to the files either for the specified 'name' or for all names in cache.
 *	Finally, if the type is not SEC_COMMIT the new attribute value specified
 *	by the caller is just put into cache.
 *
 * RETURNS: 0 if successful otherwise:
 *
 *		EINVAL 	if the arguments are not valid
 *		EPERM	if mode does not have S_WRITE bit on
 *		EFBIG	something went wrong with parsing files
 *
 *		(failed system call errors will be bubbled up)
 */  
int
putuattr (name, atnam, val, type, table)
register char	*name;
register char	*atnam;
register char	*val;
register int	type;
register int	table;
{
static struct	ehead	*head;
struct	eval	*eval;
struct	attr	*a;
register int	ai;
int		nattr;
int		nftab;
struct	attr	*atab;
struct	atfile	*ftab;
register int	i;
register struct	attr	*b;
struct	atval	*attrval = NULL;

	if ((head = getableinfo(table,&nattr,&nftab,&atab,&ftab)) == NULL)
	{
		errno = ENOENT;
		return (-1);
	}
	if (!(head->mode & S_WRITE))
	{
		errno = EPERM;
		return (-1);
	}

	/* commit all the names with new attribute values */
	if (type == SEC_COMMIT)
	{
		/* commit just specified name's new attribute values */
		if (commit (eval, atab, ftab, nattr, nftab, head, name))
			return (-1);
		return (0);
	}

	/* find the attribute table entry for specified attribute */
	if ((a = findattr (atnam, type, atab, nattr)) == NULL)
	{
		errno = EINVAL;
		return (-1);
	}

	ai = a->ai;

	/*
	 * if adding a user go through all the files
	 * in the file table and put the current user
	 * information in cache
	 */

	if (type == SEC_NEW)
	{
		/*
		 * see if this entity has an entry
		 * in the linked list of entries.
		 */
 
		if ((eval = findeval (head, name)) == NULL)
		{
			/*
			 * allocate a new entity value 
			 * and add it to the linked list.
			 */

			if ((eval = neweval (name, nattr)) == NULL)
				return (-1);

			(void) addeval (head, eval);

		}

		/*
		 * for each file in the file table go through 
		 * the attribute table looking for the first attribute
		 * that is using this file and mark it UPDATED.
		 */

		for (i = 0; i < nftab; i++)
		{
			for (b = atab; b < atab+nattr; b++)
			{
				if (i == b->fi)
				{
					/* mark attribute as being updated */
					attrval = &(eval->atvals[b->ai]);

					/*
					 * if attribute already has 
					 * a value free it.
					 */

					if (attrval->flgs & DYNAMIC)
					{
						free (attrval->val);
						attrval->flgs &= ~DYNAMIC;
					}

					/* mark value as NULL and NUKED */
					attrval->val = NULL;
					attrval->flgs = 0;
					attrval->flgs |= UPDATED;
					attrval->flgs |= NUKED;
					attrval->flgs |= CACHED;
					break;
				}
			}
		}
		return (0);

	}

	/* find specified name attribute value structure */
	if ((eval = findeval (head, name)) == NULL)
	{
		if ((eval = neweval (name, nattr)) == NULL)
			return (-1);

		if(readattr(head,atab,nattr,ftab,(atab + ai)->fi,eval))
		{
			/*
		 	 * if there is no attribute we need to add it
		 	 */
			if ((errno != ENOATTR) && (errno != ENOENT))
			{
			struct	atfile *f = ftab + (atab + ai)->fi;
				freeval (head, eval, nattr);
				if (f->fhdle)
				{
					(*(f->close))(f->fhdle);
					f->fhdle = NULL;
				}
				return (-1);
			}
		}
		(void) addeval (head, eval);
	}

	/*
	 * Check val for validity.
	 */
	{
	int	(*chk)();

		if (chk = atab[a->ai].chk) {
			if ((*chk)(val, name, atab, a->ai)) {
				if (errno == 0)
					errno = EINVAL;
				return (-1);
			}
		}
	}

	/* 
	 * here we put the new value into cache
	 */
	{
	char	*(*cpy)();
	struct	atval	*aval = &(eval->atvals[a->ai]);

		/* if previous value is in malloc'd space free it */
		if (aval->flgs & DYNAMIC)
		{
			if (aval->val != val)
				free (aval->val);
			aval->val = NULL;
			aval->flgs &= ~DYNAMIC;
		}

		/* mark attribute as existing */
		aval->flgs &= ~NOT_FOUND;

		/* mark attribute as being updated */
		aval->flgs |= UPDATED;

		/* take off the NUKED (possibly set above) */
		aval->flgs &= ~NUKED;

		/* if flag is SEC_DELETE mark the attribute NUKED */
		if (type == SEC_DELETE)
		{
			aval->val = NULL;
			aval->flgs |= NUKED;
			aval->flgs |= CACHED;
			return (0);
		}

		/* copy in new value */
		if (cpy = xtab[a->xi].cpy)
		{
			aval->val = (*(cpy))(val, &(aval->flgs));
		}
		else
		{
			aval->val = val;
			aval->flgs |= CACHED;
		}
	}
	return (0);
}


/*
 * NAME: rmufile()
 *                                                                    
 * FUNCTION: removes a user or group entry from the data base.
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
rmufile(register char *name,register int flag,register int table)
{
int		nattr;
int		nftab;
struct	attr	*atab;
struct	atfile	*ftab;

	/* a NULL return is expected */
	getableinfo(table,&nattr,&nftab,&atab,&ftab);

	if (deletefile (name,flag,nftab,atab,ftab,nattr,table))
	{
		return (-1);
	}

	return (0);
}

/*
 * NAME: enduserdb
 *                                                                    
 * FUNCTION: end the session
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
enduserdb()
{
	if (sessions && --sessions)
		return (0);

	if (userhead)
	{
		endattr (userhead, usernattr, userftab, usernftab);
		userhead = NULL;
	}

	if (grouphead)
	{
		endattr (grouphead, groupnattr, groupftab, groupnftab);
		grouphead = NULL;
	}
	if (sysckhead)
	{
		endattr (sysckhead, syscknattr, sysckftab, syscknftab);
		sysckhead = NULL;
	}
	if (porthead)
	{
		endattr (porthead, portnattr, portftab, portnftab);
		porthead = NULL;
	}
	return (0);
}

/*
 * NAME: endpwdb
 *                                                                    
 * FUNCTION: end the session
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
endpwdb()
{
	if (psessions && --psessions)
		return (0);

	if (pwdhead)
	{
		endattr (pwdhead, pwdnattr, pwdftab, pwdnftab);
		pwdhead = NULL;
	}
	return (0);
}

/*
 * NAME: getableinfo()
 *                                                                    
 * FUNCTION: gets the pointers to the attribute table, the
 *		file table, the number of attributes, the
 *		number of files in the data base, and the
 *		pointer to the static link list of cached 
 *		information.
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

struct ehead *
getableinfo(table,nattr,nftab,atab,ftab)
int	table;
int	*nattr;
int	*nftab;
struct	attr	**atab;
struct	atfile	**ftab;
{

	switch(table)
	{
		case USER_TABLE :	*atab = &useratab[0];
					*ftab = &userftab[0];
					*nattr = usernattr;
					*nftab = usernftab;
					return(userhead);

		case GROUP_TABLE :	*atab = &groupatab[0];
					*ftab = &groupftab[0];
					*nattr = groupnattr;
					*nftab = groupnftab;
					return(grouphead);

		case PASSWD_TABLE :	*atab = &pwdatab[0];
					*ftab = &pwdftab[0];
					*nattr = pwdnattr;
					*nftab = pwdnftab;
					return(pwdhead);

		case SYSCK_TABLE :	*atab = &sysckatab[0];
					*ftab = &sysckftab[0];
					*nattr = syscknattr;
					*nftab = syscknftab;
					return(sysckhead);

		case PORT_TABLE :	*atab = portatab;
					*ftab = portftab;
					*nattr = portnattr;
					*nftab = portnftab;
					return(porthead);

		case GENERIC_TABLE :	*atab = genatab;
					*ftab = genftab;
					*nattr = gennattr;
					*nftab = gennftab;
					return(generichead);

		default		:	return(NULL);

	}
}

/*
 * NAME: chksessions()
 *                                                                    
 * FUNCTION: returns the sessions count
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
chksessions()
{
	return (sessions);
}

/*
 * NAME: chkpsessions()
 *                                                                    
 * FUNCTION: returns the psessions count
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
chkpsessions()
{
	return (psessions);
}


static int
chkgrp (group, user, atab, idx)
char	*group;		/* attribute value */
char	*user;		/* user name */
struct	attr atab[];	/* attribute table */
int	idx;		/* index into attribute table */
{
	/* checks the group name to make sure it is valid
	 * returns 0 if valid, -1 if the group name is invalid
	 */
	gid_t id;

	/* check obvious errors first */
	if (group == (char *)NULL || *group == '\0')
		return (-1);

	if (grouptoID(group, &id) != 1) {
#ifdef ALLOW_GROUP_IDS_FOR_GROUP_NAME
		/* group is not a valid group name so make sure that group is
		 * a string containing only numbers with no sign and that it
		 * is a valid group id.
		 */
		char *ptr;

		id = strtol(group, &ptr, 10);
		if (*ptr != '\0' || *group == '-'
		|| IDtogroup(id) == (char *)NULL)
			return (-1);
#else !ALLOW_GROUP_IDS_FOR_GROUP_NAME
		return (-1);
#endif ALLOW_GROUP_IDS_FOR_GROUP_NAME
	}
	return (0);
}

/*
 * NAME: getgenericattr
 *
 * FUNCTION: get generic attributes
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This is a library routine. It returns the requested attribute values
 *	in malloc'd memory. A call to enduserdb() will free all the memory.
 *
 * RETURNS:
 *	zero if success, non-zero on error.
 */
int
getgenericattr (char *file,char *stanza,char *atnam,char **val,int type)
{
	int ret;

	if ((generichead = setattr(S_READ)) == NULL)
		return(-1);

	genftab[0].org = file;
	genatab[1].ai = 1;
	genatab[1].fi = 0;
	genatab[1].atnam = atnam;
	genatab[1].xi = type;
	genatab[1].field = 0;
	genatab[1].qwerks = 0;
	genatab[1].chk = 0;
	
	if(getuattr(stanza,atnam,val,type,GENERIC_TABLE))
		ret = -1;
	else
	{
		/*
		 * We copy the value since the endattr() below will free the
		 * memory allocated for the value.
		 */

		ret = 0;
		if(type == SEC_CHAR)
		{
			if(!(*val = strdup(*val)))
				ret = -1;
		}
		else if(type == SEC_LIST)
		{
			char *tmp;
			int i;

			for(i = 0; (*val)[i] || (*val)[i + 1]; i++);

			tmp = (char *)malloc(i + 2);
			if (tmp)
			{
				memset(tmp, 0, i + 2);
				memcpy(tmp, *val, i);
			}
			else
				ret = -1;
			*val = tmp;
		}
	}

	endattr (generichead, gennattr, genftab, gennftab);
	generichead = NULL;

	return(ret);
}

/*
 * NAME: putgenericattr
 *
 * FUNCTION: put the specified port attribute
 *
 * EXECUTION ENVIRONMENT: library
 *
 * RETURNS:
 *	zero for success, non-zero on error.
 */
int
putgenericattr (char *file,char *stanza,char *atnam,void *val,int type)
{
	char *ptr;
	int ret;

	if(!(genftab[0].old = (char *)malloc(strlen(file) + 2)))
		return(-1);
	genftab[0].org = file;
	if(strchr(file, '/'))
	{
		strcpy(genftab[0].old, file);
		ptr = strrchr(genftab[0].old, '/') + 1;
		*ptr = '\0';
		strcat(genftab[0].old, "o");
		strcat(genftab[0].old, strrchr(file, '/') + 1);
	}
	else
	{
		strcpy(genftab[0].old, "o");
		strcat(genftab[0].old, file);
	}
	genatab[1].ai = 1;
	genatab[1].fi = 0;
	genatab[1].atnam = atnam;
	genatab[1].xi = ((type != SEC_NEW) && (type != SEC_DELETE)) ? type : 0;
	genatab[1].field = 0;
	genatab[1].qwerks = 0;
	genatab[1].chk = 0;
	
	if ((generichead = setattr(S_READ | S_WRITE)) == NULL)
		return(-1);

	if(putuattr(stanza,atnam,val,type,GENERIC_TABLE) ||
	   putuattr(stanza,NULL,NULL,SEC_COMMIT,GENERIC_TABLE))
		ret = -1;
	else
		ret = 0;

	endattr (generichead, gennattr, genftab, gennftab);
	generichead = NULL;

	free(genftab[0].old);

	return(ret);
}

