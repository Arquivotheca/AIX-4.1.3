static char sccsid[] = "@(#)71	1.20.1.9  src/bos/usr/bin/pwdadm/pwdadm.c, cmdsuser, bos411, 9438C411a 9/23/94 15:21:09";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: execute
 *		getname
 *		main
 *		printattrs
 *		setflag
 *		setpasswd
 *		setpwflags
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

#include	<stdio.h>	/* for printfs		*/
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for chkpriv		*/
#include	<sys/signal.h>	/* for signal		*/
#include	<sys/stat.h>	/* for signal		*/
#include	<fcntl.h>	/* for read and write 	*/
#include	<userpw.h>	/* for passwd struct	*/
#include 	<locale.h>	/* for setlocale() 	*/
#include	"tcbauth.h"	/* for local defines	*/

static	void	getname(int,char **,char *);
static	void	setflag(char *,char *,char *);
static	void	setpasswd(char *);
static	void	printattrs(char *);
static	void	setpwflags(char *,char *);
static	int	execute(char **);

static	int	clear_flags = 0;
/*
 * NAME:     pwdadm
 *
 * FUNCTION: Establishes or Changes your login password
 *
 * USAGE:    pwdadm [-f flags | -q | -c] username
 *	     where:
 *		username:  specifies the user whose password is to be changed.
 *		-f flags:  specifies the flags attribute of the password
 *			   (NOCHECK, ADMIN, &/or ADMCHG)
 *		-c      :  clears the flags attribute of the password
 *		-q      :  specifies that the password status of the named
 *			   user is to be returned (flags= & lastupdate= )	
 *
 * RETURNS:  0 if successful, otherwise one of the following is returned:
 *		
 *		ESAD 	if the security authentication is denied for the 
 *			administrator
 *		EACCES	if the command can't write info into the user database
 *		ENOENT	if the user does not exist.
 *		EPERM	if the ADMIN flag is specified and invoker != root
 *			if the user is an administrative user & invoker != root
 */

main(int argc,char *argv[])
{
char	uname[PW_NAMELEN]; 	/* specified user name	*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* Suspend auditing for this process - This process is trusted */
	auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);  

	/* open data bases for read and write */
	setpwdb (S_READ|S_WRITE);
	setuserdb (S_READ|S_WRITE);

	/* getusername from command line */
	getname(argc,argv,uname);

	/* If the user is administrative & the invoker != root, exit */
	if (!gotuaccess(uname))
	{
		fprintf(stderr,CHGONERR,uname);
		exitax(PWDCHGAUD,EPERM,uname,NULL,PRINT);
 	}
		
	/* If a flag is specified */
	if ((strlen(argv[1]) == 2) && argv[1][0] == '-')
		setflag(argv[1],argv[2],uname);

	/* If no flags are specified, set the password */
	else 
		setpasswd(uname);
}

/*
 * FUNCTION:	getname
 *
 * DESCRIPTION:	gets user's name and parses the command line arguments.
 *
 * RETURNS:	No return.
 *
 */

static	void
getname(int argc,char *argv[],char *name)
{

int	id;

	/*
	 * check for invalid flags
	 */

	if ((argc > 2) && ((argv[1][0] != '-'))) 
		usage(PWADMUSAGE);

	if ((argv[1][0] == '-') && (strlen(argv[1]) != 2))
		usage(PWADMUSAGE);

	if (!strcmp (argv[1],"-f") && argc != 4)
		usage(PWADMUSAGE);

	if (!strcmp (argv[1],"-c") && argc != 3)
		usage(PWADMUSAGE);

	if (!strcmp (argv[1],"-q") && argc != 3)
		usage(PWADMUSAGE);

	/* Get the username */
	if ((argc > 1) && (argv[argc - 1][0] != '-'))
	{
		if (strlen(argv[argc - 1]) > PW_NAMELEN - 1)
		{
			fprintf(stderr,CHGONERR,argv[argc - 1]);
			fprintf(stderr,TOOLONG);
			exit(ENAMETOOLONG);
		}
		else
		{
			strcpy(name,argv[argc - 1]);
			/* Check if the user exists */
			if (getuserattr(name,S_ID,&id,SEC_INT))
			{
				fprintf(stderr,USRNONEX,name);
				exit(ENOENT);
			}
		}
	}
	/* if no name specified print usage */
	else 
		usage(PWADMUSAGE);
}

/*
 * FUNCTION:	setflag
 *
 * DESCRIPTION:	sets the user's passwd flags
 *
 * RETURNS:	No return.
 *
 */

static	void
setflag(char *option,char *flags,char *uname)
{
	switch(option[1])
	{
		/* Query the password information */
		case 'q' :	
			printattrs(uname);

		/* Set the password flags */
		case 'c' :	
			clear_flags = 1;
			/* fall through */
		case 'f' :	
			setpwflags(flags,uname);

		/* Usage error */
		default  :	usage(PWADMUSAGE);

	}
}

/*
 * FUNCTION:	setpasswd
 *
 * DESCRIPTION:	Resets the user's passwd
 *
 * PASSED:	uname = the user whose passwd will be reset
 *
 * RETURNS:	No return.
 *
 */

