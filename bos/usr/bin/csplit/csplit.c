static char sccsid[] = "@(#)87  1.23  src/bos/usr/bin/csplit/csplit.c, cmdfiles, bos412, 9446C 11/14/94 16:48:02";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: csplit
 *
 * ORIGINS: 3,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <sys/dir.h>
#include <nl_types.h>
#include <string.h>
#include <regex.h>
#include "csplit_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CSPLIT,Num,Str)

#define LAST	0L
#define ERR	-1
#define FALSE	0
#define TRUE	1
#define EXPMODE	2
#define LINMODE	3
#define EXPSIZ	(128*5)
#define	LINSIZ	(LINE_MAX+1)
/* The max no of digits the user can specify to be appended to filename */
#define MAXDIGITS  (MAXNAMLEN-1)

/* Function prototypes */
static int   natol(char *, long *);
static void  closefile(void);
static void  fatal(char *, char *);
static long  findline(regex_t *, long);
static void  flush(void);
static FILE  *getfile(void);
static char  *getline(int);
static void  line_arg(char *);
static void  num_arg(char *, int);
static void  re_arg(char *);
static void  sig(void);
static void  to_line(long);
static void  prntregerr(int, regex_t *);

/* Globals */
static char linbuf[LINSIZ];		/* Input line buffer */
static regex_t regxp;  		/* compiled regular expression structure */
static regex_t *regxpptr = &regxp;	/* pointer to regexp */
static char file[PATH_MAX] = "xx";	/* File name buffer */
static char *targ;			/* Arg ptr for error messages */
static FILE *infile, *outfile;		/* I/O file streams */
static char *infilename;		/* Name of input file */
static int silent, keep, create;	/* Flags: -s(ilent), -k(eep), (create) */
static long offset;			/* Regular expression offset value */
static long curline;			/* Current line in input file */

static int regstat;

static unsigned short filelim_flag = 0;
static int num_digits = 2; 
static int maxfiles = 1;
static int prefix_lngth=0;
static char *prefix = file;

/*
 * NAME: csplit [-ks] [-f prefix] [-n number] file arg1 [... argn]
 *                                                                    
 * FUNCTION: splits files by context
 *           -s         suppresses error messages
 *           -k         leaves created file segments intact in the event
 *                      of an error
 *           -f prefix  specifies the prefix name for the created
 *                      file segments  xx is the default prefix
 *	     -n number  Use number decimal digits to form filenames for the
 *			file pieces.  The default is 2.
 */  
main(int argc, char **argv)
{
	int c, mode, indx;
	char tmp_name[] = "/var/tmp/csXXXXXX";
	FILE *tmp_fou = NULL;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CSPLIT, NL_CAT_LOCALE);

	while((c=getopt(argc, argv, "f:n:sk")) != EOF)
		switch(c) {
		case 'f':
			prefix = optarg;
			break;
		case 'n':
			num_digits = atoi(optarg);
			break;	
		case 's':
			silent++;
			break;
		case 'k':
			keep++;
			break;
		default:
			fatal(MSGSTR(USAGE,"Usage: csplit [-s] [-k] [-f prefix] [-n number] file args ...\n"),NULL); /*MSG*/
		}

	/* If the length of the file name prefix + the number of digits added */
	/* to the end of the filename exceeds MAXNAMLEN, then issue a fatal   */
	/* error and exit. -1 for 0..MAXNAMLEN-1; -num_digits for the number  */
	/* of digits appended.				                      */
	if ((prefix_lngth=strlen(prefix))  > (MAXNAMLEN - 1 - num_digits))
	   	fatal(MSGSTR(PRELONG, "csplit: The prefix %s is too long.\n"), prefix);
	/* Otherwise, create the file with prefix, and determine the maximum */
	/* number of files that can be created with the number of digits.    */
	else  {
		(void)strcpy(file, prefix);
		if (num_digits <= 1) {
			maxfiles = 10;
			num_digits = 1;
		} else {
			for (indx=1; indx <= num_digits; indx++)
				maxfiles = maxfiles * 10;
			maxfiles = (maxfiles > OPEN_MAX) ? OPEN_MAX : maxfiles;
		}
	}
	
	argc -= optind;
	argv = &argv[optind];

	if (*argv[0] == '-') {
		int fd;
		infile = stdin;
		argv++;
		/* make temp file to hold data and then process data */
		mktemp(tmp_name);
		if ((fd=creat(tmp_name,0600)) < 0) {
			unlink(tmp_name);
			fatal(MSGSTR(CANTCREAT,"csplit: Cannot create %s\n"),tmp_name); /*MSG*/
		}
		close(fd);
		if ((tmp_fou = fopen(tmp_name, "r+")) == NULL) {
			unlink(tmp_name);
			fatal(MSGSTR(CANTOPEN,"csplit: Cannot open %s\n"),tmp_name); /*MSG*/
		}
		while (getline(0) != NULL) {
			fputs(linbuf,tmp_fou);
		}
		rewind(tmp_fou);
		infile = tmp_fou;
	}

	if(argc <= 1)
		fatal(MSGSTR(USAGE,"Usage: csplit [-s] [-k] [-f prefix] [-n number] file args ...\n"),NULL); /*MSG*/

	if(infile==(FILE *)NULL) {
		infilename = *argv;
		if((infile = fopen(infilename,"r")) == NULL)
			fatal(MSGSTR(CANTOPEN,"csplit: Cannot open %s\n"),*argv); /*MSG*/
		--argc; ++argv;
	}

	curline = 1L;
	(void)signal(SIGINT,(void (*)(int))sig);

	/*
	*	The following for loop handles the different argument types.
	*	A switch is performed on the first character of the argument
	*	and each case calls the appropriate argument handling routine.
	*/

	for(; *argv; ++argv) {
		targ = *argv;
		switch(**argv) {
		case '/':
			mode = EXPMODE;
			create = TRUE;
			re_arg(*argv);
			break;
		case '%':
			mode = EXPMODE;
			create = FALSE;
			re_arg(*argv);
			break;
		case '{':
			num_arg(*argv,mode);
			mode = FALSE;
			break;
		default:
			mode = LINMODE;
			create = TRUE;
			line_arg(*argv);
			break;
		}
	}
	create = TRUE;
	to_line(LAST);
	if (tmp_fou != NULL)
		unlink(tmp_name);
	exit(0);
}

