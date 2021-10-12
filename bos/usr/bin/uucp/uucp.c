static char sccsid[] = "@(#)25	1.24  src/bos/usr/bin/uucp/uucp.c, cmduucp, bos41J, 9521B_all 5/26/95 11:44:06";
/* 
 * COMPONENT_NAME: CMDUUCP uucp.c
 * 
 * FUNCTIONS: Muucp, cleanup, copy, ruux, split, syscfile 
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



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	uucp:uucp.c	1.5
*/
#include "uucp.h"
/* VERSION( uucp.c	5.3 -  -  ); */

/*
 * uucp
 * user id 
 * make a copy in spool directory
 */

nl_catd catd;
int Copy = 0;
static int _Transfer = 0;
char Nuser[32];
char *Ropt = " ";
char Optns[10];
char Uopts[BUFSIZ];
char Grade = 'N';
int Mail = 0;
int Notify = 0;

char	Sfile[MAXFULLNAME];
#ifndef	V7
long ulimit();
#endif

static FILE	* syscfile(char *, char *);
static void split(char *, char *, char *, char *);
mode_t omask = -1;

short	jflag = 0;	/* -j flag  Jobid printout */

main(argc, argv, envp)
char	*argv[];
char	**envp;
{
	int	ret;
	int	errors = 0;
	char	*fopt;
	char	sys1[MAXFULLNAME], sys2[MAXFULLNAME];
	char	fwd1[MAXFULLNAME], fwd2[MAXFULLNAME];
	char	file1[MAXFULLNAME], file2[MAXFULLNAME];

#ifndef V7
	long	limit, dummy;
	char	msg[100];
#endif V7

	omask = umask(UMASK);
	(void)atexit(oldmask);

	/* this fails in some versions, but it doesn't hurt */
	Uid = getuid();
	Euid = geteuid();
	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


	if (Uid == 0)
		(void) setuid(UUCPUID);

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUCP);

	Env = envp;
	fopt = NULL;
	(void) strcpy(Progname, "uucp");
	Pchar = 'U';
	*Uopts =  NULLCHAR;

	/*
	 * find name of local system
	 */
	uucpname(Myname);
	Optns[0] = '-';
	Optns[1] = 'd';
	Optns[2] = 'c';
	Optns[3] = Nuser[0] = Sfile[0] = NULLCHAR;

	/*
	 * find id of user who spawned command to 
	 * determine
	 */
	(void) guinfo(Uid, User);

	while ((ret = getopt(argc, argv, "Ccdfg:jmn:rs:x:")) != EOF) {
		switch (ret) {

		/*
		 * make a copy of the file in the spool
		 * directory.
		 */
		case 'C':
			Copy = 1;
			Optns[2] = 'C';
			break;

		/*
		 * not used (default)
		 */
		case 'c':
			break;

		/*
		 * not used (default)
		 */
		case 'd':
			break;
		case 'f':
			Optns[1] = 'f';
			break;

		/*
		 * set service grade
		 */
		case 'g':
			if (!isalpha(*optarg) && !isdigit(*optarg)) {
				fprintf(stderr, MSGSTR(MSG_CICO23,
				"%s:Ignoring invalid transfer grade of %c\n"),
					"uucp", *optarg);
			}
			else
				Grade = *optarg;
			break;

		case 'j':	/* job id */
			jflag = 1;
			break;

		/*
		 * send notification to local user
		 */
		case 'm':
			Mail = 1;
			(void) strcat(Optns, "m");
			break;

		/*
		 * send notification to user on remote
		 * if no user specified do not send notification
		 */
		case 'n':
			Notify = 1;
			(void) strcat(Optns, "n");
			(void) sprintf(Nuser, "%.8s", optarg);
			(void) sprintf(Uopts+strlen(Uopts), "-n%s ", Nuser);
			break;

		/*
		 * create JCL files but do not start uucico
		 */
		case 'r':
			Ropt = "-r";
			break;

		/*
		 * return status file
		 */
		case 's':
			fopt = optarg;
			/* "m" needed for compatability */
			(void) strcat(Optns, "mo");
			break;

		/*
		 * turn on debugging
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
#ifdef SMALL
			fprintf(stderr,
			"WARNING: uucp built with SMALL flag defined -- no debuginfo available\n");
#endif /* SMALL */
			break;

		default:
			usage();
			break;
		}
	}
	DEBUG(4, "\n\n** %s **\n", "START");
	gwd(Wrkdir);
	if (fopt) {
		if (*fopt != '/')
			(void) sprintf(Sfile, "%s/%s", Wrkdir, fopt);
		else {
			if ( access(dirname(fopt),F_OK) != 0 ) {
				(void) fprintf(stderr, MSGSTR(MSG_UUCP13,
				"The directory name %s is not valid.\n"), dirname(fopt));
				cleanup(-2);
			} else {
				(void) sprintf(Sfile, "%s", fopt);
			}
		}
	}
	/*
	 * work in WORKSPACE directory
	 */
	ret = chdir(WORKSPACE);
	if (ret != 0) {
		(void) fprintf(stderr, MSGSTR(MSG_UUCP2, 
		    "No work directory - %s - get help\n"), WORKSPACE);
		cleanup(-12);
	}

	if (Nuser[0] == NULLCHAR)
		(void) strcpy(Nuser, User);
	(void) strcpy(Loginuser, User);
	DEBUG(4, "UID %d, ", Uid);
	DEBUG(4, "User %s\n", User);
	if (argc - optind < 2) {
		usage();
	}

	/*
	 * set up "to" system and file names
	 */

	split(argv[argc - 1], sys2, fwd2, file2);
	if (*sys2 != NULLCHAR) {
		if (versys(sys2) != 0) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP4,
			      "bad system: %s\n"), sys2);
			cleanup(-11);
		}
	}
	else
		(void) strcpy(sys2, Myname);

	(void) strncpy(Rmtname, sys2, MAXBASENAME);
	Rmtname[MAXBASENAME] = NULLCHAR;

	DEBUG(9, "sys2: %s, ", sys2);
	DEBUG(9, "fwd2: %s, ", fwd2);
	DEBUG(9, "file2: %s\n", file2);

	/*
	 * if there are more than 2 argsc, file2 is a directory
	 */
	if (argc - optind > 2)
		(void) strcat(file2, "/");

	/*
	 * do each from argument
	 */

	for ( ; optind < argc - 1; optind++) {
	    split(argv[optind], sys1, fwd1, file1);
	    if (*sys1 != NULLCHAR) {
		if (versys(sys1) != 0) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP4,
			      "bad system: %s\n"), sys1);
			cleanup(-11);
		}
	    }

	    /*  source files can have at most one ! */
	    if (*fwd1 != NULLCHAR) {
		/* syntax error */
	        (void) fprintf(stderr, MSGSTR(MSG_UUCP5,
			"illegal  syntax %s\n"), argv[optind]);
	        exit(2);
	    }

	    /*
	     * check for required remote expansion of file names -- generate
	     *	and execute a uux command
	     * e.g.
	     *		uucp   owl!~/dan/   ~/dan/
	     *
	     * NOTE: The source file part must be full path name.
	     *  If ~ it will be expanded locally - it assumes the remote
	     *  names are the same.
	     */

	    if (*sys1 != NULLCHAR)
		if ((strchr(file1, '*') != NULL
		      || strchr(file1, '?') != NULL
		      || strchr(file1, '[') != NULL)) {
		        /* do a uux command */
		        if (ckexpf(file1) == FAIL)
			    exit(6);
		        ruux(sys1, sys1, file1, sys2, fwd2, file2);
		        continue;
		}

	    /*
	     * check for forwarding -- generate and execute a uux command
	     * e.g.
	     *		uucp uucp.c raven!owl!~/dan/
	     */

	    if (*fwd2 != NULLCHAR) {
		if (*sys1 == NULLCHAR)	/* a serious patch */
			strcat(sys1, Myname);
	        ruux(sys2, sys1, file1, "", fwd2, file2);
	        continue;
	    }

	    /*
	     * check for both source and destination on other systems --
	     *  generate and execute a uux command
	     */

	    if (*sys1 != NULLCHAR )
		if ( (!EQUALS(Myname, sys1))
	    	  && *sys2 != NULLCHAR
	    	  && (!EQUALS(sys2, Myname)) ) {
		    ruux(sys2, sys1, file1, "", fwd2, file2);
	            continue;
	        }


	    if (*sys1 == NULLCHAR)
		(void) strcpy(sys1, Myname);
	    else {
		(void) strncpy(Rmtname, sys1, MAXBASENAME);
		Rmtname[MAXBASENAME] = NULLCHAR;
	    }

	    DEBUG(4, "sys1 - %s, ", sys1);
	    DEBUG(4, "file1 - %s, ", file1);
	    DEBUG(4, "Rmtname - %s\n", Rmtname);
	    if (copy(sys1, file1, sys2, file2))
	    	errors++;
	}

	/* move the work files to their proper places */
	commitall();

	/*
	 * do not spawn daemon if -r option specified
	 */
