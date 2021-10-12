static char sccsid[] = " @(#)58  1.15.1.8  src/bos/usr/bin/errlg/errinstall/main.c, cmderrlg, bos411, 9428A410j 2/24/94 13:57:44";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, genexit, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:     main
 * FUNCTION: Command line interface to errinstall 
 *    Initialize message catalog. (not the same as codepoint catalog)
 *    Catch signals for cleanup after INT.
 *    Scan command line with getopt. Call usage() if command line error.
 *    Open input and output files.
 *    Init Codepoint catalog (error message catalog) via codeptcatinit()
 *    pass1  Check syntax and rule violations and build list of data.
 *    pass2  Build database.
 *
 * Note: ERRLG provides these additional functions and capabilities:
 *       a. hex codepoints.
 *       b. text length checking.
 *       c. no requirement for sorted input.
 *       d. symbolic SET designation.
 * RETURNS:   None.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <sys/err_rec.h>
#include "errinstall.h"

extern optind;
extern char *optarg;
extern char *Codeptfile;

int quietflg;
int Nomsgidflg = 1;
int Dupflg;
int forceflg;
int errflg;
int checkflg;
int Warnflg;

static Dsetlist[NERRSETS];
static Ndsets;

char *Infile;                                          
FILE *Infp;
char *Undofile;                                               
FILE *Undofp;

main(argc,argv)
char *argv[];
{
	int c;
	int i;
	int set;
	int len;
	char *cp;
	char *codeptfile;

	Undofile = NULL;
	Undofp = NULL;
	setlocale(LC_ALL,"");
	setprogname();
	catinit(MCS_CATALOG);          /* init message catalog */

	signal(SIGINT, (void(*)(int)) jsignal);  /* init INT and QUIT signals */
	signal(SIGQUIT,(void(*)(int)) jsignal);

	/* We need to initially set the name of the codepoint file so
	   that it can be used if we have to print the usage statement. */
	codeptpath(0);
	codeptfile = 0;
	Ndsets = 0;
	while((c = getopt(argc,argv,"fqcz:")) != EOF) {
		switch(c) {
		case 'q':
			quietflg++;
			break;
		case 'f':
			forceflg++;
			break;
		case 'c':
			checkflg++;
			break;
		case 'z':
			Codeptfile = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	
	codeptpath(0);

	Infile = (optind < argc) ? argv[optind] : "-";
	if(streq(Infile,"-")) {      /* input from stdin */
		Infp   = stdin;
		Infile = "stdin";
		if(!checkflg && !quietflg) {
			Undofp = stdout;
			Undofile = "stdout";
		}
	}
	else {
		if((Infp = fopen(Infile,"r")) == NULL) {  /* input from a file */
			perror(Infile);
			exit(1);
		}
		cp = basename(Infile);
		len = strlen(cp);
		/* Get space for .undo file if there is no -c or -q flag,
		   and input is not a .undo file */
		if ((!checkflg && !quietflg) && !(streq(cp + len - 5,".undo"))) {  
			Undofile = jalloc(len + 8);
			sprintf(Undofile,"%s.undo",cp);
		}
	}
	switch(setjmp(eofjmp)) {
	case 0:
		setjmpinitflg++;
		break;
	default:
		genexit(1);
	}
	
	if(!checkflg && errm_codeptcatinit() == -1) {  
		cat_fatal(CAT_INST_CODEPOINT_W,"\
Cannot create the error message file with the specified path %s.\n",
		Codeptfile);
	}
        if(!checkflg && errm_codeptcatinit() == -2) {
                cat_fatal(CAT_INST_CODEPOINT_W2,"\
The file %s is not a valid error message file.\n",
		Codeptfile);
        }

	pass1();   
	if(Errflg) {   
		cat_eprint(M(CAT_INS_TOTAL),
			"%d total errors. No modifications made to %s\n",
				Errflg,Codeptfile);
		genexit(1);
	}
	if(Undofile && Undofp != stdout)
		if ((Undofp = fopen(Undofile,"w+")) == NULL) { 
			perror(Undofile);
			genexit(1);
		}
	pass2();
	if(Errflg)  
		genexit(1);
	if(Warnflg && !checkflg)
		genexit(0);    /* exit 0 when msg overwrites detected */
	genexit(0);
}

/*
 * NAME:	genexit()
 * FUNCTION:	Generic exit for errinstall.  
 * RETURNS:     None
 *
 */

genexit(exitcode)
{

	if(exitcode) {
		if(Undofp)
			unlink(Undofile);
	}
	exit(exitcode);
}

/* 
 * NAME:	usage()
 * FUNCTION:	Prints standard usage for errinstall and exits.
 * RETURNS:	None
 */

static usage()
{
		cat_eprint(M(CAT_INS_USAGE_INST),"\
Usage:\n\
errinstall -fcq -z filename idfile\n\
Install messages in the error logging message catalog\n\
%s according to 'idfile'.\n\
\n\
-f            force flag. Overwrite duplicate Message IDs.\n\
-c            check flag. Scan file for syntax errors.\n\
-q            quiet flag. Do not produce an undo file.\n\
-z filename   Uses the error logging message catalog specified by the filename\n\
    	      parameter.\n\
If no idfile is specified, the input is taken from stdin.\n",Codeptfile);
	exit(1);
}
