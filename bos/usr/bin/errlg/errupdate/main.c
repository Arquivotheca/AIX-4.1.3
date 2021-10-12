static char sccsid[] = "@(#)21        1.20.1.11  src/bos/usr/bin/errlg/errupdate/main.c, cmderrlg, bos411, 9428A410j 3/31/94 17:04:50";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, usage, genexit, headerpr, undopr
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
 * FUNCTION: Command line interface to errupdate and top level program control.
 *
 *   1. Initialize message catalog.
 *   2. Catch signals.
 *   3. Scan command line with getopt. Call usage() if command line error.
 *   4. Open input and output files.
 *   5. Init lex reserved word symbol table.
 *   6. pass1 read in images.
 *   7. pass2 build list to keep and output.
 *   8. print statistics and exit
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <errupdate.h>

extern yydebug;

extern optind;
extern char *optarg;

int headerflg;
int forceflg;
int Errflg;
int hexflg = 1;
int quietflg;
int checkflg;
int noodmflg;
int	pflg;

int Pass;

extern char *Tmpltfile;		/* defined in raslib/tmplt.c */
extern char *Codeptfile;

char *Headerfile;
FILE *Headerfp;
char *Undofile;
FILE *Undofp;
char *Outfile;
FILE *Outfp = stdout;
char *Infile;
FILE *Infp;

int Addcount;
int Deletecount;
int Updatecount;

static FILE *jopen();

main(argc,argv)
char *argv[];
{
	int c;
	int rv;

/* This sets the language variable. */
	setlocale(LC_ALL,"");
	setprogname();
	catinit(MCS_CATALOG);
	signal(SIGINT, (void(*)(int)) jsignal);
	signal(SIGQUIT,(void(*)(int)) jsignal);
	signal(SIGSEGV,(void(*)(int)) jsignal);
	while((c = getopt(argc,argv,"pnhfcqy:")) != EOF) {
		switch(c) {
		case 'y':
			(void)settmpltfile(optarg);
			break;
		case 'p':
			pflg++;
			break;
		case 'q':
			quietflg++;
			break;
		case 'n':
			noodmflg++;
			break;
		case 'h':
			headerflg++;
			break;
		case 'f':
			forceflg++;
			break;
		case 'c':
			checkflg++;
			break;
		default:
			usage();
		}
	}
	Infile = (optind < argc) ? argv[optind] : "-";
	if(streq(Infile,"-")) {
		Infp   = stdin;
		Infile = "stdin";
		if(headerflg) {
			Headerfile = "stdout";
			Headerfp = stdout;
		}
		if(!checkflg && !quietflg) {
			Undofile = "errids.undo";
			Undofp = jopen(Undofile);
		}
	} else {
		char *cp;
		int len;

		if((Infp = fopen(Infile,"r")) == NULL) {
			perror(Infile);
			exit(1);
		}
		cp  = basename(Infile);
		len = strlen(cp);
		if(headerflg) {
			Headerfile = jalloc(len + 8);
			sprintf(Headerfile,"%s.h",cp);
			Headerfp = jopen(Headerfile);
		}
		if(!checkflg && !quietflg && !streq(cp + len - 5,".undo")) {
			Undofile = jalloc(len + 8); /* don't undo an undo */
			sprintf(Undofile,"%s.undo",cp);
			Undofp = jopen(Undofile);
			sprintf(Undofile,"%s.undo",cp);
			Undofp = jopen(Undofile);
		}
	}
	if(!noodmflg) {
		rv = udb_init();
		if(rv < 0)
			cat_fatal(CAT_UPD_CANNOTINIT,"\
Cannot initialize error template file '%s'.\n\
This could be because the directory for the file does not exist,\n\
that the file does not have read and write permission,\n\
or that a file with the wrong format has this same name.\n",Tmpltfile);
	}

	ressyminit();	/* install reserved words */
	switch(setjmp(eofjmp)) {
	case 0:
		setjmpinitflg++;
		break;
	case EXCP_UNEXPEOF:
		genexit_nostats(1);
	case EXCP_INT:
		genexit_nostats(1);
	default:
		genexit_nostats(1);
	}

	pass1();
	if(Errflg) {
		cat_eprint(CAT_UPD_XPASS1,"\
No change to error template database.\n",
			Errflg);
		genexit_nostats(1);
	}
	if(checkflg || noodmflg)
		genexit_nostats(0);

	pass2();
	if(Errflg) {
		cat_eprint(CAT_UPD_XPASS2,"\
Exiting after pass 2 with %d errors.\n",
			Errflg);
		genexit_stats(1);
	}
	genexit_stats(0);
}

genexit_nostats(exitcode)
{

	if (udb_close() < 0)
		cat_warn(M(CAT_TMPLT_COMPRESS),"\
Cannot compress template file %s.\n\
This could be because you don't have permissions, that the\n\
filesystem containing the template file is full, or a fork or\n\
exec system call failed.\n",Tmpltfile);
	if(exitcode) {
		if(Headerfp)
			unlink(Headerfile);
		if(Undofp)
			unlink(Undofile);
	}
	exit(exitcode);
}

genexit_stats(exitcode)
{
	
	if (udb_close() < 0)
		cat_warn(M(CAT_TMPLT_COMPRESS),"\
Cannot compress template file %s.\n\
This could be because you don't have permissions, that the\n\
filesystem containing the template file is full, or a fork or\n\
exec system call failed.\n",Tmpltfile);
	if(exitcode) {
		if(Headerfp)
			unlink(Headerfile);
		if(Undofp)
			unlink(Undofile);
	}
	if(!quietflg)
		prstats();
	exit(exitcode);
}

static FILE *jopen(filename)
char *filename;
{
	FILE *fp;

	if((fp = fopen(filename,"w+")) == NULL) {
		perror(filename);
		genexit_nostats(1);
	}
	return(fp);
}

headerpr(s,a,b,c,d)
char *s;
{

	if(Headerfp)
		fprintf(Headerfp,s,a,b,c,d);
}

static usage()
{
	cat_eprint(CAT_UPD_USAGE,"\
Usage:\n\
errupdate -cfhnpq -y filename filename\n\
\n\
Update the error template database according to the template stanza file\n\
'filename'.\n\
\n\
-c    check   flag. Scan stanza file for syntax errors.\n\
-f    force   flag. Do not stop on detection of duplicates.\n\
-h    header  flag. Generate header file according to description file.\n\
-n    noadd   flag. Similar to 'check' flag. Does not update repository.\n\
-p    override flag. Override normal template checking for alert flag criteria.\n\
-q    quiet   flag. Do not create an undo file.\n\
-y    filename  Uses the error record template file specified by the filename\n\
                parameter.\n\
\n\
If no filename is specified, the input is taken from stdin.\n\
\n\
A #define .h file is written to 'filename.h' that equates the stanza\n\
labels to their generated unique ERROR ID's. If no filename is specified,\n\
the .h file is written to stdout.\n");
	exit(1);
}

static prstats()
{

	cat_eprint(CAT_UPD_STAT,"\
%d entries added.\n\
%d entries deleted.\n\
%d entries updated.\n",
		Addcount, Deletecount, Updatecount);
}

undopr(s,a,b,c,d,e,f,g)
char *s;
{

	if(Undofp)
		fprintf(Undofp,s,a,b,c,d,e,f,g);
}

