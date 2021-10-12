static char sccsid[] = "@(#)66	1.18.1.2  src/bos/usr/bin/setgroups/setgroups.c, cmdsuser, bos411, 9428A410j 12/9/93 19:28:34";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addgrps
 *		addtougroups
 *		backtologin
 *		delgrps
 *		drophead
 *		getpgroups
 *		getsets
 *		getugroups
 *		isvalidgroup
 *		main
 *		printgrps
 *		resetgroups
 *		resetgrps
 *		resetreal
 *		setpgroups
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>	/* for printf()		*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<limits.h>	/* for NGROUPS_MAX 	*/
#include	<sys/priv.h>	/* for privilege	*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<userpw.h>	/* for PW_NAMELEN	*/
#include	<stdlib.h>
#include	"tcbauth.h"	/* for local flags 	*/

/* local defines */
static	void	addgrps(int flag,char *groups);
static	void	delgrps(int flag,char *groups);
static	void	resetgrps(int flag,char *newgrpset);
static	void	resetreal(int flag,char *newgrp);
static	void	printgrps(int flag);
static	void	getugroups(char **groups);
static	void	setpgroups(int flag,char *groups);
static	char	**getpgroups(void);
static	void	getsets(char **ugroups,char **pgroups);
static	void	resetgroups(char *newgrpset,char *ugroups,char **finalgrpset);
static	void	drophead(char *cgroups,char **groups);
static	void	addtougroups(int,char *,char *,char *,char **);
static	void	backtologin();
static	int	isvalidgroup(char *);

/*
 *
 * NAME:     setgroups
 *                                                                    
 * FUNCTION: resets the groupset of the session
 *                                                                    
 * USAGE:    setgroups [-] [-a | -d | -r] [groupset]
 *	     where:
 *		-		specifies that the environment is to be 
 *				reinitialized
 *		groupset	specifies one or more groups to which the user
 *				belongs
 *		-a		specifies that the groupset is to be added to
 *				the concurrent group set
 *		-d		specifies that the groupset is to be deleted
 *				from the concurrent groupset.
 *		-r		specifies that the real group ID is to be 
 *				changed.
 * RETURNS: does not return; if an error, one of the following will result:
 *		
 *		EINVAL 	if an invalid flag is specified
 *			if -r is specified with >1 group
 *			if the # of grps in the groupset > NGROUPS_MAX
 *		ENOENT 	if the group(s) does not exist
 *		EACCES 	if the invoker doesn't have read_access to the user
 *			database.
 *		EPERM 	if the invoker doesn't belong to one of the groups
 *				which are being added to the groupset.
 *		errno	if system error.
 */  

main (int argc, char *argv[])
{
register	int	c;
register	int	flag = 0;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* suspend auditing */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* suspend privilege */
	privilege(PRIV_LAPSE);

	/* check for execution at a terminal */
	if (checkfortty())
	{
		fprintf(stderr,TERMINAL,SETGROUPS);
		exitex(SETGRAUD,ENOTTY,NULL,NULL,NOPRINT);
	}

	/*
	 * Check for '-' flag.  Remember, we could
	 * have a groupname of 1 character.
	 */
	if (argv[1] && (!strcmp(argv[1],"-"))) 
	{
		/*
	 	 * we want to reset the groupset to
	 	 * its original login state 
	 	 */
		flag |= INIT;
		if (argv[2] == NULL)
			backtologin();
		argv++;
	}
	else if (argv[1])
		flag |= DELTA;
		
	/* get the other flag */
	while ((c = getopt(argc,argv,"adr")) != EOF)
	{
		switch (c)
		{
		case 'a':	addgrps(flag,argv[optind]);

		case 'd':	delgrps(flag,argv[optind]);

		case 'r': 	if (isvalidgroup(argv[optind]))
					resetreal(flag,argv[optind]);
				else
					xusage(SETGRUSAGE,SETGRAUD,NULL);

		default :	xusage(SETGRUSAGE,SETGRAUD,NULL);
		}
	}
	/*
	 * These routines never return so
	 * no need to check return codes.
	 */
	if(argv[1]) 
		resetgrps(flag,argv[1]);
	else
		printgrps(flag);
}

