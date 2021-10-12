static char sccsid[] = "@(#)82	1.22  src/bos/usr/bin/uucp/cico.c, cmduucp, bos41J, 9515A_all 4/11/95 16:12:32";
/* 
 * COMPONENT_NAME: CMDUUCP cico.c
 * 
 * FUNCTIONS: Mcico, TMname, cleanTM, cleanup, closedem, intrEXIT, 
 *            onintr, pskip, timeout 
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

/*
**	cico.c
**
** uucp file transfer program:
** to place a call to a remote machine, login, and
** copy files between the two machines.
*/

#include "uucp.h"
/* VERSION( cico.c	5.3 -  -  ); */

nl_catd catd;
#ifndef	V7
#include <sys/sysmacros.h>
#endif

#ifdef TLI
#include        <sys/tiuser.h>
#endif /* TLI */

jmp_buf Sjbuf;
extern int Errorrate;
char	uuxqtarg[MAXBASENAME] = {'\0'};
char Rmtalias[20];

extern int	(*Setup)(), (*Teardown)();	/* defined in interface.c */

int	maxmsg = MAXMSGTIME;
int	maxstart = MAXSTART;
char    xfergrade;				/* File xfer cutoff grade */

#define USAGE   "Usage: uucico [-xNUM] [-r[0|1]] [-gGrade] -sSYSTEM [-uUSERID] [-dSPOOL]\n"

extern void closedem();
mode_t omask;

static void intrEXIT(int), onintr(int), timeout(int);
static char *pskip(register char *);
extern char Loginuser[], User[];	/* From uucpdefs.c */

main(argc, argv, envp)
char *argv[];
char **envp;
{
#ifdef NOSTRANGERS
        void checkrmt();
#endif /* NOSTRANGERS */

	int ret, seq, exitcode,i;
	char file[NAMESIZE];
	char msg[BUFSIZ], *p, *q;
	char xflag[6];	/* -xN N is single digit */
	char *ttyn;
	char *iface;	/* interface name	*/
	char	cb[128];
	time_t	ts, tconv;
#ifndef	V7
	long 	minulimit, dummy;
#endif V7
	int check_work = 0;  /* set by -c flag */
	register DIR *dirp;
	register struct dirent *f = NULL;

	Ulimit = ulimit(1,0L);
	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	if (Uid == 0)
	    setuid(UUCPUID);	/* fails in ATTSV, but so what? */
	Env = envp;
	Role = SLAVE;
	strcpy(Logfile, LOGCICO);
	*Rmtname = NULLCHAR;
	Rmtalias[0] = NULLCHAR;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);

	closedem();
	time(&Nstat.t_qtime);
	tconv = Nstat.t_start = Nstat.t_qtime;
	strcpy(Progname, "uucico");
	Debug = 1;              /* Set it in case can't read config files */
        setservice(Progname);
        ret = sysaccess(EACCESS_SYSTEMS);
        ASSERT(ret == 0, MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), "Systems", ret);
        ret = sysaccess(EACCESS_DEVICES);
        ASSERT(ret == 0, MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), "Devices", ret);
        ret = sysaccess(EACCESS_DIALERS);
        ASSERT(ret == 0, MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), "Dialers", ret);
	Debug = 0;              /* Set it back in case it won't be specified */
	Pchar = 'C';
	(void) signal(SIGILL, (void(*)(int)) intrEXIT);
	(void) signal(SIGTRAP, (void(*)(int)) intrEXIT);
	(void) signal(SIGIOT, (void(*)(int)) intrEXIT);
	(void) signal(SIGDANGER, (void(*)(int)) intrEXIT);
	(void) signal(SIGFPE, (void(*)(int)) intrEXIT);
	(void) signal(SIGBUS, (void(*)(int)) intrEXIT);
	(void) signal(SIGSEGV, (void(*)(int)) intrEXIT);
	(void) signal(SIGSYS, (void(*)(int)) intrEXIT);
	if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)	/* This for sockets */
		(void) signal(SIGPIPE, (void(*)(int)) intrEXIT);
	(void) signal(SIGINT, (void(*)(int)) onintr);
	(void) signal(SIGHUP, (void(*)(int)) onintr);
	(void) signal(SIGQUIT, (void(*)(int)) onintr);
	(void) signal(SIGTERM, (void(*)(int)) onintr);
