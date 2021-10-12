static char sccsid[] = "@(#)20  1.6  src/bos/usr/bin/errlg/errmsg/main.c, cmderrlg, bos411, 9428A410j 2/24/94 13:59:18";
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
 *
 * NAME:     main
 * FUNCTION: Command line interface to errmsg and top level program control.
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
 *       e. automatic assignment of the codepoint 
 * RETURNS:   None.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <sys/err_rec.h>
#include "errmsg.h"

extern optind;
extern char *optarg;
extern char *Codeptfile;

int quietflg;
int displayflg;
int Nomsgidflg = 1;
int Dupflg;
int forceflg;
int Errflg;
int checkflg;
int Warnflg;

static Dsetlist[NERRSETS];
static Ndsets;

char *Outfile;                                      
FILE *Outfp;
char *Infile;                                           
FILE *Infp;


main(argc,argv)
char *argv[];
{
	int c;
	int i;
	int set;
	int len;
	char *cp;
	char *codeptfile;

	Outfile = NULL;
	Outfp = NULL;
	setlocale(LC_ALL,"");
	setprogname();
	catinit(MCS_CATALOG);                        /* init message catalog */

	signal(SIGINT, (void(*)(int)) jsignal);	/* init INT and QUIT signals */
	signal(SIGQUIT,(void(*)(int)) jsignal);

	/* We need to call codeptpath() to initially set the codepoint
	   filename in case we get a usage message.  The usage needs the
	   name of the codepoint file. */	
	codeptpath(0); 
	codeptfile = 0;
	Ndsets = 0;
	while((c = getopt(argc,argv,"w:cz:")) != EOF) {
		switch(c) {
		case 'w':
			cp = strtok(optarg,", \t");
			for( ; cp != NULL; cp = strtok(0,", \t")) {
				/* now make sure the "-w" args are OK */
				if((set = fcodetoset(cp)) < 0) {
					/* if the "-w" arg is "all" */
					if(streq_c(cp,"all")) {
						for(i = 0; i < NERRSETS; ++i) {
							Dsetlist[i] =
							   codetoset[i].nc_set;
						}
						Ndsets = NERRSETS;
						continue;
					}
					else {
						Ndsets = 0;
						usage();
						break;
					}
				}
				if(Ndsets < NERRSETS) {
					for(i = 0; i < Ndsets; i++) /* dups? */
						if(Dsetlist[i] == set)
							break;
					if(i == Ndsets)
						Dsetlist[Ndsets++] = set;
				}
			}
			displayflg++;
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
	Infile = (optind < argc) ? stracpy(argv[optind]) : "-";
	if(displayflg) {           /* display set contents */
		for(i = 0; i < Ndsets; i++)
			display_set(Dsetlist[i]);
		genexit(0);
	}
	
	if(errm_codeptcatinit() == -1) {  	/* create code point file, if necessary */
		cat_fatal(CAT_INST_CODEPOINT_W,"\
Cannot create the error message file with the specified path %s.\n",
		Codeptfile);
	}
	if(errm_codeptcatinit() == -2) {  	/* create code point file, if necessary */
		cat_fatal(CAT_INST_CODEPOINT_W2,"\
The file %s is not a valid error message file.\n",
		Codeptfile);
	}
	if(streq(Infile,"-")) {       /* input from stdin */
		Infp   = stdin;
		Infile = "stdin";
		if (!checkflg) {
			Outfp = stdout;
			Outfile = "stdout";
		}
	} 
	else    {
		if((Infp = fopen(Infile,"r")) == NULL) { /* input from file */
			perror(Infile);
			exit(1);
		}
		cp = basename(Infile);
		len = strlen(cp);
		if (!checkflg) {     /* get space for .out file */ 
			Outfile = jalloc(len + 8);
			sprintf(Outfile,"%s.out",cp);
		}
	}
	switch(setjmp(eofjmp)) {
	case 0:
		setjmpinitflg++;
		break;
	default:
		genexit(1);
	}

	if(!checkflg && errm_codeptcatinit() < 0)   /* init codepoint.cat */
		cat_fatal(CAT_INST_CODEPOINT_W,"\
Cannot open error message catalog %s\n\
         %s", Codeptfile,errstr());

	pass1();
	if(Errflg) {
		cat_eprint(M(CAT_INS_TOTAL),
			"%d total errors. No modifications made to %s\n",
				Errflg,Codeptfile);
		genexit(1);
	}

	if(Outfile && Outfp != stdout)
		if ((Outfp = fopen(Outfile,"w+")) == NULL) {
			perror(Outfile);
			genexit(1);
		}	
	pass2();

	if(Errflg)
		genexit(1);
	if(Warnflg && !checkflg)
		genexit(0);     /* exitcode = 0 when msg overwrites detected */
	genexit(0);
}

/* 
 * NAME:	genexit()
 * FUNCTION:	Generic exit for errmsg.  Unlinks output file
 *		and exits with the given code.
 * RETURNS:	None
 */ 

genexit(exitcode)
{

	if(exitcode) {
		if(Outfp)
			unlink(Outfile);
	}
	exit(exitcode);
}

/* 
 * NAME:	usage()
 * FUNCTION:	Prints usage statement for errmsg and exits.
 * RETURNS:	None
 */

static usage()
{
		cat_eprint(M(CAT_INS_USAGEMSG),"\
Usage:\n\
errmsg -c -w setlist -z filename idfile\n\
Add messages to the error logging message catalog\n\
%s according to the input file 'idfile'.\n\
\n\
-c            check flag. Scan file for syntax errors.\n\
-w set_list   Display messages in sets in 'set_list'\n\
              Valid codes are:\n\
              E     Error Description message set\n\
              P     Probable Cause message set\n\
              U     User Cause message set\n\
              I     Install Cause message set\n\
              F     Failure Cause message set\n\
              R     Recommended Action message set\n\
              D     Detailed Data Id message set\n\
              ALL   All\n\
              The set_list consists of codes separated by commas\n\
-z filename   Uses the error logging message catalog specified by the filename\n\
              parameter.\n\
If no idfile is specified, the input is taken from stdin.\n",Codeptfile);
	exit(1);
}

