static char sccsid[] = "@(#)03	1.11.1.9  src/bos/usr/bin/newgrp/user_util.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:35:53";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addtoadms
 *		addusers
 *		changeattr
 *		chkeyword
 *		chkname
 *		chknameexist
 *		delmems
 *		getusername
 *		getvals
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <userpw.h>
#include <sys/wait.h>
#include "tcbauth.h"


static	int	chknameexist(int,char *);

/*
 * FUNCTION:	addusers
 *
 * DESCRIPTION:	builds a new member list string from an old member
 *		list and a new string.  The string is checked for 
 *		repeated values.
 *
 * RETURNS:	None.
 *
 * PASSED:
 *	char	*header;	 the "value=" header
 *	char	*users;		 the users to be added
 *	char	*members;	 the current member list
 *	char	**usrset;	 the final list
 */

void
addusers(char *header,char *users,char *members,char **usrset)
{
register int	i = 0;	/* counter */
int		u = 0;	/* counter */
int		n = 0;	/* counter */
register int	c = 0;	/* counter */
char		**usrs;	/* new users array */
char		**mems;	/* current members array */
char		*ptr;	/* temporary pointer	*/
register int	siz;	/* size of space to malloc */

	/* get some space to hold the new string */

	if (users && *users)
	{
		siz = strlen(header) + strlen(users) + 2;
		if ((*usrset = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}

		strcpy(*usrset,header);
		strcat(*usrset,users);
		strcat(*usrset,COMMA);
	}
	else
	{
		siz = strlen(header) + 1;
		if((*usrset = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
		strcpy(*usrset,header);
	}

	/* turn a the new user comma separated string to an array */
	usrs = comtoarray(users,&u);

	/* turn a the current member comma separated string to an array */
	mems = comtoarray(members,&n);

	while(mems[i])
	{
		/*
		 * check if user member is already there
		 */
		c = 0;
		while (usrs[c])
		{
			if(!strcmp(mems[i],usrs[c]))
			{
				mems[i] = NULL;
				break;
			}
			c++;
		}
		if (mems[i] != NULL)
		{
			siz = strlen(*usrset) + strlen(mems[i]) + 2;
			if ((*usrset=(char *)realloc(*usrset, siz)) == NULL)
			{
				fprintf (stderr,MALLOC);
				exitx (errno);
			}
			strcat(*usrset, mems[i]);
			strcat(*usrset,COMMA);
		}
		i++;
	}
	/* if last character is a comma remove it */
	ptr = *usrset + strlen(*usrset) - 1;
	if (*ptr == ',')
		*ptr = '\0';
}

/*
 * FUNCTION:	chkname
 *
 * DESCRIPTION:	checks to see if the name contains invalid characters
 *		(non-alphanumeric) and begins w/ an alpha character.
 *              NLS characters allowed, but no flattening of names is done  
 *
 * PASSED:	which:   0 for user
 *			 1 for group
 *
 *		name:	 user or group name
 *
 * RETURNS:	errno or 0.
 *
 */

int
chkname(int which,char *name)
{
 
char	*namep;						/* ptr to the name */
char	illegal1stch[] = {'-', '+', '@', '~', 0};	/* bad 1st char	   */

	/* Check for NULL */
	if (!name)
	{
		fprintf(stderr,ERRADD,name);
		errno = EINVAL;
		return (-1);
	}

	namep = name;	

	/* The user or group name must not contain any of the
	 * following characters to ensure our databases remain
	 * correct:
	 *	name cannot contain a colon (:)
	 *			double quote(")
	 *			      space ( )
	 *			      pound (#)
	 *			      comma (,)
	 *			   asterisk (*)
	 *			single quote(')
	 *			     equals (=)
	 *			    newline (\n)
	 *			        tab (\t)
	 *			 back slash (\)
	 *			      slash (/)
	 *			   question (?)
	 *			  back quote(`)
	 */
	while (*namep)
	{
		if(*namep == ':'  || *namep == '"'  || *namep == ' '  ||
		   *namep == '#'  || *namep == ','  || *namep == '*'  ||
		   *namep == '\'' || *namep == '='  || *namep == '\n' ||
		   *namep == '\t' || *namep == '\\' || *namep == '/'  ||
		   *namep == '?'  || *namep == '`')
		{
			fprintf(stderr,ERRADD,name);
			errno = EINVAL;
			return (-1);
		}
		namep++;
	}

	/* check for usage */
	if (strchr (illegal1stch, *name))
	{
		if (which)
			usage(MKUSRUSAGE);
		else
			usage(MKGRPUSAGE);
	}

	/* Check for username's length < PW_NAMELEN */
	if(strlen(name) > PW_NAMELEN - 1)
	{
		fprintf(stderr,ERRADD,name);
		errno = ENAMETOOLONG;
		return (-1);
	}

	/*
	 *Check for "ALL" and "default" keywords
	 */
	if (chkeyword(name))
	{
		fprintf(stderr,ERRADD,name);
		errno = EINVAL;
		return (-1);
	}


	if (chknameexist(which,name))
	{
		if (which)
			fprintf(stderr,UEXIST,name);
		else
			fprintf(stderr,GEXIST,name);

		errno = EXISTS;
		return (-1);
	}
	return(0);
}


/*
 * NAME: chknameexist ()
 *                                                                    
 * FUNCTION: checks if name exists in /etc/passwd or /etc/group 
 *                                                                    
 * RETURNS: none.
 *
 */  

static int
chknameexist(int which,char *name)
{
struct	passwd	*pw;		/* return from getpwnam() */
struct	group	*gr;		/* return from getgrnam() */

	if (which)
	{
		setpwent ();
		set_getpwent_remote(2);	/* compat lookups only	*/
		pw = getpwnam(name);	
		set_getpwent_remote(1);	/* resume full lookups	*/
		if (pw != (struct passwd *)NULL)
		{
			endpwent();
			return(-1);
		}
		endpwent();
		return (0);
	}
	else
	{
		setgrent ();
		set_getgrent_remote(2);	/* compat lookups only	*/
		gr = getgrnam(name);
		set_getgrent_remote(1);	/* resume full lookups	*/
		if (gr != (struct group *)NULL)
		{
			endgrent();
			return (-1);
		}
		endgrent();
		return (0);
	}
}

/*
 * FUNCTION:	getvals
 *
 * DESCRIPTION:	parses input parameters ("attr=value") for attributes.
 *
 * RETURNS:	0 on success or -1.
 *
 * PASSED:
 *	char	*param;		 parameter
 *	char	**attr;		 place to put attribute name
 *	char	**val;		 place to put attribute value
 */

int
getvals(char *param,char **attr,char **val)
{
char	*valptr;		/* temporary pointer	*/


	/*
	 *  Set valptr to point to the equals sign.
	 *  If there is no equals sign, return -1.
	 *  Then set the equals to a NULL and increment.
	 */
	if (!(valptr = strchr(param,'=')))
		return(-1);
	*valptr++ = '\0';

	/*
	 *  Drop all spaces and tabs in attribute name.
	 */
	param = dropblanks(param);
	*attr = param;

	/*
	 *  Now, lets drop the leading spaces and tabs
	 *  in the value.  We can't drop all spaces and
	 *  tabs since some values contain those characters.
	 */

	while (*valptr && (*valptr == ' ' || *valptr == '\t'))
		valptr++;

	*val = valptr;

	return(0);
}

/*
 * FUNCTION:	chkeyword
 *
 * DESCRIPTION:	checks if the name is one of the keywords: 'ALL', 'default'
 *
 * PASSED:	the name to be checked.
 *
 * RETURNS:	true or false
 *
 */

int
chkeyword(char *name)
{
	if(!strcmp(name,ALL) || !strcmp(name,DEFAULT))
		return(1);

	return(0);
}

/*
 * FUNCTION:	changeattr
 *
 * DESCRIPTION:	Execs /usr/bin/chuser to make the attribute changes.
 *
 * PASSED:	username = the username, header = the "attr=" string,
 *		answer =  the value.
 *
 * RETURNS:	Return code from /usr/bin/chuser.
 *
 */

int 
changeattr(char *username,char *header,char *answer)
{
char		*attr;	/* pointer to new shell string	*/
register int	siz;
pid_t		pid;
int		rc = EINVAL;

	/* build the attr=answer string */
	siz = strlen(header) + strlen(answer) + 2;

	if((attr = (char *)malloc(siz)) == NULL)
	{
		fprintf (stderr,MALLOC);
		exitx (errno);
	}

	strcpy(attr,header);
	strcat(attr,answer);

	/* run /usr/bin/chuser to make the changes */
	if ((pid = fork()) == 0)
	{
		execl(CHUSER,"chuser",attr,username,0);
		fprintf(stderr,EXECL,CHUSER);
		fprintf (stderr,CHECK,CHUSER);
		exit (EACCES);
	}
	else if (pid == -1)
	{
		fprintf(stderr,EXECL,CHUSER);
		fprintf (stderr,CHECK,CHUSER);
		exit (errno);
	}
	else
	{
		/* get the return code */
		while (waitpid(pid, &rc, 0) == -1)
		{
			if (errno != EINTR)
			{
				rc = errno;
				break;
			}
		}
	}
	return (WEXITSTATUS(rc));
}

/*
 * FUNCTION:	getusername
 *
 * DESCRIPTION:	Checks for name passed in as an argument.  If no name
 *		was passed in, get the username of the invoker.
 *
 * RETURNS:	A pointer to name.
 *
 */

char	*
getusername(int argc,char *name)
{
int	id;
uid_t	uid;
char	*user;

	if (argc < 2)
	{
		uid = getuid();
		if ((user = IDtouser(uid)) == NULL)
			return (NULL);
		else
		{
			user = strdup(user);
			return(user);
		}
	}
	else
	{
		/* Check for username's length < PW_NAMELEN */
		if(strlen(name) > PW_NAMELEN - 1)
		{
			fprintf(stderr,CHGONERR,name);
			fprintf (stderr, TOOLONG);
			exit (ENAMETOOLONG);
		}

		if (getuserattr(name,S_ID,&id,SEC_INT))
			return(NULL);
		else
			return(name);
	}
}

/*
 * FUNCTION:	delmems
 *
 * DESCRIPTION:	deletes the groups(deletes) from the current 
 *		member list(members) and returns the result(memset).
 *
 * RETURNS:	0 on success or -1.
 *
 * PASSED:
 *	char	*header;	 the "value=" header
 *	char	*members;	 the current member list
 *	char	*deletes	 the users to be deleted
 *	char	**memset;	 the final list	
 */

int
delmems(char *header,char *members,char *deletes,char **memset)
{
register int	i = 0;		/* counter 				*/
register int	p = 0;		/* counter 				*/
int		n = 0;		/* counter 				*/
int		m = 0;		/* counter 				*/
register int	match = 0; 	/* flag 				*/
char		*finalptr;	/* final pointer to hold the string 	*/
char		**dels;		/* the array of users to delete		*/
char		**mems;		/* the array of group members 		*/
register int	siz;

	/*
	 * final groupset will not be longer than the current groupset
	 */
	siz = strlen(header) + strlen(members) + 2;
	if((*memset = (char *)malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	strcpy(*memset,header);

	/*
	 * set pointer to memset and
	 * step it through to the end
	 */
	finalptr = *memset + strlen(*memset);

	/* get groups to be deleted */
	dels = comtoarray(deletes,&n);

	/* get members of groupset */
	mems = comtoarray(members,&m);

	/*
	 * Lets go through our list of deletes
	 * making sure each user in the delete
	 * list is a member of this group.  If 
	 * user is invalid, return with error.
	 *
	 * If we have a valid user, then mark
	 * this user with a NULL for building
	 * our list later.
	 *
	 * m:  the number of members of current group list
	 * n:  the number of members to be deleted
	 * p:  count for each group member
	 * i:  count for each user to be deleted
	 */
	for (i=0;i<n;i++)
	{
		match = 0;
		for (p=0;p<m;p++)
		{
			/* valid member? */
			if (!strcmp(dels[i],mems[p]))
			{
				/* 
				 * Yes, mark member with NULL
				 * and go on to next member
				 */
				mems[p] = NULL;
				match = 1;
				break;
			}
		}
		/* No, we have an invalid member */
		if (!match)
		{
			if (strlen(dels[i]) >= siz )
			{	
				siz += (strlen(dels[i]) + 1);
				if((*memset = (char *)realloc(*memset,
						siz)) == NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
			}
			strcpy(*memset,dels[i]);
			return (-1);
		}
	}

	/*
	 * We now have a list of valid members for our
	 * group list.  The members marked for delete
	 * are now NULL, so go through the list and
	 * make sure the NULL members are taken out
	 * and build our final list of members for 
	 * this group.
	 */
	for (p=0;p<m;p++)
	{
		/* has the member been deleted? */
		if (mems[p] != NULL)
		{
			strcpy(finalptr,mems[p]);

			/*
			 * check the name length, increment finalptr
			 * just past the name and add a ","
			 */

			finalptr += strlen(mems[p]);
			*(finalptr++) = ',';
		}
	}
	/*
	 * If this was the last member to be added, back up
	 * finalptr and null terminate the list, return successful
	 */
	finalptr--;
	if (*finalptr == ',')
		*finalptr = '\0';

	return(0);
}

/*
 *
 * FUNCTION:	addtoadms
 *
 * DESCRIPTION:	adds a user to the specified group's adms member list
 *
 * RETURNS:	0 or -1, exits on error in addusers().
 *
 */

int
addtoadms(char *user,char *grouplist)
{
char	*members;			/* the current members of the group */
char	*usrset;			/* the new set of members 	    */
char	*set;				/* list to pass to putgroupattr()   */
char	*groups;			/* string of newgroups passed in    */
char	**newgroups;			/* array of newgroups passed in     */
char	**curgroups = (char **)NULL;	/* current array of groups	    */
int	i, j, newgroupct, curgroupct;	/* count of new and current lists   */
char	*curlist;			/* current list of "admgroups	    */
char	*newmembers;			/* comma-separated list		    */

	/*
	 * First, get the current list of "admgroups" and delete
	 * the "adms" entry for each group in this list.
	 * We will add the "adms" entries later.
	 */

	if(getuserattr(user, S_ADMGROUPS, &curlist, SEC_LIST))
		curgroupct = 0;
	else
		curgroups = listoarray(curlist, &curgroupct);

	newgroups = listoarray(grouplist,&newgroupct);

	/*
	 * delete the "adms" entry for each user 
	 * in the current group list.
	 */
	for (i = 0; i < curgroupct; i++)
	{

		/* 
		 * get current group member list
		 */
		if (getgroupattr(curgroups[i], S_ADMS, &members, SEC_LIST))
			if (errno != ENOATTR && errno != ENOENT)
				return(-1);

		if (members && *members)
		{
			/*
			 * delete this user
			 */
			listocom(members);
			delmems((char *)NULL, members, user, &newmembers);
		}

		/*
		 * turn string into a list for putgroupattr
		 */
		strtolist(newmembers, &set);

		/*
		 * Update the group cache's "adms" entry with the
		 * new list of users added.  Don't COMMIT yet since
		 * the calling routines COMMIT at the end of all
		 * additions and deletions.
		 */
		if(putgroupattr(curgroups[i],S_ADMS,set,SEC_LIST))
		{
			fprintf(stderr,USRTOGRPER,user,curgroups[i]);
			return(-1);
		}

	}

        /*
	 * add the "admgroups" attribute to each
	 * new user listed in the "adms=" string.
	 */
	for (i = 0; i < newgroupct; i++)
	{
		/* 
		 * Get current group member list.  If there are no
		 * current members, that's OK, so go on.
		 */
		if (getgroupattr(newgroups[i],S_ADMS,&members,SEC_LIST))
			if (errno != ENOATTR && errno != ENOENT)
				return(-1);

		if (members && *members)
		{
			/*
			 * if there are current members turn the list into a
			 * string.   Add the new list to the current list.
	 		 * No need to check return since addusers() exits
	 		 * on error.
			 */
			listocom(members);
			addusers(NULL,user,members,&usrset);
		}
		else
			usrset = user;
		
		/*
		 * turn string into a list for putgroupattr
		 */
		strtolist(usrset,&set);

		/*
		 * Update the group cache's "adms" entry with the
		 * new list of users added.  Don't COMMIT yet since
		 * the calling routines COMMIT at the end of all
		 * additions and deletions.
		 */
		if(putgroupattr(newgroups[i],S_ADMS,set,SEC_LIST))
		{
			fprintf(stderr,USRTOGRPER,user,newgroups[i]);
			return(-1);
		}
	}

	return(0);
}
