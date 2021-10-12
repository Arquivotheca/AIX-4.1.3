/* @(#)90	1.8.1.4  src/bos/usr/ccs/lib/libdbx/POWER/decode.h, libdbx, bos411, 9433A411a 8/10/94 18:08:25 */
#ifndef _h_decode
#define _h_decode
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: FPREGNO, FREGTOSYS, INSLEN, INSTRLEN, REGTOSYS, SYSREGNO,
 *	      WORDALIGN, XISL, exec_object, nargspassed
 *
 * ORIGINS: 26, 27
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
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
#include <sys/reg.h>
#include <filehdr.h>
typedef unsigned int Address;
typedef unsigned char Byte;
typedef unsigned int Word;

#define NGREG  32
#define NFREG  32
#define NSYS   9   /* iar, machine state, CR, link, CTR, XER, MQ, TID, FPSCR */
#define FPSTATREG 1	/* Indicates that there is a special FP status reg */
#define NREG   (NGREG+NSYS+fpregs+1)	/* number of registers + status */
#define MAXNREG NREG 	        /* maximum number of registers + status */
#define MAXFREG NFREG	        /* maximum number of registers + status */

#define FRP 1

#ifndef PROGCTR
#define PROGCTR 128
#endif

#define SYSREGNO(regno)	 (regno - IAR + NGREG)
#define FPREGNO(regno)	 (regno - FPR0)
#define REGTOSYS(regno)	 (regno + IAR - NGREG)
#define FREGTOSYS(regno) (regno + FPR0)

/* #define STKP 1 */

#define CODESTART 0x10000000
#define FUNCOFFSET 0

#define BITSPERBYTE 8
#define BITSPERWORD (BITSPERBYTE * sizeof(Word))
#define WORDALIGN(a) ((a) & ~03)

#define INSTRLEN(op) 4
#define INSLEN(op)   4
#define INST_SZ      4
#define MIN_INST_SZ  4
#define XISL(op)     0

#define nargspassed(frame) argcomp(frame)
#define TBSIZE	     5		/* Number of words of relevant traceback */
				/* table information 			 */

#define BP_ERRNO    SIGTRAP     /* signal received at a breakpoint */

#define MAXCOM	128
#define MAXARG	128
#define LINSIZ	NCARGS

typedef	unsigned long	POS;
typedef	struct tmap	TMAP;
typedef	struct dmap	DMAP;
typedef	TMAP		*TMAPPTR;
typedef	DMAP		*DMAPPTR;

/* file address maps */
/* Old Style */
struct dmap {
	unsigned long	b1;	/* beginning (internal) address */
	unsigned long	e1;	/* ending (internal) address	*/
	unsigned long	f1;	/* file address corresponding to b1 */
	unsigned long	b2;	/* beginning (internal) address) */
	unsigned long	e2;	/* ending (internal) address	*/
	unsigned long	f2;	/* file address corresponding to b2 */
	int	ufd;	/* UNIX file descriptor		*/
};
/* New Style */
struct xmap {
	unsigned long	b1;	/* beginning (internal) address */
	unsigned long	e1;	/* ending (internal) address	*/
	unsigned long	f1;	/* file address corresponding to tb */
	struct brbuf	*bfd;	/* bio buffer for io */
	char 		*name;	/* name of text object */
};
#define	NXMAPS		12
struct tmap {
	struct   xmap   xmap[NXMAPS];/* Maps for text segments */
	unsigned long	b2;	/* beginning (internal) address) */
	unsigned long	e2;	/* ending (internal) address	*/
	unsigned long	f2;	/* file address corresponding to b2 */
	int	ufd;	/* UNIX file descriptor		*/
};

typedef long Bpinst;

extern Bpinst BP_OP;
extern char *regnames[];
extern char *regnames_601[];	/* Registers to be used for 601 architecture */
extern char *regnames_ppc[];	/* Registers to be used for ppc architecture */

#define exec_object(mag) (((mag) == U802WRMAGIC) || \
			  ((mag) == U802ROMAGIC) || \
			  ((mag) == U802TOCMAGIC))

#include "source.h"
#include "symbols.h"

extern Address pc;
extern Address prtaddr;
extern Address printop (/* addr */);
extern Address nextaddr (/* startaddr, isnext */);
extern beginproc (/* p, argc, ptrtofuncaddr */);
extern integer extractField (/* s, rangetype */);
extern loophole (/* oldlen, newlen */);
#endif /* _h_decode */