/*
 * NAME: natol
 *                                                                    
 * FUNCTION:  natol takes an ascii argument (str) and converts it to a 
 *            long (plc).  It returns ERR if an illegal character.  
 *            The reason that atol does not return an answer (long) is 
 *            that any value for the long is legal, and this version of 
 *            atol detects error strings.
 */
static int
natol(char *str, long *plc)
{
	int f;
	*plc = 0;
	f = 0;
	for(;;str++) {
		switch(*str) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
		case '+':
			str++;
		}
		break;
	}
	for(; *str != '\0'; str++)
		if(*str >= '0' && *str <= '9')
			*plc = *plc * 10 + *str - '0';
		else
			return(ERR);
	if(f)
		*plc = -(*plc);
	return(TRUE);	/* not error */
}

/*
 * NAME: closefile
 *                                                                    
 * FUNCTION:
 *	Closefile prints the byte count of the file created, (via fseek
 *	and ftell), if the create flag is on and the silent flag is not on.
 *	If the create flag is on closefile then closes the file (fclose).
 */
static void
closefile(void)
{
	if(!silent && create) {
		(void)fseek(outfile,0L,2);
		(void)fprintf(stdout,"%ld\n",ftell(outfile));
	}
	if(create)
		(void)fclose(outfile);
}

/*
 * NAME: fatal
 *                                                                    
 * FUNCTION: 
 *	Fatal handles error messages and cleanup.
 *	Because "arg" can be the global file, and the cleanup processing
 *	uses the global file, the error message is printed first.  If the
 *	"keep" flag is not set, fatal unlinks all created files.  If the
 *	"keep" flag is set, fatal closes the current file (if there is one).
 *	Fatal exits with a value of 1.
 */
static void
fatal(char *string, char *arg)
{
	char *fls;
	int  num;

	if ((string == (char *) 0) && (arg == (char *) 0))
		prntregerr(regstat, regxpptr);
	else
		(void)fprintf(stderr,string,arg);

	if(!keep) {
		if(outfile) {
			(void)fclose(outfile);
			/* Set fls to point to the digit portion of file */
			fls = file; 
			fls += prefix_lngth;
			/* Remove all the files previously created.      */
 			for(num=atoi(fls); num >= 0; num--) {
                                (void)sprintf(fls,"%0*d",num_digits, num);
				if (strcmp(file, infilename) != 0)
                                	(void)unlink(file);
                        }

		}
	} else
		if(outfile)
			closefile();
	exit(1);
}

/* NAME:  prntregerr
 * 
 * FUNCTION:
 *		Print the error message produced by regerror().
 */

static void
prntregerr(int regstatus, regex_t *preg)
{
	char *err_msg_buff;
	size_t sobuff; 		/* size of buffer needed */

	sobuff = regerror(regstatus, preg, 0, 0);
	err_msg_buff = (char *) malloc(sizeof(char) *sobuff);
	sobuff = regerror(regstatus, preg, err_msg_buff, sobuff);

	(void)fprintf(stderr, "%s\n", err_msg_buff);
}

