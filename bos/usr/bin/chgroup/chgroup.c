static char sccsid[] = "@(#)54	1.21  src/bos/usr/bin/chgroup/chgroup.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:37:46";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: checkprime
 *		main
 *		makechanges
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

#include	<stdio.h>	/* for printfs		*/
#include	<errno.h>	/* for perror		*/
#include	<pwd.h>		/* for passwd struct 	*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for privilege 	*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<userpw.h>	/* for PW_NAMELEN 	*/
#include	<grp.h>		/* for getgrnam()	*/
#include	"tcbauth.h"	/* for local variables	*/

static	void	makechanges(char *group,int argc,char **argv);
static	int	checkprime(char *group,char *val,char *arecord);

/*
 *
 * NAME: 	chgroup
 *                                                                    
 * FUNCTION: 	changes attributes of a group
 *                                                                    
 * USAGE:       chgroup "attr=value" ... <group>
 *	        where:
 *			"attr"	: is a valid user attribute.
 *			"value"	: is a valid value for that attribute.
 *			"group"	: is the group to be changed.
 *
 *
 * PASSED:  	argc = the number of attributes entered
 *		argv[] = the attributes entered
 *
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	if the attribute or attribute value is invalid
 *			if trying to remove a user from their principal group
 *		ENOENT 	if the group or user(s) does not exist
 *		EACCES 	if the attribute cannot be changed-invoker doesn't have
 *			write access to user database.
 *		EPERM 	if the group identification and authentication fails -
 *			if admin group or attribute=admin, the invoker must be
 *			 root.
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
char	*group;
int	id;
struct	group *gr;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/*
	 * must be minimum of: chgroup "attr=value" groupname 
	 */
	if (argc < 3)
		usage(CHGRPUSAGE);

	group = argv[ argc - 1 ];

	/* 
	 * suspend auditing and privilege
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
	privilege(PRIV_LAPSE);

	/* Check for group's length < PW_NAMELEN */
	if(strlen(group) > PW_NAMELEN - 1)
	{
		fprintf(stderr,CHGONERR,group);
		fprintf(stderr,TOOLONG);
		exit(ENAMETOOLONG);
	}
	
	/*
	 * Does this group exist locally or in NIS?
	 */
	set_getgrent_remote(2);	/* compat lookups only	*/
	gr = getgrnam(group);
	set_getgrent_remote(1);	/* resume full lookups	*/
	if (gr == (struct group *)NULL)
	{
	 	fprintf(stderr,GRPNONEX,group);
		exit(ENOENT);
	}

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/*
	 * see if the invoker has permission to change this group
	 */
	if (!gotgaccess(group))
	{
		fprintf(stderr,CHGONERR,group);
		exitax(CHGRPAUD,EPERM,group,(char *)NULL,PRINT);
	}

	/*
	 * Make the changes to the group.  This routine
	 * audits and exits.
	 */
	makechanges(group,argc,argv);
}

/*
 * FUNCTION:	makechanges
 *
 * DESCRIPTION:	Calls routines to parse input, add the info
 *		to the data base, close the data base.
 *
 * PASSED:	group = the groupname, argc = how many attributes,
 *		argv[] = the parameters
 *
 * RETURNS:	None.
 *
 */

