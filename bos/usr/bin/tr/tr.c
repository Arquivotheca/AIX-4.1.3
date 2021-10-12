static char sccsid[] = "@(#)05	1.17.1.15  src/bos/usr/bin/tr/tr.c, cmdfiles, bos412, 9446C 11/14/94 16:47:05";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: tr
 *
 * ORIGINS: 3, 18, 26, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 *
 * $RCSfile: tr.c,v $ $Revision: 2.9.1.9 $ (OSF) $Date: 92/03/30 18:45:35 $
 *
 */
#define _ILS_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/localedef.h>
#include <sys/lc_core.h>
#include <sys/limits.h>
#include <string.h>
#include <wchar.h>
#include <patlocal.h>
#include <ctype.h>

#include "tr_msg.h"

/* # define DEBUG_LOCALEPATH 1  */

#define hdl __lc_collate

#define ESC_CHAR '/'	/* this character needs to be in the safe scan range */
			/* or between 0x01 and 0x3F			     */

static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_TR, Num, Str)

	/* Used to determine whether current locale is SBCS or MBCS .*/
static int mb_cur_max; /* max number of bytes per character in current locale */
static int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */

	/* For emulation of the non-portable  iswblank()  function.*/
static wctype_t blankhandle;

	/* Non-portable: locale database for LC_COLLATE information.
	 * Minimum and maximum wchar_t values for process codes in
	 * the current locale are  __OBJ_DATA(__lc_collate)->co_wc_min  and
	 * __OBJ_DATA(__lc_collate)->co_wc_max  respectively.
	 */

static wchar_t co_wc_min, co_wc_max;
static int howmanywchars;

#define MAX_STRXFRM	2

/* This value was defined as COLL_WEIGHTS_MAX + 2, actually there really */
/* is no limit to the number of bytes that a STRXFRM can return if we    */
/* let it.  However, since we only need the first primary collating      */
/* weight, we will limit this value to two wchars.  We can set the last  */
/* wchar to (wchar_t )NULL as required.                                  */

static int dflag = 0;
static int sflag = 0;
static int cflag = 0;
static int Aflag = 0;

static int badopt;
static int inString1;	/* Flag for equivalence-class error detection */

	/* Begin ASCIIPATH String representation data base:
	 *  In which a character's file code, process code,
	 *  and collation weight are all the same.
	 */
	/* EOS = NCHARS (as with ndef STRIPNULLS) */
#define	NCHARS	257
static int endchar = NCHARS;
#define	EOS	NCHARS

static unsigned short code[NCHARS];
static unsigned char squeez[NCHARS];
static unsigned short vect[NCHARS];
typedef struct {	
	int last, max, rep;	  /* last      = most recently generated character.    */
				  /* max       = high end of range currently being     */
				  /*              generated (if any), else 0.          */
				  /* rep       = number of repetitions of last left in */
				  /*              repetition currently being generated.*/
	unsigned char *p;	  /* p         = command line source String pointer.   */
	unsigned char *nextclass; /* nextclass = alternate source String pointer, into */
			 	  /*              string of members of a class.        */
	int nchars;		  /* Characters in string so far.                      */
	int belowlower;		  /* Characters below first [:lower:]                  */
	int belowupper;		  /* Characters below first [:upper:]                  */ 
	} string;
static string string1, string2;
	/* End   ASCIIPATH String representation data base */

	/* Begin LOCALEPATH String representation data base:
	 *  In which a character's file code, process code,
	 *  and collation weight(s) are all different.
	 */

	/* Structure representing a single character or a repetition.*/
struct charnode {
		int unique;			/* unique collating weight */
		wchar_t pc;			/* Process Code of character */
		int reps;			/* Number of repetitions of pc */
		} ;
typedef struct charnode strcompstruct;

	/* Structure representing an operand String in a non-POSIX locale.*/
typedef struct {
		unsigned char *strchrs; /* Original string characters */
		strcompstruct *strtab;  /* Points to table of components */
		int nchars;		/* Number of elements in strtab */
		int nexttab;		/* Next strtab[] entry to deliver */
		wint_t lastpc;		/* Most recently delivered character */
		int reps;		/* Repetitions left in current repetition */
		int belowlower;		/* Characters below first [:lower:] */
		int belowupper;		/* Characters below first [:upper:] */
	} strstruct;

	/* Static string tables for String1 and String2 */
static strstruct st1, st2;

	/* End   LOCALEPATH String representation data base */

static char * remove_escapes( char * );	/* Removes \seq (replace by byte value) */
static int  next(string *s);
static int  nextc(string *s);
static void string_to_struct(strstruct *s);
static int  get_next_character(wchar_t *wpp,unsigned char **pp);
static void generate_character_class(strstruct *s, int si);
static void generate_equiv_class(strstruct *s, wchar_t wc);
static void generate_range(strstruct *s,int ucw1,int ucw2);
static void insert_top(strstruct *s,wchar_t npc,int nreps);
static void bad_string_fail();
static void Usage();
static int  is_char_class(wchar_t wc, int ci);
static void complement_struct(strstruct *s);
static wint_t localenext(strstruct *s);
static int  intcmp(const void *, const void *);
#ifdef DEBUG_LOCALEPATH
static void dumpstring(strstruct *s, unsigned char *title);
#endif

	/* Variables for locale-dependent determination
	 * of character classes and equivalence classes.
	 */
static int POSIXlc_ctype, POSIXlc_collate;

	/* The POSIX Well-Known character class names. */
static unsigned char asclasname [12] [8] = { "alnum", "alpha", "blank", "cntrl", "digit",
		 	     "graph", "lower", "print", "punct", "space",
			     "upper", "xdigit" };
	/* Hardcoded knowledge of POSIX locale character classes */
	/* character classes are terminated by two nulls */
static unsigned char asclasmem [12] [96] = {
/* alnum */ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\0",
/* alpha */ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\0",
/* blank */ " \t\0",
/* cntrl */ "\07\10\11\12\13\14\15\00\01\02\03\04\05\06\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\177\0",
/* digit */ "0123456789\0",
/* graph */ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~\0",
/* lower */ "abcdefghijklmnopqrstuvwxyz\0",
/* print */ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ \0" ,
/* punct */ "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~\0",
/* space */ "\t\n\13\14\15 \0",
/* upper */ "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0",
/* xdigit */ "0123456789ABCDEFabcdef\0" };

	/* Various state variables for optimization */
