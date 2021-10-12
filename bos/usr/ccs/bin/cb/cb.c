static char     sccsid[] = "@(#)34      1.11.2.5  src/bos/usr/ccs/bin/cb/cb.c, cmdprog, bos411, 9428A410j 6/10/94 12:05:04";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, checkif, clearif, comment, copy, getch, getnext, getnl,
 *            gotdo, gotelse, gotif, gotop, gotstruct, gottype, keep, lookup,
 *            outs, ptabs, putch, putspace, puttmp, resetdo, unget, work,
 *            passmbc, widthmbc
 *
 * ORIGINS: 3 10 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_COPYRIGHT@
 */
/*static char rcsid[]=" RCSfile: cb, v Revision: 2.4 (OSF) Date: 90/10/07 17:36:32 "; */

/*-
 * MF:  91/05/03
 *      Support has been provided for multibyte code pages.  However, this
 *      support is limited to code pages which do NOT contain bytes in the
 *      unique code point range (0x00-0x3F) as the (N+1) byte of multibyte
 *      character.
 */
/*
 * RW: 91/05/30 A19419
 *      cb discarded comments and preprocessor directives
 *              when it was expecting an open curly
 *      cb forgot that underscores and dollar signs may be part of an
 *              identifier (the "isletter" macro does this)
 *      added CDEBUG routines
 *
 * RW: 91/06/26 P26005
 *      always start a preprocessor directive on a new line 
 */
/* PB: 92/02/05 A24146
 *      cb moved the open curly before preprocessor directives when using
 *              using the -s option.
 *      cb formatted structure initialization incorrectly as it looked for
 *              two open curly brackets to start the initialization and two
 *              close curly brackets to end the initialization.  The start
 *              is now determined by an equal sign followed by an open curly
 *              and the end by a close curly followed by a semi-colon.
*/


#define _ILS_MACROS    /* 139729 - use macros for better performance */
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <sys/localedef.h>
#include <nl_types.h>
#include <ctype.h>
#include <stdlib.h>
#include "cb_msg.h"

int     cb_mb_cur_max;                          /* local copy of MB_CUR_MAX */

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_CB, Num, Str)

/* character type flags */

#define CB_O      0200

#define isop(c)         ((c<CB_O)&&((_optable_+1)[c]&CB_O))

char    _optable_[] = {
        0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     CB_O,  0,     0,     0,     CB_O,  CB_O,  0,
        0,     0,     CB_O,  CB_O,  0,     CB_O,  0,     CB_O,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     CB_O,  CB_O,  CB_O,  0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     CB_O,  0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     CB_O,  0,     0,     0
};


#define IF      1
#define ELSE    2
#define CASE    3
#define TYPE    4
#define DO      5
#define STRUCT  6
#define OTHER   7

#define ALWAYS  01
#define NEVER   02
#define SOMETIMES       04

#define YES     1
#define NO      0

#define KEYWORD 1
#define DATADEF 2

/*A24146 - Initialization level for structures and arrays.*/
#define INITNONE -1 /*No initialization started.*/
#define INITZERO  0 /*Initialization started (ie: found "= {").*/
#define INITONE   1 /*First level of curly brackets.*/
#define INITTWO   2 /*Second level of curly brackets.*/

#define CLEVEL  20
#define IFLEVEL 10
#define DOLEVEL 10
#define OPLENGTH        10
#define LINE    256
#define LINELENG        120
#define MBSLENG 16                              /* space needed for an MB char */
#define MAXTABS 8
#define TABLENG 8
#define TEMP    1024

struct indent {         /* one for each level of { } */
        int     tabs;
        int     pdepth;
        int     iflev;
        int     ifc[IFLEVEL];
        int     spdepth[IFLEVEL];
} ind[CLEVEL];

struct indent *clev = ind;

struct keyw {
        char    *name;
        char    punc;
        char    type;
} key[] = {
        "switch", ' ', OTHER,
        "do", ' ', DO,
        "while", ' ', OTHER,
        "if", ' ', IF,
        "for", ' ', OTHER,
        "else", ' ', ELSE,
        "case", ' ', CASE,
        "default", ' ', CASE,
        "char", '\t', TYPE,
        "int", '\t', TYPE,
        "short", '\t', TYPE,
        "long", '\t', TYPE,
        "unsigned", '\t', TYPE,
        "float", '\t', TYPE,
        "double", '\t', TYPE,
        "struct", ' ', STRUCT,
        "union", ' ', STRUCT,
        "extern", ' ', TYPE,
        "register", ' ', TYPE,
        "static", ' ', TYPE,
        "typedef", ' ', TYPE,
        "const", ' ', TYPE,
        "enum", ' ', TYPE,
        "signed", ' ', TYPE,
        "void", '\t', TYPE,
        "volatile", ' ', TYPE,
        0, 0, 0
};


struct op {
        char    *name;
        char    blanks;
        char    setop;
} op[] = {
        "+=",   ALWAYS,  YES,
        "-=",   ALWAYS,  YES,
        "*=",   ALWAYS,  YES,
        "/=",   ALWAYS,  YES,
        "%=",   ALWAYS,  YES,
        ">>=",  ALWAYS,  YES,
        "<<=",  ALWAYS,  YES,
        "&=",   ALWAYS,  YES,
        "^=",   ALWAYS,  YES,
        "|=",   ALWAYS,  YES,
        ">>",   ALWAYS,  YES,
        "<<",   ALWAYS,  YES,
        "<=",   ALWAYS,  YES,
        ">=",   ALWAYS,  YES,
        "==",   ALWAYS,  YES,
        "!=",   ALWAYS,  YES,
        "=",    ALWAYS,  YES,
        "&&",   ALWAYS, YES,
        "||",   ALWAYS, YES,
        "++",   NEVER, NO,
        "--",   NEVER, NO,
        "->",   NEVER, NO,
        "<",    ALWAYS, YES,
        ">",    ALWAYS, YES,
        "+",    ALWAYS, YES,
        "/",    ALWAYS, YES,
        "%",    ALWAYS, YES,
        "^",    ALWAYS, YES,
        "|",    ALWAYS, YES,
        "!",    NEVER, YES,
        "~",    NEVER, YES,
        "*",    SOMETIMES, YES,
        "&",    SOMETIMES, YES,
        "-",    SOMETIMES, YES,
        "?",    ALWAYS, YES,
        ":",    ALWAYS, YES,
        0,      0, 0
};