#ifdef ATTSV
	(void) signal(SIGUSR1, SIG_IGN);
	(void) signal(SIGUSR2, SIG_IGN);
#endif
        ret = guinfo(Euid, User);    /* Check for bad setuid on uucico perms */
	ASSERT(ret == 0, MSGSTR(MSG_CICOA1,"BAD UID "), "", ret);
	strncpy(Uucp, User, NAMESIZE);

	setuucp(User);

	ret = guinfo(Uid, Loginuser); /* Check for bad login id */
	ASSERT(ret == 0, MSGSTR(MSG_CICOA2,"BAD LOGIN_UID "), "", ret);

	*xflag = NULLCHAR;
	Ifn = Ofn = -1;
	iface = "UNIX";

	while ((ret = getopt(argc, argv, "d:r:s:t:x:u:g:c")) != EOF) {
		switch (ret) {
		case 'd':
                        if ( eaccess(optarg, 01) != 0 ) {
                                (void) fprintf(stderr,  "%s%s%s", MSGSTR(MSG_CICOA4,
                                     "cannot access spool directory\n"),
                                        Progname, optarg);
                                exit(1);
			}
			Spool = optarg;
			break;
		case 'r':
                        if ( (Role = atoi(optarg)) != MASTER && Role != SLAVE )
			{
				(void) fprintf(stderr,
					"uucico: bad value '%s' for -r argument\n\t%s",
					optarg, MSGSTR(MSG_CICO24, USAGE));
			}
			break;
		case 's':
			if (versys(optarg)) {
				fprintf(stderr, "%s", MSGSTR(MSG_BADSYSTEM,
					"System not in Systems file"));
			    cleanup(101);
			}
			strncpy(Rmtname, optarg, MAXBASENAME);
			Rmtname[MAXBASENAME] = '\0';
			/* set args for possible xuuxqt call */
			strcpy(uuxqtarg, Rmtname);
			break;
		case 't':
			if (strlen(optarg) != strspn(optarg, "1234567890")) {
			  fprintf(stderr, MSGSTR(MSG_CICO21, "Invalid timeout value\n"));
			  exit(1);
			}
			maxmsg = atoi(optarg);
			maxstart = (atoi(optarg) * 10);
			if (maxmsg < 1) {
			  fprintf(stderr, MSGSTR(MSG_CICO21, "Invalid timeout value\n"));
			  exit(1);
			}
			break;
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			if (Debug > 9)
				Debug = 9;
			(void) sprintf(xflag, "-x%d", Debug);
			break;
		case 'c':  /* don't call if there's no local work to do */
			++check_work;
			break;
		case 'u':
			DEBUG(4, "Loginuser %s specified\n", optarg);
			strncpy(Loginuser, optarg, NAMESIZE);
			Loginuser[NAMESIZE - 1] = NULLCHAR;
			break;
		case 'g':  /* Set transfer grade cutoff value */
			if (isalnum(*optarg))
				xfergrade = *optarg;
			else
				(void) fprintf(stderr, MSGSTR(MSG_CICO23,
				"uucico: Ignoring invalid xfer grade of %c\n"),
				*optarg);
			break;
		default:
			(void) fprintf(stderr, MSGSTR(MSG_CICO24, USAGE));
			exit(1);
		}
	}

	/*
	** maxmsg and maxstart were added to allow users to vary the
	** startup time for uucico.
	*/

	DEBUG(7, "maxstart = %d\n", maxstart);
	DEBUG(7, "maxmsg = %d\n", maxmsg);

	if (Role == MASTER || *Loginuser == NULLCHAR) {
		ret = guinfo(Uid, Loginuser);
		ASSERT(ret == 0, "BAD LOGIN_UID ", "", ret);
	}

	if (Role == MASTER) {
	    if (*Rmtname == NULLCHAR) {
		DEBUG(5, "No -s specified\n" , "");
		cleanup(101);
	    }
	    /* get Myname - it depends on who I'm calling--Rmtname */
	    (void) mchFind(Rmtname);
	    myName(Myname);
	    if (EQUALSN(Rmtname, Myname, SYSNSIZE)) {
		DEBUG(5, "This system specified: -sMyname: %s, ", Myname);
		cleanup(101);
	    }
	}

	ASSERT(chdir(Spool) == 0, MSGSTR(MSG_UDEFS_11, "CANNOT CHDIR"), \
		Spool, errno);
	strcpy(Wrkdir, Spool);

	if (Role == SLAVE) {

		if (freopen(RMTDEBUG, "a", stderr) == 0) {
			errent(MSGSTR(MSG_UDEFS_1, "CANNOT OPEN"), 
				RMTDEBUG, errno, sccsid, __FILE__, __LINE__);
			freopen("/dev/null", "w", stderr);
		}
		if ( interface(iface) ) {
			(void)fprintf(stderr,
			"%s: invalid interface %s\n", Progname, iface);
			cleanup(101);
		}
		/*master setup will be called from processdev()*/
		if ( (*Setup)( Role, &Ifn, &Ofn ) ) {
			DEBUG(5, "SLAVE Setup failed", "");
			cleanup(101);
		}

		/*
		 * initial handshake
		 */
		ret = savline();
		Ifn = 0;
		Ofn = 1;
		fixline(Ifn, 0, D_ACU);
		/* get MyName - use logFind to check PERMISSIONS file */
		(void) logFind(Loginuser, "");
		myName(Myname);

		DEBUG(4,"cico.c: Myname - %s\n",Myname);
		DEBUG(4,"cico.c: Loginuser - %s\n",Loginuser);
		fflush(stderr);
		Nstat.t_scall = times(&Nstat.t_tga);
		(void) sprintf(msg, "here=%s", Myname);
		omsg('S', msg, Ofn);
		(void) signal(SIGALRM, (void(*)(int)) timeout);
		(void) alarm(2 * MAXMSGTIME);	/* give slow machines a second chance */
		if (setjmp(Sjbuf)) {

			/*
			 * timed out
			 */
			ret = restline();
			clrlock(CNULL); /* rm tty locks */
			rmlock(CNULL);
			exit(0);
		}
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				(void) alarm(0);
				ret = restline();
				clrlock(CNULL);	/* rm tty locks */
				rmlock(CNULL);
				exit(0);
			}
			if (msg[0] == 'S')
				break;
		}
		Nstat.t_ecall = times(&Nstat.t_tga);
		(void) alarm(0);
		q = &msg[1];
		p = pskip(q);
		strncpy(Rmtname, q, MAXBASENAME);
		Rmtname[MAXBASENAME] = '\0';

		seq = 0;
		while (*p == '-') {
			q = pskip(p);
			switch(*(++p)) {
			case 'x':
				Debug = atoi(++p);
				if (Debug <= 0)
					Debug = 1;
				break;
			case 'Q':
				seq = atoi(++p);
				break;
			default:
				break;
			}
			p = q;
		}
		DEBUG(4, "sys-%s\n", Rmtname);

