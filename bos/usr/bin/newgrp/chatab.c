/* @(#)86	1.6  src/bos/usr/bin/newgrp/chatab.c, cmdsuser, bos41J, 9512A_all 3/14/95 15:54:38 */
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addtoadmgroups
 *		addtogroup
 *		addtopgrp
 *		addvalue
 *		putvalue
 *		sizeof
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

#include "tcbauth.h"

struct	chusattr	chusatab [ ] =
{
	/*
	 * attributes of /etc/passwd
	 */
	{ S_ID,			NULL,	SEC_INT,    (int(*)(char *, char **))chkuid},
	{ S_PGRP,		NULL,	SEC_CHAR,	chkpgrp},
	{ S_GROUPS,		NULL,	SEC_LIST,	chkgrps},
	{ S_HOME,		NULL,	SEC_CHAR,	chkhome},
	{ S_SHELL,		NULL,	SEC_CHAR,	chkprog},
	{ S_GECOS,		NULL,	SEC_CHAR,	chkgek},

	/*
	 * attributes of /etc/security/audit/audit.cfg
	 */
	{ S_AUDITCLASSES,	NULL,	SEC_LIST,	chkaudit},

	/*
	 * attributes of /etc/security/user
	 */
	{ S_LOGINCHK,		NULL,	SEC_BOOL,	chkbool},
	{ S_SUCHK,		NULL,	SEC_BOOL,	chkbool},
	{ S_RLOGINCHK,		NULL,	SEC_BOOL,	chkbool},
	{ S_TELNETCHK,		NULL,	SEC_BOOL,	chkbool},
	{ S_DAEMONCHK,		NULL,	SEC_BOOL,	chkbool},
	{ S_ADMIN,		NULL,	SEC_BOOL,	chkbool},
	{ S_SUGROUPS,		NULL,	SEC_LIST,	chkgrps},
	{ S_ADMGROUPS,		NULL,	SEC_LIST,	chkadmgroups},
	{ S_TPATH,		NULL,	SEC_CHAR,	chktpath},
	{ S_TTYS,		NULL,	SEC_LIST,	chkttys},
	{ S_EXPIRATION,		NULL,	SEC_CHAR,	chkexpires},
	{ S_AUTH1,		NULL,	SEC_LIST,	strtolist},
	{ S_AUTH2,		NULL,	SEC_LIST,	strtolist},
	{ S_UMASK,		NULL,	SEC_INT,    (int(*)(char *, char **))chkumask},
	{ S_AUTHSYSTEM,		NULL,	SEC_CHAR,	chkauthsystem},
	{ S_REGISTRY,		NULL,	SEC_CHAR,	chkregistry},
	{ S_LOGTIMES,		NULL,	SEC_LIST,	_usertodb},
	{ S_LOGRETRIES,		NULL,	SEC_INT,	chkint},
	{ S_PWDWARNTIME,	NULL,	SEC_INT,	chkint},
	{ S_LOCKED,		NULL,	SEC_BOOL,	chkbool},
	{ S_MINAGE,		NULL,	SEC_INT,	chkminage},
	{ S_MAXAGE,		NULL,	SEC_INT,	chkmaxage},
	{ S_MAXEXPIRED,		NULL,	SEC_INT,	chkmaxexpired},
	{ S_MINALPHA,		NULL,	SEC_INT,	chkminalpha},
	{ S_MINOTHER,		NULL,	SEC_INT,	chkminother},
	{ S_MINDIFF,		NULL,	SEC_INT,	chkmindiff},
	{ S_MAXREPEAT,		NULL,	SEC_INT,	chkmaxrepeats},
	{ S_MINLEN,		NULL,	SEC_INT,	chkminlen},
	{ S_HISTEXPIRE,		NULL,	SEC_INT,	chkhistexpire},
	{ S_HISTSIZE,		NULL,	SEC_INT,	chkhistsize},
	{ S_PWDCHECKS,		NULL,	SEC_LIST,	chkpwdchecks},
	{ S_DICTION,		NULL,	SEC_LIST,	chkdictionlist},
	{ S_USREXPORT,		NULL,	SEC_BOOL,	chkbool},

