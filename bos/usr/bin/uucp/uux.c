static char sccsid[] = "@(#)42	1.23  src/bos/usr/bin/uucp/uux.c, cmduucp, bos41J, 9515B_all 4/13/95 17:58:25";
/* 
 * COMPONENT_NAME: CMDUUCP uux.c
 * 
 * FUNCTIONS: APPCMD, GENRCV, GENSEND, Muux, cleanup, onintr 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*	/sccs/src/cmd/uucp/s.uux.c
	uux.c	1.12	9/11/84 11:35:55
*/
#include "uucp.h"
/* VERSION( uux.c	5.3 -  -  ); */

nl_catd catd;
#define NOSYSPART 0
#define HASSYSPART 1

#define GENSEND(f, a, b, c) {\
ASSERT(fprintf(f, "S %s %s %s -%s %s 0666 %s %s\n", a, b, User, _Statop?"o":"", c, User, _Sfile) >= 0, "CANNOT WRITE", "", errno);\
}
#define GENRCV(f, a, b) {\
ASSERT(fprintf(f, "R %s %s %s - %s 0666 %s\n", a, b, User, *_Sfile ? _Sfile : "dummy", User) >= 0, \
"CANNOT_WRITE", "", errno);\
}

#define USAGE	 "Usage: uux [-aNAME] [-b] [-c] [-C] [-j] [-gGRADE] [-n] [-p] [-r] [-sFILE] [-xNUM] [-z] command-string\n"
#define APPCMD(p)	{(void) strcat(cmd, p); (void) strcat(cmd, " ");}

static char	_Sfile[MAXFULLNAME];
static int	_Statop;
void  onintr(register int);
mode_t omask;

char	_Grade = 'N';
/*
 *	uux
 */
main(argc, argv, envp)
char *argv[];
char **envp;
{
	FILE *fprx = NULL, *fpc = NULL, *fpd = NULL, *fp = NULL;
	int cfileUsed = 0;	/*  >0 if commands put in C. file flag  */
	int rflag = 0;		/*  C. files for receiving flag  */
	int cflag = 0;		/* if > 0 make local copy of files to be sent */
	int nflag = 0;		/* if != 0, do not request error notification */
	int zflag = 0;		/* if != 0, request success notification */
	int old_behavior = 0;   /* if set, let bsh expand pathname */
	int pipein = 0;
	int startjob = 1;
	short jflag = 0;	/* -j flag  output Jobid */
	int bringback = 0;	/* return stdin to invoker on error */
	int ret, i;
	char *getprm();
	char redir = '\0';
	char command = TRUE;
	char cfile[NAMESIZE];	/* send commands for files from here */
	char dfile[NAMESIZE];	/* used for all data files from here */
	char rxfile[NAMESIZE];	/* file for X_ commands */
	char tfile[NAMESIZE];	/* temporary file name */
	char t2file[NAMESIZE];	/* temporary file name */
	char buf[BUFSIZ];
	char inargs[BUFSIZ];
	char cmd[BUFSIZ];
	char *ap;
	char prm[BUFSIZ];
	char syspart[NAMEBUF], rest[BUFSIZ];
	char xsys[NAMEBUF];
	char	*fopt = NULL;
	char	*retaddr = NULL;
	char	*_Sdir = NULL;
	char    *c, *nextchar;
	struct stat stbuf;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);

	omask = umask(UMASK);
	(void)atexit(oldmask);

 /* we want this to run as uucp, even if the kernel doesn't */
	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	if (Uid == 0)
	    setuid(UUCPUID);
