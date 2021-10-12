static char sccsid[] = "@(#)96  1.11  src/bos/usr/bin/sysdumpdev/main.c, cmddump, bos411, 9428A410j 3/3/93 10:59:21";
/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: main, usage, prompt, jseek, jtell, jgets, jread
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * The default dump format routine has 3 functions.
 * 1. To display a dump file as passed via fork/exec from crash.
 * 2. To display a dump file as a standalone program.
 * 3. To find a named component/data_area in the dump file and output
 *    the raw binary to stdout. This is used by errdead to extract the
 *    the errlog from the dump file.
 */

#include <stdio.h>
#include <string.h>
#include <sys/dump.h>
#include <errno.h>
#include <locale.h>
#include "dmpfmt.h"

extern optind;						/* getopt variable */
extern char *optarg;				/* getopt variable */

char *Dumpfile;						/* dumpfile name */
static FILE *Dumpfp;				/* dumpfile FILE pointer */
static Dumpfd = -1;					/* in-sync with Dumpfp */

static FILE *Cmdfp = stdin;			/* subcommand file FILE pointer */
static char *Cmdfile;				/* subcommand file name */

static char *C_name;				/* list of components to display */
static char *Da_name;				/* list of data areas to display */

int checkflg;						/* check dumpfile for header consistency */
int Offsetflg;						/* supply dumpfile offset directly */
int binaryflg;						/* output raw binary (errdead,trcdead) */
int stdinflg;						/* dumpfile = stdin */
int promptflg = 1;					/* output a sumcommand prompt */
int batchflg;						/* -b flag */
int listflg;						/* -l flag */
int allflg;							/* -a flag */
int Aflg;							/* -B flag */

int da_num[1000];					/* list of data areas for -n option */
int da_idx;							/* index into da_num[] */

static cmdtmpfile();
/*
 * NAME:     main
 * FUNCTION: Command line interface to default dump format routine.
 * INPUTS:   argc,argv
 * RETURNS:  none (exits)
 *
 * Command line options:
 *      "dmpfmt dumpfile" will enter "subcommand" mode with 'dumpfile' as
 *      the dumpfile image.
 * -i   "dmpfmt -i < dmpfile" will take dumpfile input from stdin and
 *      subcommand input from /dev/tty.
 * -f   "dmpfmt -f 4" will take dumpfile input from file descriptor 4. The
 *      dumpfile must have been opened as fd 4 before dmpfmt is exec-ed.
 * -c   "dmpfmt -c dumpfile" will verify header consistency for proper lengths
 *      and magic numbers.
 * -O   "dmpfmt -O dumfile" will output a list of component names
 *      and the offset to the start of the component dump table in 'dumpfile'.
 * -C   "dmpfmt -C errlg dumpfile" will cause dmpfmt to set the current
 *      component to 'errlg'.
 * -A   "dmpfmt -C errlg -A errc_io dumpfile" will cause dmpfmt to set the
 *      current component to 'errlg' and the current data area to 'errc_io'.
 * -x   binary mode. Display selected component/data_area in binary and exit.
 * -F   "dmpfmt -F cmdfile" will take subcommands from 'cmdfile'.
 * -S   "dmpfmt -S opt=value,..." will set display options. 
 *      "dmpfmt -S help" will display these options. 
 * -B   "dmpfmt -B dumpfile" will display the all of the dumpfile.
 * -b   "dmpfmt -b dumpfile" will display the first component of the dumpfile.
 *      "dmpfmt -b -C errlg -A errc_io dumpfile" will display
 *      the 'errc_io' data area of the component 'errlg'.
 * -n   "dmpfmt -n 3,5" will display the third and fifth data area of
 *      the first component.
 * -l   "dmpfmt -l" will list the data areas before entering subcommand mode.
 * -a   "dmpfmt -a" will display all of the data areas before
 *       entering subcommand mode.
 *
 * When C_name or Da_name is used to specify a particular data area, the
 * routime cmdtmpfile() is called to generate an internal subcommand file.
 * This will avoid entering the interactive subcommand mode. The scan()
 * routine will return when it hits EOF from the subcommand file.
 */

