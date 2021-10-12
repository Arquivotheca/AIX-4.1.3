static char sccsid[] = "@(#)14        1.41  src/bos/usr/bin/errlg/errpt/main.c, cmderrlg, bos411, 9439B411a 9/28/94 11:18:52"; 

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, usage, genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Process a report of logged errors.  The default report is summary, consisting 
 * of a single line for each error entry, in reverse chronological order.  The
 * concurrent mode displays in chronological order, and waits for the next entry
 * to arrive in the error log.  Processing is as follows:
 *		For each entry in an error log
 *			If the error template for that entry exists then use it.
 *			Otherwise use default template processing.
 *			If the entry meets selection criteria then display it.
 * Template only processing is available through the 't' flag.
 *			
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <fcntl.h>
#include <errlg.h>
#include <sys/id.h>

extern char* gettmpltfile();

extern optind;
extern char *optarg;
extern char *Codeptfile;

char	Logfile[PATH_MAX];

int		Errorfd;	/* /dev/error */

int   Threshold;	/* value, in bytes, for the logfile to wrap at */
struct	CuAt* CuAtp;	/* odm record for errlog file size */

/* int rawflg; */	/* process in '-r' raw mode */
int asciiflg;		/* process in '-g' ascii mode */
int detailedflg;	/* process in '-a' detail mode */
int concurrentflg;	/* process in '-c' concurrent mode. */
int templateflg;	/* process in '-t' template mode. */
int tmpltflg;		/* specified template repository not available. */

char *Outfile;
FILE *Outfp = stdout;
FILE *Infp;

static dbinitflg;

struct log_hdr log_hdr;
struct log_entry log_entry;

uid_t  real;
uid_t  saved;

/*
 * NAME:     main
 *
 * FUNCTION:
 *	  get user environment
 *    initialize message catalog
 *    initialize signal handling
 *    process flags that generate selection possibilities.
 *    process command line options
 *    initialize errlog and errtmplt files
 *    initialize codept.cat file
 *    initialize printing environment
 *    select between template, concurrent, or default reporting
 *
 * RETURNS: none
 */


main(argc,argv)
char *argv[];
{
	/* To deal with errpt's potential security exposure,
         *  the privileges will be lowered immediately to the
         *  real id of the person who invoked the command.  
         *  They will be raised to root later - only for as
	 *  long as needed.  
	 */
	saved = geteuid();
	real  = getuid();
	seteuid(real);

	setlocale(LC_ALL,"");	/* get user environment */
	setprogname();
	catinit(MCS_CATALOG);
	
	if(signal(SIGINT, (void(*)(int)) jsignal) == SIG_IGN)
		signal(SIGINT,SIG_IGN);
	if(signal(SIGQUIT, (void(*)(int)) jsignal) == SIG_IGN)
		signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,(void(*)(int)) jsignal);
	signal(SIGHUP, (void(*)(int)) jsignal);
	signal(SIGSEGV,(void(*)(int)) jsignal);
	signal(SIGUSR1,SIG_IGN);		/* errnotify signal */
	switch(setjmp(eofjmp)) {
	case 0:
		setjmpinitflg++;
		break;
	default:
		genexit(1);
	}
	getcmdline(argc,argv);
	if(db_init(0) < 0)
		exit(1);
	dbinitflg++;
	if(codeptcatinit() < 0)
		cat_eprint(CAT_RPT_CODEPOINT_W,"\
Cannot open error message catalog %s.\n\
The error report will still run, but it will not have explanatory messages\n",
			Codeptfile);
	pr_init();
	if (templateflg)
		rpt_tmplt();
	else if(concurrentflg)
		rpt_c();
	else
		rpt();
	genexit(0);
}

/*
 * NAME:     genexit
 * FUNCTION: Delete notification and exit with the provided code.
 *
 * RETURNS: none
 */

genexit(exitcode)
{
	signal(SIGINT,SIG_IGN);
	if(dbinitflg) 
		seteuid(saved);
		notifydelete();
		seteuid(real);
	
	exit(exitcode);
}

