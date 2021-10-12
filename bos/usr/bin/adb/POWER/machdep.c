static char sccsid[] = "@(#)20        1.21  src/bos/usr/bin/adb/POWER/machdep.c, cmdadb, bos411, 9428A410j 6/19/91 18:18:18";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: ISLONG, argcomp, getdebug, inprolog, kcsflags, kheader, pcpsym,
 * 	      stktrace, unwind
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
#include <sys/debug.h>
#include "adb_msg.h"
extern nl_catd  scmc_catd;

#define MAXPROCSIZE 10000
#define FIRSTREGVAR 13
#define MAXSAVEREG  15

#define BI          0x88
#define JI          0x00
#define STM         0xd9
#define CAL         0xc8
#define NOP_NARGS   0x08
#define CAL11xxxx   0xc8110000
#define ISLONG(op)  ( ((op&0xe0)==0xc0) || ((op&0xf0)==0x80) )
#define D_NOFRAMESAVE 9

/*
 * SIGF_ADJUST is the amount the stack is adjusted by the prolog of the
 * general signal processor (getsig or kgetsig).  It used to be 72, then
 * 156, then 348.  It would be nice if this could be calculated somehow,
 * instead of hard-coded?  See also libc/csu/crt0.s for a description of
 * what a signal frame looks like.
 */
#define SIGF_ADJUST   348
#define SIGF_SAVREG   0
#define SIGF_SIGNO    (SIGF_ADJUST+0)
#define SIGF_CONTEXT  (SIGF_ADJUST+8)
#define SIGC_SAVFP    8
#define SIGC_SAVPC    12

typedef unsigned int Address ;
typedef unsigned int Word;

#ifdef _NO_PROTO
LOCAL void  argcomp();
LOCAL struct tbtable *getdebug();
LOCAL BOOL inprolog();
LOCAL  struct tbtable  *readtable();
#else
LOCAL void  argcomp(register struct tbtable *, unsigned long);
LOCAL struct tbtable *getdebug(register unsigned long);
LOCAL BOOL inprolog(register long);
LOCAL  struct tbtable  *readtable(Address);
#endif  /* _NO_PROTO */

extern  int debugflag; /*dbg*/
extern int filndx;
extern int coredump;
int kheader(h)
register FILHDR *h;
{
    if (!AOcanon(h))
	return (0);
    return (h->f_magic);
}

void pcpsym(i)
long i;
{
    register SYMPTR symp;
    char *p;

    if (i == 0 || findsym(dot-2, ISYM) == NOTFND ||
	    symfirstchar(&symbol) != '.')
	return;
/*
 * this is a awful hack, flexstr is called so that a copy is made
 * so that we can step on the first element of the copy
 */
    p = flexstr(&symbol, filndx);
    p[0] = '_';
    if ((symp = lookupsym(p)) != NULL) {
	adbpr("%8t# ");
	psymoff(get(symp->n_value + i, (slshmap.ufd > 0 ? DSP : ISP)),
		IDSYM, "");
    }
}

LOCAL char *kcsbits[] = {
    "pz",
    "lt",
    "eq",
    "gt",
    "c0",
    "reserve",
    "ov",
    "ts"
};

void kcsflags(cs)
register unsigned long cs;
{
    register int i;
    register long mask;

    for (i = 0, mask = 0x800000; i < 8; i++, mask >>= 1)
	if (cs&mask)
	    adbpr(" %s", kcsbits[i]);
}

