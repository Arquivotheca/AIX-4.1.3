static char sccsid[] = "@(#)38	1.11  src/bos/usr/bin/uucp/uusched.c, cmduucp, bos411, 9428A410j 3/5/94 09:30:21";
/* 
 * COMPONENT_NAME: CMDUUCP uusched.c
 * 
 * FUNCTIONS: Muusched, cleanup, exuucico, logent, machine 
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

/*	/sccs/src/cmd/uucp/s.uusched.c
	uusched.c	1.5	7/29/85 16:34:16
*/
#include	"uucp.h"
/* VERSION( uusched.c	5.2 -  -  ); */
nl_catd catd;

#define USAGE	MSGSTR(MSG_UUSCHED1, "[-xNUM] [-uNUM]")

struct m {
	char	mach[15];
	int	ccount;
} M[UUSTAT_TBL+2];

mode_t omask;			/* Linker needs it for utility.c */
short Uopt;
void cleanup();

void logent(){}		/* to load ulockf.c */

main(argc, argv, envp)
char *argv[];
char **envp;
{
	struct m *m, *machine();
	DIR *spooldir, *subdir;
	char *str, *rindex();
#ifdef PDA
	char f[NAME_MAX+256], subf[NAME_MAX+256];
#else
	char f[256], subf[256];
#endif
	short num, snumber;
	char lckname[MAXFULLNAME];
	int i, maxnumb;
	FILE *fp;

	Uopt = 0;
	Env = envp;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


	(void) strcpy(Progname, "uusched");
	while ((i = getopt(argc, argv, "u:x:")) != EOF) {
		switch(i){
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0) {
                                fprintf(stderr, MSGSTR(MSG_UUSCHED4,
                                "WARNING: %s: invalid debug level %s ignored, using level 1\n"),
                                Progname, optarg);

				Debug = 1;
			}
#ifdef SMALL
                        fprintf(stderr, MSGSTR(MSG_UUSCHED5,
                        "WARNING: uusched built with SMALL flag defined -- no debug info available\n"));
#endif /* SMALL */

			break;
		case 'u':
			Uopt = atoi(optarg);
			if (Uopt <= 0) {
                                fprintf(stderr, MSGSTR(MSG_UUSCHED4,
                                "WARNING: %s: invalid debug level %s ignored, using level 1\n"),
                                Progname, optarg);
				Uopt = 1;
			}
			break;
		default:
			(void) fprintf(stderr, MSGSTR(MSG_UUSCHED2, 
				"\tusage: %s %s\n"), Progname, USAGE);
			cleanup(1);
		}
	}
	if (argc != optind) {
		(void) fprintf(stderr, MSGSTR(MSG_UUSCHED2, 
			"\tusage: %s %s\n"), Progname, USAGE);
		cleanup(1);
	}

	DEBUG(9, "Progname (%s): STARTED\n", Progname);
	fp = fopen(LMTUUSCHED, "r");
	if (fp == NULL) {
		DEBUG(1, "No limitfile - %s\n", LMTUUSCHED);
	} else {
		(void) fscanf(fp, "%d", &maxnumb);
		(void) fclose(fp);
		DEBUG(4, "Uusched limit %d -- ", maxnumb);

		for (i=0; i<maxnumb; i++) {
		    (void) sprintf(lckname, "%s.%d", S_LOCK, i);
		    if ( ulockf(lckname, (time_t)  X_LOCKTIME) == 0)
			break;
		}
		if (i == maxnumb) {
		    DEBUG(4, "found %d -- cleanuping\n ", maxnumb);
		    cleanup(0);
		}
		DEBUG(4, "continuing\n", maxnumb);
	}

	bzero(&M[0], sizeof(M));

	if (chdir(SPOOL) != 0 || (spooldir = opendir(SPOOL)) == NULL)
		cleanup(101);		/* good old code 101 */
	while (gnamef(spooldir, f) == TRUE) {
	    if (EQUALSN("LCK..", f, 5))
		continue;

	    if (DIRECTORY(f) && (subdir = opendir(f))) {
	        while (gnamef(subdir, subf) == TRUE)
		    if (subf[1] == '.') {
		        if (subf[0] == CMDPRE) {
				/* Note - we can break now, but the
				 * count may be useful for enhanced
				 * scheduling
				 */
				machine(f)->ccount++;
			}
		    }
		closedir(subdir);
	    }
	}

	/* count the number of systems */
	for (num=0, m=M; m->mach[0] != '\0'; m++, num++) {
	    DEBUG(5, "machine: %s, ", M[num].mach);
	    DEBUG(5, "ccount: %d\n", M[num].ccount);
	}

	DEBUG(5, "Execute num=%d \n", num);
	srand(getpid());  /* Seed random number generator before using */
	while (num > 0) {
	    snumber = (short) (rand() % (long) (num));	/* random num */
	    (void) strcpy(Rmtname, M[snumber].mach);
	    DEBUG(5, "num=%d, ", num);
	    DEBUG(5, "snumber=%d, ", snumber);
	    DEBUG(5, "Rmtname=%s\n", Rmtname);
	    (void) sprintf(lckname, "%s.%s", LOCKPRE, Rmtname);
	    if (checkLock(lckname) != FAIL && callok(Rmtname) == 0) {
		/* no lock file and status time ok */
		DEBUG(5, "call exuucico(%s)\n", Rmtname);
		exuucico(Rmtname);
	    }
	    else {
		/* system locked - skip it */
		DEBUG(5, "system %s locked or inappropriate status--skip it\n",
		    Rmtname);
	    }
	    
	    M[snumber] = M[num-1];
	    num--;
	}
	cleanup(0);
}

