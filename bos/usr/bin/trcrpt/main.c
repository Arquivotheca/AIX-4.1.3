static char sccsid[] = "@(#)43  1.23.1.8  src/bos/usr/bin/trcrpt/main.c, cmdtrace, bos411, 9428A410j 2/3/94 08:34:12";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: main, trcrpt_sync, List, Listc
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

/*
 * command line interface for trcrpt
 *                                                                    
 * NAME:     main
 * FUNCTION: scan command line options and open log and template files.
 * INPUTS:   argc,argv
 * RETURNS:  none (exits)
 *
 * This routine is actually the command line interface for trcrpt.
 * trcupdate, which is a link to trcrpt, has its command line interface in
 * updt.c
 * The Progname variable is examined to determine what this routine is
 *   trcrpt, trcupdate, or trcrpt_test).
 * Open the message catalog by calling catinit.
 * Catch SIGINT and SIGQUIT to the default signal handler jsignal.
 * Call getcmdline to parse the command line flags.
 * Open the report file if not stdout.
 * Open the template file if not stdout.
 * Initialize error.h error strings.
 * For each logfile:
 *   Open the logfile. Lock it to avoid collision with trace.
 *   Call pass1 to compile the template file into a parse tree and
 *   then read in the logfile event by event.
 */


/*                         
 * because trcupdate is in the lite package, and trcrpt not, this routine
 * becomes the command line interface for trcupdate. When the trace package
 * is installed trcrpt is linked to trcupdate.
 * the entry for routine trcrpt_sync is removed.
 * in getcmdline a flag is added for the option -T (thread id)
 */
 

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <sys/trchkid.h>
#include "rpt.h"
#include "parse.h"

extern optind;
extern char *optarg;
extern char *getenv();

extern verboseflg;
extern char *Infile;

char *Rptfile;
char *Listfile;
char *Tmpltfile = TMPLT_DFLT;
char *Namelist  = NAMELIST_DFLT;
char *Logfile   = LOGFILE_DFLT;
char *Errfile   = "/usr/include/sys/errno.h";

int Condflg;						/* any conditional checking needed */

static char Ext[4];					/* channel number extension */
static condflg,kcondflg;
static CDebugflg;

extern FILE *Rptfp;
extern FILE *Logfp;
extern FILE *Tmpltfp;
extern FILE *Debugfp;
extern FILE *Listfp;

extern pass1excp();

/* genexit() does:
        - compress file
        - release the semaphore
        - and exit()
*/
main(argc,argv)
char *argv[];
{
	int i;
	int start,end;			/* scan command line list of files */
	int debugsv;
	int fcount;
	char *cp;

	(void) setlocale(LC_ALL,"");
	setprogname();
	catinit(MCS_CATALOG);
	Logfile   = vset(Logfile);
	Tmpltfile = vset(Tmpltfile);
	if(signal(SIGINT, jsignal) == SIG_IGN)
		signal(SIGINT,SIG_IGN);
	if(signal(SIGQUIT, jsignal) == SIG_IGN)
		signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,jsignal);
	signal(SIGSEGV,jsignal);
	if(streq(Progname,"trcrpt_test"))
		trcrpt_test(argc,argv);
