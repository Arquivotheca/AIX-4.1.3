/* @(#)05	1.10  src/bos/usr/bin/adb/functs.h, cmdadb, bos411, 9428A410j 10/6/93 15:11:03 */
#ifndef _H_FUNCTS
#define _H_FUNCTS
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Result type declarations
 */

#ifdef _NO_PROTO
/* access.c */

unsigned char  cget();
void           cput();
unsigned long  get();
void           put();
unsigned short sget();
void           sput();

/* command.c */

void command();

/* expr.c */

BOOL   expr();
BOOL   getnum();
SYMPTR lookupsym();
int    varchk();

/* format.c */

STRING exform();
long   inkdot();
void   scanform();
void   shell();

/* input.c */

BOOL eocmd();
void getformat();
int  nextchar();
char quotchar();
char rdc();
char readchar();

void           kcsflags();
int            kheader();
void           pcpsym();
void           stktrace();
STACKSTATUS    stackstatus();
#ifdef _IBMRT
struct dfdata *unwind();
#endif
#ifdef _POWER
struct tbtable *unwind();
#endif

/* main.c */

void chkerr();
void done();
void error();
long round();

/* opcode.c */

void printins();

/* output.c */

#ifndef _NO_PROTO
void
adbpr(STRING , ...);
#else
#if STRICT_VARARGS
void
adbpr(va_alist) ;
#else
void adbpr();
#endif
#endif
int  charpos();
void flushbuf();
void iclose();
void newline();
void oclose();
void printc();
void prints();
void prompt();

/* pcs.c */

void subpcs();

/* print.c */

void dollar();
int  getreg();
void printpc();
void sigprint();

/* runpcs.c */

void  delbp();
void  endpcs();
int   getsig();
int   runpcs();
BKPTR scanbkpt();
void  setup();

/* setup.c */

void setcor();
void setsym();

/* sym.c */

unsigned int findsym();
char         *flexstr();
BOOL         localsym();
void         psymoff();
void         skip_aux();
SYMPTR       symget();
void         symset();
void         valpr();
#else    /* _NO_PROTO */
/* access.c */

unsigned char  cget(long, int);
void           cput(long, int, long);
unsigned long  get(long, int);
void           put(long, int, long);
unsigned short sget(long, int);
void           sput(long, int, long);

/* command.c */

void command(STRING, char);

/* expr.c */

BOOL   expr(int);
BOOL   getnum();
SYMPTR lookupsym(STRING);
int    varchk(int);

/* format.c */

STRING exform(int, STRING, int, int);
long   inkdot(int);
void   scanform(int, STRING, int, int);
void   shell();

/* input.c */

BOOL eocmd(char);
void getformat(STRING);
int  nextchar();
char quotchar();
char rdc();
char readchar();

/* machdep.c  */
void           kcsflags();
int            kheader(register FILHDR *);
void           pcpsym(long);
void           stktrace(char);
STACKSTATUS    stackstatus(long *, long *, long *);
#ifdef _IBMRT
struct dfdata *unwind();
#endif
#ifdef _POWER
struct tbtable *unwind(unsigned long *,unsigned long *,unsigned long *);
#endif

/* main.c */

void chkerr();
void done();
void error(STRING);
long round(long, long);
int chkdir(char *);

/* opcode.c */

void printins();

/* output.c */

#ifndef _NO_PROTO
void
adbpr(STRING , ...);
#else
#if STRICT_VARARGS
void
adbpr(va_alist) ;
#else
void adbpr();
#endif
#endif
int  charpos();
void flushbuf();
void iclose();
void newline();
void oclose();
void printc(char);
void prints(char *);
void prompt();

/* pcs.c */

void subpcs(char);

/* print.c */

void dollar(int);
int  getreg(int);
void printpc();
void sigprint();

/* runpcs.c */

void  delbp();
void  endpcs();
int   getsig(int);
int   runpcs(int, int);
BKPTR scanbkpt(long);
void  setup();

/* setup.c */

void setcor();
void setsym();

/* sym.c */

unsigned int findsym(long, int);
char         *flexstr(register SYMPTR, int);
BOOL         localsym(long, long);
void         psymoff(long, int, char *);
void         skip_aux(SYMPTR, int);
SYMPTR       symget(int);
void         symset(int);
void         valpr(long, int);

#endif  /*  _NO_PROTO */
#endif  /* _H_FUNCTS */
