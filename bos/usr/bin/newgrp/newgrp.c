static char sccsid[] = "@(#)01	1.8.1.5  src/bos/usr/bin/newgrp/newgrp.c, cmdsuser, bos412, 9446B 11/15/94 17:01:09";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: backtologinenv
 *		main
 *		resenv
 *		resetreal
 *		runsetpenv
 *		setpgroups
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <userpw.h>		/* for PW_NAMELEN	*/
#include <sys/audit.h>		/* for auditing		*/
#include "tcbauth.h"		/* for local defines	*/
#include <locale.h>		/* for setlocale()	*/
#include <sys/id.h>		/* for ID_EFFECTIVE	*/

static	void	backtologinenv(int);
static	void	resenv(int,char **);
static	void	resetreal(int,char *);
static	void	setpgroups(int,char *,char *);
static	void	runsetpenv(int,char *,char *);

/*
 *
 * NAME:     newgrp
 *                                                                    
 * FUNCTION: changes the user's group identification
 *                                                                    
 * USAGE:    newgrp [- | -l] [group]
 *           where:
 *           [ - ] OR [ -l ]  specifies that the environment is to be
 *                            initialized to initial login environment
 */

main(int argc, char **argv)
{
int	flag = 0;
int	i;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* suspend auditing */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* suspend privilege */
	privilege(PRIV_LAPSE);

	/* check for execution at a terminal */
	if (checkfortty())
	{
		fprintf(stderr,TERMINAL,NEWGRP);
		exitex(SETGRAUD,ENOTTY,NULL,NULL,NOPRINT);
	}

	for ( i = 1 ; i < argc ; i++ ) {

		if (argv[i][0] == '-') {
			if ( (argv[i][1] == '\0' || (argv[i][1] == 'l') &&
							argv[i][2] == '\0')) {
				/*
				 * we want to reset the environment to
			 	 * its original login state 
				 */
				flag |= PENV_INIT | PENV_KLEEN;
				if (argv[i+1] == NULL)
					backtologinenv(flag);
			}
			else if (argv[i][1] == '-' && argv[i][2] == '\0') {
				if (argv[i+1] == NULL)
					backtologinenv(flag);
				else {
					i++;
					break;
				}
			}
			else xusage(NEWGRUSAGE,SETGRAUD,NULL);
		}
		else 
			break;
	}
		
	/* 
	 * This routine does not return.
	 */
	if (flag == 0)
		flag |= PENV_DELTA;
	resetreal(flag,argv[i]);
}

/*
 *
 * FUNCTION:	resetreal
 *
 * DESCRIPTION:	changes process' real group to newgrp.
 *
 * PASSED:	flag = INIT or DELTA, newgrp = new real group for process
 *
 * RETURNS:	No return.
 *
 */

static	void
resetreal(int flag,char *newgrp)
{
char	*ugroups;	/* current user groupset 	*/
char	*pgroups;	/* current process groupset 	*/
char	*real;		/* current real group 		*/
char	**groups;	/* return from getpcred 	*/
char	*finalgrpset;	/* final groupset 		*/
char	*name;		/* user name 			*/
char	*val;		/* return from getuserattr 	*/
uid_t	uid;		/* return from getuid 		*/
	
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

	if (!newgrp)
	{
		if(getuserattr(name,S_PGRP,(void *)&newgrp,SEC_CHAR))
		{
			fprintf(stderr,GETUSRGRPS,name);
			fprintf (stderr, CHECK , PASSWD);
			exitex(SETGRAUD,errno,NULL,NULL,NOPRINT);
		}
	}

	/* turn NULL separated string to comma separated */
	ugroups = listocom(val);

	/* reacquire privilege that was dropped earlier */
	privilege(PRIV_ACQUIRE);

	/* get process' groupset */
	if ((groups = getpcred(CRED_GROUPS)) == NULL) 
	{
		fprintf(stderr,GETPCRED);
		exitex(SETGRAUD,errno,groups,NULL,PRINT);
	}

	/* lapse privilege again */
	privilege(PRIV_LAPSE);

	if ((pgroups = (char *)strstr(groups[0],"=")) == NULL)
	{
		fprintf(stderr,GETPCRED);
		exitex(SETGRAUD,errno,NULL,NULL,PRINT);
	}
	/* drop "=" */
	pgroups++;

	/* check for valid groups and build final groupset string */
	if (flag & PENV_INIT)
	{
		if ((pgroups = (char *)malloc(strlen(ugroups) + 1)) == NULL)
		{
			fprintf (stderr,MALLOC);
			exitex(SETGRAUD,errno,NULL,NULL,PRINT);
		}
		strcpy (pgroups,ugroups);
	}

	real = getreal(flag,pgroups,ugroups,newgrp,&finalgrpset);

	/* run new process with new groupset */
	setpgroups(flag,real,finalgrpset); 

}