/* removed
   	if(streq(Progname,"trcrpt_sync")) 
  		trcrpt_sync(argc,argv); 
*/

	ressyminit();
	getcmdline(argc,argv);			/* read command line and fill in values */
	if(Rptfile == NULL) {
		Rptfile = "stdout";
		Rptfp = stdout;
	} else {
		if((Rptfp = fopen(Rptfile,"w+")) == NULL) {
			perror(Rptfile);
			genexit(1);
		}
	}
	vprint("Program name %s\n",Progname);
	vprint("Reportfile is %s.\n",Rptfile);
	Infile = Tmpltfile;
	if(!rawflag || jflag) {
		vprint("Opening template file %s\n",Tmpltfile);
		if((Tmpltfp = fopen(Tmpltfile,"r")) == NULL) {
			perror(Tmpltfile);
			genexit(1);
		}
		signal(SIGSEGV,pass1excp);
		if(jflag || checkflag) {
			gettmpltinit();
			pass1();					/* process each line */
			lexprstats();
			genexit(Errflg ? 1 : 0);
		}
		debugsv = Debugflg;				/* turn off debugging temporarily */
		Debugflg = CDebugflg;
		gettmpltinit();
		Debugflg = debugsv;
		errorstrinit();				/* init /usr/include/sys/errno.h */
		rptsym_init(Namelist,0);	/* init symbol table lookup */
		exec_init();				/* init filename correlations */
		pass1();
	}
	if(Errflg || jflag) {
		lexprstats();
		genexit(Errflg ? 1 : 0);
	}
	if(optind == argc) {			/* use default log file */
		cp = jalloc(strlen(Logfile) + 8);
		sprintf(cp,"%s%s",Logfile,Ext);
		Logfile = cp;
		start = 0;
		end = 1;
	} else {
		start = optind;
		end = argc;
	}
	prevent_init();
	fcount = 0;
	for(i = start; i < end; i++) {
		if(i > 0)
			Logfile = argv[i];
		vprint("Opening logfile %s\n",Logfile);
		if((Logfp = fopen(Logfile,"r")) == NULL) {
			perror(Logfile);
			fcount++;
			continue;
		}
		ereadinit();					/* init buffered event read routine */
		if(verboseflg > 2) {
			fclose(Logfp);
			continue;
		}
		if(Starttime || Endtime)		/* -s or -e flag requires block preprocessing */
			do_s_e();					/* will exit */

		pass2(0);						/* process each line */
		histflush();
		fclose(Logfp);
	}
	if(fcount == (end-start))
		Errflg++;
	lexprstats();
	genexit(Errflg ? 1 : 0);			/* cleanup and exit */
}

static getcmdline(argc,argv)
char *argv[];
{
	int c;
	int n;
	int len;
	int i;
	char *cp;
	int channel;
	char *options;

	channel = 0;
	options = "p::O::n:CxHt:s:e:jhrd::k::T::DLcvq01234567o:";
	while((c = getopt(argc,argv,options)) != EOF) {
		switch(c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			channel = c - '0';
			break;
		case 'p':
			Condflg++;
			pidlist_init(optarg);
			break;
		case 'O':
			Debug ("optarg = %s \n",optarg);
			setopts(optarg);
			break;
		case 'n':
			Namelist = optarg;
			break;
		case 'x':
			setopts("exec=on, svc=on");
			break;
		case 'q':
			quietflag++;
			break;
		case 'v':
			verboseflg++;
			break;
		case 'c':
			checkflag++;
			break;
		case 'e':
			if((Endtime = datetosecs2(optarg)) == -1) {
				cat_eprint(CAT_TPT_DATE_2,"\
Bad format for date string: '%s'\n\
		A date string must be of the form:  mmddhhmmssyy , where\n\
		mm is a 2 digit month,\n\
		dd is a 2 digit day of the month,\n\
		hh is a 2 digit hour,\n\
		mm is a 2 digit minute,\n\
		ss is a 2 digit second,\n\
		yy is a 2 digit year.\n",optarg);
				usage();
			}
			break;
		case 's':
			if((Starttime = datetosecs2(optarg)) == -1) {
				cat_eprint(CAT_TPT_DATE_2,"\
Bad format for date string: '%s'\n\
		A date string must be of the form:  mmddhhmmssyy , where\n\
		mm is a 2 digit month,\n\
		dd is a 2 digit day of the month,\n\
		hh is a 2 digit hour,\n\
		mm is a 2 digit minute,\n\
		ss is a 2 digit second,\n\
		yy is a 2 digit year.\n",optarg);
				usage();
			}
			break;
		case 'j':
			jflag++;
			break;
		case 'h':
			nohdrflag++;
			break;
		case 'r':
			rawflag++;
			break;
		case 'o':
			Rptfile = optarg;
			break;
		case 't':
			Tmpltfile = optarg;
			break;
		case 'C':
			CDebugflg++;
			break;
		case 'D':		/* trace */
			debuginit("Btrace");
			break;
		case 'L':		/* list parsed templates */
			Listflg++;
			break;
		case 'd':
			Condflg++;
			condflg++;
			bitmap(optarg,1);
			break;
		case 'k':
			Condflg++;
			if(!kcondflg++ && !condflg)
				memset(Condhookids,0xFF,sizeof(Condhookids));
			bitmap(optarg,0);
			break;
		case 'T':
			Condflg++;
			tidlist_init(optarg);
			break;
		case '?':
		case 'H':
		default:
			usage();
		}
	}

	/* -e and -s are exclusive of -O starttime and endtime */
	if((Starttime || Endtime) && (Opts.opt_startp || Opts.opt_endp))
		usage();

	if(!kcondflg && !condflg)
		memset(Condhookids,0xFF,sizeof(Condhookids));
	if(channel >= TRC_NCHANNELS) {
		cat_eprint(CAT_TPT_CHAN2BIG,
"Channel %d too big.\n\
The maximum channel number is %d.\n",
			channel,TRC_NCHANNELS);
		usage();
	}
	if(channel != 0 )
		sprintf(Ext,".%d",channel);
	if(Listflg) {
		Listfile = jalloc(strlen(LISTFILE) + 8);
		sprintf(Listfile,"%s%s",LISTFILE,Ext);
		vprint("Opening list file %s\n",Listfile);
		if((Listfp = fopen(Listfile,"w+")) == NULL) {
			perror(Listfile);
			genexit(1);
		}
	}
	if(!quietflag && checkflag && Listfp == NULL)
		Listfp = stderr;
	if(nohdrflag)
		CLRHOOKID(Condhookids,HKWDTOHKID(HKWD_TRACE_HEADER));
	if(verboseflg > 1) {
		vprint("   Conditional event bitmap:\n");
		for(i = 0; i < 16 * 32; i++) {	/* 32 * 8 = 256 */
			if(i % 32 == 0)
				vprint("   ");
			vprint("%02X",Condhookids[i]);
			if(i % 32 == 31)
				vprint("\n");
		}
	}
}