FILE *input = stdin;
int     strict = 0;
int     join    = 0;
int     opflag = 1;
int     keyflag = 0;
int     initlev = INITNONE; /*A24146 - Init. level for structures & arrays.*/
int     directive = FALSE;  /*A24146 - TRUE if directive detected.*/
int     paren    = 0;
int     split    = 0;
int     folded  = 0;
int     dolevel = 0;
int     dotabs[DOLEVEL];
int     docurly[DOLEVEL];
int     dopdepth[DOLEVEL];
int     structlev = 0;
int     question         = 0;
char    string[LINE];
char    *lastlook;
char    *p = string;
char    *s_mbs = NULL;                          /* output buff start of an MB */
char    *temp;                                  /* Dynamic buffer allocation */
char    *tp;
int     err = 0;
char    *lastplace;
char    *tptr;
int     maxleng     = LINELENG;
int     maxtabs     = MAXTABS;
int     count       = 0;
char    next = '\0';
int     inswitch        = 0;
int     lbegin   = 1;
int     temp_size = TEMP;       /* size of current dynamic buffer */

/* A-Za-z0-9_$ */
#define isletter(c) (isalnum(c) || c == '_' || c == '$')

/*
 * By defining CDEBUG, all input is buffered by getmore()
 */

#if CDEBUG
static int      getmore(FILE*);

static char     *sbuf;
static char     *scp;
static int      cdebug = 0;

# define READC(input) (*scp? *scp++: getmore(input))
# define UNREAD(ch, foo) ((ch >= 0) ? *--scp = ch : ch)

static int      
getmore(FILE *in)
{
        if (!sbuf)
                sbuf = malloc (BUFSIZ + 1);
        if (feof(in) || fgets(sbuf, BUFSIZ, in) == NULL) {
                return EOF;
        }
        if (cdebug) {
                printf(">>>%s", sbuf);
        }
        scp = sbuf;
        return * scp++;
}


#else /* Not CDEBUG */
# define READC getc
# define UNREAD ungetc
#endif /* CDEBUG */

struct keyw *lookup(char *first, char *last);
void    work(void);
void    gotif(void);
void    gotelse(void);
int     checkif(char *);
void    gotdo(void);
void    resetdo(void);
void    gottype(struct keyw *);
void    gotstruct(void);
void    gotop(int);
void    keep(struct op *);
int     getnl(void);
void    outs(int);
int     incomment = NO;  /* 105680 - added so that warning messages
                            would not be printed in the middle of comments */
int     comment(int);
void    putspace(int, int);
int     getch(void);
void    unget(int);
char    *getnext(int);
void    copy(char *);
void    clearif(struct indent *);
char    puttmp(int, int);
int     passmbc(int, int);
void    ptabs(int);
void    putch(int, int);
int     widthmbc(char *);

#define OUT     outs(clev->tabs); putchar('\n');opflag = lbegin = 1; count = 0
#define OUTK    OUT; keyflag = 0
#define BUMP    clev->tabs++; clev->pdepth++
#define UNBUMP  clev->tabs -= clev->pdepth; clev->pdepth = 0
#define eatspace()      while((cc=getch()) == ' ' || cc == '\t'); unget(cc)
#define eatallsp()      while((cc=getch()) == ' ' || cc == '\t' || cc == '\n'); unget(cc)

int
main(int argc, char *argv[])
{
        char    *num_ptr;

        setlocale(LC_ALL, "");
        catd = catopen(MF_CB, NL_CAT_LOCALE);
        cb_mb_cur_max = MB_CUR_MAX;

        while (--argc > 0 && (*++argv)[0] == '-') {
                switch ((*argv)[1]) {
                case 's':
                        strict = 1;
                        continue;
                case 'j':
                        join = 1;
                        continue;
                case 'l':
                        if ( (*argv)[2] == '\0' ) {
                                num_ptr = *++argv;
                                do {
                                        if ( isdigit(*num_ptr) )
                                                num_ptr++;
                                        else {
                                                fprintf(stderr, MSGSTR(NONUM, "The -l flag must be followed by a number.\n"));
                                                exit(1);
                                        }
                                } while (*num_ptr);
                                maxleng = atoi(*argv);
                                argc--;
                        } else if (isdigit((*argv)[2]) ) {
                                num_ptr = &((*argv)[3]);
                                do {
                                        if ( isdigit(*num_ptr) )
                                                num_ptr++;
                                        else {
                                                fprintf(stderr, MSGSTR(NONUM, "The -l flag must be followed by a number.\n"));
                                                exit(1);
                                        }
                                } while (*num_ptr);
                                maxleng = atoi(&(*argv)[2]);
                        } else {
                                fprintf(stderr, MSGSTR(NONUM, "The -l flag must be followed by a number.\n"));
                                exit(1);
                        }
                        maxtabs = maxleng / TABLENG - 2;
                        maxleng -= maxleng / 10;
                        continue;
                default:
                        fprintf(stderr, MSGSTR(BADOPTION, "cb: illegal option %c\n"), (*argv)[1]);
                        exit(1);
                }
        }
        if (argc <= 0)
                work();
        else {
                while (argc-- > 0) {
                        if ((input = fopen( *argv, "r")) == NULL) {
                                fprintf(stderr, MSGSTR(BADOPEN, "cb: cannot open input file %s\n"), *argv);
                                exit(1);
                        }
                        work();
                        argv++;
                }
        }
        return(0);
}


