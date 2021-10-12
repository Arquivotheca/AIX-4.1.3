static char sccsid[] = "@(#)02 1.16 src/bos/usr/bin/sccs/val.c, cmdsccs, bos41B, 9504A 12/9/94 10:00:11";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: escdodelt, findsid, initarg, nextarg, process,
 *            report, validate, main
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/************************************************************************/
/*									*/
/*  val -                                                               */
/*  val [-m name] [-r SID] [-s] [-y type] file ...                      */
/*                                                                      */
/************************************************************************/

#include 	<locale.h>
#include	<sys/types.h>
#include	"defines.h"
#include	"had.h"

#include 	"val_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_VAL, Num, Str)
#define PRINTF printf

# define	FILARG_ERR	0200	/* no file name given */
# define	UNKDUP_ERR	0100	/* unknown or duplicate keyletter */
# define	CORRUPT_ERR	040	/* corrupt file error code */
# define	FILENAM_ERR	020	/* file name error code */
# define        INVALSID_ERR    010     /* invalid SID error */
# define	NONEXSID_ERR	04	/* non-existent SID error code */
# define	TYPE_ERR	02	/* type arg value error code */
# define	NAME_ERR	01	/* name arg value error code */

static int	ret_code;	/* prime return code from 'main' program */
static int	inline_err;	/* input line error code (from 'process') */
static int	infile_err;	/* file error code (from 'validate') */
static char    **xargv;        /* command line arguments */
static char    **xargvp;       /* ptr into xargv */
static char    *linep;         /* ptr into line for command from stdin */
static int     silent;         /* set to suppress msgs */

static struct packet gpkt;
static struct sid sid;

static char	had[26];	/* had flag used in 'process' function */
static char    *type;          /* ptr to type (-y) value */
static char    *name;          /* ptr to name (-m) value */
static char    *prline;        /* line to print upon error */
static char	line[LINE_MAX+1];
char    *malloc();      /* function returning ptr to new memory */
char    *strcpy();      /* function returning ptr to copied string */

static struct  stat Statbuf;

nl_catd catd;

/* This is the main program that determines whether the command line
 * comes from the standard input or read off the original command
 * line.  See VAL(I) for more information.
*/
main(argc,argv)
int argc;
char	*argv[];
{
	Fflags = FTLJMP;

        (void)setlocale(LC_ALL,"");
	catd =catopen(MF_SCCS, NL_CAT_LOCALE);

	if (argc == 2 && argv[1][0] == '-' && !(argv[1][1])) {
		while (fgets(line,LINE_MAX+1,stdin))
			if (line[0] != '\n') {
				repl (line,'\n','\0');
				prline = line;
				process();
			}
	}
	else {
		xargv = argv+1;
		prline = NULL;
		process();
	}
	exit(ret_code);
}


/* Perform initialization before calls to nextarg() */
static initarg()
{
	static char *linebuf;

	if (xargvp = xargv) return;
	if (linebuf) free(linebuf);
	linep = linebuf = strcpy(malloc(size(line)),line);
}


/* Return a ptr to the next argument on the command line.
 * If the command line was read from stdin, arguments are delimited by
 * blanks and tabs.
 */
static char *
nextarg()
{
	register char *p;

	if (xargvp) return *xargvp++;

	p = linep + strspn(linep," \t");    /* skip blanks */
	if (!*p) return(NULL);
	linep = p + strcspn(p, " \t");      /* skip to next blank */
	if (*linep) *linep++ = '\0';
	return(p);
}


