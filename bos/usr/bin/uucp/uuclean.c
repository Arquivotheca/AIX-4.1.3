static char sccsid[] = "@(#)64	1.8  src/bos/usr/bin/uucp/uuclean.c, cmduucp, bos411, 9428A410j 11/11/93 15:07:38";
/* 
 * COMPONENT_NAME: CMDUUCP uuclean.c
 * 
 * FUNCTIONS: FULLNAME, Muuclean, cleanup, logent, luser, notifyuser, 
 *            systat 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
**	uuclean
**
**	This program emulates a Berkeley command, but it was
**	rewritten by using the basic AT&T "uucleanup" engine.  Some
**	of the functions (i.e., notifyuser()) were borrowed from
**	Berkeley, but most of this is simply an adaptation of
**	AT&T source code.		22 Aug 1989
**
**	usage:  uuclean -p prefix [-m] [-n hours] [-d directory]
**	(up to 10 prefixen can be specified)
**
*/

#include	"uucp.h"
#include	<pwd.h>

#ifdef	V7
#define O_RDONLY	0
#endif

nl_catd catd;
/* need these dummys to satisy some .o files */
void systat(){}
void logent(){}


#define FULLNAME(full,dir,file)	(void) sprintf(full, "%s/%s", dir, file);
#define	NOMTIME	72			/* default hours to age files */
#define	MAXPRE	10			/* maximum number of prefixes */

extern int get_args();

char prefix[MAXPRE][NAMESIZE];		/* Valid prefixen, per "uuclean" */
char _ShortLocal[6];			/* Short version of system name */
mode_t omask;  				/* Not used. Just to satisfy linker. */

main(argc, argv, envp)
int  argc;
char *argv[];
char **envp;
{
	DIR *spooldir, *subdir;
	char f[MAXFULLNAME], subf[MAXFULLNAME];
	char fullname[MAXFULLNAME];
	char soptName[MAXFULLNAME];	/* name from -d option */
	char buf[256], *tbuf;		/* temporary buffer */
	int i = 0, value = 0, Npre = 0, mailflag = 0, check = 0;
	time_t nomtime, ptime;
	struct stat stbuf;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


	soptName[0] = NULLCHAR;
	(void) strcpy(Logfile, CLEANUPLOGFILE);
	uucpname(Myname);
	(void) strncpy(_ShortLocal, Myname, 5);
	_ShortLocal[5] = NULLCHAR;
	(void) strcpy(Progname, "uuclean");
	nomtime = NOMTIME * (time_t)3600;
	while ((i = getopt(argc, argv, "d:mn:p:")) != EOF) {
		switch(i) {
		case 'd':	/* system name, really */
			(void) strcpy(soptName, optarg);
			break;
		case 'm':
			mailflag++;	/* should we send some mail? */
			break;
		case 'n':
			nomtime = atoi(optarg) * (time_t)3600;
			break;
		case 'p':
			if (Npre > 10) {
				fprintf(stderr, MSGSTR(B_CLEAN_2, 
				 "Maximum of 10 prefixes can be specified.\n"));
				cleanup(1);
			}
			check = 1;
			if (strlen(optarg) >= MAXFULLNAME) {
				fprintf(stderr, MSGSTR(B_CLEAN_3, 
					"Prefix '%s' too long.\n"), optarg);
				exit(1);
			}
			strcpy(prefix[Npre++], optarg);
			break;
		default:
			fprintf(stderr, MSGSTR(B_CLEAN_1, "Usage:  uuclean [-m] [-n hours] [-d subdirectory] [-p prefix] ... \n"));
			cleanup(1);
		}

 	}  
	if (argc != optind) {
		fprintf(stderr, MSGSTR(B_CLEAN_1, "Usage:  uuclean [-m] [-n hours] [-d subdirectory] [-p prefix] ...\n"));
		exit(1);
	}

	if (chdir(SPOOL) != 0) {
	    (void) fprintf(stderr, MSGSTR(MSG_UCL_13, 
			"CAN'T CHDIR (%s): errno (%d)\n"), SPOOL, errno);
		exit(1);
	}

	if ( (spooldir = opendir(SPOOL)) == NULL) {
	    (void) fprintf(stderr, MSGSTR(MSG_UCL_14, 
		"CAN'T OPEN (%s): errno (%d)\n"), SPOOL, errno);
		exit(1);
	}

	time(&ptime);
	while (gnamef(spooldir, f) == TRUE) {
	    if (EQUALSN("LCK..", f, 5))
		continue;

	    if (*soptName && !EQUALS(soptName, f))
		continue;

	    if (DIRECTORY(f)) {
	if ( (subdir = opendir(f)) == NULL) {
		fprintf(stderr, MSGSTR(B_CLEAN_4, "Can't open the subdirectory '%s'\n"), f);
		exit(1);
	}

		(void) strcpy(Rmtname, f);
		    while (gnamef(subdir, subf) == TRUE) {
			FULLNAME(fullname, f, subf);

			if (check == 0) {
				Npre = 1;
			}

				/*
				** scan the file against each prefix
				** in the list.
				**
				** if no prefixes were mentioned, we
				** have an automagic match!
				*/

			for(i = 0; i < Npre; i++) {
			   if (check == 0)	/* reset forged Npre value */
				Npre = 0;
			   if ((check == 0) || (EQUALSN(subf, prefix[i], 
				MIN(strlen(prefix[i]), strlen(subf))))) {

			      if (stat(fullname, &stbuf) == -1) {
				fprintf(stderr, MSGSTR(B_CLEAN_5, 
					"stat on %s failed!\n"), fullname);
				continue;
			      }
			   if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
				continue;

			   if ((ptime - stbuf.st_mtime) < nomtime)
				continue;
					/*
					** strip the path
					*/
			   sprintf(buf, "%s/%s", SPOOL, fullname);
			   tbuf = strrchr(buf, (int) '/');
			   *tbuf++;

				/*
				** if we are deleting a command file
				** (i.e., C.*), we can parse it out
				** and make some attempt to figure
				** out who owns the beast.
				*/

			   if (tbuf[0] == CMDPRE)
			   	notifyuser(buf);
			   unlink(buf);

			   if (mailflag)
				luser(buf, stbuf.st_uid);
			}
		    }
		}
		closedir(subdir);
	}
    }
    exit(0);
}

