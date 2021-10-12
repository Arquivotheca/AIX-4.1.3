/* @(#)53	1.28  src/bos/usr/include/NLregexp.h, libcnls, bos411, 9428A410j 5/9/94 10:16:51 */
#ifndef _H_NLREGEXP
#define _H_NLREGEXP
/*
 * COMPONENT_NAME: libcnls
 *
 * FUNCTIONS: __ecmp, __getintvl, __isthere, advance,
 *	      compile, step
 *
 * ORIGINS: 3,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */

/*
 * For C++ compilers.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <NLchar.h>
#include <values.h>


#ifndef RE_DUP_MAX	/* sys/limits.h */
#define RE_DUP_MAX 255
#endif

/* This is the new (or large charset) version of regexp.h.              */
/* The main differences are that the [bracket] range expression bitmap  */
/* is replaced by a straight list for multibyte languages, and that a   */
/* [:charclass:] definition is allowed within brackets. Ranges are      */
/* handled as a "substring" of entries, with an "or" rather than        */
/* "and" relationship. Note also that compares within brackets          */
/* is done on character values, except for dashranges, where collation  */
/* values are used.                                                     */
/* The normal method for defining character classes ([a-z]) does not    */
/* work well in an international environment; the new charclass element */
/* (with syntax "[:" name ":]", e.g. [:upper:]) provides the needed     */
/* capability.                                                          */
/*                                                                      */
/* Hex compile codes:                                                   */
/****************************************************/
/* function               normal  normal  normal    */
/*                                + STAR  +INTVL    */
/* CCHR   normal char       04      05      06      */
/* CDOT   dot in this pos   08      09      0a      */
/* CENT   end group \)      0c      0d      0e      */
/* CBACK  \[1-9] indicator  10      11      12      */
/* CDOL   EOL anchor ($)    14                      */
/* CCEOF  end compiled pat  16                      */
/* CBRA   new [ string      18      19      1a      */
/* CNEG   new [^ string     1c      1d      1e      */
/* CLASS  [:cclass:]        20                      */
/* CEQV   [=x=] (not [=.=]) 22                      */
/* CELEM  [.xx.]            24                      */
/* CRNGE  new range (a-z)   26                      */
/* CKET   new ]             28                      */
/* CPAR   start group \(    2c      2d      2e      */
/* CBIT   [ bitmap ]        30      31      32      */
/****************************************************/
/* A "typical" regular expression, e.g.                                   */
/*      'ab*[a[.LL.]c-f[:digit:]].*\(x\)\{1,2\}'  (LANG set to Sp_SP)     */
/* would be compiled into (hex values):                                   */
/*                                                                        */
/*       04 61 05 62 18 00 10 04 00 61 24 01 7d 26 01 50 01               */
/*       65 20 03 28 09 2e 00 00 06 04 78 0e 00 01 02 16                  */
/*                                                                        */
/* which is:                                                              */
/*                                                                        */
/*      a       b*    [                 a         [.LL.]      c -         */
/*   04  61  05  62  18    00 10     04  00  61  24  01  7d  26   01  50  */
/*  CCHR a  CCHR b   CBRA  length   CCHR   a    CELEM  LL   CRNGE   c     */
/*          STAR           in bytes                                       */
/*                                                                        */
/*      f        [:digit:]      .*   \(                        x          */
/*   01  65   20     03   28    09    2e     00    00 06     04  78       */
/*      f    CLASS digit CKET  CDOT  CPAR   group  length    CCHR x       */
/*                             STAR  INTVL  zero   in bytes               */
/*                                                                        */
/*    \)\{                                                                */
/*    0e      01     02     16                                            */
/*   CENT    lower  upper  CCEOF                                          */
/*   INTVL   bound  bound                                                 */
/*                                                                        */
/* Note that character values are one or two bytes outside brackets,      */
/* two bytes inside brackets.                                             */
/* Also, a subexpression followed by a star or interval (e.g. \(ab\)* or  */
/* \(ab\)\{1,2\}) will have the STAR or INTVL flag set on both the CPAR   */
/* and CENT elements.                                                     */
/*                                                                        */
/* The error numbers generated have the following meaning:                */
/*                      Note that 70 is new!!!!                           */
/*      ERROR(11)       Interval endpoint too large                       */
/*      ERROR(16)       Bad number                                        */
/*      ERROR(25)       "\digit" out of range                             */
/*      ERROR(36)       Illegal or missing delimiter                      */
/*      ERROR(41)       No remembered match string                        */
/*      ERROR(42)       \( \) imbalance                                   */
/*      ERROR(43)       Too many \(                                       */
/*      ERROR(44)       More than 2 numbers given in interval             */
/*      ERROR(45)       } expected after \                                */
/*      ERROR(46)       First number exceeds second in interval           */
/*      ERROR(49)       [] imbalance                                      */
/*      ERROR(50)       Regular expression overflow                       */
/*      ERROR(70)       Invalid endpoint in range                         */
/*      ERROR(80)       Star and interval on same expression              */
/*                                                                        */