#ifdef NOSTRANGERS
		checkrmt();	/* Do we know the remote system? */
#endif NOSTRANGERS

		if (ttylock(Rmtname)) {
			omsg('R', "LCK", Ofn);
			cleanup(101);
		}
		
		/* validate login using PERMISSIONS file */
		if (logFind(Loginuser, Rmtname) == FAIL) {
			Uerror = SS_BAD_LOG_MCH;
			logent(UerrorText(Uerror), MSGSTR(MSG_CICOL1,"FAILED"));
			systat(Rmtname, SS_BAD_LOG_MCH, UerrorText(Uerror),
			    Retrytime);
			omsg('R', "LOGIN", Ofn);
			cleanup(101);
		}

		ret = callBack();
		DEBUG(4,"return from callcheck: %s",ret ? "TRUE" : "FALSE");
		if (ret==TRUE) {
			(void) signal(SIGINT, SIG_IGN);
			(void) signal(SIGHUP, SIG_IGN);
			omsg('R', "CB", Ofn);
			logent(MSGSTR(MSG_CICOL2,"CALLBACK"), 
			       MSGSTR(MSG_CICOL3,"REQUIRED"));

			/*
			 * set up for call back
			 */
			systat(Rmtname, SS_CALLBACK, MSGSTR(MSG_CICO2,
				"CALL BACK"), Retrytime);
			gename(CMDPRE, Rmtname, 'C', file);
			chremdir(Rmtname);  /* create file in RemSpool dir */
			(void) close(creat(file, CFILEMODE));
			ttyunlock(Rmtname);  /* give up lock first! */
			xuucico(Rmtname);
			cleanup(101);
		}

		if (callok(Rmtname) == SS_SEQBAD) {
			Uerror = SS_SEQBAD;
			logent(UerrorText(Uerror), 
				MSGSTR(MSG_CICOL4,"PREVIOUS"));
			omsg('R', "BADSEQ", Ofn);
			cleanup(101);
		}

		if ((ret = gnxseq(Rmtname)) == seq) {
			omsg('R', "OK", Ofn);
			cmtseq();
		} else {
			Uerror = SS_SEQBAD;
			systat(Rmtname, SS_SEQBAD, UerrorText(Uerror), 
				Retrytime);
			logent(UerrorText(Uerror), MSGSTR(MSG_CICOL5,
				"HANDSHAKE FAILED"));
			ulkseq();
			omsg('R', "BADSEQ", Ofn);
			cleanup(101);
		}
		ttyn = ttyname(Ifn);
		if (ttyn != NULL) {
			struct stat ttysbuf;
			if ( fstat(Ifn,&ttysbuf) == 0 )
				Dev_mode = ttysbuf.st_mode;
			else
				Dev_mode = R_DEVICEMODE;
			strcpy(Dc, BASENAME(ttyn, '/'));
			chmod(ttyn, S_DEVICEMODE);/* can fail, but who cares? */
		} else
			strcpy(Dc, "notty");
		/* set args for possible xuuxqt call */
		strcpy(uuxqtarg, Rmtname);
	}

	strcpy(User, Uucp);
