static char sccsid[] = "@(#)13	1.13.1.2  src/bos/usr/bin/pwdck/pwdck.c, cmdsadm, bos411, 9428A410j 2/23/94 09:43:47";
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: MSGSTR
 *		main
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
/*
 * pwdck [ -p | -n | -y | -t ] [ user1 user2 .. | ALL ]
 *
 * The pwdck command will verify the correctness of the password information
 * for one or more users defined in the user database. If ALL is specified,
 * then all users are checked.
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <userpw.h>
#include <usersec.h>
#include <locale.h>
#include "pwdck.h"

#include "pwdck_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PWDCK,n,s) 

/* 
 * global data 
 */
struct	pwdck	**sptab;	/* table for /etc/security/passwd fields     */
struct	pwfile	**ptab;		/* table for /etc/passwd fields	     	     */
int	totalusers=0;		/* count of all lines in /etc/passwd	     */
int	pwdreq;			/* flag indicating passwords required 	     */
int	exitcode = 0;		/* exit code				     */
int	fixit,query;		/* used for interactive			     */
int	modify = 0;		/* flag to indicate whether to commit fixes  */
char	*mp;			/* pointer to message cat string	     */
char	*fxp;			/* pointer to 'fix it' cat string	     */
char 	*tmpep = "/etc/epXXXXXX"; 	/* string for temp /etc/passwd file  */
char 	*tmpesp = "/etc/security/espXXXXXX";	/* and /etc/security/passwd  */

/* command-line flags */
int 	yflag = 0;		/* report and fix			     */
int 	nflag = 0;		/* report but don't fix 		     */
int 	pflag = 0;		/* don't report but do fix 		     */
int	tflag = 0;		/* interactive: report, ask, fix	     */
int	all;			/* flag to indicate that 'ALL' was specified */

extern int errno;

