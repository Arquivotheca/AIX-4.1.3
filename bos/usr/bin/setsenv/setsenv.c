static char sccsid[] = "@(#)15	1.12.1.4  src/bos/usr/bin/setsenv/setsenv.c, cmdsuser, bos411, 9428A410j 4/4/94 12:49:13";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: getauditstr
 *		listenv
 *		main
 *		resenv
 *		set
 *		setup_env_vec
 *		validate_argv
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
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for setpriv		*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<sys/id.h>	/* for ID_EFFECTIVE 	*/
#include	"tcbauth.h"	/* for getuserattr	*/


/* local functions */
static	void	set(int flag,int argc,char **argv);
static	void	resenv(int flag,char **envarg,char *auditstr);
static	void	validate_argv(char **argv);
static	char	**setup_env_vec(int argc,char **argv,char **auditstr);
static	void	getauditstr(int argcount,char **auditstr,char **env);
static	void	listenv(void);

/*
 * COMMAND:	setsenv: setsenv [-] [envvar=value ...]
 *
 * DESCRIPTION:	sets or resets the current user's environment.
 *
 */

main(int argc,char *argv[])
{

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* suspend auditing for this process */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	privilege(PRIV_LAPSE);

	/* check for execution at a terminal */
	if (checkfortty())
	{
		fprintf(stderr,TERMINAL,SETSENV);
		exitex(SETUAUD,ENOTTY,NULL,NULL,NOPRINT);
	}

	if(argc > 1)
	{
		/*
		 * validate the user-specified environment.
		 * LOGNAME, NAME, and TTY are NOT allowed
		 * call exits on error.
		 */
		validate_argv(argv);
	}
		
	/*
	 * If no arguments, then list the protected env.
	 * If '-' is passed, reset the env to the original
	 * login env, otherwise, reset the env to the
	 * data in the user data base.  set() does not return. 
	 */

	if (argv[1])
	{
		if (argv[1][0] == '-')
		{
			if (strlen(argv[1]) != 1)
				xusage(SETSENUSAGE,SETUAUD,NULL);
			else
				set(1,--argc,++argv);
		}
		else
			set(0,--argc,++argv);

	}
	else
		listenv();

}

/*
 * FUNCTION:	set
 *
 * DESCRIPTION:	Set the new environment and spawn a shell.
 *		sets up the new environment vector by calling setup_env_vec().
 *		It then calls setpenv() to set the new environment.
 *		If the set fails a shell is spawned anyway.
 *
 * PASSED:	flag = INIT or RESET, argc = # of attributes entered, argv =
 *		array of attributes.
 *
 * RETURNS:	No return.
 *
 */

static	void
set(int flag,int argc,char **argv)
{
char	**envarg;	/* environment argument to setpenv()*/
char	*auditstr;	/* place to put auditing string     */

	/* drop the '-' */
	if (flag) 
	{
		argc--;
		argv++;
	}

	/* if there are no other arguments, don't bother with setup */
	if (argc == 0)
	{
		envarg = NULL;
		auditstr = NULL;
	}
	else
		envarg = setup_env_vec(argc,argv,&auditstr);

	resenv(flag,envarg,auditstr);
}

/*
 * FUNCTION:	resenv
 *
 * DESCRIPTION:	Set the new environment and spawn a shell.
 *
 * RETURNS:	No return.
 *
 */

static	void
resenv(int flag,char **envarg,char *auditstr)
{
int	val = PENV_DELTA;	/* flag to setpenv */

	/* if setting environment to initial login env */
	if (flag)
		val = PENV_INIT | PENV_KLEEN;

	/* audit success */
	privilege(PRIV_ACQUIRE);

	auditwrite(SETUAUD,0,auditstr,strlen(auditstr) + 1,0);

	/* drop bequeathed privilege */
	if (beqpriv())
	{
		fprintf(stderr,SETPRIV);
		exitex(SETUAUD,errno,NULL,NULL,PRINT);
	}

	setuidx(ID_EFFECTIVE,getuid());

	/* set process environment should not return */
	setpenv((char *)NULL,val,envarg,"$SHELL"); 

	privilege(PRIV_DROP);

	fprintf(stderr,SETPENV);

	/* audit fail */
	exitex(SETUAUD,errno,(char *)NULL,(char *)NULL,PRINT);
}

/*
 *
 * FUNCTION:	listenv
 *
 * DESCRIPTION:	gets the current environment variables and prints them.
 *
 * RETURNS:	None.
 *
 */

