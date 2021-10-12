static char sccsid[] = "@(#)93  1.23  src/bos/usr/bin/sccs/admin.c, cmdsccs, bos41B, 9504A 12/9/94 09:59:45";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: adjust, admin, clean_up, cmt_ba, fgetchk, getval,
 *            main, pos_ser, putmrs, range, val_list
 *
 * ORIGINS: 3, 10, 27, 18
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
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

# include       <locale.h>
# include	"defines.h"
# include	"had.h"
# include       "admin_msg.h"
# include 	<stdlib.h>

#define MSGSTR(Num, Str) catgets(catd, MS_ADMIN, Num, Str)

/*
	Program to create new SCCS files and change parameters
	of existing ones. Arguments to the program may appear in
	any order and consist of keyletters, which begin with '-',
	and named files. Named files which do not exist are created
	and their parameters are initialized according to the given
	keyletter arguments, or are given default values if the
	corresponding keyletters were not supplied. Named files which
	do exist have those parameters corresponding to given key-letter
	arguments changed and other parameters are left as is.

	If a directory is given as an argument, each SCCS file within
	the directory is processed as if it had been specifically named.
	If a name of '-' is given, the standard input is read for a list
	of names of SCCS files to be processed.
	Non-SCCS files are ignored.

	Files created are given mode 444.
*/

# define MAXNAMES 9
# define COPY 0
# define NOCOPY 1

/* ADMIN_USAGE is the default UNKNWNFLG message from the message catalog. */

# define ADMIN_USAGE "Usage: %c is not a recognized flag. \n\
\tadmin {-n|-i[Name]} [-f{1{a|Number ...}|\n\
\t{b|cNumber|dSID|fNumber|j|mModule|n|qText|tType}} ...]\n\
\t[-fv[Program]] [-m[MRList]] [-aUser ...] [-rNumber.Number]\n\
\t[-t File] [-y[Comment]] File ...\n\
\tCreates and initializes SCCS files.\n\
\n\
\tUsage: admin [-aUser ...] [-eUser ...] [-f{1{a|Number ...}|\n\
\tb|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType} ...]\n\
\t[-d{1{a|Number ...}|b|c|d|f|i|j|m|n|q|t] ...] [-dv|-fv[Program]]\n\
\t[-t[File]] File ...\n\
\tChanges existing SCCS files.\n"

/* FILENOEXIST is the default FLEXST20 message from the message catalog. */

#define FILENOEXIST "File %s does not exist.  Check path name\n\
\tof existing SCCS file or use the -n flag or the -i flag\n\
\tto create a new SCCS file. (ad20)\n"

/* ILL_DAT is the default ILLGLDATA message from the message catalog. */

#define ILL_DAT "%1$s contains prohibited data on line %2$d.\n\
\tRemove the SOH ASCII character (binary 001) or precede it \n\
\twith a \\ (backslash). (ad21)\n"

/* B_DIR is the default BDIR message from the message catalog. */

#define B_DIR  "You used the -%2$c flag with directory %1$s.\n\
\tSpecify a file, not a directory, with the -i flag or the -t flag. (ad29)\n"
#define L_2_LONG "%1$s contains a line longer than 2048 characters on line %2$d.\n"


static struct stat Statbuf;

static char Null[1];
static char ErrMsg[1024];

static char *ifile, *tfile;
static char *z;	/* for validation program name */
extern FILE *Xiop;
static char	*getval();	/* function returning character ptr */
static char	*adjust();	/* function returning character ptr */
char	*auxf();
char    *logname();
static char had[NFLAGS], had_flag[NFLAGS], rm_flag[NFLAGS];
char	*Comments;
char	*Mrs;
static char Valpgm[]	=	"val";
static int irel, fexists, num_files;
static int	VFLAG  =  0;
static struct sid newsid;
extern char *Sflags[];
static char *anames[MAXNAMES], *enames[MAXNAMES];
static char	*unlock;
static char	*locks;
static char *flag_p[NFLAGS];
static int asub, esub;
static int check_id;
int Did_id;

nl_catd catd;

