static char sccsid[] = "@(#)23	1.8  src/bos/usr/bin/uucp/uucheck.c, cmduucp, bos411, 9428A410j 11/11/93 15:07:34";
/* 
 * COMPONENT_NAME: CMDUUCP uucheck.c
 * 
 * FUNCTIONS: Muucheck, canPath, checkPerm, mkdirs, outLine 
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

/*	/sccs/src/cmd/uucp/s.uucheck.c
	uucheck.c	1.5	7/29/85 16:33:35
*/
#define UUCHECK
int Uerrors = 0;	/* error count */

/* This unusual include (#include "permission.c") is done because
 * uucheck wants to use the global static variable in permission.c
 */

#include "uucp.h"
/* VERSION( uucheck.c	5.2 -  -  ); */
#include "permission.c"

nl_catd catd;
/* These are here because uucpdefs.c is not used, and
 * some routines are referenced (never called within uucheck execution)
 * and not included.
 */

#define USAGE	"[-v] [-xNUM]"

int Debug=0;
mkdirs(){}
canPath(){}
char RemSpool[] = SPOOL; /* this is a dummy for chkpth() -- never used here */
char *Spool = SPOOL;
char *Pubdir = PUBDIR;
char *Bnptr;
char	Progname[NAMESIZE];
/* used for READANY and READSOME macros */
struct stat __s_;

/* This is stuff for uucheck */

struct tab
   {
    char *name;
    char *value;
   } tab[] =
   {
#ifdef	CORRUPTDIR
    "CORRUPT",	CORRUPTDIR,
#endif
    "LOGUUCP",	LOGUUCP,
    "LOGUUX",	LOGUUX,
    "LOGUUXQT",	LOGUUXQT,
    "LOGCICO",	LOGCICO,
    "SEQDIR",	SEQDIR,
    "STATDIR",	STATDIR,
    "PFILE",	PFILE,
    "SYSFILE",	SYSFILE,
    "DEVFILE",	DEVFILE	,
    "DIALFILE",	DIALFILE,
    "DIALERFILE",	DIALERFILE,
#ifdef	ETCLOCKS
    "ETCLOCKS",		"/etc/locks",
#endif
#ifdef	USRSPOOLLOCKS
    "USRSPOOLLOCKS",	"/usr/spool/locks",
#endif
#ifdef	NOSTRANGERS
    "NOSTRANGERS",	NOSTRANGERS,
#endif
    "LMTUUXQT",	LMTUUXQT, /* if not defined we'll stat NULL, it's not a bug */
    "LMTUUSCHED",	LMTUUSCHED, /* if not defined we'll stat NULL, it's not a bug */
    "XQTDIR",	XQTDIR,
    "WORKSPACE",	WORKSPACE,
    "admin directory",	ADMIN,
    NULL,
   };

extern char *nextarg();
int verbose = 0;	/* fsck-like verbosity */

main(argc, argv)
char *argv[];
{
    struct stat statbuf;
    struct tab *tabptr;
    int i;


	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);
	

	(void) strcpy(Progname, "uucheck");
	while ((i = getopt(argc, argv, "vx:")) != EOF) {
		switch(i){

		case 'v':
			verbose++;
			break;
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
#ifdef SMALL
				fprintf(stderr,
				"WARNING: uucheck built with SMALL flag defined -- no debug info available\n");
#endif /* SMALL */
			break;

		default:
			(void) fprintf(stderr, MSGSTR(MSG_UUCHECK_1,
			 "\tusage: %s %s\n"),Progname,
			 MSGSTR(MSG_UUCHECK_0, USAGE));
			exit(1);
		}
	}
	if (argc != optind) {
		(void) fprintf(stderr, MSGSTR(MSG_UUCHECK_1, 
			"\tusage: %s %s\n"), Progname,USAGE);
		exit(1);
	}

    if (verbose) 
	printf(MSGSTR(MSG_UUCHECK_2,
	"*** uucheck:  Check Required Files and Directories\n"));

    for (tabptr = tab; tabptr->name != NULL; tabptr++) {
        if (stat(tabptr->value, &statbuf) < 0) { 
	    fprintf(stderr, "%s - ", tabptr->name);
	    perror(tabptr->value);
	    Uerrors++;
	}
    }

    if (verbose) 
	printf(MSGSTR(MSG_UUCHECK_3,
	"*** uucheck:  Directories Check Complete\n\n"));

    /* check the permissions file */

    if (verbose) 
	printf(MSGSTR(MSG_UUCHECK_4,"*** uucheck:  Check %s file\n"), PFILE);

    Uerrors += checkPerm();
    if (verbose) 
	printf(MSGSTR(MSG_UUCHECK_5, "*** uucheck:  %s Check Complete\n\n"),
	 PFILE);

    exit(Uerrors);

}

char *Name[] = {
"U_LOGNAME", 
"U_MACHINE", 
"U_CALLBACK", 
"U_REQUEST", 
"U_SENDFILES", 
"U_READPATH", 
"U_WRITEPATH", 
"U_NOREADPATH", 
"U_NOWRITEPATH", 
"U_PROTOCOL", 
"U_COMMANDS",
"U_VALIDATE",
};