static int String2found;	/* nonzero iff nonnull String2 was specified */

/*
 * NAME:	tr
 * FUNCTION:	copies standard input to standard output with substitution, deletion,
 *		or suppression of consecutive repetitions of selected characters.
 */  
main(int argc, char **argv)
{
	register int i;
	int j;
	register int c, d;
	unsigned short *compl;
	int oc;
	int op1,op2;
	unsigned char *cp;
	int cplen;
	wchar_t wc;
	wint_t *S1toS2;
	wint_t s1pc,s2pc;
	unsigned char *locsqueeze;
	wint_t lastwco,wco,wci;
	char *loc_val;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TR, NL_CAT_LOCALE);

		/* Parse command line */
	badopt = 0;
	while ((oc = getopt(argc,argv,"cdsA")) != -1 ) {
		switch((unsigned char)oc) {
		case 'c':
			cflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 's':
			sflag++;
			break;
		case 'A':
			Aflag++;
			endchar = 256;
			break;
	    	default:
			badopt++;	/* Option syntax or bad option.*/
		} /* switch((unsigned char)oc) */
	} /* while ((oc=getopt() ... */

		/* Get translation strings */
	switch(argc-optind) {
	case 2:
		string2.p = st2.strchrs = remove_escapes(argv[optind+1]);
	case 1:
		string1.p = st1.strchrs = remove_escapes(argv[optind]);
		break;
	default:
		badopt++;	/* Zero or (more than two) translation strings specified.*/
	} /* switch(argc-optind) */



	if (string2.p == NULL) {
		string2.p = (unsigned char *) "" ;
		st2.strchrs = (unsigned char *) "" ; 
		String2found = 0;
	}
	else	
	  String2found = 1;

		/* Check for illegal combinations of options and String parameters:
		 *     -c  -d  -s    String1  String1,2
		 *	0   0   0	bad	ok
		 *	0   0   1	ok	ok
		 *	0   1   0	ok	bad
		 *	0   1   1	bad	ok
		 *	1   0   0	bad	ok
		 *	1   0   1	ok	ok
		 *	1   1   0	ok	bad
		 *	1   1   1	bad	ok
		 */
	if (  (!String2found
	       && (  (!cflag && !dflag && !sflag) 
		   ||(!cflag &&  dflag &&  sflag)
		   ||( cflag && !dflag && !sflag)
		   ||( cflag &&  dflag &&  sflag) ) )
	    ||(String2found
	       && (  (!cflag &&  dflag && !sflag)
		   ||( cflag &&  dflag && !sflag) ) ) ) {
		fprintf(stderr,MSGSTR(BADCOMB,"tr: Invalid combination of options and Strings.\n"));
		badopt++;
	}

		/* If any command errors detected, issue Usage message and terminate.*/
	if (badopt) {
		Usage();
	}	

#ifdef DEBUG_OPTS
fprintf(stderr,"tr option summary: c=%d d=%d s=%d A=%d string1=\"%s\" string2=\"%s\"\n",
	cflag,dflag,sflag,Aflag,string1.p,string2.p);
#endif

	/* Get maximum bytes per character.*/
	mb_cur_max = MB_CUR_MAX;
	mbcodeset = (mb_cur_max > 1?1:0);

	/* Determine whether environment variables allow character classes
	 * and equivalence classes to be handled by POSIX locale tables.
	 */
	loc_val =  setlocale(LC_CTYPE,NULL);
	POSIXlc_ctype =   ( strcmp(loc_val,"C")==0
				|| strcmp(loc_val,"POSIX")==0 );
	loc_val = setlocale(LC_COLLATE,NULL);
	POSIXlc_collate =   ( strcmp(loc_val,"C")==0
				|| strcmp(loc_val,"POSIX")==0 );

	if (!Aflag && (!POSIXlc_ctype || !POSIXlc_collate))
		goto LOCALEPATH;

ASCIIPATH:

	string1.last = string2.last = 0;
	string1.max = string2.max = 0;
	string1.rep = string2.rep = 0;
	string1.nextclass = (unsigned char *)0x0;
	string2.nextclass = (unsigned char *)0x0;
	string1.nchars = 0;
	string1.belowlower = -1;
	string1.belowupper = -1;
	string2.nchars = 0;
	string2.belowlower = -1;
	string2.belowupper = -1;

	inString1=1;
	if(cflag) {
		for(i=0; i<endchar; i++)
			vect[i] = 0;
		while ((c = next(&string1)) != EOS)
			vect[c] = 1;
		j = 0;
		for(i=0; i<endchar; i++)
			if(vect[i]==0) vect[j++] = i;
		vect[j] = EOS;
		compl = vect;
	}
	for(i=0; i<endchar; i++) {
		code[i] = EOS;
		squeez[i] = 0;
	}
	for(;;){
		inString1=1;
		if(cflag) 
			c = *compl++;
		else
			c = next(&string1);
		inString1=0;
		if(c==EOS) break;
		d = next(&string2);
		if(d==EOS) d = c;
		code[c] = d;
		squeez[d] = 1;
	}
        if (sflag) {
                while ((d = next(&string2)) != EOS) {
                        squeez[d] = 1;
                        /*
                         * we want to exhaust string2 as fast as possible.
                         * we therefore reduce rep to 1, if not already there.
                         */
                        if (string2.rep > 0)
                                string2.rep = 1;
                }
        }
	for(i=0;i<endchar;i++) {
		if(code[i]==EOS) code[i] = i;
		else if(dflag) code[i] = EOS;
	}
	
	/* Read and process standard input */
        {
                register FILE *fi = stdin, *fo = stdout; 
                register sf = sflag;
                register save;

                for (;;) {
                        /* Get next input one-byte character: */
                        if ((c = getc(fi)) == EOF)
                                break;
                        if ((c = code[c]) != EOS)
                                if(!sf || c!=save || !squeez[c]){
                                        save = (unsigned short)c;
                                        putc((int)c, fo);
                                }
                }
        }
	exit(0);

LOCALEPATH:
	blankhandle = get_wctype("blank");
	/* Get bounds on wchar_t values for process codes.*/
	co_wc_min = __OBJ_DATA(__lc_collate)->co_wc_min;
	co_wc_max = __OBJ_DATA(__lc_collate)->co_wc_max;
	howmanywchars = (co_wc_max - co_wc_min + 1);
	S1toS2 = (wint_t *)malloc(howmanywchars*sizeof(wint_t));
	locsqueeze = (unsigned char *)malloc(howmanywchars*sizeof(unsigned char));

	inString1 =1;
	string_to_struct(&st1);
	inString1 =0;