main(argc,argv)
int argc;
char *argv[];
{
	register int j;
	register char *p;
	char c, f;
	int i, testklt;
	extern admin();
	char *sid_ab();
	extern int Fcnt;
	struct sid sid;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);
	/*
	Set flags for 'fatal' to issue message, call clean-up
	routine and terminate processing.
	*/
	Fflags = FTLMSG | FTLCLN | FTLEXIT;

	testklt = 1;

	/*
	The following loop processes keyletters and arguments.
	Note that these are processed only once for each
	invocation of 'main'.
	*/
	for(j=1; j<argc; j++) {
		if (!strcmp(argv[j], "--")) {
			num_files+=argc-(j+1);
			p = argv[j+1];
			break;
		}
		if(argv[j][0] == '-' && (c = argv[j][1])) {
			p = &argv[j][2];
			switch (c) {

			case 'i':	/* name of file of body */
				ifile = p;
				if (*ifile && exists(ifile))
					if ((Statbuf.st_mode & S_IFMT) ==
						 S_IFDIR) {
					sprintf(ErrMsg,MSGSTR(BDIR, B_DIR),ifile,c);   /* MSG */
					fatal(ErrMsg);
				}
				break;

			case 't':	/* name of file of descriptive text */
				tfile = p;
				if (*tfile && exists(tfile))
					if ((Statbuf.st_mode & S_IFMT) ==
						S_IFDIR) {
					sprintf(ErrMsg,MSGSTR(BDIR, B_DIR),tfile,c);   /* MSG */
					fatal(ErrMsg);
				}
				break;
			case 'm':	/* mr flag */
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
					if (!p[0]) continue;
				}
				Mrs = p;
				repl(Mrs,'\n',' ');
				break;
			case 'y':	/* comments flag for entry */
				savecmt(p);
				break;
			case 'd':	/* flags to be deleted */
				testklt = 0;
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
				}
				if (!(f = *p++))
					fatal(MSGSTR(DNOARG,"admin: Specify a header flag with the -d flag. (ad1)\n"));   /* MSG */
				switch (f) {
				case IDFLAG:	/* see 'f' keyletter */
				case BRCHFLAG:	/* for meanings of flags */
				case VALFLAG:
				case TYPEFLAG:
				case MODFLAG:
				case QSECTFLAG:
				case NULLFLAG:
				case FLORFLAG:
				case CEILFLAG:
				case DEFTFLAG:
				case JOINTFLAG:
				   if (*p) {
				      sprintf(ErrMsg,
					 MSGSTR(CFLGVAL, "admin: The %c header flag does not require a value\n\twhen it is used with the -d flag. (ad12)\n"),f);   /* MSG */
				      fatal(ErrMsg);
				   }
				   break;
				case LOCKFLAG:
					if (*p) {
						/*
						set pointer to releases
						to be unlocked
						*/
						repl(p,',',' ');
						if (!val_list(p))
						        fatal(MSGSTR(BADLISTFMT,
 "admin: The specified version and release must be numeric.\n\tSeparate version and release numbers with a comma. (ad27)\n"));   /* MSG */
						if (!range(p))
						         fatal(MSGSTR(ELMNTRNG,
"admin: You cannot lock a version number or a release number\n\tless than 0 and greater than 9,999. (ad28)\n"));   /* MSG */
						if (*p != 'a')
							unlock = p;
					}
					break;

				default:
					sprintf(ErrMsg,MSGSTR(UNKNWNFLG, ADMIN_USAGE),f);   /* MSG */
					fatal(ErrMsg);
				}

				if (rm_flag[f - 'a']++) {
				   sprintf(ErrMsg,MSGSTR(FLGTWICE, "admin: Do not use the %c header flag more than once on the\n\tcommand line. (ad4)\n"),f);   /* MSG */
					fatal(ErrMsg);
				}
				break;

			case 'f':	/* flags to be added */
				testklt = 0;
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
				}
				if (!(f = *p++))
					fatal(MSGSTR(FFLGNOARG, "admin: Specify a header flag with the -f flag. (ad5)\n"));   /* MSG */
				switch (f) {
				case BRCHFLAG:	/* branch */
				case NULLFLAG:	/* null deltas */
				case JOINTFLAG:	/* joint edit flag */
				   if (*p) {
				      sprintf(ErrMsg,
				      MSGSTR(FLGVAL13, "admin: The %c header flag does not require a value\n\twhen it is used with the -f flag. (ad13)\n"),f);   /* MSG */
				      fatal(ErrMsg);
				   }
				   break;

				case IDFLAG:	/* id-kwd message (err/warn) */
					break;

				case VALFLAG:	/* mr validation */
					VFLAG++;
					if (*p)
						z = p;
					break;


				case FLORFLAG:	/* floor */
					if ((i = patoi(p)) == -1)
						fatal(MSGSTR(FLOORNOTNUM,
"admin: Specify a value greater than 0 and less than 10,000\n\twith the f header flag. (ad22)\n"));   /* MSG */
					if ((size(p) > 5) || (i < MINR) ||
							(i > MAXR))
						fatal(MSGSTR(FLOORRNG,
"admin: Specify a value greater than 0 and less than 10,000\n\
\twith the f header flag. (ad23)\n"));   /* MSG */
					break;

				case CEILFLAG:	/* ceiling */
					if ((i = patoi(p)) == -1)
						fatal(MSGSTR(CEILNONNUM,
"admin: Specify a value greater than 0 and less than 10,000\n\
\twith the c header flag. (ad24)\n"));   /* MSG */
					if ((size(p) > 5) || (i < MINR) ||
							(i > MAXR))
						fatal(MSGSTR(CEILRNG,
"admin: Specify a value greater than 0 and less than 10,000\n\
\twith the c header flag. (ad25)\n"));   /* MSG */
					break;

				case DEFTFLAG:	/* default sid */
					if (!(*p))
						fatal(MSGSTR(NODFLTSID, 
"admin: Specify an SID number with the d header flag. (ad14)\n"));   /* MSG */
					chksid(sid_ab(p,&sid),&sid);
					break;

				case TYPEFLAG:	/* type */
				case MODFLAG:	/* module name */
				case QSECTFLAG:	/* csect name */
					if (!(*p)) {
						sprintf(ErrMsg,
						MSGSTR(FLGNOVAL2, 
"admin: Specify a value for the %c header flag. (ad2)\n"),f);   /* MSG */
						fatal(ErrMsg);
					}
					break;
				case LOCKFLAG:	/* release lock */
					if (!(*p))
						/*
						lock all releases
						*/
						p = "a";
					/*
					replace all commas with
					blanks in SCCS file
					*/
					repl(p,',',' ');
					if (!val_list(p))
                                          fatal(MSGSTR(BADLISTFMT, 
"admin: The specified version and release must be numeric. \n\
\tSeparate version and release numbers with a comma. (ad27)\n"));   /* MSG */
					if (!range(p))
                                          fatal(MSGSTR(ELMNTRNG, 
"admin: You cannot lock a version number or a release number\n\tless than 0 and greater than 9,999. (ad28)\n"));   /* MSG */
					break;

				default:
					sprintf(ErrMsg,MSGSTR(UNKNWNFLG, ADMIN_USAGE),f);   /* MSG */
					fatal(ErrMsg);
				}

				if (had_flag[f - 'a']++) {
                                        sprintf(ErrMsg,MSGSTR(FLGTWICE, 
"admin: Do not use the %c header flag more than once on the\n\tcommand line. (ad4)\n"),f);   /* MSG */
					fatal(ErrMsg);
				}
				flag_p[f - 'a'] = p;
				break;

			case 'r':	/* initial release number supplied */
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
					if (!p[0]) continue;
				}
				 chksid(sid_ab(p,&newsid),&newsid);
				 if ((newsid.s_rel < MINR) ||
				     (newsid.s_rel > MAXR))
                                        fatal(MSGCM(ROUTOFRNG, 
"admin: Supply a value for the -r flag that is greater than 0 and\n\
\tless than 10,000. (cm23)\n"));   /* MSG */
				 break;

			case 'n':	/* creating new SCCS file */
			case 'h':	/* only check hash of file */
			case 'z':	/* zero the input hash */
				break;

			case 'a':	/* user-name allowed to make deltas */
				testklt = 0;
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
					if (!p[0]) continue;
				}
				if (!(*p))
                                       fatal(MSGSTR(AFLGARG, "admin: Specify a user name, group name or group number with\n\tthe -a flag. (ad8)\n"));   /* MSG */
				if (asub > MAXNAMES)
                                       fatal(MSGSTR(AFLGKEYLTRS, 
"admin: Do not use the -a flag more than 10 times on the\n\
\tcommand line. (ad9)\n"));   /* MSG */
				anames[asub++] = p;
				break;

			case 'e':	/* user-name to be removed */
				testklt = 0;
				/* Allow for blank separation. */
				if (!p[0]) {
					argv[j] = 0;
					j++;
					p = &argv[j][0];
					if (!p[0]) continue;
				}
				if (!(*p))
                                        fatal(MSGSTR(BADEARG, 
"admin: Specify user name, group name or group number you want to\n\tremove. (ad10)\n"));   /* MSG */
				if (esub > MAXNAMES)
                                        fatal(MSGSTR(EFLGKEYLTRS, 
"admin: Do not use the -e flag more than 10 times\n\
\ton the command line. (ad11)\n"));   /* MSG */
				enames[esub++] = p;
				break;

			default:
				sprintf(ErrMsg,MSGSTR(UNKNWNFLG,\
				 ADMIN_USAGE),c);
				fatal(ErrMsg);
			}

			if (had[c - 'a']++ && testklt++) {
				sprintf(ErrMsg,MSGCM(KEYLTRTWC, "admin: Use the -%c flag only once on the command line. (cm2)\n"),c);   /* MSG */
				fatal(ErrMsg);
			}
		}
		else
			num_files++;
	} /* end of for loop */

	if (num_files == 0)
		fatal(MSGCM(MISSFLNAM,"admin: Specify the file to process. (cm3)\n"));   /* MSG */

	if ((HADY || HADM) && ! (HADI || HADN))
		fatal(MSGSTR(ILLYORM,"admin: Use the -y flag or the -m flag only when you create\n\tan SCCS file. (ad30)\n"));
	if (HADI && num_files > 1) /* only one file allowed with `i' */
		fatal(MSGSTR(MTONEFIL, "admin: Create only one SCCS file at a time with the -i flag. (ad15)\n"));   /* MSG */
	if ((HADI || HADN) && ! logname())
		fatal(MSGCM(USERID,"admin: Internal error. The /etc/passwd file is not accessible.\n\tFollow local problem reporting procedures. (cm9)\n"));   /* MSG */

	setsig();

	/*
	Change flags for 'fatal' so that it will return to this
	routine (main) instead of terminating processing.
	*/
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;

	/*
	Call 'admin' routine for each file argument.
	*/
	for (j=(argc-num_files); j<argc; j++)
			do_file(argv[j],admin);

	exit(Fcnt ? 1 : 0);
}