#ifdef _NO_PROTO
void stktrace(modif)
char modif;
#else
void stktrace(char modif)
#endif
{
    	long          callpc, regs[NREGS];
    	long          oldcallpc, oldregs[NREGS];
    	int           narg;
    	long          argp;
    	register int  i;
        int loop = 0;
	struct tbtable *dfp;
	unsigned long backchain = 0;
        unsigned long looppc = 0;
        unsigned long frame = 0;
        unsigned long oldframe = 0;
	unsigned long loopfp = 0;
	/* _FP contains the value of current frame pointer */
	frame = ADBREG(_FP); 
	callpc = ADBREG(SYSREGNO(IAR));
	for (i=0; i<NREGS; ++i) regs[i] = ADBREG(i);

	looppc = callpc;
	loopfp = frame;
	backchain = get (frame , DSP );
        loop = 0;
    	while ( backchain ) {
	   if ( cntflg && ( ! cntval--)) 
		return;
	   if ( ++loop > 2000 )
		goto loop1 ;
	   oldframe = backchain;
	   oldcallpc = callpc;
	   for (i=0; i<NREGS; i++) oldregs[i] = regs[i];
	   dfp = unwind((unsigned long *)&backchain, (unsigned long *)&callpc, 
			(unsigned long *) regs);

	   if ( ! dfp ) {
	        adbpr(catgets(scmc_catd, MS_machdep, M_MSG_74,
			"can't unwind the stack\n"));
	        return;
	   }
	   /* Now get the function name and the no of arguments */
    	   adbpr("%s(", (findsym(oldcallpc, ISYM) == NOTFND ? "?" :
				flexstr(&symbol, filndx) ));

	   argcomp(dfp,oldframe);
	   adbpr(") ");
	   psymoff(callpc, IDSYM, "\n");
	   if (modif == 'C' && findsym(oldcallpc, ISYM) != NOTFND) {
	      while (localsym(frame, oldcallpc)) {
		long word = get(localval, DSP);
		adbpr("%8t%s:%10t", flexstr(&symbol, filndx));
		if (errflg != NULL) {
		    prints("?\n");
		    errflg = NULL;
		} else {
		    adbpr("%R\n", word);
		}
	      }
	   }

	   frame = backchain;
           if ( ! dfp->tb.saves_lr )
           	backchain = get ( frame, DSP );
	   else
		backchain = get ( backchain, DSP);
	   if ( dfp )
    	   	 free( (char *)dfp );
	}
        return;
loop1 : adbpr(catgets(scmc_catd, MS_machdep, M_MSG_75,
	"internal error: cannot recover from stack frames.\n"));
	adbpr("backchain = %#x, infinite loop or loop = %d\n", backchain, loop);
	callpc = looppc;
	frame  = loopfp;
}

/*
 * Return whether there is new information in this portion of the stack.
 */
STACKSTATUS stackstatus(frame, caller, regs) 
long *frame;
long *caller;
long *regs;
{
    struct tbtable *dfp;
    dfp = unwind((unsigned long *)frame,(unsigned long *) caller, 
		 (unsigned long *) regs);
    if ( ! dfp )
    	return  BADSTACK;
    *frame  = get ( (unsigned long) (*frame), DSP );
    free( (char *) dfp );
    return NEWSTACK;
}
	
struct tbtable *unwind(fp, pc, regs)
unsigned long *fp;
unsigned long *pc;
unsigned long *regs;
{
    register struct tbtable *trcbk;       /* pointer to trace back table */
    register long          context;
    register int           i;
    int			   regoffset;
    unsigned    gpr_saved, fpr_saved;
    unsigned long   link_register;
    if (debugflag)
    	adbpr("unwind(fp=%X, pc=%X, regs=%X)\n",
						fp, *pc, regs);/*dbg*/
    link_register = ADBREG(SYSREGNO(LINK));
    if (debugflag)
    	adbpr("link = %X\n", link_register );
    trcbk = getdebug(*pc);
    if ( !trcbk )  {
	return 0 ;
    }

    if (debugflag)
	adbpr("traceback table is found\n");
    gpr_saved = trcbk->tb.gpr_saved; /* total no. of gpr saved */
    fpr_saved = trcbk->tb.fpr_saved;
    regoffset = *fp - 8 * fpr_saved ; 

#if DEBUG
    if (debugflag)
	adbpr("gpr_saved = %X, fpr_saved = %X, regoffset = %X\n",
		gpr_saved, fpr_saved, regoffset); 
#endif

    /* Get the register values from the stack */
    for(i=0; i < gpr_saved; i++) {
	if ( i < 13 )
	   regs[i] = get(*fp + 24 + i, DSP);
	else
	   regs[i] = get(*fp - regoffset - (i - 13), DSP );
    }

    /* Get the floating point registers */
    if ( fpr_saved )
        for (i = 0; i < fpr_saved; i++) 
	     regs[i] = get(*fp - 64 * i, DSP );
 
    if ( coredump || trcbk->tb.saves_lr ) 
	/*fp+8 is the position of the LINK reg */
        *pc = get(( (unsigned long) *fp + 8), DSP); 
    else
  	*pc =  link_register;
  	if (debugflag)
	adbpr("pc = %X \n", pc);

#if DEBUG
    if ( debugflag ) adbpr("fp: %R call pc: %R\n", *fp, *pc);
#endif
    return (trcbk);
}

