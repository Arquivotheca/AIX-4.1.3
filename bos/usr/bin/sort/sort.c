static char sccsid[] = "@(#)98	1.59  src/bos/usr/bin/sort/sort.c, cmdfiles, bos412, 9446C 11/14/94 16:46:54";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: sort
 *
 * ORIGINS: 3, 18, 27
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 *
 * static char rcsid[] = "RCSfile: sort.c,v  Revision: 2.8.2.2  (OSF) Date: 90/10/12 15:46:37 ";
 *
 */

/*   fixed for peachtree enhancement, MAXMEM, TREEZ and cmpa()     */
/*   please refer to H.Stuettgen and A.Schuur  id: HJS-S           */

/*   This version of the sort for OSF/1 (from AIX 3.1) uses strxfrm and strcoll.
 *   To increase performance in the NLS case, the sort is not done
 *   in wchar_t's, but in char format. For a 'plain' sort, i.e.
 *   without any keys or flags, the compare is done using strcoll.
 *   If keys are specified, or folding/dictionary/printable flags,
 *   keys are extracted and converted to strxfrm format and prepended
 *   to the record. String compares are then done using strcmp.
 *   On final pass, the keys are stripped before records are written
 *   to output file.
 *   Max. record size is set to 20K. For keyed operations, the max.
 *   size is 3 times this (60K incl. strxfrm format keys).
 *   In checksort and merge operations, keys are not prepended but
 *   compared "in situ"; actual record max length is 20K.
 *   The "-A" option uses old-fashioned AT&T algorithms; the performance
 *   is about 10 times the collating sort. Use it when you can!
 */
/*
 * NAME: sort
 * FUNCTION: Sorts or merges files
 * FLAGS
 *  -A     Sorts on a byte-by-byte basis using ASCII character values.
 *  -b     Ignores leading blanks, spaces, and tabs
 *  -c     Checks that the input is sorted according to the ordering rules
 *  -d     Sorts in dictionary order.
 *  -f     Merges uppercase and lowercases letters.
 *  -i     Sorts only by character in the ASCII range octal 040 - 0176
 *  -m     Merges only, the input is already sorted
 *  -n     Sorts any initial numeric strings
 *  -o fl  Directs output to fl instead of stdout
 *  -r     reverses the order of the specified sort
 *  -t ch  Sets field separator character to char.
 *  -u     Suppresses all but one in each set of equal lines.
 *  -T dir Places all the tempory files that are created in the directory "dir"
 *  -y KB  Start up using KB kilobytes of storage.
 *  -z rsz Use rsz size records for reading in lines.
 */
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <values.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include "sort_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,SORTMSG2,Num,Str)
#define N       16
#define	C	20
#define NF	10
#define MTHRESH  8  /* threshhold for doing median of 3 qksort selection */
#define TREEZ  512  /* no less than N and best if power of 2   HJS-S-C   */
#define LINE  1024
#define SORT_LINE_MAX  20480

static int mb_cur_max; /* max number of bytes per character in current locale */

/*
 * Memory administration
 *
 * Using a lot of memory is great when sorting a lot of data.
 * Using a megabyte to sort the output of `who' loses big.
 * MAXMEM, MINMEM and DEFMEM define the absolute maximum,
 * minimum and default memory requirements.  Administrators
 * can override any or all of these via defines at compile time.
 * Users can override the amount allocated (within the limits
 * of MAXMEM and MINMEM) on the command line.
 */

#ifndef	MAXMEM
#define MAXMEM  4194304 /* 4  Megabyte maximum */      /*   HJS-S-C   */
#endif

#ifndef	MINMEM
#define	MINMEM	  16384	/* 16K minimum */
#endif

#ifndef	DEFMEM
#define DEFMEM    262144      /* start 256kb HJS-S-C  */
#endif


#define ASC 	0
#define NUM	1
#define XSTR	2
#define XNUM    3
#define sorting 0
#define merging 1

/* For <blank> detection: define macros for isblank() for SBCS and
 * iswblank() for wchar with MBCS. POSIX defines function of -b, -d, and
 * -n options options and default field separators in terms of <blank>
 * character class. Standard bindings do not provide isblank() or
 * iswblank(), so sort must provide its own.
 */
static wctype_t blankhandle;
#define iswblank(wc) (is_wctype((wc),blankhandle))
#define isblank(c) (is_wctype((wchar_t)(c),blankhandle))

/* Variables for determining collation requirements */
static unsigned char lc_collate[NL_LANGMAX+1];
static int POSIXlc_collate;

static int     Aflag;       /* used to signal the -A option ... i.e. byte compare */
static int	posflag;	/* used to signal the +pos option */

static FILE	*os;
static char	*dirtry[] = {"/var/tmp", "/usr/tmp", "/tmp", '\0'};
static char	**dirs;
static char	file1[100];
static char	*file = file1;
static char	*filep;
#define NAMEOHD 12 /* sizeof("/stm00000aa") */
static int	nfiles;

static int	*lspace;
	/* Layout of lspace:
	 * lspace = address of low end of temporary sort area
	 * cp = next address at or above lspace usable for chars of records
	 * lp = next address below ep usable for address of next record
	 *	(next record from fgetrec will be stored starting at cp++,
	 *	 and its address will be stored at lp--.)
	 * ep = next address above high end of temporary sort area
	 */
static int *newlspace;	/* For conditional realloc(lspace,...) */

static unsigned tryfor;
static unsigned alloc,oldalloc;
static char bufin[BUFSIZ], bufout[BUFSIZ];	/* Use setbuf's to avoid malloc calls.
					*/
static char tbuf[SORT_LINE_MAX];		/* buffers for strxfrm use */
static char ebuf[SORT_LINE_MAX];
static char xbuf[SORT_LINE_MAX*3+1];		/* assume strxfrm < 3x data */
static char *te = tbuf + SORT_LINE_MAX-1;
static char *xe = xbuf + (SORT_LINE_MAX*3);	

static int	maxrec;

static int 	mflg;
static int	nway;
static int	cflg;
static int	uflg;		/* -u option */
static int	uflgactive;	/* Suppress final entire-record compare if otherwise equal */
static int	outflag = 0;	/* 1 = writing output to customer output                  */
			/* 0 = writing output (if any) to intermediate files for  */
			/*     later merge because internal area overflows MAXMEM.*/
static char	*outfil;
static int unsafeout;	/*kludge to assure -m -o works*/
static int 	eargc;
static char	**eargv;
static struct btree {
    char *rp;
    int  rn;
} tree[TREEZ], *treep[TREEZ];
static int	blkcnt[TREEZ];
static long	wasfirst = 0, notfirst = 0;
static int	bonus;
static char	**blkcur[TREEZ];
static wchar_t tabchar;
static struct lconv *loc;
static wchar_t dec;
static wchar_t	decmon;
static wchar_t thsep;
static wchar_t thsepmon;
static char zero[256];
static wchar_t wcoptarg[_POSIX2_LINE_MAX*2]; /* optarg converted to widechar */
static int mbcodeset;	/* 0=current locale SBCS, 1=current locale MBCS */
				/* The following tables are not used  by the
				 * normal sort. However, the "-A" option uses
				 * them...
				 */
static char	fold[256] = {		/* table folds ASCII lowers to uppers */
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0135,0136,0137,
	0140,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0173,0174,0175,0176,0177,
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377
};
static char nofold[256] = {
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0135,0136,0137,
	0140,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0152,0153,0154,0155,0156,0157,
	0160,0161,0162,0163,0164,0165,0166,0167,
	0170,0171,0172,0173,0174,0175,0176,0177,
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377
};