#ifdef DEBUG_LOCALEPATH
dumpstring(&st1,"String1");
#endif
	if (cflag) {
		complement_struct(&st1);
#ifdef DEBUG_LOCALEPATH
dumpstring(&st1,"String1 after complement");
#endif
	}
	if(strlen((char *)st2.strchrs)!=0) {
		string_to_struct(&st2);
#ifdef DEBUG_LOCALEPATH
dumpstring(&st2,"String2");
#endif
	} else {
		st2.strtab = NULL;
	}
	for (wci=0;wci<howmanywchars;wci++) {
		S1toS2[wci] = (wint_t)co_wc_min + wci;
		locsqueeze[wci] = (unsigned char)0;
	}
	st1.nexttab = 1;
	st2.nexttab = 1;
	for (;;) {
		s1pc = localenext(&st1);
		s2pc = localenext(&st2);
		if (s1pc == -1 && s2pc == -1) 
			break;
		if (s1pc != -1) {
			if (dflag)
				S1toS2[s1pc - (wint_t)co_wc_min] = -1;
			else if (s2pc != -1)
				S1toS2[s1pc - (wint_t)co_wc_min] = s2pc;
		}
		if (sflag) {
			if (s2pc != -1)
				locsqueeze[s2pc-(wint_t)co_wc_min] = 1;
			else if (strlen((char *)st2.strchrs)==0 && s1pc != -1)
				locsqueeze[s1pc-(wint_t)co_wc_min] = 1;
		}
	}

	lastwco = (wchar_t)0;
	while ((wci=fgetwc(stdin)) != WEOF) {
		wci -= co_wc_min;
		if ((wco = (wint_t)S1toS2[wci]) != -1
				 && (!locsqueeze[wco-co_wc_min] || wco != lastwco)){
			lastwco = wco;
			putwchar((wchar_t)wco);
		}
			
	}

	free(S1toS2);
	free(locsqueeze);
	free(st1.strtab);
	if (st2.strtab != (strcompstruct *)NULL)
		free (st2.strtab);
	exit(0);
}

/*
 * NAME: next
 *
 * FUNCTION:	Get the next character represented in string s
 * ENTRY:	1. EITHER (LC_CTYPE and LC_COLLATE are each one of
 *		            C, POSIX, or En_US)
 *		   OR  -A option was specified.
 * EXIT:	1. IF (next character from s can be delivered as a single byte)
 *		   THEN return value = (int)cast of (next character or EOS)
 *		   ELSE error message is written to standard error
 *			and command terminates.
 */