#ifndef	V7
	limit = ulimit(1, dummy);
#endif V7
	if (*Ropt != '-')
#ifndef	V7
		if (limit < MINULIMIT)  {
			(void) sprintf(msg, MSGSTR(MSG_UUCP6,
			   "ULIMIT (%ld) < MINULIMIT (%ld)"), limit, MINULIMIT);
			logent(msg, MSGSTR(MSG_UUCP7, "Low-ULIMIT"));
		}
		else
#endif
			xuucico(Rmtname);
	if (jflag && *Jobid)
	    printf("%s\n", Jobid);
	cleanup(errors);
}

/*
 * cleanup lock files before exiting
 */
cleanup(code)
register int	code;
{
	rmlock(CNULL);
	clrlock(CNULL);
	if (code != 0)
		wfabort();	/* this may be extreme -- abort all work */
	if (code < 0)
		(void) fprintf(stderr, MSGSTR(MSG_UUCP8, 
			"uucp failed completely: code %d\n"), code);
	else if (code > 0) {
		if (code == 1)
		(void) fprintf(stderr, MSGSTR(MSG_UUCP9Q,
			"uucp failed partially: %d file(s) sent; %d error\n"),
			_Transfer, code);
		else
		(void) fprintf(stderr, MSGSTR(MSG_UUCP9A,
			"uucp failed partially: %d files(s) sent; %d errors\n"),
			_Transfer, code);
	}
	catclose(catd);

	exit(code);
}