/*
 *
 * FUNCTION:	isvalidgroup
 *
 * DESCRIPTION:	checks for valid group name
 *
 * RETURNS:	True or False.
 *
 */
int
isvalidgroup(char *name)
{
	return( (strlen(name)<PW_NAMELEN) && (!strchr(name,',')) );
}

/*
 *
 * FUNCTION:	addgrps
 *
 * DESCRIPTION:	adds the new groupset to the current process groupset 
 *
 * RETURNS:	No return.
 *
 * PASSED:
 *	int  flag 	   INIT or DELTA
 *	char *newgroups	   new groupset for process
 *
 */

static	void
addgrps(int flag,char *newgroups)
{
char	*ugroups;	/* current user groupset    */
char	*pgroups;	/* current process groupset */
char	*finalgrpset;	/* final groupset 	    */
char	err[50];

	if (!newgroups)
		xusage(SETGRUSAGE,SETGRAUD,NULL);

	/* get user and process groupset */
	getsets(&ugroups,&pgroups);

	/* check for valid groups and build final groupset string */
	if (flag & INIT)
	{
		if (listlen(newgroups)+listlen(ugroups)<=NGROUPS_MAX)
			addtougroups(flag,ugroups,(char *)NULL,newgroups,
							&finalgrpset);
		else
		{
			sprintf(err,"%d",NGROUPS_MAX);
			fprintf(stderr,GRPTOLG,err);
			exitex(SETGRAUD,0,NULL,NULL,NOPRINT);
		}
	}
	else
	{
		if (listlen(newgroups)+listlen(pgroups)<=NGROUPS_MAX)
			addtougroups(flag,pgroups,ugroups,newgroups,
						&finalgrpset);
		else
		{
			sprintf(err,"%d",NGROUPS_MAX);
			fprintf(stderr,GRPTOLG,err);
			exitex(SETGRAUD,0,NULL,NULL,NOPRINT);
		}
	}

	/* run new process with new groupset */
	setpgroups(flag,finalgrpset); 

}

/*
 * FUNCTION:	delgrps
 *
 * DESCRIPTION:	gets current groupset and deletes the newgrpset from it.
 *		the groups are checked for membership in the current 
 *		groupset.
 *
 * PASSED:	flag = INIT or DELTA, groups = groups to delete.
 *
 * RETURNS:	No return.
 *
 */

static	void
delgrps(int flag,char *groups)
{
char	*ugroups;	/* current user groupset    */
char	*pgroups;	/* current process groupset */
char	*finalgrpset;	/* final groupset 	    */

	if (!groups)
		xusage(SETGRUSAGE,SETGRAUD,NULL);

	/* get user and process groupset */
	getsets(&ugroups,&pgroups);

	/* 
	 *  if INIT, take the group out of the user's groupset
	 *  otherwise, take the user out of the process groupset
	 *  We are not trying to delete our primary group
	 *  so go ahead and call delmems.
	 */
	if (flag & INIT)
	{
		if (delmems(HEADER,ugroups,groups,&finalgrpset))
		{
			fprintf(stderr,GRPNINST,finalgrpset);
			exitex(SETGRAUD,ENOENT,finalgrpset,ugroups,NOPRINT);
		}
	}
	else
	{
		if (delmems(HEADER,pgroups,groups,&finalgrpset))
		{
			fprintf(stderr,GRPNINST,finalgrpset);
			exitex(SETGRAUD,ENOENT,finalgrpset,pgroups,NOPRINT);
		}
	}


	/*
	 *  If this is the last group, reset back all groups
	 *  and print error message.
	 */
	if (!listlen(finalgrpset + strlen(HEADER)))
	{
		strcat(finalgrpset,pgroups);
		fprintf(stderr,LASTGRP);
	}

	/* run new process with new groupset */
	setpgroups(flag,finalgrpset); 
}