static char	nonprint[256] = {
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

static char	dict[256] = {
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};



static struct	field {
	char *code;
	char *ignore;
	int fcmp;
	int rflg;
	int bflg[2];
	int m[2];
	int n[2];
}	fields[NF];
static struct field proto = {
	nofold,
	zero,
	ASC,
	1,
	0,0,
	0,-1,
	0,0
};
static int	nfields;
static int 	error = 2;
static int 	exit_status = 0;
static int	cmpset = 0;
static int	pos1flag;	/* pos1flag: nonzero iff a "+pos1" option has been
			 * seen but no corresponding "-pos2" option.
			 */


static void 	sort(void);
static void 	msort(char **a, char **b);
static void 	insert(struct btree **tp, int n);
static void 	merge(int a, int b);
static void 	cline(register char *tp, register char *fp);
static int	rline(register FILE *iop, register char *s);
static void 	wline(char *s);
static void 	checksort(void);
static void 	disorder(char *s, char *t);
static void 	newfile(void);
static char 	*setfil(register int i);
static void 	oldfile(void);
static void 	safeoutfil(void);
static void 	cant(char *f);
static void 	diag(char *s, char *t);
static void 	newdiag(char *s, char *t);
static void 	term(void);
static int	cmp(char *a, char *b);
static int	cmpa(register char *pa, register char *pb);
static int	Acmp(char *i, char *j);
static int	Acmpa(register char *pa, register char *pb);
static char 	*skip( register char *p, struct field *fp, int j);
static char 	*eol(register char *p);
static void 	copyproto(void);
static void 	initree(void);
static int 	cmpsave(register int n);
static void	newoptfield(char *s,int spos,int ink, struct field *p);
static void	convoptarg(char *s);
static void	Usage(void);
static void 	qksort(char **a, char **l);
static void 	rderror(char *s);
static void 	wterror(int x);
static int	fgetrec(char *s, int n, FILE *stream, char *f, int *partial);
static void 	fldtowa(char *p, char *l, char *b, char *ignore, char *code);
static unsigned grow_core(unsigned size, unsigned cursize);

static int	(*compare)(char *a, char *b) = cmpa;

main(int argc, char *argv[])
{
	register a;
	char *arg;
	struct field *p, *q;
	int i;
	int c;		/* option character */
	int kopt;	/* -k option has been seen.*/
	int badopt;	/* at least one command line syntax error */
	wchar_t *wcoptargend;
	char *optargend;
	char **oargv;	/* oargv,oartc,oargi: for checking obsolete -o location */
	int oargc,oargi,skipargs;
	int signedalloc;
#ifdef DEBUG_OPTS
int Di;
char Dig,Dco;
#endif

	setlocale(LC_ALL,"");
	/* Determine whether to use ascii or locale-dependent collation.  */
	/* This lets English fall through into locale-dependent collation.*/
	strcpy(lc_collate , setlocale(LC_COLLATE,NULL));
	POSIXlc_collate = ((strcmp(lc_collate,"C")==0) || (strcmp(lc_collate,"POSIX")==0));
	if (POSIXlc_collate) {
		Aflag = 1;
		compare = Acmpa;
	}

	blankhandle = get_wctype("blank");

	/* close any file descriptors that may have been */
	/* left open -- we may need them all		*/
	for (i = 3; i < 3 + N; i++)
		(void) close(i);
	catd = catopen(MF_SORT, NL_CAT_LOCALE);
	loc = localeconv();
	dec = loc->decimal_point[0];
	decmon = loc->mon_decimal_point[0];
	thsep = (wchar_t)loc->thousands_sep[0];
	thsepmon = (wchar_t)loc->mon_thousands_sep;
	mb_cur_max = MB_CUR_MAX;
	mbcodeset = (mb_cur_max > 1);

	copyproto();
	initree();
	eargv = argv;
	tryfor = DEFMEM;
        nfields = 0;
        pos1flag = 0;
	kopt = 0;
	badopt = 0;
	uflg = 0;
	uflgactive = 0;
	maxrec = 0;
	outfil = (char *)NULL;
			/* Command parsing: follow POSIX guidelines; allow 
			 * widely-used POSIX "obsolescent" sort key options;
			 * and allow -y option with optional argument.
			 */
	do {
	  p = &fields[nfields];
			/* Four-stage processing of each potential option character:
			 * 1,2. If it is the first character of an obsolescent option
			 *    that does not follow getopt() conventions, process it
			 *    manually and update getopt() pointers to next possible
			 *    option character.
			 * 3. If it is -y and is either (the last command line
			 *    argument) or (followed in the next command line
			 *    argument by something that does not begin with a digit)
			 *    then (treat it as non-POSIX -y with omitted argument).
			 * 4. Otherwise process it through getopt() for normal 
			 *    option processing.
			 */
	  if(optind > argc)
		c = EOF;
	  else {
		arg = argv[optind];
		c = arg[0];
	  };
	  if ( c == '+') {	
		/* Part 1 of 4: Obsolescent +pos1 option */
	    newoptfield(arg+1,0,0,p);
	    optarg = argv[optind++];
	  } else if (c == '-' && strlen(arg) > 1
			 && (iswdigit((wchar_t)arg[1]) || arg[1]=='.' )) { 	
		/* Part 2 of 4: Obsolescent -pos2 option */
	    newoptfield(arg+1,1,0,p);
	    optarg = argv[optind++];
	  } else if (c == '-' && strlen(arg) == 2 && arg[1] == 'y'
			&& (optind == argc || !isdigit(argv[optind+1][0])) ) {
		/* Part 3 of 4: -y with optional Kilobytes argument omitted */
	    tryfor = MAXMEM;
	    optarg = argv[optind++];
	  } else {	
		/* Part 4 of 4: Normal POSIX command syntax option */
	    c = getopt(argc,argv,"bcdfik:mno:rt:uy:z:AT:");
	    switch(c){
	/* Operation modification options */
	    case 'c':
		cflg = 1;
		break;;
	    case 'm':
		mflg = 1;
		cmpset = 0;
		break;
	    case 'o':
		outfil = optarg;
		break;
	    case 't':
		if (mbcodeset) {
			convoptarg(optarg);
			if(wcslen(wcoptarg) == 1)
				tabchar = wcoptarg[0];
			else badopt++;
		} else {
			if (strlen(optarg) == 1)
				tabchar = optarg[0];
			else badopt++;
		}
		break;
	    case 'u':
		uflg = 1;
		break;
	    case 'y':
		/* -y with omitted argument handled separately above */
		/* Check for -y argument = valid integer */
		if (mbcodeset) {
			convoptarg(optarg);
			wcoptargend = wcoptarg;
			tryfor = (unsigned int) wcstol(wcoptarg,&wcoptargend,10);
			if ((tryfor == 0 && wcoptargend == wcoptarg )
				|| (wcoptargend == NULL) || (*wcoptargend != '\0') )
				badopt++;
			else
				tryfor *= 1024;
		} else {
			optargend=optarg;
			tryfor = (unsigned int) strtol(optarg,&optargend,10);
			if ((tryfor == 0 && optargend == optarg )
				|| (optargend == NULL) || (*optargend != '\0') )
				badopt++;
			else
				tryfor *= 1024;
		}
		/* Limit -y request to range [MINMEM,MAXMEM] , default=DEFMEM */
		tryfor = (tryfor<MINMEM ? MINMEM : 
				(tryfor > MAXMEM ? MAXMEM :
					(tryfor==0 ? DEFMEM : tryfor)));
		break;
	    case 'z':
		if (mbcodeset) {
			convoptarg(optarg);
			maxrec = (int) wcstol(wcoptarg,&wcoptargend,10);
			if (*wcoptargend != L'\0')
			  	badopt++;
		} else {
			maxrec = (int) strtol(optarg,&optargend,10);
			if (*optargend != '\0')
		  		badopt++;
		}
		break;
	    case 'A':
		Aflag = 1;
		setlocale(LC_ALL, "C");
		compare = Acmpa;
		break;
	    case 'T':
		if (optarg[0] != '\0') {
			if ((strlen(optarg) + NAMEOHD) > sizeof(file1)) {
				newdiag(MSGSTR(PATH2, "path name too long: %s\n")
					, optarg);
				exit(2);
			}
			else dirtry[0] = optarg;
		}
		break;
	/* Field modification options, must precede field spec option if preceded by '-' */
	    case 'b':
	    case 'd':
	    case 'f':
	    case 'i':
	    case 'n':
	    case 'r':
		/* Per POSIX 1003.2/D11(4.58.3,11630f): options -b,-d,-f,-i,-n, and -r
		 * must precede option -k although b,d,f,i,n, and r may appear within a
		 * -k option as a type in a keydef.  See (4.58.10,11855-11861).
		 */
		if (kopt) badopt++;
		else {
			switch(c) {
	  		case 'd':
				p->ignore = dict;
				break;
	  		case 'f':
				p->code = fold;
				break;
	  		case 'i':
				p->ignore = nonprint;
				break;
	  		case 'n':
				p->fcmp = NUM;
				break;
			case 'b':
				if (nfields==0) p->bflg[0]++;
				else p->bflg[1-pos1flag]++;
				break;
	  		case 'r':
				p->rflg = -1;
				break;
			default:
				break;
			}
		}
		break;
	/* Sort key options */
	    case 'k':
		kopt++;
		newoptfield(optarg,0,1,p);
		break;
	    case EOF:
		break;
	    default:
		badopt++;
		break;	
	    }; /*end switch(c)*/
	  }; /* end POSIX syntax option */
	  /* Update choice of comparison routine: if the option qualifies
	   * sort, use a sort routine that allows for field processing and
	   * processed sort keys prepended to the record. (All options
	   * except  -A, -T, -o, -u, -y, and  -z  qualify sort somehow.)
	   */
	  if ( (strchr("ATouyz",(int)c) == NULL ) && c!=EOF) {
		if(Aflag)
			compare = Acmp;
		else
			compare = cmp;
		if (!mflg)
			cmpset = 1;
	  }
	} /* end do c = ... */
	while (c != EOF);


	/* Set up input file names for pre-getopt() parameter processing: 
	 * eargc = number of input Files; eargv[0..eargc-1] = array of Filenames.
	 */
	eargv = &argv[optind];
	eargc = argc - optind;

	/* Check for -o option in the file operands (POSIX obsolescent requirement) */
	for (oargv=eargv,oargc=eargc; oargc>0; oargv++,oargc--) {
		if (strncmp(*oargv,"-o",2)==0) {
			/* Found option beginning with  -o,
			 * get -o argument as output file name.
			 */
			if (strlen(*oargv) == 2 && oargc > 1) {
				/* -o with Filename in following argv[] string */
				skipargs = 2;
				outfil = *(oargv+1);
			} else if (strlen(*oargv) > 2) {
				/* -oFilename  all in one argv[] string */
				skipargs = 1;
				outfil = (*oargv)+2;
			} else continue;/* No file specified, take  -o  as last input file. */
			/* Delete -o and argument from file names list */
			eargc -= skipargs;
			oargc -= skipargs;
			for (oargi=0;oargi<oargc;oargi++)
				oargv[oargi]=oargv[oargi+skipargs];
			oargv++;
		}
	}

	if (badopt) {
		Usage();
	}

#ifdef DEBUG_OPTS
/* Dump field structures to check option processing. */
fputs("Sort files: ",stdout);

for(Di=0;Di<eargc;Di++)
 printf(" eargv[%d]=%s  ",Di,eargv[Di]);
printf("  Outfile=%s\n",outfil);
printf("Sort flags: posflag=%d Aflag=%d cflg=%d mflg=%d uflg=%d cmpset=%d\n",
	posflag,Aflag,cflg,mflg,uflg,cmpset);
printf("Sort: compare = %s()\n",(compare==cmpa?"cmpa":
			    (compare==Acmpa?"Acmpa":
			     (compare==Acmp?"Acmp":
			      (compare==cmp?"cmp":"?")))));
fputs("Sort fields: code ign fcmp rflg  b[0] b[1]  m[0] m[1]  n[0] n[1]\n".stdout);

for(Di=0;Di<=nfields;Di++){
 p=&fields[Di];
 Dco=(p->code == fold?'f':(p->code == nofold?'n':'?'));
 Dig=(p->ignore == zero?'z':(p->ignore == dict?'d':(p->ignore == nonprint?'n':'?')));
 printf(" fields[%2d]= (%2c%4c%5d%5d%6d%5d%6d%5d%6d%5d )\n",
	Di,Dco,Dig,p->fcmp,p->rflg, p->bflg[0],p->bflg[1], p->m[0],p->m[1], p->n[0],p->n[1]);
}
#endif

	q = &fields[0];
	for(a=1; a<=nfields; a++) {
		p = &fields[a];
		if(p->code != proto.code) continue;
		if(p->ignore != proto.ignore) continue;
		if(p->fcmp != proto.fcmp) continue;
		if(p->rflg != proto.rflg) continue;
		if(p->bflg[0] != proto.bflg[0]) continue;
		if(p->bflg[1] != proto.bflg[1]) continue;
		p->code = q->code;
		p->ignore = q->ignore;
		p->fcmp = q->fcmp;
		p->rflg = q->rflg;
		p->bflg[0] = p->bflg[1] = q->bflg[0];
	}
	if(eargc == 0)
		eargv[eargc++] = "-";
	if(cflg && eargc>1) {
		diag(MSGSTR(CHECK2,"can check only 1 file\n"), "");
		exit(2);
	}

	safeoutfil();

	lspace = (int *)NULL;
	if (!mflg && !cflg) {
		if ( (alloc = grow_core(tryfor,0)) == 0 ) {
			diag(MSGSTR(ALLOC2,"allocation error before sort\n"), "");
			exit(2);
		}
	} else {
		if ( (alloc = grow_core(MAXMEM,0)) == 0 ) {
			diag(MSGSTR(MALLOC2,"allocation error before merge\n"), "");
			exit(2);
		}
	}

	a = -1;
	for(dirs=dirtry; *dirs; dirs++) {
		(void) sprintf(filep=file1, "%s/stm%.5uaa", *dirs, getpid());
		while (*filep)
			filep++;
		filep -= 2;
		if ( (a=creat(file, 0600)) >=0)
			break;
	}
	if(a < 0) {
		diag(MSGSTR(LOCATE2,"can't locate temp\n"), "");
		exit(2);
	}
	(void) close(a);
	(void) unlink(file);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP, (void (*)(int))term);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, (void (*)(int))term);
	(void) signal(SIGPIPE, (void (*)(int))term);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		(void) signal(SIGTERM, (void (*)(int))term);
	nfiles = eargc;
	if(!mflg && !cflg) {
		sort();
		if (ferror(stdin))
			rderror(NULL);
		(void) fclose(stdin);
	}

	if (maxrec == 0)  maxrec = SORT_LINE_MAX;
	oldalloc = alloc;
	alloc = (N + 1) * maxrec + N * BUFSIZ;
	for (nway = N; nway >= 2; --nway) {
		if (alloc < oldalloc)
			break;
		signedalloc = alloc - (maxrec + BUFSIZ);
		alloc = (signedalloc<0 ? 0 : (unsigned)signedalloc);
	}
	if (nway < 2 || alloc == 0) {
		diag(MSGSTR(MALLOC2,"allocation error before merge\n"), "");
		term();
	}

	if (cflg)   checksort();

	wasfirst = notfirst = 0;
	a = mflg || cflg ? 0 : eargc;
	if ((i = nfiles - a) > nway) {	/* Do leftovers early */
		if ((i %= (nway - 1)) == 0)
			i = nway - 1;
		if (i != 1)  {
			newfile();
			setbuf(os, bufout);
			merge(a, a+i);
			a += i;
		}
	}
	for(; a+nway<nfiles || unsafeout&&a<eargc; a=i) {
		i = a+nway;
		if(i>=nfiles)
			i = nfiles;
		newfile();
		setbuf(os, bufout);
		merge(a, i);
	}
	if(a != nfiles) {
		oldfile();
		setbuf(os, bufout);
		merge(a, nfiles);
	}
	error = exit_status;
	term();
}

