static char sccsid[] = "@(#)28	1.38  src/bos/usr/bin/su/su.c, cmdsauth, bos41J, 9522A_all 5/30/95 15:48:24";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <locale.h>		/* for setlocale() */
#include <stdlib.h>
#include <sys/errno.h>		/* for perror()	*/
#include <sys/audit.h>		/* for auditwrite() */
#include <sys/priv.h>		/* for setpriv() */
#include <sys/id.h>		/* ID_REAL for getuidx() */
#include <login.h>		/* S_PRIMARY ... */
#include <userpw.h>		/* for PW_NAMELEN */
#include <usersec.h>		/* for user attributes */
#include <syslog.h>		/* for syslogd logging */
#include <time.h>		/* for logging su attempts */
#include <pwd.h>		/* for getpwnam */
 
extern	char	*ttyname(int fildes);
extern	char 	*strrchr(char *string, char character);
extern	char	**environ;
extern 	int 	tsm_iscurr_console(char *, char *);
 
#define	DEF_SHELL	"/usr/bin/sh "
#define	SU_CONSOLE	"/dev/console"
#define TTYLENGTH	1023		/* tty length for ckuseracct */
 
/* local defines */
#include "su_msg.h"      /* for su messages */

nl_catd   catd;
static void	gettyname(char **u_tty);
static void	getcmd(char **,char *,int *,int,char **,char **);
static void	exitsu(char *u_name,int error);
static void	sulog(int success, int u_id, char *uname, char *utty, 
				 FILE *logfp);
 
 
/*
 * NAME: su
 *                                                                    
 * FUNCTION: substitute user identification temporarily
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	USAGE: su [-] [username] ["arguments"]
 *
 *	The '-' environment flag will cause the environment to be set
 *	as if you just logged in as the username. The -c option will run
 *	only the given shell command as the user and then return.
 *
 * 	If specified the arguments can be as follows:
 *	The first argument can be the environment flag or the username.
 *	The second argument can be the username or the shell command.
 *
 *                                                                   
 * RETURNS: 
 *
 */  
 
#define   MSGSTR(Num, Str) catgets(catd, MS_SU, Num, Str)
 
#define USRNONEX MSGSTR (M_USRNONEX, "User \"%s\" does not exist.\n")
#define	NOSU MSGSTR (M_NOSU, "Cannot su to \"%s\"")
#define GETLOG MSGSTR (M_GETLOG, "Cannot get \"LOGNAME\" variable")
#define SETCRED MSGSTR (M_SETCRED, "Cannot set process credentials")
#define SETPRIV MSGSTR (M_SETPRIV, "Cannot set process privilege")
#define SETPENV MSGSTR (M_SETPENV, "Cannot set process environment")
#define NOSPACE MSGSTR (M_NOSPACE, "Cannot allocate space.\n")
#define EXPIRED MSGSTR (M_EXPIRED, " : Account has expired.\n")
#define TOOLONG MSGSTR (M_TOOLONG, " : Name is too long.\n")
#define NOWAY MSGSTR (M_NOWAY, " : Account is not accessible.\n")
#define NOPERM MSGSTR (M_NOPERM, " : No permission.\n")
#define NOAUTH MSGSTR (M_NOAUTH, " : Authentication is denied.\n")

#define SULOG 0			/* log only when su to root */
#define LOGFILE "/var/adm/sulog"	/* log file for SYSV compatibility */
 
