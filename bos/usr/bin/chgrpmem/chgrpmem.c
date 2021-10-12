static char sccsid[] = "@(#)56	1.11.1.4  src/bos/usr/bin/chgrpmem/chgrpmem.c, cmdsuser, bos411, 9428A410j 12/9/93 19:27:34";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addmem
 *		chkifadm
 *		delmem
 *		getgat
 *		isgrpadms
 *		listmem
 *		main
 *		runprog
 *		setmem
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

#include	<stdio.h>
#include 	<errno.h>
#include 	<sys/access.h>
#include 	<locale.h>		/* for setlocale() */
#include 	"tcbauth.h"

/* local defines */
static	void	addmem(int flag,char *group,char *members);
static	char	*getgat(char *name,char *attr);
static	void	delmem(int flag,char *group,char *members);
static	void	setmem(int flag,char *group,char *members);
static	void	listmem(char *group);
static	void	runprog(char *groupname,char *list);
static	int	isgrpadms(char *groupname);
static	int	chkifadm(char *adms,char *name);

/*
 *
 * NAME:     chgrpmem
 *                                                                    
 * FUNCTION: adds, deletes, or sets members or administrators of a group
 *                                                                    
 * USAGE:    chgrpmem {{ -a | -m } { + | - | = }} <list> group
 * 	     where:
 *			-a	: indicates administrators.
 *			-m	: indicates members.
 *			 +	: indicates an addition to the list.
 *			 -	: indicates a deletion from the list.
 *			 =	: indicates setting the list.
 *			"list"	: is the list of users 
 *			"group"	: is the group.
 *
 *	the invoker has to be a defined administrator or have execute
 *		access to the chgroup command.
 *                                                                   
 * 	RETURNS: 0 if successful else the return is as follows:
 *		
 *		ENOENT	if the group or one or more of the users does not exist
 *		EPERM	if the invoker isn't an administrator of the group nor
 *			has execute access to chgroup.
 *		EINVAL	if an usage error occurs (no operator,invalid NULL list)
 *		
 */  

int
main(int argc,char *argv[])
{
int	flag = 0;	      /* indicates changing a member or admin	*/
int	listmemflag = 0;      /* this flag set if listing group info	*/
int	addflag = 0;          /* flag for + option 			*/
int	delflag = 0;          /* flag for - option 			*/
int	setflag = 0;          /* flag for = option 			*/
char	*group;		      /* the group to be changed 		*/
char	*temp;		      /* temporary pointer 			*/
char	*mems = (char *)NULL; /* the member list 			*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* 
	 * Start the command line parsing.  Lots of combinations
	 * here, so start with what can't happen first.
	 */
	if (argc < 2 || argc == 3)
		usage(CHMEMUSAGE);

	/* check for arguments to list members */
	if ((argc == 2) && (argv[1][0] != '-'))
		listmemflag = 1;

	if (argc != 5 && !listmemflag)
		if (!((argc==4) && (argv[2][0]=='=') && (argv[2][1]=='\0')))
			usage(CHMEMUSAGE);

	/*
	 * if not listing, then make sure correct flags
	 * are given to change group membership
	 */

	if (!listmemflag)
	{
		switch (argv[1][1])
		{
			case 'm':	if (strcmp(argv[1],"-m"))
						usage(CHMEMUSAGE);
					flag = MEM;
					break;
	
			case 'a': 	if (strcmp(argv[1],"-a"))
						usage(CHMEMUSAGE);
					flag = ADM;
					break;
	
			default : 	if (!listmemflag)
						usage(CHMEMUSAGE);
		}
	
		if (flag)
		{
			if (argv[2][1] != '\0')
				usage(CHMEMUSAGE);
	
			switch(argv[2][0])
			{
		
				case '+' :
					addflag = 1;
					break;
		
				case '-' :
					delflag = 1;
					break;
	
				case '=' :
					setflag = 1;
					break;
	
				default  :   usage(CHMEMUSAGE);
			}
		}
	}

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	group = argv[argc-1];

	/* make sure we have a valid group */
	if (chkgrp(group,&temp))
	{
		fprintf (stderr,GRPNONEX,group);
		exit (ENOENT);
	}

	/* check for execute access to /usr/bin/chgroup */
	if (accessx(CHGROUP,X_ACC,ACC_INVOKER)) 
	{
		/* see if invoker is an administrator of this group */
	   	if (!isgrpadms(group))
		{
			fprintf(stderr,UNKSVAL,argv[0]);
			fprintf (stderr, PERMISSION);
			exitax(CHGRPAUD,EPERM,group,(char *)NULL,NOPRINT);
		}
	}

	if (listmemflag)
		listmem(group);

	/* 
	 * If our arg count is 4, then we are setting
	 * our member list to NULL.  Otherwise, our
	 * count is 5 and we have a list in [argc-2].
	 */ 
	if (argc != 4)
		mems = argv[argc-2];

	/* Close the user database */
	enduserdb();

	/* 
	 * These routines exec /usr/bin/chgroup
	 */
	if (addflag)
		addmem(flag,group,mems);
	if (delflag)
		delmem(flag,group,mems);
	if (setflag)
		setmem(flag,group,mems);
}

/*
 * FUNCTION:	isgrpadms
 *
 * DESCRIPTION:	Checks if the invoker is an administrator of the group
 *
 * PASSED: 	groupname = the group to be checked.
 *
 * RETURNS:	True if invoker is an administrator, else false.
 */