/*
 *
 * FUNCTION:	resetreal
 *
 * DESCRIPTION:	changes process' real group to new groupset.
 *
 * PASSED:	flag = INIT or DELTA, newgrp = new real group for process
 *
 * RETURNS:	No return.
 *
 */

static	void
resetreal(int flag,char *newgrp)
{
char	*ugroups;	/* current user groupset    */
char	*pgroups;	/* current process groupset */
char	*finalgrpset;	/* final groupset 	    */

	/*
	 * A NULL newgrp here will be handled by
	 * the getreal() routine and does not
	 * need checking here.
	 */

	/* get user and process groupset */
	getsets(&ugroups,&pgroups);

	/* check for valid groups and build final groupset string */
	if (flag & INIT)
		getreal(flag,ugroups,ugroups,newgrp,&finalgrpset);
	else
		getreal(flag,pgroups,ugroups,newgrp,&finalgrpset);

	/* run new process with new groupset */
	setpgroups(flag,finalgrpset); 
}

/*
 *
 * FUNCTION:	resetgrps
 *
 * DESCRIPTION:	resets the current groupset.
 *
 * PASSED:	flag = INIT or RESET, newgrpset = new process groupset
 *
 * RETURNS:	No return.
 *
 */

static	void
resetgrps(int flag,char *newgrpset)
{
char	*ugroups;	/* current user groupset */
char	*finalgrpset;	/* final groupset */

	/* get current user groupset */
	getugroups(&ugroups);

	/* check for valid groups and build final groupset string */
	resetgroups(newgrpset,ugroups,&finalgrpset);

	/* run new process with new groupset */
	setpgroups(flag,finalgrpset);
}

/*
 *
 * FUNCTION:	printgrps
 *
 * DESCRIPTION:	lists the current user and process groupset.
 *
 * RETURNS:	No return.
 *
 */

static	void
printgrps(int flag)
{
char	*pgroups;
char	*ugroups;
char	*name;		/* user name */
uid_t	uid;
	
	/* get real users name */
	uid = getuid();
	if ((name = (char *)IDtouser(uid)) == NULL)
	{
		fprintf(stderr,USRINFO);
		fprintf (stderr, CHECK, PASSWD);
		exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
	}

	/* get user and process groupset */
	getsets(&ugroups,&pgroups);

	printf ("%s:\n",name);
	printf(CURUSE,ugroups);
	printf(CURPROC,pgroups);
	printf("\n");
	exitex(SETGRAUD,0,NULL,NULL,NOPRINT);
}

/*
 *
 * FUNCTION:	getsets
 *
 * DESCRIPTION:	returns the user's and the process' groupsets
 *
 * PASSED:	ugroups = place to put user groupset, pgroups = place to put
 *		process groupset.
 *
 * RETURNS:	user groupset and process groupset
 *
 */

static	void
getsets(char **ugroups,char **pgroups)
{
char	**currgrpset;

	/* get current user's groupset */
	getugroups(ugroups);

	/* get current process' groupset */
	currgrpset = getpgroups();

	/* drop the GROUPS= */
	drophead(*currgrpset,pgroups);
}

/*
 * FUNCTION:	getugroups
 *
 * DESCRIPTION:	get the user's current groupset.
 *
 * PASSED:	groups = place to put the groupset.
 *
 * RETURNS:	groupset in groups and exits on error.
 */

static	void
getugroups(char **groups)
{
char	*val;		/* return value from getuserattr() */
char	*name;		/* user name */
uid_t	uid;
	
	/* get real users name */
	uid = getuid();
	if ((name = (char *)IDtouser(uid)) == NULL)
	{
		fprintf(stderr,USRINFO);
		fprintf (stderr, CHECK, PASSWD);
		exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
	}

	/* get his groupset from /etc/group */
	if(getuserattr(name,S_GROUPS,(void *)&val,SEC_LIST))
	{
		fprintf(stderr,GETUSRGRPS,name);
		fprintf (stderr, CHECK , GROUP);
		exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
	}

	/* turn NULL separated string to comma separated */
	*groups = listocom(val);

}