/*
 * NAME:     usage
 * FUNCTION: Display usage message for errpt.
 * RETURNS:  None
 */

static usage()
{
/* The following two lines should be uncommented out and replace the       */
/* existing lines once FFDC is to be used and documented.		   */
/* -->  cat_eprint(CAT_RPT_USAGE,
   -->             " usage:   errpt -actrgY -s startdate -e enddate\n\        */
/* Also add the following line to the cat eprint statement to be placed       */
/* after the -g flag line						      */
/* --> -Y         Print only error log entries having symptom string data.\n\ */

/* Commenting out the -r flag function. */
/* -r         Output raw binary error record structures.\n\ */

	cat_eprint(CAT_RPT_USAGE,
		"Usage:   errpt -actg -s startdate -e enddate\n\
         -N resource_name_list -S resource_class_list -R resource_type_list\n\
         -T err_type_list -d err_class_list -j id_list -k id_list\n\
         -J label_list -K label_list -l seq_no_list -F flags_list \n\
		 -m machine_id -n node_id -i filename -y filename -z filename\n\
\n\
Process error log entries from the supplied file(s).\n\
-i filename  Uses the error log file specified by the filename parameter.\n\
-y filename  Uses the error record template file specified by the filename\n\
			 parameter.\n\
-z filename  Uses the error logging message catalog specified by the filename\n\
			 parameter.\n\
\n\
Output formatted error log entries sorted chronologically.\n\
-a         Print a detailed listing. Default is a summary listing.\n\
-c         Concurrent mode. Display error log entries as they arrive.\n\
-t         Print error templates instead of error log entries.\n\
-g         Output raw ascii  error record structures.\n\
\n\
Error log entry qualifiers:\n\
-s startdate  Select entries posted later   than date. (MMddhhmmyy)\n\
-e enddate    Select entries posted earlier than date. (MMddhhmmyy)\n\
-N list       Select resource_names   in 'list'.\n\
-S list       Select resource_classes in 'list'.\n\
-R list       Select resource_types   in 'list'.\n\
-T list       Select types            in 'list'.\n\
-d list       Select classes          in 'list'.\n\
-j list       Select ids              in 'list'.\n\
-k list       Select ids  NOT         in 'list'.\n\
-J list       Select labels           in 'list'.\n\
-K list       Select labels NOT       in 'list'.\n\
-l list       Select sequence_numbers in 'list'.\n\
-F list       Select templates according to the value of the\n\
              Alert, Log, or Report field.\n\
-m machine_id Select entries for the machine id as output by uname -m.\n\
-n node_id    Select entries for the node id    as output by uname -n.\n\
\n\
'list' is a list of entries separated by commas.\n\
error_type  = PERM,TEMP,PERF,PEND,UNKN,INFO\n\
error_class = H (HARDWARE), S (SOFTWARE), O (errlogger MESSAGES), U (UNDETERMINED)\n");
	exit(1);
}

/*
 * NAME:     getcmdline
 * FUNCTION: Process the command line for errpt.
 * RETURNS:  None
 */