void
work(void)
{
        register int    c;
        register struct keyw *lptr;
        char    *pt;
        char    cc;
        int     ct, save_paren, save_split;

        /* initialize/allocate dynamic buffer and pointers */
        temp = (char *) malloc((size_t) temp_size);
        lastplace = tptr = temp;

        while ((c = getch()) != EOF) {
                switch (c) {

                case '{':
                        /*A24146 - Check if the current open curly is part
                        of an initialization.  If so, increase initlev to 
                        indicate that another init level has been started.*/
                        if (initlev >= INITZERO)
                             initlev++;

                        /*A24146 - If initializing a structure or array that 
                        uses curly brackets within the initialization statement
                        (ie: "= { {}, {}, ... {} };"), do not format the second
                        or greater set of curly brackets as usually done.*/
                        if (initlev >= INITTWO) {
                             putch(c,NO);
                             if (strict) {
                                  putch(' ', NO);
                                  eatspace();
                             }
                        }
                        else
                        {
                             if ((lptr = lookup(lastlook, p)) != NULL) {
                                     if (lptr->type == ELSE)
                                             gotelse();
                                     else if (lptr->type == DO)
                                             gotdo();
                                     else if (lptr->type == STRUCT)
                                             structlev++;
                             }
     
                             if (++clev >= &ind[CLEVEL-1]) {
                                     fprintf(stderr, MSGSTR(CURLY, "too many levels of curly brackets\n"));
                                     clev = &ind[CLEVEL-1];
                             }
                             clev->pdepth = 0;
                             clev->tabs = (clev - 1)->tabs;
                             clearif(clev);
     
                             /*A24146 - Only output a space under the following
                             conditions.*/
                             if (strict && clev->tabs > 0) {
                                     if (p == string && keyflag != KEYWORD)
                                             ;
                                     else if (*(p - 1) == ' ')
                                             ;
                                     else
                                             putch(' ', NO);
                             }
     
                             putch(c, NO);
                             getnl();
                             if (keyflag == DATADEF) {
                                     OUT;
                             } else {
                                     OUTK;
                             }
                             clev->tabs++;
                        }
                        continue;

                case '}':
                        pt = getnext(1);

                        /*A24146 - Check if the current close curly ends
                        initialization of a previous open curly two or
                        more levels deep.  If so, format appropriately.*/
                        if (initlev >= INITTWO) {
                             if (*pt == ',') {
                                  if(strict) {
                                       putspace(' ',NO);
                                       eatspace();
                                  }
                                  putch(c,NO);
                                  putch(*pt,NO);
                                  *pt = '\0';
                                  ct = getnl();
                                  pt = getnext(1);
                             }
                             else if (*pt == '}') {
                                  if(strict) {
                                       putspace(' ',NO);
                                       eatspace();
                                  }
                                  putch(c,NO);
                                  getnl();
                                  OUT;
                             }
                             else
                             {
                                  putch(c,NO);
                             }
                        }
                        else
                        {
                             /*A24146 - Decrease initlev if processing the
                             end of the initialization (ie: "};"). This is the
                             opposite of the increase done for the start of
                             initialization (ie: "= {").*/
                             if (initlev >= INITZERO && *pt == ';') {
                                  initlev--;
                             }

                             outs(clev->tabs);
                             if (--clev < ind)
                                     clev = ind;
                             ptabs(clev->tabs);
                             putch(c, NO);
                             lbegin = 0;
                             lptr = lookup(pt, lastplace + 1);
                             c = *pt;
                             if (*pt == ';' || *pt == ',') {
                                     putch(*pt, NO);
                                     *pt = '\0';
                                     lastplace = pt;
                             }
                             ct = getnl();
                             if ((dolevel && clev->tabs <= dotabs[dolevel]) || 
                                 (structlev) || 
                                 (lptr != 0 && lptr->type == ELSE && 
                                     clev->pdepth == 0)) {
                                     if (c == ';') {
                                             OUTK;
                                     }
                                     /*A24146 - Do not write blank after close
                                     curly if directive detected in input
                                     stream, otherwise the close curly will
                                     be misaligned with its corresponding open
                                     curly bracket.*/
                                     else if ((strict && !directive) || 
                                              (lptr != 0 && 
                                                   lptr->type == ELSE && 
                                                   ct == 0)) {
                                             putspace(' ', NO);
                                             eatspace();
                                     } 
                                     else if (lptr != 0 && lptr->type == ELSE) {
                                             OUTK;
                                     }
                                     if (structlev) {
                                             structlev--;
                                             keyflag = DATADEF;
                                     }
                             } else {
                                     OUTK;
                                     if (strict && clev->tabs == 0) {
                                             if ((c = getch()) != '\n') {
                                                     putchar('\n');
                                                     putchar('\n');
                                                     unget(c);
                                             } else {
                                                     putchar('\n');
                                                     if ((c = getch()) != '\n')
                                                             unget(c);
                                                     putchar('\n');
                                             }
                                     }
                             }
                             if (lptr != 0 && lptr->type == ELSE && 
                                     clev->pdepth != 0) {
                                     UNBUMP;
                             }
                             if (lptr == 0 || lptr->type != ELSE) {
                                     clev->iflev = 0;
                                     if (dolevel && docurly[dolevel] == NO && 
                                         clev->tabs == dotabs[dolevel] + 1)
                                             clev->tabs--;
                                     else if (clev->pdepth != 0) {
                                             UNBUMP;
                                     }
                             }
                        }

                        /*A24146 - Check if the current close curly is part
                        of an initialization.  If so, decrease initlev to
                        indicate that a level has been processed.*/
                        if (initlev >= INITZERO)
                             initlev--;

                        continue;
                case '(':
                        paren++;
                        if ((lptr = lookup(lastlook, p)) != NULL) {
                                if (!(lptr->type == TYPE || 
                                    lptr->type == STRUCT))
                                        keyflag = KEYWORD;
                                if (strict) {
                                        putspace(lptr->punc, NO);
                                        opflag = 1;
                                }
                                putch(c, NO);
                                if (lptr->type == IF)
                                        gotif();
                        } else {
                                putch(c, NO);
                                lastlook = p;
                                opflag = 1;
                        }
                        continue;
                case ')':
                        if (--paren < 0)
                                paren = 0;
                        putch(')', NO);
                        if ((lptr = lookup(lastlook, p)) != NULL) {
                                if (lptr->type == TYPE || lptr->type == STRUCT)
                                        opflag = 1;
                        } else if (keyflag == DATADEF)
                                opflag = 1;
                        else
                                opflag = 0;
                        outs(clev->tabs);
                        pt = getnext(1);
                        if ((ct = getnl()) == 1 && !strict) {
                                if (dolevel && clev->tabs <= dotabs[dolevel])
                                        resetdo();
                                if (clev->tabs > 0 && (paren != 0 || keyflag == 0)) {
                                        if (join) {
                                                eatspace();
                                                putch(' ', YES);
                                                continue;
                                        } else {
                                                OUT;
                                                split = 1;
                                                continue;
                                        }
                                } else if (clev->tabs > 0 && *pt != '{') {
                                        BUMP;
                                }
                                OUTK;
                        } else if (strict) {
                                if (clev->tabs == 0) {
                                        if (*pt != ';' && *pt != ',' && 
                                            *pt != '(' && *pt != '[') {
                                                OUTK;
                                        }
                                } else {
                                        if (keyflag == KEYWORD && paren == 0) {
                                                if (dolevel && clev->tabs
                                                     <= dotabs[dolevel]) {
                                                        resetdo();
                                                        eatspace();
                                                        continue;
                                                }
                                                if (*pt != '{') {
                                                        BUMP;
                                                        OUTK;
                                                } else {
                                                        /*A24146 - If directive
                                                        TRUE, then do not move
                                                        open curly before close
                                                        paren.*/
                                                        if (directive)
                                                        {
                                                             OUTK;
                                                        }
                                                        else
                                                        {
                                                             *pt='\0';
                                                             eatspace();
                                                             unget('{');
                                                        }
                                                }
                                        } else if (ct) {
                                                if (paren) {
                                                        if (join) {
                                                                eatspace();
                                                        } else {
                                                                split = 1;
                                                                OUT;
                                                        }
                                                } else {
                                                        OUTK;
                                                }
                                        }
                                }
                        } else if (dolevel && clev->tabs <= dotabs[dolevel])
                                resetdo();
                        continue;
                case ' ':
                case '\t':
                        if ((lptr = lookup(lastlook, p)) != NULL) {
                                if (!(lptr->type == TYPE || lptr->type == STRUCT))
                                        keyflag = KEYWORD;
                                else if (paren == 0)
                                        keyflag = DATADEF;
                                if (strict) {
                                        if (lptr->type != ELSE) {
                                                if (lptr->type == TYPE) {
                                                        if (paren != 0)
                                                                putch(' ', YES);
                                                } else
                                                        putch(lptr->punc, NO);
                                                eatspace();
                                        }
                                } else
                                        putch(c, YES);
                                switch (lptr->type) {
                                case CASE:
                                        outs(clev->tabs - 1);
                                        continue;
                                case ELSE:
                                        pt = getnext(1);
                                        eatspace();
                                        if ((cc = getch()) == '\n' && !strict) {
                                                unget(cc);
                                        } else {
                                                unget(cc);
                                                if (checkif(pt))
                                                        continue;
                                        }
                                        gotelse();
                                        if (strict)
                                                unget(c);
                                        if (getnl() == 1 && !strict) {
                                                OUTK;
                                                if (*pt != '{') {
                                                        BUMP;
                                                }
                                        } else if (strict) {
                                                if (*pt != '{') {
                                                        OUTK;
                                                        BUMP;
                                                }
                                        }
                                        continue;
                                case IF:
                                        gotif();
                                        continue;
                                case DO:
                                        gotdo();
                                        pt = getnext(1);
                                        if (*pt != '{') {
                                                eatallsp();
                                                OUTK;
                                                docurly[dolevel] = NO;
                                                dopdepth[dolevel] = clev->pdepth;
                                                clev->pdepth = 0;
                                                clev->tabs++;
                                        }
                                        continue;
                                case TYPE:
                                        if (paren)
                                                continue;
                                        if (!strict)
                                                continue;
                                        gottype(lptr);
                                        continue;
                                case STRUCT:
                                        gotstruct();
                                        continue;
                                }
                        } else if (lbegin == 0 || p > string)
                                if (strict)
                                        putch(c, NO);
                                else
                                        putch(c, YES);
                        continue;
                case ';':
                        putch(';', NO);
                        if (paren != 0) {
                                if (strict) {
                                        putch(' ', YES);
                                        eatspace();
                                }
                                opflag = 1;
                                continue;
                        }
                        outs(clev->tabs);
                        pt = getnext(1);
                        lptr = lookup(pt, lastplace + 1);
                        if (lptr == 0 || lptr->type != ELSE) {
                                clev->iflev = 0;
                                if (clev->pdepth != 0) {
                                        UNBUMP;
                                }
                                if (dolevel && docurly[dolevel] == NO && 
                                    clev->tabs <= dotabs[dolevel] + 1)
                                        clev->tabs--;
                        }
                        getnl();
                        OUTK;
                        continue;
                case '\n':
                        if ((lptr = lookup(lastlook, p)) != NULL) {
                                pt = getnext(1);
                                if (lptr->type == ELSE) {
                                        if (strict)
                                                if (checkif(pt))
                                                        continue;
                                        gotelse();
                                        OUTK;
                                        if (*pt != '{') {
                                                BUMP;
                                        }
                                } else if (lptr->type == DO) {
                                        OUTK;
                                        gotdo();
                                        if (*pt != '{') {
                                                docurly[dolevel] = NO;
                                                dopdepth[dolevel] = clev->pdepth;
                                                clev->pdepth = 0;
                                                clev->tabs++;
                                        }
                                } else {
                                        OUTK;
                                        if (lptr->type == STRUCT)
                                                gotstruct();
                                }
                        } else if (p == string)
                                putchar('\n');
                        else {
                                if (clev->tabs > 0 && 
                                    (paren != 0 || keyflag == 0)) {
                                        if (join) {
                                                putch(' ', YES);
                                                eatspace();
                                                continue;
                                        } else {
                                                OUT;
                                                split = 1;
                                                continue;
                                        }
                                } else if (keyflag == KEYWORD) {
                                        OUTK;
                                        continue;
                                }
                                OUT;
                        }
                        continue;
                case '"':
                case '\'':
                        putch (c, NO);          /* output the delimter */

                        /* GH 02/01/91 cb hangs when string is missing a " */
                        /* A17168 */

                        while ((cc = getch()) != c && cc != (char)EOF) {

                                passmbc(cc, NO);        /* output inputted character */

                                if (cc == '\\') {/* get escaped character and output it */
                                        cc = getch();
                                        passmbc(cc, NO);
                                        continue;
                                }

                                if (cc == '\n') {
                                        outs(clev->tabs);
                                        lbegin = 1;
                                        count = 0;
                                }
                        } /*end of while */

                        if (cc == (char)EOF) {
                                unget(EOF);
                                continue;
                        }
                        putch(cc, NO);
                        opflag = 0;
                        if (getnl() == 1) {
                                unget('\n');
                        }
                        continue;
                case '\\':
                        putch('\\', NO);
                        putch(getch(), NO);
                        continue;
                case '?':
                        question = 1;
                        gotop(c);
                        continue;
                case ':':
                        if (question == 1) {
                                question = 0;
                                gotop(c);
                                continue;
                        }
                        putch(c, NO);
                        if (structlev)
                                continue;
                        if ((lptr = lookup(lastlook, p)) != NULL) {
                                if (lptr->type == CASE)
                                        outs(clev->tabs - 1);
                        } else {
                                lbegin = 0;
                                outs(clev->tabs);
                        }
                        getnl();
                        OUTK;
                        continue;
                case '/':
                        if ((cc = getch()) != '*') {
                                unget(cc);
                                gotop(c);
                                continue;
                        }
                        putch(c, NO);
                        putch(cc, NO);
                        incomment = YES;
                        cc = comment(YES);
                        if (cc != 2) {
                                if (getnl() == 1) {
                                        if (cc == 0) {
                                                OUT;
                                        } else {
                                                outs(0);
                                                putchar('\n');
                                                lbegin = 1;
                                                count = 0;
                                        }
                                        lastlook = NULL;
                                }
                        }
                        incomment = NO;
                        continue;
                case '[':
                        putch('[', NO);
                        ct = 0;
                        while ((c = getch()) != ']' || ct > 0) {
                                putch(c, NO);
                                if (c == '[')
                                        ct++;
                                if (c == ']')
                                        ct--;
                        }
                        putch(c, NO);
                        continue;
                case '#':
                        putch('#', NO);         /* output '#' character */
                        while ((cc = getch()) != '\n') {
                                passmbc(cc, NO);        /* output inputted character */

                                if (cc == '\\') { /* get escaped character and output it */
                                        cc = getch();
                                        passmbc(cc, NO);
                                }
                        }
                        putch('\n', NO);                /* output '\n' */

                        /* If K+R style, then preprocessing directives 
                           start in the first column */
                        if (strict)
                                lbegin = 0;

                        save_paren = paren;
                        paren = 0;
                        save_split = split;
                        split = 0;
                        outs(clev->tabs);
                        paren = save_paren;
                        split = save_split;
                        lbegin = 1;
                        count = 0;
                        continue;
                case '=':
                        /*A24146 - Check if the current equal sign starts a
                        struct or array initialization (ie: "= { ... };").*/
                        pt = getnext(1);
                        if (*pt == '{')
                             initlev = INITZERO;
                        gotop(c);
                        continue;
                case ',':
                        opflag = 1;
                        putch(c,YES);
                        if (strict){
                             if ((cc = getch()) != ' ')
                                  unget(cc);
                             if(cc != '\n')
                                  putch(' ',YES);
                        }
                        continue;
                default:
                        if (isop(c))
                                gotop(c);
                        else {
                                if (isletter(c) && lastlook == NULL)
                                        lastlook = p;
                                putch(c, NO);
                                if (isdigit(c) || c == '.')
                                {
                                    c = getch();
                                    while(isdigit(c) || c == '.' || c == 'e' ||
                                         c == 'E' || ((c == '+' || c == '-') &&
                                         (*(p-1) == 'e' || *(p-1) == 'E')))
                                    {
                                        putch(c, NO);
                                        c = getch();
                                    }
                                    unget(c);
                                }
                                if (keyflag != DATADEF)
                                        opflag = 0;
                        }
                }
        }
}