/* getdebug() will return pointer to the trace back table. Trace back table is
   obtained by scanning forward from the current PC. The beginning of
   the table is marked by a fullword of zeros, which is an illegal RT follow
   on instruction. The trace back table is word aligned.
*/

#define	DFMAGIC	 0x00000000
#define WORDALIGN(a)   ( (a) & ~03 )
#define ValidText(addr)  ( (addr) >= 0x10000000 )

LOCAL struct tbtable *getdebug(pc)
register unsigned long pc;
{
    register Address trace_addr;
    Address   *beginaddr, *endaddr;
    struct tbtable *tracetable;
    long  trctbl, curpc;
    unsigned searchend = 0xffffffff;
    int looksbad = 0;
    long badread = -2; /* this is not initialized to zero, because we are 
			  looking zero as a returned value. Also, it is not 
			  initialized to -1, this is to avoid the -1 returned 
			  by ptrace */

    register long limit = pc + MAXPROCSIZE;


    if (debugflag)
    adbpr("getdebug(pc=%X)\n", pc ); /*dbg*/

    /* The search for traceback table starts here. */
    for (trace_addr = WORDALIGN(pc); trace_addr < searchend; 
				trace_addr += sizeof(Word) ) {
	if ( !ValidText(trace_addr) ) {
		adbpr("warning: could not locate the trace back table from starting address %X\n", trace_addr );
		return FALSE;
    	}
     	badread = get(trace_addr, ISP);
	if ( badread == 0 ) { /* we found the trace back table */
	     tracetable = readtable(trace_addr + 4);	
	     looksbad = 0;
	     return tracetable;
	}
    } /* end of for */

    adbpr(catgets(scmc_catd, MS_machdep, M_MSG_76,
	"adb could not find the trace back table\n"));
    return (struct tbtable *) 0; /* you should never reach here */
}

/* readtable() reads the trace back table. */
LOCAL struct tbtable *
readtable(addr)
Address addr;
{
   struct tbtable * tbl_tmp;
   char * tb;
   unsigned int i;
    
   tbl_tmp = (struct tbtable *) malloc( (unsigned) sizeof(struct tbtable));
   tb = (char *)tbl_tmp;
   for(i=0; i < sizeof(struct tbtable); i++ ) {
	*tb = cget(addr+i, ISP);
	tb++;
   }
   return tbl_tmp;
}
/* 
    FUNCTION: gettype wil return the type info of a particular parameter
    Input   : 
	value : used for holding the value of parmtype field of traceback			table.
	position: used for finding out which bit you want in the parmtype,

    Return Value:  A bit at a given location.
    Output : None.
*/

LOCAL unsigned short gettype(value, position)
unsigned value;     	/* word used for holding the parmtype */
unsigned short position; 
{
	return ( value >> (position + 1 - 1) & ~(0xffffffff << 1) );
}

