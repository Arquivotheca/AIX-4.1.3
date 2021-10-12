/* @(#)12       1.9.1.12  src/bos/usr/ccs/bin/lex/ldefs.h, cmdlang, bos411, 9428A410j 6/10/94 12:24:08 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
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
#ifndef _H_LDEFS
#define _H_LDEFS
# define _ILS_MACROS     /* 139709 - use macros for better performance */
# include <stdio.h>
# include <stdlib.h>
# include <locale.h>
# include <nl_types.h>
# include <string.h>
# include <wstring.h>
# include <sys/types.h>
# include <sys/localedef.h>
# include <sys/lc_core.h>
# include <sys/limits.h>
# include <ctype.h>
# include "hash.h"

#ifndef  _BLD                                   /* kept for compatibility with osf code */
#include "lex_msg.h"
#define MSGSTR(Num, Str) catgets(catd,MS_LEX,Num,Str)
extern nl_catd catd;
#else
#define MSGSTR(Num,Str) Str
#endif

# define PP 1
# define CWIDTH 8

# define NCH 256

# define TOKENSIZE      1000
# define DEFSIZE        40
# define DEFCHAR        1000
# define STARTCHAR      1024
# define STARTSIZE      256
# define CCLARRAY_SIZE  1000
# ifdef SMALL
# define TREESIZE       600
# define NTRANS         1500
# define NSTATES        300
# define MAXPOS         1500
# define NOUTPUT        1500
# define NWHSPACE       20                      /* 20% vacancy of hash table */
# endif

# ifndef SMALL
# define TREESIZE       2000
# define NSTATES        2500
# define MAXPOS         5000
# define NTRANS         5000
# define NOUTPUT        5000
# define NWHSPACE       20                      /* 20% vacancy of hash table */
# endif
# define NACTIONS       100
# define ALITTLEEXTRA   100

/*
 * Definitions for lex rule nodes.
 */

# define RCCL   NCH+90                          /* CCL rule */
# define RNCCL  NCH+91                          /* NCCL rule */
# define RSTR   NCH+92                          /* STR rule */
# define RSCON  NCH+93                          /* SCON rule */
# define RNEWE  NCH+94                          /* NEWE rule */
# define FINAL  NCH+95                          /* done everything */
# define RNULLS NCH+96                          /* NULLS rule */
# define RCAT   NCH+97                          /* catenation rule */
# define STAR   NCH+98
# define PLUS   NCH+99
# define QUEST  NCH+100
# define DIV    NCH+101
# define BAR    NCH+102
# define CARAT  NCH+103
# define S1FINAL NCH+104                        /* end definitions section */
# define S2FINAL NCH+105                        /* end rules section */
# define RWCHAR NCH+106                         /* WCHAR rule */

# define DEFSECTION 1
# define RULESECTION 2
# define ENDSECTION 5
# define TRUE 1
# define FALSE 0

# define PC 1
# define PS1 1
/*
 * Definitions for special character classes, these must be single bit values and must be
 * in the 8-bit range as they are used in wmatch, which contains wide (MB) character codes
 * and cannot conflict.
 */
# define CCLDOT         0x0001
# define CCLALNUM       0x0002
# define CCLALPHA       0x0004
# define CCLBLANK       0x0008
# define CCLCNTRL       0x0010
# define CCLDIGIT       0x0020
# define CCLGRAPH       0x0040
# define CCLLOWER       0x0080
# define CCLPRINT       0x0100
# define CCLPUNCT       0x0200
# define CCLSPACE       0x0400
# define CCLUPPER       0x0800
# define CCLXDIGIT      0x1000
/*
 * debugging stuff
 */
# ifdef DEBUG
# define LINESIZE 100
extern int yydebug;
extern int debug;               /* 1 = on */
extern int charc;
# endif

# ifndef DEBUG
# define freturn(s) s
# endif

/*
 * collation stuff
 */
#define EXISTSCOLLEL(wc) __OBJ_DATA(__lc_collate)->co_coltbl[wc].ct_collel != (_LC_collel_t *)NULL
#define CE_SYM(wc, i) __OBJ_DATA(__lc_collate)->co_coltbl[wc].ct_collel[i].ce_sym
#define CE_WGT(wc, i, order) __OBJ_DATA(__lc_collate)->co_coltbl[wc].ct_collel[i].ce_wgt.n[order]
#define WGT_STR(index, order) __OBJ_DATA(__lc_collate)->co_subs[index].tgt_wgt_str[order]
#define COLLWGT(wc, order) __OBJ_DATA(__lc_collate)->co_coltbl[wc].ct_wgt.n[order]
#define WGT_STR_INDEX(wc) COLLWGT(wc, 1)
#define CE_WGT_STR_INDEX(wc, i) CE_WGT(wc, i, 1)
#define EXISTS_WGT_STR(colval) (colval == _SUB_STRING)
#define COLLNAME_SIZE 32
#define MCCOLL_SIZE 500