/*
 * NAME: sort
 * FUNCTION: setup the buffers for the sort and call msort routine
 */
static void 
sort(void)
{
	register char *cp;
	register char **lp, **ep;
	char *keep, *ekeep, **mp, **lmp;
	char **oldep, **oldlp;
	int *oldlspace, oldlspace2lp, oldlspace2cp;
	int lspacemove;
	FILE *iop;
	int n;
	int done, i, first;
	int partial;
	char *f;

	/*
	** Records are read in from the front of the buffer area.
	** Pointers to the records are allocated from the back of the buffer.
	** If a partially read record exhausts the buffer, it is saved and
	** then copied to the start of the buffer for processing with the
	** next coreload.
	*/
	first = 1;
	done = 0;
	keep = NULL;
	ekeep = NULL;
	i = 0;
	ep = (char **) (((char *) lspace) + alloc);
	do {
		if ((f=setfil(i++)) == NULL) /* open first file */
			iop = stdin;
		else if ((iop=fopen(f,"r")) == NULL)
			cant(f);
	} while ((i < eargc) && (iop == NULL));
	if (iop==NULL)
		term();
	setbuf(iop,bufin);
	do {
		lp = ep - 1;
		cp = (char *) lspace;
		*lp-- = cp; /* move record from previous coreload */
		if (keep && ekeep)
			for(; keep < ekeep; *cp++ = *keep++);
		while ((char *)lp - cp > 1) {
			n = fgetrec(cp,(char *) lp - cp, iop, f, &partial);
			if (n == 0) {
				if (ferror(iop))
					rderror(f);

				if (keep != 0 )
					/* The kept record was at
					   the EOF.  Let the code
					   below handle it.       */;
				else
				if (i < eargc) {
					do {
						if ((f=setfil(i++)) == NULL)
							iop = stdin;
						else if ((iop=fopen(f,"r")) == NULL )
							cant(f);
					} while ((i < eargc) && (iop == NULL));
					if (iop==NULL) {
						done++;
						break;
					}
					setbuf(iop,bufin);
					continue;
				}
				else {
					done++;
					break;
				}
			}
			cp += n-1;
			if ( !partial ) {
				cp += 2;
				if ( cp - *(lp+1) > maxrec )
					maxrec = cp - *(lp+1);
				*lp-- = cp;
				keep = 0;
			}
			else 
			if ( cp + 2 < (char *) lp ) {
				/* Input record does not end with \n . Append '\n' and
				 * '\0' to end of input record. We do not attempt to force
				 * the end of the record to a character boundary.
				 */
				/* the last record of the input */
				/* file is missing a NEWLINE    */
				if(f == NULL) newdiag(MSGSTR(NEWLINE4,
				  "warning: missing NEWLINE added at EOF\n"), "");
				else newdiag(MSGSTR(NEWLINE5,
				  "warning: missing NEWLINE added at end of input file %s\n")
						, f);
				*++cp = '\n';
				*++cp = '\0';
				*lp-- = ++cp;
				keep = 0;
			}
			else {  /* the buffer is full */
				keep = *(lp+1);
				ekeep = ++cp;
			}
			if ((char *)lp - cp <= 2 && first == 1) {
				/* full buffer */
				tryfor = alloc;
				oldlspace = lspace;
				oldlspace2cp = (int)cp - (int)lspace;
				oldlspace2lp = (int)lp - (int)lspace;
				tryfor = grow_core(tryfor,alloc);
				if (tryfor == 0)
					/* could not grow */
					first = 0;
				else { /* move pointers */
					oldep = (char **)((int)lspace + alloc);
					oldlp = (char **)((int)lspace + oldlspace2lp);
					alloc += tryfor;
					lspacemove = (int)lspace - (int)oldlspace;
					cp = (char *)((int)lspace + oldlspace2cp);
					ep = (char **)((int)lspace + alloc);
					lp = (char **)((int)ep - (int)oldep + (int)oldlp);
					for ( mp = oldep-1, lmp = ep-1;
					      mp > oldlp; ) {
						*lmp-- = (char *)((int)(*mp--) + lspacemove);
					}
				}
			}
		}
		if (keep != 0 && *(lp+1) == (char *) lspace) {
			fprintf(stderr,MSGSTR(TOOLONG2,"fatal: record too large %d\n"),LINE);
			term();
		}
		first = 0;
		lp += 2;
		if(done == 0 || nfiles != eargc)
			newfile();
		else
			oldfile();
		setbuf(os, bufout);
		msort(lp, ep);
		if (ferror(os))
			wterror(sorting);
		(void) fclose(os);
	} while(done == 0);
}


/*
 * NAME: msort
 * FUNCTION: setup the merge sort and call qksort to do the actual sorting
 */
static void 
msort(char **a, char **b)
{
	register struct btree **tp;
	register int i, j, n;
	char *save;

	i = (b - a);
	if (i < 1)
		return;
	else if (i == 1) {
		wline(*a);
		return;
	}
	else if (i >= TREEZ)
		n = TREEZ; /* number of blocks of records */
	else n = i;

	/* break into n sorted subgroups of approximately equal size */
	tp = &(treep[0]);
	j = 0;
	do {
		(*tp++)->rn = j;
		b = a + (blkcnt[j] = i / n);
		qksort(a, b);
		blkcur[j] = a = b;
		i -= blkcnt[j++];
	} while (--n > 0);
	n = j;

	/* make a sorted binary tree using the first record in each group */
	for (i = 0; i < n;) {
		(*--tp)->rp = *(--blkcur[--j]);
		insert(tp, ++i);
	}
	wasfirst = notfirst = 0;
	bonus = cmpsave(n);


	j = uflg;

	/* If -u option specified, suppress character-code comparison of records
	 * with equal collation values based on sort options.
	 */
	if (uflg) uflgactive++;

	tp = &(treep[0]);
	while (n > 0)  {
		wline((*tp)->rp);
		if (j) save = (*tp)->rp;

		/* Get another record and insert.  Bypass repeats if uflg */

		do {
			/* Find group the record came from.*/
			i = (*tp)->rn;
			/* if (the group is not empty)
			 * {pop next record out of group, insert it into tree,
			 *  then leave tp at next collated record in the tree.}
			 */
			if (--blkcnt[i] > 0) {
				(*tp)->rp = *(--blkcur[i]);
				insert(tp, n);
			/* else {move to next collated record in the tree} */
			} else {
				if (--n <= 0) break;
				bonus = cmpsave(n);
				tp++;
			}
		} while (j && (*compare)((*tp)->rp, save) == 0);
	}
}


