static char sccsid[] = "@(#)52        1.15  src/bos/usr/bin/errlg/errclear/main.c, cmderrlg, bos411, 9428A410j 3/29/94 17:04:03";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, usage, getcmdline, ndaystodate, numchk
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
 * Clear out records from an error log file.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <locale.h>
#include <errlg.h>


int	Badc;
static char *ndaystodate();
extern optind;
extern char *optarg;
extern char *Tmpltfile;

char Logfile[PATH_MAX]=ERRLOG_DFLT;
int Threshold;				/* value, in bytes, for the logfile to wrap at */

int tmpltflg;            /* specified template repository available. */


int		Errorfd;	/* /dev/error */
struct log_hdr log_hdr;
struct log_entry log_entry;

#define SECONDS_IN_DAY 86400

/*
 * NAME:
 *    main
 *
 * FUNCTION:
 *    initialize signal handling
 *    process command line options
 *    build selection criteria list
 *    read templates
 *    clear errlog entries that meet criteria
 *
 * RETURN VALUE:
 *    none ( no return from clear() )
 */


main(argc,argv)
char *argv[];
{
	int cnt;	/* count of items returned by getattr() */
	char *str;

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
	
	getcmdline(argc,argv);

	if(tmpltinit(0) < 0) {					/* open errtmplt */
		cat_warn(CAT_NO_TMPLT_FILE,
			"The error template repository file %s is either unreadable, or\ndoes not exist.\n",
				 Tmpltfile);
		tmpltflg = 0;
	}
	else
		tmpltflg = 1;

	clear();	/* will exit */

}

static usage()
{
	cat_eprint(CAT_CLR_USAGE,"\
Usage:\n\
errclear -J err_label_list -K err_label_list -N resource_name_list\n\
         -R resource_type_list -S resource_class_list -T err_type_list\n\
         -d err_class_list -i filename -m machine_id -n node_id\n\
         -j id_list -k id_list -l seq_no_list -y filename number_of_days \n\
\n\
Delete error log entries in the specified list that are older than\n\
number_of_days specified.  Number_of_days refers to the number of twenty\n\
four hour periods from command invocation time.\n\
-J list       Select only error_labels     in 'list'.\n\
-K list       Select only error_labels not in 'list'.\n\
-N list       Select only resource_names   in 'list'.\n\
-S list       Select only resource_classes in 'list'.\n\
-R list       Select only resource_types   in 'list'.\n\
-T list       Select only error_types      in 'list'.\n\
-d list       Select only error_classes    in 'list'.\n\
-i filename   Uses the error log file specified by the filename parameter.\n\
-j list       Select only error_ids        in 'list'.\n\
-k list       Select only error_ids  not   in 'list'.\n\
-l list       Select sequence_numbers in 'list'.\n\
-m machine_id Delete entries for the machine id as output by uname -m.\n\
-n node_id    Delete entries for the node id    as output by uname -n.\n\
-y filename   Uses the error record template file specified by the filename\n\
              parameter.\n\
\n\
'list' is a list of entries separated by commas.\n\
error_type  = PERM,TEMP,PERF,PEND,UNKN,INFO\n\
error_class = H (HARDWARE), S (SOFTWARE), O (errlogger MESSAGES), U (UNDETERMINED)\n");
	exit(1);
}

static getcmdline(argc,argv)
char *argv[];
{
	int c;
	char *options;
	int jflag = 0, kflag = 0, logfileset = 0;
	int Jflag = 0, Kflag = 0;
	char *datestr;

	options = "y:i:m:n:N:S:T:R:d:j:k:J:K:l:";

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
		case 'y':
			(void)settmpltfile(argv[optind-1]);
			break;
		case 'i':
			strcpy(Logfile,argv[optind-1]); 
			logfileset++;
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
		case 'm':
		case 'n':
		case 'N':
		case 'S':
		case 'R':
			lst_init(c,optarg);
			break;
		case 'j':
			if (kflag) {
				cat_eprint(CAT_ERRPT_jANDk,
				"The flags j and k are mutually exclusive.\n");
				usage();
			}
			if(num_chk(optarg,16)) {	/* error ids are hex */
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
			if (jflag) {
				cat_eprint(CAT_ERRPT_jANDk,
				"The flags j and k are mutually exclusive.\n");
				usage();
			}
			if(num_chk(optarg,16)) {	/* error ids are hex */
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
		default:
			usage();
		}
	}

	/* If the user didn't give a log file to clear, then use the log   */
	/* file value in ODM.  This is stored in the attribute errlg_file. */
	if (!logfileset)    
		getlogpath();

	/* days to delete must be left on cmd line */
	if((argc - optind) == 1) {
		if((datestr=ndaystodate(argv[optind])) != 0)
			lst_init('e',datestr);
		}
	else {
		cat_eprint(CAT_CLR_NO_DAYS,"\
Number of days is required, and must be zero or greater.\n");
		usage();
	}
}

static char *
ndaystodate(c_ndays)
char *c_ndays;
{
	time_t jtime;
	int ndays;
	struct tm *tm;
	static char buf[16];
	
	if(numchk(c_ndays) < 0) {
		cat_eprint(CAT_CLR_NONNUM,
			"Non-numeric character '%c' in '%s'.\n",Badc,c_ndays);
		usage();
	}
	ndays = strtol(c_ndays,0,10);
	if(ndays < 0) {
		cat_eprint(CAT_CLR_NEGDAYS,
			"You entered %s for number of days.\n\
You must enter a positive integer.\n",c_ndays);
		usage();
	}
	if(ndays == 0)
		return(0);

	time(&jtime);

	/* The maximum number of days that can be cleared is base on   */
	/* the number of seconds elapsed since Greenwich mean time,    */
	/* Jan 1, 1970.  Therefore, the maximum number of specified days */
	/* will increment daily.				       */	
	if(ndays > (jtime / SECONDS_IN_DAY)) {
		cat_eprint(CAT_CLR_MAXDAYS,
"The maximum number of days that can be specified is %d.\n\
NOTE: The errlog file has not been changed.\n", (jtime / SECONDS_IN_DAY));
		usage();
	}

	jtime -= SECONDS_IN_DAY * ndays;
	tm = localtime(&(jtime));
	sprintf(buf,"%02d%02d%02d%02d%02d",
		tm->tm_mon+1,	/* 0-based -> 1-based */
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_year);
	return(buf);
}

/*
 * return true if string is a valid decimal number
 */

static numchk(str)
char *str;
{
	char *cp;
	int c;
	int hexflg;

	cp = str;
	hexflg = 0;
	if(cp[0] == '0' && (cp[1] == 'X' || cp[1] == 'x')) {
		cp += 2;
		hexflg++;
	}
	while(c = *cp++) {
		if(hexflg && isxdigit(c) || !hexflg && isdigit(c))
			continue;
		Badc = c;
		return(-1);
	}
	return(0);
}
