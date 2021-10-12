static char sccsid[] = "@(#)57	1.16.1.6  src/bos/usr/bin/chsh/chsh.c, cmdsuser, bos411, 9428A410j 3/25/94 14:51:00";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: chkshells
 *		getshells
 *		getushell
 *		main
 *		setshell
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

#include	<stdio.h>	/* for printfs			*/
#include 	<pwd.h>		/* for passwd struct		*/
#include 	<locale.h>	/* for setlocale()		*/
#include	<sys/stat.h>	/* for stat()			*/
#include 	<langinfo.h>	/* for language information	*/
#include 	<stdlib.h>	/* for rpmatch	*/
#include 	"tcbauth.h"	/* for local defines		*/

static	char	**getshells(char *,int *);
static	void	chkshells(char *,char *,char **,char *,int);
static	char	*getushell(char *);
static	void	setshell (char *,char *,char *,char **,int);

/*
 *
 * NAME:     chsh
 *                                                                    
 * FUNCTION: changes the initial program in /etc/passwd
 *                                                                    
 * USAGE:    chsh <username> [shell]
 *	     where:
 *			username is a valid user of the system
 *			shell is a valid shell
 *                                                                   
 *	If no user name is supplied, the user corresponding to the
 * 		real user id of the current process is used.
 *	Ordinary users can only change their own shells.
 *	Security administrators can change the shells of any
 *		non-administrative user.
 * 	Ordinary users can only change their shells to one on the 
 *	 	approved list in /etc/security/login.
 * 	The user is prompted with a list of proper shells and can
 *	 	enter any shell on that list.
 * 	If there is no list the user cannot change shells.
 * 	If command is not invoked on the Trusted path, it must be
 * 		installed with the BYPASS_TPATH system privilege.
 * 	This function will invoke chuser to do the functional work.
 *
 * 	RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	if the specified shell is invalid
 *		ENOENT 	if the user argument does not exist
 *			if the user doesn't have an initial program
 *		EACCES 	if the shell cannot be read in login.cfg or in /etc/pw
 *		ENOSYS 	if there is no shell default list in login.cfg 
 *		EPERM 	if the user identification and authentication fails
 *			if the invoker doesn't have x_access to chuser
 *		errno	if exec fails.
 */  

main(int argc,char *argv[])
{
int	n;		/* number of shells			 	*/
char	*ushell;	/* the users shell			 	*/
char	*name;		/* the user to be changed		 	*/
char	**shells;	/* the array of current available shells 	*/
char	answer[BUFSIZ];	/* the input from the screen		 	*/
register int    i=0;	/* counter				 	*/
char	*yprompt;
char	*nprompt;
char	*p;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc > 3)
		usage(CHSHUSAGE);

	if (argv[1] && argv[1][0] == '-')
		usage(CHSHUSAGE);

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/* if no user given get invoker name and check for existence */
	if ((name = getusername(argc,argv[1])) == NULL)
	{
		if (argv[1])
		{
			fprintf(stderr,USRNONEX,argv[1]);
			exit (errno);
		}
		fprintf(stderr,USRINFO);
		fprintf (stderr, CHECK, PASSWD);
		exit (errno);
	}
	/*
	 * See if the invoker can change the shell -- a user can change their
	 * own shell only; root can change anyone's;group security can change
	 * only a ordinary user's shell (admin=false).
	*/

	if (!gotiaccess(name))
	{
		fprintf(stderr,CHGONERR,name);
		fprintf (stderr, PERMISSION);
		exitax(CHUSRAUD,EPERM,name,(char *)NULL,NOPRINT);
	} 

	/* get available shells */
	shells = getshells(name,&n);

	/* get user's current shell */
	ushell = getushell(name);

	/* if more than one argument it must be a new shell */
	if (argv[1] && argv[2])
	{
		if (argv[2][0] == '/')
			setshell (name,ushell,argv[2],shells,n);
		else
			usage(CHSHUSAGE);
	}

	/* print current shells */
	printf (ACURRSHELL);
	for (i=0;i<n;i++)
		printf("\t\t%s\n",shells[i]);

	/* print user's current shell */
	printf (UCURRSHELL,name);
	printf ("\t\t%s\n",ushell);

	/* prompt for response */
	yprompt = nl_langinfo(YESSTR);
	if (*yprompt == '\0')
		yprompt = "yes";
	/* Use answer to save yesstr, since next call to nl_langinfo
	   may overwrite static area. */
	strcpy(answer, yprompt);	/* Assume answer is big enough. */

	nprompt = nl_langinfo(NOSTR);
	if (*nprompt == '\0')
		nprompt = "no";

	printf (CHANGE,answer,nprompt);

	/* read response */
	fgets (answer,BUFSIZ,stdin);

	/*
	 *  Take off the ending newline
	 *  character from the fgets.
	 */

	if ((p = strrchr (answer,'\n')) != NULL)
		*p = '\0';

	if (rpmatch(answer) == 1)
	{
		/* prompt for shell */
		printf (TO);

		/* read shell */
		fgets (answer,BUFSIZ,stdin);

		/*
	 	 *  Take off the ending newline
	 	 *  character from the fgets.
	 	 */

		if ((p = strrchr (answer,'\n')) != NULL)
			*p = '\0';

		/* set it */
		setshell (name,ushell,answer,shells,n);
	}
	else 
	{
		printf (SETSHELL);
		exit (0);
	}
}

