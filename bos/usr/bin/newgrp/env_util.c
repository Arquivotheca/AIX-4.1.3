static char sccsid[] = "@(#)00	1.6.1.2  src/bos/usr/bin/newgrp/env_util.c, cmdsuser, bos411, 9428A410j 10/7/93 12:11:54";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: beqpriv
 *		exitex
 *		getreal
 *		getrealgroup
 *		runusrshell
 *		xusage
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

#include <sys/priv.h>
#include <sys/audit.h>
#include <userpw.h>
#include "tcbauth.h"

static	void	runusrshell(char *,int,char *,char *);

/*
 * FUNCTION:	xusage
 *
 * DESCRIPTION:	prints usage and calls exitex to exec another shell
 *
 * RETURNS:	No return.
 *
 */

void
xusage(char *program,char *event,char *vals)

{
	fprintf(stderr,program);
	runusrshell(event,EINVAL,vals,(char *)NULL);
}

/*
 *
 * FUNCTION:	exitex
 *
 * DESCRIPTION:	cuts audit record, calls runusrshell to exec another shell.
 *
 * RETURNS:	No return.
 *
 * PASSED:
 * 	char	*type;		 the audit event name
 * 	int	result;		 error value usually errno
 * 	char	*name;		 the group or user name effected
 * 	char	*attrs;		 the values changed
 * 	int	pflag;		 whether or not to print messages
 */

void
exitex(char *type,int result,char *name,char *attrs,int pflag)
{
	if (result && pflag)
	{
		fflush(stderr);
		switch (result)
		{
			case EPERM : fprintf (stderr, PERMISSION);
					break;

			case EINVAL : fprintf (stderr, ERBADVAL);
					break;

			case EXISTS : result = EEXIST;
					break;

			case NOEXISTS : result = ENOENT;
					break;

			case ESAD : 	break;

			case ENAMETOOLONG : fprintf (stderr, TOOLONG);
					break;

			case ENOMEM : fprintf (stderr, NOMEM);
					break;

			case ENOTTY : fprintf (stderr, ERNOTERM);
					break;

			case BADATTR : fprintf (stderr, ERBADATTR);
					result = EINVAL;
					break;

			case BADVALUE : fprintf (stderr, ERBADVAL);
					result = EINVAL;
					break;

			default : fprintf (stderr, ".\n");
		}
	}

	else
	{
		if (pflag)
			fprintf (stderr, ".\n");
	}

	runusrshell(type,result,name,attrs);
}

/*
 *
 * FUNCTION:	runusrshell
 *
 * DESCRIPTION:	gets user's shell from environ and runs it.
 *
 * RETURNS:	No return.
 *
 * PASSED:
 * 	char	*type;		 the audit event name
 * 	int	result;		 error value usually errno
 * 	char	*name;		 the group or user name effected
 * 	char	*attrs;		 the values changed
 *
 */

static	void
runusrshell(char *type,int result,char *name,char *attrs)
{
char	*program;
char	cprog[BUFSIZ];
priv_t	priv;
register int	siz;
register int	siz1;
int	fd;
char	*tty;

	/*
	 * if the user has taken away any of stdout and stdin and
	 * stderr, then we must ensure that we exec the shell
	 * through the controlling terminal.
	 */

	fd = open("/dev/tty", O_RDWR);
	tty = ttyname(fd);
	close(fd);
	fd = open(tty, O_RDWR);
	close(0); close(1); close(2);
	dup(fd); dup(fd); dup(fd);
	close(fd);

	privilege(PRIV_ACQUIRE);

	/* audit the event failure */
	siz = strlen(name) + 1;
	siz1 = strlen(attrs) + 1;
	auditwrite(type,AUDIT_FAIL,name,siz,attrs,siz1,0);

	seteuid(getuid());

	/* drop effective privilege */
	priv.pv_priv[0] = 0;
	priv.pv_priv[1] = 0;
	
	setpriv(PRIV_SET|PRIV_EFFECTIVE, &priv,sizeof(priv_t));

	/* is this redundant ? */
	privilege (PRIV_DROP);

	/* get environment shell to exec */
	if (program = (char *)getenv("SHELL"))
	{
		strcpy(cprog,strrchr(program,'/') + 1);
		execl(program,cprog,(char *)0);
	}

	/* exec /usr/bin/sh as last resort */
	execl(BINSH,"sh",(char *)0);

	/* if exec'ing /usr/bin/sh doesn't work --> good luck */
	fprintf(stderr,EXECL,BINSH);
	sleep (5);
	exit (errno);
}