static	void
listenv(void)
{
char	**env;

	privilege(PRIV_ACQUIRE);

	if ((env = getpenv(PENV_SYS)) == NULL) 
	{
		fprintf(stderr,GETPENV);
		exitex(SETUAUD,errno,NULL,NULL,PRINT);
	}

	privilege(PRIV_LAPSE);

	while(*env)
		printf("%s\n",*env++);
	
	resenv(0,(char **)NULL,(char *)NULL);

}


/*
 *
 * FUNCTION:	validate_argv
 *
 * DESCRIPTION:	validates the user-specified environment. Makes sure that
 *		user did not specify a bad attr=value string, and also
 *		make sure user is NOT trying to set TTY, NAME, or LOGNAME
 *		environment variables.
 *
 * PASSED: 	argv = args from command line.
 *
 * RETURNS:	None.
 *
 */

static	void
validate_argv(char **argv)
{
char	*ptr;
char	*val;

	if ((argv[1][0]=='-') && (argv[1][1]=='\0'))
		argv++;
	argv++;

	while(*argv)
	{
		ptr = *argv;
		val = ptr;

		/* check for at least an = on the command line argument */
		while ((*ptr != '=') && (*ptr != '\0'))
			ptr++;
			
		/* if we get a NULL before a '=' */
		if (*ptr != '=')
		{
			xusage(SETSENUSAGE,SETUAUD,NULL);
		}
		*ptr = '\0';

		if (!strcmp(val,"TTY") || !strcmp(val,"NAME") ||
		    !strcmp(val,"LOGNAME"))
		{
			fprintf(stderr,CHGONERR,*argv);
			exitex(SETUAUD,EPERM,NULL,NULL,PRINT);
		}
		/* restore the '=' */
		*ptr = '=';
		argv++;
	}
}

			
/*
 *
 * FUNCTION:	setup_env_vec
 *
 * DESCRIPTION:	set up the environment vector to pass to setpenv().
 *		parses the command line input of the form "attr=value",
 *		and builds the new environment vector from these values
 *		and current system environment variables TTY,NAME and LOGIN.
 *
 * PASSED:	argc = # of args, argv = array of args, auditstr = the string
 *		of new env to audit.
 *
 * RETURNS:	pointer to new environment vector or NULL if error.
 *
 */

static	char	**
setup_env_vec(int argc,char **argv,char **auditstr)
{
char	**env;		/* the place to store the environment args */
int	argcount;

	/* allocate memory for the environment array, it should look like: */
	/* PENV_SYS + argv[..] +  NULL  */
	if ((env = (char **)malloc(sizeof(char *) * (argc+2))) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETUAUD,errno,NULL,NULL,PRINT);
	}

	/* add the PENV_SYS string			*/
	/* allocate space for the PENV_SYS string 	*/
	if ((env[0] = (char *)malloc (strlen(PENV_SYSSTR)+1)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitex(SETUAUD,errno,NULL,NULL,PRINT);
	}
	strcpy(env[0],PENV_SYSSTR);

	/* store the user supplied environment variables */
	for (argcount=1; argcount<=argc; argcount++)
	{
		/* allocate space to hold the given string plus NULL */
		if ((env[argcount] = (char *)malloc (strlen(*argv)+1)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitex(SETUAUD,errno,NULL,NULL,PRINT);
		}

		/* add the environment string to the penv array */
		strcpy(env[argcount],*argv++);
	}

	/* terminate the environment vector with a NULL */
	env[argcount] = (char *)NULL;

	getauditstr(argcount,auditstr,env);

	return(env);
}


/*
 *
 * FUNCTION:	getauditstr
 *
 * DESCRIPTION:	get space for and copy in auditing string to "auditstr".
 *
 * RETURNS:	No return.
 *
 */

static	void
getauditstr(int argcount,char **auditstr,char **env)
{
int	i;
int	siz;

	for (i=0; i<argcount; i++)
	{
		if (i == 0)
		{
			if ((*auditstr = malloc(strlen(env[i]) + 1)) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitex(SETUAUD,errno,NULL,NULL,PRINT);
			}
			strcpy(*auditstr,env[i]);
		}
		else
		{
			/* allow for blank */
			siz = strlen(*auditstr) + strlen(env[i]) + 2;
			if ((*auditstr = realloc(*auditstr,siz)) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitex(SETUAUD,errno,*auditstr,NULL,PRINT);
			}
			strcat(*auditstr, " ");
			strcat(*auditstr, env[i]);
		}
	}
}