/* Insert the element at tp[0] into its proper place in the array of size n */
/* Pretty much Algorith B from 6.2.1 of Knuth, Sorting and Searching */
/* Special case for data that appears to be in correct order */

static void
insert(struct btree **tp, int n)
{
    register struct btree **lop, **hip, **midp;
    register int c;
    struct btree *hold;

    midp = lop = tp;
    hip = lop++ + (n - 1);
    if ((wasfirst > notfirst) && (n > 2) &&
	((*compare)((*tp)->rp, (*lop)->rp) >= 0)) {
	wasfirst += bonus;
	return;
    }
    while ((c = hip - lop) >= 0) { /* leave midp at the one tp is in front of */
	midp = lop + c / 2;
	if ((c = (*compare)((*tp)->rp, (*midp)->rp)) == 0) 
		if (Aflag) break; /* match */
	if (c <= 0) lop = ++midp;   /* c < 0 => tp > midp */
	else       hip = midp - 1; /* c > 0 => tp < midp */
    }
    c = midp - tp;
    if (--c > 0) { /* number of moves to get tp just before midp */
	hip = tp;
	lop = hip++;
	hold = *lop;
	memcpy(lop, hip, (c)*sizeof(*lop));
	lop += c;
	*lop = hold;
	notfirst++;
    } else wasfirst += bonus;
}


/*
 * NAME: merge
 * FUNCTION: merge sorted files together
 */
static void
merge(int a, int b)
{
	FILE *tfile[N];
	char *buffer = (char *) lspace;
	char	*save;
	char *iobuf;
	register int nf;		/* number of merge files */
	register struct btree **tp;
	register int i, j;
	char	*f;

	save = (char *) lspace + (nway * maxrec);
	iobuf = save + maxrec;
	tp = &(treep[0]);
	for (nf=0, i=a; i < b; i++)  {
		f = setfil(i);
		if (f == 0)
			tfile[nf] = stdin;
		else if ((tfile[nf] = fopen(f, "r")) == NULL) {
			cant(f);
			continue;
		}
		(*tp)->rp = buffer + (nf * maxrec);
		(*tp)->rn = nf;
		setbuf(tfile[nf], iobuf);
		iobuf += BUFSIZ;
		if (rline(tfile[nf], (*tp)->rp)==0) {
			nf++;
			tp++;
		} else {
			if(ferror(tfile[nf]))
				rderror(f);
			(void) fclose(tfile[nf]);
		}
	}


	/* make a sorted btree from the first record of each file */
	for (--tp, i = 1; i++ < nf;) insert(--tp, i);

	bonus = cmpsave(nf);
	tp = &(treep[0]);
	j = uflg;

	/* If -u option specified, suppress character-code comparison of
	 * records with equal collation values based on sort options.
	 */
	if (uflg) uflgactive++;

	while (nf > 0) {
		wline((*tp)->rp);
		if (j) cline(save, (*tp)->rp);

		/* Get another record and insert.  Bypass repeats if uflg */

		do {
			i = (*tp)->rn;
			if (rline(tfile[i], (*tp)->rp)) {
				if (ferror(tfile[i]))
					rderror(setfil(i+a));
				(void) fclose(tfile[i]);
				if (--nf <= 0) break;
				++tp;
				bonus = cmpsave(nf);
			} else insert(tp, nf);
		} while (j && (*compare)((*tp)->rp, save) == 0 );
	}


	for (i=a; i < b; i++) {
		if (i >= eargc)
			(void) unlink(setfil(i));
	}
	if (ferror(os))
		wterror(merging);
	(void) fclose(os);
}

/*
 * NAME: cline
 * FUNCTION: copy line
 */
static void
cline(register char *tp, register char *fp)
{
	while ((*tp++ = *fp++) != '\n');
}

/*
 * NAME: rline
 * FUNCTION: read line
 *           Because the lines may be read form temporary work files,
 *           we must check the state of the sort. For merge and cksort
 *           operations, (and plain sorts), we read as usual. For keyed
 *	     sorts, the key area (which may contain zero bytes) is first
 *	     read, then the remainder (text portion) of the record.
 */
static int
rline(register FILE *iop, register char *s)
{
	register int n;
	int rlen, maxlen;

	maxlen = maxrec ;
	/* If line is not being read as part of a (merge by sort -m) or
	 * (check by sort -c) then line must be being read as part of
	 * (merge intermediate output files that cumulatively exceed
	 * MAXMEM). If (cmpset && !Aflag), wline() wrote prefixed keys,
	 * so read those before line content.
	 */
	if (!mflg && !cflg && (cmpset == 1) && !Aflag) {
		maxlen = maxrec-2;
		fread(s, 1, 2, iop);
		rlen = ((s[0] << 8) | (s[1]));
		rlen -= 2;
		if (rlen > maxlen)
			rlen = maxlen;
		s +=2;
		if (fread(s, 1, rlen, iop) < rlen)
			return(1);
		maxlen -= rlen;
		s += rlen;
	}
	if (fgets(s,maxlen,iop) == NULL )
		n = 0;
	else
		n = strlen(s);
	if ( n == 0 )
		return(1);
	s += n - 1;
	if ( *s == '\n' )
		return(0);
	if ( n < maxlen) {
		newdiag(MSGSTR(NEWLINE4,"warning: missing NEWLINE at EOF added\n"),"");
		*++s = '\n';
		return(0);
	}
	else {
		fprintf(stderr,MSGSTR(TOOLONG2,"fatal: line too long %d\n"),LINE);
		term();
	}
	return(0);
}

/*
 * NAME: wline
 * FUNCTION: write line
 */
static void
wline(char *s)
{
	size_t rlen;

	/* If (cmpset and !Aflag), line has prefixed keys. If line is
	 * not being written to user output file then line must be being
	 * written to intermediate output file for later merge because
	 * cumulative output size exceeds MAXMEM, so write those before
	 * line content.
	 */
	if (cmpset == 1 && !Aflag) {
		rlen = ((s[0] << 8) | (s[1]));
		if (!outflag) {
			/* write key information to temporary file */
			fwrite(s, 1, rlen, os);
		}
		/* strip key information */
		s += rlen;

	}
	(void) fputs(s,os);
}

/*
 * NAME: checksort
 * FUCNTION: has file already been sorted.
 */
static void
checksort(void)
{
	char *lines[2];
	register char **s;
	char *f;
	register int i, j, r;
	register FILE *iop;

	s = &(lines[0]);
	f = setfil(0);
	if (f == 0)
		iop = stdin;
	else if ((iop = fopen(f, "r")) == NULL) {
		cant(f);
		term();
	}
	setbuf(iop, bufin);

	i = 0;   j = 1;
	s[0] = (char *) lspace;
	s[1] = s[0] + maxrec;
	if ( rline(iop, s[0]) ) {
		if (ferror(iop))
			rderror(f);
		(void) fclose(iop);
		exit(exit_status);
	}
	while ( !rline(iop, s[j]) )  {
		r = (*compare)(s[i], s[j]);
		if (r < 0)
			disorder(MSGSTR(DISORDER2,"disorder: %s\n"), s[j]);
		if (r == 0 && uflg)
			disorder(MSGSTR(NUNIQUE2,"not unique: %s\n"), s[j]);
		r = i;  i = j; j = r;
	}
	if (ferror(iop))
		rderror(f);
	(void) fclose(iop);
	exit(exit_status);
}

/*
 * NAME: disorder
 * FUNCTION: added NULL character to the end of the string t and print error message
 */
static void
disorder(char *s, char *t)
{
	register char *u;
	for(u=t; *u!='\n';u++) ;
	*u = 0;
	newdiag(s, t);
	error = 1;
	term();
}

/*
 * NAME: newfile
 * FUNCTION: open file for writting
 */
static void
newfile(void)
{
	register char *f;

	f = setfil(nfiles);
	if((os=fopen(f, "w")) == NULL) {
		newdiag(MSGSTR(CREATE2,"can't create %s\n"), f);
		term();
	}
	nfiles++;
	outflag = 0;
}

/*
 * NAME: setfil
 * FUNCTION: set up unique temp file name
 */
static char *
setfil(register int i)
{
	if(i < eargc)
		if(eargv[i][0] == '-' && eargv[i][1] == '\0')
			return(0);
		else
			return(eargv[i]);
	i -= eargc;
	filep[0] = i/26 + 'a';
	filep[1] = i%26 + 'a';
	return(file);
}

/*
 * NAME: oldfile
 * FUNCTION: open output file or set os to stdout
 */
static void
oldfile(void)
{
	if(outfil) {
		if((os=fopen(outfil, "w")) == NULL) {
			newdiag(MSGSTR(CREATE2,"can't create %s\n"), outfil);
			term();
		}
	} else
		os = stdout;
	outflag = 1;	/* set to mark output file... */
}

/*
 * NAME: safeoutfil
 * FUNCTION: check the output file is it safe to use as an output file
 */
