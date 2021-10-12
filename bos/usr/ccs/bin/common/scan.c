static char     sccsid[] = "@(#)97      1.18.1.6  src/bos/usr/ccs/bin/common/scan.c, cmdprog, bos411, 9428A410j 7/8/94 17:52:38";
/*
 * COMPONENT_NAME: (CMDPROG) scan.c
 *
 * FUNCTIONS: asmout, gchar, getmore, lxcom, lxenter, lxget, lxinit, lxmore  
 *            lxres, lxstr, lxtitle, mainp1, ungchar, yylex                   
 *
 * ORIGINS: 27 3 9 32
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 *
 * Modified: May 91 by RWaters: Added ILS changes.
 * Modified: May 91 by RWaters: Changed to parse ansi preprocessing
 * Modified: June 91 by RWaters: P26007 Now parses #pragma
 * Modified: July 91 by RWaters: P27180 reformatted
 * 
 */
/*   
 *  @OSF_COPYRIGHT@
 */

/* AIWS C compiler */

#include "mfile1.h"
#include "messages.h"
#include <ctype.h>

#include <limits.h>
#define  LONGLONG_MAX  0x7fffffffffffffffll
#define  ULONGLONG_MAX 0xffffffffffffffffull

#include <locale.h>
/* temporarily */

#include <stdlib.h>
#include <mbstr.h>
#ifndef WEOF
# define WEOF EOF               /* Just in case */
#endif  !WEOF

char    *getmem();

#ifndef ASMBUF
# define ASMBUF 50
#endif  !AMSBUF

char    asmbuf[ASMBUF];
char    *asmp;
int     asm_esc = 0; /* asm escaped used in file */
/* lexical actions */

#define A_ERR 0                 /* illegal character */
#define A_LET 1                 /* saw a letter */
#define A_DIG 2                 /* saw a digit */
#define A_1C 3                  /* return a single character */
#define A_STR 4                 /* string */
#define A_CC 5                  /* character constant */
#define A_BCD 6                 /* GCOS BCD constant */
#define A_SL 7                  /* saw a / */
#define A_DOT 8                 /* saw a . */
#define A_PL 9                  /* + */
#define A_MI 10                 /* - */
#define A_EQ 11                 /* = */
#define A_NOT 12                /* ! */
#define A_LT 13                 /* < */
#define A_GT 14                 /* > */
#define A_AND 16                /* & */
#define A_OR 17                 /* | */
#define A_WS 18                 /* whitespace (not \n) */
#define A_NL 19                 /* \n */
#define A_MUL 20                /* * */
#define A_MOD 21                /* % */
#define A_ER 22                 /* ^ */

/* character classes */

#define LEXLET 01
#define LEXDIG 02
#define LEXOCT 04
#define LEXHEX 010
#define LEXWS 020
#define LEXDOT 040

/* reserved word actions */

#define AR_TY 0                 /* type specifier word */
#define AR_RW 1                 /* simple reserved word */
#define AR_CL 2                 /* storage class word */
#define AR_S 3                  /* struct */
#define AR_U 4                  /* union */
#define AR_E 5                  /* enum */
#define AR_A 6                  /* asm */
#define AR_QU 7                 /* type qualifier word */
#define AR_GO 8                 /* goto */


static UCONSZ ranges[] = { INT_MAX, UINT_MAX, LONG_MAX, ULONG_MAX,
		           LONGLONG_MAX, ULONGLONG_MAX };
#define RNGSIZE (sizeof (ranges) / sizeof (UCONSZ))

/* text buffer */
#define LXTSZ 8192
char    yytext[LXTSZ];
char    *lxgcp;

/*****************************************************************************
* I added an option to pass source lines through to the intermediate and     *
* assembler files for debugging.  To implement that, all of the calls to     *
* getchar() and ungetc() are changed to gchar() and ungchar().  Gchar        *
* reads characters from a line buffer and calls getmore() to refill it.      *
*****************************************************************************/

int     cdebug;                 /* pass through source code */
int     ntrnodes = TREESZ;      /* number of tree nodes */
static char     *sbuf = 0;      /* pointer to begining of source buffer */
static char     *scp = "";      /* current character in sbuf[] */

int     *TypIDtab;              /* table containing sue/ptr debugger type IDs*/
int     *AryIDtab;              /* table containing array debugger type IDs  */
int     *Nxt_TypID;
int     *Nxt_AryID;

static getmore();

#define nextchar() (*scp? *scp++: getmore())
#define ungchar(ch,foo) (((ch >= 0)&&(ch < 255))? *--scp = ch: ch)

static int      MB_MAX; /* used to hold MB_CUR_MAX */

int inchar;  /* temporary input character storage */
/*
 * Handles line splicing before returning the next character - defect 64415 
 */
#define gchar() (inchar = 0, ((inchar = nextchar()) != '\\')?\
                    inchar\
                :\
                    (((inchar = nextchar()) == '\n')?\
                        (((inchar = nextchar()) != EOF)?\
                            (++lineno, inchar)\
                        :\
                            (UERROR(ALWAYS, MESSAGE(113)), EOF))\
                    :\
                        (ungchar(inchar, stdin), '\\')))

/*
 * Returns a wide character from char *scp
 */
static wchar_t
nextwchar()
{
        static wchar_t wtemp;
        char    temp;
        int     i;

        if (!scp || !*scp) {
                temp = getmore();
                if (temp == (char)EOF) return(WEOF);
                ungchar (temp, stdin);
        }
        mbtowc (&wtemp, scp, MB_MAX);

        if (mbsadvance(scp) == (char *) - 1) {
                scp++;
        } else
                scp = mbsadvance(scp);

        return( wtemp );
}

/*
 * Handles line splicing before returning next character - defect 64415
 */
static void     
ungwchar(wch, foo)
wchar_t wch;
FILE*foo;
{
        int     i;
        char    ch[100];

        wctomb( ch, wch);
        i = mblen (ch, MB_MAX);
        for (i--; i >= 0; i--) {
                *--scp = ch[i];
        }
}


static wchar_t
gwchar()
{
        int inwchar = 0;
        if ((inwchar = nextwchar()) != '\\')
                return(inwchar);
        else {
                if ((inwchar = nextwchar()) == '\n')
                        if ((inwchar = nextwchar()) != WEOF) {
                                ++lineno;
                                return(inwchar);
                        }
                        else {
                                /* unexpected EOF */
                                UERROR(ALWAYS, MESSAGE(113));
                                return(WEOF);
                        }
                else {
                        ungwchar(inwchar, stdin);
                        return('\\');
                }
        }
}


/* -------------------- mainp1 -------------------- */