/*
 *  Ensure reasonable ulimit (MINULIMIT)
 */

#ifndef	V7
	minulimit = ulimit(1,dummy);
	ASSERT(minulimit >= MINULIMIT, MSGSTR(MSG_CICOA3,"ULIMIT TOO SMALL"),
	    Loginuser, minulimit);
#endif
	if (Role == MASTER && callok(Rmtname) != 0) {
		logent(MSGSTR(MSG_CICOL21,"SYSTEM STATUS"), 
		       MSGSTR(MSG_CICOL6,"CAN NOT CALL"));
		cleanup(101);
	}

	chremdir(Rmtname);

	(void) strcpy(Wrkdir, RemSpool);
	if (Role == MASTER) {

		/*
		 * master part
		 */
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		if (Ifn != -1 && Role == MASTER) {
			(void) (*Write)(Ofn, EOTMSG, strlen(EOTMSG));
			(void) close(Ofn);
			(void) close(Ifn);
			Ifn = Ofn = -1;
			clrlock(CNULL);
			rmlock(CNULL);
			sleep(3);
		}
		(void) sprintf(msg, MSGSTR(MSG_CICO9,"call to %s "), Rmtname);
		if (ttylock(Rmtname) != 0) {
			logent(msg, MSGSTR(MSG_CICOL22,"LOCKED"));
			CDEBUG(1, "Currently Talking With %s\n",
			    Rmtname);
 			cleanup(100);
		}

		/* if we have been given the check_work flag, check the
		   work queue before making the call and exit if there's
		   no local work to do */
		
		if (check_work) {
		    if (dirp = opendir(RemSpool)) {
			while (f = readdir(dirp))
			    if (f->d_name[0] == 'C' && f->d_name[1] == '.')
				break;  /* found a work file */
			closedir(dirp);
		    }
		    if (! f) {  /* no work: exit more or less quietly */
			ttyunlock(Rmtname);
			logent(MSGSTR(MSG_CICOL17,"conversation complete"), 
			    MSGSTR(MSG_CICO4,"OK"));  /* XXX */
			cleanup(SS_OK);  /* XXX */
		    }
		}

		Nstat.t_scall = times(&Nstat.t_tga);
		Ofn = Ifn = conn(Rmtname);
		Nstat.t_ecall = times(&Nstat.t_tga);
		if (Ofn < 0) {
			ttyunlock(Rmtname);
			logent(UerrorText(Uerror), 
				MSGSTR(MSG_CICOL7,"CONN FAILED"));
			systat(Rmtname, Uerror, UerrorText(Uerror), Retrytime);
			cleanup(101);
		} else {
			logent(msg, MSGSTR(MSG_CICOL8,"SUCCEEDED"));
			ttyn = ttyname(Ifn);
                        if (ttyn != NULL) {
				struct stat ttysbuf;
				if ( fstat(Ifn,&ttysbuf) == 0 )
					Dev_mode = ttysbuf.st_mode;
				else
					Dev_mode = R_DEVICEMODE;
				chmod(ttyn, M_DEVICEMODE);
			}
		}
	
		if (setjmp(Sjbuf)) {
			ttyunlock(Rmtname);
			Uerror = SS_LOGIN_FAILED;
			logent(Rmtname, UerrorText(Uerror));
			systat(Rmtname, SS_LOGIN_FAILED,
			    UerrorText(Uerror), Retrytime);
			DEBUG(4, "%s - failed\n", UerrorText(Uerror));
			cleanup(101);
		}
		(void) signal(SIGALRM, (void(*)(int)) timeout);
		/* give slow guys lots of time to thrash */
		(void) alarm(3 * MAXMSGTIME);
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				continue; /* try again */
			}
			if (msg[0] == 'S')
				break;
		}
		(void) alarm(0);
		if(EQUALSN("here=", &msg[1], 5)){
			/*
			 * this is a problem.  We'd like to compare with an
			 * untruncated Rmtname but we fear incompatability.
			 * So we'll look at most 6 chars (at most).
			 */
			if(!EQUALSN(&msg[6], Rmtname, (strlen(Rmtname)< 7 ?
						strlen(Rmtname) : 6))){
				if(!EQUALSN(&msg[6], Rmtalias, 
					(strlen(Rmtalias)< 7 ?
					strlen(Rmtalias) : 6))){
					ttyunlock(Rmtname);
					Uerror = SS_WRONG_MCH;
					logent(&msg[6], UerrorText(Uerror));
					systat(Rmtname, SS_WRONG_MCH, 
						UerrorText(Uerror), Retrytime);
					DEBUG(4, "%s - failed\n", 
						UerrorText(Uerror));
					cleanup(101);
				}
			}
		}
		CDEBUG(1,MSGSTR(MSG_CICOCD1,"Login Successful: System=%s\n"),
			&msg[6]);
		seq = gnxseq(Rmtname);
		(void) sprintf(msg, "%s -Q%d %s", Myname, seq, xflag);
		omsg('S', msg, Ofn);
		(void) alarm(2 * MAXMSGTIME);	/* give slow guys some thrash time */
		for (;;) {
			ret = imsg(msg, Ifn);
			DEBUG(4, "msg-%s\n", msg);
			if (ret != 0) {
				(void) alarm(0);
				ttyunlock(Rmtname);
				ulkseq();
				cleanup(101);
			}
			if (msg[0] == 'R')
				break;
		}
		(void) alarm(0);

		/*  check for rejects from remote */
		Uerror = 0;
		if (EQUALS(&msg[1], "LCK")) 
			Uerror = SS_RLOCKED;
		else if (EQUALS(&msg[1], "LOGIN"))
			Uerror = SS_RLOGIN;
		else if (EQUALS(&msg[1], "CB"))
			Uerror = SS_CALLBACK;
		else if (EQUALS(&msg[1], "You are unknown to me"))
			Uerror = SS_RUNKNOWN;
		else if (EQUALS(&msg[1], "BADSEQ"))
			Uerror = SS_SEQBAD;
		else if (!EQUALS(&msg[1], "OK"))
			Uerror = SS_UNKNOWN_RESPONSE;
		if (Uerror)  {
			ttyunlock(Rmtname);
			systat(Rmtname, Uerror, UerrorText(Uerror), Retrytime);
			logent(UerrorText(Uerror), MSGSTR(MSG_CICOL5,
				"HANDSHAKE FAILED"));
			CDEBUG(1, MSGSTR(MSG_CICOCD2,"HANDSHAKE FAILED: %s\n"),
				 UerrorText(Uerror));
			ulkseq();
			cleanup(101);
		}
		cmtseq();
	}
	DEBUG(4, " Rmtname %s, ", Rmtname);
	DEBUG(4, "Role %s,  ", Role ? "MASTER" : "SLAVE");
	DEBUG(4, "Ifn - %d, ", Ifn);
	DEBUG(4, "Loginuser - %s\n", Loginuser);

	/* alarm/setjmp added here due to experience with uucico
	 * hanging for hours in imsg().
	 */
	if (setjmp(Sjbuf)) {
		ttyunlock(Rmtname);
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICOL11,
				"TIMEOUT"));
		DEBUG(4, "%s - timeout\n", "startup");
		cleanup(101);
	}
	(void) alarm(maxstart);
	ret = startup(Role);
	(void) alarm(0);

	if (ret != SUCCESS) {
		ttyunlock(Rmtname);
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICOL1,
			"FAILED"));
		Uerror = SS_STARTUP;
		CDEBUG(1, "%s\n", UerrorText(Uerror));
		systat(Rmtname, Uerror, UerrorText(Uerror), Retrytime);
		exitcode = 101;
	} else {
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICO4,"OK"));
		systat(Rmtname, SS_INPROGRESS, UerrorText(SS_INPROGRESS),Retrytime);
		Nstat.t_sftp = times(&Nstat.t_tga);

		exitcode = cntrl(Role);
		Nstat.t_eftp = times(&Nstat.t_tga);
		DEBUG(4, "cntrl - %d\n", exitcode);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGALRM, (void(*)(int)) timeout);

		if (exitcode == 0) {
			(void) time(&ts);
			(void) sprintf(cb, MSGSTR(MSG_CICO7,
				"conversation complete %s %ld"),Dc, ts - tconv);
			logent(cb, MSGSTR(MSG_CICO4,"OK"));
			systat(Rmtname, SS_OK, UerrorText(SS_OK), Retrytime);

		} else {
			logent(MSGSTR(MSG_CICOL17,"conversation complete"), 
				MSGSTR(MSG_CICOL1,"FAILED"));
			systat(Rmtname, SS_CONVERSATION,
			    UerrorText(SS_CONVERSATION), Retrytime);
		}
		(void) alarm(2 * MAXMSGTIME);	/* give slow guys some thrash time */
		omsg('O', "OOOOO", Ofn);
		CDEBUG(4, MSGSTR(MSG_CICOCD3,"send OO %d,"), ret);
		if (!setjmp(Sjbuf)) {
			for (;;) {
				omsg('O', "OOOOO", Ofn);
				ret = imsg(msg, Ifn);
				if (ret != 0)
					break;
				if (msg[0] == 'O')
					break;
			}
		}
		(void) alarm(0);
	}
	cleanup(exitcode);
}