static	void
setpasswd(char *uname)
{
char		*npass; 	/* new password 			  */
struct userpw  	*pw;		/* pointer to the user's passwd structure */
struct userpw  	newpw;		/* passwd structure if getuserpw fails	  */
char            *msg = (char *)NULL;
DBM		*pwdbm;

	/* open database dbm files */
	pwdbm = pwdbm_open();

	/*
	 * Get user's passwd structure.  This pointer is static
	 * to getuserpw and might be overwritten.
	 */
	if ((pw = getuserpw(uname)) != NULL)
	{
		strcpy(newpw.upw_name,pw->upw_name);
		if ((newpw.upw_passwd = malloc(strlen(pw->upw_passwd)+1))==NULL)
		{
			endpwdb();
			enduserdb();
			fprintf(stderr,MALLOC);
			exitx (errno);
		}
			
		strcpy(newpw.upw_passwd,pw->upw_passwd);
		newpw.upw_flags = pw->upw_flags;
	}
	else
	{
		/* initialize a new passwd struct */
		if(errno == ENOENT)
		{
			/* initialize new userpw struct */
			strcpy(newpw.upw_name,uname);
			newpw.upw_passwd = NULL;
			newpw.upw_flags = 0;
			newpw.upw_lastupdate = 0;
		}
		/* If getting passwd entry failed, exit */
		else
		{
			endpwdb();
			enduserdb();
			fprintf(stderr,CHGONERR,uname);
			exitax(PWDCHGAUD,EACCES,uname,NULL,PRINT);
		}

	}

	/* catch signal */
	signal (SIGINT, (void(*)(void)) onint);

	/*
	 * If the passwd comes over the net, the proper method of changing
	 * the password is via chpass().  However, pwdadm implements
	 * a number of local only things such as password administrators
	 * which prohibits use of this interface.  If we encounter a remote 
	 * user then we should be using the passwd command which can properly 
	 * handle all of these users.
	 */
	if (ispassrmtnam(uname))
	{
		char    *args[3];
		int	rc;

		endpwdb();
		enduserdb();
		args[0] = "/usr/bin/passwd";
        	args[1] = uname;
        	args[2] = NULL;
        	rc = execute(args);

		/*  Could not fork or exec failed. */
	        if ((rc == -1) || (rc == 127))
        	{
                	perror("pwdadm");
			exit(errno);
        	}

		/* Other failure, such as password mismatch */
        	if (rc)	exit(rc);

		return;
	}

	/* Otherwise implement the local case */
	printf (CHGPASS,uname);

	/* set admin flag */
	newpw.upw_flags |= PW_ADMCHG;

	/* get a new passwd */
	if ((npass = (char *)newpass (&newpw)) == NULL)
	{
		endpwdb();
		enduserdb();
		if (errno == ESAD)
		{
			fprintf(stderr,NOAUTH,uname);
			xaudit(PWDCHGAUD,ESAD,uname,(char *)NULL);
		}
		fprintf(stderr,ERCHGPASS,uname);
		exitax(PWDCHGAUD,errno,uname,NULL,PRINT);
	}

	signal (SIGINT, SIG_IGN);
	
	/* Make changes on the /etc/security/passwd file */
	newpw.upw_passwd = npass;
	newpw.upw_lastupdate = time ((long *) 0);
	newpw.upw_flags |= PW_ADMCHG;

	/* if password is NULL inform user */
	if (newpw.upw_passwd[0] == '\0')
	{
		printf(SETPASS,uname);
	}

	/* update /etc/security/passwd */
	if (putuserpwhist (&newpw, &msg))
	{
		endpwdb();
		enduserdb();
		fprintf(stderr,CHGONERR,uname);
		exitax(PWDCHGAUD,errno,uname,NULL,PRINT);
	}
	else
	{
		if (msg)
		{
			fputs(msg, stderr);
			free(msg);
			msg = (char *)NULL;
		}
	}


	/* 
	 * Change "*" to "!" in the /etc/passwd file,  but not if he's a 
	 * yp user. This case is particularly for the '+user' yp entry 
	 * where the passwd is local but the other data is remote.
	 */
	if (!isuserrmtnam(uname))
		if (updpasswd(uname))
		{
			fprintf(stderr,CHGONERR,uname);
			exitax(PWDCHGAUD,errno,uname,NULL,PRINT);
		}

	enduserdb();
	endpwdb();

	if (pwdbm != NULL)
	{
		if (pwdbm_add(pwdbm, uname))
			fprintf (stderr, DBM_ADD_FAIL);
		(void) dbm_close(pwdbm);
	}

	exitax(PWDCHGAUD,0,uname,NULL,NOPRINT);
}

/*
 * FUNCTION:	printattrs
 *
 * DESCRIPTION:	Queries the password information from /etc/security/passwd
 *
 * PASSED:	user = the user whose passwd info to query
 *
 * RETURNS:	No return.
 *
 */