static void
safeoutfil(void)
{
	register int i;
	struct stat ostat, istat;

	if(!mflg||outfil==0)
		return;
	if(stat(outfil, &ostat)==-1)
		return;
	if ((i = eargc - N) < 0) i = 0;	/*-N is suff., not nec. */
	for (; i < eargc; i++) {
		if(stat(eargv[i], &istat)==-1)
			continue;
		if(ostat.st_dev==istat.st_dev&&
		   ostat.st_ino==istat.st_ino)
			unsafeout++;
	}
	return;
}

/*
 * NAME: cant
 * FUNCTION: print error message when unable to open a file
 */
static void
cant(char *f)
{
	newdiag(MSGSTR(OPEN2,"can't open %s\n"), f);
	exit_status = 2;
}

/*
 * NAME:
 * FUNCTION: print error message
 */
static void
diag(char *s, char *t)
{
	register FILE *iop;

	iop = stderr;
	(void) fputs("sort: ", iop);
	(void) fputs(s, iop);
	(void) fputs(t, iop);
	(void) fputs("\n", iop);
}

/*
 * NAME: newdiag()
 * FUNCTION: Internationalizable version of diag(): print error message.
 */
static void
newdiag(char *s, char *t)
{
	fputs("sort: ",stderr);

	if (t != NULL && *t != '\0' )
		fprintf(stderr,s,t);
	else
		fputs(s,stderr);
}

/*
 * NAME: term
 * FUNCTION: clean up and exit 
 */
static void
term(void)
{
	register i;

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGTERM, SIG_IGN);
	if(nfiles == eargc)
		nfiles++;
	for(i=eargc; i<=nfiles; i++) {	/*<= in case of interrupt*/
		(void) unlink(setfil(i));	/*with nfiles not updated*/
	}
	exit(error);
}

/*
 * NAME: cmp
 * FUNCTION: compare two strings
 * 		This routine is substantially expanded and changed to be
 *		able to handle the prefixed form of the data. If the compare
 *		code is either XNUM or XSTR, then the field is prepended to
 *		the record; otherwise, the old method for finding the key
 *		and compare it holds.
 *		Note that, for string compares, if the caller specified either
 *		folding or dictionary sort, then the fields are moved (and
 *		either compacted or folded) to work areas, where they are
 *		compared. If neither folding or dictionary order is speci-
 *		fied, the compare takes place in situ.
 */
static int
cmp(char *a, char *b)
{
	register char *pa, *pb;
	char *la, *lb;
	char sava, savb;
	char *ipa, *ipb, *jpa, *jpb;
	char *code, *ignore;
	register int sa;
	int sb;
	int i, j;
	int k;
	struct field *fp;
	wchar_t wcchar, *pwcchar=&wcchar;
	int chrlen;

	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		pa = a;
		pb = b;
		if ( (fp->fcmp == ASC) || (fp->fcmp == NUM) ) {
			if(k >= 0) {	/* keys are in original place */
				la = skip(pa, fp, 1);
				pa = skip(pa, fp, 0);
				lb = skip(pb, fp, 1);
				pb = skip(pb, fp, 0);
			} else {
				la = eol(pa);
				lb = eol(pb);
			}
		} else  {		/* keys are prepended to record */
			pa += 2;
			pb += 2;
			for (i = 0; i < k; i++) {
				pa += ((pa[0] << 8) | (pa[1]));
				pb += ((pb[0] << 8) | (pb[1]));
			}
			la = pa + ((pa[0] << 8) | (pa[1]));
			lb = pb + ((pb[0] << 8) | (pb[1]));
			pa += 2;
			pb += 2;
		}
		if( (fp->fcmp == NUM) || (fp->fcmp == XNUM) ) {
			sa = sb = fp->rflg;

			/* Skip leading <blank>s in both records.
			 * (POSIX Draft 11 decouples  -n  from implied  -b . )
			 */
			if (mbcodeset) {
				while( *pa != '\n' && (chrlen=mbtowc(pwcchar,pa,mb_cur_max))>0
					&& iswblank(wcchar) )
					pa += chrlen;
				while( *pb != '\n' && (chrlen=mbtowc(pwcchar,pb,mb_cur_max))>0
					&& iswblank(wcchar) )
					pb += chrlen;
			} else {
                		while((*pa != '\n') && isblank(*pa))
                       		 	pa++;
                		while((*pb != '\n') && isblank(*pb))
                       		 	pb++;
                	}

			if(*pa == '-') {
				pa++;
				sa = -sa;
			}
			if(*pb == '-') {
				pb++;
				sb = -sb;
			}
				/* Find the radix position: first byte position to the
				 * right of the rightmost digit or thousands-separator
				 * from the left end of the field.
				 */
			for (ipa = pa; ipa<la; ipa++) {
				if ( !(iswdigit((wchar_t)*ipa)
					||(wchar_t)*ipa==thsep||(wchar_t)*ipa==thsepmon) )
					break;
			}
			for (ipb = pb; ipb<lb; ipb++) {
				if ( !(iswdigit((wchar_t)*ipb)
					||(wchar_t)*ipb==thsep||(wchar_t)*ipb==thsepmon) )
					break;
			}
			jpa = ipa;
			jpb = ipb;
			i = 0;
			if(sa==sb)
				/* If signs are the same, find the most significant
				 * digit position to left of the radix position in which
				 * the numbers differ. Skip thousands separators.
				 * There is no attempt here to check LC_NUMERIC grouping
				 * or LC_MONETARY mon_grouping sizes, or to infer
				 * zero-digits in omitted character positions between
				 * separators according to grouping or mon_grouping.
				 */
				while(ipa > pa && ipb > pb) {
					ipa--;
					ipb--;
					while(ipa>pa && 
						((wchar_t)*ipa==thsep || (wchar_t)*ipa==thsepmon)){
						ipa--;
					}
					while(ipb>pb && 
						((wchar_t)*ipb==thsep || (wchar_t)*ipb==thsepmon)){
						ipb--;
					}
					if(j = *ipb - *ipa)
						i = j;
				}
			/* If either number contains more significant digits than the other,
			 * or if the signs are different, the comparison is determined by
			 * the presence of a nonzero significant digit in either number
			 * and the sign of that number.
			 */
			while(ipa > pa)
				if(*--ipa != '0'
					&& (unsigned char)*ipa!=thsep 
						&& (unsigned char)*ipa!=thsepmon)
					return(-sa);
			while(ipb > pb)
				if(*--ipb != '0'
					&& (unsigned char)*ipb!=thsep 
						&& (unsigned char)*ipb!=thsepmon)
					return(sb);
			if(i) return(i*sa);
				/* No discriminating corresponding character position was
				 * found to the left of the radix position. Now sort on the
				 * leftmost character position in which corresponding
				 * positions to the right of the radix position differ.
				 */
			if((wchar_t)*(pa=jpa) == dec || (wchar_t)*(pa) == decmon)
				pa++;
			if((wchar_t)*(pb=jpb) == dec || (wchar_t)*(pb) == decmon)
				pb++;
			if(sa==sb)
				while(pa<la && iswdigit((wchar_t)*pa)
				   && pb<lb && iswdigit((wchar_t)*pb))
					if(i = *pb++ - *pa++)
						return(i*sa);
			while(pa<la && iswdigit((wchar_t)*pa))
				if(*pa++ != '0')
					return(-sa);
			while(pb<lb && iswdigit((wchar_t)*pb))
				if(*pb++ != '0')
					return(sb);
			continue;
		}
		if (fp->fcmp == XSTR) {		/* 'key' is prepended */
			sa = strcmp(pb, pa);
			if (sa == 0)
				continue;		
			return(sa*fp->rflg);
		}
		if (fp->fcmp == ASC) {		/* record is not prepended */
			code = fp->code;
			ignore = fp->ignore;
			if ( (ignore == zero) && (code == nofold) ) {
				sava = *la;
				*la = '\0';
				savb = *lb;
				*lb = '\0';
				sa = strcoll(pb, pa);
				*la = sava;
				*lb = savb;
			} else {		/* have to use work areas */
				ipa = tbuf;
				fldtowa(pa, la, ipa, ignore, code);
				ipb = ebuf;
				fldtowa(pb, lb, ipb, ignore, code);
				sa = strcoll(ipb, ipa);
			}
			if ( sa == 0 )
				continue;
			return(sa*fp->rflg);
		}
	}
	if(uflgactive)
		return(0);

       /* If no return yet: the records collate equally on all qualifications
	* specified by  sort  options. To make sort results independent of the
	* order of sort records, now sort on character codes in the specified
	* sort fields (if any) and then character codes of the entire record.
	* uflgactive  suppresses this final refining sort while suppressing
	* multiple records with equal sort values for  -u  option.
	*/
	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		if ( (fp->fcmp == XNUM) || (fp->fcmp == XSTR) ) {
			pa = a;
			pb = b;
			pa += ((pa[0] << 8) | (pa[1]));
			pb += ((pb[0] << 8) | (pb[1]));
			return(cmpa(pa, pb));
		}
	}
	return(cmpa(a, b));
}

/*   this routine was recoded in 370 assembler as part of the peachtree
 *   activities.                HJS-S
 *
 * NAME: cmpa
 * FUNCTION: compare two strings
 *			This routine is modified to use strcoll.
 */
static int
cmpa(register char *pa, register char *pb)
{
	register int alen, blen; 
 	register int r,i,j;
	wchar_t wcpa[BUFSIZ],*pwcpa = &wcpa[0],wcpb[BUFSIZ],*pwcpb = &wcpb[0];
	int wcpalen,wcpblen;

	alen = eol(pa) - pa;
	blen = eol(pb) - pb;
	pa[alen] = '\0';
	pb[blen] = '\0';
	r = strcoll(pb, pa) * fields[0].rflg;

		/* Strings that have different collation elements with the same
		 * collation values in corresponding positions will effectively
		 * collate on their relative positions in the source file. To make
		 * such strings collate consistently independent of their positions
		 * in the file, compare their characters' wchar_t values. (P37708)
		 */
 	if (r == 0) {
 	    /* Compare characters' wide char values */
	    wcpalen = (int)mbstowcs(pwcpa,pa,alen+1);
	    wcpblen = (int)mbstowcs(pwcpb,pb,blen+1);
	    i = (wcpalen<wcpblen?wcpalen:wcpblen);
 	    j=0;
	    do {
 		r = (int)(wcpb[j] - wcpa[j]);
 		j++;
 	    } while (--i > 0 && r == 0) ;
 	}

	pa[alen] = '\n';
	pb[blen] = '\n';
	return (r);
}