/*
 * clean and exit with "code" status
 */
cleanup(code)
	register int code;
{
	int ret;
	char *ttyn;

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	clrlock(CNULL);
	rmlock(CNULL);
        (*Teardown)( Role, Ifn, Ofn );
	DEBUG(4, "exit code %d\n", code);
	if (code) {
		CDEBUG(1, "%s", MSGSTR(MSG_CICOCD4, 
			"Conversation Complete: Status FAILED\n\n"));
	}
	else {
		CDEBUG(1, "%s", MSGSTR(MSG_CICOCD5, 
			"Conversation Complete: Status SUCCEEDED\n\n"));
	}

	closedem();   
	cleanTM();
	if (code == 0)
		/* Have uuxqt do all remote job requests.
		   If only the Rmtname jobs are done then
		   no local X. files will get executed even
		   though remote data files are now present. */
		xuuxqt(NULL, 0);
	catclose(catd);
	exit(code);
}

short TM_cnt = 0;
char TM_name[MAXNAMESIZE];

cleanTM()
{
	register int i;
	char tm_name[MAXNAMESIZE];

	DEBUG(7,"TM_cnt: %d\n",TM_cnt);
	for(i=0; i < TM_cnt; i++) {
		(void) sprintf(tm_name, "%s.%3.3d", TM_name, i);
		DEBUG(7, "tm_name: %s\n", tm_name);
		unlink(tm_name);
	}
}