void
gotif(void)
{
        outs(clev->tabs);
        if (++clev->iflev >= IFLEVEL - 1) {
                fprintf(stderr, MSGSTR(IFDEEP, "too many levels of if\n"));
                clev->iflev = IFLEVEL - 1;
        }
        clev->ifc[clev->iflev] = clev->tabs;
        clev->spdepth[clev->iflev] = clev->pdepth;
}


void
gotelse(void)
{
        clev->tabs = clev->ifc[clev->iflev];
        clev->pdepth = clev->spdepth[clev->iflev];
        if (--(clev->iflev) < 0)
                clev->iflev = 0;
}


int
checkif(char *pt)
{
        register struct keyw *lptr;
        int     cc;

        if ((lptr = lookup(pt, lastplace + 1)) != 0) {
                if (lptr->type == IF) {
                        if (strict)
                                putch(' ', YES);
                        copy(lptr->name);
                        *pt = '\0';
                        lastplace = pt;
                        if (strict) {
                                putch(lptr->punc, NO);
                                eatallsp();
                        }
                        clev->tabs = clev->ifc[clev->iflev];
                        clev->pdepth = clev->spdepth[clev->iflev];
                        keyflag = KEYWORD;
                        return(1);
                }
        }
        return(0);
}


void    
gotdo(void)
{
        if (++dolevel >= DOLEVEL - 1) {
                fprintf(stderr, MSGSTR(DODEEP, "too many levels of do\n"));
                dolevel = DOLEVEL - 1;
        }
        dotabs[dolevel] = clev->tabs;
        docurly[dolevel] = YES;
}