/* ARGSUSED */
mainp1( int argc, char *argv[] )
{  /* control multiple files */

        register i;
        register char   *cp;
        extern int      idebug, bdebug, tdebug, edebug, xdebug;
#ifndef ddebug
        extern int      ddebug;
#endif
        extern int      NoRegisters;
        char    *release = "AIX Version 2.2.1 Enhanced";

        /* HACK HACK malloc so fits on PC */
        dimtab = (int *)getmem(ndiments * sizeof(int));
        Nxt_TypID = TypIDtab = (int *)getmem(ndiments * sizeof(int));
        Nxt_AryID = AryIDtab = TypIDtab + ndiments - 1;
        node = (NODE * )getmem(ntrnodes * sizeof(NODE));

#ifdef  XCOFF
        saved_lab = (int *)getmem(max_strings * sizeof(int));
        saved_str = (int *)getmem(max_chars * sizeof(int));
#endif  XCOFF

        setlocale(LC_ALL, "");
        MB_MAX = MB_CUR_MAX;

        offsz = caloff();
        for ( i = 1; i < argc; ++i ) {
                if ( *(cp = argv[i]) == '-' && *++cp == 'X' ) {
                        while ( *++cp ) {
                                switch ( *cp ) {

                                case 'r':
                                        fprintf( stderr, TOOLSTR(M_MSG_284, "Version: %s\n"), release );
                                        break;

                                case 'd':
#ifndef ddebug
                                        ++ddebug;
#endif
                                        break;

                                case 'i':
                                        ++idebug;
                                        break;

                                case 'b':
                                        ++bdebug;
                                        break;

                                case 't':
                                        ++tdebug;
                                        break;

                                case 'e':
                                        ++edebug;
                                        break;

                                case 'x':
                                        ++xdebug;
                                        break;

                                case 'R':
                                        ++NoRegisters;
                                        break;

                                case 'c':
                                        ++cdebug;
                                        break;
                                }
                        }
                }
        }

#ifdef  ONEPASS
        p2init( argc, argv );
#endif  ONEPASS

        if (!(stab = (struct symtab *)getmem((nstabents + 1) * sizeof(struct symtab ))))
                cerror(TOOLSTR(M_MSG_212, "stab out of space\n"));

        for ( i = 0; i < nstabents; ++i ) {
                stab[i].stype = tyalloc(TNULL);
                stab[i].psname = 0;
        }

        lineno = 1;

        InitTypes();
        InitParse();
        lxinit();
        tinit();
        mkdope();

        /* initialization of paramstk to allow it to grow as needed. */
        paramstk = (int *)malloc(paramsz * sizeof(int));

        /* initialization of protostk to allow it to grow as needed. */
        protostk = (int *)malloc(protosz * sizeof(int));

        /* dimension table initialization */

        dimtab[TNULL] = 0;
        dimtab[UNDEF] = 0;
        dimtab[TVOID] = 0;
        dimtab[CHAR] = SZCHAR;
        dimtab[INT] = SZINT;
        dimtab[SHORT] = SZSHORT;
        dimtab[LONG] = SZLONG;
        dimtab[LNGLNG] = SZLNGLNG;
        dimtab[SCHAR] = SZCHAR;
        dimtab[FLOAT] = SZFLOAT;
        dimtab[DOUBLE] = SZDOUBLE;
        dimtab[LDOUBLE] = SZLDOUBLE;
        dimtab[UCHAR] = SZCHAR;
        dimtab[USHORT] = SZSHORT;
        dimtab[UNSIGNED] = SZINT;
        dimtab[ULONG] = SZLONG;
        dimtab[ULNGLNG] = SZLNGLNG;

        /*
        ** Array dimtab continues after the above basic types.
        ** Variable curdim is set at one more than the value of last
        ** basic type defined in m_ind/manifest.h
        */
        curdim = NBTYPES;
        reached = 1;
        lintnrch = 0;

#ifdef  LINT
        OutFileBeg(LINTBOF);
#endif  LINE

#ifdef  CFLOW
        OutFileBeg(CFLOWBOF);
#endif  CFLOW

        yyparse();
        yyaccpt();

        /* Clear out leftover scoped-out externals */
        clearst();

        deftents();

#ifdef  XCOFF
        prFTN();
        unbuffer_str();
# if    IS_COMPILER
        printf("\t.extern\t._ptrgl[pr]\n");
# endif ISCOMPILER
#endif  XCOFF

        ejobcode( nerrors ? 1 : 0 );

#ifdef  LINT
        OutFileEnd(LINTEOF);
#endif  LINT

#ifdef  CFLOW
        OutFileEnd(CFLOWEOF);
#endif  CFLOW

        return(nerrors ? 1 : 0);

}


#define CSMASK 0177
#define CSSZ 128

short   lxmask[CSSZ+1];

/* -------------------- lxenter -------------------- */

lxenter( s, m )
register char   *s;
register short  m;
{
        /* enter a mask into lxmask */
        register c;
        while ( c = *s++)
                lxmask[c+1] |= m;
}


#define lxget(c,m) (lxgcp=yytext,lxmore(c,m))

/* -------------------- lxmore -------------------- */

lxmore( c, m )
register c, m;
{
        register char   *cp;

        *(cp = lxgcp) = c;
        while ( c = gchar(), lxmask[c+1] & m ) {
                if ( cp < &yytext[LXTSZ-1] ) {
                        *++cp = c;
                }
        }
        ungchar(c, stdin);
        *(lxgcp = cp + 1) = '\0';
}


struct lxdope {
        short   lxch;   /* the character */
        short   lxact;  /* the action to be performed */
        short   lxtok;  /* the token number to be returned */
        short   lxval;  /* the value to be returned */
} lxdope[] = {

        '$',    A_ERR,  0,      0,      /* illegal characters go here... */
        '_',    A_LET,  0,      0,      /* letters point here */
        '0',    A_DIG,  0,      0,      /* digits point here */
        ' ',    A_WS,   0,      0,      /* whitespace goes here */
        '\n',   A_NL,   0,      0,
        '"',    A_STR,  0,      0,      /* character string */
        '\'',   A_CC,   0,      0,      /* character constant */
        '`',    A_BCD,  0,      0,      /* GCOS BCD constant */
        '(',    A_1C,   LP,     0,
        ')',    A_1C,   RP,     0,
        '{',    A_1C,   LC,     0,
        '}',    A_1C,   RC,     0,
        '[',    A_1C,   LB,     0,
        ']',    A_1C,   RB,     0,
        '*',    A_MUL,  MUL,    MUL,
        '?',    A_1C,   QUEST,  0,
        ':',    A_1C,   COLON,  0,
        '+',    A_PL,   PLUS,   PLUS,
        '-',    A_MI,   MINUS,  MINUS,
        '/',    A_SL,   DIVOP,  DIV,
        '%',    A_MOD,  DIVOP,  MOD,
        '&',    A_AND,  AND,    AND,
        '|',    A_OR,   OR,     OR,
        '^',    A_ER,   ER,     ER,
        '!',    A_NOT,  UNOP,   NOT,
        '~',    A_1C,   UNOP,   COMPL,
        ',',    A_1C,   CM,     CM,
        ';',    A_1C,   SM,     0,
        '.',    A_DOT,  STROP,  DOT,
        '<',    A_LT,   RELOP,  LT,
        '>',    A_GT,   RELOP,  GT,
        '=',    A_EQ,   ASSIGN,         ASSIGN,
        -1,     A_1C,   0,      0,
};