/*
 * FUNCTION:	setpgroups
 *
 * DESCRIPTION:	calls setpcred to set the process groupset.
 *
 * RETURNS:	No return.
 *
 * PASSED:
 *	int flag		INIT or DELTA
 *	char *groups		current process groupset
 *
 */

static	void
setpgroups(int flag,char *real,char *groups) 
{
char		*ptr;	    /* to increment through groups string 	*/
char		*creds[3];  /* array to go to setpcred 			*/
register int 	siz;	    /* space to get 				*/

	siz = strlen(REAL_GROUP) + strlen(real) + 1;
	if ((ptr = (char *)malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETGRAUD,errno,real,groups,PRINT);
	}
	
	strcpy(ptr,REAL_GROUP);
	strcat(ptr,real);

	/* set up new group array */
	creds[0] = ptr;
	creds[1] = groups;
	creds[2] = (char *)NULL;

	/* reacquire lapsed privilege */
	privilege(PRIV_ACQUIRE);

	/* set process credentials with new groupset */
	if(setpcred((char *)NULL,creds))
	{
		fprintf(stderr,SETPCRED);
		exitex(SETGRAUD,errno,creds[0],creds[1],PRINT);
	}

	/* reset process environment */
	resenv(flag,creds);
}

/*
 * FUNCTION:	resenv
 *
 * DESCRIPTION:	cuts audit record and calls runsetpenv.
 *
 * RETURNS:	none.
 *
 */

static	void
resenv(int flag,char **creds)
{
register int siz;	/* space to get */
register int siz1;	/* space to get */

	/* audit credential change */
	siz = strlen(creds[0]) + 1;
	siz1 = strlen(creds[1]) + 1;
	auditwrite(SETGRAUD,AUDIT_OK,creds[0],siz,creds[1],siz1,0);

	runsetpenv(flag,creds[0],creds[1]);
}

/*
 *
 * FUNCTION:	runsetpenv
 *
 * DESCRIPTION:	calls setpenv to reset the environment.
 *
 * RETURNS:	no return on success, exits on error.
 *
 */

static	void
runsetpenv(int flag,char *real,char *groups)
{

	/* set the effective user id to the real */
	setuidx (ID_EFFECTIVE,getuid());

	/* drop bequeathed privilege */
	if (beqpriv())
	{
		fprintf(stderr,SETPRIV);
		exitex(SETGRAUD,errno,NULL,NULL,PRINT);
	}

	/* set process environment should not return */
	setpenv((char *)NULL,flag,(char **)NULL,(char *)NULL); 

	privilege(PRIV_DROP);

	fprintf(stderr,SETPENV);

	/* audit failure */
	exitex(SETGRAUD,errno,real,groups,PRINT);
}

/*
 *
 * FUNCTION:	backtologinenv
 *
 * DESCRIPTION:	calls setpcred and setpenv to reset the
 *		environment back to the initial login env.
 *
 * RETURNS:	no return on success, exits on error.
 *
 */

static	void
backtologinenv(int flag)
{
char	*name;			/* user name 		*/
uid_t	uid;			/* return from getuid 	*/
char	**logname;		/* login uid 		*/
char	u_name[PW_NAMELEN];	/* save name 		*/

	/* reacquire lapsed privilege */
	privilege(PRIV_ACQUIRE);

	/* get real users name */
	uid = getuid();
	if ((name = (char *)IDtouser(uid)) == NULL)
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

	/* reset process environment */
	runsetpenv(flag,(char *)NULL,(char *)NULL);
}