/* This function processes a command line.  It
 * determines which keyletter values are present on the command
 * line and assigns the values to the correct storage place.  It
 * then calls validate for each file name on the command line.
*/
static process()
{
	register char   *p;
	register int	testklt;
	int	num_files;
	char	c;

	silent = FALSE;
	num_files = inline_err = 0;

	/*
	clear out had flags for each 'line' processed
	*/
	zero(had,sizeof(had));
	/*
	scan for all flags.
	*/
	for (initarg(); p = nextarg();)
		if ((*p == '-') && (*++p != '-')) {
nextflag:
			testklt = TRUE;
			c = *p++;
			switch (c) {
				case 's':
					testklt = 0;
					/*
					turn on 'silent' flag.
					*/
					silent = TRUE;
					if (*p)
						goto nextflag;
					break;
				case 'r':
					/*
					check for invalid SID.
					*/
					if (!*p)
						p = nextarg();
					if (setjmp(Fjmp))
						inline_err |= INVALSID_ERR;
					else {
						extern char *sid_ab();

						chksid(sid_ab(p,&sid),&sid);
						if (((sid.s_rel < MINR) ||
						    (sid.s_rel > MAXR)) ||
						    (sid.s_rel > 0 && sid.s_lev == 0) ||
						    (sid.s_rel > 0 && sid.s_lev > 0
						    && sid.s_br > 0 && sid.s_seq == 0))
							inline_err |= INVALSID_ERR;
					}
					break;
				case 'y':
					if (!*p)
						p = nextarg();
					type = p;
					break;
				case 'm':
					if (!*p)
						p = nextarg();
					name = p;
					break;
				default:
					inline_err |= UNKDUP_ERR;
			}
			/*
			use 'had' array and determine if the keyletter
			was given twice.
			*/
			if ((c >= 'a' && c <= 'z') && 
				had[c - 'a']++ && testklt++)
				inline_err |= UNKDUP_ERR;
		}
		else {
			/*
			assume file name if no '-' preceded argument or
			if "--" preceeded argument.
			*/
			if (*p == '-') {
				if (*++p == NULL)
					p = nextarg();
				else
					inline_err |= UNKDUP_ERR;
			}
			if (p)
			 	num_files++;    /* at least 1 file found    */
			break;
		}
	/*
	check if any files were named as arguments
	*/
	if (num_files == 0)
		inline_err |= FILARG_ERR;
	/*
	report any errors in command line.
	*/
	report(inline_err,"val");
	/*
	loop through 'validate' for each file on command line.
	*/
	while ( p )
	{
		int validate();

		do_file(p,validate);
		p = nextarg();
	}
}


/* This function actually does the validation on the named file.
 * It determines whether the file is an SCCS-file or if the file
 * exists.  It also determines if the values given for type, SID,
 * and name match those in the named file.  An error code is returned
 * if any mismatch occurs.  See VAL(I) for more information.
*/
static validate(file)
char    *file;
{
	extern char             *auxf(), *sname();
	extern struct idel      *dodelt();
	extern char             *Sflags[];
	register char           *p;
	struct stats            stats;

	infile_err = 0;

	if (setjmp(Fjmp)) {
		infile_err |= gpkt.p_iop? CORRUPT_ERR : FILENAM_ERR;
		goto out;
	}
	sinit(&gpkt,file,2);
	/*
	read delta table checking for errors and/or
	SID.
	*/
	if (!dodelt(&gpkt,&stats,(struct sid *)NULL,0))
		fmterr(&gpkt);

	finduser(&gpkt);
	doflags(&gpkt);
	/*
	check if 'y' flag matched '-y' arg value.
	*/
	if (HADY)
		if (!(p = Sflags[TYPEFLAG - 'a']) || !equal(type,p))
			infile_err |= TYPE_ERR;
	/*
	check if 'm' flag matched '-m' arg value.
	*/
	if (HADM)
		if (!equal(name,
		    (p = Sflags[MODFLAG - 'a'])? p : auxf(sname(file),'g')))
			infile_err |= NAME_ERR;

	if (gpkt.p_line[1] != BUSERTXT || gpkt.p_line[0] != CTLCHAR)
		fmterr(&gpkt);
	flushto(&gpkt,EUSERTXT,1);

	/*
	If a valid sid was specified, determine whether it occurs.
	*/
	if (HADR && !(inline_err & INVALSID_ERR))
		if (!findsid())
			infile_err |= NONEXSID_ERR;

	/*
	read remainder of file so 'readmod'
	can check for corruptness.
	*/
	gpkt.p_chkeof = 1;
	while (readmod(&gpkt))
		;

out:    if (gpkt.p_iop) fclose(gpkt.p_iop);
	report(infile_err,file);
	ffreeall();
}