/*
 * generate copy files for s1!f1 -> s2!f2
 *	Note: only one remote machine, other situations
 *	have been taken care of in main.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */

copy(s1, f1, s2, f2)
char *s1, *f1, *s2, *f2;
{
	FILE *cfp, *syscfile();
	struct fullstat stbuf, stbuf1;
	int status;
	int type, statret;
	char dfile[NAMESIZE];
	char cfile[NAMESIZE];
	char file1[MAXFULLNAME], file2[MAXFULLNAME];
	char msg[BUFSIZ];

	type = 0;
	(void) strcpy(file1, f1);
	(void) strcpy(file2, f2);
	if (!EQUALS(s1, Myname))
		type = 1;
	if (!EQUALS(s2, Myname))
		type = 2;

	switch (type) {
	case 0:

		/*
		 * all work here
		 */
		DEBUG(4, "all work here %d\n", type);

		/*
		 * check access control permissions
		 */
		if (ckexpf(file1))
			 return(-6);
		if (ckexpf(file2))
			 return(-7);
		if (uidstat(file1, &stbuf) != 0) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP10,
			    "can't get file status %s\n copy failed\n"), file1);
			return(8);
		}
		statret = uidstat(file2, &stbuf1);
		if (statret == 0
		  && stbuf.st_ino == stbuf1.st_ino
		  && stbuf.st_dev == stbuf1.st_dev) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP11,
			    "%s %s - same file; can't copy\n"), file1, file2);
			return(5);
		}
		if (chkperm(file1, file2, strchr(Optns, 'd')) ) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP12,
			    "permission denied\n"));
			cleanup(1);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP13,
			  "directory name illegal - %s\n"), file1);
			return(9);
		}
		/* see if I can read this file as read uid, gid */
		/* osf note: "accessx" is an aix specific routine */
		if ( accessx(file1,R_OK,ACC_INVOKER)) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP14,
			    "uucp can't read (%s) mode (%o)\n"),
			    file1, stbuf.st_mode);
			(void) fprintf(stderr, MSGSTR(MSG_UUCP14C,
			    "use cp command\n"));
			return(3);
	        }
		/* osf note: "accessx" is an aix specific routine */
		if ( accessx(file2,W_OK,ACC_ALL)) {
			if (errno != ENOENT) {
				(void) fprintf(stderr, MSGSTR(MSG_UUCP15,
				       "can't write file (%s) mode (%o)\n"),
						 file2, stbuf1.st_mode);
				return(4);
			}
		}

		/*
		 * copy file locally
		 */
		DEBUG(2, "local copy:  uidxcp(%s, ", file1);
		DEBUG(2, "%s\n", file2);

		/*
		 * Change.  Copy was done so new file was owned by
		 * effective user.  Now copy is done so file is owned
		 * by real user for security reasons.
		 */
		(void) setuid(Uid);
		status = xcp(file1, file2);
		(void) setuid(Euid);
		if (status == FAIL) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP16,
			    "can't copy file (%s) errno %d\n"), file2, errno);
			return(5);
		}
		(void) chmod(file2, (int)stbuf.st_mode);

                /*
                 * if user specified -m, notify "local" user
                 */
		if ( Mail ) {
			sprintf(msg,
			"REQUEST: %s!%s --> %s!%s (%s)\n(SYSTEM %s) copy succeeded\n",
			s1, file1, s2, file2, User, s2 );
			(void) mailst(User, msg, "", "");
		}
		/*
		* if user specified -n, notify "remote" user
		*/
		if ( Notify ) {
			sprintf(msg, "%s from %s!%s arrived\n",
				file2, s1, User );
			(void) mailst(Nuser, msg, "", "");
		}
		return(0);
	case 1:

		/*
		 * receive file
		 */
		DEBUG(4, "receive file - %d\n", type);

		/*
		 * expand source and destination file names
		 * and check access permissions
		 */
		if (file1[0] != '~')
			if (ckexpf(file1))
				 return(6);
		if (ckexpf(file2))
			 return(7);

		/*
		 * insert JCL card in file
		 */
		cfp = syscfile(cfile, s1);
		(void) fprintf(cfp, "R %s %s %s %s %s %o %s\n", file1, file2,
			User, Optns,
			*Sfile ? Sfile : "dummy",
			0777, Nuser);
		(void) fclose(cfp);
		(void) sprintf(msg, "%s!%s --> %s!%s", Rmtname, file1,
		    Myname, file2);
		logent(msg, MSGSTR(MSG_UUCP17,"QUEUED"));
		break;
	case 2:

		/*
		 * send file
		 */
		if (ckexpf(file1))
			 return(10);
		/* XQTDIR hook enables 3rd party uux requests (cough) */
		if (file2[0] != '~' && !EQUALS(Wrkdir, XQTDIR))
			if (ckexpf(file2))
				 return(7);
		DEBUG(4, "send file - %d\n", type);

		if (uidstat(file1, &stbuf) != 0) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP18,
			    "can't get status for file %s\n"), file1);
			return(8);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			(void) fprintf(stderr, MSGSTR(MSG_UUCP19,
			    "directory name illegal - %s\n"), file1);
			return(9);
		}

		/*
		 * make a copy of file in spool directory
		 */
		/* note "accessx" is an aix specific routine */
		if (Copy || accessx(file1,R_OK,ACC_ALL)) {
			gename(DATAPRE, s2, Grade, dfile);

			if (uidxcp(file1, dfile))
			    return(16);

			(void) chown(dfile, UUCPUID, UUCPGID);
			(void) chmod(dfile, DFILEMODE);
			wfcommit(dfile, dfile, s2);
		} else {

			/*
			 * make a dummy D. name
			 * cntrl.c knows names < 6 chars are dummy D. files
			 */
			(void) strcpy(dfile, "D.0");
		}

		/*
		 * insert JCL card in file
		 */
		cfp = syscfile(cfile, s2);
		(void) fprintf(cfp, "S  %s %s %s %s %s %o %s %s\n",
		    file1, file2, User, Optns, dfile,
		    stbuf.st_mode & 0777, Nuser, Sfile);
		(void) fclose(cfp);
		(void) sprintf(msg, "%s!%s --> %s!%s", Myname, file1,
		    Rmtname, file2);
		logent(msg, MSGSTR(MSG_UUCP17,"QUEUED"));
		break;
	}
	_Transfer++;
	return(0);
}