struct lxdope *lxcp[CSSZ+1];

/* -------------------- lxinit -------------------- */

lxinit()
{
        register struct lxdope *p;
        register i;
        register char   *cp;

        /* set up character classes */
        if (devdebug[ANSI_MODE])
                lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", LEXLET );
        else
                lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_", LEXLET );

        lxenter( "0123456789", LEXDIG );
        lxenter( "0123456789abcdefABCDEF", LEXHEX );
        lxenter( " \t\r\b\f\v", LEXWS );
        lxenter( "01234567", LEXOCT );
        lxmask['.'+1] |= LEXDOT;

        /* make lxcp point to appropriate lxdope entry for each character */

        /* initialize error entries */

        for ( i = 0; i <= CSSZ; ++i )
                lxcp[i] = lxdope;

        /* make unique entries */

        for ( p = lxdope; ; ++p ) {
                lxcp[p->lxch+1] = p;
                if ( p->lxch < 0 )
                        break;
        }

        /* handle letters, digits, and whitespace */
        /* by convention, first, second, and third places */

        cp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        while ( *cp )
                lxcp[*cp++ + 1] = &lxdope[1];
        cp = "123456789";
        while ( *cp )
                lxcp[*cp++ + 1] = &lxdope[2];
        cp = "\t\b\r\f\013";
        while ( *cp )
                lxcp[*cp++ + 1] = &lxdope[3];
        /* first line might have title */
        lxtitle();

}


int     lxmatch;  /* character to be matched in char or string constant */

/* -------------------- lxstr -------------------- */

lxstr(ct)
{
        /* match a string or character constant, up to lxmatch */

        register c;
        register wchar_t wc;
        char    lxchar, lxchar1;
        int     wideerrs;
        register CONSZ val;
        register i;

        wideerrs = 0;
        i = 0;

morestr:
        while ( (wc = gwchar()) != lxmatch ) {
                switch ( wc ) {

                case (wchar_t)WEOF:
                        /* "unexpected EOF" */
                        UERROR( ALWAYS, MESSAGE(113) );
                        break;

                case L'\n':
                        /* "newline in string or char constant" */
                        UERROR( ALWAYS, MESSAGE(78) );
                        ++lineno;
                        lxtitle();
                        break;

                case L'\\':
                        switch ( c = gchar() ) {

                        default:
                                /* "\%c unknown escape sequence" */
                                WERROR( devdebug[ANSI_PARSE], MESSAGE(129), c );
                                val = c;
                                goto mkcc;

                        case 'a':
                                val = '\007'; /* ascii bell */
                                goto mkcc;

                        case '\'':
                                val = '\''; /* an apostrophe */
                                goto mkcc;

                        case '"':
                                val = '"'; /* a quote */
                                goto mkcc;

                        case '?':
                                val = '?'; /* a question mark */
                                goto mkcc;

                        case '\\':
                                val = '\\'; /* a backslash */
                                goto mkcc;

                        case 'x':
                        case 'X':
                                /* start collecting a hexadecimal constant */
                                val = 0;
                                {
                                        register char   *cp, lxchar;
                                        lxget( lxchar, LEXHEX);
                                        for ( cp = yytext + 1; *cp; ++cp ) {
                                                /* this code won't work for all wild character sets,
                                                   but seems ok for ascii and ebcdic */
                                                val <<= 4;
                                                if ( isdigit( *cp ) )
                                                        val += *cp - '0';
                                                else if ( isupper( *cp ) )
                                                        val += *cp - 'A' + 10;
                                                else
                                                        val += *cp - 'a' + 10;
                                        }
                                }

                                /* we might want to make locale
                                 * specific check if that's needed.
                                 */
                                /* we check is the value is representable
                                 * in an unsigned char.
                                 */
                                if ((val < 0) || (val > UCHAR_MAX))
                                        /* "constant value (0x%x) exceeds (0x%x)" */
                                        WARNING( WSTORAGE, MESSAGE(139),
                                            val, UCHAR_MAX );

                                goto mkcc;

                        case 'n':
                                val = '\n';
                                goto mkcc;

                        case 'r':
                                val = '\r';
                                goto mkcc;

                        case 'b':
                                val = '\b';
                                goto mkcc;

                        case 't':
                                val = '\t';
                                goto mkcc;

                        case 'f':
                                val = '\f';
                                goto mkcc;

                        case 'v':
                                val = '\013';
                                goto mkcc;

                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                                val = c - '0';
                                c = gchar();  /* try for 2 */
                                if ( lxmask[c+1] & LEXOCT ) {
                                        val = (val << 3) | (c - '0');
                                        c = gchar();  /* try for 3 */
                                        if ( lxmask[c+1] & LEXOCT ) {
                                                val = (val << 3) | (c - '0');
                                        } else
                                                ungchar( c , stdin);
                                } else
                                        ungchar( c , stdin);

                                /* we might want to make locale
                                 * specific check if that's needed.
                                 */

                                /* we check is the value is representable
                                 * in an unsigned char.
                                 */
                                if ((val < 0) || (val > UCHAR_MAX))
                                        /* "constant value (0x%x) exceeds (0x%x)" */
                                        WARNING( WSTORAGE, MESSAGE(139), val, UCHAR_MAX );
                                goto mkcc1;

                        }
                default:
                        val = wc;
mkcc:
                        val = CCTRANS(val);
mkcc1:
                        if ( lxmatch == '\'' ) {
                                val = CHARCAST(val);  /* it is, after all, a "character" constant */
                                makecc( val, i );
                        } else { /* stash the byte into the string */
                                if ( strflg ) {
                                        if ( ct == 0 || i < ct )
                                                putbyte( val );
                                        else if ( i == ct )
                                                /* "non-null byte ignored in string initializer" */
                                                WARNING( ALWAYS, MESSAGE(81) );
                                } else {
#ifdef  XCOFF
                                        savestr( (int) val );
#else
                                        bycode( (int)val, i );
#endif  XCOFF
                                }
                        }
                        ++i;
                        continue;
                }
                break;
        }
        /* end of string or  char constant */

        /* skip white space and white space characters */
        for (; ; ) {
                switch ( (lxcp[(lxchar=gchar())+1])->lxact ) {
                case A_SL:
                        if ((lxchar = gchar()) == '*' )
                                lxcom();
                        else {
                                ungchar((int)lxchar, stdin);
                                ungchar('/', stdin);
                                goto done;
                        }
                        continue;

                case A_STR:
                        /* if yylval.intval == 1 and wideerrs == 0
                         * then me have a "string" mixed with an initial
                         * L"string". And if we did not warn about that,
                         *  do so.
                         */
                        if (yylval.intval && !wideerrs) {
                                /* "'string literals' mixed with 'wide string literals'" */
                                WERROR( ALWAYS, MESSAGE(130) );
                                wideerrs++;
                        }
                        goto morestr;

                case A_NL:
                        ++lineno;
                        lxtitle();
                        continue;

                case A_WS:
                        continue;

                case A_LET:
                        if (lxchar == 'L') {
                                if ( (lxchar1 = gchar()) == '"' ) {
                                        /* if yylval.intval == 0 and wideerrs == 0
                                           then me have a "string" mixed with an initial
                                           L"string". And if we did not warn about that,
                                           do so.
                                        */
                                        if (!yylval.intval && !wideerrs) {
                                                /* "'string literals' mixed with 'wide string literals'" */
                                                WERROR( ALWAYS, MESSAGE(130) );
                                                wideerrs++;
                                        }
                                        goto morestr;
                                }
                                else
                                        ungchar((int)lxchar1, stdin);
                        }
                        /* fall thru */
                default :
                        ungchar((int)lxchar, stdin);
                        goto done;
                }
        }

done:

        if ( lxmatch == '"' ) {
                /* "array not large enough to store terminating null" */
                if (ct != 0 && i == ct)
                        WARNING( WSTORAGE, MESSAGE(175) );

                if ( strflg ) { /* end the string */
                        if ( ct == 0 || i < ct )
                                putbyte( 0 );  /* the null at the end */
                } else {  /* the initializer gets a null byte */
#ifdef  XCOFF
                        savestr( -1 );
                        strsize = ++i;  /* for type in buildtree */
#else
                        bycode( 0, i++);
                        bycode( -1, i );
                        strsize = i;  /* for type in buildtree */
#endif  XCOFF
                }
        } else { /* end the character constant */
                /* "empty character constant" */
                if ( i == 0 )
                        UERROR( ALWAYS, MESSAGE(36) );

                /* in a different locale, we might test:
                 *     if (i>(sizeof(wchar_t)/sizeof(char)))
                 */
                if ( i > (SZINT / SZCHAR) || ( WPORTABLE && i > 1 ) )
                        /* "too many characters in character constant" */
                        WERROR( ALWAYS, MESSAGE(107) );
        }
}