TMname(file, pnum)
char *file;
{

	(void) sprintf(file, "%s/TM.%.5d.%.3d", RemSpool, pnum, TM_cnt);
	if (TM_cnt == 0)
	    (void) sprintf(TM_name, "%s/TM.%.5d", RemSpool, pnum);
	DEBUG(7, "TMname(%s)\n", file);
	TM_cnt++;
}

/*
 * intrrupt - remove locks and exit
 */
static void onintr(register int inter)
{
	char str[30];
	/* I'm putting a test for zero here because I saw it happen
	 * and don't know how or why, but it seemed to then loop
	 * here for ever?
	 */
	if (inter == 0)
	    exit(99);
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "SIGNAL %d", inter);
	logent(str, MSGSTR(MSG_CICOL19,"CAUGHT"));
	cleanup(inter);
}

/*ARGSUSED*/
static void intrEXIT(int inter)
{
	char	cb[10];
	extern int errno;

	(void) sprintf(cb, "%d", errno);
	logent(MSGSTR(MSG_CICOL20,"INTREXIT"), cb);
	(void) signal(SIGIOT, SIG_DFL);
	(void) signal(SIGILL, SIG_DFL);
	clrlock(CNULL);
	rmlock(CNULL);
	closedem();
	(void) setuid(Uid);
	abort();
}