#define _CCHR   0x04            /* normal char follows                    */
#define _CDOT   0x08            /* dot: any char...                       */
#define _CENT   0x0c            /* end group - \) - here                  */
#define _CBACK  0x10            /* \number; n follows                     */
#define _CDOL   0x14            /* end-of-line anchor ($)                 */
#define _CCEOF  0x16            /* end-of-line seen                       */
#ifdef _KJI
#define _CBRA   0x18            /* start new []; count & items follow     */
#define _CNEG   0x1c            /* start [^; count & items follow         */
#endif
#define _CLASS  0x20            /* charclass follows                      */
#define _CEQV   0x22            /* equiv class value follows              */
#define _CELEM  0x24            /* collation element follows              */
#define _CRNGE  0x26            /* range start and end chars follow       */
#ifdef _KJI
#define _CKET   0x28            /* end new brackets                       */
#endif
#define _CPAR   0x2c            /* start group - \( - next is group #     */
#ifndef _KJI
#define _CBIT	0x30		/* [bitmap] using unique collating value  */
#endif

#define _STAR   0x01            /* asterisk, i.e., 0 or more            */
#define _INTVL  0x02            /* range \{m,n\} follows                */
#define _NEG    0x04            /* bracket expr negation                */

#define _NBRA   9               /* count of groups \(..\) */

#define _PLACE(c)       *ep++ = (c >> 8), *ep++ = c
#define _GETWC(sp)      ((sp[0] << 8) | sp[1])

                                /* The following macro will return a wchar_t
                                   and place the char * pointer pas the
                                   converted single- or multibyte character
                                   It is used by _GETVAL.
                                 */
#ifdef _KJI
#define _CHNEXT(s)          (NCisshift(s[0]) ? s+=2, _NCd2(s[-2], s[-1]) : *s++)
#else
#define _CHNEXT(s)	    (*s++)
#endif

                                /* The following macro is called with a
                                   char * pointer and returns a collating
                                   value and a coluniq value. The char *
                                   is bumped to past the element (e.g.,
                                   past the "ch"). If 1-to-n, return -1
                                   and the coluniq value for the "repla-
                                   ced" character.
                                 */
#define _GETVAL(co,cu,s,p,ch)   \
         if ( ((co = ((cu = NCcoluniq(ch = _CHNEXT(s))), NCcollate(ch))) < 0) \
                          && ((co = _NLxcolu(co, (unsigned char**)&s, &p, &cu)) == -1) )
#ifndef _KJI
#define MIN_UNIQ_COLVAL		257
#define _SETBIT(c)							\
		{							\
		__delta = NCcoluniq((wchar_t)c) - MIN_UNIQ_COLVAL;	\
		*(ep+((__delta >> 3) & 0x1fff)) |= __bits[__delta & 7];	\
		}
#define _SETBITU(c)							\
		{							\
		__delta = c - MIN_UNIQ_COLVAL;				\
		*(ep+((__delta >> 3) & 0x1fff)) |= __bits[__delta & 7];	\
		}
#define _BITSET								\
		next_character = lp;					\
		_GETVAL(cvalue,uvalue,next_character,pwc,wc);		\
		__delta = uvalue - MIN_UNIQ_COLVAL;			\
		if ((*(ep+((__delta >> 3) & 0x1fff)) & __bits[__delta & 7]) == 0)
static int __delta;		/* used by _SETBIT and _BITSET		*/
static char __bits[8] = {1,2,4,8,16,32,64,128};
#endif	

/* Following variable names required by spec. */
char *loc1, *loc2;
int           circf;
int           sed, nbra;

/* Following variable names are undocumented, but required by sed. */
int      nodelim;
char *locs;
char *braslist[_NBRA];
char *braelist[_NBRA];

#include <NLctype.h>

#define __CHECK_FOR_NULL(character,eof,errornum) \
                         {   if (!(character) && (character) != (eof) ) \
                                    ERROR((errornum));                    }


/* As the "is" functions aren't functions, but macros, we cannot put    */
/* the "function" in the array below; thus another layer of indirection */
/* Wrap _NO_PROTO around these - p46680 */
#ifdef _NO_PROTO
_ALPHA(c) {return(NCisalpha(c));}
_UPPER(c) {return(NCisupper(c));}
_LOWER(c) {return(NCislower(c));}
_DIGIT(c) {return(NCisdigit(c));}
_ALNUM(c) {return(NCisalnum(c));}
_SPACE(c) {return(NCisspace(c));}
_PRINT(c) {return(NCisprint(c));}
_PUNCT(c) {return(NCispunct(c));}
_XDIGIT(c) {return(isascii(c) && isxdigit(c));}
_CNTRL(c) {return(NCiscntrl(c));}
_GRAPH(c) {return(NCisgraph(c));}
#ifdef _KJI
_JALPHA(c) {return(isjalpha(c));}
_JDIGIT(c) {return(isjdigit(c));}
_JSPACE(c) {return(isjspace(c));}
_JPUNCT(c) {return(isjpunct(c));}
_JPAREN(c) {return(isjparen(c));}
_JKANJI(c) {return(isjkanji(c));}
_JHIRA(c) {return(isjhira(c));}
_JKATA(c) {return(isjkata(c));}
_JXDIGIT(c) {return(isjxdigit(c));}
#endif
#else
extern int NCisalpha(int), NCisupper(int), NCislower(int), NCisdigit(int),
	   NCisalnum(int), NCisspace(int), NCisprint(int), NCispunct(int),
	   NCiscntrl(int), NCisgraph(int);
extern  wchar_t NCcoluniq (wchar_t);
extern int NCcollate(wchar_t);
extern short _NLxcolu(short, unsigned char **, wchar_t**, wchar_t*);

