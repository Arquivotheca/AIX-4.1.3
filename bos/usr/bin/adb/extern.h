/* @(#)03	1.6.1.1  src/bos/usr/bin/adb/extern.h, cmdadb, bos411, 9428A410j 10/6/93 15:29:18 */
#ifndef  _H_EXTERN
#define  _H_EXTERN
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  No functions
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  External data declarations
 */

extern ADBMSG      ADWRAP;
extern ADBMSG      BADCOM;
extern ADBMSG      BADDAT;
extern ADBMSG      BADEQ;
extern ADBMSG      BADFIL;
extern ADBMSG      BADKET;
extern ADBMSG      BADLOC;
extern ADBMSG      BADMAGIC;
extern ADBMSG      BADMOD;
extern ADBMSG      BADNAM;
extern ADBMSG      BADRAD;
extern ADBMSG      BADSYM;
extern ADBMSG      BADSYN;
extern ADBMSG      BADTXT;
extern ADBMSG      BADVAR;
extern ADBMSG      BADWAIT;
extern ADBMSG      ENDPCS;
extern ADBMSG      EXBKPT;
extern ADBMSG      LONGFIL;
extern ADBMSG      NOADR;
extern ADBMSG      NOBKPT;
extern ADBMSG      NOCFN;
extern ADBMSG      NOEOR;
extern ADBMSG      NOFORK;
extern ADBMSG      NOMATCH;
extern ADBMSG      NOPCS;
extern ADBMSG      NOTOPEN;
extern ADBMSG      SZBKPT;

extern int      adbreg[];
extern int      adrflg;
extern long     adrval;
extern int      argcount;
extern BKPTR    bkpthead;
extern BOOL     cntflg;
extern int      cntval;
extern STRING   corfil;
extern int      datsiz;
extern int      ditto;
extern long     dot;
extern int      dotinc;
extern int      entrypt;
extern int      eof;
extern STRING   errflg;
extern BOOL     executing;
extern long     expv;
extern int      fcor;
extern int      fsym;
extern int      infile;
extern char     lastc;
extern int      lastcom;
extern long     localval;
extern long     locval;
extern int      loopcnt;
extern STRING   lp;
extern short    magic;
extern int      maxfile;
extern int      maxoff;
extern int      maxpos;
extern unsigned maxstor;
extern BOOL     mkfault;
extern int      outfile;
extern char     peekc;
extern int      pid;
extern char     promptstr[];
extern MAP      qstmap;
extern int      radix;
extern REGLIST  *reglist;
extern REGLIST  reglist_pwr[];
extern REGLIST  reglist_601[];
extern void     (*sigint)();
extern int      signo;
extern void     (*sigqit)();
extern MAP      slshmap;
extern int      stksiz;
extern long     symbas;
extern SYMTAB   symbol;
extern STRING   symfil;
extern int      symnum;
extern SYMSLAVE *symvec;
extern int      txtsiz;
extern int      var[];
extern int      wtflag;
extern int      data_scn_num;
extern int      bss_scn_num;
extern int      text_scn_num;
extern int      dbg_scn_num;
#if DEBUG
extern unsigned traceflag;
#endif

/* additions */
extern LDFILE   *txtstream;
extern char     *strtab;
extern char     *dbgtab;
extern double	fpregs[];

#endif  /* _H_EXTERN */