void
resetdo(void)
{
        if (docurly[dolevel] == NO)
                clev->pdepth = dopdepth[dolevel];
        if (--dolevel < 0)
                dolevel = 0;
}


void
gottype(struct keyw *lptr)
{
        char    *pt;
        struct keyw *tlptr;
        int     c;

        while (1) {
                pt = getnext(1);
                if ((tlptr = lookup(pt, lastplace + 1)) != 0) {
                        putch(' ', YES);
                        copy(tlptr->name);
                        *pt = '\0';
                        lastplace = pt;
                        if (tlptr->type == STRUCT) {
                                putch(tlptr->punc, YES);
                                gotstruct();
                                break;
                        }
                        lptr = tlptr;
                        continue;
                } else {
                        putch(lptr->punc, NO);
                        while ((c = getch()) == ' ' || c == '\t')
                                ;
                        unget(c);
                        break;
                }
        }
}


void
gotstruct(void)
{
        int     c;
        int     cc;
        char    *pt;

        while ((c = getch()) == ' ' || c == '\t')
                if (!strict)
                        putch(c, NO);
        if (c == '{') {
                structlev++;
                unget(c);
                return;
        }
        if (isalpha(c)) {
                putch(c, NO);
                c = getch();
                while (isletter(c)) {
                        putch(c, NO);
                        c = getch();
                }
        }
        unget(c);
        pt = getnext(1);
        if (*pt == '{')
                structlev++;

        if (strict) {
                eatallsp();
                putch(' ', NO);
        }
}