static void
makechanges(char *group,int argc,char *argv[])
{
register int	i;			  /* counter 			 */
char		*attr;			  /* pointer to attribute 	 */
char		*val;			  /* pointer to attribute value  */
char		*arecord = (char *)NULL;  /* audit record 		 */
register int	siz;			  /* amount of space to allocate */

	for (i = 1; i < (argc - 1); i++)
	{
		/*
		 * build the audit record now to 
		 * preserve the attr=value string.
		 */
		if (i == 1)
		{
			if ((arecord = malloc(strlen(argv[i]) + 1)) == NULL)
			{
				fprintf (stderr,MALLOC);
				exitx (errno);
			}
			strcpy(arecord,argv[i]);
		}
		else
		{
			siz = strlen(arecord) + strlen(argv[i]) + 2;
			if ((arecord = realloc(arecord,siz)) == NULL)
			{
				fprintf (stderr,MALLOC);
				exitx (errno);
			}
			strcat(arecord,SPACE);
			strcat(arecord, argv[i]);
		}

		if (getvals(argv[i],&attr,&val))
			usage(CHGRPUSAGE);

		/* Only root can change the admin attribute. */
		if (!strcmp(attr,S_ADMIN))
		{
			if(!gotaccess())
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(CHGRPAUD,EPERM,group,arecord,PRINT);
			}
		}
				
		/*
		 * If the attribute is "users", we must ensure that
		 * we don't remove a user from their primary group.
		 * Call checkprime() with "val" representing the
		 * list of users.
		 */

		if (!strcmp(attr,S_USERS))
		{
			if(checkprime(group,val,arecord))
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(CHGRPAUD,BADVALUE,group,arecord,PRINT);
			}
		}
 
		/* add the attribute */
		if (addvalue(group,attr,val,arecord))
		{
			fprintf(stderr,CHGTOERR,attr,val);
			exitax(CHGRPAUD,errno,group,arecord,PRINT);
		}

	}

	/*
	 * commit the changes to the data base
	 */
	if (putuserattr((char *)NULL,(char *)NULL,((void *) 0),SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,group);
		exitax(CHUSRAUD,errno,group,arecord,PRINT);
	}

	if (putgroupattr(group,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,group);
		exitax(CHGRPAUD,errno,group,arecord,PRINT);
	}

	/*
	 * If the ID for the group is changed, /etc/passwd
	 * is not updated with new ID numbers for users with
	 * this group as their primary group.
	 */
	if (!strcmp(attr, S_ID))
		fprintf(stderr,WARN_NO_UPDATE,CHGROUP,PASSWD);

	/* Close the user database */
	enduserdb();

	/* audit successful change */
	exitax(CHGRPAUD,0,group,arecord,NOPRINT);

}

/*
 * FUNCTION:	checkprime
 *
 * DESCRIPTION:	Checks to see if we are deleting a user that
 *		has this group as a primary group.
 *
 * PASSED:	group = the group to be checked, userlist = the list of users
 *		arecord = audit record in case of error.
 *
 * RETURNS:	true or false.
 *
 */

static	int
checkprime(char *group,char *userlist,char *arecord)
{
struct passwd	*pwd;
register int	i = 0, j = 0;
int		userct = 0;
int		id;
char		*users;			   /* the string of users passed in */
char		*members;		   /* the current members of group  */
char		**newusrs = (char **)NULL; /* the new user array 	    */
char		**currmems;		   /* the current member array 	    */
int		memberct;		   /* permanent count of 'members'  */

	if (getgroupattr(group,S_ID,(void *)&id,SEC_INT))
		return(-1);

	if (getgroupattr(group,S_USERS,(void *)&members,SEC_LIST))
		return(-1);

	currmems = listoarray(members,&memberct);

	if (userlist && *userlist)
	{
		if ((users = (char *)malloc (strlen(userlist) + 1)) == NULL)
		{
			fprintf (stderr,MALLOC);
			exitx (errno);
		}
		strcpy(users,userlist);
		newusrs = comtoarray(users,&userct);
	}

	setpwent();

	for(i = 0; i < memberct; i++)
	{
		if((pwd = (struct passwd *)getpwnam(currmems[i])) != NULL)
		{
			if(pwd->pw_gid == id)
			{
				for(j = 0; j < userct; j++)
					if(!strcmp(currmems[i], newusrs[j]))
						break;
				if(j == userct)
				{
					fprintf(stderr,DROP,currmems[i],group);
					exitax(CHGRPAUD,EINVAL,group,arecord,
						NOPRINT);
				}
			}
		}
	}

	endpwent();

	return(0);
}