/* -------------------- lxcom -------------------- */

lxcom()
{
        register wchar_t wc;
        register c;
        /* saw a "/_*": process a comment */

#ifdef  LINT
        switch ( wc = gwchar() ) {
        case L'V':
                ungwchar(wc, stdin);
                /* Look directly at the input buffer */
                if (strncmp(scp, "VARARGS", strlen("VARARGS")) == 0) {
                        scp += strlen("VARARGS");
                        wc = gwchar();
                        if ((wc < L'0') || (wc > L'9')) {
                                ungwchar(wc, stdin);
                                lintvarg = 0;
                        } else
                                lintvarg = wc - '0';
                }
                break;

        case L'L':
                ungwchar(wc, stdin);
                /* Look directly at the input buffer */
                if (strncmp(scp, "LINTLIBRARY", strlen("LINTLIBRARY")) == 0)
                        lintlib = 1;
                else if ( strncmp( scp, "LINTSTDLIB", strlen("LINTSTDLIB")) == 0) {
                        lintrsvd = 1;
                        lintused = 1;
                        lintlib = 1;
                }
                break;

        case L'A':
                ungwchar(wc, stdin);
                /* Look directly at the input buffer */
                if ( strncmp( scp, "ARGSUSED", strlen("ARGSUSED" )) == 0)
                        lintargu = 1;
                break;

        case L'N':
                ungwchar(wc, stdin);
                /* Look directly at the input buffer */
                if ( strncmp( scp, "NOTREACHED", strlen("NOTREACHED" )) == 0)
                        lintnrch = 1;
                else if ( strncmp( scp, "NOTUSED", strlen("NOTUSED")) == 0) {
                        lintused = 1;
                        lintlib = 1;
                } else if ( strncmp( scp, "NOTDEFINED", strlen("NOTDEFINED")) == 0) {
                        lintdefd = 1;
                }
                break;

        default:
                ungwchar(wc, stdin);
                break;
        }
#endif  LINT

        for (; ; ) {
                wc = gwchar();
                switch ( wc ) {

                case (wchar_t)WEOF:
                        /* "unexpected EOF"  */
                        UERROR( ALWAYS, MESSAGE(113) );
                        return;

                case L'\n':
                        ++lineno;
                        continue;

                case L'/':
                        if ( (wc = gwchar()) == L'*' )
                                /* "nested comments not supported" */
                                WARNING( ALWAYS, MESSAGE(127) );
                                /* ignore text until we reach the first
                                   '*' followed immediately by a '/'.
                                 */
                        else
                                ungwchar(wc, stdin);
                        continue;

                default:
                        continue;

                case L'*':
                        if ( (wc = gchar()) == L'/' ) {
                                return;
                        } else
                                ungwchar( wc , stdin);
                        continue;
                }
        }
}