/*
 *	syscfile(file, sys)
 *	char	*file, *sys;
 *
 *	get the cfile for system sys (creat if need be)
 *	return stream pointer
 *
 *	returns
 *		stream pointer to open cfile
 *		
 */

static FILE	*
syscfile(file, sys)
char 	*file, *sys;
{
	FILE	*cfp;

	if (gtcfile(file, sys) == FAIL) {
		gename(CMDPRE, sys, Grade, file);
		ASSERT(access(file, 0) != 0, 
			MSGSTR(MSG_UDEFS_17, "FILE_EXISTS"), file, errno);
		cfp = fdopen(creat(file, CFILEMODE), "w");
		svcfile(file, sys);
	} else
		cfp = fopen(file, "a");
	/*  set Jobid -- C.jobid */
	(void) strncpy(Jobid, BASENAME(file, '.'), NAMESIZE);
	Jobid[NAMESIZE-1] = '\0';
	ASSERT(cfp != NULL, MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), file, errno);
	return(cfp);
}


/*
 * split - split the name into parts:
 * sys - system name
 * fwd - intermediate destinations
 * file - file part
 */

static
void
split(arg, sys, fwd, file)
char	*arg, *sys, *fwd, *file;
{
    register char *cl, *cr, *n;
    *sys = *fwd = *file = NULLCHAR;
	
    for (n=arg ;; n=cl+1) {
	cl = strchr(n, '!');
	if ( cl == NULL) {
	    /* no ! in n */
	    (void) strcpy(file, n);
	    return;
	}

	if (EQUALS(Myname, n))
		continue;

	cr = strrchr(n, '!');
	(void) strcpy(file, cr+1);
	(void) strncpy(sys, n, cl-n);
	sys[cl-n] = NULLCHAR;

	if (cr != cl) {
	    /*  more than one ! */
	    (void) strncpy(fwd, cl+1, cr-cl-1);
	    fwd[cr-cl-1] = NULLCHAR;
	}
	return;
    }
    /*NOTREACHED*/
}