/*
 * NAME: findline
 *                                                                    
 * FUNCTION:
 *	Findline returns the line number referenced by the current argument.
 *	Its arguments are a pointer to the compiled regular expression (expr),
 *	and an offset (oset).  The variable lncnt is used to count the number
 *	of lines searched.  First the current stream location is saved via
 *	ftell(), and getline is called so that R.E. searching starts at the
 *	line after the previously referenced line.  The while loop checks
 *	that there are more lines (error if none), bumps the line count, and
 *	checks for the R.E. on each line.  If the R.E. matches on one of the
 *	lines the old stream location is restored, and the line number
 *	referenced by the R.E. and the offset is returned.
 */

static long findline(regex_t *expr, long oset)
{
	static int benhere;
	long lncnt = 0, saveloc;


	saveloc = ftell(infile);
	if(curline != 1L || benhere)		/* If first line, first time, */
		(void) getline(FALSE);			/* then don't skip */
	else
		lncnt--;
	benhere = 1;
	while(getline(FALSE) != NULL) {
		lncnt++;
		if((regexec(expr, linbuf, (size_t) 0, (regmatch_t *) NULL, 0))==0) {
			(void)fseek(infile,saveloc,0);
			return(curline+lncnt+oset);
		}
	}
	(void)fseek(infile,saveloc,0);
	return(curline+lncnt+oset+2);
}

/*
 * NAME: flush
 *                                                                    
 * FUNCTION: 
 *	Flush uses fputs to put lines on the output file stream (outfile)
 *	Since fputs does its own buffering, flush doesn't need to.
 *	Flush does nothing if the create flag is not set.
 */
static void
flush(void)
{
	if(create)
		(void)fputs(linbuf,outfile);
}

/*
 * NAME: getfile
 *                                                                    
 * FUNCTION:
 *	Getfile does nothing if the create flag is not set.  If the
 *	create flag is set, getfile positions the file pointer (fptr) at
 *	the end of the file name prefix on the first call (fptr=0).
 *	Next the file counter (ctr) is tested for MAXFLS, fatal if too
 *	many file creations are attempted.  Then the file counter is
 *	stored in the file name and incremented.  If the subsequent
 *	fopen fails, fatal is called.  If the fopen succeeds, the stream 
 *	(opfil)	is returned.
 */

static FILE *getfile(void)
{
	char *fptr;
	static int ctr = 0;
	FILE *opfil;

	if(create) {
		if (filelim_flag == 1)
			fatal(MSGSTR(FILELIM,"csplit: File limit reached.\n"),NULL);

		fptr = file; 
		fptr += prefix_lngth; 
                (void)sprintf(fptr,"%0*d",num_digits, ctr);
		ctr++;

		/* The current file is ok.  May not be ok for the next file  */
		/* to be created.					     */
		if (ctr > maxfiles)
			filelim_flag = 1;

		if((strcmp(file, infilename) == 0) || (opfil = fopen(file,"w")) == NULL) 
			fatal(MSGSTR(CANTCREAT,"csplit: Cannot create %s\n"),file);
		return(opfil);
	}
	return(NULL);
}

/*
 * NAME: getline
 *                                                                    
 * FUNCTION:
 *	Getline gets a line via fgets from the input stream "infile".
 *	The line is put into linbuf and may not be larger than LINSIZ.
 *	If getline is called with a non-zero value, the current line
 *	is bumped, otherwise it is not (for R.E. searching).
 */

static char *getline(int bumpcur)
{
	char *ret;
	if(bumpcur)
		curline++;
	ret=fgets(linbuf,LINSIZ,infile);
	return(ret);
}

/*
 * NAME: line_arg
 *                                                                    
 * FUNCTION:
 *	Line_arg handles line number arguments.
 *	line_arg takes as its argument a pointer to a character string
 *	(assumed to be a line number).  If that character string can be
 *	converted to a number (long), to_line is called with that number,
 *	otherwise error.
 */
static void
line_arg(char *line)
{
	long to;

	if(natol(line,&to) == ERR)
		fatal(MSGSTR(BADLNUM,"csplit: %s: bad line number\n"),line); /*MSG*/
	to_line(to);
}

/*
 * NAME: num_arg
 *                                                                    
 * FUNCTION: 
 *	Num_arg handles repeat arguments.
 *	Num_arg copies the numeric argument to "rep" (error if number is
 *	larger than 11 characters or } is left off).  Num_arg then converts
 *	the number and checks for validity.  Next num_arg checks the mode
 *	of the previous argument, and applys the argument the correct number
 *	of times. If the mode is not set properly its an error.
 */