static int islong (int intmode) {
   /* intmode: 0 =  hex or octal, 1 = decimal
      so that I can get the type promotion right for ansi-c */
        /* for unsuffixed constant, assume INT to be our basic type (start = 0)
	   - normal promotion rules apply. */
	unsigned i, curr = 0, maxSigned;
        UCONSZ nlastcon;
        unsigned lcount=0;
	
        /* setup limits: ANSI does not allow anything past unsigned long */
	if (devdebug[ANSI_MODE]) {
            unsigned long lcon = (unsigned long) lastcon;
            nlastcon = lcon;        /* nlastcon <= ranges[maxSigned+1] */
	    maxSigned = 2;
	    }
	else {
            nlastcon = (UCONSZ) lastcon;
	    maxSigned = 4;
	    }
  
	/* The following loop scans the suffix and sets up the current range */
	for (i=0; i<3; ++i) {  /* max. 3 characters allowed */
            int lxchar = gchar();
            int c = tolower(lxchar);
	    if (c == 'u') {
	      if USUFFIX(curr) {  /* if already unsigned */
	        ungchar(lxchar, stdin);  /* put it back */
		break;
	      }
	      ++curr;  /* not yet unsigned - make it so */
	    }
	    else if (c == 'l') {
	      if (curr >= maxSigned) {  /* if too long */
	        ungchar(lxchar, stdin);  /* put it back */
		break;
	      }
	      curr += 2;  /* not too long - lengthen it */
              lcount++;
	    }
	    else {  /* some other char - suffix is complete */
	        ungchar(lxchar, stdin);  /* put it back */
		break;
	    }
	  }

        if (lcount == 2)
          /* "non ansi integer constant suffix ll" */
          WARNING(WANSI, MESSAGE(105));
 
	/* This is where we do any promotions that may be required */
        if (!curr && devdebug[PROMOTION]) { /* fix: ALL promotions, including u -> ul */
	  if (nlastcon > ranges[maxSigned])   /* if > largest unsigned range */
	    curr = maxSigned + 1;       /* promote to largest possible range */
	  else {                       /* find the right range to promote to */
	    /* if oct or hex and suffix not unsigned,
	       use both signed and unsigned limits (step = 1)
	       - otherwise use signed or unsigned (step = 2) */
	    unsigned step = (intmode || USUFFIX(curr)) ? 2 : 1;
	    while (nlastcon > ranges[curr])   /* implies: curr < maxSigned */
            {
	      curr += step;
            }
	  }
	}
 
	/* we know what type we want - set the type and do the conversions */
        switch (curr) { /* curr always <= 5 */
	    case 0:
	        yylval.intval = INT;
	        CONVtoINT(lastcon);
	        break;
	    case 1:
	        yylval.intval = UNSIGNED;
	        CONVtoUINT(lastcon);
	        break;
            case 2:
	        yylval.intval = LONG;
                CONVtoLONG(lastcon);
	        break;
            case 3:
	        yylval.intval = ULONG;
                CONVtoULONG(lastcon);
	        break;
            case 4:
	        yylval.intval = LNGLNG;
                CONVtoLNGLNG(lastcon);
	        break;
            case 5:
	        yylval.intval = ULNGLNG;
	        break;
	} /* switch */

	return ICON;
}

static int octdecimal (register int lxchar) {
        register char   *cp;
        ungchar(lxchar, stdin);
	if (yytext[0] == '0') {         /* convert in octal */
	    for (cp = yytext + 1; *cp; ++cp) {
	        if (*cp > '7')          /* bad octal digit */
	          UERROR( ALWAYS, MESSAGE(124), *cp );
	        else {
		  lastcon <<= 3;
	          lastcon += *cp - '0';
		}
	    }
            return islong (0);	        /* 0 = octal integer constant */
        }

        for (cp = yytext; *cp; ++cp) {  /* convert in decimal */
	    /* don't let lastcon overflow needlessly */
	    lastcon *= (CONSZ)10;
	    lastcon += (CONSZ)(*cp - '0');
	}
        return islong (1);	        /* 1 = decimal integer constant */
}

