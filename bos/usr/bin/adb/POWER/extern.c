static char sccsid[] = "@(#)19	1.9.1.2  src/bos/usr/bin/adb/POWER/extern.c, cmdadb, bos411, 9433A411a 8/10/94 16:53:36";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  No functions.
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

#include "defs.h"

#if DEBUG
unsigned traceflag = 0;         /* for debugging printout */
#endif

ADBMSG ADWRAP   = "address wrap around";
ADBMSG BADCOM   = "bad command";
ADBMSG BADDAT   = "data address not found";
ADBMSG BADEQ    = "unexpected `='";
ADBMSG BADFIL   = "bad file format";
ADBMSG BADKET   = "unexpected ')'";
ADBMSG BADLOC   = "automatic variable not found";
ADBMSG BADMAGIC = "bad core magic number";
ADBMSG BADMOD   = "bad modifier";
ADBMSG BADNAM   = "not enough space for symbols";
ADBMSG BADRAD   = "must have 2<= radix <= 16";
ADBMSG BADSYM   = "symbol not found";
ADBMSG BADSYN   = "syntax error";
ADBMSG BADTXT   = "text address not found";
ADBMSG BADVAR   = "bad variable";
ADBMSG BADWAIT  = "wait error: process disappeared!";
ADBMSG ENDPCS   = "process terminated";
ADBMSG EXBKPT   = "too many breakpoints";
ADBMSG LONGFIL  = "filename too long";
ADBMSG NOADR    = "address expected";
ADBMSG NOBKPT   = "no breakpoint set";
ADBMSG NOCFN    = "c routine not found";
ADBMSG NOEOR    = "newline expected";
ADBMSG NOFORK   = "try again";
ADBMSG NOMATCH  = "cannot locate value";
ADBMSG NOPCS    = "no process";
ADBMSG NOTOPEN  = "cannot open";
ADBMSG SZBKPT   = "bkpt: command too long";

int	 adbreg[NREGS+NKREGS-1];	/* adb's register set */
int      adrflg;              /* TRUE if user specified an address */
long     adrval;              /* The address so specified */
int      argcount;            /* Number of supplied files */
BKPTR    bkpthead = 0;        /* Boints to start of bkpt linked list */
BOOL     cntflg;              /* TRUE if user specified an count */
int      cntval;              /* The count so specified */
STRING   corfil = "core";     /* Name of the core file being used */
int      datsiz;              /* Size of data segment; default from core */
int      ditto;               /* Dot when command was entered */
long     dot;                 /* Current address */
int      dotinc;              /* Value by which dot is incremented */
int      entrypt;             /* Entry point for object file */
int      eof;                 /* Result of last read from < input */
STRING   errflg;              /* Error message */
BOOL     executing;           /* TRUE when command is ":" */
long     expv;                /* Result of evaluating an expression */
int      fcor;                /* Core file file descriptor */
int      fsym;                /* a.out file descriptor */
int      infile;              /* File descriptor for redirected input */
char     lastc = '\n';        /* Result of last read */
int      lastcom = '\n';      /* Type of last command */
long     localval;            /* Address of a local variable */
long     locval;
int      loopcnt;             /* Process execution counter */
STRING   lp;                  /* Points at current char in input line */
short    magic;               /* Magic number for object file */
int      maxfile = 1<<24;     /* Maximum length of object file */
int      maxoff;              /* Max offset for symbolic addresses */
int      maxpos;              /* Maximum print width on output */
unsigned maxstor = 4<<28;     /* Maximum storage address */
BOOL     mkfault;             /* Counts interrupts */
int      outfile = 1;         /* File descriptor for user output file */
char     peekc;
int      pid;                 /* Processid of user program */
char     promptstr[MAXPROMPT];
MAP      qstmap;              /* ? map */
int      radix = DEFRADIX;    /* Current output radix */
/*
 * Register description information, in three parts.
 * Numbered regs must be first, in the obvious order, so the decompiler can
 * use the bits in an instruction to index this table.  The "$r" command prints
 * the first two parts in reverse order (so the lowest regs are at the bottom
 * of the screen).  The "<" and ">" commands match names from all three parts.
 */
REGLIST  *reglist=reglist_pwr;

