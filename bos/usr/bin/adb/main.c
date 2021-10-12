static char sccsid[] = "@(#)%M  1.15.1.10  src/bos/usr/bin/adb/main.c, cmdadb, bos41B, 9504A  12/14/94  18:07:42";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: main, chkerr, done, error, fault, nocore, round
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "adb_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
/*
 *  Main calling routine
 */

#include <locale.h>
#include "defs.h"

/*
 * Include files for hardware platform support. 
 */
#include "ops.h"
#include "disassembly.h"
#include <sys/systemcfg.h>

unsigned short current_hardware; /* Used to indicate hardware platform */

extern REGLIST  *reglist;        /* set to use correct register list. */
extern REGLIST  reglist_pwr[];   /* lists defined in extern.c */
extern REGLIST  reglist_601[];
extern REGLIST  reglist_ppc[];

LOCAL jmp_buf jbenv;
LOCAL int     exitflg;
#ifdef _NO_PROTO
LOCAL void fault();
#else
LOCAL void fault(int);
#endif  /* _NO_PROTO */
#if 0
LOCAL void nocore();
#endif

/* Set up files and initial address mappings */
#define SHARED_OBJECTS   512
int debugflag = 0; /* this is to debug adb */
int coredump = 0;
char dir_name[80] = "/usr/lib/adb";
extern int kmem_core ;
main(argc, argv)
int     argc;
STRING  *argv;
{
    int fd;
    unsigned int len;
    char *cmd;
    char * tmp_dirname;
#if KADB
    STRING  s,getenv();
#endif

    setlocale(LC_ALL, "");
    scmc_catd = catopen("adb.cat", NL_CAT_LOCALE);

    /* Let the default prompt of adb to be "adb" */
    (void) strncpy(promptstr, "adb\n", MAXPROMPT-1);
    while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
	--argc;
	++argv;
	switch (argv[0][1]) {
	case 'w':
	    wtflag = O_RDWR;
	    break;
	case 'p':
	    (void)strncpy(promptstr, &argv[0][2], MAXPROMPT-1);
	    break;
	case 'Y':
		debugflag = TRUE;
		break;
	case 'k' :
		kmem_core = TRUE;
		break;
	case 'I':
	    len = (unsigned)strlen(argv[0]);
	    tmp_dirname = (char *)calloc( len, 1);
	    (void)strncpy(tmp_dirname, &argv[0][2], len);
	    tmp_dirname[len-2] = '\0';
	    if ( chkdir(tmp_dirname) ) {
	    	(void)strncpy(dir_name, &argv[0][2], MAXPROMPT-1);
	    }
	    free(tmp_dirname);
	    break;
	default:
	    adbpr(catgets(scmc_catd, MS_main, M_MSG_78,
	          "Usage: adb [ -w ] [ -Idir ] [ -k ] [ -pPROMPT ] [ a.out [ core ]]\n") );

            exit(2);
	}
    }

    if (argc > 1)
	symfil = argv[1];
    if (argc > 2)
	corfil = argv[2];
#if KADB
    else if ((s = getenv("RDPORT")) != NULL) {
	adbpr("Using %s\n",s);
	corfil = s;
    }
#endif
    argcount = argc;
     /*
      * Determine the hardware platform and set external variable
      * current_hardware to indictate this.
      */
#ifdef _POWER
     if( __power_rs1() || __power_rsc() ) {
	current_hardware = PWR;
	reglist = reglist_pwr;
     } else if( __power_rs2() ) {
	current_hardware = PWRX;
	reglist = reglist_pwr;
     } else if( __power_601() ) {
	current_hardware = SET_601;
	reglist = reglist_601;
     } else if( __power_603() ) {
	current_hardware = SET_603;
	reglist = reglist_ppc;
     } else if( __power_604() ) {
	current_hardware = SET_604;
	reglist = reglist_ppc;
     } else if( __power_620() ) {
	current_hardware = SET_620;
	reglist = reglist_ppc;
     } else {
  /* Not a recognized hardware type. Set to ANY. */
	current_hardware = ANY;
	reglist = reglist_pwr;
     }