/*
 * NAME: Acmp
 * FUNCTION: compare two strings
 *			This is the oldfashioned AT&T code; assuming the
 *			native collating sequence. It is activated using
 *			the -A flag.
 */
static int
Acmp(char *i, char *j)
{
	register char *pa, *pb;
	register char *ignore;
	char *code;
	char *la, *lb;
	char *ipa, *ipb, *jpa, *jpb;
	register int sa;
	int sb;
	int a, b;
	int k;
	struct field *fp;
	wchar_t wcchar, *pwcchar=&wcchar;
	int chrlen;

	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		pa = i;
		pb = j;
		if(k >= 0) {
			la = skip(pa, fp, 1);
			pa = skip(pa, fp, 0);
			/* showme(pa,la); */	/* showme() routine for debugging */
			lb = skip(pb, fp, 1);
			pb = skip(pb, fp, 0);
			/* showme(pb,lb); */	/* showme() routine for debugging */
		} else {
			la = eol(pa);
			lb = eol(pb);
		}
		if(fp->fcmp==NUM) {
			sa = sb = fp->rflg;

			/* Skip leading <blank>s in both records.
			 * (POSIX Draft 11 decouples  -n  from implied  -b .)
			 */
			if (mbcodeset) {
				while( *pa != '\n' && (chrlen=mbtowc(pwcchar,pa,mb_cur_max))>0
					&& iswblank(wcchar) )
					pa += chrlen;
				while( *pb != '\n' && (chrlen=mbtowc(pwcchar,pb,mb_cur_max))>0
					&& iswblank(wcchar) )
					pb += chrlen;
			} else {
               		 	while((*pa != '\n') && isblank(*pa))
               		         	pa++;
               		 	while((*pb != '\n') && isblank(*pb))
               		         	pb++;
               		}

			if(*pa == '-') {
				pa++;
				sa = -sa;
			}
			if(*pb == '-') {
				pb++;
				sb = -sb;
			}
				/* See explanation of algorithm in cmp() above.*/
			for(ipa = pa; ipa<la&& (isdigit(*ipa)||*ipa==thsep||*ipa==thsepmon); ipa++);
			for(ipb = pb; ipb<lb&& (isdigit(*ipb)||*ipb==thsep||*ipb==thsepmon); ipb++);
			jpa = ipa;
			jpb = ipb;
			a = 0;
			if(sa==sb)
				while(ipa > pa && ipb > pb) {
					ipa--;
					ipb--;
					while(ipa>pa && (*ipa==thsep || *ipa==thsepmon))
						ipa--;
					while(ipb>pb && (*ipb==thsep || *ipb==thsepmon))
						ipb--;
					if(b = *ipb - *ipa)
						a = b;
				}
			while(ipa > pa)
				if(*--ipa != '0')
					return(-sa);
			while(ipb > pb)
				if(*--ipb != '0')
					return(sb);
			if(a) return(a*sa);
			if(*(pa=jpa) == dec || *(pa=jpa) == decmon)
				pa++;
			if(*(pb=jpb) == dec || *(pb=jpb) == decmon)
				pb++;
			if(sa==sb)
				while(pa<la && isdigit(*pa)
				   && pb<lb && isdigit(*pb))
					if(a = *pb++ - *pa++)
						return(a*sa);
			while(pa<la && isdigit(*pa))
				if(*pa++ != '0')
					return(-sa);
			while(pb<lb && isdigit(*pb))
				if(*pb++ != '0')
					return(sb);
			continue;
		}
		code = fp->code;
		ignore = fp->ignore;
loop: 
		while(ignore[*pa])
			pa++;
		while(ignore[*pb])
			pb++;
		if(pa>=la || *pa=='\n')
			if(pb<lb && *pb!='\n')
				return(fp->rflg);
			else continue;
		if(pb>=lb || *pb=='\n')
			return(-fp->rflg);

		sa = code[*pb++] - code[*pa++];

		if(sa == 0)
			goto loop;
		return(sa*fp->rflg);
	}
	if(uflgactive)
		return(0);

       /* If no return yet: the records collate equally on all qualifications
	* specified by  sort  options. To make sort results independent of the
	* order of sort records, now sort on characters of the entire record.
	* uflgactive  suppresses this final refining sort while suppressing
	* multiple records with equal sort values for  -u  option.
	*/
	return(Acmpa(i, j));
}
/*
 * NAME: Acmpa
 * FUNCTION: compare two strings
 *			This is the oldfashioned AT&T code, assuming that
 *			collation is according to the native code order.
 *			It is activated using the -A flag.
 */
static int
Acmpa(register char *pa, register char *pb)
{

	while(*pa == *pb++)
		if(*pa++ == '\n')
			return(0);
	return(
		*pa == '\n' ? fields[0].rflg:
		*--pb == '\n' ?-fields[0].rflg:
		*pb > *pa   ? fields[0].rflg:
		-fields[0].rflg
	);
}

/*
 * NAME: skip
 * FUNCTION: skip a field
 */

static char *
skip(register char *p, struct field *fp, int j)
{
	register i;
	register wchar_t tbc;
	wchar_t wcchar, *pwcchar=&wcchar;
	int chrlen=1;

	if( (i=fp->m[j]) < 0)
		return((char *)eol(p));
	/* Skip characters to the beginning of the next field.
	 * The next field begins at the first character following the next field separator,
	 * where a field separator is
	 *    if (tabchar specified by  -t tabchar  option)
	 *    then {next occurrence of tabchar at or following initial p*}
	 *    else {next occurrence of one or more consecutive <blank>s following
	 *	    a non<blank>.}
	 * The sorted part of the next field begins at
	 *    if (-b option or b field modifier applies to the field)
	 *    then {first non-<blank> character at or past beginning of field}
	 *    else {beginning of the field} .
	 */
	if (mbcodeset) {	/* skip in multibyte code set */
	  if (tbc = tabchar) {	/* skip past next tabchar */
		while (--i >= 0) {
			while((chrlen=mbtowc(pwcchar,p,mb_cur_max))>0 && wcchar!=tbc)
				if(*p != '\n')
					p += chrlen;
				else    return(p);
			if (i >= 0) {
				chrlen=mbtowc(pwcchar,p,mb_cur_max);
				p += (chrlen<1?1:chrlen);
			}
		}
	  } else {		/* skip past end of non-<blank> string following
				 * next <blank> string
				 */
	    	while (--i >= 0) {
			while((chrlen=mbtowc(pwcchar,p,mb_cur_max))>0 && iswblank(wcchar))
				p += chrlen;
			while(*p != '\n' 
			      && (chrlen=mbtowc(pwcchar,p,mb_cur_max))>0 && !iswblank(wcchar))
				p += chrlen;
			if (*p == '\n')
				return(p);

		}
	  }
	} else { 		/* skip in single byte code set */
	  if (tbc = tabchar) {	/* skip past next tabchar */
		while (--i >= 0) {
			while(*p != tbc)
				if(*p != '\n')
					p++;
				else    return(p);
			if (i >= 0)
				p++;
		}
	  } else {		/* skip past end of non-<blank> string following
				 * next <blank> string
				 */
	    	while (--i >= 0) {
			while(isblank(*p))
				p++;
			while(*p != '\n' && !isblank(*p))
				p++;
                        if (*p == '\n')
                        	return(p);
		}
	  }
	}

	/* This is actually the last character in the field fp->m[j]-1 */
	if ((j==1) && (fp->n[j]==0)) {
		if (tabchar)
			return (mbcodeset?p-chrlen:p-1);
		else
			return (p);
	}

	/* Skip leading blanks in field if -b option or b modifier applies.*/
	/* Note that -b only skips <blank>s. It does not skip non-<blank>
	 * field separator characters if specified by  -t  option.
	 */
	if(fp->bflg[j]) {
		if (mbcodeset) {	/* skip in multibyte code set */
			if (fp->m[j] > 0 && !tabchar) {
				p += mblen(p,mb_cur_max);
			}
			while((chrlen=mbtowc(pwcchar,p,mb_cur_max))>0 && iswblank(wcchar))
				p += chrlen;
		} else {		/* skip in single byte code set */
			if (fp->m[j] > 0 && !tabchar)
				p++;
			while(isblank(*p))
				p++;
		}
	}

 	/* Skip to character position specified by  n  of  +m.n  or  y  of  -kx.y */
	i = fp->n[j];
	while((i-- > 0) && (*p != '\n'))
		if ( (chrlen=mbtowc(pwcchar,p,mb_cur_max)) >0 )
			p += chrlen;
		else p++;


	return(p);
}

/*
 * NAME: eol
 * FUNCTION: find the end of the line
 */
static char *
eol(register char *p)
{
	if ((p = (char *)strchr(p, '\n')) == NULL) {
		newdiag(MSGSTR(NULLS2,
			"cannot process data file (check for null chars)\n"),"");
		exit(2);
	}

	return((char *)p);
}

/*
 * NAME: copyproto
 * FUNCTION: copy the prototype for the sort fields
 */
static void
copyproto(void)
{
	register i;
	register int *p, *q;

	p = (int *)&proto;
	q = (int *)&fields[nfields];
	for(i=0; i<sizeof(proto)/sizeof(*p); i++)
		*q++ = *p++;
}

/*
 * NAME: initree
 * FUNCTION: initialize the binary search tree
 */