_ALPHA(int c) {return(NCisalpha(c));}
_UPPER(int c) {return(NCisupper(c));}
_LOWER(int c) {return(NCislower(c));}
_DIGIT(int c) {return(NCisdigit(c));}
_ALNUM(int c) {return(NCisalnum(c));}
_SPACE(int c) {return(NCisspace(c));}
_PRINT(int c) {return(NCisprint(c));}
_PUNCT(int c) {return(NCispunct(c));}
_XDIGIT(int c) {return(isascii(c) && isxdigit(c));}
_CNTRL(int c) {return(NCiscntrl(c));}
_GRAPH(int c) {return(NCisgraph(c));}
#ifdef _KJI
extern int isjalpha(int), isjdigit(int), isjspace(int), isjpunct(int),
	   isjparen(int), isjkanji(int), isjhira(int), isjkata(int),
	   isjxdigit(int);
_JALPHA(int c) {return(isjalpha(c));}
_JDIGIT(int c) {return(isjdigit(c));}
_JSPACE(int c) {return(isjspace(c));}
_JPUNCT(int c) {return(isjpunct(c));}
_JPAREN(int c) {return(isjparen(c));}
_JKANJI(int c) {return(isjkanji(c));}
_JHIRA(int c) {return(isjhira(c));}
_JKATA(int c) {return(isjkata(c));}
_JXDIGIT(int c) {return(isjxdigit(c));}
#endif
#endif
static struct __isarray {
        char *isstr;
#ifdef _NO_PROTO
        int (*isfunc)();
#else
        int (*isfunc)(int);
#endif
} __istab[] = {
        { "alpha", _ALPHA },
        { "upper", _UPPER },
        { "lower", _LOWER },
        { "digit", _DIGIT },
        { "alnum", _ALNUM },
        { "space", _SPACE },
        { "print", _PRINT },
        { "punct", _PUNCT },
        { "xdigit", _XDIGIT },
        { "cntrl", _CNTRL },
        { "graph", _GRAPH }

#ifdef _KJI
                                ,
        { "jalpha", _JALPHA },
        { "jdigit", _JDIGIT },
        { "jspace", _JSPACE },
        { "jpunct", _JPUNCT },
        { "jparen", _JPAREN },
        { "jkanji", _JKANJI },
        { "jhira", _JHIRA },
        { "jkata", _JKATA },
        { "jxdigit", _JXDIGIT }
#endif

#define _NISTAB (sizeof(__istab) / sizeof(struct __isarray))
};
#define _IFBUFLEN 16
        static char  __ifbuf[_IFBUFLEN];

static int      __ebra;
static unsigned int      __low;
static unsigned int      __ssize;
#ifndef _KJI
#define _BRACKET_LEN	48
#endif

#ifdef _NO_PROTO
int           advance();
static void   __getintvl();
static int    __isthere();
static int    __ecmp();
#else
int                   advance(char *lp, register char *ep);
static void   __getintvl(char *str);
static int    __isthere(char *sp, char *bp, char **next_character);
static 		__ecmp(char *a, char *b, int count);
#endif