/*
 * FUNCTION: 	beqpriv
 *
 * DESCRIPTION: drop bequeathed privilege.
 *
 * RETURN: 	return from setpriv().
 *
 */

int
beqpriv()
{
priv_t	priv;
	
	priv.pv_priv[0] = 0;
	priv.pv_priv[1] = 0;
	
	return(setpriv(PRIV_SET|PRIV_BEQUEATH, &priv,sizeof(priv_t)));
}

/*
 * FUNCTION:	getreal
 *
 * DESCRIPTION:	builds grouplist with real group changed.
 *
 * RETURNS:	new real group or exits on error.
 *
 * PASSED:
 *	int flag		INIT or DELTA
 *	char *pgroups		current process groupset
 *	char *ugroups		current user groups
 *	char *newreal		new real group
 *	char **finalgrpset	final groupset
 *
 */

char *
getreal(int flag,char *pgroups,char *ugroups,char *newreal,char **finalgrpset)
{
int		match = 0, ug = 0, pg = 0, c = 0;
int		id;
char		**pgrps;
char		**ugrps;
char		group[PW_NAMELEN];
register int	siz;
register int	check = 1; /* indicates whether to check for group membership */

	/* get some space for the new groupset */
	siz = strlen(HEADER) + strlen(pgroups) + strlen(newreal) + 3;
	if((*finalgrpset = (char *)malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}

	if (!newreal)
	{
		getrealgroup(ugroups,group);
		if (pgroups)
		{
			while (*pgroups && *pgroups != ',')
				pgroups++;
			if (*pgroups == ',')
				pgroups++;

			/*
			 * we don't have to check the groupset
			 * because we got it out of the data base 
			 */
			check = 0;
		}

		newreal = group;
	}

	/* Check for groups's length < PW_NAMELEN */
	if(strlen(newreal) > PW_NAMELEN - 1)
	{
		fprintf(stderr,LONGGROUP,newreal);
		exitex(SETGRAUD,ENAMETOOLONG,newreal,pgroups,NOPRINT);
	}

	if (getgroupattr(newreal,S_ID,(void *)&id,SEC_INT))
	{
		fprintf(stderr,GRPNONEX,newreal);
		exitex(SETGRAUD,ENOENT,newreal,pgroups,NOPRINT);
	}

	/* change group strings to arrays */
	ugrps = comtoarray(ugroups,&ug);
	pgrps = comtoarray(pgroups,&pg);

	/*
	 * is the new real group a member of the user's groupset?
	 * If we are root (getuid() == 0), bypass the check and
	 * let root change to whatever he wants
	 */
	if (check && getuid())
	{
		while(ugrps[c])
		{
			if(strcmp(ugrps[c++],newreal) == 0)
			{
				match=1;
				break;
			}
		}

		/* if not in current groupset --> error */
		if (!match)
		{
			fprintf(stderr,GRPNINST,newreal);
			exitex(SETGRAUD,EPERM,newreal,pgroups,NOPRINT);
		}
	}

	/* copy in GROUPS= , and the new real group */
	strcpy(*finalgrpset,HEADER);
	strcat(*finalgrpset,newreal);

	/* if no process groups return */
	if (!pgroups)
		return (newreal);

	/* build the new groupset here */
	c = 0;
	while (pgrps[c])
	{
		/* don't copy newreal if there... */
		if (strcmp(pgrps[c], newreal))
		{
			strcat(*finalgrpset,COMMA);
			strcat(*finalgrpset, pgrps[c]);
		}
		c++;
	}
	return(newreal);
}

/*
 *
 * FUNCTION:	getrealgroup
 *
 * DESCRIPTION:	parses group string and gets the real group name.
 *
 * RETURNS:	No return.
 *
 */

void
getrealgroup(char *groups,char *group)
{
register int	siz;
char 		*ptr;

	memset (group,NULL,PW_NAMELEN);
	ptr = groups;
	while ((*ptr != ',') && (*ptr != '\0'))
		ptr++;

	siz = ptr - groups; 
	if (siz > PW_NAMELEN - 1)
	{
		*ptr = '\0';
		fprintf(stderr,LONGGROUP,groups);
		exitex(SETGRAUD,ENAMETOOLONG,groups,++ptr,NOPRINT);
	}
	strncpy(group,groups,siz);
}