static void 
initree(void)
{
	register struct btree **tpp, *tp;
	register int i;

	for (tp = &(tree[0]), tpp = &(treep[0]), i = TREEZ; --i >= 0;)
	    *tpp++ = tp++;
}

static int 
cmpsave(register int n)
{
	register int award;

	if (n < 2) return (0);
	for (n++, award = 0; (n >>= 1) > 0; award++);
	return (award);
}

/*
 * NAME: newoptfield
 * FUNCTION: Process field specification option string.
 */ 
static void
newoptfield(char *s,int spos,int ink, struct field *p)
{
  /* Entry: ink == 1 => s points to start of "-k" option value string
   *        ink == 0 && spos == 0 => s points to start of "+" pos1 string
   *        ink == 0 && spos == 1 => s points to start of "-" pos2 string
   *        nfields = number of prior field spec options
   *	    pos1flag = 0 if no previous call to newoptfield(), or if previous call
   *			to newoptfield() was for "-m.n" or "-k...,m.n"; 
   *		     = 1 if previous call to newoptfield() was for "+m.n" or "-km.n".
   * Only single-byte portable character set characters are accepted as option
   * values for +Pos1, -Pos2, and -kOptlist options.
   */
  wchar_t *sc;
  int rpart;	/* 0 if processing "+m.n" part,  1 if processing "-m.n" part (obsolescent)*/
		/* 0 if processing "-km.n" part, 1 if processing ",m.n" part (POSIX -k)*/
  int state;	/* 0 => no part of "m.n" processed */
  		/* 1 => "m" of "m.n" has been processed */
		/* 2 => "." of "m.n" has been processed */
  int numval;

  if ( ink || !spos || !pos1flag ) {   /* Either starting "-k", or starting "+m.n", or starting
					* "-m.n" for which there was no corresponding "+m.n" .
					*/
	if(++nfields>=NF) {
		diag(MSGSTR(KEYS2,"too many keys\n"), "");
		exit(2);
	}
	if(!spos || (ink && *s!=','))
		posflag++;
	copyproto();
  }
  p = &fields[nfields];
  state = 0;
  rpart = (!ink && spos);

  convoptarg(s);

  for (sc=wcoptarg;*sc!=L'\0';sc++)
  {
	switch(*sc){
	case L'.':
		if (state++ == 0) { /* Omitted "m" of "m.n" defaults to 0 */
		  p->m[rpart] = 0;
		  state++;
		}
		if (state > 2) { /* Too many "." */
		  Usage();
		}
		break;
	case L'-':
		if (ink || !rpart++ ) {
		  Usage();
		}
		else
		  state = 0;
		break;
	case L',':	/* found -k...,m.n */
		if (!ink || rpart++ ) {
		  Usage();
		}
		else
		  state = 0;
		break;
	case L'b':
		p->bflg[rpart]++;
		break;
	case L'd':
		p->ignore = dict;
		break;
	case L'f':
		p->code = fold;
		break;
	case L'i':
		p->ignore = nonprint;
		break;
	case L'n':
		p->fcmp = NUM;
		break;
	case L'r':
		p->rflg = -1;
		break;
	default:
		if(iswdigit(*sc)) {
		  numval = (int) wcstol(sc,&sc,10);
		  if((ink?numval-(state==2?0:1):numval)<0) { 
			Usage();
		  }
		  if(ink) {    /* Convert -k measurements to +pos1 -pos2 measurements.
				* Per POSIX 1003.2/D11(4.58.7,11785-11789),
				* -ka.b,c.d  =	if d==0 then +(a-1).(b-1) -c.d
				*			else +(a-1).(b-1) -(c-1).d
				*/
		    if(rpart)
		      if(state && numval>0)
		        p->m[rpart]--;
		      else;
		    else
		      numval--;
		  };
		  sc--;
		  if (!state++){
			p->m[rpart] = numval;
		  } else {
			p->n[rpart] = numval;
		  }
		} else {
			 Usage();
		}
		break;
	} /* switch(*sc) */
  } /* for (sc=... */
  pos1flag = !rpart;
}

/*
 * NAME: convoptarg
 * FUNCTION: convert multibyte string optarg to wchar_t string wcoptarg 
 */
static void
convoptarg(char *s)
{
  /* Globals: static wchar_t *wcoptarg, extern char *optarg */
  int n,rval;
	/* Entry conditions:
	 * 1. MB_CUR_MAX > 1 (in multibyte locale)
	 * 2. s points to argument for a -t, -y, or -z option
	 * Exit conditions:
	 * 1. IF s' pointed to a valid MBCS string
	 *    THEN wcoptarg points to wchar_t string for s'*
	 *    ELSE sort Usage message is written to standard error
	 *         and program terminates with nozero exit code
	 */
  n = strlen(s)+1;
  rval = mbstowcs(wcoptarg,s,n);
  if (rval == -1) {
	 Usage();
  }
}

 /*
 * NAME: Usage
 * FUNCTION: Display Usage message and exit >0
 */
static void
Usage(void)
{
  fprintf(stderr,MSGSTR(USAGE2,
"Usage:   sort\t[-Abcdfimnru] [-T Directory] [-t Character] [-o File]\n\
\t\t[-y[Kilobytes]] [-z Recordsize] [-k Keydefinition]...\n\
\t\t[[+Position1][-Position2]]... [File]...\n"));
  exit(2);
}

#define qsexc(p,q) t= *p;*p= *q;*q=t
#define qstexc(p,q,r) t= *p;*p= *r;*r= *q;*q=t

/*
 * NAME: qksort
 * FUNCTION: sort the binary tree
 */
static void 
qksort(char **a, char **l)
{
	register char **i, **j;
	register char **lp, **hp;
	char *t;
	int c, delta;
	unsigned n;


start:
	if((n=l-a) <= 1)
		return;

	n /= 2;
	if (n >= MTHRESH) {
		lp = a + n;
		i = lp - 1;
		j = lp + 1;
		delta = 0;
		c = (*compare)(*lp, *i);
		if (c < 0) --delta;
		else if (c > 0) ++delta;
		c = (*compare)(*lp, *j);
		if (c < 0) --delta;
		else if (c > 0) ++delta;
		if ((delta /= 2) && (c = (*compare)(*i, *j)))
		    if (c > 0) n -= delta;
		    else       n += delta;
	}
	hp = lp = a+n;
	i = a;
	j = l-1;


	for(;;) {
		if(i < lp) {
			if((c = (*compare)(*i, *lp)) == 0) {
				--lp;
				qsexc(i, lp);
				continue;
			}
			if(c < 0) {
				++i;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = (*compare)(*hp, *j)) == 0) {
				++hp;
				qsexc(hp, j);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					++hp;
					qstexc(i, hp, j);
					i = ++lp;
					goto loop;
				}
				qsexc(i, j);
				--j;
				++i;
				continue;
			}
			--j;
			goto loop;
		}


		if(i == lp) {
			if(lp-a >= l-hp) {
				qksort(hp+1, l);
				l = lp;
			} else {
				qksort(a, lp);
				a = hp+1;
			}
			goto start;
		}


		--lp;
		qstexc(j, lp, i);
		j = --hp;
	}
}

/*
 * NAME: rderror
 * FUNCTION: print read error
 */
static void 
rderror(char *s)
{
	newdiag(MSGSTR(EREAD2,"read error on %s\n"), s == NULL ? MSGSTR(STDIN2,"stdin") : s);
	term();
}

/*
 * NAME: wterror
 * FUNCTION: print write error
 */
static void
wterror(int x)
{
	if (x == sorting)
		newdiag(MSGSTR(EWRITE2,"Write error while sorting.\n"),"");
	else	newdiag(MSGSTR(EWRITE3,"Write error while merging.\n"),"");
	term();
}

/*
 * NAME: grow_core
 * FUNCTION:	Increase the size of temporary sorting area  lspace .
 * ENTRY:	1. IF lspace!=NULL
 *		   THEN lspace points to an available block of size cursize  
 * EXIT:	1. Return value = IF lspace points to an available block 
 *					of size  (size + cursize)
 *				  THEN difference between (size of new block
 *					 pointed to by lspace) and cursize'
 *				  ELSE 0 .
 *		2. 0 <= return value AND return value <= size'.
 *		3. ANY POINTER INTO lspace MAY NOW HAVE AN INVALID VALUE.
 * INFORMALLY: increase the size of lspace from cursize by the largest 
 *		possible amount not larger than size.
 */
static unsigned 
grow_core(unsigned size, unsigned cursize)
{
	size_t newsize;
	unsigned long longnewsize;

	longnewsize = (unsigned long) size + (unsigned long) cursize;
	if (longnewsize < MINMEM)
		longnewsize = MINMEM;
	else
	if (longnewsize > MAXMEM)
		longnewsize = MAXMEM;
	newsize = (size_t) longnewsize;
	if (lspace==(int *)NULL ) {
		newlspace = (int *)malloc(newsize);
	} else {
		newlspace = (int *)realloc(lspace,newsize);
	}
	if (newlspace == (int *)NULL ) {
		return(0);
	} else {
		lspace = newlspace;
		return(newsize-cursize);
	}
}

/*
 * NAME: fgetrec
 * FUNCTION: get a char string from the stream and, if field sorting or
 *           other special functions (e.g., folding) is needed, retrieve
 *	     sort fields, converting string fields to strxfrm format, and
 *	     prepend sort key(s) to the record. If possible, the record
 *	     is built in the sort's work area; if that is not possible
 *	     (not enough space) it is built in a work area and moved.
 *           The key area is structured as follows:
 *		Bytes 0-1:		key length (offset to data)
 *		Bytes 2-3:		length flag field 0
 *		Bytes 4-i:		key field 0
 *		Bytes i+1-i+2:		length flag field 1
 *		Bytes i-3...		key field 1 ...
 *	     Subsequent fields follow in same format.	
 */