char *
compile(char *instring,
        register char *ep,
        const char *endbuf,
        int eof)
{
        INIT    /* Dependent declarations and initializations */
        register c;
        wchar_t  wchr;
        wchar_t *p;
        char *lastep = 0;      /* addr of start of simple r-e,  */
                                /* for or-ing _INTVL or _STAR flag  */
        int cclcnt;
        char bracket[_NBRA], *bracketp;
        char *subexpr_start_location[_NBRA];
        char *ib;
        int dashfl;
        struct nextelt {
            char      Class;
            char      rangeable;
            short     cvalue;
            wchar_t     uvalue;
        } next, prev;

        int closed;
#ifndef _KJI
        int neg;		/* [bitmap] is negated */
#else
        char *cnclptr;	/* addr of _CBRA's count bytes */
#endif
        int lc;
        int i, cflg;
        unsigned int subexpr_length;

        c = GETC();
        __CHECK_FOR_NULL(c,eof,36);

        if(c == eof || c == '\n') {
                if(c == '\n') {
                                /* This apparently superfluous logic
                                 * is required by sed
                                 */
                        UNGETC(c);
                        nodelim = 1;
                }
                if(*ep == 0 && !sed)    /* WRONG  *ep uninitialized! */
                        ERROR(41);
                RETURN(ep);
        }
        bracketp = bracket;

        circf = closed = nbra = __ebra = 0;
        if(c == '^')
                circf++;
        else
                UNGETC(c);
        while(1) {
                        /* Will we overflow ep with this element?
                         * Aside from bracket lists, one r.e. element
                         * can produce no more than 3 bytes of compiled text.
                         */
            if(ep >= endbuf-3) ERROR(50);

            c = GETC();
            __CHECK_FOR_NULL(c,eof,36);

            if(c != '*' && ((c != '\\') || (PEEKC() != '{')))     /*}*/
                    lastep = ep;
            if(c == eof) {
                    *ep++ = _CCEOF;
                    RETURN(ep);
            }
            switch(c) {

        case '.':
                *ep++ = _CDOT;
                continue;

        case '\n':
                if(!sed) {
                        UNGETC(c);
                        *ep++ = _CCEOF;
                        nodelim = 1;
                        RETURN(ep);
                }
                else ERROR(36);
        case '*':
                        /* Accept * as ordinary character if first in
                         * pattern or if following \(.
                         * Undocumented, possibly POSIX-conflicting:
                         * also accept * following \).
                         */
                if(lastep == 0 || *lastep == _CPAR )       /* BDN */
                        goto defchar;

                if(*lastep == _CENT )
                  {
                  /* In a subexpression, set the STAR on CPAR '\(' as well  */
                  /* as CENT '\)'.                                           */
                  /* The pointer 'lastep' points to the _CENT and one past    */
                  /* it is the the number of that subreference.               */

                   if (*(subexpr_start_location[(int) *(lastep+1)]) & _INTVL)
                        /* Do not allow STAR and INTVL on the same code */
                        ERROR(80);

                   *(subexpr_start_location[(int) *(lastep+1)]) |=  _STAR;
                  }

                if (*lastep & _INTVL)
                    /* Do not allow STAR and INTVL on the same code */
                    ERROR(80);

                *lastep |= _STAR;


                continue;

        case '$':
                if(PEEKC() != eof && PEEKC() != '\n')
                        goto defchar;
                *ep++ = _CDOL;
                continue;

        case '[':
	/*	Bracket expressions are converted to a bitmask for single
	 *	byte languages.  This allows advance() to determine whether
	 *	a character matches by a simple bitmap test.  Internationalized
	 *	classes result in bits being set for all characters within
	 *	that class.  Multibyte languages use a list or range of
	 *	characters which must be sequentially searched for a match.
         *
         *      Support for Posix NL bracket extensions, including
         *      equivalence classes and collating symbols.
         *      Syntactic rules for dash ranges are simplified: '-' is
         *      ordinary after '[', before ']', and immediately following
         *      a dashrange '-'.
         *      Any element may appear syntactically as a dashrange endpoint,
         *      including those that turn out to be semantically illegal:
         *      noncollating char; [:class:]; start>end; or previous endpoint
         *      as starting point, e.g. a-m-z.
         */
#ifndef _KJI
		if (ep >= endbuf - _BRACKET_LEN)
			ERROR(50);
		neg = 0;
		*ep++ = _CBIT;
		if ((c=PEEKC()) == '^') {
			neg++;
			GETC();
		}
		bzero((char *)ep, _BRACKET_LEN);
#else
                if((c = PEEKC()) == '^') {
                    	*ep++ = _CBRA|_NEG;     /* Bracket-^ start  */
                    	GETC();
                }
                else
                    	*ep++ = _CBRA;		/* Bracket start, no ^  */
                cnclptr = ep;
                *ep++ = 0;              /* Space for count,   */
                *ep++ = 0;              /* filled in at ]     */
#endif
                prev.Class = 0;
                next.Class = 0;
                dashfl = 0;
                if ((c = PEEKC()) == '-' || c == ']') {
                    prev.Class = _CCHR;
                    prev.rangeable = 1;
                    prev.uvalue = c;
                    GETC();
                }

                while (1) {             /* Iterate over elements of bracket list */
                    if(ep >= endbuf-6)
                        ERROR(50);

                                /* Worst case: 6 bytes could be added to
                                 * ep in the case of  ... - ]
                                 */

                    c = GETC();
                    __CHECK_FOR_NULL(c,eof,49);

                    if (c == '\0') ERROR(49); /* Stop when NUL is found*/
                    if (c == ']') {
                        if (prev.Class != 0) {
                            UNGETC(c);
                            goto stuffp;
                        }
                        if (dashfl) {		/* Trailing dash is ordinary character */
#ifndef _KJI
				_SETBIT('-')
#else
                                *ep++ = _CCHR;
                                wchr = '-';
                                _PLACE(wchr);
#endif
                        }
                        break;
                    }
                    else if (c == '-' && !dashfl) {
                        dashfl = 1;
                        continue;
                    }
                                /* Get next element into structure
                                   next.  It may be a:

                                    _CLASS  [:class:]
                                    _CEQV    [=collating-element=]
                                    _CDOT    [=.=]
                                    _CELEM   [.xx.] (collating-symbol)
                                    _CCHR    character
                                 */
                    else if (c == '[' &&
                        ((lc=PEEKC()) == ':' || lc == '.' || lc == '=')) {
                        ib = __ifbuf;
                        GETC();
                        while ( (c = GETC()) != lc || PEEKC() != ']') {
                            __CHECK_FOR_NULL(c,eof,49);

                            if (c == '\n' || c == eof) ERROR(49);
                            *ib++ = c;
#ifdef _KJI
                            if (NCisshift(c)) *ib++ = GETC();
#endif
                            if (ib>__ifbuf+_IFBUFLEN-2)
                                ib-=2;
                                        /* ifbuf is long enough that if we
                                         * discard characters here, the contents
                                         * are already known to be invalid.
                                         */

                        }
                        *ib = '\0';
                        ib = __ifbuf;
                        GETC();         /* Advance over trailing ]      */
                        if (lc == ':') {
                            for (i = 0; i < _NISTAB; i++) {
                                if((strcmp((char *)__ifbuf,__istab[i].isstr))==0)
                                    break;
                            }
                            if (i >= _NISTAB) ERROR(49);
                            next.Class = _CLASS;
                            next.rangeable = 0;
                            next.uvalue = i;
                        }
                        else if (lc == '.') {
                            next.Class = _CELEM;
                            next.rangeable = 1;

                            _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                            if ((next.cvalue == 0) || (ib[0] != '\0'))
                                ERROR(36);
                        }
                        else {
                                        /* Equivalence class.  Special-case '.'
                                         * to mean any char with a collating value;
                                         * represent as CDOT in compiled string.
                                         */
                            if ((__ifbuf[0] == '.') && (__ifbuf[1] == '\0')){
                                next.Class = _CDOT;
                                next.rangeable = 0;
                            }
                            else {
                                next.Class = _CEQV;
                                _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                                next.rangeable = 1;
                                if ((next.cvalue == 0) || (ib[0] != '\0'))
                                    ERROR(36);
                                if (next.cvalue == next.uvalue)
                                        next.Class = _CELEM;


                            }
                        }
                    }
                    else {                      /* Ordinary character,
                                                 * including [ followed by
                                                 * anything but :=.
                                                 */
                        next.Class = _CCHR;
                        next.rangeable = 1;
#ifdef _KJI
                        if (NCisshift(c))
                             _NCdec2(c, GETC(), c);
#endif
                        next.uvalue = c;
                    }
        /* Next element has been built and placed in next.
         * Now dispose of it.                                   */
                    if (dashfl) {
                        dashfl = 0;
                        /*
                         * '-' seen, not immediately following '['.
                         * The element preceding '-' is in struct prev and
                         * the element following is in struct next.
                         * It's legal if both prev and next are collatable
                         * and prev <= next.
                         */
                        if (prev.Class == 0 ||
                                (!prev.rangeable || !next.rangeable))
                            ERROR(70);
                                        /* one end of range was char-class
                                         * or noncollating char, or 'start'
                                         * of range was really endpoint of
                                         * a preceding range, e.g. [a-m-z]
                                         */
                        prev.rangeable = 0;
                                        /* Inhibit [a-m-z]              */
                        if (prev.Class == _CCHR) {
                            ib = __ifbuf;
#ifdef _KJI
                            _NCe2(prev.uvalue, ib[0], ib[1]);
#else
			    *ib = prev.uvalue;
#endif
                            _GETVAL(prev.cvalue,prev.uvalue,ib,p,wchr);
                            if (prev.cvalue == 0)
                                ERROR(70);
                        }
                        if (next.Class == _CCHR) {
                            ib = __ifbuf;
#ifdef _KJI
                            _NCe2(next.uvalue, ib[0], ib[1]);
#else
			    *ib = next.uvalue;
#endif
                            _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                            if (next.cvalue == 0)
                                ERROR(70);
                        }
                        if (next.uvalue < prev.uvalue)
                                ERROR(70);

#ifndef _KJI
			for (i=prev.uvalue; i<=next.uvalue; i++) {
				_SETBITU(i)
			}
#else
                        *ep++ = _CRNGE;
                        if (prev.Class == _CEQV)
                            _PLACE(prev.cvalue);
                        else
                            _PLACE(prev.uvalue);
                        _PLACE(next.uvalue);
                        if (next.Class == _CEQV) {
                            *ep++ = _CEQV;
                            _PLACE(next.cvalue);
                        }
#endif
                        prev.Class = 0;
                    }
                    else {              /* not a range */
                        if (prev.Class != 0) {
                                        /* Insert class and value in ep.
                                         * If [:class:], 1 byte of value;
                                         * if [=.=], no value;
                                         * otherwise 2 bytes of value.
                                         */
        stuffp:
#ifndef _KJI
                            switch (prev.Class) {
                            case _CLASS:
				for (i=1; i<256; i++) {
				    if ((*__istab[prev.uvalue].isfunc)(i) != 0) {
					_SETBIT(i)
				    }
				}
                                break;
                            case _CEQV:
				for (i=1; i<256; i++) {
				    if (NCcollate(i) == prev.cvalue) {
					_SETBIT(i)
				    }
				}
                                break;
                            case _CELEM:
                            case _CCHR:
				_SETBIT(prev.uvalue)
                            case _CDOT:
                                break;
			    }
#else
                            *ep++ = prev.Class;
                            switch (prev.Class) {
                            case _CLASS:
                                *ep++ = _NCbot(prev.uvalue);
                                break;
                            case _CEQV:
                                _PLACE(prev.cvalue);
                                break;
                            case _CELEM:
                            case _CCHR:
                                _PLACE(prev.uvalue);
                            case _CDOT:
                                break;
                            }
#endif
                        }
                        prev=next;
                        next.Class = 0;
                    }
                }

#ifndef _KJI
		if (neg != 0) {
		    for (i=0; i<_BRACKET_LEN; i++)
		    	*ep++ = ~*ep;
		    *(ep-_BRACKET_LEN) &= 0xfe;  /* eliminate NUL */
		}
		else
		    ep += _BRACKET_LEN;
#else
                *ep++ = _CKET;          /* trailing sentinel          */
                wchr = ep-cnclptr;              /* Store [] string length     */
                *cnclptr = _NCtop(wchr);      /* at head of string            */
                *(cnclptr+1) = _NCbot(wchr);
#endif

                continue;

            case '\\':
                if ((c = GETC()) == '\0') ERROR(36);
                switch(c) {

                case '(':
                    if(nbra >= _NBRA)
                            ERROR(43);
                    *bracketp++ = nbra;
                    subexpr_start_location[nbra] = (char *) ep;
                    *ep++ = _CPAR;
                    *ep++ = nbra++;
                    *ep++ = 0;              /* Space for count,   */
                    *ep++ = 0;              /* filled in at /)     */
                    continue;

                case ')':
                    if(bracketp <= bracket )
                            ERROR(42);
                    *ep++ = _CENT;
                    *ep = *--bracketp;
                    subexpr_length =
                      (char *) ep - subexpr_start_location[*ep]  - 1;

                   if (subexpr_length > 0xffff)
                      /* Overflow, subexpr to long */
                      ERROR(50);


                   /* Now set the length of the subexpression */
                   *(subexpr_start_location[*ep] + 2)  =
                                    subexpr_length >> 8;

                   *(subexpr_start_location[*ep] + 3)  =
                                    subexpr_length & 0x00ff ;


                    ep++;
                    closed++;
                    continue;

                case '{':                                       /*}*/
                    if(lastep == 0)
                            goto defchar;

                    if(*lastep == _CENT)
                     {
                      if (*(subexpr_start_location[*(lastep+1)]) &  _STAR)
                        /* Do not allow STAR and INTVL on the same code */
                        ERROR(80);

                        /* Set INTVL on the CPAR and CENT */
                      *(subexpr_start_location[*(lastep+1)]) |=  _INTVL;
                     }

                    if (*lastep &  _STAR)
                        /* Do not allow STAR and INTVL on the same code */
                        ERROR(80);

                    *lastep |= _INTVL;

                    cflg = 0;
                    c = GETC();
            nlim:
                    i = 0;
                    do {
                            if('0' <= c && c <= '9')
                                    i = 10 * i + c - '0';
                            else
                                    ERROR(16);
                    } while(((c = GETC()) != '\\') && (c != ','));
                    if(i > RE_DUP_MAX)
                            ERROR(11);
                    *ep++ = i;
                    if(c == ',') {
                            if(cflg++)
                                    ERROR(44);
                            if((c = GETC()) == '\\')
                                    *ep++ = RE_DUP_MAX;
                            else goto nlim;              /* get 2'nd number */
                    }                           /*{*/
                    if(GETC() != '}')
                            ERROR(45);
                    if(!cflg)   /* one number */
                            *ep++ = i;
                    else if( *(ep -1)
                            < *(ep -2) || *(ep -1) == 0)
                            ERROR(46);
                    continue;

                case '\n':
                    ERROR(36);

                case 'n':
                    c = '\n';
                    goto defchar;

                default:
                    if(c >= '1' && c <= '9') {
                            if((c -= '1') >= closed)
                                    ERROR(25);
                            *ep++ = _CBACK;
                            *ep++ = c;
                            continue;
                    }
                }
/* Drop through to default to use \ to turn off special chars */
            defchar:
            default:
                lastep = ep;
                *ep++ = _CCHR;
                *ep++ = c;
#ifdef _KJI
                if (NCisshift(c))
                     *ep++ = GETC();
#endif
            }
        }
}