REGLIST  reglist_pwr[NREGS+NKREGS+NREGSYNONYMS] = {
    "r0",   GPR0,
    "r1",   GPR1,
    "r2",   GPR2,
    "r3",   GPR3,
    "r4",   GPR4,
    "r5",   GPR5,
    "r6",   GPR6,
    "r7",   GPR7,
    "r8",   GPR8,
    "r9",   GPR9,
    "r10",  GPR10,
    "r11",  GPR11,
    "r12",  GPR12,
    "r13",  GPR13,
    "r14",  GPR14,
    "r15",  GPR15,
    "r16",  GPR16,
    "r17",  GPR17,
    "r18",  GPR18,
    "r19",  GPR19,
    "r20",  GPR20,
    "r21",  GPR21,
    "r22",  GPR22,
    "r23",  GPR23,
    "r24",  GPR24,
    "r25",  GPR25,
    "r26",  GPR26,
    "r27",  GPR27,
    "r28",  GPR28,
    "r29",  GPR29,
    "r30",  GPR30,
    "r31",  GPR31,

    "iar",  SYSREGNO(IAR),
    "msr",  SYSREGNO(MSR),
    "cr",   SYSREGNO(CR),
    "lr",   SYSREGNO(LR),
    "ctr",  SYSREGNO(CTR),
    "xer",  SYSREGNO(XER),
    "mq",   SYSREGNO(MQ),
    "tid",  SYSREGNO(TID),
    "fpscr",  SYSREGNO(FPSCR),

    "pc",   SYSREGNO(IAR),
    "fp",   STKP,
    "stkp", STKP,
    "pcp",  _PCP,
    "toc",  TOC,
    "link", SYSREGNO(LR),
};
REGLIST  reglist_601[NREGS+NKREGS+NREGSYNONYMS] = {
    "r0",   GPR0,
    "r1",   GPR1,
    "r2",   GPR2,
    "r3",   GPR3,
    "r4",   GPR4,
    "r5",   GPR5,
    "r6",   GPR6,
    "r7",   GPR7,
    "r8",   GPR8,
    "r9",   GPR9,
    "r10",  GPR10,
    "r11",  GPR11,
    "r12",  GPR12,
    "r13",  GPR13,
    "r14",  GPR14,
    "r15",  GPR15,
    "r16",  GPR16,
    "r17",  GPR17,
    "r18",  GPR18,
    "r19",  GPR19,
    "r20",  GPR20,
    "r21",  GPR21,
    "r22",  GPR22,
    "r23",  GPR23,
    "r24",  GPR24,
    "r25",  GPR25,
    "r26",  GPR26,
    "r27",  GPR27,
    "r28",  GPR28,
    "r29",  GPR29,
    "r30",  GPR30,
    "r31",  GPR31,

    "iar",  SYSREGNO(IAR),
    "msr",  SYSREGNO(MSR),
    "cr",   SYSREGNO(CR),
    "lr",   SYSREGNO(LR),
    "ctr",  SYSREGNO(CTR),
    "xer",  SYSREGNO(XER),
    "mq",   SYSREGNO(MQ),
    NULL,   SYSREGNO(TID),
};
REGLIST  reglist_ppc[NREGS+NKREGS+NREGSYNONYMS] = {
    "r0",   GPR0,
    "r1",   GPR1,
    "r2",   GPR2,
    "r3",   GPR3,
    "r4",   GPR4,
    "r5",   GPR5,
    "r6",   GPR6,
    "r7",   GPR7,
    "r8",   GPR8,
    "r9",   GPR9,
    "r10",  GPR10,
    "r11",  GPR11,
    "r12",  GPR12,
    "r13",  GPR13,
    "r14",  GPR14,
    "r15",  GPR15,
    "r16",  GPR16,
    "r17",  GPR17,
    "r18",  GPR18,
    "r19",  GPR19,
    "r20",  GPR20,
    "r21",  GPR21,
    "r22",  GPR22,
    "r23",  GPR23,
    "r24",  GPR24,
    "r25",  GPR25,
    "r26",  GPR26,
    "r27",  GPR27,
    "r28",  GPR28,
    "r29",  GPR29,
    "r30",  GPR30,
    "r31",  GPR31,

    "iar",  SYSREGNO(IAR),
    "msr",  SYSREGNO(MSR),
    "cr",   SYSREGNO(CR),
    "lr",   SYSREGNO(LR),
    "ctr",  SYSREGNO(CTR),
    "xer",  SYSREGNO(XER),
    NULL,   SYSREGNO(MQ),
    NULL,   SYSREGNO(TID),
};
void      (*sigint)();         /* Current interrupt handling routine */
int      signo;               /* Signal that caused process halt */
void      (*sigqit)();         /* Current quit handling routine */
MAP      slshmap;             /* / map */
int      stksiz;              /* Size of stack from core file */
int      txtsiz;              /* Size of text segment; default from core */
long     symbas;              /* A.out file address of symbol table */
SYMTAB   symbol;              /* I/O buffer for a.out symbols */
STRING   symfil = "a.out";    /* Name of the object file being used */
int      symnum;              /* Number of symbols in symbol table */
SYMSLAVE *symvec;             /* Beginning of slave array */
int      var[VARNO];          /* Holds values of adb variables */
int      wtflag = O_RDONLY;   /* Set to O_RDWR if user gave -w flag */
int      data_scn_num = 0;    /* section number for data section */
int      bss_scn_num = 0;     /* section number for bss section */
int      text_scn_num = 0;    /* section number for text section */
int      dbg_scn_num = 0;     /* section number for .debug section */

struct user u;                /* User block in core file */

/* additions */
LDFILE  *txtstream = NULL;    /* file stream for text */
char    *strtab;
char    *dbgtab;
double 	fpregs[MAXFPREGS+1];  /* to store the values of floating point
				 registers */