static getcmdline(argc,argv)
char *argv[];
{
	int c, logfileset = 0;
	char *options;
	int jflag = 0, kflag = 0;
	int Jflag = 0, Kflag = 0;

        /* The following line should be uncommented out and replace the */
        /* existing line once FFDC is to be used and documented.        */
        /* --> options = "J:K:l:ractgYs:e:m:n:F:S:N:R:d:j:k:T:i:y:z:";  */

	/* options = "J:K:l:ractgs:e:m:n:F:S:N:R:d:j:k:T:i:y:z:"; */
	options = "J:K:l:actgs:e:m:n:F:S:N:R:d:j:k:T:i:y:z:";
	while((c = getopt(argc,argv,options)) != EOF) {
		switch(c) {
		case 'J':
			if (Kflag) {
				cat_eprint(CAT_ERRPT_JANDK,
				"The flags J and K are mutually exclusive.\n");
				usage();
			}
			Jflag = 1;
			lst_init(c,optarg);
			break;
		case 'K':
			if (Jflag) {
				cat_eprint(CAT_ERRPT_JANDK,
				"The flags J and K are mutually exclusive.\n");
				usage();
			}
			Kflag = 1;
			lst_init(c,optarg);
			break;
		case 'i':
			strcpy(Logfile,optarg); 
			logfileset++;
			break;
		case 'y':
			(void)settmpltfile(optarg);
			break;
		case 'z':
			Codeptfile = stracpy(optarg);
			break;
	/* Commenting out the -r flag function. */
	/*	case 'r':
			rawflg++;
			break;   */
		case 'g':
			asciiflg++;
			break;
		case 'a':
			detailedflg++;
			break;
		case 'c':
			concurrentflg++;
			break;
		case 't':
			templateflg++;
			break;
		case 'j':
			if (kflag)
			{
				cat_eprint(CAT_ERRPT_jANDk,
				"The flags j and k are mutually exclusive.\n");
				usage();
			}
			if(num_chk(optarg,16)) {	/* errids are hex. */
				lst_init(c,optarg);
				jflag = 1;
			}
			else {
				cat_eprint(CAT_INVALID_NUMBER,"\
The value specified %s is not valid for flag %c.\n\
No processing was performed.\n",optarg,c);
				usage();
			}
			break;
		case 'k':
			if (jflag)
			{
				cat_eprint(CAT_ERRPT_jANDk,
				"The flags j and k are mutually exclusive.\n");
				usage();
			}
			if(num_chk(optarg,16)) {	/* errids are hex. */
				lst_init(c,optarg);
				kflag = 1;
			}
			else {
				cat_eprint(CAT_INVALID_NUMBER,"\
The value specified %s is not valid for flag %c.\n\
No processing was performed.\n",optarg,c);
				usage();
			}
			break;
		case 'l':
			if(num_chk(optarg,10)) {	/* sequence ids are decimal */
				lst_init(c,optarg);
			}
			else {
				cat_eprint(CAT_INVALID_NUMBER,"\
The value specified %s is not valid for flag %c.\n\
No processing was performed.\n",optarg,c);
				usage();
			}
			break;
		case 'd':
			if (valid_class(optarg))
				lst_init(c,optarg);
			else
				usage();
			break;
		case 'T':
			if (valid_type(optarg))
				lst_init(c,optarg);
			else
				usage();
			break;
		case 'F':
			if (valid_keyword(optarg))
				lst_init(c,optarg);
			else
				usage();
			break;	
		case 'm':
		case 'n':
		case 's':
		case 'e':
		case 'N':
		case 'R':
		case 'S':
               /* The following line should be uncommented out            */
	       /* and --> removed once FFDC is to be used and documented. */
	       /* --> case 'Y':				                  */
			lst_init(c,optarg);
			break;
		default:
			usage();
		}	/* end switch */
	}	/* end while */

	/* If the user didn't enter a log file name, then get the   */
	/* current value from ODM.  This is stored in the attribute */
	/* errlg_file.						    */	
	if (!logfileset)  
		getlogpath();
}

/*
 * NAME:     db_init
 * FUNCTION: Initialize the errtmplt and errlog files, and
 *           notification methods.
 * RETURNS:  -1 failure to open errlog
 *            0 success
 */

db_init()
{
	int	rc;


	if(tmpltinit(0) < 0) {					/* open errtmplt */
		if(templateflg) {
			cat_eprint(CAT_NO_TMPLT_FILE, "The error template repository file %s\nis either unreadable, or does not exist. \n", gettmpltfile());
			perror("Open");
			exit(1);
		}
		else {
			cat_warn(CAT_NO_TMPLT_FILE, "The error template repository file %s\nis either unreadable, or does not exist.\n", gettmpltfile());
			tmpltflg = 0;
		}
	}
	else
		tmpltflg = 1;

	rc = 0;

	if(!templateflg) {		/* if not -t for errpt */
		notifyinit();				/* open errnotify */
		if(logopen(O_RDONLY) < 0)	/* open errlog */
			rc = -1;
	}

	return(rc);
}