int
checkPerm ()
{
    int type;
    int error=0;
    char defaults[BUFSIZ];

    for (type=0; type<2; type++) {
	/* type = 0 for LOGNAME, 1 for MACHINE */
 
	if (verbose) 
	printf("** %s \n\n",
	    type == U_MACHINE
		?MSGSTR(MSG_UUCHECK_6, "MACHINE PHASE (when we call or execute their uux requests)")
		:MSGSTR(MSG_UUCHECK_7, "LOGNAME PHASE (when they call us)" ));

	Fp = fopen(PFILE, "r");
	if (Fp == NULL) {
		if (verbose) 
			printf(MSGSTR(MSG_UUCHECK_8,"can't open %s\n"),PFILE);
		exit(1);
	}

	for (;;) {
	    if (parse_tokens (_Flds) != 0) {
		fclose(Fp);
		break;
	    }
	    if (_Flds[type] == NULL)
	        continue;

	    fillFlds();
	    /* if no ReadPath set num to 1--Path already set */
	    fillList(U_READPATH, _RPaths);
	    fillList(U_WRITEPATH, _WPaths);
	    fillList(U_NOREADPATH, _NoRPaths);
	    fillList(U_NOWRITEPATH, _NoWPaths);
	    if (_Flds[U_COMMANDS] == NULL) {
		strcpy(defaults, DEFAULTCMDS);
		_Flds[U_COMMANDS] = defaults;
	    }
	    fillList(U_COMMANDS, _Commands);
	    error += outLine(type);
	}
    if (verbose) printf("\n");
    }
    return(error);
}

int
outLine(type)
int type;
{
	register int i;
	register char *p;
	char *arg, cmd[BUFSIZ];
	int error = 0;
	char myname[MAXBASENAME+1];

	if (_Flds[type][0] == 0)
	    return(0);

	if (type == U_LOGNAME) { /* for LOGNAME */

	    p = _Flds[U_LOGNAME];
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_9,"When a system logs in as: "));

	    while (*p != '\0') {
		p = nextarg(p, &arg);
		if (verbose) 	printf("(%s) ", arg);
	    }
	    if (verbose)  printf("\n");

	    if (callBack()) {
		if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_10,
			"\tWe will call them back.\n\n"));
		return(0);
	    }
	}
	else {	/* MACHINE */
	    p = _Flds[U_MACHINE];
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_11,"When we call system(s): "));
	    while (*p != '\0') {
		p = nextarg(p, &arg);
		if (verbose) printf("(%s) ", arg);
	    }
	    if (verbose) printf("\n");

	}

	if (verbose) 
		if(requestOK())
			printf(MSGSTR(MSG_UUCHECK_12,
			"\tWe DO allow them to request files.\n"));
		else
			printf(MSGSTR(MSG_UUCHECK_13,
			"\tWe DO NOT allow them to request files.\n"));

	if (type == U_LOGNAME) {
	   if (verbose) {
		if(switchRole())
		   printf(MSGSTR(MSG_UUCHECK_14, 
		   "\tWe WILL send files queued for them on this call.\n"));
		else
		   printf(MSGSTR(MSG_UUCHECK_15,
		   "\tWe WILL NOT send files queued for them on this call.\n"));
	  }
	}

	if (verbose) 
	       printf(MSGSTR(MSG_UUCHECK_16,"\tThey can send files to\n"));

	if (_Flds[U_WRITEPATH] == NULL) {
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_17,"\t    %s (DEFAULT)\n"), Pubdir);
	}
	else {
	    for (i=0; _WPaths[i] != NULL; i++)
		if (verbose) printf("\t    %s\n", _WPaths[i]);
	}

	if (_Flds[U_NOWRITEPATH] != NULL) {
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_18,"\tExcept\n"));
	    for (i=0; _NoWPaths[i] != NULL; i++)
		if (verbose) printf("\t    %s\n", _NoWPaths[i]);
	}

	if (requestOK()) {
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_19,
			"\tThey can request files from\n"));
	    if (_Flds[U_READPATH] == NULL) {
		if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_17,"\t    %s (DEFAULT)\n"), Pubdir);
	    }
	    else {
		for (i=0; _RPaths[i] != NULL; i++)
		    if (verbose) printf("\t    %s\n", _RPaths[i]);
	    }

	    if (_Flds[U_NOREADPATH] != NULL) {
		if (verbose) 
			printf(MSGSTR(MSG_UUCHECK_18,"\tExcept\n"));
		for (i=0; _NoRPaths[i] != NULL; i++)
		    if (verbose) printf("\t    %s\n", _NoRPaths[i]);
	    }
	}

	myName(myname);
	if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_21,
		"\tMyname for the conversation will be %s.\n"), myname);
	if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_22,
		"\tPUBDIR for the conversation will be %s.\n"), Pubdir);
	if (verbose) printf("\n");

	if (type == U_MACHINE) {
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_23,"Machine(s): "));
	    p = _Flds[U_MACHINE];
	    while (*p != '\0') {
		p = nextarg(p, &arg);
		if (verbose) printf("(%s) ", arg);
	    }
	    if (verbose) 
		printf(MSGSTR(MSG_UUCHECK_24,
		"\nCAN execute the following commands:\n"));

	    for (i=0; _Commands[i] != NULL; i++) {
		if (cmdOK(BASENAME(_Commands[i], '/'), cmd) == FALSE) {
		    if (verbose) 
			printf(MSGSTR(MSG_UUCHECK_25,
			"Software Error in permission.c\n"));
		    error++;
		}
		if (verbose) 
			printf(MSGSTR(MSG_UUCHECK_26,
			"command (%s), fullname (%s)\n"),
		    	BASENAME(_Commands[i], '/'), cmd);
	    }
	    if (verbose) printf("\n");
	}

	return(error);
}