/*
	FUNCTION: argcomp

	Input Parameters: pointer to traceback table, current stack pointer.
	Output: The no. of parameters a function can have.
	Return Value: none.
	
	Data structure: The only structure being used here is the traceback 
	   table, i.e. struct tbtable, which is defined in the sys/debug.h.
	
	Assumption: A valid traceback table exists.

	Algorithm: First find the total number of parameters.
		   for each parameter find out whether it is a fixed 
		   or a floating point parameter, then find out
		   the location of the parameter, i.e. whether it
		   resides in GPR, FPR, or on the stack. Then get
		   its value from the appropriate place.
		   If there is a valid core file, then read the 
		   arguments from the stack present in the core file.

	Convention: If there are more than 8 fixed parameters then
		    these parameters are passed on the stack.
*/

	
LOCAL void argcomp(tb, fp)
register struct tbtable *tb;
unsigned long fp;
{
    int fixed_reg;         /* Used for keeping track of fixed point register */
    int float_reg;	   /* Used for keeping track of floating point regs */
    int i, param_value, next_bit;
    unsigned  short type_bit = 31; 
    double dbl_val;
    float  float_val;
    unsigned short type;
    unsigned long stkptr = fp + 24; /* fp+24 is the place where first argument
				       goes on the stack */
    int total_params = 0;

    total_params =  tb->tb.fixedparms + tb->tb.floatparms;

    fixed_reg = 3; /* fixed_reg params from GPR3 ... GPR10 */
    float_reg = 1; /* float_reg params from FPR01 ... FPR13 */

    for ( i=0; i < total_params; i++) {
	if ( (31 - i) < 0 ) {
	/* This part of the loop is executed only when all the arguments
	   from parmcntv are printed out.
	*/
	       param_value = get(stkptr, DSP);
	       stkptr = stkptr + 4;
	       adbpr("%X", param_value);
	       continue;
	}
        type = gettype(tb->tb_ext.parminfo, type_bit--);
	if ( type == 0 ) {
	   /* This is a fixed (4 byte) parameter. If there are more than
	      8 fixed point parameters then they are passed on the stack.
	   */
	  if ( ( !tb->tb.parmsonstk) && fixed_reg <= 10 )
			    /* The number of parameters is less than 8,
			       read the parameter value from registes */
		if ( !coredump )
	       		param_value = ptrace(PT_READ_GPR, pid, fixed_reg, 0);
		else  {
			/*param_value = get(stkptr, DSP); */
			param_value = adbreg[fixed_reg];
			stkptr = stkptr + 4;
		}
	   else {
		if ( coredump )
			param_value = get(stkptr, DSP);
		else
	       		param_value = ptrace(PT_READ_D, pid, stkptr, 0);
	       stkptr = stkptr + 4;
	   }
	   fixed_reg++;
	   adbpr("%X", param_value);
	}
	else {
	   /* This is a floating point parameter, check for the 
	      next bit in tb->parmtype, 
	          next bit 0 : floating point short parameter
	          next bit 1 : floating point long parameter
	   */
	   next_bit = gettype(tb->tb_ext.parminfo,type_bit--);
	   if ( next_bit ) {
	      if ( (! tb->tb.parmsonstk) &&  float_reg <= 12 ) {
 		 dbl_val = ptrace(PT_READ_FPR, pid, float_reg, 0, 0 );
		 float_reg++;
	      }
	      else {
		 union { char w[8]; double d_val; } data;
		 int i;
		 char dbuf1[10];
		 extern int errno;

		 if ( ! coredump ) {
		     errno = 0;
 		     param_value = ptrace(PT_READ_BLOCK, pid, stkptr, 8, dbuf1);
		     if ( errno != 0 ) adbpr("error in reading 8 blocks\n");
		 }
	      	 else {
		     char * tmp = (char *)&param_value;
		     param_value = get(stkptr, DSP);
		     for( i = 0; i < 4; i++) 
			dbuf1[i] = *tmp++;
		     param_value = get(stkptr+4, DSP);
		     tmp = (char *)&param_value;
		     for( i = 4; i < 8; i++) 
			dbuf1[i] = *tmp++;
		}
	 	 stkptr = stkptr + 8;  /* the stk here is double word aligned */
		 for( i = 0; i < 8; i++ )
			 data.w[i] = dbuf1[i];
		 dbl_val = data.d_val;
	      }
	      adbpr("%f", dbl_val);
	      fixed_reg += 2; /* Skip the next two fixed point registers. */
	   }
	   else {
		if ( (! tb->tb.parmsonstk) && float_reg <=12 ) {
 		   float_val = ptrace(PT_READ_FPR, pid, float_reg, 0, 0 );
		   fixed_reg += 1; /* Skip the next fixed point registers. */
		   float_reg++;
		}
		else {
 		   param_value = ptrace(PT_READ_D, pid, stkptr, 0, 0 );
		   float_val = param_value;
		   stkptr = stkptr + 4;
	        }
	        adbpr("%f", float_val);
	   }
 	}
    	if ( i < (total_params-1) )
 		adbpr(",");
    } /* end of for */
}

/*
 * inprolog: Check whether fp has been initialized yet
 */
LOCAL BOOL inprolog(tempcallpc)
register long tempcallpc;
{
    return (FALSE);
}