static	int
isgrpadms(char *groupname)
{
char	*name;		/* invoker's login name			*/
char	*adms = NULL;	/* list of administrators of this group	*/
int	err = 0;

	/* get the groupname's administrators */
	err = getgroupattr(groupname,S_ADMS,(void *)&adms,SEC_LIST);

	/* if the group doesn't have any adms users, that's fine */
	if (err || !adms)
		return(0);

	/* get the invoker's login name */
	if ((name = (char *)IDtouser(getuid())) == NULL)
		return (0);

	/* check if the invoker is in the list of administrators */
	if(chkifadm(adms,name))
		return(0);

	return(1);
}

/*
 * FUNCTION:	chkifadm
 *
 * DESCRIPTION:	Parses the list of administrators to see whether the 
 *		name passed is in this list.
 *
 * PASSED:	adms = list of administrators, name = the name to be checked.
 *
 * RETURNS:	0 or -1.
 */

static	int
chkifadm(char *adms,char *name)
{
char	**adm;	/* temporary array pointer */
int	n;	/* number of elements 	   */
int	i;	/* temp counter		   */

	/*
	 * Set up array for checks.  Then
	 * loop through and check for name.
	 */
	adm = comtoarray(adms,&n);

	for ( i=0 ; i<n ; i++)
	{
		if (!strcmp(name,adm[i]))
			return (0);
	}
	return(-1);
}

/*
 * FUNCTION:	addmem
 *
 * DESCRIPTION:	Gets the current member list, adds the 
 *		new members to the list and execs chgroup.
 *
 * PASSED:	flag = indicates admin or not, group = the group to be
 *		changed, members = the list of users.
 *
 * RETURNS:	no return.
 */

static	void
addmem(int flag,char *group,char *members)
{
char	*users;		/* pointer to list of current users or admins	*/
char	*usrset;	/* the final userset after adding the new list	*/

	/* get current user or group set */
	users = getgat(group,flag==ADM?S_ADMS:S_USERS);

	/*
	 * Add the new list to the current list.
	 * No need to check return since addusers() exits
	 * on error.
	 */
	addusers(flag==ADM?AHEAD:MHEAD,users,members,&usrset);

	/* execute /usr/bin/chgroup */
	runprog(group,usrset);
}

/*
 * FUNCTION:	getgat
 *
 * DESCRIPTION:	Uses getgroupattr to get the group's current members or
 *		administrators.
 *
 * PASSED:	name = the name of the group, attr = admin or user
 *
 * RETURNS:	a comma-separated null terminated list or NULL.
 *
 */

static char	*
getgat(char *name,char *attr)
{
char	*val;	/* place to hold the return	*/
	
	if(getgroupattr(name,attr,(void *)&val,SEC_LIST))
		return((char *)NULL);

	return(listocom(val));
}

/*
 * FUNCTION:	delmem
 *
 * DESCRIPTION:	Deletes the members from the current member list
 *		and execs chgroup.
 *
 * PASSED:	flag = admin or ordinary, group = the group to change,
 *		members = the members to delete.
 *
 * RETURNS:	No return.
 *
 */

static	void
delmem(int flag,char *group,char *members)
{
char	*users;		/* the current user set	*/
char	*newmemset;	/* the new member list */

	/* get current user or group set */
	users = getgat(group,flag==ADM?S_ADMS:S_USERS);

	/* add the new list to the current list */
	if (delmems(flag==ADM?AHEAD:MHEAD,users,members,&newmemset))
	{
		fprintf(stderr,USRNIN,newmemset);
		exit (EINVAL);
	}

	/* execute /usr/bin/chgroup */
	runprog(group,newmemset);
}

/*
 * FUNCTION:	setmem
 *
 * DESCRIPTION:	execs chgroup with the new list.
 *
 * PASSED:	flag = admin or not, group = the group to change, 
 *		members = the new users to set.
 *		
 * RETURNS:	No return.
 *
 */

static	void
setmem(int flag,char *group,char *members)
{
char	*list;		/* pointer to final set */
char	*header;	/* header for chgroup 	*/
int	siz;		/* size of list 	*/
		
	header = (flag==ADM?AHEAD:MHEAD);

	/* get some space to hold the new string */
	if (members && *members)
	{
		siz = strlen(header) + strlen(members) + 2;
		if ((list = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}

		strcpy(list,header);
		strcat(list,members);
	}
	else
	{
		siz = strlen(header) + 1;
		if((list = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
		strcpy(list,header);
	}

	/* execute /usr/bin/chgroup */
	runprog(group,list);
}

/*
 * FUNCTION:	listmem
 *
 * DESCRIPTION:	lists the groups members and administrators.
 *
 * PASSED:	group = the groupname
 *
 * RETURNS:	No return.
 *
 */

static	void
listmem(char *group)
{
char	*users;	/* pointer to the users list	*/
char	*adms;	/* pointer to the admins list	*/
int	id;
	
	if (getgroupattr (group,S_ID,&id,SEC_INT))
	{
		fprintf(stderr,GRPNONEX,group);
		exit (ENOENT);
	}
		
	printf ("%s:\n",group);
	users = getgat(group,S_USERS);
	printf(MEMBS,users);

	adms = getgat(group,S_ADMS);
	printf(ADMIN,adms);

	printf ("\n");

	exit(0);
}

/*
 * FUNCTION:	runprog
 *
 * DESCRIPTION:	runs /usr/bin/chgroup
 *
 * PASSED:	groupname = the group's name, list = the list to change
 *
 * RETURNS:	No return.
 *
 */

static	void
runprog(char *groupname,char *list)
{
	execl(CHGROUP,"chgroup",list,groupname,0);
	fprintf (stderr,EXECL,CHGRPMEM);
	fprintf (stderr,CHECK,CHGROUP);
	exit (EACCES);
}