step(register char *p1, register char *p2)
{
        register unsigned c;

        if(circf) {
                loc1 = p1;
                return(advance(p1, p2));
        }
        /* fast check for first character */
        if(*p2 == _CCHR) {
                c = p2[1];
                do {
                        if (*p1==c)
                        if(advance(p1, p2)) {
                                loc1 = p1;
                                return(1);
                        }
#ifdef _KJI
                        if (NCisshift(*p1)) p1++;
#endif
                } while(*p1++);
                return(0);
        }
                /* regular algorithm */
        do {
                if(advance(p1, p2)) {
                        loc1 = p1;
                        return(1);
                }
#ifdef _KJI
                 if (NCisshift(*p1)) p1++;
#endif
        } while(*p1++);
        return(0);
}

advance(char *lp, register char *ep)
{
        char *curlp, *nxtep, *curwp;
        int c2, lc;
        register int c;

        char *bbeg;
        int subexpr_index;
        int ct;
        char RE_code;
#ifndef _KJI
	short cvalue;
	wchar_t uvalue;
	wchar_t *pwc, wc;
#endif

        char *next_character;   /* This is used to point to the */
                                         /* next 'character' in the 'lp' */
                                         /* string. The __isthere()      */
                                         /* routine will pass this value */
                                         /* back since it knows how big  */
                                         /* the 'character' is.          */

        while(1) {
                switch(*ep++) {

            case _CCHR:
#ifdef _KJI
                    c = *ep++;
                    if(c == *lp++)
                          if (!NCisshift(c) || *ep++ == *lp++)
				continue;
#else
		    if (*ep++ == *lp++)
			continue;
#endif
                    return(0);

            case _CDOT:
#ifdef _KJI
                    if (*lp==0)
                    	return(0);
                    else lp +=NLchrlen(lp);
                    continue;
#else
		    if (*lp++ != '\0')
			continue;
		    return(0);
#endif

            case _CDOL:
                    if(*lp == 0)
                            continue;
                    return(0);

            case _CCEOF:
                    loc2 = lp;
                    return(1);

            case _CPAR:
                    braslist[*ep++] = lp;
                    ep += 2;

                    continue;

            case _CENT:
                    braelist[*ep++] = lp;
                    continue;

            case _CPAR | _INTVL:
            case _CPAR | _STAR:
                    subexpr_index = *ep++;
                    braslist[subexpr_index] = lp;
                    nxtep = ep + _GETWC(ep);  /* Point at the INTVL range */

                    ep +=2;         /* Move past the subexpr length */
                    if (*(ep - 4) == (_CPAR | _INTVL ) )
                      {
                        /* Get the interval info */

                        __getintvl(nxtep);
                        nxtep +=2;       /* Point past the interval range */

                        /* If the __low is > 0, then the _CENT | _INTVL */
                        /* will take care of everything.                */
                        if (__low > 0 )
                              continue;
                      }

                    /* Advance() must be called from here because         */
                    /* even if the subexpression in not matched, we still */
                    /* need to continue.                                  */

                    if (advance(lp,ep) == 1)
                        return(1);         /* Matched the expression */

                    braelist[subexpr_index] = lp;
                    /* Skip past the RE for the subexpression and continue */
                    ep = nxtep;
                    if (lp != locs  || *nxtep != _CCEOF)
                        continue;

                    return(0);


            case _CBACK | _INTVL:
            case _CBACK | _STAR:
            case _CENT | _INTVL:
            case _CENT | _STAR:

                    RE_code = *(ep -1 );
                    subexpr_index = *ep++;

                    if (RE_code & _CENT )
                      /* Set the end of the expression for \) */
                       braelist[subexpr_index] = lp;

                    bbeg = braslist[subexpr_index];
                    ct = braelist[subexpr_index] - bbeg;

                    if (ct == 0)
                      {
                        /* The subexpression matched a NULL string */
                        /* If we cannot advance from here, the     */
                        /* pattern cannot be matched               */

                        if (lp != locs && advance(lp,ep))
                              return(1);

                         return(0);
                       }

                    if ( RE_code & _INTVL  )
                     {
                        /* Get the interval values */
                       __getintvl(ep); /* Point at the interval values */
                       ep +=2;             /* Move past the interval values */
                     }
                   else
                     {
                        /* For the STAR, act as if the user had done an */
                        /* expression like \{0,\}                       */

                           __low = 0;
                           __ssize = MAXINT;
                     } /* endif */


                   if (RE_code == (_CENT | _INTVL ) )
                      /* Decrement __low because we already found one */
                       __low--;

                    while (__low--)
                     {

                         if (!__ecmp(bbeg, lp, ct))
                             return(0);

                         lp += ct;
                     }

                    curlp = lp;

                    while(__ssize-- && __ecmp(bbeg,lp,ct))
                          lp += ct ;


                    if  (lp != locs)
                     {
                        while(lp >= curlp) {
                             if(advance(lp, ep)) return(1);
                             lp -= ct;
                         }
                     } /* endwhile */

                    return(0);



            case _CCHR | _INTVL:
                    c = *ep++;
#ifdef _KJI
                    if (NCisshift(c)) {
                            c2 = *ep++;
                            __getintvl(ep);
                            while(__low--) {
                                    if(*lp++ != c || *lp++ != c2)
                                            return(0);
                            }
                            curlp = lp;
                            while (__ssize-- && *lp == c && lp[1] == c2) lp += 2;
                    } else {
#endif
                            __getintvl(ep);
                            while(__low--) {
                                    if(*lp++ != c)
                                            return(0);
                            }
                            curlp = lp;
                            while (__ssize-- && *lp == c) lp++;
#ifdef _KJI
		    }
#endif
                    ep += 2;
                    goto star;

            case _CDOT | _INTVL:
                    __getintvl(ep);
                    while(__low--) {
#ifdef _KJI
                            if (NCisshift(*lp)) lp++;
#endif
                            if(*lp++ == '\0')
                                    return(0);
                    }
                    curlp = lp;
                    while(__ssize-- && *lp != '\0') {
#ifdef _KJI
                            lp += (NCisshift(*lp) ? 2 : 1);
#else
			    lp++;
#endif
                    }
                    ep += 2;
                    goto star;

            case _CBACK:
                    bbeg = braslist[*ep];
                    ct = braelist[*ep++] - bbeg;

                    if(__ecmp(bbeg, lp, ct)) {
                            lp += ct;
                            continue;
                    }
                    return(0);

            case _CDOT | _STAR:
                    curlp = lp;
                    while(*lp) lp++;
                    goto star;

            case _CCHR | _STAR:
                    curlp = lp;
                    c = *ep++;
#ifdef _KJI
                    if (NCisshift(c)){
                            c2 =  *ep++;
                            while(*lp == c && lp[1] == c2) lp += 2;
                    }else {
                            while (*lp == c) lp++;
                    }
#else
		    while (*lp++ == c);
		    --lp;
#endif
                    goto star;

#ifndef _KJI
	    case _CBIT:
		    _BITSET
			return(0);
		    ep += _BRACKET_LEN;
		    lp = next_character;
		    continue;

	    case _CBIT | _INTVL:
		    nxtep = ep + _BRACKET_LEN;
		    __getintvl(nxtep);
		    while (__low--) {
			    _BITSET
				return(0);
			    lp = next_character;
		    }
		    curlp = lp;
		    while (__ssize--) {
			    _BITSET
				break;
			    lp = next_character;
		    }
		    ep = nxtep += 2;
		    goto star;

	    case _CBIT | _STAR:
		    nxtep = ep + _BRACKET_LEN;
		    curlp = lp;
		    while (1) {
			    _BITSET
				break;
			    lp = next_character;
		    }
		    ep = nxtep;
		    goto star;
#else

            case _CBRA:
            case _CBRA | _NEG:
                    nxtep = ep + _GETWC(ep);
                    ep += 2;
                    if(!__isthere(lp, ep,&next_character)) return(0);

                    lp = next_character;
                    ep = nxtep;
                    continue;

            case _CBRA | _INTVL:
            case _CBRA | _INTVL | _NEG:
                    nxtep = ep + _GETWC(ep);
                   ep += 2;
                    __getintvl(nxtep);
                    while (__low--) {
                            if(!__isthere(lp, ep,&next_character)) return(0);
                            lp = next_character;
                    }
                    curlp = lp;
                    while(__ssize-- && __isthere(lp, ep, &next_character))
                          lp =  next_character;
                    ep = nxtep += 2;
                    goto star;

            case _CBRA | _STAR:
            case _CBRA | _STAR | _NEG:
                    nxtep = ep + _GETWC(ep);
                    ep += 2;
                    curlp = lp;
                    while(__isthere(lp, ep,&next_character))
                          lp  = next_character;

                    ep = nxtep;
                    goto star;
#endif

            star:

/* The logic of the backtracking done in this routine is based on the
 * characteristics of the SJIS code set; where a single-byte character must
 * be in the range 0x00-0xff, 0xa0-0xdf, and the first byte ("shift byte")
 * of a 2-byte character must be in the range 0x80-0x9f, 0xe0-0xfc.
 *
 * Let "N" denote a non-shift byte (ASCII or katakana), and "S" denote a
 * shift byte.  If the byte stream ends with "...S", it must end with a
 * two-byte character (otherwise we would not be at this point in the stream).
 * Otherwise, "...NSS...SSN" parses as "...N(SS)...(SS)(N)" if there are an
 * even number (including zero) of shift bytes preceding the last N, or
 * "...N(SS)...(SS)(SN)" if there are an odd number.
 * (And similarly if we reach the anchor point, curlp, instead of finding
 * an N).
 * In the worst case, this algorithm has to back up all the way to the
 * beginning, but it will only have to do so once.  (After backstepping the
 * last character, the preceding string of (SS) characters can be
 * backstepped quickly.)  Thus, we can process an entire ".*" in linear time.
 *
 * Note that this routine also works well in the NLS case, as we never
 * will find a shift byte; so it will just step back once...
 */
                    while (lp != locs) {
                            if (advance(lp, ep)) return (1);
                            if (lp <= curlp) return (0);
                            --lp;
#ifdef _KJI
                            if (NCisshift(*lp)) lp--;
                            else {
                               for (curwp = lp;
                                    curwp > curlp && NCisshift(curwp[-1]);
                                    --curwp);
                               if ((lp-curwp) & 1) --lp;
                            }
#endif
                    }
                    return (0);

                }
        }
}
                        /* this routine gets the low (or only) value into
                         * __low, and the delta to the high value into
                         * __ssize (for \{m,\}, set _ssize to max.)
                         * RE_DUP_MAX is a POSIX variable.
                         */