extern int UNIQ_ORDER;

#define ISOCTAL(wc) ((wc >= '0') && (wc <= '7'))
#define ISHEX(wc) (((wc >= '0') && (wc <= '9')) || ((wc >= 'a') && (wc <= 'f')) || ((wc >= 'A') && (wc <= 'F')))

struct cclstruct {
    short len;
    wchar_t *cclptr;
};

typedef struct cclstruct ccltypedef;

extern int      sargc;
extern char     **sargv;
extern wchar_t  buf[520];
extern int      ratfor;         /* 1 = ratfor, 0 = C */
extern int      cplusplus;      /* 1 = C++, 0 = ratfor|C */
extern int      yyline;         /* line number of file */
extern int      sect;
extern int      eof;
extern int      lgatflg;
extern int      divflg;
extern int      funcflag;
extern int      pflag;
extern int      casecount;
extern int      chset;                          /* 1 = char set modified */
extern FILE     *fin, *fout, *fother, *errorf;
extern int      fptr;
extern char     *ratname, *cname;
extern int      prev;                           /* previous input character */
extern int      pres;                           /* present input character */
extern int      peek;                           /* next input character */
extern int      *name;
extern wchar_t  *wname;                         /* wide char name table */
extern int      *left;
extern int      *right;
extern int      *parent;
extern wchar_t  *nullstr;
extern int      tptr;
extern wchar_t  pushc[TOKENSIZE];
extern wchar_t  *pushptr;
extern wchar_t  slist[STARTSIZE];
extern wchar_t  *slptr;
extern wchar_t  **def, **subs, *dchar;
extern wchar_t  **sname, *schar;
extern short    *excl;
extern wchar_t  *dp, *sp;
extern int      dptr, sptr;
extern wchar_t  *bptr;                          /* store input position */
extern wchar_t  *tmpstat;
extern int      count;
extern int      **foll;
extern int      *nxtpos;
extern int      *positions;
extern int      *gotof;
extern int      *nexts;
extern wchar_t  *nchar;
extern int      **state;
extern int      *sfall;                         /* fallback state num */
extern char     *cpackflg;                      /* true if state has been character packed */
extern int      *atable, aptr;
extern int      nptr;
extern wchar_t  symbol[NCH];
extern int      wsymbollen;
extern int      wsymboli;
extern wchar_t  *wsymbol;
extern wchar_t  cindex[NCH];
extern int      xstate;
extern int      stnum;
extern int      ctable[];
extern int      ZCH;
extern int      ccount;
extern wchar_t  match[NCH];
extern wchar_t  *extra;
extern int      nactions;               /* Current size of extra */
extern wchar_t  *pcptr, *pchar;
extern int      pchlen;
extern int      nstates, maxpos;
extern int      yytop;
extern int      report;
extern int      ntrans, treesize, outsize, whspace;
extern long     rcount;
extern int      optim;
extern int      *stoff;
extern int      *verify, *advance;
extern int      wcranksize, wmatchsize, xcclsize;
extern hash_t   *wcrank, *wmatch;
extern xccl_t   *xccl;
extern int      xccltop;
extern int      wmatchlist;
extern int      scon;
extern wchar_t  *psave;
extern wchar_t  *getl();
extern void     *myalloc();
extern int      buserr(), segviol();
extern int      mbcurmax;
extern int      multibytecodeset;
extern int      defsize;
extern int      defchar;
extern int      yaccerror;
extern char     co_nord;
extern wchar_t  co_wc_min;
extern wchar_t  co_wc_max;
extern wchar_t  co_col_min;
extern wchar_t  co_col_max;
extern wchar_t  *collname;
extern char     *mbcollname;
extern wchar_t  *mccollist;
extern int      mccollisti;
extern wchar_t  *nmccollist;
extern int      nmccollisti;
extern wchar_t  *mccollset;
extern int      mccollseti;
extern int      mcsize;
extern ccltypedef  *cclarray;
extern int      cclarrayi;
extern short    yytext_type;
extern int      cclarray_size;
extern short    nohormmsg;
extern short    noxmsg;
#endif /* _H_LDEFS */