main(argc,argv)
char *argv[];
{
	int c;
	char *cp;
	int offset;

	(void) setlocale(LC_ALL,"");
	setprogname();		/* macro to fill in Progname */
	catinit(MCS_CATALOG);
	while((c = getopt(argc,argv,"cOC:A:xibBlf:an::DF:S::H")) != EOF) {
		switch(c) {
		case 'c':
			checkflg++;
			break;
		case 'O':
			Offsetflg++;
			break;
		case 'C':
			C_name = optarg;
			break;
		case 'A':
			Da_name = optarg;
			break;
		case 'F':
			Cmdfile = optarg;
			if((Cmdfp = fopen(Cmdfile,"r")) == NULL) {
				perror(Cmdfile);
				genexit(1);
			}
			promptflg = 0;
			break;
		case 'x':
			promptflg = 0;
			binaryflg++;
			break;
		case 'i':
			stdinflg++;
			break;
		case 'S':
			setopts(optarg);
			break;
		case 'B':
			Aflg++;
			break;
		case 'f':
			Dumpfd = atoi(optarg);
			break;
		case 'b':
			batchflg++;
			break;
		case 'n':
			cp = strtok(optarg,", \t\n");
			while(cp) {
				if(da_idx >= 1000)
					break;
				if(numchk(cp))
					da_num[da_idx++] = atoi(cp)-1;
				cp = strtok(0,", \t\n");
			}
			break;
		case 'l':
			listflg++;
			break;
		case 'a':
			allflg++;
			break;
		case 'D':
			debuginit("Btrace");
			break;
		case 'H':
		case '?':
			usage();
		}
	}
	if(Dumpfd > -1) {
		offset = lseek(Dumpfd,0,1);
		if((Dumpfp = fdopen(Dumpfd,"r")) == NULL) {
			perror(Dumpfd);
			genexit(1);
		}
		else if(offset == 0) {
		/* lvm control block info could be in sector 0 */
			if(jread(&offset,sizeof(offset)) == sizeof(offset)) {
				if(offset != DMP_MAGIC)
					offset = 512;
				else
					offset = 0;
			}
			else
				offset = 0;
		}
		jseek(offset);		/* init stdio */
		Dumpfile = "FD";
	} else if(stdinflg) {
		if(!Aflg && !batchflg && Cmdfile == 0) {
			if(C_name || Da_name) {
				promptflg = 0;
				cmdtmpfile();
			} else if((Cmdfp = fopen("/dev/tty","r")) == NULL) {
				Debug("forced batchflg: %s\n",errstr());
				batchflg++;
			}
		}
		Dumpfile = "stdin";
		Dumpfp = stdin;
	} else {
		if(optind == argc) {
			cat_eprint(CAT_ARGC1,"Specify a dump file.\n");
			usage();
		}
		if(Cmdfile == 0 && (C_name || Da_name)) {
			promptflg = 0;
			cmdtmpfile();
		}
		Dumpfile = argv[optind];
		if((Dumpfp = fopen(Dumpfile,"r")) == NULL) {
			perror(Dumpfile);
			genexit(1);
		}
		/* lvm control block info could be in sector 0 */
		if(fread(&offset,sizeof(offset),1,Dumpfp) == 1) {
			if(offset != DMP_MAGIC) {
				fseek(Dumpfp,512L,0);
			}
			else {
				fseek(Dumpfp,0,0);
			}
		}

	}
	if(checkflg) {			/* -c option */
		dsp_compinit();		/* init routine automatically checks for errors */
		exit(0);			/* exit */
	}
	if(Offsetflg) {			/* -O option */
		dsp_compinit();		/* init table of components and offsets */
		dsp_compoffsets();	/* display this table */
		exit(0);			/* exit */
	}
	if(Aflg) {				/* -B */
		dsp_compinit();		/* init table of components and offsets */
		dsp_compall();		/* display all dataareas of all components */
		exit(0);			/* exit */
	}
	scan();					/* enter subcommand mode */
	exit(0);				/* and exit */
}

/*
 * NAME:     cmdtmpfile
 * FUNCTION: Generate a subcommand file, as if specified by -F subcommand file.
 * INPUTS:   none
 * RETURNS:  none
 *
 * Generating a subcommand file allows the scan() routine to always take
 * its subcommand input from Cmdfp, which will either be a /dev/tty (stdin),
 * a file specified by the -F option, or the tmpfile generated here.
 */
