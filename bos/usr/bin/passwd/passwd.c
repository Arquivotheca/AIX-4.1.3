static char sccsid[] = "@(#)24	1.29.1.4  src/bos/usr/bin/passwd/passwd.c, cmdsuser, bos411, 9428A410j 12/9/93 19:28:15";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: getuname
 *		main
 *		runprog
 *		setpass
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
 
#include <sys/audit.h>		/* writes audit records	*/
#include <locale.h>		/* for setlocale () 	*/
#include <sys/stat.h>		/* for setlocale () 	*/
#include <fcntl.h>		/* for setlocale () 	*/
#include <userpw.h>		/* for PW_NAMELEN	*/
#include "tcbauth.h"		/* for local defines 	*/

static	void	setpass(char *);
static	void	getuname(char **,int argc,char **);
static	void	runprog(char *,char *);

/*
 * NAME:     passwd
 *
 * FUNCTION: Establishes or Changes your login password
 *
 * USAGE:    passwd [ -f | -s ] [username]
 *	     where:
 *			-f changes your finger information
 *			-s changes your login shell
 *
 *	if no user name is supplied the invoker's passwd is changed.
 *
 *	The -f and -s parameters will invoke chfn (change 'finger information')
 *	and chsh (change the user's login shell) respectively. 'chfn' and 
 *	'chsh' are different programs.
 *
 */

main (int argc, char **argv)
{
char		*uname; 	/* specified user name */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* suspend auditing for this process - This process is trusted */
	auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);  

	/* parse user name from command line */
	getuname(&uname,argc,argv);

	/* set user's passwd */
	setpass(uname);

	/* write audit record showing success */
	exitax(PWDCHGAUD,0,uname,NULL,NOPRINT);
}

/*
 * NAME: 	setpass()
 *                                                                    
 * FUNCTION: 	performs all the password setting functionality.
 *                                                                    
 * PASSED: 	uname = the user's name
 *
 * RETURNS: 	none
 */  

static void
setpass(char *uname)
{
	int     rc;                      /* return code                      */
	int	loop = 0;		 /* loop counter		     */
	int     reenter;                 /* recall chpass()                  */
	char    *tp      = (char *)NULL; /* temporary pointer                */
	char	*op	 = (char *)NULL; /* Pointer to user's old password   */
	char	savepasswd[MAX_PASS+1];	 /* User's old cleartext password    */
	char    *message = (char *)NULL; /* return message from chpass()     */
	char    passwd[MAX_PASS+1];      /* user's cleartext password        */

	/*
	 * Set the keyboard generated signals to be caught (INT and
	 * QUIT) so the user can give up and not change their password.
	 */
	signal (SIGINT, (void (*)(void))onint);
	signal (SIGQUIT, (void (*)(void))onint);

	printf(CHGPASS,uname);

	/*
  	 * chpass() will stay in this loop until the user
	 * enters a valid new password.
	 */
	do
	{
		do
		{
			loop++;

			rc = chpass(uname, tp, &reenter, &message);
			if (message)
			{
				if (reenter)
				{
					if(_passwdentry(message,passwd))
					{
	                			fprintf(stderr,ERCHGPASS,uname);
                				exitax(PWDCHGAUD,errno,uname,
							NULL,PRINT);
					}
					tp = passwd;
					/*
					 * Store old password provided, on the
					 * first loop.  This will be resupplied
					 * to chpass() if the user must reenter
					 * a different new password.
					 */
					if (loop == 1)
					{
						strcpy(savepasswd, passwd);
						op = &savepasswd[0];
					}
				}
				else
					fputs(message, stderr);
	
				free(message);
				message = (char *)NULL;
			}
		} while (reenter);

		tp = op; /* Supply old password since we know it */ 
	} while (rc == 1);

	if (rc)
	{
		if (errno == ESAD)
			xaudit(PWDCHGAUD,errno,uname,(char *)NULL);
		else
		{
			fprintf(stderr,ERCHGPASS,uname);
                	exitax(PWDCHGAUD,errno,uname,NULL,PRINT);
		}
	}
}

/*
 * NAME: 	getuname()
 *                                                                    
 * FUNCTION: 	Finds the user name requesting password change.  The routine
 *	     	first checks any command line names provided.  If this fails 
 *  	     	then we will go with the real user id.
 *                                                                    
 * PASSED:	uname = user's name passed back in allocated storage
 *		argc  = number of arguments from command line
 *		argv  = command line
 *
 * RETURNS: 	none
 */  
static void
getuname(char **uname, int argc, char **argv)
{
	char	*name = (char *)NULL;	 /* name specified on command line   */
	char	shortname[PW_NAMELEN];	 /* shortened (max 8 byte) user name */
	int	invoker;		 /* User id of target user  	     */
	int	opt, fflag = 0, sflag = 0;

	while ((opt = getopt (argc, argv, "fs")) != EOF)
	{
		switch(opt)
		{
			case 'f':
				if (fflag || sflag)
					usage(PASUSAGE);
				fflag++;
				break;
			case 's':
				if (fflag || sflag)
					usage(PASUSAGE);
				sflag++;
				break;
			default:
				usage(PASUSAGE);
		}
	}

	if (argc > (optind + 1))
		usage(PASUSAGE);

	if (argv[optind])
	{
		name = argv[optind];

		/*
		 * Create shortened AIX name since the username may contain
		 * a DCE cellname.
		 */
		_normalize_username(shortname, name);

                /* name specified: does user exists? */
                if(getuserattr (shortname, S_ID, (void *)&invoker,SEC_INT))
                {
                        /* user not found */
                        if (errno == ENOENT)
                        {
                                fprintf(stderr,USRNONEX,shortname);
                                exitax(PWDCHGAUD,NOEXISTS,shortname,NULL,PRINT);
                        }
                        fprintf(stderr,ERCHGPASS,shortname);
                        exitax(PWDCHGAUD,errno,shortname,NULL,PRINT);
                }
        }
	else	/* No name found, then get real user's name */
	{
               	if ((name = IDtouser(getuid())) == NULL) 
		{
			/* user not found */
                       	fprintf(stderr,USRINFO);
			exitax(PWDCHGAUD,errno,NULL,NULL,NOPRINT);
               	} 
		if ((name = (char *)strdup(name)) == (char *)NULL)
		{
			fprintf(stderr,MALLOC);
			exitax(PWDCHGAUD,errno,name,NULL,PRINT);
		}
	}
	*uname = name;

	if (fflag)
		runprog(CHFN,*uname);

	if (sflag)
		runprog(CHSH,*uname);
}

/*
 * NAME:	runprog()
 *                                                                    
 * FUNCTION: 	exec chfn or chsh.
 *                                                                    
 * PASSED:	prog = program to exec, uname = username passwd to program.
 *
 * RETURNS: 	none
 */  

static void
runprog(char *prog,char *uname)
{

	if (uname)
		execl(prog, prog, uname, 0);
	else
		execl(prog, prog, 0);

	fprintf(stderr,ERCHGPASS,uname);
	exitax(PWDCHGAUD,errno,uname,NULL,PRINT);
}