/* init environment for fork-exec */
	Env = envp;

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUX);

	/*
	 * determine local system name
	 */
	(void) strcpy(Progname, "uux");
	Pchar = 'X';
	(void) signal(SIGILL, (void(*)(int)) onintr);
	(void) signal(SIGTRAP, (void(*)(int)) onintr);
	(void) signal(SIGIOT, (void(*)(int)) onintr);
	(void) signal(SIGEMT, (void(*)(int)) onintr);
	(void) signal(SIGFPE, (void(*)(int)) onintr);
	(void) signal(SIGBUS, (void(*)(int)) onintr);
	(void) signal(SIGSEGV, (void(*)(int)) onintr);
	(void) signal(SIGSYS, (void(*)(int)) onintr);
	(void) signal(SIGTERM, SIG_IGN);
	uucpname(Myname);
	Ofn = 1;
	Ifn = 0;
	*_Sfile = '\0';

	/*
	 * we need at least one argument
	 */
	if (argc < 2) {
		(void) fprintf(stderr, MSGSTR(MSG_UUX_2,
		"\tusage: %s %s\n"), Progname,MSGSTR(MSG_UUX_1, USAGE));
		exit(2);
	}   

	/*
	 * since getopt() can't handle the pipe input option '-'
	 * I'll change it to "-p"
	 */
	for (i=0; i<argc; i++)
	    if (EQUALS(argv[i], "-"))
		argv[i] = "-p";

	while ((i = getopt(argc, argv, "a:bcCejg:nprs:x:z")) != EOF) {
		switch(i){

		/*
		 * use this name in the U line
		 */
		case 'a':
			retaddr = optarg;
			break;

		/*
		 * if return code non-zero, return command's input
		 */
		case 'b':
			bringback = 1;
			break;

		/* do not make local copies of files to be sent (default) */
		case 'c':
			cflag = 0;
			break;

		/* make local copies of files to be sent */
		case 'C':
			cflag = 1;
			break;
		/*
		 * use bsd shell expansion behavior (i.e, "old" behavior)
		 */
		case 'e':
			old_behavior++;
			break;

		/*
		 * set priority of request
		 */
		case 'g':
			if (!isalpha(*optarg) && !isdigit(*optarg)) {
				fprintf(stderr, MSGSTR(MSG_CICO23,
				  "%s:Ignoring invalid transfer grade of %c\n"),
					"uux", *optarg);
			}
			else
				_Grade = *optarg;
			break;


		case 'j':	/* job id */
			jflag = 1;
			break;


		/*
		 * do not send failure notification to user
		 */
		case 'n':
			nflag++;
			break;

		/*
		 * send success notification to user
		 */
		case 'z':
			zflag++;
			break;

		/*
		 * -p or - option specifies input from pipe
		 */
		case 'p':
			pipein = 1;
			break;

		/*
		 * do not start transfer
		 */
		case 'r':
			startjob = 0;
			break;

		case 's':
			fopt = optarg;
			_Statop++;
			break;

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;

		default:
			usage();
		}
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	if ( optind >= argc )
		usage();

	/*
	 * copy arguments into a buffer for later
	 * processing
	 */
	inargs[0] = '\0';
	for (; optind < argc; optind++) {
		DEBUG(4, "arg - %s:", argv[optind]);
		(void) strcat(inargs, " ");
		(void) strcat(inargs, argv[optind]);
	}
	
        /*
         * Check for special redirection operators: >>, >|, >&, and <<.
         * These special characters are not allowed to be used by uux
         * for XPG4 compliance.
         */
        if ((c = strchr(inargs, '>')) != NULL) {
                nextchar = c;
                nextchar++;
                switch (*nextchar) {
                        case '>':
                        case '|':
                        case '&':
                                (void) fprintf(stderr, MSGSTR(MSG_UUX_17,
     "The operators <<, >>, >|, and >& cannot be used in a command string.\n"));
                                cleanup(1);
                }
        }

        if ((c = strchr(inargs, '<')) != NULL) {
                nextchar = c;
                nextchar++;
                if (*nextchar == '<') {
                        (void) fprintf(stderr, MSGSTR(MSG_UUX_17,
     "The operators <<, >>, >|, and >& cannot be used in a command string.\n"));
                        cleanup(1);
                }
        }

	/*
	 * get working directory and change
	 * to spool directory
	 */
	DEBUG(4, "arg - %s\n", inargs);
	gwd(Wrkdir);
	if(fopt){
		if(*fopt != '/')
			(void) sprintf(_Sfile, "%s/%s", Wrkdir, fopt);
		else
			(void) sprintf(_Sfile, "%s", fopt);

	}
	if (chdir(WORKSPACE) != 0) {
	    (void) fprintf(stderr, MSGSTR(MSG_UUX_3,
		"No spool directory - %s - get help\n"), WORKSPACE);
	    cleanup(12);
	}

	/*
	 * make sure status file is writable if it exists,
	 * or creatable if it doesn't
	 */
	if (_Statop) {
	    _Sdir = dirname(_Sfile);
	    if ((access(_Sdir, W_OK|R_OK|F_OK)) != 0){
		fprintf(stderr,MSGSTR(MSG_UUX_13,
		    "permission denied for directory %s\n"), _Sdir);
		cleanup(2);
	    } 
	    if ((access(_Sfile, W_OK|F_OK)) != 0) {
		if (errno != ENOENT) {
		    fprintf(stderr,MSGSTR(MSG_UUX_13,
			"permission denied for file %s\n"), _Sfile);
		    cleanup(2);
		}
	    }
	}

	/*
	 * find remote system name
	 * remote name is first to know that 
	 * is not > or <
	 */
	ap = inargs;
	xsys[0] = '\0';
	while ((ap = getprm(ap, NULL, prm)) != NULL) {
		if (prm[0] == '>' || prm[0] == '<') {
			ap = getprm(ap, NULL, prm);
			continue;
		}

		/*
		 * split name into system name
		 * and command name
		 */
		split(prm, xsys, rest);
		break;
	}
	if (xsys[0] == '\0')
		(void) strcpy(xsys, Myname);
	strncpy(Rmtname, xsys, MAXBASENAME);
	Rmtname[MAXBASENAME] = '\0';
	DEBUG(4, "xsys %s\n", xsys);

	/*
	 * check to see if system name is valid
	 */
	if (versys(xsys) != 0) {
		/*
		 * bad system name
		 */
		fprintf(stderr, MSGSTR(MSG_UUX_4, "bad system name: %s\n"), 
			xsys);
		if (fprx != NULL)
			(void) fclose(fprx);
		if (fpc != NULL)
			(void) fclose(fpc);
		cleanup(11);
	}

	/*
	 * determine id of user starting remote 
	 * execution
	 */
	guinfo(Uid, User);
	(void) strcpy(Loginuser,User);

	DEBUG(6, "User %s\n", User);
	if (retaddr == NULL)
		retaddr = User;

	/*
	 * initialize command buffer
	 */
	*cmd = '\0';

	/*
	 * generate JCL files to work from
	 */

	/*
	 * fpc is the C. file for the local site.
	 * collect commands into cfile.
	 * commit if not empty (at end).
	 * 
	 * the appropriate C. file.
	 */
	gename(CMDPRE, xsys, _Grade, cfile);
	DEBUG(9, "cfile = %s\n", cfile);
	ASSERT(access(cfile, 0) != 0, MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), 
		cfile, errno);
	fpc = fdopen(ret = creat(cfile, CFILEMODE), "w");
	ASSERT(ret >= 0 && fpc != NULL, 
		MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), cfile, errno);
	setbuf(fpc, CNULL);

	/*  set Jobid -- C.jobid */
	(void) strncpy(Jobid, BASENAME(cfile, '.'), NAMESIZE);
	Jobid[NAMESIZE-1] = '\0';

	/*
	 * rxfile is the X. file for the job, fprx is its stream ptr.
	 * if the command is to be executed locally, rxfile becomes
	 * a local X. file, otherwise we send it as a D. file to the
	 * remote site.
	 */
	
	gename(DATAPRE, xsys, 'X', rxfile);
	DEBUG(9, "rxfile = %s\n", rxfile);
	ASSERT(access(rxfile, 0) != 0, MSGSTR(MSG_UDEFS_17, "FILE EXISTS"),
		rxfile, errno);
	fprx = fdopen(ret = creat(rxfile, DFILEMODE), "w");
	ASSERT(ret >= 0 && fprx != NULL, 
		MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), rxfile, errno);
	setbuf(fprx, CNULL);
	clearerr(fprx);

	(void) fprintf(fprx,"%c %s %s\n", X_USER, User, Myname);
	if (zflag) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_5,
			"%c return status on success\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDZERO);
	}

	if (nflag) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_6, 
			"%c don't return status on failure\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDNOTHING);
	} else {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_7, 
			"%c return status on failure\n"), X_COMMENT);
		fprintf(fprx,"%c\n", X_NONZERO);
	}

	if (bringback) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_8, 
			"%c return input on abnormal exit\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_BRINGBACK);
	}
	if (_Statop)
		(void) fprintf(fprx,"%c %s\n", X_MAILF, _Sfile);

	if (retaddr != NULL) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_9,
		  "%c return address for status or input return\n"), X_COMMENT);
		(void) fprintf(fprx,"%c %s\n", X_RETADDR, retaddr);
	}

	/*
	 * create a JCL file to spool pipe input into
	 */
	if (pipein) {
		/*
		 * fpd is the D. file into which we now read
		 * input from stdin
		 */
		
		gename(DATAPRE, Myname, 'B', dfile);
		
		ASSERT(access(dfile, 0) != 0, 
			MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), dfile, errno);
		fpd = fdopen(ret = creat(dfile, DFILEMODE), "w");
		ASSERT(ret >= 0 && fpd != NULL, 
			MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), dfile, errno);

		/*
		 * read pipe to EOF
		 */
		while (!feof(stdin)) {
			ret = fread(buf, 1, BUFSIZ, stdin);
			ASSERT(fwrite(buf, 1, ret, fpd) == ret, 
				MSGSTR(MSG_UDEFS_2, "CANNOT_WRITE"),
				dfile, errno);
		}
		ASSERT(ferror(fpd) == 0, MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"),
                       dfile, errno);
		(void) fclose(fpd);

		/*
		 * if command is to be executed on remote
		 * create extra JCL
		 */
		if (!EQUALSN(Myname, xsys, SYSNSIZE)) {
			GENSEND(fpc, dfile, dfile, dfile);
		}

		/*
		 * create file for X_ commands
		 */
		(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
		(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);

		wfcommit(dfile, dfile, xsys);

	}
	/*
	 * parse command
	 */
	ap = inargs;
	while ((ap = getprm(ap, NULL, prm)) != NULL) {
		DEBUG(4, "prm - %s\n", prm);

		/*
		 * redirection of I/O
		 */
		if (prm[0] == '>' || prm[0] == '<') {
			redir = prm[0];
			continue;
		}

		if (prm[0] == '{') {
			char *rquote, *strchr();
			if ((rquote = strchr(prm, '}')) != NULL) {
				redir = '>';
				*(ap-1) = ' ';
				ap = ap - strlen(prm) + 1;
				continue;
			}
		}

		/*
		 * some terminator
		 */
		if ( prm[0] == '|' || prm[0] == '^'
		  || prm[0] == '&' || prm[0] == ';') {
			if (*cmd != '\0')	/* not 1st thing on line */
				APPCMD(prm);
			command = TRUE;
			continue;
		}

		/*
		 * process command or file or option
		 * break out system and file name and
		 * use default if necessary
		 */
		ret = split(prm, syspart, rest);
		DEBUG(4, "syspart -> %s, ", syspart);
		DEBUG(4, "rest -> %s, ", rest);
		DEBUG(4, "ret -> %d\n", ret);

		if (command  && redir == '\0') {
			/*
			 * command
			 */
			APPCMD(rest);
			command = FALSE;
			continue;
		}

		if (syspart[0] == '\0') {
			(void) strcpy(syspart, Myname);
			DEBUG(6, "syspart -> %s\n", syspart);
		} else if (versys(syspart) != 0) {
			/*
			 * bad system name
			 */
			fprintf(stderr, MSGSTR(MSG_UUX_11,
			  "bad system name: %s\n"), syspart);
			if (fprx != NULL)
				(void) fclose(fprx);
			if (fpc != NULL)
				(void) fclose(fpc);
			cleanup(11);
		}

		/*
		 * process file or option
		 */

		/*
		 * process file argument
		 * expand filename and create JCL card for
		 * redirected output
		 * e.g., X file sys
		 */
		if (redir == '>') {
			if (ckexpf(rest))
				cleanup(6);
			ASSERT(fprintf(fprx, "%c %s %s\n", X_STDOUT, rest,
			 syspart) >= 0, MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"),
                            rxfile, errno);
			redir = '\0';
			continue;
		}

		/*
		 * if no system specified, then being
		 * processed locally
		 */
		if (ret == NOSYSPART && redir == '\0') {

			/*
			 * option
			 */
			APPCMD(rest);
			continue;
		}


		/* local xeqn + local file  (!x !f) */
		if ((EQUALSN(xsys, Myname, SYSNSIZE))
		 && (EQUALSN(xsys, syspart, SYSNSIZE))) {
			/*
			 * create JCL card
			 */
			if (ckexpf(rest))
				cleanup(7);
			/*
			 * JCL card for local input
			 * e.g., I file
			 */
			if (redir == '<') {
				(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
			} else
				APPCMD(rest);
			ASSERT(fprx != NULL, 
				MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
				rxfile, errno);
			redir = '\0';
			continue;
		}

		/* remote xeqn + local file (sys!x !f) */
		if (EQUALSN(syspart, Myname, SYSNSIZE)) {
			/*
			 * check access to local file
			 * if cflag is set, copy to spool directory
			 * otherwise, just mention it in the X. file
			 */
			if (ckexpf(rest))
				cleanup(6);
			DEBUG(4, "rest %s\n", rest);

			/* see if I can read this file as read uid, gid */
			/* osf note: "accessx" is an aix specific routine */
			if ((accessx(rest,R_OK,ACC_INVOKER)) != 0) {
				fprintf(stderr,MSGSTR(MSG_UUX_13,
				   "permission denied %s\n"), rest);
				cleanup(1);
			}

			/* D. file for sending local file */
			gename(DATAPRE, xsys, 'A', dfile);

			if (cflag) {
				/* make local copy */
				if (uidxcp(rest, dfile) != 0) {
				    fprintf(stderr,MSGSTR(MSG_UUX_15,
					"can't copy %s\n"), rest);
				    cleanup(5);
				}
				(void) chmod(dfile, DFILEMODE);
				/* generate 'send' entry in command file */
				GENSEND(fpc, rest, dfile, dfile);
				wfcommit(dfile, dfile, xsys);	/* commit input */
			} else		/* don't make local copy */
				GENSEND(fpc, rest, dfile, "D.0");

			/*
			 * JCL cards for redirected input in X. file,
			 * e.g.
			 * I D.xxx
			 * F D.xxx
			 */
			if (redir == '<') {
				/*
				 * don't bother making a X_RQDFILE line that
				 * renames stdin on the remote side, since the
				 * remote command can't know its name anyway
				 */
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
				(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
			} else {
				APPCMD(BASENAME(rest, '/'));;
				/*
				 * generate X. JCL card that specifies
				 * F file 
				 */
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
			}
			redir = '\0';

			continue;
		}

		/* local xeqn + remote file (!x sys!f ) */
		if (EQUALS(Myname, xsys)) {
			/*
			 * expand receive file name
			 */
			if (ckexpf(rest))
				cleanup(6);
			/*
			 * tfile is command file for receive from remote.
			 * we defer commiting until later so 
			 * that only one C. file is created per site.
			 *
			 * dfile is name of data file to receive into;
			 * we don't use it, just name it.
			 *
			 * the name of the remote is appended to the
			 * X_RQDFILE line to help uuxqt in finding the
			 * D. file when it arrives.
			 */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    	MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), 
					tfile, errno);
				svcfile(tfile, syspart);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, MSGSTR(MSG_UDEFS_1,"CANNOT OPEN"),
				tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);

			/* prepare JCL card to receive file */
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, 
				MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
				dfile, errno);
			(void) fclose(fp);
			rflag++;
			if (rest[0] == '~')
				if (ckexpf(rest))
					cleanup(7);

			/*
			 * generate receive entries
			 */
			if (redir == '<') {
				(void) fprintf(fprx,
					"%c %s/%s/%s\n", X_RQDFILE, Spool,
					syspart, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s/%s/%s %s\n",
				X_RQDFILE, Spool, syspart, dfile,
				BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}

			redir = '\0';
			continue;
		}

		/* remote xeqn/file, different remotes (xsys!cmd syspart!rest) */
		if (!EQUALS(syspart, xsys)) {
			/*
			 * strategy:
			 * request rest from syspart.
			 *
			 * set up a local X. file that will send rest to xsys,
			 * once it arrives from syspart.
			 *
			 * arrange so that the xsys D. file (fated to become
			 * an X. file on xsys), rest is required and named.
			 *
			 * pictorially:
			 *
			 * ===== syspart/C.syspartR.... =====	(tfile)
			 * R rest D.syspart...			(dfile)
			 *
			 *
			 * ===== local/X.local... =====		(t2file)
			 * F Spool/syspart/D.syspart... rest	(dfile)
			 * C uucp -C rest D.syspart...		(dfile)
			 *
			 * ===== xsys/D.xsysG....		(fprx)
			 * F D.syspart... rest			(dfile)
			 * or, in the case of redir == '<'
			 * F D.syspart...			(dfile)
			 * I D.syspart...			(dfile)
			 *
			 * while we do push rest around a bunch,
			 * we use the protection scheme to good effect.
			 *
			 * we must rely on uucp's treatment of requests
			 * form XQTDIR to get the data file to the right
			 * place ultimately.
			 */

			/* build (or append to) C.syspartR... */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    	MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), 
					tfile, errno);
				svcfile(tfile, syspart);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, MSGSTR(MSG_UDEFS_1,"CANNOT OPEN"),
				tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, 
				MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
				dfile, errno);
			(void) fclose(fp);

			/* build local/X.localG... */
			gename(XQTPRE, Myname, _Grade, t2file);
			ASSERT(access(t2file, 0)!=0, 
				MSGSTR(MSG_UDEFS_17, "FILE EXISTS"), 
				t2file, errno);
			(void) close(creat(t2file, CFILEMODE));
			fp = fopen(t2file, "w");
			ASSERT(fp != NULL, 
				MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), 
				t2file, errno);
			setbuf(fp, CNULL);
			(void) fprintf(fp, "%c %s/%s/%s %s\n", X_RQDFILE,
				Spool, syspart, dfile, BASENAME(rest, '/'));
			(void) fprintf(fp, "%c uucp -C %s %s!%s\n",
				X_CMD, BASENAME(rest, '/'), xsys, dfile);
			ASSERT(ferror(fp) == 0, 
				MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
				t2file, errno);
			(void) fclose(fp);

			/* put t2file where uuxqt can get at it */
			wfcommit(t2file, t2file, Myname);

			/* generate xsys/X.sysG... cards */
			if (redir == '<') {
				(void) fprintf(fprx, "%c %s\n",
					X_RQDFILE, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}
			redir = '\0';
			continue;
		}

		/* remote xeqn + remote file, same remote (sys!x sys!f) */
		if (rest[0] == '~')	/* expand '~' on remote */
			if (ckexpf(rest))
				cleanup(7);
		if (redir == '<') {
			(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
		}
		else
			APPCMD(rest);
		redir = '\0';
		continue;

	}

	/*
	 * place command to be executed in JCL file
	 */
	(void) fprintf(fprx, "%c %s\n", X_CMD, cmd);
	ASSERT(ferror(fprx) == 0, 
		MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
		rxfile, errno);
	(void) fclose(fprx);		/* rxfile is ready for commit */
	logent(cmd, "QUEUED");

	/* the X. name must have the X.Jobid so status reporting is by jobid */
	(void) sprintf(tfile, "X.%s", Jobid);
	if (EQUALS(xsys, Myname)) {
		/* local xeqn -- use X_ file here */
		wfcommit(rxfile, tfile, xsys);
		
		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuuxqt(Myname, old_behavior);
	} else {
		/* remote xeqn -- send rxfile to remote */
		/* put it in a place where cico can get at it */
		wfcommit(rxfile, rxfile, xsys);

		GENSEND(fpc, rxfile, tfile, rxfile);
	}

	cfileUsed = (ftell(fpc) != 0L);	/* was cfile used? */
	ASSERT(ferror(fpc) == 0, MSGSTR(MSG_UDEFS_2, "CANNOT WRITE"), 
		cfile, errno);
	(void) fclose(fpc);

	/*
	 * has any command been placed in command JCL file
	 */
	if (cfileUsed) {
		wfcommit(cfile, cfile, xsys);
		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuucico(xsys);
	} else
		unlink(cfile);

	/* commit any leftover C. files for remote receive */
	commitall();
	if (jflag)	/* print Jobid */
	    printf("%s\n", Jobid);

	cleanup(0);
}


/*
 * cleanup and unlink if error
 *	code	-> exit code
 * return:
 *	none
 */
cleanup(code)
register int code;
{
	rmlock(CNULL);
	if (code) {
		wfabort();
		fprintf(stderr, MSGSTR(MSG_UUX_14,
			"uux failed ( %d )\n"), code);
	}
	DEBUG(1, "exit code %d\n", code);
	catclose(catd);

	if (code < 0)
		exit(-code);
	else
		exit(code);
}

/*
 * catch signal then cleanup and exit
 */
void onintr(register int inter)
{
	char str[30];
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "XSIGNAL %d", inter);
	logent(str, "XCAUGHT");
	cleanup(-inter);
}

static
usage()
{

	(void) fprintf(stderr, MSGSTR(MSG_UUX_16, USAGE));
	exit(2);
}