main(int argc, char **argv)
{
int 	eflag = PENV_DELTA;	/* environment flag */
int	rc;			/* return code */
int	login_rest_errno;	/* errno from loginrestrictions		*/
char 	u_name[PW_NAMELEN];	/* User's name.  Normal PW_NAMELEN long */
char 	*fullname;		/* User's name.  May contain DCE cellname */
char 	*u_tty;			/* Terminal name */
char	*cmd = '\0';		/* command to execute */
char	**logname;		/* process's login user name */
char    pseudoname[TTYLENGTH];	/* the tty name and console name if appl */
int	cur_u_id;		/* Current login user name */
FILE 	*logfp;			/* SYSV sulog file */
char	*opasswd = NULL;
char	*msg = NULL;
extern	int login_restricted;	/* need to set this, for use by sysauth */
 
	(void ) setlocale(LC_ALL,"");
	catd = catopen (MF_SU, NL_CAT_LOCALE);
 
	openlog("su", LOG_ODELAY, LOG_AUTH); 	/* sylogd logging */

	if ((logfp = fopen (LOGFILE, "a")) != NULL)   /* SYSV sulog file */
	{
		chmod (LOGFILE, 0600);
		chown (LOGFILE, 0, 0);
	}		/* if can't open the log file, go ahead and su */

	gettyname(&u_tty);
 
	/* getusername and any other args */
	getcmd(&cmd,u_name,&eflag,argc,argv,&fullname);
	
	/* if user is root they can su to anyone */
	if (cur_u_id = getuid())
	{

		/* check to see if this is the console and add to list */
		/* because we need to check if the user can use the console */
		bzero(pseudoname,TTYLENGTH);
		if ((tsm_iscurr_console (u_tty , pseudoname)) == 0) 
			sprintf(pseudoname,"%s,%s",SU_CONSOLE,u_tty);
		else
			sprintf(pseudoname,"%s",u_tty);

		/* Call loginrestrictions, but don't fail yet if the user
		 * is restricted.  Instead, pass this information along to
		 * sysauth (via the login_restricted variable).  Standard
		 * sysauth need to check loginrestrictions before calling
		 * passwdexpired (tsm_chpass) for a possible password change.
		 */
		
		rc = loginrestrictions(fullname, S_SU, pseudoname, &msg);
		login_restricted = (rc) ? 1 : 0;
		login_rest_errno = errno;

		/* Need to call enduserdb() here because loginrestrictions
		 * is careless.  If this isn't done, subsequent getuserattr()
		 * calls may return cached information based on the value of 
		 * AUTHSTATE before authentication, instead of after.  This
		 * could lead to local info being returned instead of DCE
		 * (or vice versa).
		 */

		enduserdb();

		if(tsm_authenticate(fullname, S_PRIMARY, &opasswd))
		{
			rc = errno;
			if (opasswd && *opasswd)
				memset(opasswd, 0, strlen(opasswd));
			/* log ALL su's */ 
			sulog(0, cur_u_id, u_name, u_tty, logfp);
			closelog();
			fclose (logfp);
			fprintf(stderr,NOSU,u_name);
			exitsu(u_name,rc);
		}

		/* If the user authentication succeeded, only now we will
		 * let them see any messages from loginrestrictions.
		 */

		if(msg)
		{
			puts(msg);
			free(msg);
			msg = NULL;
		}

		if(login_restricted)
		{
			rc = login_rest_errno;
			if (opasswd && *opasswd)
				memset(opasswd, 0, strlen(opasswd));
			/* log ALL su's */ 
			sulog(0, cur_u_id, u_name, u_tty, logfp);
			closelog();
			fclose (logfp);
			fprintf(stderr,NOSU,u_name);
			exitsu(u_name,rc);
		}

	}
	else
	{
		/*  Only do this section if we are root.	*/

		char krb_blank[12] = "KRB5CCNAME=";
		char *krb_old = NULL;
		char *krb_new = NULL;
		int  env_rc = 0;

		/* Clear out KRB5CCNAME variable to make sure it's not inherited
		 * from any previous process or authentication attempt.  Usually
		 * taken care of in tsm_authenticate, but it is not called when
		 * su is executed as root.
		 */

		krb_old = getenv("KRB5CCNAME");
		if (krb_old)
		{
			krb_new = strdup(krb_blank);
			if (krb_new)
			{
				env_rc = putenv(krb_new);
				if (env_rc)
					free(krb_new);
			}
		}
	}

	/* get the login user name from cred */
	if ((logname = getpcred(CRED_LUID)) == NULL)
	{
		rc = errno;
		sulog(0, cur_u_id, u_name, u_tty, logfp); /*sylogd logging */
		closelog();
		fclose (logfp);
		fprintf(stderr,GETLOG);
		exitsu(u_name,rc);
	}
 
	/* set process credentials: uid,gid,groupset etc. */
	if (setpcred (fullname, logname))
	{
		rc = errno;
		sulog(0, cur_u_id, u_name, u_tty, logfp); /*sylogd logging */
		closelog();
		fclose (logfp);
		fprintf(stderr,SETCRED);
		exitsu(u_name,rc);
	}
 
	/* drop bequeathed privilege */
	if (beqpriv())
	{
		rc = errno;
		sulog(0, cur_u_id, u_name, u_tty, logfp); /*sylogd logging */
		closelog();
		fclose (logfp);
		fprintf(stderr,SETPRIV);
		exitsu(u_name,rc);
	}
 
	/* audit success */
	auditwrite ("USER_SU",AUDIT_OK,u_name,strlen(u_name) + 1,NULL);
 
	rc = errno;
	sulog(1, cur_u_id, u_name, u_tty, logfp); /* successful su */
	closelog();
	fclose (logfp);

	/* setpenv will never return if successful */
	setpenv(fullname,eflag,(char **)NULL,cmd);
 
	fprintf(stderr,SETPENV);
	exitsu(u_name,rc);
}
 
