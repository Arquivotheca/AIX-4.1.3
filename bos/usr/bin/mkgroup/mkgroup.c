static char sccsid[] = "@(#)62	1.16.1.3  src/bos/usr/bin/mkgroup/mkgroup.c, cmdsuser, bos411, 9428A410j 3/1/94 18:39:13";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addattrs
 *		addself
 *		addthegroup
 *		main
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

#include	<stdio.h>	/* for printfs		*/
#include	<locale.h>	/* for setlocale()	*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for setpriv		*/
#include	"tcbauth.h"	/* for local stuff 	*/

static	void	addself(register char *);
static	void	addthegroup(register char *,register char *,gid_t);
static	void	addattrs (int,int,char *,int,char **);


/*
 *
 * NAME:     mkgroup
 *                                                                    
 * FUNCTION: creates a new group.
 *                                                                    
 * USAGE:    mkgroup [-a] [-A] "attr=value" ... group
 *	     where:
 *	              -a : is an administrative group.
 *	              -A : the real user ID is the group administrator.
 *		   group : the new group.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	if the group argument is not valid (contains invalid
 *			chars)
 *		EEXIST 	if the group already exists
 *		EACCES 	if the invoker doesn't have write access to the user
 *			database.
 *		EPERM 	if the group identification and authentication fails
 *			if -a is specified and invoker != root
 *		ENAMETOOLONG 	if the group name is too long
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
char	*group = argv[argc - 1];	/* groupname 			*/
register int	c;			/* counter for getopt		*/
register int	aflag = 0;		/* flag to indicate admin group */
register int	Aflag = 0;		/* flag to indicate adding self */
gid_t	id;
int	f;				/* ids file descriptor		*/
uid_t	auid;
uid_t	uid;
gid_t	agid;
gid_t	gid;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* check arguments */
	if (argc < 2 || (*group == '-'))
		usage(MKGRPUSAGE);

	/* 
	 * suspend auditing for this process
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* 
	 * suspend privilege
	 */
	privilege(PRIV_LAPSE);

	/* Open the user database */
	setuserdb( S_READ | S_WRITE);

	/* check groupname for restrictions */
	if(chkname(0,group))
		exitax(MKGRPAUD,errno,group,NULL,PRINT);

	/* parse the command line flags */
	if (argv[1][0] == '-')
	{
		while ((c = getopt(argc,argv,"aA")) != EOF)
		{
			switch (c)
			{
			case 'a':	aflag = 1;
					continue;
	
			case 'A': 	Aflag = 1;
					continue;

			default :	usage(MKGRPUSAGE);
			}
		}
	}

	/*
	 * Make sure we have access to make
	 * an administrative group 
	 */
	if (aflag && !gotaccess())
	{
		fprintf(stderr,ERRADDADM,group);
		exitax(MKGRPAUD,EPERM,group,NULL,PRINT);
	}

	/* 
	 * if -a make an admin group
	 * if -A make an ordinary group with admin = invoker
	 * if -a and -A make an admin group with admin = invoker
	 */

	/* get the admin and ordinary ids */
	f = getids(group,&auid,&uid,&agid,&gid,MKGRPAUD);

	id = aflag==1?agid:gid;

	/*
	 * No need to check return code here.
	 * Add the new group to data base.
	 */
	addthegroup(aflag==0?_FALSE:_TRUE,group,id);

	if (Aflag)
		addself(group);

	/*
	 *  Add any user supplied attributes
	 *  at this time.  No need for error checking 
	 *  here since this routine exits on error.
	 */
	addattrs(aflag,Aflag,group,argc,argv);

	/* write the data from cached memory to the data base */
	if (putuserattr((char *)NULL,(char *)NULL,((void *) 0),SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,group);
		exitax(CHUSRAUD,errno,group,NULL,PRINT);
	}
	
	if(putgroupattr(group,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,group);
		exitax(MKGRPAUD,errno,group,NULL,PRINT);
	}

	/* Close the user database */
	enduserdb();

	/*
	 * Increment the id and rewrite the /etc/security/.ids file.
	 * The first argument passed is 1 which tells inkids()
	 * that we've added a new group.
	 */
	inkids(1,aflag,auid,uid,agid,gid,f);

	/* audit success */
	exitax(MKGRPAUD,0,group,id,NOPRINT);
}

/*
 * FUNCTION:	addself
 *
 * DESCRIPTION:	adds the invoker as an administrator of the group
 *
 * PASSED: 	the group to be added (groupname).
 *
 * RETURNS:	No return.
 *
 */

static	void
addself(register char *groupname)
{
char	*username;	/* user to be made an administrator */
uid_t	uid;

	uid = getuid();
	if ((username = (char *)IDtouser(uid)) == NULL)
	{
		fprintf(stderr,ERRADM,username,groupname);
		fprintf(stderr, ".\n");
	}

	if(putgroupattr(groupname,S_ADMS,(void *)username,SEC_LIST))
	{
		fprintf(stderr,ERRADM,username,groupname);
		exitax(MKGRPAUD,errno,groupname,NULL,PRINT);
	}
}

/*
 * FUNCTION:	addthegroup
 *
 * DESCRIPTION:	adds entries to /etc/group and /etc/security/group, and
 *		updates /etc/security/group with "admin = true|false"
 *
 * RETURNS:	None.
 *
 */

static	void
addthegroup(register char *type,register char *groupname,gid_t gid)
{
	if (putgroupattr(groupname,S_ID,(void *)gid,SEC_INT))
	{
		fprintf(stderr,ERRADD,groupname);
		exitax(MKGRPAUD,errno,groupname,NULL,PRINT);
	}

	if (putgroupattr(groupname,S_ADMIN,(void *)type,SEC_BOOL))
	{
		fprintf(stderr,ERRADD,groupname);
		exitax(MKGRPAUD,errno,groupname,NULL,PRINT);
	}
}

/*
 * FUNCTION:	addattrs
 *
 * DESCRIPTION:	add the attributes to the specific groups database files
 *
 * RETURNS:	None.
 *
 */

static void
addattrs (int aflag,int Aflag, char *group,int argc,char **argv)
{
int	i=1;
char	*attr;
char	*val;

	/*
	 * want to look at second or third arguments
	 * depending on if aflag and/or Aflag is
	 * set to true.
	 */
	if (aflag && Aflag)
		i = 3;
	else
		if (aflag || Aflag)
			i = 2;

	/* get the passed in attribute values */
	while (argv[i] && (i != (argc - 1)))
	{
		if (getvals(argv[i],&attr,&val))
			usage(MKGRPUSAGE);
		
		/*
		 *  Only root can change the admin attribute.
		 */
		if (!strcmp(attr,S_ADMIN))
		{
			if(!gotaccess())
			{
				fprintf(stderr,CHGTOERR,attr,val);
				exitax(CHGRPAUD,EPERM,group,(char *)NULL,
							PRINT);
			}
		}
				
		/* add the attribute */
		if (addvalue(group,attr,val,(char *)NULL))
		{
			fprintf(stderr,CHGTOERR,attr,val);
			exitax(CHGRPAUD,errno,group,(char *)NULL,PRINT);
		}
		i++;
	}
}