void
gotop(int c)
{
        char    optmp[OPLENGTH];
        char    *op_ptr;
        struct op *s_op;
        char    *a, *b;

        op_ptr = optmp;
        *op_ptr++ = c;
        for (*op_ptr = getch(); isop(*op_ptr); *++op_ptr = getch())
                ;
        if (!strict)
                unget(*op_ptr);
        else if (*op_ptr != ' ')
                unget( *op_ptr);
        *op_ptr = '\0';
        s_op = op;
        b = optmp;
        while ((a = s_op->name) != 0) {
                op_ptr = b;
                while ((*op_ptr == *a) && (*op_ptr != '\0')) {
                        a++;
                        op_ptr++;
                }
                if (*a == '\0') {
                        keep(s_op);
                        opflag = s_op->setop;
                        if (*op_ptr != '\0') {
                                b = op_ptr;
                                s_op = op;
                                continue;
                        } else
                                break;
                } else
                        s_op++;
        }
}


void
keep(struct op *o)
{
        char    *s;
        int     ok;

        ok = !strict;
        if (strict && ((o->blanks & ALWAYS)
             || ((opflag == 0 && o->blanks & SOMETIMES) && clev->tabs != 0)))
                putspace(' ', YES);
        if (strlen(o->name) > 0) {
                for (s = o->name; *(s + 1) != '\0'; s++)
                        putch(*s, NO);
                putch(*s, ok);
        }
        if (strict && ((o->blanks & ALWAYS)
             || ((opflag == 0 && o->blanks & SOMETIMES) && clev->tabs != 0)))
                putch(' ', YES);
}


int
getnl(void)
{
        register int    ch;
        char    *savp;
        int     gotcmt;

        gotcmt = 0;
        savp = p;

        while ((ch = getch()) == '\t' || ch == ' ')
                putch(ch, NO);
        if (ch == '/') {
                if ((ch = getch()) == '*') {
                        putch('/', NO);
                        putch('*', NO);
                        incomment = YES;
                        comment(NO);
                        incomment = NO;
                        ch = getch();
                        gotcmt = 1;
                } else {
                        if (inswitch)
                                *(++lastplace) = ch;
                        else {
                                inswitch = 1;
                                *lastplace = ch;
                        }
                        unget('/');
                        return(0);
                }
        }
        if (ch == '\n') {
                if (gotcmt == 0)
                        p = savp;
                return(1);
        }
        unget(ch);
        return(0);
}