/*
 * catch SIGALRM routine
 */
static void timeout(int s)
{
	longjmp(Sjbuf, 1);
}

static char *
pskip(register char *p)
{
	while( *p && *p != ' ' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}

void
closedem()
{
        register i, maxfiles;

#ifdef ATTSVR3
        maxfiles = ulimit(4,0);
#else /* !ATTSVR3 */  /* AIX goes here. */
        maxfiles = _NFILE;
#endif /* ATTSVR3 */

        for (  i = 3; i < maxfiles; i++ )
                if ( i != Ifn && i != Ofn && i != fileno(stderr) )
                        (void) close(i);
}

#ifdef NOSTRANGERS

/*
 *      checkrmt()
 *
 *      if the command NOSTRANGERS is executable and if we don't
 *      know the remote system (i.e., it doesn't appear in the
 *      Systems file), run NOSTRANGERS to log the attempt and hang up.
 */

static void
checkrmt()
{
	int     pid, waitrv;

	if ( (access(NOSTRANGERS, 1) == 0) && versys(Rmtname)) {

		DEBUG(5, "Invoking NOSTRANGERS for %s\n", Rmtname);
		(void) signal(SIGHUP, SIG_IGN);
		errno = 0;

		if ( (pid = fork()) == 0 ) {
			execlp(NOSTRANGERS, "stranger",
					Rmtname, (char *)NULL);
			perror("cico.c: execlp NOSTRANGERS failed");
		} else if ( pid < 0 ) {
			perror("cico.c: fork failed");
		} else {
			while ( (waitrv = wait((int *)0)) != pid ) {
				if ( waitrv < 0  && errno != EINTR ) {
					perror("cico.c: wait failed");
					break;
				}
			}
		}

		omsg('R', "You are unknown to me", Ofn);
		cleanup(101);
	}
	return;
}
#endif /* NOSTRANGERS */