	/*
	 * attributes of /etc/security/limits
	 */
	{ S_UFSIZE,		NULL,	SEC_INT,    (int(*)(char *, char **))chkulimit},
	{ S_UCPU,		NULL,	SEC_INT,    (int(*)(char *, char **))chkint},
	{ S_UDATA,		NULL,	SEC_INT,    (int(*)(char *, char **))chkdata},
	{ S_USTACK,		NULL,	SEC_INT,    (int(*)(char *, char **))chkstack},
	{ S_UCORE,		NULL,	SEC_INT,    (int(*)(char *, char **))chkint},
	{ S_URSS,		NULL,	SEC_INT,    (int(*)(char *, char **))chkint},
	/*
	 * attributes of /etc/security/environ
	 */
	{ S_USRENV,		NULL,	SEC_LIST,	strtolist},
	{ S_SYSENV,		NULL,	SEC_LIST,	strtolist}

};

struct	chgrattr	chgratab [ ] =
{
	{ S_ID,			NULL,	SEC_INT,	(int(*)(char *, char **))chkgid},

	{ S_ADMIN,		NULL,	SEC_BOOL,	chkbool},

	{ S_USERS,		NULL,	SEC_LIST,	chknames},

	{ S_ADMS,		NULL,	SEC_LIST,	chknames},

	{ S_GRPEXPORT,		NULL,	SEC_BOOL,	chkbool},
};

int chusatabsiz = sizeof(chusatab)/sizeof(struct chusattr);
int chgratabsiz = sizeof(chgratab)/sizeof(struct chgrattr);

/*
 * FUNCTION:	putvalue
 *
 * DESCRIPTION:	matches the parameter with the attribute and adds it to
 *		the user database.
 *
 * PASSED:	user = username, attr = attribute name, val = attribute value,
 *		arecord = audit record in case of error.
 *
 * RETURNS:	0 or error.
 *
 */

int
putvalue(char *user,char *attr,char *val,char *arecord,char *event)
{
struct chusattr	*ptr;		/* ptr to table				*/
void		*ret;		/* return value from check routines	*/
int		err = 1;	/* flag to indicate valid attribute	*/
int		result = 0;	/* return from check routines		*/

	/* go through table looking for attribute name */
	for (ptr = chusatab;ptr < &chusatab[chusatabsiz]; ptr++)
	{
		if (!strcmp(attr, ptr->gattr))
		{
			err = 0;

			/* 
			 * Empty strings are allowed for all
			 * attributes except S_ID and S_PGRP.
			 */
			if (val && (*val == '\0'))
			{
				if (!(strcmp(ptr->gattr,S_ID)) || 
					       !(strcmp(ptr->gattr,S_PGRP)))
				{
				     fprintf(stderr,CHGTOERR,attr,val);
				     exitax(event,BADVALUE,user,arecord,PRINT);
				}

				if (putuserattr (user,ptr->gattr,
						(void *)NULL,SEC_DELETE))
					return(-1);
				else
					return (0);
			}

			/*
			 * call check routine: this will return a
			 * pointer to a void to handle ints and chars
			 */
			if (ptr->check)
			{
				if (result = (*(ptr->check))(val,&ret))
				{
					if ((result != EXISTS) && 
							(result != NOEXISTS))
					{
				   		fprintf(stderr,CHGTOERR,
								attr,val);
					}
					exitax(event,result,user,
							arecord,PRINT);
				}
				
			}
			else
				ret = (void *)val;

			/*
			 * If the attribute is 'S_PGRP', update
			 * both "pgrp" and "groups=" in the user cache.
			 */
			if(!strcmp(ptr->gattr,S_PGRP))
				if (addtopgrp(user,ret))
				{
					fprintf(stderr,GETUSRGRPS,user);
					fprintf(stderr,CHECK,GROUP);
					exitax(CHGRPAUD,errno,user,NULL,
								NOPRINT);
				}

			/*
			 * If the attribute is 'S_GROUPS',
			 * update the group cache to contain
			 * these groups as well as the user's
			 * primary group, in case the user
			 * provided a list not containing the
			 * primary group.
			 */
			if(!strcmp(ptr->gattr,S_GROUPS))
				if (addtogrouplist(user,ret))
				{
					fprintf(stderr,GETUSRGRPS,user);
					fprintf(stderr,CHECK,GROUP);
					exitax(CHGRPAUD,errno,user,NULL,
								NOPRINT);
				}

			/*
			 * If the attribute is 'S_ADMGROUPS',
			 * update /etc/security/group file's
			 * "adms" attribute for each group in
			 * the admgroups list.
			 */
			if(!strcmp(ptr->gattr,S_ADMGROUPS))
				if (addtoadms(user,ret))
				{
					fprintf(stderr,GETUSRGRPS,user);
					fprintf(stderr,CHECK,SGROUP);
					exitax(CHGRPAUD,errno,user,NULL,
								NOPRINT);
				}

			/* change the user database here */
			if(putuserattr(user,ptr->gattr,(void *)ret,(int)NULL))
				return(-1);

			break;
		}
	}

	if (err)
	{
		fprintf(stderr,CHGTOERR,attr,val);
		exitax(event,BADATTR,user,arecord,PRINT);
	}

	return(0);
}