static int 
next(string *s)
{
	int c,n;
	int base;
	unsigned char basechar;
				/* Next member of char class to return */
	static unsigned char *nextclass = (unsigned char *)NULL;
	unsigned char *dp;	/* Points to ending : in :] of a class name */
	int badstring,si,badclass;
	int state,save1,save2;
	unsigned char char1,char2,opchar,*savep;

		/* If we are generating class members, get the next one */
	if (s->nextclass != (unsigned char *)0x0) {
		if (!((*s->nextclass == '\0') && (s->nextclass[1] == '\0'))) {
			s->last = *s->nextclass++;
			s->nchars++;
			return (s->last);
		} else	s->nextclass = (unsigned char *)0x0;
	}

	if(--s->rep > 0){
		s->nchars++;
		return(s->last);
	}
	if(s->last < s->max) {
		s->nchars++;
		return(++s->last);
	}

	s->max=0;
	char1 = *s->p;
	save1 = (unsigned int)nextc(s);

	if ((char1=='[') || (*s->p=='-')) {
		/* Check for character class, equivalence class, range,
		 * or repetition in ASCIIPATH. Implementation uses a state
		 * machine to parse the POSIX syntax. Convention used is
		 * that syntax characters specified by POSIX must appear
		 * as explicit characters while user-specified characters
		 * (range endpoints, repetition character, and equivalence
		 *  class character) may use escape sequences.
		 */
	/* STATE  STRING SEEN	     *p         ACTION
	 *
	 *   1    [=	            '*'	        STATE=4		'[<char1>*'		<char1>='='
	 *			    '<char1>="  STATE=8		'[=<char1>='
	 *			    other       STATE=9
	 *
	 *   2	  [:	            '*'		STARE=5		'[<char1>*'		<char1>=':'
	 *		            <class>:]	ACCEPT		'[:<class>:]'
	 *			    other	STATE=9
	 *
	 *   3	  [<char1>          '*'		STATE=5		'[<char1>*'
	 *			    other	STATE=9
	 *
	 *   4    [=*               '='		STATE=8		'[=<char1>='		<char1>='*'
	 *                          '<digit>'	STATE=7		'[<char1>*<digit>'	<char1>='='
	 *                          ']'		ACCEPT		'[<char1>*]'		<char1>='='
	 *			    other	STATE=9
	 *
	 *   5	  [<char1>*         ']'		ACCEPT		'[<char1>*]'
	 *			    '<digit>'	STATE=7         '[<char1>*<digit>]'
	 *			    other	STATE=9
	 *
	 *   6    <char1>-          '<char2>'	ACCEPT		'<char1>-<char2>'
	 *			    other	STATE=9
	 *
	 *   7    [<char1>*<digit>  '<digit>'	STATE=7a	'[<char1>*<digit>'
	 *                          ']'		ACCEPT		'[<char1>*<digit>]'
	 *			    other	STATE=9
	 *
	 *   7a   [<char1>*<digits> '<digit>'	STATE=7a	'[<char1>*<digits>'
	 *                          ']'		ACCEPT		'[<char1>*<digits>]'
	 *
	 *   8    [=<char1>=        ']'		ACCEPT		'[=<char1>=]'
	 *			    other	STATE=9
	 *
	 *   9    '[<other>' 'c-'		ACCEPT		'[' or 'c' (set to process second char next)
	 *
	 */
		n = 0xffff;	/* For short path to STATE_7b */
		savep = s->p;	/* Save current string pointer */

		if (char1 == '[') {
			opchar = *((s->p)++);
			if      (opchar == '=') state=1;
			else if (opchar == ':') state=2;
			else {	/* Allow escape conversion of char1. */
				s->p--;
				char1 = (unsigned int)nextc(s);
				state=3;
			}
		} else {
			s->p++;
			char1 = save1;
			state=6;
		}
		while (state!=0) {
		switch(state) {
		case 1:
			opchar = *((s->p)++);
			if (opchar == '*') state=4;
			else { /* Allow escape conversion of char1. */
				s->p--;
				char1 = (unsigned int)nextc(s);
 				if (*((s->p)++) == '=') state=8;
				else state=9;
			}
			break;
		case 2:
			opchar = *((s->p)++);
			if (opchar == '*') state=5;
			else {
				/* Check for valid well-known character class name */
				s->p--;
				if ((dp = (unsigned char *)strchr((char *)s->p,':'))== NULL) {
					state=9;
					break;
				}
				if ( *(dp+1) != ']' ) {
					state=9;
					break;
				}
				*dp = '\0';
				badstring = 1; /* Until proven 0 by finding class name */
				for (si=0;si<12;si++) {
					if (strcmp((char *)s->p, (char *)asclasname[si])==0) {
						badstring = 0;
						break;
					}
				}
				if (badstring) {
					state=-1;
					break;
				} else {
					*dp = ':';
					s->p = dp+2;
				}
				/* Check for invalid use of character class in String2:*/
				if (s == &string2 && !(sflag && dflag)){
					badclass = 0;
					switch(si) {
					case 6:
						if(s->nchars != string1.belowupper)
							badclass++;
						break;
					case 10:
						if(s->nchars != string1.belowlower)
							badclass++;
						break;
					default: badclass++;
					}
					if(badclass) {
						fprintf(stderr,MSGSTR(CLASINS2,
					"tr: String2 contains an invalid character class.\n"));
						exit(1);
					}
				}

				/* For a character class: set string's alternate-source
				 * pointer to the hardcoded string of members of the class.
				 */
				if (si==6  && s->belowlower==-1) 
					s->belowlower = s->nchars;
				if (si==10 && s->belowupper==-1)
					s->belowupper = s->nchars;
				s->nextclass = &asclasmem[si][0];
				s->last = *s->nextclass++;
				state=0;
			}
			break;
		case 3:
			opchar = *((s->p)++);
			if (opchar == '*') state=5;
			else state=9;
			break;
		case 4:
			opchar = *((s->p)++);
			if (opchar == '=') {
				char1 = '*';
				state=8;
			}
			else if (opchar >= '0' && opchar <= '9') state=7;
			else if (opchar == ']') state=10; /* 7b */
			else state=9;
			break;
		case 5:
			opchar = *((s->p)++);
			if (opchar == ']') state=10; /* 7b */
			else if (opchar >= '0' && opchar <= '9') state=7;
			else state=9;
			break;
		case 6:
			if ((save2=(unsigned int)nextc(s)) != EOS) {
				char2=save2;
				if(char2 < char1) {
					fprintf(stderr,MSGSTR(BADORDER,
						"tr: Range endpoints out of order.\n"));
					exit(1);
				}
				s->max  = char2;
				s->last = char1;
				state=0;
			} else {
				state=9;
			}
			break;
		case 7:
			base = (opchar=='0') ? 8 : 10;  /* which base */
			basechar = (opchar=='0') ? '7' : '9';
			n = opchar - (unsigned int)'0';
			while((c = (unsigned int)*s->p)>='0' && c<=basechar) {
				n = base*n + c - (unsigned int)'0';
				s->p++;
			}
			if(*s->p++ != ']') {
				state=9;
				break;
			}
			if(n==0) n = 0xffff; /* Unspecified length */
		case 10: /* 7b, must follow case 7 without break; */
			/* ACCEPT action for repetitions from states 4, 5, and 7. */
			/* POSIX 1003.2/D11 Rule: No repetitions in String1 */
			if (inString1) {
				fprintf(stderr,MSGSTR(REPINS1,
					"tr: Character repetition in String1\n"));
				Usage();
			}
			s->rep  = n;
			s->last = char1;
			state=0;
			break;
		case 8:
			if (*s->p++ == ']') {
				/* POSIX 1003.2/D11 Rule: No equiv classes in String2 */
				if (inString1==0 && (!dflag || !sflag)) {
					fprintf(stderr,MSGSTR(EQVINS2,
						"tr: Equivalence class in String2\n"));
					Usage();
				}
				/* Ascii equivalence classes are just one character */
				s->last = char1;
				state=0;
			} else state=9;
			break;
		case 9:
			/*
			 * Pass first character as a literal and then parse
			 * again starting at the next character position.
			 */
			s->p    = savep;
			s->last = save1;
			state=0;
			break;
		default: /* ERROR state */
			bad_string_fail();
			break;
		} /* switch (state) */
		} /* while (state != 0) */
	} else { /* if ((char1=='[') || (*s->p=='-')) */
		s->last = save1;
	} /* if ((char1=='[') || (*s->p=='-')) ... else */
	s->nchars++;
	return(s->last);
}

/*
 * NAME: nextc
 *
 * FUNCTION: get the next character from string s with escapes resolved
 * ENTRY:	1. EITHER (LC_CTYPE and LC_COLLATE are each one of
 *		            C or POSIX)
 *		   OR  -A option was specified.
 * EXIT:	1. IF (next character from s can be delivered as a single byte)
 *		   THEN return value = (int)cast of (next character or EOS)
 *		   ELSE error message is written to standard error
 *			and command terminates.
 */
static int 
nextc(string *s)
{
	register int i, n;
	int chrlen;
	unsigned char *msgstr;
	unsigned char c;

	c = *s->p++;
	if (c == '\0') {
		--s->p;
		return (EOS);
		}
	else
		if (c == ESC_CHAR) {
			c = *s->p++;
			if (c == '0')
				c='\0';
			}

	return((int)c);
}


/* NAME:	string_to_struct
 * FUNCTION:	Map operand string into string representation structure.
 * ENTRY:	1. s->strchrs points to an operand string.
 *		2. At least one of LC_CTYPE and LC_COLLATE is not one of
 *		    C or POSIX. (Must use locale-dependent processing.)
 * EXIT:	1. s->strchrs = s'->strchrs
 *		2. s-> represents the string in s->strchrs.
 */