/*
 *
 * FUNCTION:	getpgroups
 *
 * DESCRIPTION:	get current process' groupset.
 *
 * RETURNS:	pointer to current group set.
 *
 */

static	char	**
getpgroups(void)
{
char	**groups;	/* place to store groupset */

	/* reacquire privilege that was dropped earlier */
	privilege(PRIV_ACQUIRE);

	/* get process' groupset */
	if ((groups = getpcred(CRED_GROUPS)) == NULL) 
	{
		fprintf(stderr,GETPCRED);
		exitex(SETGRAUD,errno,groups,NULL,NOPRINT);
	}

	/* lapse privilege again */
	privilege(PRIV_LAPSE);

	return(groups);
}

/*
 *
 * FUNCTION:	setpgroups
 *
 * DESCRIPTION:	uses setpcred to set the current groupset.
 *
 * PASSED:	flag = INIT or DELTA, groups = new process' groupset.
 *
 * RETURNS:	No return.
 *
 */

static	void
setpgroups(int flag,char *groups)
{
char	*creds[3];		/* new groupset 		      */
char	group[PW_NAMELEN];	/* real group name 		      */
char	*ptr;			/* to increment through groups string */
register int siz;		/* space to get 		      */
register int siz1;		/* space to get 		      */

	/* set up REAL_GROUP string */
	drophead(groups,&ptr);
	getrealgroup(ptr,group);

	siz = strlen(REAL_GROUP) + strlen(group) + 1;
	if ((ptr = malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETGRAUD,errno,group,groups,NOPRINT);
	}
	
	strcpy(ptr,REAL_GROUP);
	strcat(ptr,group);

	/* set up new group array */
	creds[0] = ptr;
	creds[1] = groups;
	creds[2] = (char *)NULL;

	/* reacquire lapsed privilege */
	privilege(PRIV_ACQUIRE);

	/* audit credential change */
	siz = strlen(group) + 1;
	siz1 = strlen(groups) + 1;
	auditwrite(SETGRAUD,AUDIT_OK,group,siz,groups,siz1,0);

	/* set process credentials with new groupset */
	if(setpcred((char *)NULL,creds))
	{
		fprintf(stderr,SETPCRED);
		exitex(SETGRAUD,errno,group,groups,PRINT);
	}

	/* run new shell */
	exitex(SETGRAUD,0,NULL,NULL,NOPRINT);
}

/*
 *
 * FUNCTION:	addtougroups
 *
 * DESCRIPTION:	checks whether the newgroups are in user's current groupset,
 *		 whether any are already in the process' current groupset.
 *
 * RETURNS:	None.
 *
 * PASSED:
 *	char *ugroups		current user or process groupset
 *	char *agroups		current available groupset
 *	char *newgrpset		newgroups to add
 *	char **finalgrpset	place to put final groupset.
 *
 */