struct m	*
machine(name)
char	*name;
{
	struct m *m;
	int	namelen;

	namelen = strlen(name);
	DEBUG(9, "machine(%s) called\n", name);
	for (m = M; m->mach[0] != '\0'; m++)
		/* match on overlap? */
		if (EQUALSN(name, m->mach, SYSNSIZE)) {
			/* use longest name */
			if (namelen > strlen(m->mach))
				(void) strcpy(m->mach, name);
			return(m);
		}

	/*
	 * The table is set up with 2 extra entries
	 * When we go over by one, output error to errors log
	 * When more than one over, just reuse the previous entry
	 */
	if (m-M >= UUSTAT_TBL) {
	    if (m-M == UUSTAT_TBL) {
		errent(MSGSTR(MSG_UUSCHED3, "MACHINE TABLE FULL"), "", 
		UUSTAT_TBL, sccsid, __FILE__, __LINE__);
	    }
	    else
		/* use the last entry - overwrite it */
		m = &M[UUSTAT_TBL];
	}

	(void) strcpy(m->mach, name);
	m->ccount = 0;
	return(m);
}

exuucico(name)
char *name;
{
	char cmd[BUFSIZ];
	short ret;
	char uopt[5];
	char sopt[BUFSIZ];

	(void) sprintf(sopt, "-s%s", name);
	if (Uopt)
	    (void) sprintf(uopt, "-x%.1d", Uopt);

	if (vfork() == 0) {
	    /* pass uucico the undocumented -c flag so that it will not call
	       the remote system unless there is still local work to do */
	    if (Uopt)
	        (void) execle(UUCICO, "UUCICO", "-c", "-r1",
		    uopt, sopt, 0, Env);
	    else
	        (void) execle(UUCICO, "UUCICO", "-c", "-r1", sopt, 0, Env);

	    cleanup(100);
	}
	(void) wait(&ret);

	DEBUG(3, "ret=%d, ", ret);
	DEBUG(3, "errno=%d\n", errno);
}


void
cleanup(code)
int	code;
{
	rmlock(CNULL);
	clrlock(CNULL);
	catclose(catd);

	exit(code);
}