static void
string_to_struct(strstruct *s)
{
	unsigned char *p,*maxp;
	/* 
	 * FIRST wchar_t is wchar_t-casted unsigned char
	 * from command line; SECOND is (wchar_t)\0 .
	 */
	wchar_t wp,wp2,wp3;
	int i,si,chrlen,baseint,n;
	strcompstruct *sis;
	unsigned char basechr;
	unsigned char *dp;
	int badstring;
	int state;
	unsigned char char1,char2,opchar,*savep;

	/* Initialise */
	s->strtab = (strcompstruct *)malloc((howmanywchars+2)*sizeof(strcompstruct));
	s->nchars = 0;
	s->lastpc = (wint_t)-1;
	s->reps = 0;
	s->belowlower = -1;
	s->belowupper = -1;
	maxp = s->strchrs + strlen((char *)s->strchrs);
	
	/* Process characters from String: */
	for (p=s->strchrs; p!=NULL && p<=maxp && *p!='\0';) {
		
   	    /* Process characters starting with *p */
	    char1 = *p;
	    if((chrlen=get_next_character(&wp3,&p))==0) {
		bad_string_fail();
	    }

		/* Range or some kind of class */
		if ((char1=='[') || (*p=='-')) {
		/*
		 * Description of the state machine for parsing POSIX ranges,
		 * classes, and repetitions is in comment in routine next() for
		 * ASCIIPATH. Difference here is in handling char1 and char2 as
		 * wp and wp2, and in  the ACCEPT actions.
		 */
		n = 0xffff;	/* For short path to state 7b */
		savep = p;

		if (char1 == '[') {
			opchar = *p++;
			wp = (wchar_t)opchar;
			if      (opchar == '=') state=1;
			else if (opchar == ':') state=2;
			else { /* Allow escape conversion of char1. */
				p--;
				if((chrlen=get_next_character(&wp,&p))==0) {
					if (*p==0)
						p--;
					else
						bad_string_fail();
				}
				state=3;
			}
		} else {
			p++;
			wp = wp3;
			state=6;
		}
		while (state != 0) {
		switch(state) {
		case 1:
			opchar = *p++;
			if (opchar == '*') state=4;
			else { /* Allow escape conversion of char1. */
				p--;
				if((chrlen=get_next_character(&wp,&p))==0) {
					bad_string_fail();
				}
 				if (*p++ == '=') state=8;
				else state=9;
			}
			break;
		case 2:
			opchar = *p++;
			if (opchar == '*') state=5;
			else {
				/* Check for valid well-known character class name */
				p--;
				if ((dp = (unsigned char *)strchr((char *)p,':'))== NULL) {
					state=9;
					break;
				}
				if ( *(dp+1) != ']' ) {
					state=9;
					break;
				}
				*dp = '\0';
				badstring = 1; /* Until proven 0 by finding class name */
				for (si=0;si<12;si++) {
					if (strcmp((char *)p,(char *)asclasname[si])==0) {
						badstring = 0;
						break;
					}
				}
				if (badstring) {
					state=-1;
					break;
				} else {
					*dp = ':';
					p = dp+1;
				}
				/* Generate string component for the class:*/
				generate_character_class(s,si);
				state=0;
			}
			break;
		case 3:
			opchar = *p++;
			if (opchar == '*') state=5;
			else state=9;
			break;
		case 4:
			opchar = *p++;
			if (opchar == '=') {
				wp = '*';
				state=8;
			}
			else if (opchar >= '0' && opchar <= '9') state=7;
			else if (opchar == ']') state=10; /* 7b */
			else state=9;
			break;
		case 5:
			opchar = *p++;
			if (opchar == ']') state=10; /* 7b */
			else if (opchar >= '0' && opchar <= '9') state=7;
			else state=9;
			break;
		case 6:
			if((chrlen=get_next_character(&wp2,&p)) != 0) {
				int ucw1,ucw2;
				char * dummy1=NULL;
				int dummy2;
				ucw1=__wcuniqcollwgt(wp3);
				ucw2=__wcuniqcollwgt(wp2);
				if (ucw1>ucw2) {
					fprintf(stderr,MSGSTR(BADORDER,
						"tr: Range endpoints out of order.\n"));
					exit(1);
				}
				generate_range(s,ucw1,ucw2);
				state=0;
			} else {
				state=9;
			}
			break;
		case 7:
			baseint = (opchar=='0') ? 8 : 10;  /* which base */
			basechr = (opchar=='0') ? (unsigned char)'7' : (unsigned char)'9';
			n = opchar - (unsigned int)'0';
			while (*p >= (unsigned char)'0' && *p <= basechr) {
				n = n*baseint +(unsigned int)*p -(unsigned int)'0';
				p++;
			}
			if(*p++ != ']') {
				state=9;
				break;
			}
			if(n==0) n = 0xffff; /* Unspecified length */
		case 10: /* 7b, must follow case 7 without break; */
			/* ACCEPT action for repetitions from states 4, 5, and 7. */
			/* POSIX 1003.2/D11 Rule: No repetitions in String1 */
			if (inString1) {
				fprintf(stderr,MSGSTR(REPINS1,
					"tr: Character repetition in String1\n"));
				Usage();
			}
			/* Generate repetition. */
			insert_top(s,wp,n);
			state=0;
			break;
		case 8:
			if (*p++ == ']') {
				/* POSIX 1003.2/D11 Rule: No equiv classes in String2 */
				if (inString1==0 && (!dflag || !sflag)) {
					fprintf(stderr,MSGSTR(EQVINS2,
						"tr: Equivalence class in String2\n"));
					Usage();
				}
				generate_equiv_class(s,wp);
				state=0;
			} else state=9;
			break;
		case 9:
			/*
			 * Pass first character as a literal and then parse
			 * again starting at the next character position.
			 */
			p    = savep;
			insert_top(s,wp3,1);
			state=0;
			break;
		default: /* ERROR state */
			bad_string_fail();
			break;
		} /* switch (state) */
		} /* while (state != 0) */
	    } else { /* if ((char1=='[') || (*p=='-')) */
		/* Single character */
		insert_top(s,wp3,1);
	    } /* if ((char1=='[') || (*p=='-')) ... else */
	} /* for (p=s->strchrs,chrlen=get_next_character(&wp,&p);... */
}