/* -------------------- yylex -------------------- */
yylex()
{
        int     float_col = 0;

        register int lxchar;
        register struct lxdope *p;
        register struct symtab *sp;
        int     id;
        int     chrcnt = 0;             /* char. counter for ambiguous unary ops */
        char    tempchar[80];           /* temp chars for ambig. unary ops */

        for (; ; ) {

                p = lxcp[(lxchar=gchar())+1];
                switch ( p->lxact ) {

                case A_MUL:
                        /* * *= are valid, '*' '/' is a warning */
                        switch (lxchar = gchar()) {
                        case '=':
                                return(yylval.intval = ASG_MUL);

                        case '/':
                                /* "*_/ found outside of a comment context" */
                                WERROR( ALWAYS, MESSAGE(128) );

                        default:
                                goto onechar;
                        }

                case A_MOD:
                        /* % %= are valid */
                        if ( (lxchar = gchar()) == '=')
                                return(yylval.intval = ASG_MOD);
                        goto onechar;

                case A_ER :
                        /* ^ ^= are valid */
                        if ( (lxchar = gchar()) == '=')
                                return(yylval.intval = ASG_ER);
                        /* goto onechar; */

onechar:
                        ungchar( lxchar , stdin);

                case A_1C:
                        /* eat up a single character, and return an opcode */
                        yylval.intval = p->lxval;
                        return( p->lxtok );

                case A_ERR:
                        /* "illegal character: %03o (octal)" */
                        UERROR( ALWAYS, MESSAGE(51), lxchar );
                        break;

                case A_LET:
                        /* collect an identifier, check for reserved word, and return */
                        lxget( lxchar, LEXLET | LEXDIG );
                        /* check if the collected string is "L" and that the
                           next char is a "'" or a '"'.
                        */
                        if (yytext[0] == 'L' && yytext[1] == '\0')
                                switch (lxchar = gchar()) {
                                case '\'': /* char constant */
                                        lxmatch = '\'';
                                        lastcon = 0;
                                        lxstr(0);
                                        yylval.intval = USHORT;
                                        return( ICON );

                                case '\"': /* string constant */
                                        yylval.intval = USHORT; /* fake wide strings */
                                        lxmatch = '"';
                                        return( WSTRING );

                                default: /* neither */
                                        ungchar(lxchar, stdin);
                                }

                        /* check "keywords" : lxres is a misnomer */
                        if ( (lxchar = lxres()) > 0 )
                                return( lxchar ); /* reserved word */

                        if ( lxchar == 0 )
                                continue;

                        id = lookup( yytext,
                                /* tag name for struct/union/enum */
                                (stwart & TAGNAME) ? STAG : 
                                /* member name for struct/union */
                                (stwart & (INSTRUCT | INUNION | FUNNYNAME)) ? SMOS : 
                                /* label name */
                                (stwart & LABNAME) ? SLABEL : 
                                /* regular identifier */
                                0 );

                        sp = &stab[id];
                        if ( sp->sclass == TYPEDEF && !SeenType() && !stwart ) {
#ifdef  CXREF
                                CXRefName(id, lineno);
#endif  CXREF
                                stwart = instruct;
                                yylval.nodep = mkty( sp->stype );
                                sp->suse = -lineno;
                                return( TYDEF );
                        }
                        stwart = (stwart & SEENAME) ? instruct : 0;
                        yylval.intval = id;
                        return( NAME );

                case A_DIG:
                        /* collect a digit string, then look at last one... */
                        {
                                CONSZ maxint;

                                /* float_col is needed to disambiguate the string
                                   123l to mean long int rather than long double.
                                   lxget (directly below) reads the '123' and then
                                   the switch reads the 'l or L' and goes directly
                                   into the middle of the floating point collection
                                   code. So we to test whether we're looking
                                   at the 'l or L' legally => if float_col == 1.
                                   float_col == 1 only after reading an 'e or E or .'
                                   BRAIN DEATH...sigh
                                */
                                float_col = 0;

                                lastcon = 0;
                                lxget( lxchar, LEXDIG );
                                switch ( lxchar = gchar() ) {

                                case 'x':
                                case 'X':
                                        lxmore( 'x', LEXHEX );
                                        if ( yytext[0] != '0' || yytext[1] != 'x' || strlen(yytext) == 2 )
                                                /* "illegal hex constant"  */
                                                UERROR( ALWAYS, MESSAGE(59) );
                                        /* convert the value */
                                        {
                                                register char   *cp;
                                                for ( cp = yytext + 2; *cp; ++cp ) {
                                                        /* this code won't work for all wild character sets,
                                                   but seems ok for ascii and ebcdic */
                                                        lastcon <<= 4;
                                                        if ( isdigit( *cp ) )
                                                                lastcon += *cp - '0';
                                                        else if ( isupper( *cp ) )
                                                                lastcon += *cp - 'A' + 10;
                                                        else
                                                                lastcon += *cp - 'a' + 10;
                                                }
                                        }

                                        /* 0 =  hex or octal so that I can get the
                                                type promotion right for ansi-c
                                        */
                                        return (islong (0));

                                case '.':
                                        lxmore( lxchar, LEXDIG );
getfp:
                                        float_col = 1; /* collecting a float */
                                        if ( (lxchar = gchar()) == 'e' || lxchar == 'E' ) { /* exponent */

                                        case 'e':
                                        case 'E':
                                                float_col = 1; /* collecting a float */
                                                if ( (lxchar = gchar()) == '+' || lxchar == '-' ) {
                                                        *lxgcp++ = 'e';
                                                        *lxgcp++ = lxchar;
                                                } else {
                                                        ungchar(lxchar, stdin);
                                                        *lxgcp++ = 'e';
                                                }
                                                lxchar = gchar();
                                                if (!isdigit(lxchar)) {
                                                        /* bad floating number */
                                                        UERROR( ALWAYS, MESSAGE(189) );

                                                }
                                                lxmore( lxchar, LEXDIG );
                                                /* now have the whole thing... */
                                        } else {  /* no exponent */
                                                ungchar( lxchar , stdin);
                                        }
                                        if ( (lxchar = gchar()) == 'f' || lxchar == 'F' ) {
                                        case 'f':
                                        case 'F':
                                                /* is a real float? if not
                                                   it can only be octal or decimal.
                                                 */
                                                if (!float_col)
                                                        return octdecimal(lxchar);
                                                yylval.intval = FLOAT;
                                                return( isitfloat( yytext ) );
                                        } else if (lxchar == 'l' || lxchar == 'L') {
                                        case 'l':
                                        case 'L':
                                                /* is a real float? if not
                                                   it can only be octal or decimal.
                                                 */
                                                if (!float_col)
                                                        return octdecimal(lxchar);
                                                yylval.intval = LDOUBLE;
                                                return( isitfloat( yytext ) );
                                        } else {
                                                ungchar( lxchar , stdin);
                                        }

                                        yylval.intval = DOUBLE;
                                        return( isitfloat( yytext ) );

                                default:
                                        return octdecimal (lxchar);
                                }
                        } /* end of case A_DIG */

                case A_DOT:
                        /* look for a dot:
                         *    if followed by a digit, floating point
                         *    if followed by two more dots, ellipsis
                         */
                        lxchar = gchar();
                        if ( lxmask[lxchar+1] & LEXDIG ) {
                                ungchar(lxchar, stdin);
                                lxget( '.', LEXDIG );
                                goto getfp;
                        }

                        if ( lxmask[lxchar+1] & LEXDOT ) {
                                if ( lxmask[(lxchar=gchar())+1] & LEXDOT ) {
                                        return( ELLIPSIS );
                                } else {
                                        /* saw two dots but not a third,
                                         * return the last character seen
                                         * and reset lxchar to the dot we
                                         * did see
                                         */
                                        ungchar(lxchar, stdin);
                                        lxchar = '.';
                                }
                        }

                        stwart = FUNNYNAME;
                        goto onechar;

                case A_STR:
                        /* string constant */
                        yylval.intval = 0; /* fake the regular type string type */
                        lxmatch = '"';
                        return( STRING );

                case A_CC:
                        /* character constant */
                        lxmatch = '\'';
                        lastcon = 0;
                        lxstr(0);
                        yylval.intval = UCHAR;
                        return( ICON );

                case A_BCD:
                        {
                                register i;
                                int     j;
                                for ( i = 0; i < LXTSZ; ++i ) {
                                        if ( ( j = gchar() ) == '`' )
                                                break;
                                        if ( j == '\n' ) {
                                                /* "newline in BCD constant" */
                                                UERROR( ALWAYS, MESSAGE(77) );
                                                break;
                                        }
                                        yytext[i] = j;
                                }
                                yytext[i] = '\0';
                                /* "BCD constant exceeds 6 characters" */
                                if ( i > 6 )
                                        UERROR( ALWAYS, MESSAGE(10) );
                                /* "gcos BCD constant illegal" */
                                UERROR( ALWAYS, MESSAGE(48) );
                                yylval.intval = 0;  /* not long */
                                return( ICON );
                        }

                case A_SL:
                        /* / "/_*" /= are valid*/
                        switch (lxchar = gchar()) {
                        case '*':
                                lxcom();
                                /* continue yylex main for loop */
                                continue;

                        case '=':
                                return(yylval.intval = ASG_DIV);

                        default:
                                goto onechar;
                        }

                case A_WS:
                        continue;

                case A_NL:
                        ++lineno;
                        lxtitle();
                        continue;

                case A_NOT:
                        /* ! */
                        if ( (lxchar = gchar()) != '=' )
                                goto onechar;
                        yylval.intval = NE;
                        return( EQUOP );

                case A_MI:
                        /* - -> -= -- are valid */
                        switch (lxchar = gchar()) {
                        case '-':
                                yylval.intval = DECR;
                                return( INCOP );

                        case '>':
                                stwart = FUNNYNAME;
                                yylval.intval = STREF;
                                return( STROP );

                        case '=':
                                return(yylval.intval = ASG_MINUS);

                        default :
                                goto onechar;
                        }

                case A_PL:
                        /* + ++ += */
                        switch (lxchar = gchar()) {
                        case '+':
                                yylval.intval = INCR;
                                return( INCOP );

                        case '=':
                                return(yylval.intval = ASG_PLUS);

                        default :
                                goto onechar;
                        }

                case A_AND:
                        /* & && &= are valid */
                        switch (lxchar = gchar()) {
                        case '&':
                                return( yylval.intval = ANDAND );

                        case '=':
                                return( yylval.intval = ASG_AND );

                        default :
                                goto onechar;
                        }

                case A_OR:
                        /* | || |= are valid */
                        switch (lxchar = gchar()) {
                        case '|':
                                return( yylval.intval = OROR );

                        case '=':
                                return( yylval.intval = ASG_OR );

                        default :
                                goto onechar;
                        }

                case A_LT:
                        /* < << <= <<= are valid */
                        if ( (lxchar = gchar()) == '<' ) {
                                if ((lxchar = gchar()) == '=')
                                        return(yylval.intval = ASG_LS);
                                else {
                                        ungchar(lxchar, stdin);
                                        yylval.intval = LS;
                                        return( SHIFTOP );
                                }
                        }
                        if ( lxchar != '=' )
                                goto onechar;
                        yylval.intval = LE;
                        return( RELOP );

                case A_GT:
                        /* > */
                        if ( (lxchar = gchar()) == '>' ) {
                                if ((lxchar = gchar()) == '=')
                                        return(yylval.intval = ASG_RS);
                                else {
                                        ungchar(lxchar, stdin);
                                        yylval.intval = RS;
                                        return(SHIFTOP );
                                }
                        }
                        if ( lxchar != '=' )
                                goto onechar;
                        yylval.intval = GE;
                        return( RELOP );

                case A_EQ:
                        /* = */
                        switch ( lxchar = gchar() ) {

                        case '=':
                                yylval.intval = EQ;
                                return( EQUOP );

                        case '+':
                                yylval.intval = ASG_PLUS;
                                goto warn;

                        case '-':
                                yylval.intval = ASG_MINUS;
                                /* goto warn; */

warn:
                                while ( lxmask[ (lxchar=gchar())+1] & LEXWS) {
                                        /* save any white spaces in temp array to restore later */
                                        tempchar[chrcnt++] = lxchar;
                                }
                                if ( (lxmask[lxchar] & (LEXLET | LEXDIG | LEXDOT)) || 
                                    (lxchar == '-') || (lxchar == '+') || 
                                    (lxchar == '&') || (lxchar == '(') || 
                                    (lxchar == '*') ) {
                                        /* "ambiguous assignment for non-ansi compilers" */
                                        WARNING( WPORTABLE, MESSAGE(12) );
                                }
                                /* restore the last char read by the while*/
                                ungchar( lxchar , stdin);  /* restore meaningful char */
                                while (chrcnt--)  /* restore any white spaces */
                                        ungchar( (int)tempchar[chrcnt], stdin);

                                /* restore the op character */
                                lxchar = *(scp - 1);
                                chrcnt = 0;
                                break;

                        case '*':
                                yylval.intval = ASG_MUL;
                                goto warn;

                        case '/':
                                yylval.intval = ASG_DIV;
                                goto warn;

                        case '%':
                                yylval.intval = ASG_MOD;
                                goto warn;

                        case '&':
                                yylval.intval = ASG_AND;
                                goto warn;

                        case '|':
                                yylval.intval = ASG_OR;
                                goto warn;

                        case '^':
                                yylval.intval = ASG_ER;
                                goto warn;

                        case '<':
                                if ( (lxchar = gchar()) != '<' ) {
                                        /* "=<%c illegal" */
                                        UERROR( ALWAYS, MESSAGE(8), lxchar );
                                }
                                ungchar(lxchar, stdin);
                                yylval.intval = ASG_LS;
                                goto warn;

                        case '>':
                                if ( (lxchar = gchar()) != '>' ) {
                                        /* "=>%c illegal" */
                                        UERROR( ALWAYS, MESSAGE(9), lxchar );
                                }
                                ungchar(lxchar, stdin);
                                yylval.intval = ASG_RS;
                                goto warn;

                        default:
                                goto onechar;

                        }

                        goto onechar;

                default:
                        cerror(TOOLSTR(M_MSG_213, "yylex error, character %03o (octal)"), lxchar );

                }

                /* ordinarily, repeat here... */
                cerror(TOOLSTR(M_MSG_214, "out of switch in yylex" ));
        }
}