/*
 * generate and execute a uux command
 */

ruux(rmt, sys1, file1, sys2, fwd2, file2)
char *rmt, *sys1, *file1, *sys2, *fwd2, *file2;
{
    char cmd[BUFSIZ];



    if ( jflag )
    {

	if (*Uopts != NULLCHAR)
	    (void) sprintf(cmd, "uux -e -j -C %s %s!uucp -C \\(%s\\) ", Ropt, rmt, Uopts);
	else
	    (void) sprintf(cmd, "uux -e -j -C %s %s!uucp -C ", Ropt, rmt);
    }
    else
    {

	if (*Uopts != NULLCHAR)
	    (void) sprintf(cmd, "uux -e -C %s %s!uucp -C \\(%s\\) ", Ropt, rmt, Uopts);
	else
	    (void) sprintf(cmd, "uux -e -C %s %s!uucp -C ", Ropt, rmt);

    }

    if (*sys1 == NULLCHAR || EQUALS(sys1, Myname)) {
        if (ckexpf(file1))
  	    exit(1);
	(void) sprintf(cmd+strlen(cmd), " %s!%s ", sys1, file1);
    }
    else
	if (!EQUALS(rmt, sys1))
	    (void) sprintf(cmd+strlen(cmd), " \\(%s!%s\\) ", sys1, file1);
	else
	    (void) sprintf(cmd+strlen(cmd), " \\(%s\\) ", file1);

    if (*fwd2 != NULLCHAR) {
	if (*sys2 != NULLCHAR)
	    (void) sprintf(cmd+strlen(cmd),
		" \\(%s!%s!%s\\) ", sys2, fwd2, file2);
	else
	    (void) sprintf(cmd+strlen(cmd), " \\(%s!%s\\) ", fwd2, file2);
    }
    else {
	if (*sys2 == NULLCHAR || EQUALS(sys2, Myname))
	    if (ckexpf(file2))
		exit(7);
	(void) sprintf(cmd+strlen(cmd), " \\(%s!%s\\) ", sys2, file2);
    }

    DEBUG(2, "cmd: %s\n", cmd);
    logent(cmd, MSGSTR(MSG_UUCP17,"QUEUED"));
    system(cmd);
    return;
}

static
usage()
{

	(void) fprintf(stderr, MSGSTR( MSG_UUCP3A,
	"Usage:  uucp [-c|-C] [-d|-f] [-gGRADE] [-j] [-m] [-nUSER]\\\n[-r] [-sFILE] [-xDEBUG_LEVEL] source-files destination-file\n"));
	cleanup(-2);
}