/* NAME:	get_next_character
 * FUNCTION:	Get the next wchar_t character from a possibly multibyte string.
 * ENTRY:
 * EXIT:	1. Return value = IF (*pp' pointed to a valid character file code
 *					or a valid escape sequence beginning with '\')
 *				  THEN (the number of bytes in the character)
 *				  ELSE 0
 *		2. *wpp = IF (return value = 0)
 *			  THEN (wchar_t) process code for character *pp' pointed to
 *			  ELSE unchanged
 *		3. *pp  points to next un-get'ed character in string.
 */
static int
get_next_character(wchar_t *wpp,unsigned char **pp)
{
	int i,n,chrlen,rc;

	if (**pp == ESC_CHAR) {
		(*pp)++;
		if (**pp == '0') 
			*wpp = (wchar_t)L'\0';
		else
			*wpp = (wchar_t)**pp;
		(*pp)++;
		return (1);
		}

	if ((chrlen = mbtowc(wpp,(char *)*pp,mb_cur_max))>0) {
		*pp += chrlen;
		rc = chrlen;
	} else
		rc = 0;
	return(rc);
}

/* NAME: 	generate_character_class
 * FUNCTION:	Generate members of a character class.
 * ENTRY:
 * EXIT:
 */
static void
generate_character_class(strstruct *s, int si)
{
	wchar_t testwc[2];
	int prevtop;
	int chrsbelow;
	int sci;
	strcompstruct *scp, *prevtopscp, *newtopscp;
	unsigned int ci;
	size_t n;
	int pseudoclass,gen_si;
	int badclass;

	prevtop = s->nchars;
	prevtopscp = s->strtab + s->nchars;
	testwc[1] = (wchar_t)'\0';
	badclass=0;

	/* Determine whether this is a genuine character class or a POSIX
	 * case-conversion pseudoclass. It is a pseudoclass if:
	 *	s == &st2; and
	 *	(if si==[:lower:] then st1.belowupper is exactly as many
	 *	  characters as are now in the string; or
	 *	if si==[:upper:] then st1.belowlower is exactly as many
	 *	  characters as are now in the string; or
	 * Useful undocumented knowledge: si==6 is [:lower:] and si==10 is
	 * [:upper:]. (These are indices into asclasname[].)
	 */
	if (s==&st2) {
		if (dflag && sflag) {
			pseudoclass = 0;
		} else if (   (si==6  && st1.belowupper != -1)
			   || (si==10 && st1.belowlower != -1)) {
			chrsbelow = 0;
			for (scp=&(s->strtab[1]); scp<=prevtopscp; scp++)
				chrsbelow += scp->reps;
			if (si==6) {
				if (chrsbelow == st1.belowupper)
					pseudoclass = 1;
				else	/* Error: [:lower:] in String2 not in same
					 * position as [:upper:] in String1.
					 */
					badclass = 1;
			} else {
				if (chrsbelow == st1.belowlower)
					pseudoclass = 1;
				else	/* Error: [:upper:] in String2 not in same
					 * position as [:lower:] in String1.
					 */
					badclass = 1;
			}
		} else	/* Error: [:lower:] or [:upper:] is in String2
			 * but the other one is not in String1, or else
			 * any class other than [:lower:] or [:upper:]
			 * in String2 without options  -ds.
			 */
			badclass = 1;

		if (badclass) {
			fprintf(stderr,MSGSTR(CLASINS2,
					"tr: String2 contains an invalid character class.\n"));
			exit(1);
		}
	} else pseudoclass = 0;

	/* If this is the first occurrence of either of the case conversion 
	 * classes, determine its position in the string.
	 */
	if (  (si==6  && s->belowlower==-1)
	    ||(si==10 && s->belowupper==-1)){
		chrsbelow = 0;
		for (scp=&(s->strtab[1]);scp<=prevtopscp;scp++)
			chrsbelow += scp->reps;
		if(si==6)
			s->belowlower = chrsbelow;
		else
			s->belowupper = chrsbelow;
	}

	/* If generating case-converted characters in String2, start with
	 * the same character class as in String1 to occupy the same number
	 * of character positions in the same order.
	 */
	if (pseudoclass==0)	gen_si=si;
	else			gen_si=(si==6?10:6);

	/* Generate genuine character class: enumerate the
	 * process codes and insert those that are members.
	 */
	for (ci=co_wc_min;ci<=co_wc_max;ci++) {
		testwc[0] = (wchar_t)ci;
		if (is_char_class(testwc[0],gen_si)) {
			insert_top(s,testwc[0],1);
		}
	} /* for (all process codes) */
	/* Sort the newly inserted characters in collation order if required for POSIX.
	 * Collation sort is required if both String1 and String2 were specified and in
	 * addition we are not processing a String1 component that will be complemented.
	 */
	if (String2found && !(inString1 && cflag))
	  qsort((void *)&(s->strtab[prevtop+1]),(size_t)s->nchars-prevtop,sizeof(strcompstruct),(int (*)(const void*, const void*))intcmp);


	/* POSIX case-conversion request: molest the genuine character class into a
	 * case-conversion substring by replacing each character we just inserted
	 * with the tolower() or towupper() converted character.
	 */
	if ( pseudoclass!=0 ) {
		newtopscp = s->strtab + s->nchars;
		for (scp=prevtopscp+1;scp<=newtopscp;scp++) {
			char * dummy1=NULL;
			int dummy2;
			testwc[0] = (si==6 ? towlower(scp->pc) : towupper(scp->pc));
			scp->pc = testwc[0];
			scp->unique=__wcuniqcollwgt((wchar_t)testwc[0]);
		}
	}

}

/* NAME:	generate_equiv_class
 * FUNCTION:	Generate members of an equivalence class.
 * ENTRY:
 * EXIT:
 */