static void
__getintvl(char *str)
{
        __low = *str++;
        __ssize = (*str == RE_DUP_MAX)? MAXINT: *str - __low;
}

static 		__ecmp(char *a, char *b, register int count)
{
        while(count--)
                if(*a++ != *b++)
                        return(0);
        return(1);
}

#ifdef _KJI
/* This routine replaces the _ISTHERE macro; it matches the pattern         */
/* within brackets (bp) against the char in sp. It will advance in the bp   */
/* expression until a match occurs or the pattern is empty.                 */
/* The _NEG case is handled by switching the return codes.                  */

static int
__isthere(char *sp, char *bp, char **next_character)
{
                        int     c, lc;
                        short co;
                        wchar_t cu;
                        wchar_t w;
                        wchar_t *p;
                        int     ishere, nothere;
                        char *sa = sp;

                        nothere = (bp[-3] >> 2) & 1;
                        ishere = nothere ^ 1;

                        if(sp == (char *)0 || *sp == '\0' ||
                              next_character == (char **)0)
                                  return(0);

                        *next_character = sp;

                       /* Set *next_character to the next 'character' */
                       /* in the string.  This is so that when there  */
                       /* are multiple character 'characters' like LL */
                       /* the string pointer can be properly incremented. */
                       /* The value *next_character is incemented here  */
                       /* so that if 'sa' is not incremented,           */
                       /* *next_character will still point to the next  */
                       /* 'character'.                                  */

                       _GETVAL(co, cu, (*next_character), p, w);

                        if(NCisshift(*sa))
                                c = _GETWC(sa);
                        else    c = sa[0];

                        do {

                            sa = sp;
                            switch(*bp++) {

                                case _CCHR:
                                        lc = _GETWC(bp);
                                        if(lc == c) {
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CRNGE:
                                        _GETVAL(co, cu, sa, p, w);
                                        lc = _GETWC(bp);
                                        if(cu >= lc) {
                                                lc = ((bp[2] << 8) | bp[3]);

                                                if(cu <= lc) {
                                                       *next_character = sa;
                                                        return(ishere);
                                                }
                                        }
                                        bp += 4;
                                        break;

                                case _CELEM:
                                        _GETVAL(co, cu, sa, p, w);
                                        if(cu == _GETWC(bp)) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CEQV:
                                        _GETVAL(co, cu, sa, p, w);
                                        if(co == _GETWC(bp)) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CDOT:
                                        _GETVAL(co, cu, sa, p, w);
                                        if (co != 0) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        break;

                                case _CLASS:
                                        if((*__istab[*bp++].isfunc)(c)) {
                                                return(ishere);
                                        }
                                        break;

                                default:
                                        break;
                                }

                        } while( *bp != _CKET);

                        /* If the pointer (sa) has been incremented, then */
                        /* set *next_character to point to the new location */
                        if (sa != sp)
                            *next_character = sa;

                        return(nothere);
}
#endif /* _KJI */

#ifdef __cplusplus
}
#endif

#endif /*  _H_NLREGEXP */