static	void
addtougroups(int flag,char *ugroups,char *agroups,
		char *newgrpset,char **finalgrpset)
{
register int	i = 0;
register int	found = 0;
int	c = 0;
int	pflag = 0;
int	id;
int	ag = 0;
int	ug,n;
char	**ugrps;
char	**agrps = (char **)NULL;
char	**news;
char	group[PW_NAMELEN];
register int	siz;

	/* get real group for auditing */
	getrealgroup(ugroups,group);

	/*
	 * Get some space to hold the new string.  Since
	 * we had comma-separated lists (ugroups and newgrpset),
	 * the commas are already counted, so we need space
	 * for only one more comma to separate the strings
	 * and one NULL.
	 */
	siz = strlen(HEADER) + strlen(ugroups) + strlen(newgrpset) + 2;
	if((*finalgrpset = malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETGRAUD,errno,group,newgrpset,NOPRINT);
	}

	/* turn strings into arrays */
	ugrps = comtoarray(ugroups,&ug);
	news = comtoarray(newgrpset,&n);
	if (agroups)
		agrps = comtoarray(agroups,&ag);

	/* copy in GROUPS= , real group, and a comma */
	strcpy(*finalgrpset,HEADER);
	strcat(*finalgrpset,ugrps[0]);
	strcat(*finalgrpset,COMMA);

	while(news[i])
	{
		/* Check for groups's length < PW_NAMELEN */
		if(strlen(news[i]) > PW_NAMELEN - 1)
		{
			fprintf(stderr,LONGGROUP,news[i]);
			exitex(SETGRAUD,ENAMETOOLONG,news[i],NULL,NOPRINT);
		}

		/*
		 * if newgroup is real skip
		 * it because we've already copied it in
		 */
		if(!strcmp(news[i],ugrps[0]))
			break;

		/*
		 * see if the groups are allowed to be added
		 * to the user's process groupset by
		 * checking the user's available groups
		 */
		if (flag & DELTA)
		{
			for (c = 0; c < ag; c++)
			{
				if(!strcmp(news[i],agrps[c]))
				{
					found = 1;
				 	/*
				 	 * copy in the new group and
					 * add a comma to end
					 */
					strcat(*finalgrpset, news[i]);
					strcat(*finalgrpset,COMMA);
					break;
				}
			}
			/* if not in user's groupset --> error */
			if(!found)
			{
				fprintf(stderr,GRPNINST,news[i]);
				if (getgroupattr(news[i],S_ID,(void *)&id,
								SEC_INT))
					exitex(SETGRAUD,ENOENT,group,newgrpset,
								NOPRINT);
				else
					exitex(SETGRAUD,EPERM,group,newgrpset,
								NOPRINT);
			}
			found = 0;
		}


		/*
		 * Go through the user's groupset to see
		 * if the new group is in it.
		 * If it is remove it from the groups
		 * to be copied in at the end and
		 * copy it in to the final groupset.
		 * If it isn't print error.
		 */
		for (c = 1; c < ug; c++)
		{
			if(!strcmp(news[i],ugrps[c]))
			{
				found = 1;
				/*
				 * copy in the new group and
				 * delete it from the groups that
				 * remain in the user group string
				 */
				if (flag & INIT)
				{
					strcat(*finalgrpset, news[i]);
					strcat(*finalgrpset,COMMA);
				}
				ugrps[c] = NULL;
				break;
			}
		}

		/* if not in user's groupset --> error */
		if (flag & INIT)
		{
			if(!found)
			{
			   if (getgroupattr(news[i],S_ID,(void *)&id,SEC_INT))
			   {
				fprintf(stderr,GRPNINST,news[i]);
				exitex(SETGRAUD,ENOENT,group,newgrpset,
								NOPRINT);
			   }
			   fprintf(stderr,GRPNINST,news[i]);
			   exitex(SETGRAUD,EPERM,group,newgrpset,NOPRINT);
			}
		}
		found = 0;
		i++;
	}

	/*
	 * Copy in the groups that are left.
	 */
	for (c = 1; c < ug; c++)
	{
		if(ugrps[c])
		{
			strcat(*finalgrpset, ugrps[c]);
			strcat(*finalgrpset,COMMA);
		}
	}

	/*
	* This was the last member to be added, so back up
	* finalptr and null terminate the list.
	*/

	*finalgrpset--;
	**finalgrpset = '\0';
}

/*
 *
 * FUNCTION:	resetgroups
 *
 * DESCRIPTION:	builds the new groupset.
 *
 * PASSED:	newgrpset = new group set, ugroups = current user groupset, 
 *		finalgrpset = final group set.
 *
 * RETURNS:	None.
 *
 */