static void 
generate_equiv_class(strstruct *s, wchar_t wc)
{
	wchar_t wcsxfrmvalue[MAX_STRXFRM];
	wchar_t equivwxs[MAX_STRXFRM];
	wchar_t testwc[2];
	wchar_t *testwcp = &testwc[0];
	int prevtop;
	unsigned int ci;
	size_t n;

	/* Get primary collation weight part of base character's wcsxfrm() string.*/
	testwc[0] = wc;
	testwc[1] = (wchar_t)'\0';
	wcsxfrm(equivwxs,testwcp,MAX_STRXFRM);
	prevtop = s->nchars;
	/* Enumerate the valid process codes and test each for equivalence.*/
	for (ci=co_wc_min;ci<=co_wc_max;ci++) {
		/* Compare primary collation weights, first part of wcsxfrm() string */
		testwc[0] = (wchar_t)ci;
		n = wcsxfrm(wcsxfrmvalue,testwcp,MAX_STRXFRM);
		if (n != -1 && wcsxfrmvalue[0]==equivwxs[0]) {
			insert_top(s,testwc[0],1);
		}
	}
	/* Sort the newly inserted characters in collation order if required for POSIX.
	 * Collation sort is required if both String1 and String2 were specified and in
	 * addition we are not processing a String1 component that will be complemented.
	 */
	if (String2found && !(inString1 && cflag))
	  qsort((void *)&(s->strtab[prevtop+1]),(size_t)s->nchars-prevtop,sizeof(strcompstruct),(int (*) (const void *, const void *))intcmp);
}

/* NAME:	generate_range
 * FUNCTION:	Generate members of a range of characters.
 * ENTRY:	wcsncmp(ucw1,ucw2,1) <= 0
 *              both ucw1 and ucw2 are unique collating weights
 * EXIT:
 */
static void
generate_range(strstruct *s,int ucw1,int ucw2)
{
	int prevtop;
	unsigned int ci;
	size_t n;
	char *dummy1=NULL;
	int dummy2;
	int ucw;

	prevtop = s->nchars;
	/* Enumerate the valid process codes and test each for membership.*/
	for (ci=co_wc_min;ci<=co_wc_max;ci++) {
		ucw=__wcuniqcollwgt((wchar_t)ci);
		/* Compare unique collation weights */
		if ((ucw>=ucw1) && (ucw<=ucw2))
			insert_top(s,(wchar_t)ci,1);
			/* don't need primary weight here so use ucw */
	}
	/* Sort the newly inserted characters in collation order if required for POSIX.
	 * Collation sort is required if both String1 and String2 were specified and in
	 * addition we are not processing a String1 component that will be complemented.
	 */
	if (String2found && !(inString1 && cflag))
	  qsort((void *)&(s->strtab[prevtop+1]),(size_t)s->nchars-prevtop,sizeof(strcompstruct),(int (*)(const void*, const void*))intcmp);
}

/* NAME:	insert_top
 * FUNCTION:	Insert a character at top of a string representation list.
 * ENTRY:
 * EXIT:
 */
static void 
insert_top(strstruct *s,wchar_t npc,int nreps)
{
	strcompstruct *new;
	int nospace;
	char *dummy1=NULL;
	int dummy2;
	int ucw;

	nospace = 0;
	if (s->nchars++ < howmanywchars) {
		new = &(s->strtab[s->nchars]);
		new->pc   = npc;
		new->reps = nreps;
		ucw=__wcuniqcollwgt(npc);
		new->unique = ucw;
	} else {
		nospace = 1;
	}

	if (nospace) {
		fprintf(stderr,MSGSTR(NOSPACE,"Insufficient storage.\n"));
		exit(1);
	}
}

/* NAME:	bad_string_fail
 * FUNCTION:	Issue diagnostic about invalid String1 or String2 content between
 *		 square brackets to standard error
 *		 and immediately terminate the tr command with return value 1.
 * ENTRY:
 * EXIT:
 */
static void
bad_string_fail()
{
	fprintf(stderr,MSGSTR(BADSTR,"Bad string between [ and ].\n"));
	exit(1);
}

/* NAME:	Usage
 * FUNCTION:	Issue Usage message to standard error and immediately terminate
 *               the tr command with return value 1. 
 * ENTRY:
 * EXIT:
 */
static void
Usage()
{
	fprintf(stderr, MSGSTR(BADUSE2,
"Usage: tr [ -c | -cds | -cs | -ds | -s ] [-A] String1 String2\n\
       tr { -cd | -cs | -d | -s } [-A] String1\n") );
	exit(1);
}

/* NAME:	is_char_class
 * FUNCTION:	Tests whether a wchar_t belongs to one of the 12 POSIX
 *		 well-known character classes.	
 * ENTRY:
 * EXIT:	Return value = same as   iswctype(wc,wctype(asclasname[ci]))
 */
static int
is_char_class(wchar_t wc, int ci)
{
	/* This function can be delivered in three ways:
	 * 1. By crude switch as is done here.
	 * 2. By table of pointers to the isw* functions, indexed by ci.
	 * 3. By   return(iswctype(wc,wctype(asclasname[ci])));
	 * This method should be faster than the  iswctype  method.
	 * This method will work with is_w*() implemented as either
	 * functions or macros.
	 * A compiler using currently well-known methods will convert this
	 * switch into code at least as efficient as a table of branches to
	 * the  isw*()  invocations.
	 */
	int itsin;
	switch(ci) {
	case  0:	/* alnum */
		itsin = iswalnum(wc);
		break;
	case  1:	/* alpha */
		itsin = iswalpha(wc);
		break;
	case  2:	/* blank */
		itsin = is_wctype(wc,blankhandle);
		break;
	case  3:	/* cntrl */
		itsin = iswcntrl(wc);
		break;
	case  4:	/* digit */
		itsin = iswdigit(wc);
		break;
	case  5:	/* graph */
		itsin = iswgraph(wc);
		break;
	case  6:	/* lower */
		itsin = iswlower(wc);
		break;
	case  7:	/* print */
		itsin = iswprint(wc);
		break;
	case  8:	/* punct */
		itsin = iswpunct(wc);
		break;
	case  9:	/* space */
		itsin = iswspace(wc);
		break;
	case 10:	/* upper */
		itsin = iswupper(wc);
		break;
	case 11:	/* xdigit */
		itsin = iswxdigit(wc);
		break;
	default:
		itsin = 0;
	} /* switch(si) */
	return(itsin);
}

/* NAME:	complement_struct
 * FUNCTION:	Complement a string representation structure in place.
 * ENTRY:	1. s->poolnext and s->poolmax are set for a dynamically allocated
 *		    block of size (co_wc_max - co_wc_min + 1)*sizeof(strcompstruct)
 * EXIT:	1. Original s->comppool has been free()d. A new one has replaced it.
 *		2. s-> represents the complement of the string represented 
 *		    by s'-> with respect to the character set of LC_CTYPE.
 */
