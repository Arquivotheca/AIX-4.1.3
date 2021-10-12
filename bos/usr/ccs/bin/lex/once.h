/* @(#)15       1.8.1.7  src/bos/usr/ccs/bin/lex/once.h, cmdlang, bos411, 9428A410j 12/17/93 11:50:21 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
#ifndef _H_ONCE
#define _H_ONCE

#include "hash.h"

/*
 * because of external definitions, this code should occur only once
 */

int ctable[2*NCH] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
100,101,102,103,104,105,106,107,108,109,
110,111,112,113,114,115,116,117,118,119,
120,121,122,123,124,125,126,127,128,129,
130,131,132,133,134,135,136,137,138,139,
140,141,142,143,144,145,146,147,148,149,
150,151,152,153,154,155,156,157,158,159,
160,161,162,163,164,165,166,167,168,169,
170,171,172,173,174,175,176,177,178,179,
180,181,182,183,184,185,186,187,188,189,
190,191,192,193,194,195,196,197,198,199,
200,201,202,203,204,205,206,207,208,209,
210,211,212,213,214,215,216,217,218,219,
220,221,222,223,224,225,226,227,228,229,
230,231,232,233,234,235,236,237,238,239,
240,241,242,243,244,245,246,247,248,249,
250,251,252,253,254,255};
int     ZCH = NCH;
FILE    *fout = NULL, *errorf = {stdout};
int     sect = DEFSECTION;
int     prev = '\n';    /* previous input character */
int     pres = '\n';    /* present input character */
int     peek = '\n';    /* next input character */
wchar_t *pushptr = pushc;
wchar_t *slptr = slist;

#ifndef  _BLD                                   /* kept for compatibility with osf code */
nl_catd catd;
# endif /* _BLD */

char    *cname = "/usr/lib/lex/ncform";
char    *ratname = "/usr/lib/lex/nrform";       /* not supported */

int     ccount          = 1;
int     casecount       = 1;                    /* number of rules to match */
int     aptr            = 1;
int     nstates         = NSTATES,
        maxpos          = MAXPOS;
int     treesize        = TREESIZE,
        ntrans          = NTRANS;
int     yytop;
int     outsize         = NOUTPUT;              /* yycrank table size */
int     whspace         = NWHSPACE;             /* yywcrank hash table vacancy percentage */
int     sptr            = 1;
int     optim           = TRUE;
int     report          = 2;
int     debug;                                  /* 1 = on */
int     charc;
int     sargc;
char    **sargv;
wchar_t buf[520];
int     ratfor;                                 /* 1 = ratfor, 0 = C */
int     cplusplus;                              /* 1 = C++, 0 = ratfor|C */
int     yyline;                                 /* line number of file */
int     eof;
int     lgatflg;
int     divflg;
int     funcflag;
int     pflag;
int     chset;                                  /* 1 = char set modified */
FILE    *fin, *fother;
int     fptr;
int     *name;
wchar_t *wname;                                 /* wide char name table */
int     *left;
int     *right;
int     *parent;
wchar_t *nullstr;
int     tptr;
wchar_t pushc[TOKENSIZE];
wchar_t slist[STARTSIZE];
wchar_t **def, **subs, *dchar;
wchar_t **sname, *schar;
short   *excl;
wchar_t *dp, *sp;
int     dptr;
wchar_t *bptr;                                  /* store input position */
wchar_t *tmpstat;
int     count;
int     **foll;
int     *nxtpos;
int     *positions;
int     *gotof;
int     *nexts;
wchar_t *nchar;
int     **state;
int     *sfall;                                 /* fallback state num */
char    *cpackflg;                              /* true if state has been character packed */
int     *atable;
int     nptr;
wchar_t symbol[NCH];                            /* workspace for recording CCLs */
int     wsymbollen      = 0;                    /* cirrent size of table */
int     wsymboli;                               /* next open slot in table */
wchar_t *wsymbol;                               /* wide char symbol matches */
wchar_t cindex[NCH];                            /* maps chars into their respective CCLs */
int     xstate;
int     stnum;
wchar_t match[NCH];
wchar_t *extra;
int     nactions        = NACTIONS;             /* Initialization for original size of extra */
wchar_t *pchar, *pcptr;
int     pchlen          = TOKENSIZE;
long    rcount;
int     *stoff;
int     *verify, *advance;                      /* transition tables */
hash_t  *wcrank;                                /* MB transition hash table */
hash_t  *wmatch;                                /* MB CCL mapping hash table */
xccl_t  *xccl;                                  /* special CCL table */
int     xccltop         = 0;                    /* number of xccl elements */
int     wmatchlist      = 0;                    /* first MB in MB CCL hash table */
int     wcranksize      = 0;                    /* wcrank hash table size */
int     wmatchsize      = 0;                    /* wmatch hash table size  */
int     xcclsize        = 0;                    /* xccl sorted table size */
int     scon;
wchar_t *psave;
int     buserr(), segviol();
int     multibytecodeset;
int     mbcurmax;
int     defsize;
int     defchar;
int     yaccerror       = FALSE;  /* flag to indicate that severe errors
                                     were found while parsing the input */
/* local copies of collation values */
char	co_nord;                                /* # of collation orders */
wchar_t co_wc_min;                              /* min process code */
wchar_t co_wc_max;                              /* max process code */
wchar_t co_col_min;                             /* min coll weight */
wchar_t co_col_max;                             /* max coll weight */
wchar_t *collname;                              /* collating name */
char *mbcollname;                               /* multibyte version of collating name */
wchar_t *mccollist=0;                           /* list of multi-character collating elements */
int     mccollisti=0;
wchar_t *nmccollist=0;                          /* inverse list of multi-character collating elements */
int     nmccollisti=0;
wchar_t *mccollset=0;                           /* set of multi-character collating elements */
int     mccollseti=0;
int     mcsize;
ccltypedef *cclarray;                           /* array of pointers to character classes */
int     cclarrayi = 0;                          /* index to cclarray */

short   yytext_type;                                /* true if yytext is to be a pointer */
int     cclarray_size = 0;                      /* maximum character class size */
short	nohormmsg = 0;                          /* flag for NOHORM message so only printed once */
short   noxmsg = 0;                          /* flag for NOX message so only printed once */

#endif /* _H_ONCE */