/*
**	notifyuser(file)
**		Mail a message to the user saying his request
**		was axed.  Also, if we can figure it out, tell
**		him what the problem was (i.e., Couldn't contact
**		the remote system, corrupted JCL script, didn't
**		say "pretty please with sugar on top," etc...)
**
**
**	Return code:  None.
*/

notifyuser(file)
char *file;
{
	FILE *fp;
	int numrq;
	char frqst[100], lrqst[100];
	char msg[BUFSIZ];
	char *args[10];

	memset(msg, 0, sizeof msg);

				/*
				** If problems arise while
				** attempting to parse the
				** file, forget it -- close
				** shop and go home.
				*/
	if ((fp = fopen(file, "r")) == NULL) {
		return;
	}
	if (fgets(frqst, 100, fp) == NULL) {
		fclose(fp);
		return;
	}

	numrq = 1;
	while (fgets(lrqst, 100, fp))
		numrq++;
	fclose(fp);
	sprintf(msg, MSGSTR(B_CLEAN_6, "File %s delete. \nCould not contact remote. \n%d requests deleted.\n"), file, numrq);
	if (numrq == 1) {
		strcat(msg, MSGSTR(B_CLEAN_7, "REQUEST:  "));
		strcat(msg, frqst);
	} else {
		strcat(msg, MSGSTR(B_CLEAN_8, "FIRST REQUEST:  "));
		strcat(msg, frqst);
		strcat(msg, MSGSTR(B_CLEAN_9, "\nLAST REQUEST:  "));
		strcat(msg, lrqst);
	}
	get_args(frqst, args, 10);
	mailst(args[3], msg, "", "");

}


/*
**
**	luser(file, uid);
**		This route will determine who owns the file "file"
**		and create a message buffer to send to the local user
**		via mailst().  Again, this is only for local users so
**		most of the messages end up in UUCP's or root's mailbox.
**
**	Returns:  nothing.
*/

luser(file, uid)
char *file;
int uid;
{
	static struct passwd *pwd;

	char mstr[256];

	memset(mstr, 0, sizeof mstr);
	sprintf(mstr, MSGSTR(B_CLEAN_10, "uuclean deleted file %s\n"), file);

	setpwent();
	if ((pwd = getpwuid(uid)) != NULL)
		  mailst(pwd->pw_name, mstr, "", "");
}

/*
**	cleanup(code)
**		Exit the program with the code passed.
**
**	returns:  We don't...
*/

cleanup(code)
int code;
{

	catclose(catd);   /* get rid of that shared memory */

	exit(code);
}