/* Determine whether the requested sid exists.
 * If the sid is unambiguous (R.L or R.L.B.S) it must occur as is.
 * If R is given, any R.... is sufficient.
 * If R.L.B is given, any R.L.B.... is sufficient.
 */
static findsid()
{
	register struct idel *rdp;
	register int n;

	for (n = maxser(&gpkt); n; n--) {
		rdp = &gpkt.p_idel[n];
		if (rdp->i_sid.s_rel == sid.s_rel)
			if (!sid.s_lev ||
			    sid.s_lev == rdp->i_sid.s_lev &&
			    sid.s_br == rdp->i_sid.s_br &&
			    (!sid.s_seq || sid.s_seq == rdp->i_sid.s_seq))
				return(TRUE);
	}
	return(FALSE);
}


/* This function will report the error that occurred on the command
 * line.  It will print one diagnostic message for each error that
 * was found in the named file.
*/
static report(code,file)
register int	code;
register char	*file;
{
	char	percent;

	ret_code |= code;
	if (!code || silent) return;
	percent = '%';		/* '%' for -m and/or -y messages */
	if (prline) {
		printf("%s\n\n",prline);
		prline = NULL;
	}
	if (code & NAME_ERR)
		PRINTF(MSGSTR(MMSMTCH, " %s: The value specified by -m does not match\n\
\tthe %cM%c identification keyword value specified in the SCCS file.\n"),file,percent,percent);  /* MSG */
	if (code & TYPE_ERR)
		PRINTF(MSGSTR(YMSMTCH, " %s: The parameter specified by -y\n\
\tdoes not match the text specified by %cY%c in the SCCS file.\n"),file,percent,percent);  /* MSG */
	if (code & NONEXSID_ERR)
		printf(MSGSTR(SIDNOEXST, " %s: The specified SID does not exist.\n\
\tCheck the p-file for the correct SID number.\n"),file);  /* MSG */
	if (code & INVALSID_ERR)
		printf(MSGSTR(SIDINVLD, " %s: The specified SID is not valid.\n\
\tCheck the p-file for the correct SID number.\n"),file);  /* MSG */
	if (code & FILENAM_ERR)
		printf(MSGSTR(OPNSCCS, " %s: Cannot open the file or\n\
\tthe file is not an SCCS file.\n\
\tCheck path name and permissions.\n\
\tThe val command validates SCCS files only.\n"),file);  /* MSG */
	if (code & CORRUPT_ERR)
		printf(MSGSTR(CRPTSCCS, " %s is a damaged SCCS file.\n\
\tThe specified SCCS file has been edited without use of SCCS conventions.\n\
\tRestore the most recent backup copy of this file.\n"),file);  /* MSG */
	if (code & UNKDUP_ERR)
		printf(MSGSTR(DUPKEYLTR, " %s: The specified flag does not exist\n\
\tfor this command or is a duplicate on the command line.\n"),file);  /* MSG */
	if (code & FILARG_ERR)
		printf(MSGSTR(MSGFILARG, " %s: Specify a file name or -.\n"),file);  /* MSG */
	if ((code & UNKDUP_ERR) || (code & FILARG_ERR))
		printf(MSGSTR(VAL_USAGE, " Usage: val [ -s ] [ -r SID ] [ -m Name ] [ -y Type ] File...\n"));
}

/* Null routine to satisfy external reference from dodelt() */
escdodelt()
{
}