#else
	current_hardware = ANY;
	reglist = reglist_pwr;
#endif

    fd_info = (struct fdinfo *) calloc(SHARED_OBJECTS, sizeof(struct fdinfo) );
    fd_info[0].pathname = symfil;
    fd_info[0].membername = (char *) 0;
    fd = getfile(corfil, 2);
    if ( ( fd != -1) &&  (strcmp("-", corfil)  != 0 ) )  {
	slshmap.ufd = fcor = fd;
	setcor();
    }
    if ( ! coredump ) { 
	getldrinfo(0);
	readobj(0,0);
    }

    setsym();
	
    /* Set up variables for user */
    maxoff = MAXOFF;
    maxpos = MAXPOS;

    /* these maps doesn't make sense, this is becauseof shared lib support */
    var[VARB] = slshmap.b1 ? slshmap.b1 : qstmap.b1; 
    var[VARD] = datsiz;    /* the size of data section of a.out */
    var[VARE] = entrypt;   /* this will be the entry point of the a.out */
    var[VARM] = magic;     /* a.out's magic number */
    var[VARS] = stksiz;    /* the size of the stack  */
    var[VART] = txtsiz;    /* size of the text section */

    if ((sigint = signal(SIGINT, SIG_IGN)) != SIG_IGN) {
	sigint = fault;
	if (SIG_ERR == signal(SIGINT, fault))
	    perror( "main: signal()" );
    }
    sigqit = signal(SIGQUIT, SIG_IGN);
#if 0 /* TEMP */
    if (SIG_ERR == signal(SIGILL, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGIOT, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGEMT, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGFPE, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGBUS, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGSEGV, nocore))
	perror( "main" );
    if (SIG_ERR == signal(SIGSYS, nocore))
	perror( "main" );
#endif

    (void)setjmp(jbenv);

    if (executing) {
	delbp();
    }
    executing = FALSE;

    for (;;) {
	flushbuf();
	if (errflg != NULL) {
	    adbpr("%s\n", errflg);
	    exitflg = (errflg != NULL);
	    errflg = NULL;
	}
	if (mkfault) {
	    mkfault = FALSE;
	    printc('\n');
	  /*  prints(DBNAME); */
	}
	prompt();
	lp = 0;
	(void) rdc();
	lp--;
	if (eof) {
	    if (infile) {
		iclose();
		eof = 0;
		longjmp(jbenv, 1);
	    } else
		done();
	} else
	    exitflg = 0;

	command((STRING)NULL, lastcom);
	if (*lp && lastc != '\n')
	    error(catgets(scmc_catd,MS_extern,E_MSG_102,NOEOR));
    }
}

long 
round(a, b)
long a;
long b;
{
    int w;

    w = (a / b) * b;
    if (w < a) w += b;
    return (w);
}

/* error handling */

void 
chkerr()
{
    if (errflg != NULL || mkfault)
	error(errflg);
}

void 
error(n)
STRING n;
{
    errflg = n;
    iclose();
    oclose();
    longjmp(jbenv, 1);
}

LOCAL void 
fault(a)
{
    if (SIG_ERR == signal(a, fault))
	perror( "fault: signal()" );
/* Skip to EOF.  No sense printing error if unseekable, so ignore it. */
    (void)lseek(infile, 0L, 2);
    mkfault = TRUE;
}

#if 0
LOCAL /*int*/ nocore()
{
    static char haltmsg[] = "adb system failure -- exiting\n";
    (void)write(2, haltmsg, sizeof(haltmsg)-1);
    exit(2);
}
#endif

void done()
{
    endpcs();
    exit(exitflg);             
}

chkdir(pathname)
char *pathname;
{
	char *cmd ;
	cmd = (char *) calloc( (unsigned)(strlen(pathname)+10), 1);
	sprintf(cmd,"[ -d %s ]\n", pathname);
	if ( system( cmd ) != 0 ) {
	   adbpr("%s: doesn't exist.\n", pathname);
	   return 0;
	}
	free(cmd);
	return 1;
}