void
ptabs(int n)
{
        int     i;
        int     num;

        if (n > maxtabs) {
                if (!folded) {
                        printf(MSGSTR(FOLDED, "/* code folded from here */\n"));
                        folded = 1;
                }
                num = n - maxtabs;
        } else {
                num = n;
                if (folded && !incomment) {
                        folded = 0;
                        printf(MSGSTR(UNFOLDING, "/* unfolding */\n"));
                }
        }
        for (i = 0; i < num; i++)
                putchar('\t');
}


void
outs(int n)
{
        if (p > string) {
                if (lbegin) {
                        ptabs(n);
                        lbegin = 0;
                        if (split == 1) {
                                split = 0;
                                if (clev->tabs > 0)
                                        printf("    ");
                        }
                }
                *p = '\0';
                printf("%s", string);
                lastlook = p = string;
        } else {
                if (lbegin != 0) {
                        lbegin = 0;
                        split = 0;
                }
        }
}


void
putch(int c, int ok)
{
        register int    cc;
        register int    x;
        char    tmp[MBSLENG];
        int     i, j;

        /*
         * Not enough room in the output buffer, so send output it and reuse it
         * Make sure that we are not splitting an MB
         */
        if (p >= &string[LINE-1]) {
                if (s_mbs != NULL)                      /* save the MB */ {
                        for (i = 0; s_mbs + i != p; i++)
                                tmp[i] = s_mbs[i];
                        p = s_mbs;
                }

                outs (clev->tabs);                      /* output buffer */
                count = 0;

                if (s_mbs != NULL)                      /* restore the MB */ {
                        s_mbs = p;
                        for (j = 0; j < i; j++)
                                *p++ = tmp[j];
                }
        }

        /* Copy the new character into the output buffer */

        *p++ = c;
        *p   = 0;
        x = ((c == ' ') ? 1 : 0);                       /* remember char was space */

        if (s_mbs == NULL)                              /* not in an MB */ {
                if (mbsinvalid(p - 1))                  /* remember the start of one */
                        s_mbs = (p - 1);
                else    /* or update display count */
                        count += 1;
        } else  /* in an MB */   {
                if (mbsinvalid(s_mbs) == NULL)          /* finished MB */ {
                        count += widthmbc (s_mbs);
                        s_mbs = NULL;
                } else if (p - s_mbs >= cb_mb_cur_max)  /* don't mess up on a bad char */ {
                        count += cb_mb_cur_max;         /* just count the chars */
                        s_mbs = NULL;
                }
        }

        /* output a line if we pass the line length */

        if (((count + TABLENG * clev->tabs) >= maxleng) && ok && !folded) {
                if (x)
                        *--p = 0;                               /* don't need whitespace */
                OUT;
                split = 1;
                if ((cc = getch()) != '\n')             /* eat up immediate NL */
                        unget (cc);
        }
}


struct keyw *
lookup(char *first, char *last)
{
        struct keyw *ptr;
        char    *cptr, *ckey, *k;

        if (first == last || first == NULL)
                return(NULL);

        cptr = first;
        while (*cptr == ' ' || *cptr == '\t')
                cptr++;
        if (cptr >= last)
                return(NULL);

        ptr = key;
        while ((ckey = ptr->name) != NULL) {
                for (k = cptr; (*ckey == *k && *ckey != '\0'); k++, ckey++)
                        ;
                if (*ckey == '\0' && (k == last || (k < last && !isletter(*k)))) {
                        opflag = 1;
                        lastlook = NULL;
                        return(ptr);
                }
                ptr++;
        }
        return(0);
}


int
comment(int ok)
{
        register int    ch;
        int     hitnl;

        hitnl = 0;
        while ((ch  = getch()) != EOF) {
                passmbc(ch, NO);
                if (ch == '*') {
gotstar:
                        if ((ch  = getch()) == '/') {
                                putch(ch, NO);
                                return(hitnl);
                        }
                        passmbc(ch, NO);
                        if (ch == '*')
                                goto gotstar;
                }
                if (ch == '\n') {
                        if (ok && !hitnl) {
                                outs(clev->tabs);
                        } else {
                                outs(0);
                        }
                        lbegin = 1;
                        count = 0;
                        hitnl = 1;
                }
        }
        /* GH 02/01/91 A17168 */
        unget(EOF);
        hitnl = 2;
        return(hitnl);
}


void
putspace(int ch, int ok)
{
        if (p == string)
                putch(ch, ok);
        else if (*(p - 1) != ch)
                putch(ch, ok);
}


int     
getch(void)
{
        register char   c;

        if (inswitch) {
                if (next != '\0') {
                        c = next;
                        next = '\0';
                        return(c);
                }
                if (tptr <= lastplace) {
                        if (*tptr != '\0')
                                return(*tptr++);
                        else if (++tptr <= lastplace)
                                return(*tptr++);
                }
                inswitch = 0;
                lastplace = tptr = temp;
        }
        return(READC(input));
}


void
unget(int c)
{
        if (inswitch) {
                if (tptr != temp)
                        *(--tptr) = c;
                else
                        next = c;
        } else
                UNREAD(c, input);
}