/*
 * NAME: sulog()
 *
 * FUNCTIONS: 1)  log by syslogd.
 *                if SULOG defined then log when su to ANYONE.
 *                if SULOG not defined then log only when su to root.
 *	      2)  log all su attempts in /var/adm/sulog (for SYSV
 *			   compatibility).
 *
 * RETURN: none.
 *
 */
static void
sulog(int success, int u_id, char *uname, char *utty, FILE *logfp)
{
	long tbuf;
	struct tm *current;

/* log by syslogd first */

#ifdef SULOG
	if (success)
		syslog(LOG_NOTICE, "from %s to %s at %s \
\n", IDtouser(u_id), uname ,utty);
	else	
		syslog(LOG_CRIT, "BAD SU from %s to %s at %s \
\n", IDtouser(u_id), uname ,utty);
#else
	if (strcmp(uname,"root") == 0)
		if (success)
			syslog(LOG_NOTICE, "from %s to %s at %s \
\n", IDtouser(u_id), uname ,utty);
		else	
			syslog(LOG_CRIT, "BAD SU from %s to %s at %s \
\n", IDtouser(u_id), uname ,utty);
#endif

/* log by /var/adm/sulog for SYSV compatibility */

	if (logfp)
	{
		time (&tbuf);
		current = localtime (&tbuf);

		fprintf(logfp, "SU %.2d/%.2d %.2d:%.2d %c %s %s-%s\n",
	   	current->tm_mon+1,current->tm_mday,current->tm_hour,
		current->tm_min,success?'+':'-',(strchr(utty+1,'/')+1),
		IDtouser(u_id),uname);
	}

	return;
}	
 
/*
 * NAME: gettyname()
 *
 * FUNCTION: get terminal name.
 *
 * RETURN: no return.
 *
 */
 
static void
gettyname(char **u_tty)
{
	/* Get terminal name */
	if(!((*u_tty=ttyname(0))||(*u_tty=ttyname(1))||(*u_tty = ttyname(2))))
		*u_tty = "/dev/tty??";
}
 
/*
 * NAME: getcmd()
 *
 * FUNCTION: parse command if any from command line.
 *
 * RETURN: 0 if ok, -1 if error.
 * PASSED:
 * 	char	**cmd;		 place to store command string
 * 	char	*u_name;	 user's name
 * 	int	*eflag;		 '-' (to indicate init flag to setpenv() )
 * 	int	argc;		 argc from command line
 * 	char	*argv[];	 argv from command line
 *
 */
 