/*
 * FUNCTION:	getshells
 *
 * DESCRIPTION:	gets the available shells listed in /etc/security/login.cfg
 *
 * PASSED:	username = the user to be changed, shells = array of available
 *		shells.
 *
 * RETURNS:	Number of shells.
 *
 */

static	char	**
getshells(char *username,int *num)
{
int		n = 0;		/* number of shells			*/
char		*val;		/* the values returned from data base	*/
char		**validlist;	/* the valid shells 			*/
char		**shells;	/* the array of available shells	*/
int		i = 0;		/* counter				*/
int		c = 0;		/* counter				*/
struct stat	sbuf;           /* stat buffer                  	*/

	/* get the available user shells from the data base */
	if(getconfattr(SC_SYS_LOGIN,SC_SHELLS,(void *)&val,SEC_LIST))
	{
		fprintf(stderr,SERRGET,username);
		fprintf (stderr,CHECK,LOGINFILE);
		exit (errno);
	}

	/* turn the NULL terminated list to a comma separated string */
	shells = listoarray(val,&n);

	/* get some space for out new shell list */
	if ((validlist = (char **)malloc(n * sizeof (char *))) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}


	for (i=0; i < n; i++)
	{
		/* if shell does exist, add it to the list */
		if (!stat(shells[i], &sbuf))
		{
			validlist[c] = shells[i];
			c++;
		}

	}
	
	/* save return value */
	*num = c;

	return(validlist);
}

/*
 * FUNCTION:	chkshells
 *
 * DESCRIPTION:	checks the users shell against the available shells and
 *		checks the entered shell against the available shells.
 *
 * PASSED:	name = the user to be changed, answer = the new shell,
 *		shells = the available shells, ushell = the current shell,
 *		n = # of available shells
 *
 * RETURNS:	0 or exits.
 *
 */

static	void
chkshells(char *name,char *answer,char *shells[],char *ushell,int n)
{
register int	found = 0;	/* flag		*/
register int	i = 0;		/* counter 	*/

	/* check new shell against available shells */
	while(shells[i])
	{
		if (!strcmp(answer,shells[i]))
		{
			found++;
			break;
		}
		i++;
	}

	/* If the new shell is invalid */
	if (!found)
	{
		fprintf(stderr,ERRSET,name,answer);
		fprintf (stderr, ERBADVAL);
		exit (EINVAL);
	}

	/*
	 * The user must have a current valid shell listing in
	 * /etc/passwd to allow a change in shells, so check
	 * the user's current shells against available shells
	 */
	if (ushell && *ushell)
	{
		found = 0;
		i=0;
		while(shells[i])
		{
			if (!strcmp(ushell,shells[i]))
			{
				found++;
				break;
			}
			i++;
		}
	}
	else
		found = 1;

	/* If the user doesn't have a valid initial program/shell */
	if (!found)
	{
		fprintf(stderr,CHGTOERR,ushell,answer);
		fprintf (stderr, ERBADVAL);
		exit (EINVAL);
	}
}

/*
 * FUNCTION:	getushell
 *
 * DESCRIPTION:	gets the users shell from /etc/passwd.
 *
 * PASSED:	username = the user's name to be changed. 
 *
 * RETURNS:	The shell.
 *
 */

static	char	*
getushell(char *username)
{
struct	passwd	*pwd;		/* pointer to passwd structures	*/
register char	*curshell;	/* user's current shell */

	setpwent();
	
	if ((pwd = (struct passwd *)getpwnam(username)) == NULL)
		curshell = (char *)NULL;
	else
		curshell = pwd->pw_shell;
		
	endpwent();
	return(curshell);
}

/*
 * FUNCTION:	setshell
 *
 * DESCRIPTION:	subroutine to run "chuser shell=newshell name"
 *
 * PASSED:	name = the user's name to be changed. 
 *		ushell, the user's current shell.
 *		newshell, the new shell.
 *		shells, the list of valid shells.
 *		n, how many shells in the list.
 *
 * RETURNS:	none.
 *
 */

static void
setshell (char *name,char *ushell,char *newshell,char **shells,int n)
{
	int rc;

	/* check for valid shell */
	chkshells(name,newshell,shells,ushell,n);

	/* set user's shell with new value */
	rc = changeattr(name,SHEAD,newshell);
	enduserdb();

	if (rc)
		exit(rc);

	/* audit this success */
	exitax(CHUSRAUD,rc,name,newshell,NOPRINT);
}