struct lxrdope {
        /* dope for reserved, in alphabetical order */

        char    *lxrch; /* name of reserved word */
        short   lxract; /* reserved word action */
        short   lxrval; /* value to be returned */
} lxrdope[] = {
        /*****  "asm",          AR_A,   0,      *********removed********/
        "auto",                 AR_CL,  AUTO,
        "break",        AR_RW,  BREAK,
        "char",                 AR_TY,  CHAR,
        "case",                 AR_RW,  CASE,
        "continue",     AR_RW,  CONTINUE,
        "const",        AR_QU,  CONST,
        "double",       AR_TY,  DOUBLE,
        "default",      AR_RW,  DEFAULT,
        "do",           AR_RW,  DO,
        "extern",       AR_CL,  EXTERN,
        "else",                 AR_RW,  ELSE,
        "enum",                 AR_E,   ENUM,
        "for",          AR_RW,  FOR,
        "float",        AR_TY,  FLOAT,
        /*****  "fortran",      AR_CL,  FORTRAN,        ****removed*******/
        "goto",                 AR_GO,  GOTO,
        "if",           AR_RW,  IF,
        "int",          AR_TY,  INT,
        "long",                 AR_TY,  LONG,
        "return",       AR_RW,  RETURN,
        "register",     AR_CL,  REGISTER,
        "switch",       AR_RW,  SWITCH,
        "struct",       AR_S,   0,
        "sizeof",       AR_RW,  SIZEOF,
        "signed",       AR_TY,  SIGNED,
        "short",        AR_TY,  SHORT,
        "static",       AR_CL,  STATIC,
        "typedef",      AR_CL,  TYPEDEF,
        "unsigned",     AR_TY,  UNSIGNED,
        "union",        AR_U,   0,
        "void",                 AR_TY,  TVOID,
        "volatile",     AR_QU,  VOLATILE,
        "while",        AR_RW,  WHILE,
        "",             0,      0,      /* to stop the search */
};


/* -------------------- lxres -------------------- */