/*
Modify the usage message in order to becoherent with the explanation, options :
         - d, k, s, e
         - A option is removed
*/
usage()
{

	cat_eprint(CAT_TPT_USAGE5,"\
usage: trcrpt -h -r -j -d idlist -k idlist -p pidlist -t tmpltfile \n\
      -s startdate -e enddate -c -DLvq -T tidlist -x -n namelist \n\
	   -O optlist logfile(s)\n\
Write formatted report of logfile to stdout\n\
\n\
-h           Omit heading and footing\n\
-r           Output unformatted trace entries in length(4)|data format\n\
-j           Output the list of trace ids, version numbers, and primary label\n\
-d idlist    Output trace entries only for ids in 'idlist'\n\
-k idlist    Exclude trace entries in 'idlist'\n\
-p pidlist   Output only pids (or execnames) in list. INTR = interrupts\n\
-t tmpltfile Use 'tmpltfile' as template file instead of /etc/trcfmt\n\
-s startdate Select entries later than date. (MMddhhmmssyy)\n\
-e enddate   Select entries earlier than date. (MMddhhmmssyy)\n\
-c           Check the template file for syntax errors. No report\n\
-D           Output debug information to file Btrace\n\
-L           Generate list file to %s. Requires -c flag.\n\
-v           Print filenames as they are opened. -vv gives additional info\n\
-q           Suppress detailed output of syntax error messages\n\
-T tidlist   Exclude or include trace entries in tidlist \n\
-x           Print executable name for every event\n\
-n namelist  Specify the kernel name list file to be used to interpret\n\
             addresses\n\
-O optlist   -O option=value, ... (-O help for additional info)\n\
\n\
The -s and -e flags are exclusive of the -O starttime and endtime.\n\
If logfile is unspecified, trcrpt uses the default logfile\n\
%s\n",
		TMPLT_DFLT,LOGFILE_DFLT);
	genexit(1);
}

/*
 * Set the Condhookids bitmap according to the list of
 * hooks in 'hookbuf' (-d option).
 * If setflg is false, then clear the bits instead.
 * There is a similar routine in trace/main.c
 */
static bitmap(hookbuf,setflg)
char *hookbuf;
{
	char *cp;
	int len;
	int n;
	int i;
	char *delims;

	if(hookbuf == NULL)
		return;
	Debug("bitmap(%s,%d)\n",optarg,setflg);
	delims = ", \t\n";
	for(cp = strtok(hookbuf,delims); cp; cp = strtok(0,delims)) {
		if((n = strtoid_cmd(cp)) < 0)
			usage();
		len = strlen(cp);
		if(setflg) {
			if(len == 3)
				SETHOOKID(Condhookids,n);
			else
				for(i = 0; i < 16; i++)
					SETHOOKID(Condhookids,16*n + i);
		} else {
			if(len == 3)
				CLRHOOKID(Condhookids,n);
			else
				for(i = 0; i < 16; i++)
					CLRHOOKID(Condhookids,16*n + i);
		}
	}
}