static void
complement_struct(strstruct *s)
{
	wchar_t testwc[2];
	int prevtop;
	int n;
	strcompstruct *originalstrtab, *sc, *maxsc;
	int originalnchars;
	unsigned int ci;
	unsigned char *inString1, *inString1p, *maxinString1p;

	prevtop = s->nchars;
	testwc[1] = (wchar_t)L'\0';
	originalstrtab = s->strtab;
	originalnchars = s->nchars;
	maxsc = originalstrtab + originalnchars;
	/* Use the unused part of String1 representation for its complement */
	s->strtab = &(s->strtab[s->nchars+1]);
	s->nchars = 0;
	/* Mark process codes that are in String1 */
	inString1 = (unsigned char *)malloc(howmanywchars*sizeof(unsigned char));
	maxinString1p = inString1+howmanywchars;
	for (inString1p=inString1;inString1p<maxinString1p;inString1p++)
		*inString1p=0;
	for (sc=originalstrtab+1;sc<=maxsc;sc++) {
		inString1p = inString1 + sc->pc - co_wc_min;
		*inString1p = 1;
	}
	/* Put all unmarked process codes into complement. */
	for (ci=co_wc_min;ci<=co_wc_max;ci++) {
		inString1p = inString1 + ci - co_wc_min;
		if (*inString1p == 0) {
			testwc[0] = (wchar_t)ci;
			insert_top(s,testwc[0],1);
		}
	}
	/* Sort the complement characters in collation order if required for POSIX.
	 * Collation sort is required if both String1 and String2 were specified and in
	 * addition we are not processing a String1 component that will be complemented.
	 */
	if (String2found)
	  qsort((void *)&s->strtab[1],(size_t)s->nchars,sizeof(strcompstruct),(int (*) (const void*, const void*))intcmp);
	/* Clean up debris */
	free (inString1);
}

/* NAME:	localenext
 * FUNCTION:	Deliver next wchar_t character from a completed string structure.
 *		(This is the LOCALEPATH version of next() .)
 * ENTRY:	1. s->nextc is set to first component strcompstruct of the string
 *		    by the caller before the first call to this routine.
 * EXIT:	1. Return value = if (there is another character not yet delivered)
 *				   then (that character)  else  -1.
 */
static wint_t 
localenext(strstruct *s)
{
	strcompstruct *thisc;

	if (s->reps) {
		s->reps--;
		return (s->lastpc);
	}
	if (s->nexttab <= s->nchars) {
		thisc = &(s->strtab[s->nexttab]);
		s->nexttab++;
		if (thisc->reps > 1) {
			s->reps = thisc->reps-1;
		}
		s->lastpc = (wint_t)thisc->pc;
		return ((wint_t)thisc->pc);
	} else {
		return(-1);
	}
}

#ifdef DEBUG_LOCALEPATH
/* NAME:	dumpstring
 * FUNCTION:	Debugging only: dump contents of a *strstruct string representation.
 * ENTRY:	1. s-> may be a String or a Complemented-String.
 * EXIT:	1. Output dumped to standard error.
 */
static void
dumpstring(strstruct *s, unsigned char *title)
{
strcompstruct *sc, *maxsc;
unsigned char cc;
fprintf(stderr,"String structure dump: %s\n",title);
fprintf(stderr,"pc(X)    uniq(X)  reps  pc(c)\n");
maxsc=s->strtab + s->nchars+1;
for(sc=&s->strtab[1];sc<maxsc;sc++){
 cc=(unsigned char)sc->pc;
 fprintf(stderr,"%5.4X%11.4X%5d",sc->pc,sc->unique,sc->reps);
 if (isprint(cc)) fprintf(stderr,"%7c\n",cc);
 else 
	fputc('\n',stderr);
}
fprintf(stderr,"End of string structure dump: %s\n",title);
}
#endif

/*
 * remove_escapes - takes \seq and replace with the actual character value.
 * 		\seq can be a 1 to 3 digit octal quantity or {abfnrtv\}
 *
 *		This prevents problems when trying to extract multibyte
 * 		characters (entered in octal) from the translation strings
 *
 * Note:	translation can not be done in place, since the result may
 * 		be larger than the source.
 */

char *
remove_escapes( char *s ){
    char *d;			/* Position in destination of next byte */
    int i,n;
    int mb_cur_max = MB_CUR_MAX;
    char * ret_val;

    /* Prepare for the worse case, where the entire pattern is repetitions */
    /* of ESC_CHAR, by allocating twice the size of the input string       */

    if ((d = ret_val = malloc(strlen(s)*2+1))==NULL) {
	perror("tr");
	exit(1);
	}

    while (*s) {		/* For each byte of the string */
	switch (*s) {
	  default:
	    i = mblen(s, mb_cur_max);
	    if (i < 0) i=1;	/* Not a MB char - just move one byte */
	    
	    if ((i==1) && (*s == ESC_CHAR))  /* escape ESC_CHAR */
		*d++ = ESC_CHAR;

	    while(i--) { *d++ = *s++; }; /* Move the character */
	    break;

	  case '\\':
	    switch (*++s) {
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7':	
			i = n = 0;
			while (i<3 && *s >= '0' && *s <= '7') {
			    n = n*8 + (*s++ - '0');
			    i++;
			}
			if (n==0)	{	/* create escaped zero */
				*d++ = ESC_CHAR;
				n = '0';
				}
			else
			if (n==ESC_CHAR)  /* create escaped ESC_CHAR */
				*d++ = ESC_CHAR;

			*d++ = n; break;

	      case 'a':	*d++ = '\a';	s++; break;
	      case 'b':	*d++ = '\b';	s++; break;
	      case 'f':	*d++ = '\f';	s++; break;
	      case 'n':	*d++ = '\n';	s++; break;
	      case 'r':	*d++ = '\r';	s++; break;
	      case 't':	*d++ = '\t';	s++; break;
	      case 'v':	*d++ = '\v';	s++; break;
	      case '\\':*d++ = '\\';	s++; break;
	      default:	*d++ = *s++;	break;
	    }
	}			/*     switch */
    }				/* while (*s) */
    *d = '\0';
    return (ret_val);
}

/* intcmp():  comparison function used for qsort()
 *
 * compares the ints pointed to by the arguments.
 * This means that the structure pointers passed in must start with
 * the unique collating weight.
 */

static int  intcmp(const void * i1, const void * i2)
{
return(*(int *)i1 - *(int *)i2);
}