/*
 * FUNCTION:	addvalue
 *
 * DESCRIPTION:	Matches the parameter with the attribute and adds it to
 *		the user database.
 *
 * PASSED:	name: the group name, attr: the attribute name, val: the 
 *		attribute value, arecord: ptr to audit record in case of error.
 *
 * RETURNS:	0 on success or non-zero on error.
 *
 */

int
addvalue(char *name,char *attr,char *val,char *arecord)
{
struct chgrattr	*ptr;		/* ptr to table				*/
void		*ret;		/* return value from check routines	*/
int		err = 1;	/* flag to indicate valid attribute	*/
int		result = 0;

	/* go through table looking for attribute name */
	for (ptr = chgratab;ptr < &chgratab[chgratabsiz]; ptr++)
	{
		if (!strcmp(attr, ptr->gattr))
		{
			err = 0;
			/*
			 * Empty strings are allowed for all
			 * attributes except S_ID
			 */
			if (val && (*val == '\0'))
			{
				if (!(strcmp(ptr->gattr,S_ID)))
				{
				     fprintf(stderr,CHGTOERR,attr,val);
				     exitax(CHGRPAUD,BADVALUE,name,arecord,
								  PRINT);
				}
				if (putgroupattr (name,ptr->gattr,
					(void *)NULL,SEC_DELETE))
					return(-1);
				else
					return (0);
			}

			/*
			 * call check routine: this will return a 
			 * pointer to a void to handle ints and chars
			 */
			if (ptr->check)
			{
				if (result = (*(ptr->check))(val,&ret))
				{
					if (result != NOEXISTS)
					{		
				   		fprintf(stderr,CHGTOERR,
							attr,val);
					}
				   	exitax(CHGRPAUD,result,
							name,arecord,PRINT);
				}
			}
			else
				ret = (void *)val;

			/*
			 * If the attribute is 'S_ADMS',
			 * update /etc/security/user file's
			 * "admgroups" attribute for each user in
			 * the adms list.
			 */
			if(!strcmp(ptr->gattr,S_ADMS))
				if (addtoadmgroups(name,ret))
				{
					fprintf(stderr,GETAT,S_ADMGROUPS,name);
					fprintf(stderr,".\n");
					fprintf(stderr,CHECK,USERFILE);
					exitax(CHGRPAUD,errno,name,NULL,
								NOPRINT);
				}

			/*
			 * add the attribute here
			 */
			if(putgroupattr(name,ptr->gattr,(void *)ret,(int)NULL))
				return(-1);

			break;
		}
	}

	if (err)
	{
		fprintf(stderr,CHGTOERR,attr,val);
		exitax(CHGRPAUD,BADATTR,name,arecord,PRINT);
	}

	return(0);
}

