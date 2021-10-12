/* @(#)01	1.9  src/bos/usr/bin/adb/defs.h, cmdadb, bos411, 9428A410j 2/24/94 12:35:12 */
#ifndef  _H_DEFS
#define  _H_DEFS
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  No functions, just declarations.
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
 *  Common definitions
 */

#include "machdep.h"
#include <core.h>
#include <fcntl.h>
#include <setjmp.h>
#include <a.out.h>
#include <stdio.h>
#ifdef FREAD
/* FREAD is defined within fcntl.h, but not used here.  Need ldfcn.FREAD */
#undef FREAD
#endif
#ifdef FWRITE
/* FWRITE is defined within fcntl.h, but not used here.  Need ldfcn.FWRITE */
#undef FWRITE
#endif
#include <ldfcn.h>
#undef BUFSIZ
#define BUFSIZ 1024
#include <stdio.h>
#include <ctype.h>
#include "mode.h"
#include "functs.h"
#include "extern.h"
#include "sys.h"
#include "lib.h"

#define VARNO 36
#define VARB  11
#define VARD  13
#define VARE  14
#define VARM  22
#define VARS  28
#define VART  29

#define NSP      0
#define ISP      1
#define DSP      2
#define NSYM     0
#define ISYM     4
#define DSYM     8
#define IDSYM    12
#define ENDSYMS  (-1)
#define EXTRASYM (-2)

#define BKPTSET  1
#define BKPTEXEC 2

/*
 *  Ptrace mode defines
 */
#define SETTRC PT_TRACE_ME
#define RIUSER PT_READ_I
#define RDUSER PT_READ_D
#define RUREGS PT_READ_U
#define WIUSER PT_WRITE_I
#define WDUSER PT_WRITE_D
#define CONTIN PT_CONTINUE
#define EXIT   PT_KILL
#define RREG   PT_READ_GPR
#define WREG   PT_WRITE_GPR
#define RBLOCK PT_READ_BLOCK
#define WBLOCK PT_WRITE_BLOCK

/* Where some systems have PT_SINGSTEP... */
#define SINGLE 9

#define U_AR0 ((long *)adbreg)
#define ADBREG(A) (XL(U_AR0[A]))
#define LVADBREG(A) (U_AR0[A])
/* This must not match any real register offset */
#define NOTREG    (-1)

#define DBNAME  "adb\n"

#define LPRMODE "%R"
#define OFFMODE "+%R"

/* Symbol table in a.out */
#define SYMBOLSIZE  SYMESZ

#define DEFRADIX        16
#define MAXARG          32
#define LINSIZ         256
#define STAR             4
#define MAXOFF         255
#define MAXPOS          80
#define MAXLIN         128
#define STARREDCMD  0x8000
#define LOBYTE        0xff
#define EVEN         (~01)
#define NOTFND 0x7fffffffL
#define MAXPROMPT       32

#ifndef TRUE
#define TRUE   1
#define FALSE  0
#endif

#if DEBUG
#define TRPRINT(S) if (traceflag & 1) (void)fprintf(stderr, S)
#define TR1PRINT(S, V) if (traceflag & 1) (void)fprintf(stderr, S, V)
#else
#define TRPRINT(S)
#define TR1PRINT(S, V)
#endif

#if 0 /* use this if libIN.a is available */
#undef AOcanon
extern int   AOcanon();
#endif

#ifdef CROSSDB
extern short XXshort();
extern long  XXlong();
#define	XH(a)		XXshort((a), CPU)
#define	XL(a)		XXlong((a), CPU)
#else
#define	XH(a)		((short)(a))
#define	XL(a)		((long)(a))
#endif

#define	symfirstchar(p)	((p)->n_zeroes ? (p)->n_name[0] : strtab[(p)->n_offset])

#ifndef LOCAL
#define LOCAL static
#endif

#ifndef LINK
#define LINK LR
#endif

#ifndef _FP
#define _FP STKP
#endif

#ifndef _PCP
#define _PCP 14
#endif
#endif  /* _H_DEFS */