static void
getcmd(char **cmd,char *u_name,int *eflag,int argc,char **argv,char **fullname)
{
char	*shell;		/* the user's shell when exec'ing a command */
register int	i;	/* counter */
register int	siz;	/* size of buffer to malloc */
struct	passwd *pw;     /* buffer returned from getpwnam */
 
	/* check for "-" */
	if ((strlen(argv[1]) == 1) && (argv[1][0] == '-'))
	{
		*eflag = PENV_INIT | PENV_KLEEN; /* exec with flag ("-") */
		argc--;
		argv++;
	}
 
	/* next arg must be username */
	if (argv[1])
	{
		/* if next argument is "-" assume root user name */
		if (argv[1][0] == '-')
		{
			*fullname = "root";
			strcpy(u_name, *fullname);
			argc++;
			argv--;
		}
		else
		{
		      *fullname = argv[1];
		      _truncate_username(*fullname);
		      _normalize_username(u_name, *fullname);
			
		      if ((pw = getpwnam(*fullname)) == NULL)
		      {
				fprintf (stderr,USRNONEX,*fullname);
				catclose (catd);
				exit (errno);
		      }
		}
	}
	/* if no name specified assume root */
	else
	{
		*fullname = "root";
		strcpy(u_name, *fullname);
		return;
	}
 
	/* next arg must be command */
	if (argv[2])
	{
		/* get user shell from /etc/passwd */

		if ( (pw->pw_shell) && (*(pw->pw_shell)) )
			shell = pw->pw_shell;
		else
			shell = DEF_SHELL;

 
		siz = strlen(shell) + strlen(argv[2]) + 2;
 
		if((*cmd = (char *)malloc((size_t)siz)) == NULL)
		{
			fprintf (stderr,NOSPACE);
			catclose (catd);
			exit (errno);
		}
 
		sprintf (*cmd,"%s %s",shell,argv[2]);
 
		/* get the rest of the command from argv[] */
		for (i=3;i<=(argc-1);i++)
		{
			siz = strlen(*cmd) + strlen(argv[i]) + 4;
			if ((*cmd = realloc((void*)*cmd,(size_t)siz)) == NULL)
			{
				fprintf (stderr,NOSPACE);
				catclose (catd);
				exit (errno);
			}
			strcat(*cmd, " ");
			strcat(*cmd, argv[i]);
		}
	}
	return;
}
 
/*
 * NAME: beqpriv()
 *
 * FUNCTION: drop bequeathed privilege.
 *
 * RETURN: return from setpriv().
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
 * NAME: exitsu
 *
 * FUNCTION: Audit the result of "su", print error, and exit with errno.
 *
 * RETURN VALUE DESCRIPTIONS: NONE.
 *
 */
 
static void
exitsu(char *u_name,int error)
{
 
	switch (error)
	{
		case	ENOENT : fprintf (stderr,".\n");
				break;
 
		case	ENOATTR : fprintf (stderr,".\n");
				break;
 
		case	ESTALE : fprintf (stderr,EXPIRED);
				break;
 
		case	ENAMETOOLONG : fprintf (stderr,TOOLONG);
				break;
 
		case	EINVAL : fprintf (stderr,".\n");
				break;
 
		case	EACCES : fprintf (stderr,NOWAY);
				break;
 
		case	EPERM : fprintf (stderr,NOPERM);
				break;
 
		case	ESAD : fprintf (stderr,NOAUTH);
				break;
 
		default : fprintf (stderr,".\n");
	}
 
	fflush(stderr);
 
	auditwrite ("USER_SU",AUDIT_FAIL,u_name,strlen(u_name) + 1,NULL);
	catclose (catd);
	exit(error);
}

/*
 * NAME:     readpasswd
 *
 * FUNCTION: Get a password from the user.
 *
 * RETURN:   NONE
 *
 */
void
readpasswd(char *message, char *passwd)
{
	char *tp;

	if((tp = getpass(message)) != NULL)
	{
		strncpy(passwd, tp, MAX_PASS);
		passwd[MAX_PASS] = '\0';
	}
	else
		passwd[0] = '\0';
}
 