/*
 *
 * FUNCTION:	addtogroup
 *
 * DESCRIPTION:	adds a user to the specified group's member list
 *
 * RETURNS:	0 or -1, exits on error in addusers().
 *
 */

int
addtogroup(char *user,char *group)
{
char	*members;	/* the current members of the group 	      */
char	*grpset;	/* the current groups for this user 	      */
char	*newgrpset;	/* the new set of groups 	    	      */
char	*usrset;	/* the new set of members		      */
char	*finaluserset;  /* final set of users for putgroupattr() call */
char	*finalgrpset;   /* final set of groups for putuserattr() call */

	/*
	 *  We are going to update the /etc/group file, but the
	 *  cache information for groups exists in both the user
	 *  and the group cache.  If the S_USERS attribute of
	 *  the group cache is updated, then we must also update
	 *  the S_GROUPS attribute of the user cache.  This will
	 *  ensure that the information is consistent in both
	 *  the /etc/group file and the /etc/passwd file.
	 */ 

	/* 
	 *  get current group membership lists for
	 *  both S_USERS and S_GROUPS.
	 */
	if (getgroupattr(group,S_USERS,&members,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(-1);

	if (getuserattr(user,S_GROUPS,&grpset,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(-1);

	/*
	 * If the lists aren't empty, turn them into 
	 * strings and add the new list to the current
	 * list.  No need to check return codes from
	 * addusers since addusers() exists on error.
	 */
	if (members && *members)
	{
		listocom(members);
		addusers(NULL,user,members,&usrset);
	}
	else
		usrset = user;
	
	if (grpset && *grpset)
	{
		listocom(grpset);
		addusers(NULL,group,grpset,&newgrpset);
	}
	else
		newgrpset = group;

	/*
	 * turn strings into lists for putgroupattr/putuserattr
	 */
	strtolist(usrset,&finaluserset);
	strtolist(newgrpset,&finalgrpset);

	/*
	 * Change the cache information at this time.  The
	 * cached information won't be written out until
	 * the SEC_COMMIT gets passed in to putuserattr() and
	 * putgroupattr().
	 */
	if(putgroupattr(group,S_USERS,finaluserset,SEC_LIST))
	{
		fprintf(stderr,USRTOGRPER,user,group);
		return(-1);
	}

	if(putuserattr(user,S_GROUPS,finalgrpset,SEC_LIST))
	{
		fprintf(stderr,USRTOGRPER,user,group);
		return(-1);
	}

	return(0);
}

/*
 *
 * FUNCTION:	addtoadmgroups
 *
 * DESCRIPTION:	adds a group to the specified users' admgroups member list
 *
 * RETURNS:	0 or -1, exits on error in addusers().
 *
 */

int
addtoadmgroups(char *group,char *userlist)
{
char	*members;			/* the current members of the group */
char	*groupset;			/* the new set of members 	    */
char	*set;				/* list to pass to putuserattr()    */
char	*users;				/* the string of users passed in    */
char	**newusers;			/* array of newusers passed in      */
char	**curusers = (char **)NULL;	/* current array of users 	    */
int	i, j, newuserct, curuserct;	/* count of new and current lists   */
char	*curlist;			/* current list of users in adms=   */
char	*newmembers;			/* comma-separated list 	    */


	/*
	 * First, get the current list of "adms" and delete
	 * the "admgroups" entry for each user in this list.
	 * We will add the "admgroups" entries later.
	 */

	if(getgroupattr(group, S_ADMS, &curlist, SEC_LIST))
		curuserct = 0;
	else
		curusers = listoarray(curlist, &curuserct);

	newusers = listoarray(userlist,&newuserct);

	/*
	 * delete the "admgroups" entry for each group 
	 * in the current user list.
	 */
	for (i = 0; i < curuserct; i++)
	{
		/* 
		 * get current member list
		 */
		if (getuserattr(curusers[i], S_ADMGROUPS, &members, SEC_LIST))
			if (errno != ENOATTR && errno != ENOENT)
				return(-1);

		if (members && *members)
		{
			/*
			 * delete this group from the list.
			 */
			listocom(members);
			delmems((char *)NULL, members, group, &newmembers);
		}

		/*
		 * turn string into a list for putuserattr
		 */
		strtolist(newmembers, &set);

		/*
		 * Update the user cache's "admgroups" entry with the
		 * new list of groups added.  Don't COMMIT yet since
		 * we do the COMMIT's at the end of all additions and
		 * deletions.
		 */
		if(putuserattr(curusers[i],S_ADMGROUPS,set,SEC_LIST))
		{
			fprintf(stderr,ERRADM,group,curusers[i]);
			fprintf(stderr,".\n");
			return(-1);
		}

	}

	/*
	 * add the "admgroups" attribute to each
	 * new user listed in the "adms=" string.
	 */
	for (i = 0; i < newuserct; i++)
	{
		/* 
		 * Get current user member list.  If there are no
		 * current members, that's OK, so go on.
		 */
		if (getuserattr(newusers[i],S_ADMGROUPS,&members,SEC_LIST))
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
			addusers(NULL,group,members,&groupset);
		}
		else
			groupset = group;
		
		/*
		 * turn string into a list for putgroupattr
		 */
		strtolist(groupset,&set);

		/*
		 * Update the user cache's "admgroups" entry with the
		 * new list of groups added.  Don't COMMIT yet since
		 * we do the COMMIT's at the end of all additions and
		 * deletions.
		 */
		if(putuserattr(newusers[i],S_ADMGROUPS,set,SEC_LIST))
		{
			fprintf(stderr,ERRADM,group,newusers[i]);
			fprintf(stderr,".\n");
			return(-1);
		}
	}

	return(0);
}

/*
 *
 * FUNCTION:	addtogrouplist
 *
 * DESCRIPTION:	adds a user to the specified group's "groups=" member list
 *
 * RETURNS:	0 or -1, exits on error in addusers().
 *
 */

int
addtogrouplist(char *user,char *grouplist)
{
char	**newgroups;	/* array of newgroups passed in     */
int	i, newgroupct;	/* count of new and current lists   */
char	*curlist;	/* current list of "groups"	    */


	/*
	 * For each group in the group list, add
	 * the user to the current members of the
	 * group.  Convert new grouplist to an array.
	 */

	newgroups = listoarray(grouplist, &newgroupct);

	for (i = 0; i < newgroupct; i++)
	{
		if (addtogroup(user,newgroups[i]))
		{
			fprintf(stderr,GETUSRGRPS,user);
			fprintf(stderr,CHECK,GROUP);
			exitax(CHGRPAUD,errno,user,NULL,NOPRINT);
		}
	}
	return(0);
}

/*
 *
 * FUNCTION:	addtopgrp
 *
 * DESCRIPTION:	adds a user to the new primary group
 *
 * RETURNS:	0 or -1, exits on error in addusers().
 *
 */

int
addtopgrp(char *user,char *newpgrp)
{
char	*oldpgrp;	       /* the current pgrp for this user 	     */
char	*oldpgrpmembers;       /* the current members of the oldpgrp 	     */
char	*newpgrpmembers;       /* the current members of the newpgrp 	     */
char	*grpset;	       /* the current groups for this user 	     */
char	*oldgrpset;	       /* the S_GROUPS before the new pgrp is added  */
char	*newgrpset;	       /* the new set of S_GROUPS for this user      */
char	*newusrset;	       /* the new set of S_USERS of new pgrp	     */
char	*oldusrset;	       /* the new set of S_USERS of old pgrp	     */
char	*finalgrpset;          /* final set of S_GROUPS for putuserattr()    */
char	*finaloldusrset;       /* final set of S_USERS for old pgrp          */
char	*finalnewusrset;       /* final set of S_USERS for new pgrp          */

	/*
	 *  We are going to update the /etc/group file and the
	 *  /etc/passwd file with the S_PGRP attribute.  We first
	 *  check the current "pgrp".  Then we take the user out
	 *  of the S_USERS list of that group.  Now we take the
	 *  old pgrp out of the S_GROUPS entry in the user cache.
	 *  Then the user's S_PGRP attribute is updated in the
	 *  user cache and this new "pgrp" is added to his list
	 *  of "groups=", also in the user cache.  Now we add him
	 *  to the S_USERS list of the new "pgrp".  This will sync
	 *  up all the cached data and assure data consistency.
	 *
	 * 
	 *  Get the following information:
	 *	1) current "pgrp" for new user
	 *	2) current members for old "pgrp"
	 *	3) current members of the new "pgrp"
	 *	4) current "groups=" entry for new user
	 */
	if (getuserattr(user,S_PGRP,&oldpgrp,SEC_CHAR))
			return(-1);

	if (getgroupattr(oldpgrp,S_USERS,&oldpgrpmembers,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(-1);

	if (getgroupattr(newpgrp,S_USERS,&newpgrpmembers,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(-1);

	if (getuserattr(user,S_GROUPS,&grpset,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
			return(-1);

	/*
	 * Take the user out of the S_USERS list of
	 * the old pgrp.  If it's empty, no need to
	 * delete the user.
	 */
	if (oldpgrpmembers && *oldpgrpmembers)
	{
		listocom(oldpgrpmembers);
		delmems((char *)NULL, oldpgrpmembers, user, &oldusrset);
	}

	/*
	 * Take the old pgrp out of the S_GROUPS
	 * list of this user.  If this is empty,
	 * no need to process.
	 */
	if (grpset && *grpset)
	{
		listocom(grpset);
		delmems((char *)NULL, grpset, oldpgrp, &oldgrpset);
	}

	/*
	 * Add the user to the S_USERS list of the 
	 * new pgrp.
	 */
	if (newpgrpmembers && *newpgrpmembers)
	{
		listocom(newpgrpmembers);
		addusers(NULL,user,newpgrpmembers,&newusrset);
	}
	else
		newusrset = user;

	/*
	 * Add the new primary group to the S_GROUPS
	 * list of the new user.  If this list is not
	 * empty, the list is already a comma-separated
	 * list.
	 */
	if (oldgrpset && *oldgrpset)
	{
		addusers(NULL,newpgrp,oldgrpset,&newgrpset);
	}
	else
		newgrpset = newpgrp;

	/*
	 * turn strings into lists for putgroupattr/putuserattr
	 */
	strtolist(oldusrset,&finaloldusrset);
	strtolist(newgrpset,&finalgrpset);
	strtolist(newusrset,&finalnewusrset);

	/*
	 * Change the cache information at this time.  The
	 * cached information won't be written out until
	 * the SEC_COMMIT gets passed in to putuserattr() and
	 * putgroupattr().  This will happen in calling routines.
	 */
	if(putgroupattr(oldpgrp,S_USERS,finaloldusrset,SEC_LIST))
	{
		fprintf(stderr,USRTOGRPER,user,oldpgrp);
		return(-1);
	}

	if(putgroupattr(newpgrp,S_USERS,finalnewusrset,SEC_LIST))
	{
		fprintf(stderr,USRTOGRPER,user,newpgrp);
		return(-1);
	}

	if(putuserattr(user,S_GROUPS,finalgrpset,SEC_LIST))
	{
		fprintf(stderr,USRTOGRPER,user,newpgrp);
		return(-1);
	}

	if(putuserattr(user,S_PGRP,newpgrp,SEC_CHAR))
	{
		fprintf(stderr,USRTOGRPER,user,newpgrp);
		return(-1);
	}

	return(0);
}