lxres()
{
        /* check to see of yytext is reserved; if so,
         * do the appropriate action and return 
         * otherwise, return -1 */

        register c, ch;
        register struct lxrdope *p;

        ch = yytext[0];

        if ( !islower(ch) )
                return( -1 );

        switch ( ch ) {

        case 'a':
                c = 0;
                break;
        case 'b':
                c = 1;
                break;
        case 'c':
                c = 2;
                break;
        case 'd':
                c = 6;
                break;
        case 'e':
                c = 9;
                break;
        case 'f':
                c = 12;
                break;
        case 'g':
                c = 14;
                break;
        case 'i':
                c = 15;
                break;
        case 'l':
                c = 17;
                break;
        case 'r':
                c = 18;
                break;
        case 's':
                c = 20;
                break;
        case 't':
                c = 26;
                break;
        case 'u':
                c = 27;
                break;
        case 'v':
                c = 29;
                break;
        case 'w':
                c = 31;
                break;

        default:
                return( -1 );
        }

        for ( p = lxrdope + c; p->lxrch[0] == ch; ++p ) {
                if ( !strcmp( yytext, p->lxrch ) ) { /* match */
                        switch ( p->lxract ) {

                        case AR_TY:
                                /* type specifier word */
                                stwart = instruct;
                                yylval.nodep = mkty(tyalloc((TWORD)p->lxrval));
                                return( TYPE );

                        case AR_RW:
                                /* ordinary reserved word */
                                return( yylval.intval = p->lxrval );

                        case AR_CL:
                                /* class word */
                                yylval.intval = p->lxrval;
                                return( CLASS );

                        case AR_S:
                                /* struct */
                                stwart = INSTRUCT | SEENAME | TAGNAME;
                                yylval.intval = INSTRUCT;
                                return( STRUCT );

                        case AR_U:
                                /* union */
                                stwart = INUNION | SEENAME | TAGNAME;
                                yylval.intval = INUNION;
                                return( STRUCT );

                        case AR_E:
                                /* enums */
                                stwart = SEENAME | TAGNAME;
                                return( yylval.intval = ENUM );

                        case AR_GO:
                                /* goto */
                                stwart = SEENAME | LABNAME;
                                return( yylval.intval = GOTO );

                        case AR_A:
                                /* asm */
                                asm_esc = 1; /* warn the world! */
                                asmp = asmbuf;
                                lxget( ' ', LEXWS );
                                if ( gchar() != '(' ) {
                                        /* "bad asm construction" */
                                        UERROR( ALWAYS, MESSAGE(16) );
                                        return( 0 );
                                }

                                lxget( ' ', LEXWS );
                                if ( gchar() != '"' ) {
                                        /* "bad asm construction" */
                                        UERROR( ALWAYS, MESSAGE(16) );
                                        return( 0 );
                                }

                                while ( (c = gchar()) != '"' ) {
                                        if ( c == '\n' || c == EOF ) {
                                                /* "bad asm construction" */
                                                UERROR( ALWAYS, MESSAGE(16) );
                                                return( 0 );
                                        }

                                        *asmp++ = c;
                                        if ( asmp >= &asmbuf[ASMBUF-1] ) {
                                                UERROR( ALWAYS, "asm > %d chars", ASMBUF );
                                        }
                                }
                                lxget( ' ', LEXWS );
                                if ( gchar() != ')' ) {
                                        /* "bad asm construction" */
                                        UERROR( ALWAYS, MESSAGE(16) );
                                        return( 0 );
                                }

                                *asmp++ = '\0';
                                return( ASM );

                        case AR_QU:
                                /* type qualifier word */
                                yylval.intval = p->lxrval;
                                return( QUAL );

                        default:
                                cerror(TOOLSTR(M_MSG_215, "bad AR_?? action" ));
                        }
                }
        }
        return( -1 );
}


/* -------------------- lxtitle -------------------- */

lxtitle()
{
        /* called after a newline; set linenumber and file name */

        register c, val;
        register char   *cp;
        char    *p, *strip();

        for (; ; ) {  /* might be several such lines in a row */
                if ( (c = gchar()) != '#' ) {
                        if ( c != EOF )
                                ungchar(c, stdin);
                        return;
                }

                /* skip white space */
                lxget( ' ', LEXWS );

                /*
                ** If this is a #line directive, just skip the "line" part
                ** If this is a #pragma directive, ignore the entire line
                ** Otherwise unget the contents of yytext
                */
                lxget(' ', LEXLET);
                if (strcmp(yytext, " line") != 0) {
                        /* Skip over pragma directives entirely */
                        if (strcmp(yytext, " pragma") == 0) {
                                *scp = 0;               /* done with this line */
                                lineno++;
                                continue;
                        }
                        /* Apparently this isn't an ansi preprocessor. */
                        for (cp = yytext; *cp; cp++)
                                ;
                        for (cp--; cp >= yytext; cp--)
                                ungchar(*cp, stdin);
                }

                /* skip white space */
                lxget( ' ', LEXWS );

                /* get the line number */
                val = 0;
                for ( c = gchar(); isdigit(c); c = gchar() ) {
                        val = val * 10 + c - '0';
                }
                ungchar( c, stdin );
                lineno = val;

                /* skip white space */
                lxget( ' ', LEXWS );

                /* get the file name */
                if ( (c = gchar()) != '\n' ) {
                        ungchar(c, stdin);
                        /* Look directly at the input buffer */
                        strcpy (ftitle, scp);
                        ftitle[ strlen(ftitle)-1] = 0;  /* remove <CR> */
                        *scp = 0;                       /* done with this line */

#if     defined (LINT) || defined (CFLOW)
                        p = strip(ftitle);

                        /* Get include filename. */
                        ifname = getmem(strlen(p) + 1);
                        strcpy(ifname, p);
#endif  LINT || CFLOW

#ifdef  CXREF
                        printf("%s\n", ftitle);
#endif  CXREF
                }
        }
}


#ifndef MYASMOUT
/* -------------------- asmout -------------------- */

/* write out asm string
 * this is a null function for lint
 */
asmout()
{
# if    IS_COMPILER
#  ifndef       ONEPASS
        putchar( IASM );
#  endif        ONEPASS
        printf( "%s\n", asmbuf );
# endif IS_COMPILER
}


#endif  !MYASMOUT

/*****************************************************************************
* getmore: read a line from the input, echo it if appropriate, and pass      *
* a character back                                                           *
*****************************************************************************/

/* -------------------- getmore -------------------- */

#define INBUFSZ 8192
static
getmore()
{
        register char   *cp;

        if (!sbuf)
                sbuf = getmem(INBUFSZ);
        if (feof(stdin) || fgets(sbuf, INBUFSZ - 1, stdin) == NULL) {
                return EOF;
        }
        if (cdebug) {
                printf("#%s", sbuf);
                /* check for partial line */
                for (cp = sbuf; *cp; cp++)
                        ;
                if (*--cp != '\n')
                        printf("...\n");
        }
        scp = sbuf;
        return * scp++;
}