main(argc,argv)
int argc;
char *argv[];
{
	int 	i;			/* counter thru user table 	      */
	int 	flag;			/* option flag 			      */
	char	**users;		/* list of commandline-specified users*/
	struct	pwfile	**pwdtp;	/* pointer to /etc/passwd entry       */
	time_t	old_pw;			/* orig time for /etc/passwd 	      */
	time_t	old_spw;		/* orig time for /etc/security/passwd */
	time_t	new_pw;			/* last mod time for /etc/passwd      */
	time_t	new_spw;		/* last mod time for /etc/security/pw */
	struct	stat	dbm_pw;		/* stat for /etc/passwd.pag file      */
	int	epfd;			/* descriptor for /etc/passwd	      */
	int	espfd;			/* descriptor for /etc/security/passwd*/
extern	int	optind;			/* index of argument after flags      */

	/* suspend auditing for this process */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* suspend privilege */
	privilege(PRIV_LAPSE);

	setlocale(LC_ALL, "");
	catd = catopen(MF_PWDCK, NL_CAT_LOCALE);

	while ((flag = getopt (argc, argv, "pynt")) != EOF) 
	{
		switch (flag) 
		{
			case 'p':
				pflag++;
				break;
			case 'y':
				yflag++;
				break;
			case 'n':
				nflag++;
				break;
			case 't':
				tflag++;
				break;
		}
	}

	/* Validate the values for pflg, yflg, and nflg.  */
	if (pflag + yflag + nflag + tflag != 1)
		usage ();

	/* allocate memory for the users list	*/
	users = (char **)pwmalloc(sizeof(char *) * argc); 

	/* set up list of users specified on the command line */
	for (i=0,argv+=optind,all=0; *argv!=NULL; i++,argv++)
	{
		if (strcmp(*argv,"ALL") == 0)
			all = 1;
		users[i] = *argv;
	}
	/* if no users specified, no good */
	if (i==0)
		usage();

	users[i] = (char *)NULL;	/* terminate the list */

	/* if 'ALL' is specified, there should be no other 
	 * users specified 
	 */
	if (all && (i > 1))
		usage();

	/* build tables representing both /etc/passwd 
	 * and /etc/security/passwd.
	 */

	/* first open the user data base */
	if (setuserdb(S_READ) != 0) 
	{
		mp = MSGSTR(MOPENDB,DOPENDB);
		pwexit(mp, AOPENDB, (char *)NULL);
	}

	/* open and lock the /etc/passwd file */
	epfd = pwopen(PASSFILE);

	/* get the original mod time */
	old_pw = gettime(PASSFILE);

	/* open and lock the /etc/security/passwd file. */
	espfd = pwopen(SPASSFILE);

	/* get the original mod time */
	old_spw = gettime(SPASSFILE);

	/* build a table of selected (or ALL) /etc/passwd entries (ptab[]) 
	 */
	buildpw(users,epfd);

	/* build the table of ALL /etc/security/passwd entries 
	 * (sptab[]).
	 * NOTE: We need them all so we can run ckauth() 
	 * for each user.
	 */
	buildsecpw(NEW,(char *)NULL,(char *)NULL,espfd);

	/* if we're not fixing anything,
	 * then close and unlock the database */
	if (nflag)
	{
		close(epfd);
		close(espfd);
	}

	/* read all user names from /etc/passwd
	 * by looping through the /etc/passwd table (ptab[]).
	 * users[] contains the users from the command line.
	 */
	for( pwdtp=ptab; *pwdtp != NULL; pwdtp++)
	{
		/* does this user 
		 * need to be checked? 
		 */
		if ( ! ((*pwdtp)->check) )
			continue;

		if ( !all && !userfound((*pwdtp)->user,users) )
			continue;

		/* check alpha makeup of 
		 * username in /etc/passwd. 
		 */
		if (ckusername(*pwdtp) == -1)
			continue;

		/* check the password field 
		 * in /etc/passwd 
		 */
		 ckpwfield(*pwdtp);

		/* check if a password is required
		 * for this user.  pwdreq is used by
		 * ckuserpw() and ckauth().
		 */
		 pwdreq = pwrequired((*pwdtp)->user);

		/* check /etc/security/passwd
		 * for it's userpw info: 
		 * password,flags,lastupdate.  
		 */
		 ckuserpw(*pwdtp);

		/* every SYSTEM;authname entry 
		 * in /etc/security/user must have 
		 * 'authname' in /etc/security/passwd 
		 */
		 ckauth(*pwdtp);
	}

	if (all)
	{
		/* each /etc/security/passwd entry must correspond to the 
		 * authentication name of a local user.
		 * by default: every user in /etc/passwd has an authname
		 * (unless explicitly set to auth1 = NONE),
		 * so delete the entries found in /etc/security/passwd that
		 * don't exist in /etc/passwd or in /etc/security/user.
		 * perform this check only if we're fixing ALL users.
		 */
		ckauthnames();

		/* check to make sure that the entries in /etc/security/passwd
		 * are in the same order as /etc/passwd.
		 */
		ckauthorder();
	}

	pwfree((void *) users);

	/* write out the corrected database
	 * and close and unlock the files
	 * MAKE sure it hasn't been changed by another
	 * process while we were still running. If so, report it,
	 * ask, and write out the file. The other process' 
	 * changes are lost if we overwrite.
	 */

	new_pw = gettime(PASSFILE);
	new_spw = gettime(SPASSFILE);

	/* write out the password files only if there 
	   was anything to fix (modify)*/

	if (modify && !nflag)
	{
		if ((old_pw != new_pw) || (old_spw != new_spw))
		{
			mp = MSGSTR(MMODIF,DMODIF);
			fxp = MSGSTR(FMODIF,DFMODIF);
			if(!report(mp, AMODIF, fxp, (char *)NULL, FIX))
			{	/* if he says "don't fix it" */
				close(epfd);
				close(espfd);
				exit(-1);
			}
		}

		/* write the password tables out;
		 * writepw() frees the memory too */
		writepw(epfd,espfd);
	}
	
	/* close the open database files */

	if (!nflag)
	{
		close(epfd);
		close(espfd);
	}

	/*
	 * Now see if the password file is newer than the look-aside
	 * files.  This will cause a warning message with no action
	 * taken.
	 */

	new_pw = gettime(PASSFILE);

	if (stat ("/etc/passwd.pag", &dbm_pw) == 0 &&
			new_pw > dbm_pw.st_mtime)
	{
		mp = MSGSTR(MOLDDBM,DOLDDBM);
		(void)report(mp,AOLDDBM,(char *)NULL,(char *)NULL,NFIX);
	}

	exit(exitcode);
}