static	void
printattrs(char *user)
{
int	val1;		/* The lastupdate value */
char	*val2;		/* The flags value 	*/

	printf("%s: \n",user);

	/* Get the password info - lastupdate & flags */
	if (!getconfattr(user,SC_LASTUPDATE,&val1,SEC_INT))
		printf("\t%s = %d\n",SC_LASTUPDATE,val1);

	if (!getconfattr(user,SC_FLAGS,&val2,SEC_INT))
		printf("\t%s = %s\n",SC_FLAGS,listocom(val2));

	printf("\n");

	/* Close the user & pw databases */
	enduserdb();
	endpwdb();

	/* Exit successfully */
	exit(0);
}

/*
 * FUNCTION:	setpwflags
 *
 * DESCRIPTION:	Sets the password flags for the specified user
 *
 * PASSED:	flags = the new flags to set, user = the user whose passwd flags
 *		are to be set.
 *
 * RETURNS:	No return.
 *
 */

static	void
setpwflags(char *flags,char *user)
{
struct	userpw	*pw;	/* Pointer to the user's current passwd structure */
char		*val;	/* A temp flags value 				  */
struct	userpw	npw;	/* in case there is no entry 			  */
DBM		*pwdbm;

	/* open database dbm files */
	pwdbm = pwdbm_open();


	/* get user's current passwd struct */
	if ((pw = getuserpw(user)) == NULL)
	{
		/* if entry doesn't exist create it */
		if (errno != ENOENT)
		{
			fprintf(stderr,CHGONERR,user);
			exitax(PWDFLGAUD,EPERM,user,flags,PRINT);
		}
		pw = &npw;
		strcpy(pw->upw_name,user);
		if ((pw->upw_passwd = malloc(2)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx (errno);
		}
		strcpy(pw->upw_passwd,STAR);
		pw->upw_lastupdate = time ((long *) 0);

	}

	pw->upw_flags = 0;

	if (!clear_flags)
	{
		val = flags = dropblanks(flags);

		while (*flags)
		{
			if (*flags == ',')
			{
				*flags++ = '\0';
				/*
				 * Check each flag
				 */
 				if (!strcmp(val,ADMCHG))
 					pw->upw_flags |= PW_ADMCHG;
 				else if (!strcmp(val,_ADMIN))
 				{
 					if(!getuid())
 						pw->upw_flags |= PW_ADMIN;
 					else
 					{
 			      			fprintf(stderr,CHGTOERR,FLAGS,
								ADMINFLAGS);
 			      			exitax(PWDFLGAUD,EPERM,user,
								val,PRINT);
 					}
 				}
 				else if (!strcmp(val,NOCHECK))
 					pw->upw_flags |= PW_NOCHECK;
 				else
 				{
 					fprintf(stderr,CHGTOERR,FLAGS,val);
 					exitax(PWDFLGAUD,EINVAL,user,val,PRINT);
 				}

				val = flags;
			}
			else
				flags++;
		}
		flags++;
		*flags = '\0';

 		if (!strcmp(val,ADMCHG))
 			pw->upw_flags |= PW_ADMCHG;
 		else if (!strcmp(val,_ADMIN))
 		{
 			if(!getuid())
 				pw->upw_flags |= PW_ADMIN;
 			else
 			{
 			      	fprintf(stderr,CHGTOERR,FLAGS,ADMINFLAGS);
 			      	exitax(PWDFLGAUD,EPERM,user,val,PRINT);
 			}
 		}
 		else if (!strcmp(val,NOCHECK))
 			pw->upw_flags |= PW_NOCHECK;
 		else
 		{
 			fprintf(stderr,CHGTOERR,FLAGS,val);
 			exitax(PWDFLGAUD,EINVAL,user,val,PRINT);
 		}
	}

	/* Update the passwd database w/ the new flags */
        /* Since it is only a change to the flags, the password history */
        /* list should not be updated, so the routine used is putuserpw. */

	if(putuserpw(pw))
	{
		fprintf(stderr,CHGONERR,user);
		exitax(PWDFLGAUD,EACCES,user,flags,PRINT);
	}	

	/* change "*" to "!" in the /etc/passwd file */
	if (updpasswd(user))
	{
		fprintf(stderr,CHGONERR,user);
		exitax(PWDCHGAUD,errno,user,NULL,PRINT);
	}

	enduserdb();
	endpwdb();

	if (pwdbm != NULL)
	{
		(void) pwdbm_add(pwdbm, user);
			fprintf (stderr, DBM_ADD_FAIL);
		(void) dbm_close(pwdbm);
	}

	/* Exit successfully */
	exitax(PWDFLGAUD,0,user,flags,NOPRINT);
}

/*
 * NAME: 	execute
 * 
 * FUNCTION: 	Used for invoking passwd command.  
 * 
 * RETURNS:  	Return code from passwd command.
 */
static int
execute(char **args)
{
int     pid;
int     status;

	if ((pid = fork()) == 0)        /* child */
	{
		execv(args[0], args);
		exit(EACCES);
	}

	if (pid == -1)
	return -1;

	while (waitpid(pid, &status, 0) == -1)
		if (errno != EINTR)
			break;

	return(status);
}