/*
	Routine that actually does admin's work on SCCS files.
	Existing s-files are copied, with changes being made, to a
	temporary file (x-file). The name of the x-file is the same as the
	name of the s-file, with the 's.' replaced by 'x.'.
	s-files which are to be created are processed in a similar
	manner, except that a dummy s-file is first created with
	mode 444.
	At end of processing, the x-file is renamed with the name of s-file
	and the old s-file is removed.
*/

static struct packet gpkt;	/* see file defines.h */
static char	Zhold[BUFSIZ];	/* temporary z-file name */

static admin(afile)
char	*afile;
{
	struct deltab dt;	/* see file defines.h */
	struct stats stats;	/* see file defines.h */
	struct stat sbuf;
	FILE	*iptr, *fdfopen();
	register int k;
	register char *cp, *q;
	char	*in_f;		/* ptr holder for lockflag vals in SCCS file */
	char	nline[LINE_MAX+1];
	char	*getline();
	char	*p_lval, *tval=0, *lval=0;
	char	f;		/* character holder for flag character */
	char	line[LINE_MAX+1];
	char	*del_ba(), *fmalloc();
	int	i;		/* used in forking procedure */
	int	ck_it  =  0;	/* used for lockflag checking */
	int	status;		/* used for status return from fork */
	int	ceil, floor;
	extern	nfiles;
	extern	char had_dir;

	extern char	*acl_get();
	char		*sptr;

	if (setjmp(Fjmp))	/* set up to return here from 'fatal' */
		return;		/* and return to caller of admin */

	zero((char *)&stats,sizeof(stats));

	if (HADI && had_dir) /* directory not allowed with -i keyletter */
		fatal(MSGSTR(DIRIKYLTR, "admin: Name a file, not a directory, with the -i flag. (ad26)\n"));   /* MSG */

	fexists = exists(afile);

	if (HADI)
		HADN = 1;
	if (HADI || HADN) {
		if (HADM && !VFLAG)
			fatal(MSGCM(MRNOTALD, "admin: The SCCS file you specified does not allow MR numbers. (cm24)\n"));   /* MSG */

		if (VFLAG && !HADM)
			fatal(MSGCM(MRSREQ,"admin: Specify an MR number or numbers on the command line or as\n\tstandard input. (cm26)\n"));   /* MSG */

	}

	if (!HADN && HADR)
		fatal(MSGSTR(RONLYIORN, "admin: Use the -r flag only with the -i flag or the -n flag. (ad16)\n"));   /* MSG */

	if (HADN && HADT && !(*tfile))
		fatal(MSGSTR(TFLGNOARG, "admin: Specify a file name for the -t flag when you create\n\ta new SCCS file. (ad17)\n"));   /* MSG */

	if (HADN && HADD)
		fatal(MSGSTR(NODWITHN, "admin: Do not use the -d flag with the -n flag. (ad18)\n"));   /* MSG */

	if (HADN && fexists) {
		sprintf(ErrMsg,MSGSTR(FILEEXISTS, 
"admin: File %s exists.  Do not use the -n flag or\n\tthe -i flag with an existing file. (ad19)\n"),afile);   /* MSG */
		fatal(ErrMsg);
	}

	if (!HADN && !fexists) {
		sprintf(ErrMsg,MSGSTR(FLEXST20, FILENOEXIST),afile);   /* MSG */
		fatal(ErrMsg);
	}
	if (HADH) {
		/*
		   fork here so 'admin' can execute 'val' to
		   check for a corrupted file.
		*/
		if ((i = fork()) < 0)
			fatal(MSGCO(CANTFORK, "admin: Cannot create another process at this time.\n\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));   /* MSG */
		if (i == 0) {		/* child */
			/*
			   perform 'val' with appropriate keyletters
			*/
			execlp(Valpgm, Valpgm, "-s", afile, 0);
			sprintf(ErrMsg,MSGCO(CANTEXEC, 
"admin: Cannot execute %s.\n\tCheck path name and permissions or\n\
\tuse local problem reporting procedures. (co50)\n"),Valpgm);   /* MSG */
			fatal(ErrMsg);
		}
		else {
			wait(&status);	   /* wait on status from 'execlp' */
			if (status)
				fatal(MSGCO(CORUPTFILE, 
"admin: The file is damaged.\n\tUse local problem reporting procedures. (co6)\n"));   /* MSG */
			return;		/* return to caller of 'admin' */
		}
	}

	/*
	Lock out any other user who may be trying to process
	the same file.
	*/
	if (!HADH && lockit(copy(auxf(afile,'z'),Zhold),2,getpid()))
		fatal(MSGCM(LOCKCREAT, "admin: Cannot lock the specified file.\n\
\tCheck path name and permissions or\n\
\twait until the file is not in use. (cm4)\n"));   /* MSG */

	if (fexists)
		sinit(&gpkt,afile,1);	/* init pkt & open s-file */
	else {
		xfcreat(afile,0444);	/* create dummy s-file */
		sinit(&gpkt,afile,0);	/* and init pkt */
	}

	if (!HADH)
		/*
		   set the flag for 'putline' routine to open
		   the 'x-file' and allow writing on it.
		*/
		gpkt.p_upd = 1;

	if (HADZ) {
		gpkt.do_chksum = 0;	/* ignore checksum processing */
		gpkt.p_ihash = 0;
	}

	/*
	Get statistics of latest delta in old file.
	*/
	if (!HADN) {
		stats_ab(&gpkt,&stats);
		gpkt.p_wrttn++;
		newstats(&gpkt,line,"0");
	}

	if (HADN) {		/*   N E W   F I L E   */

		/*
		Beginning of SCCS file.
		*/
		sprintf(line,"%c%c%s\n",CTLCHAR,HEAD,"00000");
		putline(&gpkt,line);

		/*
		Statistics.
		*/
		newstats(&gpkt,line,"0");

		dt.d_type = 'D';	/* type of delta */

		/*
		Set initial release, level, branch and
		sequence values.
		*/
		if (HADR)
			{
			 dt.d_sid.s_rel = newsid.s_rel;
			 dt.d_sid.s_lev = newsid.s_lev;
			 dt.d_sid.s_br  = newsid.s_br ;
			 dt.d_sid.s_seq = newsid.s_seq;
			 if (dt.d_sid.s_lev == 0) dt.d_sid.s_lev = 1;
			 if ((dt.d_sid.s_br) && ( ! dt.d_sid.s_seq))
				dt.d_sid.s_seq = 1;
			}
		else
			{
			 dt.d_sid.s_rel = 1;
			 dt.d_sid.s_lev = 1;
			 dt.d_sid.s_br = dt.d_sid.s_seq = 0;
			}
		time(&dt.d_datetime);		/* get time and date */

		copy(logname(),dt.d_pgmr);	/* get user's name */

		dt.d_serial = 1;
		dt.d_pred = 0;

		del_ba(&dt,line);	/* form and write */
		putline(&gpkt,line);	/* delta-table entry */

		/*
		If -m flag, enter MR numbers
		*/

		if (Mrs) {
			mrfixup();
			if (z && valmrs(&gpkt,z))
				fatal(MSGCM(INVMRS, "admin: Use a valid MR number or numbers. (cm25)\n"));   /* MSG */
			putmrs(&gpkt);
		}

		/*
		Enter comment line for `chghist'
		*/

		if (HADY)
			putline(&gpkt,Comments);
		else {
			/*
			insert date/time and pgmr into comment line
			*/
			cmt_ba(&dt,line);
			putline(&gpkt,line);
		}
		/*
		End of delta-table.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EDELTAB);
		putline(&gpkt,line);

		/*
		Beginning of user-name section.
		*/
		sprintf(line,CTLSTR,CTLCHAR,BUSERNAM);
		putline(&gpkt,line);
	}
	else
		/*
		For old file, copy to x-file until user-name section
		is found.
		*/
		flushto(&gpkt,BUSERNAM,COPY);

	/*
	Write user-names to be added to list of those
	allowed to make deltas.
	*/
	if (HADA)
		for (k = 0; k < asub; k++) {
			sprintf(line,"%s\n",anames[k]);
			putline(&gpkt,line);
		}

	/*
	Do not copy those user-names which are to be erased.
	*/
	if (HADE && !HADN)
		while ((cp = getline(&gpkt)) &&
				!(*cp++ == CTLCHAR && *cp == EUSERNAM)) {
			for (k = 0; k < esub; k++) {
				cp = gpkt.p_line;
				while (*cp)	/* find and */
					cp++;	/* zero newline */
				*--cp = '\0';	/* character */

				if (equal(enames[k],gpkt.p_line)) {
					/*
					Tell getline not to output
					previously read line.
					*/
					gpkt.p_wrttn = 1;
					break;
				}
				else
					*cp = '\n';	/* restore newline */
			}
		}

	if (HADN) {		/*   N E W  F I L E   */

		/*
		End of user-name section.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EUSERNAM);
		putline(&gpkt,line);
	}
	else
		/*
		For old file, copy to x-file until end of
		user-names section is found.
		*/
		if (!HADE)
			flushto(&gpkt,EUSERNAM,COPY);

	/*
	For old file, read flags and their values (if any), and
	store them. Check to see if the flag read is one that
	should be deleted.
	*/
	if (!HADN)
		while ((cp = getline(&gpkt)) &&
				(*cp++ == CTLCHAR && *cp == FLAG)) {

			gpkt.p_wrttn = 1;	/* don't write previous line */

			cp += 2;	/* point to flag character */
			k = *cp - 'a';
			f = *cp;
			if (f == LOCKFLAG) {
				i=0;

				p_lval = cp;
				++p_lval;
				tval = fmalloc(size(gpkt.p_line)-5);
				copy(++p_lval,tval);
				lval = tval;
				while(tval[i])
					++i;
				tval[i-1] = '\0';
			}

			if (!had_flag[k] && !rm_flag[k]) {
				had_flag[k] = 2;	/* indicate flag is */
							/* from file, not */
							/* from arg list */

				if (*++cp != '\n') {	/* get flag value */
					q = fmalloc(size(gpkt.p_line)-5);
					copy(++cp,q);
					flag_p[k] = q;
					while (*q)	/* find and */
						q++;	/* zero newline */
					*--q = '\0';	/* character */
				}
			}
			if (rm_flag[k])
				if (f == LOCKFLAG) {
					if (unlock) {
						in_f = lval;
						if ((lval = adjust(in_f)) &&
							!had_flag[k])
							ck_it = had_flag[k] = 1;
					}
					else had_flag[k] = 0;
				}
				else had_flag[k] = 0;
		}


	/*
	Write out flags.
	*/
	for (k = 0; k < NFLAGS; k++)
		if (had_flag[k]) {
			if (flag_p[k] || lval) {
				if (('a' + k) == LOCKFLAG && had_flag[k] == 1) {
					if(!(strcmp(flag_p[k], "a")) || !(strcmp(lval, "a")))
						locks = "a";
					else if (lval && flag_p[k])
						locks =
						cat(nline,lval," ",flag_p[k],0);
					else if (lval)
						locks = lval;
					else locks = flag_p[k];
					sprintf(line,"%c%c %c %s\n",
						CTLCHAR,FLAG,'a' + k,locks);
					locks = 0;
					if (tval) {
						ffree(tval);
						tval = lval = 0;
					}
					if (ck_it)
						had_flag[k] = ck_it = 0;
				}
				else if(('a' + k) == CEILFLAG 
					&& flag_p[FLORFLAG - 'a']) {
					if((ceil = patoi(flag_p[k])) < (floor = patoi(flag_p[FLORFLAG - 'a']))) {
						sprintf(ErrMsg,MSGSTR(CEILCONS,"admin: The ceiling value %1$u is less than the lowest release(the floor). (ad31)\n"), ceil);
						fatal(ErrMsg);
					}
					else sprintf(line,"%c%c %c %s\n",
						CTLCHAR,FLAG,'a'+k,flag_p[k]);
				}
				else if(('a' + k) == FLORFLAG 
					&& flag_p[CEILFLAG - 'a']) {
					if((floor = patoi(flag_p[k])) > (ceil = patoi(flag_p[CEILFLAG - 'a']))) {
						sprintf(ErrMsg,MSGSTR(FLORCONS,"admin: The floor value %1$u is greater than the highest release(the ceiling). (ad32)\n"), floor);
						fatal(ErrMsg);
					}
					else sprintf(line,"%c%c %c %s\n",
						CTLCHAR,FLAG,'a'+k,flag_p[k]);
				}
				else sprintf(line,"%c%c %c %s\n",
					CTLCHAR,FLAG,'a'+k,flag_p[k]);
			}
			else
				sprintf(line,"%c%c %c\n",
					CTLCHAR,FLAG,'a'+k);

			/* flush imbeded newlines from flag value */
			i = 4;
			if (line[i] == ' ')
				for (i++; line[i+1]; i++)
					if (line[i] == '\n')
						line[i] = ' ';
			putline(&gpkt,line);

			if (had_flag[k] == 2) {	/* flag was taken from file */
				had_flag[k] = 0;
				if (flag_p[k]) {
					ffree(flag_p[k]);
					flag_p[k] = 0;
				}
			}
		}

	if (HADN) {
		/*
		Beginning of descriptive (user) text.
		*/
		sprintf(line,CTLSTR,CTLCHAR,BUSERTXT);
		putline(&gpkt,line);
	}
	else
		/*
		Write out BUSERTXT record which was read in
		above loop that processes flags.
		*/
		gpkt.p_wrttn = 0;
		putline(&gpkt,(char *) 0);

	/*
	Get user description, copy to x-file.
	*/
	if (HADT) {
		if (*tfile) {
			iptr = xfopen(tfile,0);
			fgetchk(line,LINE_MAX + 1,iptr,tfile,&gpkt);
			fclose(iptr);
		}

		/*
		If old file, ignore any previously supplied
		commentary. (i.e., don't copy it to x-file.)
		*/
		if (!HADN)
			flushto(&gpkt,EUSERTXT,NOCOPY);
	}

	if (HADN) {		/*   N E W  F I L E   */

		/*
		End of user description.
		*/
		sprintf(line,CTLSTR,CTLCHAR,EUSERTXT);
		putline(&gpkt,line);

		/*
		Beginning of body (text) of first delta.
		*/
		sprintf(line,"%c%c %u\n",CTLCHAR,INS,1);
		putline(&gpkt,line);

		if (HADI) {		/* get body */
			register int lctr=0;

			/*
			Set indicator to check lines of body of file for
			keyword definitions.
			If no keywords are found, a warning
			will be produced.
			*/
			check_id = 1;
			/*
			Set indicator that tells whether there
			were any keywords to 'no'.
			*/
			Did_id = 0;
			if (*ifile)
				iptr = xfopen(ifile,0);	/* from a file */
			else
				iptr = stdin;	/* from standard input */

			/*
			Read and copy to x-file, while checking
			first character of each line to see that it
			is not the control character (octal 1).
			Also, count lines read, and set statistics'
			structure appropriately.
			The 'fgetchk' routine will check for keywords.
			*/
                        lctr = fgetchk(line,LINE_MAX + 1,iptr,ifile,&gpkt);
                        stats.s_ins = ( lctr > 99999 ) ? 99999 : lctr;
                        if ( lctr > 99999 )
                                fprintf (stderr, MSGSTR (LINECNTWARN,
"admin: WARNING - The number of lines in this file exceeds\n\
\t 99,999. The header section of the s. file will record 99999 in the\n\
\tlines inserted field. This is only a warning.\n"));
			stats.s_del = stats.s_unc = 0;


			/*
			If no keywords were found, issue warning.
			*/
			if (!Did_id) {
				if (had_flag[IDFLAG - 'a'])
					if(!(flag_p[IDFLAG -'a']))
						fatal(MSGCM(NOIDKEYWRDS,
"admin: The file must contain SCCS identification keywords.\n\
\tInsert one or more SCCS identification keywords into the file. (cm6)\n")); 
/* MSG */
					else
						fatal(MSGCM(INVIDKYWRDS,
"admin: The SCCS file requires one or more specific\n\
\tidentification keywords.\n\
\tAdd the keywords to the file. (cm10)\n"));   /* MSG */
				else
					fprintf(stderr,MSGCM(NOIDKEYWRDS7,
"admin: There are no SCCS identification keywords in the file. (cm7)\n"));   /* MSG */
			}

			check_id = 0;
			Did_id = 0;
		}

		/*
		End of body of first delta.
		*/
		sprintf(line,"%c%c %u\n",CTLCHAR,END,1);
		putline(&gpkt,line);
	}
	else {
		/*
		Indicate that EOF at this point is ok, and
		flush rest of (old) s-file to x-file.
		*/
		gpkt.p_chkeof = 1;
		while (getline(&gpkt)) ;
	}

	/*
	Flush the buffer, take care of rewinding to insert
	checksum and statistics in file, and close.
	*/
	flushline(&gpkt,&stats);

	/*
	Change x-file name to s-file, and delete old file.
	Unlock file before returning.
	*/
	if (!HADH) {
		if (!HADN) {
			stat(gpkt.p_file,&sbuf);
			sptr = acl_get(gpkt.p_file);
		}
		rename(auxf(gpkt.p_file,'x'),gpkt.p_file);
		if (!HADN) {
			acl_put(gpkt.p_file, sptr, 1);
			chown(gpkt.p_file,sbuf.st_uid,sbuf.st_gid);
		}
		xrm(&gpkt);
		unlockit(auxf(afile,'z'),getpid());
	}
}


#if 1

static fgetchk(strp,len,inptr,file,pkt)
register char *strp;
register int len;
FILE *inptr;
register char *file;
register struct packet *pkt;
{
	register int c, k, l;
	int lastc = '\n';
	register char *s;

	k = 1;
	l = 0;
	s = strp;
	while ( (c=getc(inptr)) != EOF ) {
		/*
		 * Make sure line does not start with Ctrl-A
		 */
		if ( l == 0 && c == CTLCHAR ) {
			sprintf(ErrMsg,MSGSTR(ILLGLDATA, ILL_DAT), file,k);   /* MSG */
			fatal(ErrMsg);
		}

		/*
		 * Make sure there are no nulls in the file.
		 */
		if ( c == '\0' ) {
			sprintf(ErrMsg,MSGSTR(ILLGLDATA, ILL_DAT), file,k);   /* MSG */
			fatal(ErrMsg);
		}

		/*
		 * Check line length
		 */
		if ( ++l >= len ) {
			sprintf(ErrMsg,MSGSTR(LINE2LONG, L_2_LONG), file,k);   /* MSG */
			fatal(ErrMsg);
		}

		if ( (*s++ = c)  == '\n' ) {
			*s = '\0';
			++k;
			l = 0;

			if (check_id)
				chkid(strp,flag_p['i'-'a']);

			putline(pkt,strp);

			s = strp;
		}

		lastc = c;

	}

	/*
	 * The last character in the file must be a newline
	 */
	if ( lastc != '\n')
		fatal(MSGCO(PRMTREOF,"admin: The end of the file was premature.\n\
\tMake sure that the last line of the file ends with a newline character or\n\
\tuse local problem reporting procedures. (co5)\n"));		/*MSG*/

	return(k - 1);
}

#else

static fgetchk(strp,len,inptr,file,pkt)
register char *strp;
register int len;
FILE *inptr;
register char *file;
register struct packet *pkt;
{
	register int k;

	for (k = 1; fgets(strp,len,inptr); k++) {
		if (*strp == CTLCHAR) {
			sprintf(ErrMsg,MSGSTR(ILLGLDATA, ILL_DAT), file,k);   /* MSG */
			fatal(ErrMsg);
		}

		if (check_id)
			chkid(strp,flag_p['i'-'a']);

		putline(pkt,strp);
	}

        if (strp[(strlen(strp)) - 1] != '\n')
		fatal(MSGCO(PRMTREOF,"admin: The end of the file was premature.\n\
\tMake sure that the last line of the file ends with a newline character or\n\
\tuse local problem reporting procedures. (co5)\n"));		/*MSG*/

	return(k - 1);
}
#endif


static clean_up()
{
	xrm(&gpkt);
	if (Xiop)
		fclose(Xiop);
	if(gpkt.p_file[0])
		unlink(auxf(gpkt.p_file,'x'));
	if (HADN)
		unlink(gpkt.p_file);
	if (!HADH)
		unlockit(Zhold,getpid());
}


static cmt_ba(dt,str)
register struct deltab *dt;
char *str;
{
	register char *p;
	char *date_ba();

	p = str;
	*p++ = CTLCHAR;
	*p++ = COMMENTS;
	*p++ = ' ';
	copy(MSGSTR(DATETIME, "date and time created"),p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	date_ba(&dt->d_datetime,p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	copy(MSGSTR(BYMSG, "by"),p);
	while (*p++)
		;
	--p;
	*p++ = ' ';
	copy(dt->d_pgmr,p);
	while (*p++)
		;
	--p;
	*p++ = '\n';
	*p = 0;
}


static putmrs(pkt)
struct packet *pkt;
{
	register char **argv;
	char str[64];
	extern char **Varg;

	for (argv = &Varg[VSTART]; *argv; argv++) {
		sprintf(str,"%c%c %s\n",CTLCHAR,MRNUM,*argv);
		putline(pkt,str);
	}
}


char	*adjust(line)
char	*line;
{
	register int k;
	register int i;
	char	*t_unlock;
	char	t_line[LINE_MAX+1];
	char	rel[5];

	t_unlock = unlock;
	while(*t_unlock) {
		NONBLANK(t_unlock);
		t_unlock = getval(t_unlock,rel);
		while ((k = pos_ser(line,rel)) != -1) {
			for(i = k; i < (size(rel) + k); i++) {
				line[i] = '+';
				if (line[i++] == ' ')
					line[i] = '+';
				else if (line[i] == '\0')
					break;
				else --i;
			}
			k = 0;
			for(i = 0; i < length(line); i++)
				if (line[i] == '+')
					continue;
				else if (k == 0 && line[i] == ' ')
					continue;
				else t_line[k++] = line[i];
			if (t_line[strlen(t_line) - 1] == ' ')
				t_line[strlen(t_line) - 1] = '\0';
			t_line[k] = '\0';
			line = t_line;
		}
	}
	if (length(line))
		return(line);
	else return(0);
}


char	*getval(sourcep,destp)
register char	*sourcep;
register char	*destp;
{
	while (*sourcep != ' ' && *sourcep != '\t' && *sourcep != '\0')
		*destp++ = *sourcep++;
	*destp = 0;
	return(sourcep);
}


static val_list(list)
register char *list;
{
	register int i;

	if(!(strcmp(list, "a")))
		return(1);
	else for(i = 0; list[i] != '\0'; i++)
		if ((mblen(list, MB_CUR_MAX) == 1)) { 
			if(list[i] == ' ' || NUMERIC((int)(list[i])))
				continue;
			else if (list[i] == 'a' 
					&& (list[i+1] == ' ' || list[i+1] == '\0')) {
				list[0] = 'a';
				list[1] = '\0';
				return(1);
			}
			else return(0);
		}
		else return(0);
	return(1);
}


static pos_ser(s1,s2)
char	*s1;
char	*s2;
{
	register int offset;
	register char *p;
	char	num[5];

	p = s1;
	offset = 0;

	while(*p) {
		NONBLANK(p);
		p = getval(p,num);
		if (equal(num,s2)) {
			return(offset);
}
		offset = offset + size(num);
	}
	return(-1);
}


static range(line)
register char *line;
{
	register char *p;
	char	rel[LINE_MAX+1];

	p = line;
	if (*p == 'a') return(1);
	while(*p) {
		NONBLANK(p);
		p = getval(p,rel);
		if (size(rel) > 5)
			return(0);
		else if((strtoul(rel, NULL, 10)) == 0)
			return(0);
	}
	return(1);
}