static int
fgetrec(char *s, int n, FILE *stream, char *f, int *partialp)
{
	char  *p;
	char  *l;
	char *ignore;
	char *code;
	char *tb;
	char *eb;
	char *yb;
	char *xb;
	static char *xbb;

	static int recl = 0;
	int k;
	struct field *fp;

	char savechar;
	int i, j, rc;

	*partialp = 0;		/* default is full record */

	if ( (cmpset == 0) || (Aflag == 1) ) {
		recl = 0;
		if (fgets(s, n, stream) == NULL)
			return (0);
		if ((rc = strlen(s)) == 0)
			return (0);
		else {
			*partialp = (s[rc-1] != '\n');	/* trailing '\n' */
			return (rc);		/* means complete read */
		}
	}

			/* The following code will prepend the record
			   with an encoded version of the sort keys.
			 */
	if (recl != 0) {
		if (recl < n) {
			memcpy(s, xbb, recl+1);
			j = --recl;
			recl = 0;
			return(j);
		} else {
			memcpy(s, xbb, n);
			xbb += n;
			recl -= n;
			*partialp = 1;	/* indicate partial read */
			return(n);
		}
	}
	tb = tbuf;
	if (fgets(tb, SORT_LINE_MAX-1, stream) == NULL) {
		rc = 0;
	}
	else	rc = strlen(tb);

	if (rc == 0)
		return (rc);
	
	te = &(tb[strlen(tb)-1]);
	if (*te == '\n') {
		te += 2;
		rc++;
	} 
	else {	
		if (rc < (SORT_LINE_MAX-1)) {
				/* the last record of the input */
				/* file is missing a NEWLINE    */
			if(f == NULL) newdiag(MSGSTR(NEWLINE4,
			  "warning: missing NEWLINE added at EOF\n"), "");
			else newdiag(MSGSTR(NEWLINE5,
			  "warning: missing NEWLINE added at end of input file %s.\n")
					, f);
			*++te = '\n';
			*++te = '\0';
			rc += 2;
		}
		else {
			fprintf(stderr,MSGSTR(TOOLONG2,
					"fatal: line too long %d\n"),SORT_LINE_MAX);
			term();
		}
	}
	eb = ebuf;
	if ((rc * 5) < n) {
		xb = xbb = s;
		xe = s + n - 1;
	} else {
		xb = xbb = xbuf;
		xe = xbuf + (sizeof(xbuf) -1);
	}
	xb += 2;		/* save space for length */
	if (nfields > 0) {
		*xb++ = '\0';
		*xb++ = '\2';
	}
	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		p = tb;
		if(k >= 0) {
			l = skip(p, fp, 1);
			p = skip(p, fp, 0);
		} else {
			l = eol(p);
		}
		if (l < p)
			l = p;
		if((fp->fcmp==NUM) || (fp->fcmp==XNUM)) {
			fp->fcmp=XNUM;
			j = l - p + 3;
			*xb++ = (j >> 8);
			*xb++ = (j & 0xff);
			if ((l-p) > (xe - xb - 1)) {
				if (xbb == s) {
					j = xb - xbb;
					memcpy(xbuf, xbb, j);
					xb = xbb = xbuf;
					xb += j;
					xe = xbuf + (sizeof(xbuf) -1);
				}
				else {  
					fprintf(stderr,MSGSTR(TOOLONG2,
					"fatal: line too long %d\n"),SORT_LINE_MAX);
					term();
				}
			}
			memcpy(xb, p, (l-p));
			xb += (l-p);
			*xb++ = '\0';
			continue;
		}

		fp->fcmp=XSTR;
		eb = ebuf;
		yb = xb;
		xb += 2;
		code = fp->code;
		ignore = fp->ignore;
		if ( (ignore != dict) && (ignore != nonprint) && 
							(code != fold) ) {
			savechar = l[0];
			*l = '\0';
			j = strxfrm(xb, p, (xe - xb - 1));
			if (j  > (xe - xb - 1)) {
				if (xbb == s) {
					i = xb - xbb;
					memcpy(xbuf, xbb, i);
					yb = xb = xbb = xbuf;
					yb += i-2;
					xb += i;
					xe = xbuf + (sizeof(xbuf) -1);
					j = strxfrm(xb, p, (xe - xb - 1));
				}
			}
			if (j  < (xe - xb - 1)) {
				yb[0] = ((j+3) >> 8);
				yb[1] = ((j+3) & 0xff);
				xb += ++j;
				l[0] = savechar;
			}	
			else { 
				fprintf(stderr,MSGSTR(TOOLONG2,
					"fatal: line too long %d\n"),SORT_LINE_MAX);
				term();
			}
		} else {
			fldtowa(p, l, eb, ignore, code);
			j = strxfrm(xb, ebuf, (xe - xb - 1));
			if (j  > (xe - xb - 1)) {
				if (xbb == s) {
					i = xb - xbb;
					memcpy(xbuf, xbb, i);
					yb = xb = xbb = xbuf;
					yb += i-2;
					xb += i;
					xe = xbuf + (sizeof(xbuf) -1);
					j = strxfrm(xb, ebuf, (xe - xb - 1));
				}
			}
			if (j  < (xe - xb - 1)) {
				j = strxfrm(xb, ebuf, (xe - xb - 1));
				xb += ++j;
				j += 2;
				*yb++ = (j >> 8);
				*yb = (j & 0xff);
			} else { 
				fprintf(stderr,MSGSTR(TOOLONG2,
					"fatal: line too long %d\n"),SORT_LINE_MAX);
				term();
			}
		}
	}
	recl = xb - xbb;
	xbb[0] = (recl >>  8);
	xbb[1] = (recl & 0xff);
	if (rc > (xe - xb - 1)) {
		if (xbb == s) {
			i = xb - xbb;
			memcpy(xbuf, xbb, i);
			xb = xbb = xbuf;
			xb += i;
			xe = xbuf + (sizeof(xbuf) -1);
		}
	}
	if (rc < (xe - xb - 1)) 
		memcpy(xb, tb, rc+1);
	else { 
		fprintf(stderr,MSGSTR(TOOLONG2,
			"fatal: line too long %d\n"),SORT_LINE_MAX);
		term();
	}
	recl += rc;
	/* recl = current record length and key length including \0 terminator
 	 ** n    = the amount of allocated memory available
	 ** if the recl is <= the amount of allocated memory available
	 **     copy the record into allocated memory
	 ** else copy as much as possible and save the rest until the
	 **     next call to fgetrec().  Set recl to amount remaining
	 **     and return the amount copied.
	  */
	if (recl <= n) {		 	     /* Defect 8046 */
		if (s != xbb)
			memcpy(s, xbb, recl);
		j = --recl;
		recl = 0;
		return(j);
	}
	else {
		/* There must be enough room for the \n and \0.
		** if not, fool it into copying all except \n and
		** \0.  If the two are not copied together, there
		** are problems in sort() when it goes to look for
		** the \n if \0 was the only byte copied the second
		** time around */
		if (recl-1 == n) n--;		  /* Defect 14770 */
		memcpy(s, xbb, n);
		s[n] = '\0';
		xbb += n;
		recl -= n;
		*partialp = 1;
		return(n);
	}

}
/*
 * NAME: fldtowa
 * FUNCTION: copies input string between (p) and (l) to work area (b),
 *	     removing non-sorting characters (i.e. non-dictionary and/or
 *	     non-printing) and optionally folding into lowercase.
 */
static void 
fldtowa(char *p, char *l, char *b, char *ignore, char *code)
{
	wchar_t wc;
	wchar_t *pwc = &wc;
	char *sb;
	int i,j;
	char up[MB_LEN_MAX];
	char *upp;
	int chrlen;

	sb = b;
	while (p < l) {
		if (mbcodeset) {	/* Copy multibyte character set field */
			/* Skip characters excluded from consideration in the field */
			if (ignore == dict) {     /* skip MBCS non-(<alphanum> or <blank>) */
				while((chrlen=mbtowc(pwc,p,mb_cur_max))>0 
					&& !(iswalnum(wc) || iswblank(wc)) && p<l)
				p += chrlen;
			}
			if (ignore == nonprint) { /* skip MBCS nonprinting non-(\t , \n) */
				while((chrlen=mbtowc(pwc,p,mb_cur_max))>0 
					&& !(iswprint(wc) || wc == L'\t' || wc == L'\n' ) && p<l)
				p += chrlen;
			}
			if (p>=l) continue;
			/* Copy one character in the field */
			if (code == fold) {	  /* copy MBCS folding lower case to UPPER */
				if ( (chrlen = mbtowc(pwc,p,mb_cur_max))>0 ) {
					p += chrlen;
					wc = towupper(wc);
					chrlen = wctomb(up,wc);
					for(i=0,upp=up;i<chrlen;i++)
						*b++ = *upp++;
				} else p++;
			} else {		  /* copy MBCS directly */
				i = mblen(p,mb_cur_max);
				for (j=0;j<i;j++);
					*b++ = *p++;
			}
		} else {		/* Copy single byte character set field */
			/* Skip characters excluded from consideration in the field */
			if (ignore == dict) {     /* skip SBCS non-(<alphanum> or <blank>) */
		   		while( !(isalnum(*p) || isblank(*p)) && p<l ) {
                        		p++;
				}
			}
			if (ignore == nonprint) { /* skip SBCS nonprinting non-(\t , \n) */
		   		while( !(isprint(*p) || *p == '\t' || *p == '\n') && p<l )
					p++;
			}
			if (p>=l) continue;
			/* Copy one character in the field */
			if (code == fold) {	  /* copy SBCS folding lower case to UPPER */
				*b++ = toupper(*p);
				p++;
			} else {		  /* copy SBCS directly */
				*b++ = *p++;
			}
		}
	}
	*b = '\0';

 	/* if an input string contains only non-sorting characters, treat it 
	   as a null string.						     */
	if (strcmp(sb, "\n") == 0)
		*sb = '\0';
} 