static void
num_arg(char *arg, int md)
{
	long repeat, toline;
	char rep[12];
	char *ptr;

	ptr = rep;
	for(++arg; *arg != '}'; arg++) {
		if(ptr == &rep[11])
			fatal(MSGSTR(RPT2LNG,"csplit: %s: Repeat count too large\n"),targ); /*MSG*/
		if(*arg == (char) NULL)
			fatal(MSGSTR(MISSBRKT,"csplit: %s: missing '}'\n"),targ); /*MSG*/
		*ptr++ = *arg;
	}
	*ptr = (char) NULL;
	if((natol(rep,&repeat) == ERR) || repeat < 0L)
		fatal(MSGSTR(ILLRPT,"csplit: Illegal repeat count: %s\n"),targ); /*MSG*/
	if(md == LINMODE) {
		toline = offset = curline;
		for(;repeat > 0L; repeat--) {
			toline += offset;
			to_line(toline);
		}
	} else	if(md == EXPMODE)
			for(;repeat > 0L; repeat--)
				to_line(findline(regxpptr,offset));
		else
			fatal(MSGSTR(NOOP,"csplit: No operation for %s\n"),targ); /*MSG*/
}

/*
 * NAME: re_arg
 *                                                                    
 * FUNCTION:
 *	Re_arg handles regular expression arguments.
 *	Re_arg takes a csplit regular expression argument.  It checks for
 *	delimiter balance, computes any offset, and compiles the regular
 *	expression.  Findline is called with the compiled expression and
 *	offset, and returns the corresponding line number, which is used
 *	as input to the to_line function.
 */
static void
re_arg(char *string)
{
	char *ptr;
	char ch;
	int l;
	
	ch = *string;
      if(MB_CUR_MAX > 1) {
	ptr = string+1;
	while (*ptr != ch) {
		if(*ptr == '\\')
			++ptr;
		if ((l=mblen(ptr, MB_CUR_MAX))>0) 
			ptr += l;
		else
			fatal(MSGSTR(ILLCHAR,"csplit: Illegal character in pattern.\n"),"");
		if(*ptr == (char) NULL)
			fatal(MSGSTR(MISSDEL,"csplit: %s: missing delimiter\n"),targ);
		}
      }
      else 
      {
	ptr = string; 
	while(*(++ptr) != ch) {
		if(*ptr == '\\')
			++ptr;
		if(*ptr == (char) NULL)
			fatal(MSGSTR(MISSDEL,"csplit: %s: missing delimiter\n"),targ);
		}
      }
	*ptr='\0';
	if(natol(ptr+1,&offset) == ERR)
		fatal(MSGSTR(ILLOFF,"csplit: %s: illegal offset\n"),string); /*MSG*/
	if ((regstat=regcomp(regxpptr, string+1, 0)) != 0)
		fatal((char *) 0, (char *) 0);	/* will call prntregerr */
	*ptr = ch;
	to_line(findline(regxpptr,offset));
}

/*
 * NAME: sig
 *                                                                    
 * FUNCTION:
 *	Sig handles breaks.  When a break occurs the signal is reset,
 *	and fatal is called to clean up and print the argument which
 *	was being processed at the time the interrupt occured.
 */
static void
sig(void)
{
	(void)signal(SIGINT,(void (*)(int))sig);
	fatal(MSGSTR(INTSIG,"csplit: Interrupt - program aborted at arg '%s'\n"),targ);	/*MSG*/
}

/*
 * NAME: to_line
 *                                                                    
 * FUNCTION: 
 *	To_line creates split files.
 *	To_line gets as its argument the line which the current argument
 *	referenced.  To_line calls getfile for a new output stream, which
 *	does nothing if create is False.  If to_line's argument is not LAST
 *	it checks that the current line is not greater than its argument.
 *	While the current line is less than the desired line to_line gets
 *	lines and flushes (error if EOF is reached).
 *	If to_line's argument is LAST, it checks for more lines, and gets
 *	and flushes lines till the end of file.
 *	Finally, to_line calls closefile to close the output stream.
 */
static void
to_line(long ln)
{
	outfile = getfile();
	if(ln != LAST) {
		if(curline > ln)
			fatal(MSGSTR(OUTRNG,"csplit: %s - out of range\n"),targ); /*MSG*/
		while(curline < ln) {
			if(getline(TRUE) == NULL)
				fatal(MSGSTR(OUTRNG,"csplit: %s - out of range\n"),targ); /*MSG*/
			flush();
		}
	} else		/* last file */
		if(getline(TRUE) != NULL) {
			flush();
			while(TRUE) {
				if(getline(TRUE) == NULL)
					break;
				flush();
			}
		} else
			fatal(MSGSTR(OUTRNG,"csplit: %s - out of range\n"),targ); /*MSG*/
	closefile();
}