static	void
resetgroups(char *newgrpset,char *ugroups,char **finalgrpset)
{
int	i=0, ug = 0, g = 0, c = 0, match = 0,id;
char		**grps;
char		**ugrps;
char		group[PW_NAMELEN];
register int	siz;

	/* separate out real group for auditing */
	getrealgroup(newgrpset,group);

	siz = strlen(HEADER) + strlen(newgrpset) + 1;
	if((*finalgrpset = malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETGRAUD,errno,group,newgrpset,NOPRINT);
	}

	/* copy in GROUPS= and new groupset */
	strcpy(*finalgrpset,HEADER);
	strcat(*finalgrpset,newgrpset);
	
	/* turn group strings into arrays */
	ugrps = comtoarray(ugroups,&ug);
	grps = comtoarray(newgrpset,&g);

	while(grps[i])
	{
		/* Check for groups's length < PW_NAMELEN */
		if(strlen(grps[i]) > PW_NAMELEN - 1)
		{
			fprintf(stderr,LONGGROUP,grps[i]);
		   	exitex(SETGRAUD,ENAMETOOLONG,grps[i],NULL,NOPRINT);
		}

		/* is newgroup member in the current user groupset */
		for (c = 0; c < ug; c++)
		{
			if(strcmp(ugrps[c],grps[i]) == 0)
			{
				match=1;
				break;
			}
		}
		if (!match)
		{
			fprintf(stderr,GRPNINST,grps[i]);
			if (getgroupattr(grps[i],S_ID,(void *)&id,SEC_INT))
				exitex(SETGRAUD,ENOENT,group,newgrpset,
								NOPRINT);
			else
				exitex(SETGRAUD,EPERM,group,newgrpset,NOPRINT);
		}
		match = 0;
		i++;
	}
}

/*
 *
 * FUNCTION:	drophead
 *
 * DESCRIPTION:	drops the "GROUPS="
 *
 * RETURNS:	None.
 *
 */

static	void
drophead(char *cgroups,char **groups)
{
char	*value;
char	*ptr;

	if ((value = strchr(cgroups,'=')) == NULL)
	{
		fprintf(stderr,GETPCRED);
		exitex(SETGRAUD,errno,cgroups,NULL,NOPRINT);
	}
	
	/* truncate at first space */
	ptr = ++value;

	if (ptr && *ptr)
		if (ptr = strchr(ptr,' '))
			*ptr = '\0';	

	*groups = value;
}

/*
 *
 * FUNCTION:	backtologin
 *
 * DESCRIPTION:	resets the groupset back to its state at login
 *
 * RETURNS:	No return.
 *
 */

static	void
backtologin()
{
char	**logname;
uid_t	uid;
char	*name;
char	u_name[PW_NAMELEN];

	/* reacquire the privilege dropped earlier */
	privilege(PRIV_ACQUIRE);

	uid = getuid();
	if ((name = (char*)IDtouser(uid)) == NULL)
	{
		fprintf(stderr,USRINFO);
		fprintf (stderr, CHECK, PASSWD);
		exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
	}

	/*
	 * copy into a safe place because 
	 * the name pointer might be overwritten
	 */
	if (strlen (name) > PW_NAMELEN - 1)
	{
		fprintf(stderr,LONGNAME,name);
		exitex(SETGRAUD,ENAMETOOLONG,name,NULL,NOPRINT);
	}
	strcpy (u_name,name);
	
	/* get the login user name from cred */
	if ((logname = getpcred(CRED_LUID)) == NULL)
	{
		fprintf(stderr,GETPCRED);
		exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
	}

	/* set process credentials: uid,gid,groupset etc. */
	if (setpcred (u_name, logname))
	{
		fprintf(stderr,SETPCRED);
		exitex(SETGRAUD,errno,NULL,NULL,PRINT);
	}

	privilege(PRIV_DROP);

	exitex(SETGRAUD,0,NULL,NULL,NOPRINT);
}
