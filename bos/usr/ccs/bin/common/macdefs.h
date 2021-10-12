/* @(#)54       1.5.1.1  src/bos/usr/ccs/bin/common/macdefs.h, cmdprog, bos411, 9428A410j 4/29/93 08:10:45 */
/*
 * COMPONENT_NAME: (CMDPROG) macdefs.h
 *
 * FUNCTIONS: CCTRANS, ENUMSIZE, FIXDEF, FIXSTRUCT, IsRegVar, SETDCON, makecc
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

/*
 *  @OSF_COPYRIGHT@
*/
/* AIWS C compiler */

#ifndef TWOPASS
#define ONEPASS
#endif

# define TRUE 1
# define FALSE 0

extern char *getmem();
        /* chars are unsigned */
# define makecc(val,i)  lastcon = (lastcon<<8)|val;

# define ARGINIT 0      /* args start at 0(ap)      */
# define AUTOINIT 0     /* autos start at 0(STKPTR) */
# define SZCHAR 8
# define SZINT 32
# define SZFLOAT 32
# define SZDOUBLE 64
# define SZLDOUBLE 64
# define SZLNGLNG 64
# define SZLONG 32
# define SZSHORT 16
# define SZPOINT 32
# define ALCHAR 8
# define ALINT 32
# define ALFLOAT 32
# define ALDOUBLE 32
# define ALLDOUBLE 32
# define ALLNGLNG 32
# define ALLONG 32
# define ALSHORT 16
# define ALPOINT 32
# define ALSTRUCT 8
# define ALSTACK 32
# define ALFTN 16       /* see bfcode() */

/*      size in which constants are converted */
/*      should be long if feasable */

# define CONSZ long long
# define UCONSZ unsigned long long
# define FCONSZ double
# define CONFMT "%ld"

/*      size in which offsets are kept */
/*      should be large enough to cover address space in bits */

# define OFFSZ unsigned

/*      character set macro */

# define  CCTRANS(x) x

/* register cookie for stack poINTer */

#         define MINRVAR  7     /* 7 free registers! + 1 saved scratch */
#         define MAXRVAR 13
#         define STKREG  22     /* pseudo-registers, xlated by adrput() */
#         define ARGREG  23
#         define NARGREG 24
#         define MAXTEMPS 6     /* killed registers: 0,2,3,4,5,15 */
#ifdef XCOFF
#         define RRMAX    9     /* round robin register pool size */
#else
#         define RRMAX   10     /* round robin register pool size */
#endif
#         define REGSZ 16+6+3   /* normal, float, psuedo-registers  */

# define IsRegVar(x) ((x) >= MINRVAR && (x) <= MAXRVAR )

# define TMPREG   STKREG
# define MINFPREG 16
# define MAXFPREG 21
# define MINFPVAR 18            /* 4 fp register vars */

# define FORCEREG 2             /* usual register for FORCE nodes */
# define FORCEFLOATREG 16       /* register for arith if FORCE */

        /* various standard pieces of code are used */
# define STDPRTREE
# define LABSIZE 10     /* number of chars in label - used to malloc space */
# define LABFMT "L.%d"

/* show stack grows positively */
#undef BACKAUTO
#undef BACKTEMP

/* show no field hardware support on Kimono */
/* or at least not much */
#undef FIELDOPS

/* bytes are numbered from left to right */
#define LTORBYTES

/* we want prtree included */
# define STDPRTREE

# define ENUMSIZE(high,low) INT

# define FIXDEF(p) fixdef(p)
# define FIXSTRUCT(p,q) strend(p)
# define SETDCON(p) 1           /*  setdcon(p) no need for side effects */
# define LAST_P2_LABEL  32767   /* f77 uses 16 bits for labels */

/* This character introduces an assembler comment */

#define ASM_CCHAR       '#'

/* The name of the General Data CSect */
#define DATA_SECT       "..STATIC"