char    *
getnext(int must)
{
        int     c;
        char    *beg;
        int     nlct;

        directive=FALSE; /*A24146 - Preprocessor directive not detected.*/
        nlct = 0;

        if (tptr > lastplace) {
                tptr = lastplace = temp;
                err = 0;
                inswitch = 0;
        }
        tp = beg = lastplace;
        if (inswitch && tptr <= lastplace)
                if (isletter(*lastplace) || ispunct(*lastplace) || isop(*lastplace))
                        return(lastplace);
space:
        c = READC(input);
        while (isspace(c))
        {
                puttmp(c, 1);
                c = READC(input);
        }
        beg = tp;
        puttmp(c, 1);
        if (c == '/') {
                if (puttmp((c = READC(input)), 1) == '*') {
cont:
                        /* GH 02/01/91 cb hangs when comment not finished */
                        /* A17168 */
                        while ((c = READC(input)) != '*' && c != EOF) {
                                puttmp(c, 0);
                                if (must == 0 && c == '\n')
                                        if (nlct++ > 2)
                                                goto done;
                        }
                        puttmp(c, 1);
star:
                        if (puttmp((c = READC(input)), 1) == '/') {
                                beg = tp;
                                puttmp((c = READC(input)), 1);
                        } else if (c == '*')
                                goto star;
                        else if (c == EOF)
                                goto done;
                        else
                                goto cont;
                } else
                        goto done;
        }
        if (isspace(c))
                goto space;

        if (c == '#' && tp > temp + 1 && *(tp - 2) == '\n') {
                if (strict)
                     directive=TRUE; /*A24146 - TRUE if a directive detected.*/

                /*A24146 - Remove look ahead for only several directives,
                as this causes inconsistent formating for code with 1 to 3
                directives, compared to code with 4 or more directives.*/
                /*if(prect++ > 2)goto done;*/
                while (puttmp ((c = READC(input)), 1) != '\n')
                        if (c == '\\')
                                puttmp (READC (input), 1);
                goto space;
        }
        if (isletter(c)) {
                c = READC(input);
                while (isletter(c)) {
                        puttmp(c, 1);
                        c = READC(input);
                }
                UNREAD(c, input);
        }
done:
        puttmp('\0', 1);
        lastplace = tp - 1;
        inswitch = 1;
        return(beg);
}


void
copy(char *s)
{
        while (*s != '\0')
                putch(*s++, NO);
}


void
clearif(struct indent *cl)
{
        int     i;

        for (i = 0; i < IFLEVEL - 1; i++)
                cl->ifc[i] = 0;
}


char
puttmp(int c, int keep)
{
        char    *tmp_buf;

        if (tp < &temp[temp_size-120])
                *tp++ = c;
        else
        /* reach the high water mark of current buffer, expand the buffer */
        {
                tmp_buf = (char *) realloc(temp, (size_t) temp_size + TEMP);
                if (tmp_buf != 0) {/* expand buffer successfully, then adjust size and pointers */

                        temp_size += TEMP;
                        tptr = tmp_buf + (tptr - temp);
                        tp = tmp_buf + (tp - temp);
                        lastplace = tmp_buf + (lastplace - temp);
                        temp = tmp_buf;
                        *tp++ = c;
                } else
                /* no more memory available or fail in realloc function */
                {
                        if (keep) {
                                if (tp >= &temp[temp_size-1]) {
                                        fprintf(stderr, MSGSTR(BIGCOMMENT, "can't look past huge comment - quiting\n"));
                                        exit(1);
                                }
                                *tp++ = c;
                        } else if (err == 0) {
                                err++;
                                fprintf(stderr, MSGSTR(TRUNCATING, "truncating long comment\n"));
                        }
                }
        }
        return(c);
}


/*
 * NAME: passmbc
 *
 * FUNCTION: Pass a whole character starting with the supplied byte and
 * getting subsequent bytes - if necessary - from input.
 *
 * ALGORITHM: Output the first byte;  while the output character is not
 * complete, read and output the next byte.
 *
 * RETURNS: i => number of bytes output.
 *
 * HISTORY: initial coding      May 1991 M S Flegel of IBM(ACTC)
 */
int
passmbc (int c, int ok)
{
        char    mbs[16];
        int     i;

        if (cb_mb_cur_max <= 1)                 /* optimize */ {
                putch(c, ok);
                return(1);
        }

        mbs[0] = c;                                     /* initialize output character */
        mbs[1] = 0;                                     /* terminate buffer */
        i = 0;

        while ((i + 1 < cb_mb_cur_max) && (mbsinvalid (mbs) != NULL)) {
                putch (mbs[i++], ok);                   /* output character */
                mbs[i] = getch ();                      /* get next byte of MB */
                mbs[i+1] = 0;                           /* terminate buffer */
        }
        putch (mbs[i], ok);                             /* output last byte */

        return (i + 1);                         /* return how many passed */
}


/*
 * NAME: widthmbc
 *
 * FUNCTION: Figure out the display width of a multibyte character.
 *
 * ALGORITHM: If the __max_display_width global is 1, then simply return 1;
 * otherwise, use "wcwidth" to determine its width.
 *
 * RETURNS: >0 => number of columns required to display
 *           0 => error
 *
 * HISTORY: initial coding      April 1991 M S Flegel of IBM(ACTC)
 */

int
widthmbc (char *p_s)                            /* pointer to multibyte character */
{
        wchar_t wc;
        int     i;

        if (p_s == NULL)                                /* nothing to calculate */
                return (0);

        if (__max_disp_width <= 1)                      /* at most one wide */
                return (1);

        if (mbtowc (&wc, p_s, MB_CUR_MAX) <= 0) /* invalid - nothing to calculate */
                return (0);

        i = wcwidth (wc);                               /* convert it */
        return ((i > 0) ? i : 0);
}