static cmdtmpfile()
{

	Cmdfp = tmpfile();					/* get a rw temp file */
	/* These fprintfs do not need to be internationalized because they 
	 * are writing to an internal file. 				   */
	if(C_name)							/* select component */
		fprintf(Cmdfp,"comp %s -\n",C_name);
	if(Da_name)							/* select data area */
		fprintf(Cmdfp,"%s\n",Da_name);
	else
		fprintf(Cmdfp,"1\n");			/* default is first data area */
	fseek(Cmdfp,0,0);					/* seek to beginning */
}

/*
 * NAME:     prompt
 * FUNCTION: Output the subcommand prompt to stdout under control of
 *           'promptflg'
 * INPUTS:   none
 * RETURNS:  none
 *
 * The global variable 'promptflg' is turned off in batch mode, binary (-x)
 * mode, and when -C or -A is used to select a specific data area.
 */
prompt()
{

	if(promptflg) {
		cat_print(0,"-> ");
		fflush(stdout);		/* fflush is necessary because there is no '\n' */
	}
}

/*
 * NAME:     jseek
 * FUNCTION: seek to specified offset in dumpfile.
 * INPUTS:   offset   Byte offset to seek to.
 * RETURNS:  none
 *
 * This routines jseek, jtell, and jread allow the file access method
 * to be localized in main.c and hidden from the rest of dmpfmt.
 */
jseek(offset)
{

	return(fseek(Dumpfp,offset,0));
}

/*
 * NAME:     jtell
 * FUNCTION: return current file offset in dumpfile.
 * INPUTS:   none
 * RETURNS:  Byte offset in dumpfile.
 */
jtell()
{

	return(ftell(Dumpfp));
}

/*
 * NAME:     jread
 * FUNCTION: read dumpfile from current offset.
 * INPUTS:   ubuf    Buffer to read into.
 *           ucount  Number of bytes to read.
 * RETURNS:  number of bytes read. (EOF == 0 bytes read)
 */
jread(ubuf,ucount)
char *ubuf;
{
	int ucountsv;
	int c;

	ucountsv = ucount;
	Debug2("jread %03x|%d\n",ucount,ucount);
	while(--ucount >= 0) {
		if((c = getc(Dumpfp)) == EOF)
			break;
		*ubuf++ = c;
	}
	return(ucountsv - (ucount + 1));
}

/*
 * NAME:     jgets
 * FUNCTION: read a subcommand line, stripping of '\n'
 * INPUTS:   line    Buffer to read into.
 *           count   Max number of bytes to read.
 * RETURNS:  0 if EOF encountered.
 */
jgets(line,count)
char *line;
{
	int len;

	/* This is called by xtr.c which is used by crash. */
	/* Crash has no plans to be internationalized. */

	if(fgets(line,count,Cmdfp) == 0)
		return(0);
	len = strlen(line);
	if(len == 0) {
		Debug("length of line is 0\n");
		return(0);
	}
	if(line[len-1] == '\n')
		line[len-1] = '\0';
	return(1);
}

/*
 * NAME:     usage
 * FUNCTION: Output a usage message and exit.
 * INPUTS:   none.
 * RETURNS:  none.  (exits)
 */
usage()
{

	cat_eprint(CAT_USAGE1,"\
Usage:\n\
%s  -bBlaxi -n da_list -f file_desc -C comp -F subcommand_file filename\n\
Format output from system dump data_area by data_area.\n\
-b       batch mode. display selected component to stdout and exit.\n\
-B       batch mode. display all components to stdout and exit.\n\
-l       list data_areas of component before prompt.\n\
-a       display data_areas of component before prompt.\n\
-n list  display data_areas in 'list' before prompt.\n\
         ('list' is a list of numbers separated by commas)\n\
-x       output unformatted in binary.\n\
-f desc  Use file descriptor 'desc' as input. (fork/exec-ed by crash)\n\
-C comp  Display component 'comp' and exit.\n\
-F file  Use 'file' as input for subcommands instead of from console.\n\
-i       Take dump input from stdin\n",
		Progname);
	cat_eprint(CAT_USAGE2,"\
If a file descriptor is not specified, input is from 'filename' and\n\
   the selected component is the first.\n\
If 'filename' is not specified, dump input is taken from stdin,\n\
   interactive subcommand input is taken from /dev/tty, and\n\
   the selected component is the first one.\n\
Use the comp subcommand to change components.\n\
Type '?' at the prompt for help.\n");
	exit(1);
}

