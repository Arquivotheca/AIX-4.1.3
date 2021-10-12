static char sccsid[] = "@(#)40    1.59.3.68  src/bos/usr/ccs/lib/libdbx/eval.c, libdbx, bos41J, 9510A_all 2/20/95 12:39:59";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: assign
 *		binary_op
 *		boolean_operation
 *		canpush
 *		changepc
 *		chkenable
 *		chksp
 *		cond
 *		dereference
 *		do_operation
 *		do_unary_operation
 *		eval
 *		evalcmdlist
 *		func
 *		getcase
 *		gripe
 *		ispic
 *		isredirected
 *		isspecialpic
 *		list
 *		listi
 *		lowercase
 *		lval
 *		popintarg
 *		poplonglong
 *		poplonglongarg
 *		poprealarg
 *		popsmall
 *		prflregs
 *		prhexflt
 *		printpc
 *		pushregvalue
 *		pushsmall
 *		registers
 *		rpush
 *		screen
 *		set
 *		setout
 *		stop
 *		stop1
 *		stopinst
 *		stopvar
 *		topeval
 *		touch_sym
 *		trace
 *		trace1
 *		traceall
 *		traceat
 *		tracedata
 *		traceinst
 *		traceproc
 *		unary_op
 *		unary_operation
 *		unsetout
 *		uppercase
 *		watch
 *
 *   ORIGINS: 26,27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Tree evaluation.
 */

#include "defs.h"
#include "envdefs.h"
#include "tree.h"
#include "operators.h"
#include "debug.h"
#include "eval.h"
#include "events.h"
#include "symbols.h"
#include "scanner.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#include "main.h"
#include "frame.h"
#include "execute.h"
#include "resolve.h"
#include "cplusplus.h"
#include "cma_thread.h"
#if defined (CMA_THREAD) || defined (K_THREADS)
#include "k_thread.h"
#endif /* CMA_THREAD || K_THREADS */
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/mode.h>
#include "ops.h"

#ifdef KDBX
public Boolean new_command = true; /* if command = "\n" => reset to false */
public int is_diss;	/* set to true while disassembling */
#endif /* KDBX */

public Stack stack[STACKSIZE];
public Stack *sp = &stack[0];
public Boolean useInstLoc = false; /* are we doing instruction step or next? */
public Boolean ScrUsed = false;   /* Indicates oper switched screen  */
public cases casemode = filedep;
public Boolean indirect = false;  /* Indicates indirect addressing mode */
public Address assign_addr = NOADDR;
public Address assign_size = 0;
public Boolean ptrtomemberfunction = false;
public boolean notderefed = true;

/*  filecmdcursrc is set to cursource in eval for the 
      file command.  The purpose for this is so we
      know whether or not to use the "use" path when
      the user enters a fullpath name  */
char *filecmdcursrc = NULL;

extern Node findvar();
extern int stepcount;
extern double pow();
extern int sourcefiles;
extern int inclfiles;
extern int *envptr;			/* setjmp/longjmp data */
extern int eventId;
extern boolean isunsignedpic();
extern Boolean is_fortran_padded();
extern int lazy;
extern int initializestring;

private traceall();
private traceinst();
private traceat();
private traceproc();
private tracedata();
private stopinst();
private stopvar();
private set();
private list();
private listi();
private func();
int ScrPid;   /* the pid of the process
                 which open a virtual terminal for screen command */
extern int newdebuggee;

/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
};
extern struct subdim *subdim_head;
extern struct subdim *subdim_tail;
boolean subarray;				/* eval'ing a subarray */
extern Address array_addr;
extern Symbol  array_sym;
extern boolean specificptrtomember;
extern Symbol Subar_save_sym;
extern Address Subar_offset;
extern Ttyinfo ttyinfo;  

#define isfunc(s) ((s->class == FUNC) || (s->class == CSECTFUNC))

#define chksp() \
{ \
    if (sp < &stack[0]) { \
	panic( catgets(scmc_catd, MS_eval, MSG_77, "stack underflow")); \
    } \
}

#define poprealarg(n, fr, qr) { \
    Symbol nt; \
    eval(p->value.arg[n]); \
    nt = p->value.arg[n]->nodetype; /* Save, fixCobol*Arg may change it */ \
    fixCobolFloatArg(p->value.arg[n]); \
    if (size(p->value.arg[n]->nodetype) == sizeof(float)) { \
      fr = pop(float); \
    } else if (size(p->value.arg[n]->nodetype) == sizeof(double)) { \
      fr = pop(double); \
    } else { \
      qr.val[1] = pop(double); \
      qr.val[0] = pop(double); \
    } \
    p->value.arg[n]->nodetype = nt; /* Restore it for later evaluation */ \
}
#define popintarg(n, r) { \
    Symbol nt; \
    eval(p->value.arg[n]); \
    nt = p->value.arg[n]->nodetype; /* Save, fixCobol*Arg may change it */ \
    fixCobolIntArg(p->value.arg[n]); \
    r = popsmall(p->value.arg[n]->nodetype); \
    p->value.arg[n]->nodetype = nt; /* Restore it for later evaluation */ \
}
#define poplonglongarg(n, r) { \
    Symbol nt; \
    eval(p->value.arg[n]); \
    nt = p->value.arg[n]->nodetype; /* Save, fixCobol*Arg may change it */ \
    fixCobolIntArg(p->value.arg[n]); \
    r = poplonglong(p->value.arg[n]->nodetype); \
    p->value.arg[n]->nodetype = nt; /* Restore it for later evaluation */ \
}

#define Boolrep char	/* underlying representation type for booleans */

/*  this macro is used for the '-' and '~' unary
      operators.  It should be used if there is
      ever a new unary operator that returns the
      same type as its operand  */
#define do_unary_operation(op)                   \
  switch(operand_type)                           \
  {                                              \
    case ISLONG:                                 \
      push(long, op r0);                         \
      break;                                     \
    case ISUNSIGNED:                             \
      push(unsigned long, op ur0);               \
      break;                                     \
    case ISLONGLONG:                             \
      push(LongLong, op llr0);                   \
      break;                                     \
    case ISUNSIGNEDLONGLONG:                     \
      push(uLongLong, op ullr0);                 \
      break;                                     \
  }                                              

/*  this macro is used for the (double) and '!'
      unary operators.  It should be used for
      any unary operator that will return a
      type independent of the operand type  */
#define unary_operation(result_type, op)         \
  switch(operand_type)                           \
  {                                              \
    case ISLONG:                                 \
      push(result_type, op r0);                  \
      break;                                     \
    case ISUNSIGNED:                             \
      push(result_type, op ur0);                 \
      break;                                     \
    case ISLONGLONG:                             \
      push (result_type, op llr0);               \
      break;                                     \
    case ISUNSIGNEDLONGLONG:                     \
      push (result_type, op ullr0);              \
      break;                                     \
  }                                              

/*  this macro should be used for any binary
      operation that returns the same type
      as its operands  */
#define do_operation(op)                         \
  switch(operand_type)                           \
  {                                              \
    case ISLONG:                                 \
      push(long, r0 op r1);                      \
      break;                                     \
    case ISUNSIGNED:                             \
      push(unsigned long, ur0 op ur1);           \
      break;                                     \
    case ISLONGLONG:                             \
      push(LongLong, llr0 op llr1);              \
      break;                                     \
    case ISUNSIGNEDLONGLONG:                     \
      push(uLongLong, ullr0 op ullr1);           \
      break;                                     \
  }                                              

/*  this macro should be used for any binary
      operation that returns a Boolrep  */
#define boolean_operation(op)                    \
  switch(operand_type)                           \
  {                                              \
    case ISLONG:                                 \
      push(Boolrep, r0 op r1);                   \
      break;                                     \
    case ISUNSIGNED:                             \
      push(Boolrep, ur0 op ur1);                 \
      break;                                     \
    case ISLONGLONG:                             \
      push (Boolrep, llr0 op llr1);              \
      break;                                     \
    case ISUNSIGNEDLONGLONG:                     \
      push (Boolrep, ullr0 op ullr1);            \
      break;                                     \
  }                                              

#define ispic(t) (((t)->class == PIC) || ((t)->class == RPIC))
#define isspecialpic(t) ((((t)->class == PIC) || ((t)->class == RPIC)) &&\
			 (((t)->symvalue.usage.storetype >= 'a') &&\
			  ((t)->symvalue.usage.storetype <= 'q')))


/* Read local debug info for file of variable */
public touch_sym(s)
Symbol s;
{
  Symbol file_sym;

  if (s->class != MODULE) {
     for (file_sym = s;
          (file_sym != nil) && (file_sym->class != MODULE);
           file_sym = file_sym->block);
             if (file_sym == nil)
          return;
  } else {
    file_sym = s;
  }
  if (file_sym->symvalue.funcv.untouched)
     touch_file(file_sym);
}


/*
 * Command-level evaluation.
 */

public Node topnode;

public topeval(p)
Node p;
{
    if (traceeval) 
    {
	(*rpt_error)(stderr, "topeval(");
	prtree(rpt_error, stderr, p);
	(*rpt_error)(stderr, ")\n");
    }
    eval(topnode = traverse(p, 0));

    cpp_emptyVirtualList();
    if( topnode != p ) {
	tfree( p );
    }
    tfree( topnode );
}

/*
 * NAME: eval
 *
 * FUNCTION: Evaluate a parse tree leaving the value on the
 *           top of the stack.
 *
 * PARAMETERS:
 *      p      - input Node  
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: nothing 
 */

#define SETLEN 32

public eval (p)
register Node p;
{
    int i;
    char *rr;
    long r0, r1;
    Address addr = 0;
    long n;
    unsigned len = 0;
    Symbol s;
    Node n1, n2;
    boolean b;
    File file;
    String str;
    int TEMPCNT;
    int errcond;
    int pid;
    ExecStruct	**dpi_info;
    struct TraceStruct	trdata;
    Boolean isFPTEE = false;
    long *athisptr, *newthis;
    Symbol theclass, theptrmem, ptrtomemclasstype;

#ifdef KDBX
    int nbp;
#define MAX_XCALL_ARGS 4
    long xcall_args[MAX_XCALL_ARGS];
    extern do_switch();
    extern do_cpu();
    extern use_local_breaks;	/* When set, the breakpoints set in the */
				/* kernel debugger are local, to ensure */
				/* a right behavior of step and next */

    use_local_breaks = 0;
    is_diss = 0;
#endif /* KDBX */

    checkref(p);
    if (traceeval) {
	(*rpt_error)(stderr, "begin eval %s\n", opname(p->op));
    }

  switch (degree(p->op)) {
    case BINARY:
      binary_op(p);
      break;

    case UNARY:
      unary_op(p);
      break;

    default:
      /*  handle everything that is not unary or binary  */
      switch (p->op) {
	case O_SYM:
	    s = p->value.sym;
 
	    if (lazy)           /* If in lazy reading mode,            */
		touch_sym(s);   /* read local debug info automatically */
	    if (s == retaddrsym) {
		push(long, return_addr());
	    } else if (isvariable(s)) {
                if (s != program && s->storage != EXT && 
                                                !isactive(container(s))) {
		    error( catgets(scmc_catd, MS_eval, MSG_82,
					  "\"%s\" is not active"), symname(s));
		}

                if (is_f90_sym(s))
                  s = p->value.sym = convert_f90_sym(s, NULL);

                /* Process fortran "Cray" pointee variable         */
                /* Actual address is stored in its pointer (chain) */
                if (s->class == FPTEE) {
                   s = s->chain;
                   isFPTEE = true;
                }
		addr = address(s, nil);
		if (isvarparam(s)) {
		    if (s->storage == INREG || preg(s, nil) != -1) {
			pushregvalue(s, addr, nil, sizeof(Address));
		    } else {
		        rpush(addr, sizeof(Address));
		    }
		} else {
		    push(Address, addr);
		}
	        /* If we are dealing with pascal file type, all we have  */
	        /* is the address of the pascal file type descriptor     */
	        /* block. We need to look into the structure and get the */
		/* real address of the file buffer. At this point, this  */
		/* address is fixed at 96 bytes off the top for XLP.     */
                if (rtype(s->type)->class == FILET and
                    s->language == pascalLang)
		{
		  addr = pop(Address);
                  addr = addr + 96;
		  push(Address, addr);
                }
		else {
                  if (s->param && rtype(s)->class == CLASS &&
                      !(rtype(s)->symvalue.class.passedByValue)) 
		  {
                     dereference();
                     notderefed = true;
                  }
                }
                if (isFPTEE) {
                   addr = pop(Address);
                   rpush(addr, sizeof(Address));
                   isFPTEE = false;
                }
	    } else if (s->class == COMMON) {
		addr = s->symvalue.common.com_addr;
		push(Address, addr);
            } else if (isblock(s) || (s->class == LABEL &&
                                      s->language == pascalLang)) {
		push(Symbol, s);
	    } else if (isconst(s)) {
		eval(constval(s));
	    } else {
		error( catgets(scmc_catd, MS_eval, MSG_83,
					 "cannot evaluate a %s"), classname(s));
	    }
	    break;

	case O_LCON:
	case O_CCON:
	    r0 = p->value.lcon;
	    pushsmall(size(p->nodetype), r0);
	    break;

        case O_ULCON:
            push(unsigned long, p->value.lcon);
            break;

        case O_LLCON:
            push(LongLong, p->value.llcon);
            break;

        case O_ULLCON:
            push(uLongLong, p->value.llcon);
            break;

	case O_FCON:
	    push(double, p->value.fcon);
	    break;

	case O_QCON:
	    push(double, p->value.qcon.val[0]);
	    push(double, p->value.qcon.val[1]);
	    break;

	case O_KCON:
	    push(double, p->value.kcon.real);
	    push(double, p->value.kcon.imag);
	    break;

	case O_QKCON:
	    push(double, p->value.qkcon.real.val[0]);
	    push(double, p->value.qkcon.real.val[1]);
	    push(double, p->value.qkcon.imag.val[0]);
	    push(double, p->value.qkcon.imag.val[1]);
	    break;

	case O_SETCON:
	    rr = (char *) p->value.lcon;
	    if (p->nodetype->type->class == RANGE)
 	    {
	      len = SETLEN;
  	    }
            else 
	    if (p->nodetype->type->class == SCAL)
            {
	      len = p->nodetype->type->symvalue.iconval;
 	      len = (len + BITSPERBYTE - 1) / BITSPERBYTE; 
            }
  	    for (i=0; i<len; i++)
	    {
               pushsmall(1, *rr);
	       rr++;
	    }
            break;
 
	case O_SCON:
	    len = size(p->nodetype);
	    mov(p->value.scon, sp, len);
	    sp += len;
	    break;

	case O_INDEX:
            s = p->value.arg[0]->nodetype;
            p->value.arg[0]->nodetype = t_addr;

	    if (p->value.arg[1] != nil &&
		p->value.arg[1]->op == O_DOTDOT) { /* have a subarray */
		Node q = p->value.arg[0];
		while(q->op == O_INDEX)  /* look past multi-D array stuff  */
		    q = q->value.arg[0];
		if (q->op != O_DOT &&	 /* for structs and pointers:   */
		    q->op != O_INDIR &&  /*   only set subarray *after* */
                    q->op != O_INDIRA && /*   eval of left node         */
                    q->op != O_ADD &&    /*   O_ADD is for ptr to array */
                    q->op != O_DOTSTAR)
		    subarray = true;
		eval(p->value.arg[0]);
		subarray = true;
	    }
	    else
                eval(p->value.arg[0]);

            p->value.arg[0]->nodetype = s;

	    if (!subarray)
            {
	       n = pop(Address);
	       eval(p->value.arg[1]);
	       evalindex(s, n, popsmall(p->value.arg[1]->nodetype));
            }
	    else
	    {
		struct subdim *last_ptr = nil;
		struct subdim *ptr = nil;

		while (p->op == O_INDEX)

		{
		   ptr = (struct subdim *)malloc(sizeof(struct subdim));
		   if (!subdim_head)
		      subdim_head = ptr;
		   if (last_ptr)
		      last_ptr->next = ptr;
		   /*
		    * Temporarily turn this off so it doesn't 
		    * interfere with the upcoming calls to eval
		    */
		   subarray = false;
		   if (p->value.arg[1]->op == O_DOTDOT)
		   {
		      Node lower = p->value.arg[1]->value.arg[0];
		      Node upper = p->value.arg[1]->value.arg[1];

		      eval(lower);
		      if (isconst(lower->nodetype))
			ptr->lb =
			  (long) popsmall(constval(lower->nodetype)->nodetype);
		      else 
			ptr->lb = (long) popsmall(lower->nodetype);
		      eval(upper);
		      if (isconst(upper->nodetype))
			ptr->ub = 
			  (long) popsmall(constval(upper->nodetype)->nodetype);
		      else 
			ptr->ub = (long) popsmall(upper->nodetype);
		      ptr->back = last_ptr;
		      ptr->next = nil;
		   }
		   else
		   {
		      eval(p->value.arg[1]);
		      ptr->lb = ptr->ub = (long) pop(long);
		      ptr->back = last_ptr;
		      ptr->next = nil;
		   }
		   subarray = true;    /* turn it back on */
		   p = p->value.arg[0];
		   last_ptr = ptr;
		}
		subdim_tail = last_ptr;
		array_sym  = p->nodetype;
                array_addr = pop(Address);
		push(Address, array_addr);
	    }
	    break;

	case O_DOT:
	    s = p->value.arg[1]->value.sym;
            if (p->nodetype->class == FUNC) {
               push(Address, p->nodetype);
            }
            else {
               if (s->class == FUNC) {
                  push(Address, s->symvalue.funcv.beginaddr);
               }
               else {
                  int Subar_of;
	          eval(p->value.arg[0]);
	          n = pop(long);

	          if (s->class == BASECLASS) 
                  {
	             push(long, n + (s->symvalue.baseclass.offset))
                     Subar_of=(s->symvalue.baseclass.offset);
                  }
	          else
                  {
	             push(long, n + (s->symvalue.field.offset / 8));
                     Subar_of=(s->symvalue.field.offset / 8);
                  }
                  if(subarray)
                        Subar_offset=n+Subar_of;
               }
            }
	    break;

        case O_DOTSTAR:
            theclass = p->value.arg[0]->nodetype;
            theptrmem = p->value.arg[1]->nodetype;
                                 /* get to the pointer to member symbol     */
            s = rtype(theptrmem);
                                 /* evaluate the address of the specific    */
                                 /* instance of the class                   */
            eval(p->value.arg[0]);
            if (rtype(theptrmem)->class == CPPREF) {
               theptrmem = rtype(theptrmem)->type;
            }
                                /* get the address of the instace (this ptr)*/
            athisptr = (long *) pop(long);
                                /* evaluate the member pointer portion of   */ 
                                /* the expression                           */
            eval(p->value.arg[1]);
                                 /* we are evaluating a specific instance   */
            specificptrtomember = true;
                                /* if we are processing a pointer to member */
                                /* function then
                                /* leave function and this pointers on stack */
            if (s->type->class == FFUNC) {
                long function;
                long ttdisp, ppdisp, ffdisp;
                struct ve {
                     long tdisp;
                     long faddr;
                } *ventry;
                ptrtomemberfunction = true;
                                /* determine what type of pointer to member  */
                                /* function we are using                     */
                                /* see C++ "mapping.document" for details on */
                                /* the following formats                     */
                if (s->symvalue.ptrtomem.hasVBases) {   /* mvp3 format */
                    ppdisp = pop(long);
                    ttdisp = pop(long);
                    rpush((char *)athisptr + ppdisp,sizeof(long));
                    newthis =  (long *) ((char *) pop(long));
                    newthis = (long *) ((char *) newthis + ttdisp);
                }
                else {
                     pop(long);                /* get rid of ppdisp         */
                     if (s->symvalue.ptrtomem.hasMultiBases) {
                         ttdisp = pop(long);
                         newthis = (long *)((char *) athisptr + ttdisp);
                     }
                     else {
                         pop(long);           /* get rid of ttdisp          */
                         newthis = athisptr;
                     }
                }
                ffdisp = pop(long);
                function = pop(long);
                if (ffdisp == -1 && function == 0) {
                     specificptrtomember = false;
                     ptrtomemberfunction = false;
                     beginerrmsg();
                     (*rpt_error)(stderr, catgets(scmc_catd, MS_eval, MSG_84,
					  "reference through nil pointer"));
                     enderrmsg();
                }
                if (s->type->symvalue.member.attrs.func.isVirtual
                             != CPPVIRTUAL) {
                     rpush(function,sizeof(long));
                     push(long,newthis);
                }
                else {
                     ventry = (struct ve*)((char *)(*newthis) + ffdisp);
                     rpush(ventry->faddr, sizeof(long));
                     rpush(ventry->tdisp, sizeof(long));
                }
            }
            else {
                long mmdisp, ppdisp;
                                /* otherwise we are processing a pointer to */
                                /* a data element -- just leave the pointer */
                                /* to the data on the stack                 */
                if (!(s->symvalue.ptrtomem.hasVBases)) {
                   pop(long);   /* get rid of ppdisp                        */
                   mmdisp = pop(long);
                   if (mmdisp == -1) {
                      specificptrtomember = false;
                      beginerrmsg();
                      (*rpt_error)(stderr, catgets(scmc_catd, MS_eval, MSG_84,
					   "reference through nil pointer"));
                      enderrmsg();
                   }
                   push(long, ((char *) athisptr + mmdisp));
                }
                else {
                   ppdisp = pop(long);
                   mmdisp = pop(long);
                   if (mmdisp == -1) {
                      specificptrtomember = false;
                      beginerrmsg();
                      (*rpt_error)(stderr, catgets(scmc_catd, MS_eval, MSG_84,
					   "reference through nil pointer"));
                      enderrmsg();
                   }
                   rpush((char *) athisptr + ppdisp, sizeof(long));
                   newthis =  (long *) ((char *) pop(long));
                   newthis = (long *) ((char *) newthis + mmdisp);
                   push (long, newthis);
                }
            }
            break;

	case O_COMMA:
	    eval(p->value.arg[0]);
	    if (p->value.arg[1] != nil) {
		eval(p->value.arg[1]);
	    }
	    break;

	case O_ASSIGN:
	    assign(p->value.arg[0], p->value.arg[1]);
#ifdef CMA_THREAD
	    /* If we have cma threads, we need to alert libpthreads */
	    /* that we might have changed something...              */
	    if (!isfinished(process) && running_thread)
	       setInconsistency();
#endif /* CMA_THREAD */
	    break;

	case O_CHFILE:
	    if (p->value.scon == nil) {
              char *filepointer;

              /*  find the path of the current file  */
              filepointer = findsource (cursource, NULL); 

              if (filepointer == NULL)
                filepointer = cursource;

              /*  strip off the beginning "./" if it exists  */
              if (!strncmp(filepointer, "./", 2))
                filepointer += 2;

              /*  print out the path of the current file  */
	      (*rpt_output)(stdout, "%s\n", filepointer);
	    } else {
                unsigned char search_filetable = FILE_CMD;

                if (lazy) {
                    char *x, *y;
                    Symbol filesym;
                    x = strdup(p->value.scon);
                    y = rindex(x, '.');
                    if (y) *y = '\0';
                    filesym = lookup(identname(x,true));
                    touch_sym(filesym);
                }
		file = opensource(p->value.scon, &search_filetable);
		if (file == nil) {
		    error( catgets(scmc_catd, MS_eval, MSG_90,
					 "cannot read \"%s\""), p->value.scon);
		} else {
		    fclose(file);
                    /*  if cursource did not get set in findsource  */ 
                    if (search_filetable)
                    {
                      /*  set it here  */
		      setsource(p->value.scon);
                      filecmdcursrc = cursource;
		    }
		}
		action_mask |= LISTING;
	    }
	    break;

	case O_CONT:
#if defined (CMA_THREAD) || defined (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    cont(p->value.arg[0]->value.lcon);
	    if (!isstopped)
	       printnews(false);
	    action_mask |= EXECUTION;
	    action_mask &= ~CONTEXT_CHANGE;
	    action_mask &= ~LISTING;
	    action_mask &= ~ELISTING;
	    break;

	case O_KLOAD:
#ifdef KDBX
	    update_load_maps();
#endif
	    break;

	case O_LISTI:
	    listi(p);
	    break;

	case O_LIST:
	    list(p);
	    break;

	case O_LLDB:
#ifdef KDBX
	    if (p->value.arg[0] != nil) {
		kdbx_lldb(p->value.arg[0]->value.scon);
	    } else {
		kdbx_lldb(nil);
	    }
#endif
	    break;

	case O_FUNC:
	    func(p->value.arg[0]);
	    if (p->value.arg[0] != nil) {
		action_mask |= LISTING;
		action_mask |= FUNC_CHANGE;
	    }
	    break;

        case O_THREAD:
#if defined (CMA_THREAD) ||  defined (K_THREADS)
            {
              thread_op th_op;
              checkref(p->value.arg[0]);
              assert(p->value.arg[0]->op == O_LCON);
              th_op = (thread_op)(p->value.arg[0]->value.lcon);
              switch (th_op) {
                case th_default:
                case th_info:
                     threads(th_op, p->value.arg[1]);
                     break;
                case th_hold:
                case th_unhold:
                     threads(th_op, p->value.arg[1]);
	             action_mask |= THREAD_CHANGE;
                     break;
                case th_run:
                case th_ready:
                case th_susp:
#ifdef K_THREADS
                case th_wait:
#endif /* K_THREADS */
                case th_term:
                     if (p->value.arg[1] != nil) {
                        if (lib_type ==  KERNEL_THREAD) {
                            (*rpt_error)(stderr,
                                         catgets(scmc_catd,MS_pthread,MSG_774,
               "Usage: \"thread [run | wait | susp | term]\" takes no id\n"));
                            break;
                        } else {
                            (*rpt_error)(stderr,
                                         catgets(scmc_catd,MS_eval,MSG_731,
               "Usage: \"thread [run | ready | susp | term]\" takes no id\n"));
                            break;
                        }
                     }
                     threads(th_op, nil);
                     break;
                case th_current:
                     if (p->value.arg[1]) {
                        /* if one argument is provided, make sure it is id */
                        if (p->value.arg[1]->op != O_LCON) {
                            (*rpt_error)(stderr,
			                 catgets(scmc_catd, MS_eval, MSG_732,
                          "Usage: \"thread current\" takes zero or one id\n"));
                        } else {
                           /* if one thread id is provided, switch to it... */
                           threads(th_current, p->value.arg[1]);
	                   action_mask |= THREAD_CHANGE;
                        }
                     } else {
                        /* else (no arg), display what is current thread. */
                        threads(th_current, nil);
                     }
                     break;
                case th_run_next:
                     /* Check and make sure only one element in list */
                     if (p->value.arg[1] == nil ||
                         p->value.arg[1]->op != O_LCON) {
                         (*rpt_error)(stderr,
			                 catgets(scmc_catd, MS_eval, MSG_733,
                                 "Usage: \"thread run_next\" takes one id\n"));
                         break;
                     }
                     threads(th_run_next, p->value.arg[1]);
                     break;
                default:
                     /* do nothing, should never reach here. */
                     break;
              }
            }
#endif /* CMA_THREAD || K_THREADS */
            break;

        case O_ATTRIBUTE:
#if defined (CMA_THREAD) || defined (K_THREADS)
            {
              attribute_op attr_op;
              checkref(p->value.arg[0]);
              assert(p->value.arg[0]->op == O_LCON);
              attr_op = (attribute_op)(p->value.arg[0]->value.lcon);
              switch (attr_op) {
                case attr_default:
                     attribute(attr_default, p->value.arg[1]);
                     break;
                default:
                     /* do nothing, should never reach here. */
                     break;
              }
            }
#endif /* CMA_THREAD || K_THREADS */
            break;

        case O_CONDITION:
#if defined (CMA_THREAD) || defined (K_THREADS)
            {
              condition_op cv_op;
              checkref(p->value.arg[0]);
              assert(p->value.arg[0]->op == O_LCON);
              cv_op = (condition_op)(p->value.arg[0]->value.lcon);
              switch (cv_op) {
                case cv_default:
                     condition(cv_default, p->value.arg[1]);
                     break;
                case cv_wait:
                case cv_nowait:
                     if (p->value.arg[1] != nil) {
                         (*rpt_error)(stderr,
			                 catgets(scmc_catd, MS_eval, MSG_734,
    		      "Usage: \"condition { wait | nowait }\" takes no id\n"));
                         break;
                     }
                     condition(cv_op, nil);
                default:
                     /* do nothing, should never reach here. */
                     break;
              }
            }
#endif /* CMA_THREAD || K_THREADS */
            break;

        case O_MUTEX:
#if defined (CMA_THREAD) || defined (K_THREADS)
            {
              mutex_op mu_op;
              checkref(p->value.arg[0]);
              assert(p->value.arg[0]->op == O_LCON);
              mu_op = (mutex_op)(p->value.arg[0]->value.lcon);
              switch (mu_op) {
                case mu_default:
                     mutex(mu_default, p->value.arg[1]);
                     break;
                case mu_wait:
                case mu_nowait:
                case mu_lock:
                case mu_unlock:
                     if (p->value.arg[1] != nil) {
                        if (lib_type ==  KERNEL_THREAD) {
                           (*rpt_error)(stderr,
                                         catgets(scmc_catd, MS_pthread, MSG_775,
         "Usage: \"mutex { lock | unlock }\" takes no id \n"));
                           break;
                         } else {
                           (*rpt_error)(stderr,
                                         catgets(scmc_catd, MS_eval, MSG_735,
         "Usage: \"mutex { wait | nowait | lock | unlock }\" takes no id \n"));
                           break;
                         }

                     }
                     mutex(mu_op, nil);
                default:
                     /* do nothing, should never reach here. */
                     break;
              }
            }
#endif /* CMA_THREAD || K_THREADS */
            break;

	case O_EXAMINE:
	    eval(p->value.examine.beginaddr);
	    r0 = pop(long);
	    if (p->value.examine.endaddr == nil) {
		n = p->value.examine.count;
		if (n == 0) {
		    printvalue(r0, p->value.examine.mode);
		} else if (streq(p->value.examine.mode, "i")) {
#ifdef KDBX
		    is_diss = 1;
		    if (varIsSet("$progress") && ! new_command)
			printninst(n, prtaddr);
		    else
#endif  /* KDBX */
		    printninst(n, (Address) r0);
		} else {
#ifdef KDBX
		    if (varIsSet("$progress") && ! new_command)
			printndata(n, prtaddr, p->value.examine.mode);
		    else
#endif /* KDBX */
		    printndata(n, (Address) r0, p->value.examine.mode);
		}
	    } else {
		eval(p->value.examine.endaddr);
		r1 = pop(long);
		if (streq(p->value.examine.mode, "i")) {
#ifdef KDBX
		    is_diss = 1;
#endif /* KDBX */
		    printinst((Address)r0, (Address)r1);
		} else {
		    printdata((Address)r0, (Address)r1, p->value.examine.mode);
		}
	    }
	    break;

	case O_MOVE:
	    list(p);
	    break;

	case O_PRINT:
            Subar_offset=0;
            Subar_save_sym=0;
	    for (n1 = p->value.arg[0]; n1 != nil; n1 = n1->value.arg[1]) {
		eval(n1->value.arg[0]);
		printval(n1->value.arg[0]->nodetype, 0);
		(*rpt_output)(stdout, " " );
	    }
	    (*rpt_output)(stdout, "\n" );
	    break;

	case O_PSYM:
	    if (p->value.arg[0]->op == O_SYM) {
		psym(p->value.arg[0]->value.sym);
	    } else {
		psym(p->value.arg[0]->nodetype);
	    }
	    break;

	case O_SYMTYPE:
	    if( (p->value.arg[0])->op == O_CPPREF ) {
		symboltype( (p->value.arg[0])->value.arg[0]->nodetype );
	    } else {
		symboltype(p->value.arg[0]->nodetype);
	    }
	    break;

	case O_QLINE:
	    eval(p->value.arg[1]);
	    break;

	case O_SIZEOF:
	    if (p->value.arg[0]->op == O_SYM) {
		push(long,size(p->value.arg[0]->value.sym));
	    } else {
		push(long,size(p->value.arg[0]->nodetype));
	    }
	    break;

#define STOPPED 0177

	case O_STEP:
#ifdef KDBX
            if (p->value.step.source && (nlines_total == 0)) {
              (*rpt_output)(stdout,
                            "Stepping at source level impossible (no source file compiled with -g)\n");
              break;
            }
            use_local_breaks = 1;
#endif /* KDBX */

#if defined (CMA_THREAD) || defined  (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    b = inst_tracing;
	    inst_tracing = (Boolean) (not p->value.step.source);
	    for (; stepcount > 0; stepcount--) {
	        if (p->value.step.skipcalls) {
		    next();
	        } else {
		    stepc();
	        }
		if (process->status != STOPPED)
		   break;
	    }
	    stepcount = 1;
	    inst_tracing = b;
	    useInstLoc = (Boolean) (not p->value.step.source);
            action_mask |= EXECUTION;
            action_mask &= ~CONTEXT_CHANGE;
            action_mask &= ~LISTING;
            action_mask &= ~ELISTING;
	    printnews(false);
	    break;

	case O_WHATIS:
	{
            Node n;
            Symbol s;
	    n = p->value.arg[0];
	    /*
	     * Clear the second operand, since it is not a freeable entity
	     */
	    p->value.arg[1] = NULL;
	    if (n->op == O_RVAL)
		n = n->value.arg[0];
	    if (n->op == O_CPPREF)
		n = n->value.arg[0];
	    s = n->nodetype;
	    if (s->class == CPPSYMLIST)
	    {
		cppSymList list = s->symvalue.sList;
		for (; list != nil; list = list->next)
		    printdecl(list->sym);
	    }
	    else 
	    {
		if (n->op == O_SYM)
		{
		    if (lazy)
		        touch_sym(n->value.sym);
		    printdecl(n->value.sym);
		}
		else
		    printdecl(s);
	    }
	    break;
	}

	case O_WHERE:
	    wherecmd();
	    break;

	case O_WHEREIS:
	    assert(p->value.arg[0]->op == O_NAME);
	    printwhereis( rpt_output, stdout, p->value.arg[0]->value.name);
	    break;

	case O_WHICH:
	{
            Node n;
            Symbol s;
	    n = p->value.arg[0];
	    if (n->op == O_CPPREF)
		n = n->value.arg[0];
	    s = n->nodetype;
	    if (s->class == CPPSYMLIST)
	    {
		cppSymList list = s->symvalue.sList;

		/* find the funcList node */
		Node p = n, q = nil;
		int argument = 0;

		if (p->op == O_CPPREF)
		{
		    q = p;
		    p = p->value.arg[0];
		}
		while (p->op == O_RVAL)
		{
		    q = p;
		    p = p->value.arg[0];
		}
		if (p->op == O_DOT)
		{
		    argument = 1;
		    q = p;
		}

		for (; list != nil; list = list->next)
		{
		    if (q != nil)
		        q->value.arg[argument] = build(O_SYM, list->sym);
		    else
		        n = build(O_SYM, list->sym);
		    prtree(rpt_output, stdout, n);
		    (*rpt_output)(stdout, "\n" );
		}
	    }
	    else 
	    {
		if (n->op == O_SYM)
		    printwhich(rpt_output, stdout, n->value.sym, true);
		else
		{
		    if (s->language == cppLang)
			prtree(rpt_output, stdout, n);
		    else
			printwhich(rpt_output, stdout, s, true);
		}
		(*rpt_output)(stdout, "\n" );
	    }
	    break;
	}

	case O_ALIAS:
	    n1 = p->value.arg[0];
	    n2 = p->value.arg[1];
	    /*
	     * Since the operands will be used for the keywords table, clear
	     * them so we don't try to free them as part of the Node
	     */
	    p->value.arg[0] = p->value.arg[1] = NULL;
	    if (n2 == nil) {
		if (n1 == nil) {
		    alias(nil, nil, nil);
		} else {
		    alias(n1->value.name, nil, nil);
		}
	    } else if (n2->op == O_NAME) {
		str = ident(n2->value.name);
		alias(n1->value.name, nil, strdup(str));
	    } else {
		if (n1->op == O_COMMA) {
		    alias(
			n1->value.arg[0]->value.name,
			(List) n1->value.arg[1]->value.arg[0],
			n2->value.scon
		    );
                    if (tracetree) {
     	                (*rpt_error)(stderr, "name list = ");
	                prtree( rpt_error, stderr, n1->value.arg[1] );
	                (*rpt_error)(stderr, "\n");
                    }
		} else {
		    alias(n1->value.name, nil, n2->value.scon);
		}
	    }
	    break;

	case O_UNALIAS:
	    unalias(p->value.arg[0]->value.name);
	    break;

	case O_CALLPROC:
#if defined (CMA_THREAD) ||  defined (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    callproc(p, false);
	    break;

	case O_CALL:
#if defined (CMA_THREAD) ||  defined (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    callproc(p, true);
	    break;

	case O_CASE:
	    if (p->value.lcon == -1)
		switch(casemode) {
		   case  mixed:
		       (*rpt_output)(stdout, catgets(scmc_catd,
			MS_eval, MSG_95, "Symbols are not folded (mixed).\n"));
			break;
		   case  lower:
			(*rpt_output)(stdout, catgets(scmc_catd, MS_eval,
			MSG_96, "Symbols are folded to lower case.\n"));
			break;
		   case  upper:
			(*rpt_output)(stdout, catgets(scmc_catd, MS_eval,
			MSG_97, "Symbols are folded to upper case.\n"));
			break;
		   case filedep:
		   default:
			(*rpt_output)(stdout, catgets(scmc_catd, MS_eval,
			 MSG_98,
			 "Symbols are folded based upon current language.\n"));
			   switch(symcase) {
			      case mixed:
                                (*rpt_output)(stdout, catgets(scmc_catd,
                                              MS_eval, MSG_95, 
                                "Symbols are not folded (mixed).\n"));
			        break;
			      case lower:
                                (*rpt_output)(stdout, catgets(scmc_catd,
                                              MS_eval, MSG_96, 
                                "Symbols are folded to lower case.\n"));
			        break;
			      case upper:
                                (*rpt_output)(stdout, catgets(scmc_catd,
                                              MS_eval, MSG_97,
                                "Symbols are folded to upper case.\n"));
			        break;
			      default:
                                (*rpt_output)(stdout, catgets(scmc_catd,
                                              MS_eval, MSG_95, 
                                "Symbols are not folded (mixed).\n"));
			     }
			     break;
		}		
	    else {
		symcase = casemode = (cases) p->value.lcon;
		if (symcase == filedep)
		    srcfilename(pc);
		action_mask |= CONFIGURATION;
	    }
	    break;

	case O_CATCH:
	    if (p->value.arg[0]->value.lcon == 0) {
		printsigscaught(process);
	    } else {
		psigtrace(process, p->value.arg[0]->value.lcon, true);
		action_mask |= CONFIGURATION;
	    }
	    break;

	case O_CLEAR:
	    if (clearbps(p))
		action_mask |= BREAKPOINT;
	    break;

	case O_CLEARI:
	    if (clearbps_i(p))
		action_mask |= BREAKPOINT;
	    break;

	case O_EDIT:
	    edit(p->value.scon);
	    break;

        case O_DEBUG:
            debug(p);
	    break;

	case O_DUMP:
	    if (p->value.arg[0] == nil)
		dumpall();
	    else 
	    {
		s = p->value.arg[0]->nodetype;
		dump(s == curfunc ? nil : s);
	    }
	    break;

	case O_GOTO:
#if defined (CMA_THREAD) ||  defined (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    changepc(p);
	    break;

	case O_HELP:
	    help(p);
	    break;

	case O_IGNORE:
	    if (p->value.arg[0]->value.lcon == 0) {
		printsigsignored(process);
	    } else {
		psigtrace(process, p->value.arg[0]->value.lcon, false);
		action_mask |= CONFIGURATION;
	    }
	    break;

        case O_MAP:
            printloaderinfo();
            break;

	case O_PROMPT:
	    if (p->value.arg[0] != nil) {
		free(prompt);
	        prompt = p->value.arg[0]->value.scon;

		/*
		 * Since the scon value will get freed if the prompt is changed
		 * again, clear the operand so we don't try to free it as part
		 * of the Node
		 */
		p->value.arg[0]->value.scon = NULL;
	    }
	    else 
		(*rpt_output)(stdout, "%s\n", prompt);
	    break; 

	case O_RETURN:
#if defined (CMA_THREAD) ||  defined (K_THREADS)
            /* Switch context back to running thread if CMA thread exists */
            if (OK_TO_SWITCH)
               switchThread(running_thread);
#endif /* CMA_THREAD || K_THREADS */
	    rtnfunc(p->value.arg[0] == nil ? nil : p->value.arg[0]->nodetype);
	    break;

	case O_REGS:
	    registers();
	    break;

	case O_RUN:
/* dead code ??  Does run subcommand always get executed in commands.y? */
#ifdef KDBX
	    (*rpt_error)(stderr, "run or rerun not allowed in kdbx\n");
#else /* KDBX */
	    if (norun) {
	       (*rpt_error)(stderr, catgets(scmc_catd, MS_eval, MSG_106,
			      "run or rerun allowed only on initial process"));
	    }
	    else {
	       	run();
		action_mask |= EXECUTION;
		action_mask &= ~CONTEXT_CHANGE;
		action_mask &= ~LISTING;
		action_mask &= ~ELISTING;
	    }
#endif /* KDBX */
	    break;

	case O_SET:
	    set(p->value.arg[0], p->value.arg[1]);
	    /*
	     * Set to NULL so we won't try to free in tfree().  This value is
	     * used for storing the value of the set variable, and freed when
	     * the set variable is unset.
	     */
	    p->value.arg[1] = NULL;
	    break;

	case O_SEARCH:
	    search(p->value.arg[0]->value.lcon, p->value.arg[1]->value.scon);
	    action_mask |= ELISTING;
	    break;

	case O_SOURCE:
	    setinput(p->value.scon);
	    break;

	case O_STATUS:
	    status();
	    break;

	case O_TRACE:
	case O_TRACEI:
	    trace(p);
	    action_mask |= TRACE_ON;
	    break;

	case O_STOP:
	case O_STOPI:
	    stop(p);
	    action_mask |= BREAKPOINT;
	    break;

	case O_UNSET:
	    undefvar(p->value.arg[0]->value.name);
	    break;

	case O_WATCH:
	    watch(p);
	    break;

	case O_ADDEVENT:
	    addevent(p->value.trace.cond, p->value.trace.actions);
	    break;

	case O_DELETE:
	    n1 = p->value.arg[0];
	    while (n1->op == O_COMMA) {
		n2 = n1->value.arg[0];
		assert(n2->op == O_LCON);
		if (not delevent((unsigned int) n2->value.lcon)) {
		    error( catgets(scmc_catd, MS_eval, MSG_107,
					"unknown event %ld"), n2->value.lcon);
		} else
		    action_mask |= BREAKPOINT;
		n1 = n1->value.arg[1];
	    }
	    assert(n1->op == O_LCON);
	    if (not delevent((unsigned int) n1->value.lcon)) {
		error( catgets(scmc_catd, MS_eval, MSG_107,
					"unknown event %ld"), n1->value.lcon);
	    } else
		action_mask |= BREAKPOINT;
	    break;

	case O_DELALL:
	    if ( deleteall())
		action_mask |= BREAKPOINT;
	    break;

	case O_ENDX:
	    endprogram();
	    action_mask |= EXECUTION_COMPLETED;
	    break;

	case O_IF:
	    if (cond(p->value.trace.cond)) {
		evalcmdlist(p->value.trace.actions, false);
	    }
	    break;

	case O_ONCE:
	    event_once(copynode(p->value.trace.cond), p->value.trace.actions);
	    break;

	case O_PRINTCALL:
	    printcall(p->value.sym, whatblock(return_addr()));
	    break;

	case O_PRINTIFCHANGED:
	    printifchanged(p->value.arg[0]);
	    break;

	case O_PRINTRTN:
	    printrtn(p->value.sym);
	    break;

	case O_PRINTSRCPOS:
	    getsrcpos();

            /*
             * Start of trace output.  Indicate by setting
             * eventnum to nil.
             */
            trdata.reporting_trace_output = 1;
            (*rpt_trace)( &trdata );

	    dpi_current_location( &trdata );
	    trdata.eventnum = eventId;
	    trdata.token = nil;
	    trdata.value = nil;

	    if (p->value.arg[0] == nil) {
		printsrcpos();
		(*rpt_output)(stdout, "\n" );
		printlines(curline, curline);
	    } else if (p->value.arg[0]->op == O_QLINE) {
                /* Check filename to find out if this is a "tracei" */
                if ((p->value.arg[0]->value.arg[1]->value.lcon == 0) ||
                    (p->value.arg[0]->value.arg[0]->op == O_SCON &&
                     p->value.arg[0]->value.arg[0]->value.scon == nil)) {
		    (*rpt_output)(stdout, "tracei: ");
	    	    printinst(pc, pc);
	    	} else {
		    if (canReadSource()) {
			    (*rpt_output)(stdout, "trace");
			    if ((sourcefiles > 1) || inclfiles)
			        (*rpt_output)(stdout, " in %s:", 
                                              basefile(cursource));
			    else
			        (*rpt_output)(stdout, ":");
			    printlines(curline, curline);
		    }
		}
	    } else {
		eval(p->value.arg[0]);

		    printsrcpos();

		    msgbegin;
		    prtree( rpt_output, stdout, p->value.arg[0]);
		    msgend( trdata.token );

		    msgbegin;
		    printval(p->value.arg[0]->nodetype, 0);
		    msgend( trdata.value );

		    (*rpt_output)(stdout, ": %s = %s\n", 
			trdata.token, trdata.value );
	    }
            trdata.reporting_trace_output = 0;
	    (*rpt_trace)( &trdata );
  	    dispose( trdata.token );
	    dispose( trdata.value );
	    break;

	case O_PROCRTN:
	    procreturn(p->value.sym);
	    action_mask |= EXECUTION;
	    action_mask &= ~CONTEXT_CHANGE;
	    action_mask &= ~LISTING;
	    action_mask &= ~ELISTING;
	    break;

	case O_STOPIFCHANGED:
	    stopifchanged(p);
	    break;

	case O_STOPX:
	    isstopped = true;
	    break;

	case O_TRACEON:
	    traceon(p->value.trace.inst, p->value.trace.event,
		p->value.trace.actions);
	    break;

	case O_TRACEOFF:
	    traceoff(p->value.lcon);
	    break;

	case O_SCREEN:
            if(ScrUsed) {
              warning(catgets(scmc_catd, MS_eval, MSG_714,
                      "screen subcommand can only be invoked once.\n"));
            }
            else if(norun)  {
              warning(catgets(scmc_catd, MS_eval, MSG_713,
     "screen subcommand can only be invoked from the originating process.\n"));
            }
            else {
              int fd;
              fd = screen(1);  /* return -1 if fail to open an Xwindow */
              if(fd != -1) reset_debuggee_fd(fd);
            }
            break;

	case O_MULTPROC:
	    pid = curpid();
	    if (p->value.arg[0]->value.lcon == 0) 
	    {
              switch(multproc)
              {
                case on :
                  (*rpt_output)(stdout,
                  "Multi-process debugging is enabled\n");
                   break;
                case parent :
                  (*rpt_output)(stdout,
                  "Multi-process debugging is enabled to follow the parent\n");
                   break;
                case child :
                  (*rpt_output)(stdout,
                  "Multi-process debugging is enabled to follow the child\n");
                   break;
                case off :
                  (*rpt_output)(stdout,
                  "Multi-process debugging is disabled\n");
                   break;
              }
	    } else {
                fork_type new_multproc;
	        new_multproc = (fork_type) p->value.arg[0]->value.lcon;
                if (new_multproc == off)
	          errcond = ptrace(MULTI, pid, 0, 0, 0);
                else
	          errcond = ptrace(MULTI, pid, 0, 1, 0);

	        if (errcond < 0)
		    panic( catgets(scmc_catd, MS_eval, MSG_124,
				    "Could not alter multi-processing mode."));
	        else {
		    multproc = new_multproc;
		    action_mask |= CONFIGURATION;
		}
	    }
	    break;

	case O_DETACH:
	    bpfree();
            errcond = detach(curpid(), p->value.arg[0]->value.lcon);
	    if (errcond < 0)
	        warning( catgets(scmc_catd, MS_eval, MSG_125,
				 "Could not detach from process.  Use quit."));
	    else
	       {
		action_mask |= DETACHED;
               }
	    break;

	case O_OBJECT:
	    objname = p->value.scon;
	    objfree();
	    symbols_init();
	
	    /*
	    types_reinit();
	    */
	    readobj(objname);
	    break;

        case O_SWITCH:
#ifdef KDBX
            checkref(p->value.arg[0]);
            assert(p->value.arg[0]->op == O_LCON);
            do_switch(p->value.arg[0]->value.lcon);
#endif  /* KDBX */
            break;

        case O_CPU:
#ifdef KDBX
            checkref(p->value.arg[0]);
            assert(p->value.arg[0]->op == O_LCON);
            do_cpu(p->value.arg[0]->value.lcon);
#endif /* KDBX */
            break;

        case O_XCALL:
#ifdef KDBX
            assert(p->value.arg[0]->op == O_NAME);

            nbp =0;
            for (n1 = p->value.arg[1]; n1 != nil; n1 = n1->value.arg[1]) {
                eval(n1->value.arg[0]);
                xcall_args[nbp] = pop(long);
                if (nbp == MAX_XCALL_ARGS) break;
                    else nbp++;
            }
            if (nbp == MAX_XCALL_ARGS) {
                printf("Invalid argument number (%d) to print function\n", nbp);
            } else {
                do_xcall(p->value.arg[0]->value.name, nbp, xcall_args);
            }
#endif /* KDBX */
            break;

	default:
	    panic( catgets(scmc_catd, MS_eval, MSG_126, "eval: bad op %d"),
									p->op);
    }
                                            /* if we are not evaluating data */
    if (p->op != O_SYM && p->op != O_RVAL && p->op != O_DOT &&
        p->op != O_DOTSTAR) {
                                            /* reset the defered and ptr to  */
                                            /* member flags                  */
         notderefed = true;
         specificptrtomember = false;
    }
    if (traceeval) { 
	(*rpt_error)(stderr, "end eval %s\n", opname(p->op));
    }
  }
}

/*
 * NAME: unary_op
 *
 * FUNCTION: Handle unary operations.
 *
 * PARAMETERS:
 *      p      - input Node  
 *
 * MACROS: poprealarg, popintarg, poplonglongarg, 
 *         do_unary_operation, unary_operation
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0
 */

int unary_op(p)
Node p;
{
  int i;
  double fr0, fr1;
  quadf qr0, qr1;

  if (isreal(p->op))
  {
    poprealarg(0, fr0, qr0);

    switch (p->op)
    {
      case O_FTOQ:
        push(double, (double) fr0);
        push(double, (double) 0);
        break;

      case O_NEGQ:
        qsub(&qr0, (double) 0, (double) 0, qr0.val[0], qr0.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_NEGF:
        push(double, -fr0);
        break;

    }
  }
  else if (isint(p->op))
  {
    long r0;
    unsigned long ur0;
    LongLong llr0;
    uLongLong ullr0;
    Symbol op_type;
    unsigned char operand_type = ISLONG;

    op_type = rtype(p->value.arg[0]->nodetype);

    if (op_type->symvalue.rangev.is_unsigned)
      operand_type |= ISUNSIGNED;
    if (op_type->symvalue.rangev.size == sizeofLongLong)
      operand_type |= ISLONGLONG;

    switch (operand_type)
    {
      case ISLONG:
        popintarg(0, r0);
        break;
      case ISUNSIGNEDLONG:
        popintarg(0, ur0);
        break;
      case ISLONGLONG:
        poplonglongarg(0, llr0);
        break;
      case ISUNSIGNEDLONGLONG:
        poplonglongarg(0, ullr0);
        break;
    }

    switch (p->op)
    {
      case O_ITOF:
      case O_ITOQ:
        unary_operation(double, (double))

        if (p->op == O_ITOQ)
          push(double, (double) 0);
        break;

      case O_NEG:
        do_unary_operation(-);
        break;

      case O_NOT:
        unary_operation(Boolrep, !)
        break;

      case O_COMP:
        do_unary_operation(~);
        break;
    }
  }
  else
  {
    Symbol s;
    Address addr = 0;
    unsigned len = 0;
    
    eval (p->value.arg[0]);

    switch (p->op)
    {
	/*
	 * Pop an address and push the contents back on the stack.
	 * If the address is actually a register number, then push
	 * the (possibly saved on stack) register value(s).
	 */

	case O_CPPREF:
	    rpush(pop(Address), sizeof(Address));
	    break;

	case O_INDIR:
	case O_INDIRA:
	    if (p->nodetype->class == STRINGPTR) break;
	case O_RVAL:
            s = p->nodetype;

            if (is_f90_sym(s))
              s = p->nodetype = convert_f90_sym(s, NULL);

            /* don't have to do this for label  */
            if (s->class == LABEL || s->class == FUNC) break;

            /* or a pointer to a member function*/
            if (ptrtomemberfunction) { 
               ptrtomemberfunction = false;
               break; 
            }

	    /* or string constants */
	    if (s->class == ARRAY && s->type == t_char &&
		p->value.arg[0]->op == O_SCON)
	    {
		/*
		 * This case is really for dbx vars that have been
		 * set to string constants, for example
		 * set $stepignore="function".  We want the quotes to
		 * be there if you print $stepignore.
		 */
		if (s->language == primlang)
		    s->language = cLang;
		break;
	    }

            /*  if this is a debugger variable  */
            if (s == t_int || s == t_char || s == t_boolean
             || s == dt_uint || s == t_longlong || s == t_ulonglong
             || s == t_quad)
               break;

	    indirect = true;
	    if (sp > &stack[0])
	       addr = pop(long);
            /* In case of pascal pointer, there are two words in stack.  */
            /* The first one is the size of what the pointer is pointing */
            /* at, and the second one is the actual address we want.     */
            if (sp > &stack[0] and s->language == pascalLang and
                (p->op == O_INDIR or p->op == O_INDIRA))
               addr = pop(long);
	    if (subarray) {
	       if (s->class == ARRAY)
	         error( catgets(scmc_catd, MS_eval, MSG_116,
		 		"Subarray of pointer to array not supported."));
	       else		/* Break because we don't need to do this */
	         break;		/* when we are processing a subarray.     */
	    }
	    if (p->op == O_INDIRA) 
		len = sizeof(Address);
	    else 
                len = size(s);
	    if (s->class != REF &&
	       (s->storage == INREG || (s->param && preg(s, nil) != -1)))
		   pushregvalue(s, addr, nil, len);
	    else
            {
	      if (addr == 0)
	          error( catgets(scmc_catd, MS_eval, MSG_84,
					     "reference through nil pointer"));
	      /* To acount for read/write offset within a file. The */
	      /* offset is stored at next address after the file    */
	      /* pointer itself. For Pascal ONLY!                   */
              if (s->language == pascalLang and
	          (p->op == O_RVAL and rtype(s)->class == FILET)) 
	      {
		 int offset;
		 dread(&offset, addr+sizeof(Address), sizeof(Word));
		 dread(&addr, addr, len);
                 offset = (offset > 0) ? offset-1 : 0;
                 push(Address, addr+offset)
	      } 
	      else 
		 rpush(addr, len);
	    }
	    break;

	case O_TYPERENAME:
            typecast(p);
	    break;

	case O_UP:
	    checkref(p->value.arg[0]);
	    assert(p->value.arg[0]->op == O_LCON);
	    pop(long);
	    up(p->value.arg[0]->value.lcon);
	    break;

	case O_DOWN:
	    checkref(p->value.arg[0]);
	    assert(p->value.arg[0]->op == O_LCON);
	    pop(long);
	    down(p->value.arg[0]->value.lcon);
	    break;

    }
  }

  return(0);
}

/*
 * NAME: binary_op
 *
 * FUNCTION: Handle binary operations.
 *
 * PARAMETERS:
 *      p      - input Node  
 *
 * MACROS: poprealarg, popintarg, poplonglongarg, 
 *         do_operation, boolean_operation
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0
 */

int binary_op(p)
Node p;
{
  int i;
  double fr0, fr1;
  quadf qr0, qr1;

  if (isreal(p->op))
  {
    poprealarg(1, fr1, qr1);
    poprealarg(0, fr0, qr0);

    switch (p->op)
    {
      case O_ADDF:
        push(double, fr0+fr1);
        break;

      case O_ADDQ:
        qadd(&qr0, qr0.val[0], qr0.val[1], qr1.val[0], qr1.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_SUBQ:
        qsub(&qr0, qr0.val[0], qr0.val[1], qr1.val[0], qr1.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_MULQ:
        qmul(&qr0, qr0.val[0], qr0.val[1], qr1.val[0], qr1.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_DIVQ:
        qdiv(&qr0, qr0.val[0], qr0.val[1], qr1.val[0], qr1.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_EXPQ:
        qpow(&qr0, qr0.val[0], qr0.val[1], qr1.val[0], qr1.val[1]);
        push(double, qr0.val[0]);
        push(double, qr0.val[1]);
        break;

      case O_LTQ:
        /* _qdbcmp() - quad comparsion routine */
        /* returns -1 when a < b		   */
        /* 		0 when a == b		   */
        /*          1 when a > b		   */
        /* 	       -2 when a or b is NaN	   */
        push(Boolrep, _qdbcmp(&qr0, &qr1) == -1);
        break;

      case O_LEQ:
        i = _qdbcmp(&qr0, &qr1);	
        push(Boolrep, (i == -1 or i == 0));
        break;

      case O_GTQ:
        push(Boolrep, _qdbcmp(&qr0, &qr1) == 1 );
        break;

      case O_GEQ:
        i = _qdbcmp(&qr0, &qr1);	
        push(Boolrep, (i == 1 or i == 0));
        break;

      case O_EQQ:
        push(Boolrep, _qdbcmp(&qr0, &qr1) == 0 );
        break;

      case O_NEQ:
        push(Boolrep, _qdbcmp(&qr0, &qr1) != 0 );
        break;

      case O_EXP:
        push(double, pow(fr0,fr1));
        break;

      case O_SUBF:
        push(double, fr0-fr1);
        break;

      case O_MULF:
        push(double, fr0*fr1);
        break;

      case O_DIVF:
        if (fr1 == 0) {
            error( catgets(scmc_catd, MS_eval, MSG_85,
                                                  "error: division by 0"));
        }
        push(double, fr0 / fr1);
        break;

      case O_LTF:
        push(Boolrep, fr0 < fr1);
        break;

      case O_LEF:
        push(Boolrep, fr0 <= fr1);
        break;

      case O_GTF:
        push(Boolrep, fr0 > fr1);
        break;

      case O_GEF:
        push(Boolrep, fr0 >= fr1);
        break;

      case O_EQF:
        push(Boolrep, fr0 == fr1);
        break;

      case O_NEF:
        push(Boolrep, fr0 != fr1);
        break;
    }
  }
  else
  {
    long r0, r1 = 0;
    unsigned long ur0, ur1 = 0;
    LongLong llr0, llr1 = 0;
    uLongLong ullr0, ullr1 = 0;
    Symbol op_type;
    unsigned char operand_type = ISLONG;

    if (p->nodetype == t_boolean)
    {
      op_type = get_output_nodetype (rtype(p->value.arg[0]->nodetype),
                                     rtype(p->value.arg[1]->nodetype));
      op_type = rtype(op_type);
    }
    else
    {
      op_type = rtype(p->nodetype);
    }

    if (op_type->symvalue.rangev.is_unsigned)
      operand_type |= ISUNSIGNED;
    if (op_type->symvalue.rangev.size == sizeofLongLong)
      operand_type |= ISLONGLONG;

    switch (operand_type)
    {
      case ISLONG:
        popintarg(1, r1);
        popintarg(0, r0);
        break;
      case ISUNSIGNEDLONG:
        popintarg(1, ur1);
        popintarg(0, ur0);
        break;
      case ISLONGLONG:
        poplonglongarg(1, llr1);
        poplonglongarg(0, llr0);
        break;
      case ISUNSIGNEDLONGLONG:
        poplonglongarg(1, ullr1);
        poplonglongarg(0, ullr0);
        break;
    }

    switch (p->op)
    {
      case O_ADD:
        do_operation(+); 
        break;

      case O_BOR:
        do_operation(|);
        break;

      case O_BAND:
        do_operation(&);
        break;

      case O_BXOR:
        do_operation(^);
        break;

      case O_SUB:
        do_operation(-);
        break;

      case O_MUL:
        do_operation(*);
        break;

      case O_DIV:
        if ((r1 == 0) && (ur1 == 0) && (llr1 == 0) && (ullr1 == 0)){
            error( catgets(scmc_catd, MS_eval, MSG_85,
                                                  "error: division by 0"));
        }
        do_operation(/);
        break;

      case O_MOD:
        if ((r1 == 0) && (ur1 == 0) && (llr1 == 0) && (ullr1 == 0)){
            error( catgets(scmc_catd, MS_eval, MSG_88,
                                                    "error: mod by 0"));
        }
        
        do_operation(%);
        break;

      case O_LT:
        boolean_operation(<);
        break;

      case O_LE:
        boolean_operation(<=);
        break;

      case O_GT:
        boolean_operation(>);
        break;

      case O_GE:
        boolean_operation(>=);
        break;

      case O_EQ:
        boolean_operation(==);
        break;

      case O_NE:
        boolean_operation(!=);
        break;

      case O_AND:
        boolean_operation(&&);
        break;

      case O_OR:
        boolean_operation(||);
        break;
 
      case O_SL:
        do_operation(<<);
        break;

      case O_SR:
        do_operation(>>);
        break;
    }
  }
  return(0);
}

int detach(int pid, int signal)
{
#ifdef K_THREADS
  int stat;                          /* status returned by pwait()     */
  extern tid_running_thread;         /* use by ptrace(PTT_CONTINUE)    */
  extern nb_k_threads_sig;           /* number of threads with signal  */
  struct ptthreads buf_ptthreads;    /* buffer for ptrace(PTT_CONTINUE)*/
#endif /* K_THREADS */
  int errcond;

#ifdef K_THREADS
  /* nb_k_threads_sig:number of threads with TTRCSIG (pending signal)*/
  /* if there are more than one thread witch have received the       */
  /* break-point : continue to clear the signal                      */
  while ((nb_k_threads_sig > 1) && process->is_bp )
  {
    buf_ptthreads.th[0] = NULL;
    if (traceexec)
      (*rpt_output)(stdout, "!! ptrace(%d,%d,%d,%d,%x)\n",
                    PTT_CONTINUE, tid_running_thread, 1, 0, &buf_ptthreads);
    if (ptrace(PTT_CONTINUE,tid_running_thread, 1, 0,&buf_ptthreads) < 0)
       panic( catgets(scmc_catd, MS_process, MSG_285,
                     "error %d trying to continue process"), errno);
    
    pwait(pid, &stat);
    if (traceexec)
      (*rpt_output)(stdout, "!! wait status = 0x%x, errno = %d\n",
                    stat, errno);
    getinfo(process, stat);
  }
#endif  /* K_THREADS */

  if (traceexec)
    (*rpt_output)(stdout, "!! ptrace(%d,0x%x,%d,%d,%x)\n",
                  DETACH, pid, 1, signal, 0);

  errcond = ptrace(DETACH, pid, 1, signal, 0);
  return errcond;
}


/* 
 * Dereference the pointer on the stack.
 */

 /* this routine pops the last value on the stack, determines what this */
 /* address is pointing to and pushes what it is pointing to back on    */
 /* the stack                                                           */

void dereference() 
{
    Address addr;

    addr = pop(Address);
    dread(&addr, addr, sizeof(Address));
    notderefed = false;
    push(Address, addr);
}

/*
 * Evaluate a list of commands.
 */

public evalcmdlist(cl, first_line)
Cmdlist cl;
Boolean first_line;
{
    Command c;

    foreach (Command, c, cl)
      /*  if not first_line or it is the first line and tracing in 
            a function  */
      if (!first_line
       || ((c->op == O_PRINTSRCPOS) && (c->value.arg[1] != NULL)))
	evalcmd(c);
    endfor
}

/*
 * Push "len" bytes onto the expression stack from address "addr"
 * in the process.  If there isn't room on the stack, print an error message.
 */

public rpush(addr, len)
Address addr;
unsigned len;
{
    if (not canpush(len)) {
	error( catgets(scmc_catd, MS_eval, MSG_128,
					  "expression too large to evaluate"));
    } else {
	chksp();
	dread(sp, addr, len);
	sp += len;
    }
}

/*
 * Check if the stack has n bytes available.
 */

public Boolean canpush(n)
Integer n;
{
    return (Boolean) (sp + n < &stack[STACKSIZE]);
}

/*
 * Push a small scalar of the given type onto the stack.
 */

public pushsmall(s, v)
int s;
long v;
{
    switch (s) {
	case sizeof(char):
	    push(char, v);
	    break;

	case sizeof(short):
	    push(short, v);
	    break;

	case sizeof(long):
	    push(long, v);
	    break;

	default:
	    panic( catgets(scmc_catd, MS_eval, MSG_129,
					       "bad size %d in pushsmall"), s);
    }
}

/*
 * Push a value in a register for the given symbol, perhaps on the stack.
 * If the length (in words) is greater than one, then push
 * consecutive registers.
 */

public pushregvalue (s, r, f, n)
Symbol s;
int r;
Frame f;
unsigned n;
{
    register int i, j;
    register Frame frp;
    register Symbol b;

    r = (r < IAR) ? r : ((r < FPR0) ? SYSREGNO(r) : FPREGNO(r));
    frp = f;
    if (frp == nil) {
	b = s->block;
	while (b != nil && b->class == MODULE) {
	    b = b->block;
	}
	if (b == nil) {
	    frp = nil;
	} else {
	    frp = findframe(b);
	    if (frp == nil) {
		error( catgets(scmc_catd, MS_eval, MSG_130,
			    "[internal error: nil frame for %s]"), symname(s));
	    }
	}
    }
    if (r < NGREG + NSYS) {
    	j = r + n / sizeof(Word);
    	for (i = r; i < j; i++) {
		push(Word, savereg(i, frp));
    	}
    	j = n % sizeof(Word);
    	if (j > 0) {
		pushsmall(j, savereg(i, frp));
    	}
    }
    else {
	r -= (NGREG + NSYS);
    	j = r + n / sizeof(double);
    	for (i = r; i < j; i++) {
		push(double, savefreg(i, frp));
    	}
    	j = n % sizeof(double);
    	if (j > 3) {
		push(Word, savefreg(i, frp));
		j -= 4;
    	}
	if (j > 0) {
		pushsmall(j, savefreg(i, frp));
	}
    }
}

/*
 * NAME: popsmall 
 *
 * FUNCTION: Pop an item of the given type from the stack 
 *           and convert it to a long. 
 *
 * PARAMETERS:
 *      t      - input Symbol
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a long integer
 */

public long popsmall(t)
Symbol t;
{
    register integer n;
    long r = 0;

    n = size(t);
    if (n == sizeof(char)) {
        if (( (t->class == RANGE or t->class == PACKRANGE)
            and t->symvalue.rangev.lower >= 0) ||
	    ((t->class == VAR) && (t->type == dt_uchar)) ||
	    isunsignedpic(t) || 
            (t->class == TYPE && 
               ((streq(ident(t->name), "char"))
             || streq(ident(t->name), "unsigned char")))) {
	    r = (long) pop(unsigned char);
	} else {
	    r = (long) pop(signed char);
	}
    } else if (n == sizeof(short)) {
        if (( (t->class == RANGE or t->class == PACKRANGE)
            and t->symvalue.rangev.lower >= 0) ||
	    ((t->class == VAR) && (t->type == dt_ushort)) ||
	    isunsignedpic(t) ||
            (t->class == TYPE && streq(ident(t->name), "unsigned short"))) {
	    r = (long) pop(unsigned short);
	} else {
	    r = (long) pop(short);
	}
    } else if (n == 3) {	    /* This case can only happens when    */
        char *ptr;		    /* it's a packed subrange of integers */
				    /* where int is packed into 3 bytes.  */
        ptr = (char *) &r;
        popn(n, ptr+1);
    } else if (n == sizeof(long)) {
	r = pop(long);
    } else if (n == sizeofLongLong) {
	r = (long) pop(LongLong);
    } else {
	error( catgets(scmc_catd, MS_eval, MSG_131,
				  "[internal error: size %d in popsmall]"), n);
    }
    return r;
}

/*
 * NAME: poplonglong
 *
 * FUNCTION: Pop an item of the given type from the stack 
 *           and convert it to a longlong.
 *
 * PARAMETERS:
 *      t      - input Symbol
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a long long integer
 */

LongLong poplonglong(t)
Symbol t;
{
    Symbol rtype_t = rtype(t);
    register integer n;
    LongLong r = 0;

    n = size(t);

    if (n < sizeofLongLong)
    {
      if (rtype_t->symvalue.rangev.is_unsigned)
        r = (LongLong) (unsigned long) popsmall(t);
      else
        r = (LongLong) popsmall(t);
    }
    else
      r = pop(LongLong);

    return r;
}

/*
 * Evaluate a conditional expression.
 */

public Boolean cond(p)
Node p;
{
    Boolean b;
    int i;

    if (p == nil) {
	b = true;
    } else {
	eval(p);
	i = pop(Boolrep);
	b = (Boolean) i;
    }
    return b;
}

/*
 * Return the address corresponding to a given tree.
 */

public Address lval(p)
Node p;
{
    if (p->op == O_RVAL) {
	eval(p->value.arg[0]);
    } else {
	eval(p);
    }
    return (Address) (pop(long));
}

/*
 * Process a trace command, translating into the appropriate events
 * and associated actions.
 */

public trace(p)
Node p;
{
    Node exp, place, cond;

    exp = p->value.arg[0];
    cond = p->value.arg[2];
    if (cond != nil)
	chkboolean(cond);
    place = p->value.arg[1];

    /*
     * Set to NULL so we won't try to free in tfree().  These Nodes are used
     * for printing events, and freed when the event is cleared.
     */
    p->value.arg[0] = p->value.arg[1] = p->value.arg[2] = NULL;

    if (exp != nil && place == nil)
    {
        /* we must handle the possibility of a C++ multiple function list */
        if (exp->nodetype->class != CPPSYMLIST)
	{
	    if (exp->op != O_SYM && exp->nodetype->class == FUNC)
		trace1(p, build(O_SYM, exp->nodetype), nil, cond);
	    else
		trace1(p, exp, nil, cond);
	}
        else
        {
            cppSymList list = exp->nodetype->symvalue.sList;
            for (; list != nil; list = list->next)
                trace1(p, build(O_SYM, list->sym), nil, cond);
        }
    }
    else if (place != nil)
    {
        /* we must handle the possibility of a C++ multiple function list */
        if (place->nodetype->class != CPPSYMLIST)
	    if (place->op != O_SYM && place->nodetype->class == FUNC)
		trace1(p, exp, build(O_SYM, place->nodetype), cond);
	    else
		trace1(p, exp, place, cond);
        else
        {
            cppSymList list = place->nodetype->symvalue.sList;
            for (; list != nil; list = list->next)
                trace1(p, exp, build(O_SYM, list->sym), cond);
        }
    }
    else
        trace1(p, nil, nil, cond);
}

public trace1(p, exp, place, cond)
Node p, exp, place, cond;
{
    Node left;

    if (exp != nil && exp->nodetype->class == MEMBER &&
        exp->nodetype->symvalue.member.type == FUNCM ||
        place != nil && place->nodetype->class == MEMBER &&
        place->nodetype->symvalue.member.type == FUNCM)
    {
	error(catgets(scmc_catd, MS_eval, MSG_600,
	      "Cannot set a breakpoint in a pure virtual function."));
    }
    /* check and see if "exp" is sometime we can trace or stop */
    else if (exp != nil && exp->nodetype->class == TAG) {
	error(catgets(scmc_catd, MS_eval, MSG_639,
	      "Cannot trace or stop a %s."), classname(exp->nodetype));
    }

    if (exp == nil) {
    	if (place->op == O_QLINE or place->op == O_LCON)
        {
	  if (place == nil)
            place = build(O_SYM, program);
          traceinst(p->op, place, cond);
	}
        else
        {
          traceall(p->op, place, cond);
	}
    } else if (exp->op == O_QLINE or
               (exp->op == O_LCON and p->op == O_TRACE and place == nil)) {
	traceinst(p->op, exp, cond);
    } else if (place != nil &&
		((place->op == O_QLINE) || (place->op == O_LCON))) {
	traceat(p->op, exp, place, cond);
    } else {
	if (place == nil)
		place = build(O_SYM, program);
	left = exp;
	if (left->op == O_RVAL or left->op == O_CALL) {
	    left = left->value.arg[0];
	}
	if (left->op == O_CPPREF)
	{
	    /* dereference the reference, and trace the resulting */
	    /* address. 					  */
	    eval(left);
	    exp->op = O_LCON;
	    if ((exp->value.lcon = pop(Address)) == nil)
		error(catgets(scmc_catd, MS_eval, MSG_637, 
		      "Cannot trace an uninitialized reference variable."));
	    trace1(p, exp, place, cond);
	}
	else if (left->op == O_SYM and isblock(left->value.sym)) {
	    traceproc(p->op, left->value.sym, exp, place, cond);
	} else {
	    tracedata(p->op, exp, place, cond);
	}
    }
}

/*
 * Process a watch command, translating into appropriate events and actions
 */

public watch (p)
Node p;
{
    Node exp, place, cond;
    Node left;

    exp = p->value.arg[0];
    place = p->value.arg[1];
    cond = p->value.arg[2];
    if (cond != nil)
	chkboolean(cond);
    if (place == nil)
	place = build(O_SYM, program);
    if (place->op == O_QLINE)
        traceat(p->op, exp, place, cond);
    else
        tracedata(p->op, exp, place, cond);
}

/*
 * Set a breakpoint that will turn on tracing.
 */

private traceall(op, place, cond)
Operator op;
Node place;
Node cond;
{
    Symbol s;
    Node event;
    Command action;

    if (place == nil) {
	s = program;
    } else {
	s = place->value.sym;
    }
    event = build(O_EQ, build(O_SYM, procsym), build(O_SYM, s));
    action = build(O_PRINTSRCPOS,
	build(O_QLINE, nil, build(O_LCON, (op == O_TRACE) ? 1 : 0)),
	place);
    if (cond != nil) {
	action = build(O_IF, cond, buildcmdlist(action));
    }
    action = build(O_TRACEON, (op == O_TRACEI), buildcmdlist(action));
    action->value.trace.event = addevent(event, buildcmdlist(action));
    action->value.trace.event->op = op;
    action->value.trace.event->cond = cond;
    if (isstdin()) {
	printevent(action->value.trace.event);
    }
}

/*
 * Set up the appropriate breakpoint for tracing an instruction.
 */

private traceinst(op, exp, cond)
Operator op;
Node exp;
Node cond;
{
    Node event, wh;
    Command action;
    Event e;

    if (exp->op == O_LCON) {
        /* Use the filename to distinguish "trace" and "tracei" */
        if (op == O_TRACEI)
           wh = build(O_QLINE, build(O_SCON, nil, 0), exp);
        else
           wh = build(O_QLINE, build(O_SCON, strdup(cursource), 0), exp);
    } else {
	wh = exp;
    }
    if (op == O_TRACEI) {
	event = build(O_EQ, build(O_SYM, pcsym), wh);
    } else {
	event = build(O_EQ, build(O_SYM, linesym), wh);
    }
    action = build(O_PRINTSRCPOS, wh, NULL);
    if (cond) {
	action = build(O_IF, cond, buildcmdlist(action));
    }
    e = addevent(event, buildcmdlist(action));
    e->op = op;
    e->cond = cond;
    if (isstdin()) {
	printevent(e);
    }
}

/*
 * Set a breakpoint to print an expression at a given line or address.
 */

private traceat(op, exp, place, cond)
Operator op;
Node exp;
Node place;
Node cond;
{
    Node event;
    Command action;
    Event e;

    if (op == O_TRACEI) {
	event = build(O_EQ, build(O_SYM, pcsym), place);
    } else {
	event = build(O_EQ, build(O_SYM, linesym), place);
    }
    action = build(O_PRINTSRCPOS, exp, place);
    if (cond != nil) {
	action = build(O_IF, cond, buildcmdlist(action));
    }
    e = addevent(event, buildcmdlist(action));
    e->op = op;
    e->exp = exp;
    e->cond = cond;
    if (isstdin()) {
	printevent(e);
    }
}

/*
 * Construct event for tracing a procedure.
 *
 * What we want here is
 *
 * 	when $proc = p do
 *	    if <condition> then
 *	        printcall;
 *	        once $pc = $retaddr do
 *	            printrtn;
 *	        end;
 *	    end if;
 *	end;
 *
 * Note that "once" is like "when" except that the event
 * deletes itself as part of its associated action.
 */

private traceproc(op, p, exp, place, cond)
Operator op;
Symbol p;
Node exp;
Node place;
Node cond;
{
    Node event;
    Command action;
    Cmdlist actionlist;
    Event e;

    action = build(O_PRINTCALL, p);
    actionlist = list_alloc();
    cmdlist_append(action, actionlist);
    event = build(O_EQ, build(O_SYM, pcsym), build(O_SYM, retaddrsym));
    action = build(O_ONCE, event, buildcmdlist(build(O_PRINTRTN, p)));
    cmdlist_append(action, actionlist);
    if (cond != nil) {
	actionlist = buildcmdlist(build(O_IF, cond, actionlist));
    }
    event = build(O_EQ, build(O_SYM, procsym), build(O_SYM, p));
    e = addevent(event, actionlist);
    e->op = op;
    e->exp = exp;
    e->cond = cond;
    if (isstdin()) {
	printevent(e);
    }
}

/*
 * Set up breakpoint for tracing data.
 */

private tracedata (op, exp, place, cond)
Operator op;
Node exp;
Node place;
Node cond;
{
    Symbol p;
    Node event;
    Command action;

    if (size(exp->nodetype) > MAXTRSIZE) {
	error( catgets(scmc_catd, MS_eval, MSG_132,
	      "expression too large to trace (limit is %d bytes)"), MAXTRSIZE);
    }
    p = (place == nil) ? tcontainer(exp) : place->value.sym;
    if (p == nil) {
	p = program;
    }
    action = build(O_PRINTIFCHANGED, exp, build(O_LCON,(long) 0));
    if (cond != nil) {
	action = build(O_IF, cond, buildcmdlist(action));
    }
    action = build(O_TRACEON, (op == O_TRACEI && exp->op == O_QLINE), 
                   buildcmdlist(action));
    event = build(O_EQ, build(O_SYM, procsym), build(O_SYM, p));
    action->value.trace.event = addevent(event, buildcmdlist(action));
    action->value.trace.event->op = op;
    action->value.trace.event->exp = exp;
    action->value.trace.event->cond = cond;
    if (isstdin()) {
	printevent(action->value.trace.event);
    }
}

/*
 * Setting and unsetting of stops.
 */

public stop(p)
Node p;
{
    Node exp, place, cond;

    exp = p->value.arg[0];
    cond = p->value.arg[2];
    if (cond != nil)
	chkboolean(cond);
    place = p->value.arg[1];

    /*
     * Set to NULL so we won't try to free in tfree().  These Nodes are used
     * for printing events, and freed when the event is cleared.
     */
    p->value.arg[0] = p->value.arg[1] = p->value.arg[2] = NULL;
    if (place != nil) 
    {
	/* we must handle the possibility of a C++ multiple function list */
        if (place->nodetype->class != CPPSYMLIST)
	    if (place->op != O_SYM && isfunc(place->nodetype))
		stop1(p, exp, build(O_SYM, place->nodetype), cond);
	    else
		stop1(p, exp, place, cond);
	else
        { 
	    cppSymList list = place->nodetype->symvalue.sList;
            for (; list != nil; list = list->next)
                stop1(p, exp, build(O_SYM, list->sym), cond);
        }
    } 
    else
        stop1(p, exp, nil, cond);
}

public stop1(p, exp, place, cond)
Node p, exp, place, cond;
{
    Node t;
    Symbol s;
    Command action;
    Event e;

    if (place != nil && place->nodetype->class == MEMBER &&
        place->nodetype->symvalue.member.type == FUNCM)
    {
	error(catgets(scmc_catd, MS_eval, MSG_600,
	      "Cannot set a breakpoint in a pure virtual function."));
    }
    /* check and see if "exp" is sometime we can trace or stop */
    else if (exp != nil && exp->nodetype->class == TAG) {
	error(catgets(scmc_catd, MS_eval, MSG_639,
	      "Cannot trace or stop a %s."), classname(exp->nodetype));
    }

    if (exp != nil) {
	stopvar(p->op, exp, place, cond);
    } else {
	action = build(O_STOPX);
	if (cond != nil) {
	    action = build(O_IF, cond, buildcmdlist(action));
	}
	if (place == nil or place->op == O_SYM) {
	    if (place == nil) {
		s = program;
	    } else {
		s = place->value.sym;
	    }
	    t = build(O_EQ, build(O_SYM, procsym), build(O_SYM, s));
	    if (cond != nil) {
		action = build(O_TRACEON, (p->op == O_STOPI),
		    buildcmdlist(action));
		e = addevent(t, buildcmdlist(action));
		action->value.trace.event = e;
	    } else {
		e = addevent(t, buildcmdlist(action));
	    }
            e->op = p->op;
            e->cond = cond;
	    if (isstdin()) {
		printevent(e);
	    }
	} else {
	    stopinst(p->op, place, cond, action);
	}
    }
}

private stopinst(op, place, cond, action)
Operator op;
Node place;
Node cond;
Command action;
{
    Node event;
    Event e;

    if (op == O_STOP) {
	event = build(O_EQ, build(O_SYM, linesym), place);
    } else {
	event = build(O_EQ, build(O_SYM, pcsym), place);
    }
    e = addevent(event, buildcmdlist(action));
    e->op = op;
    e->cond = cond;
    if (isstdin()) {
	printevent(e);
    }
}

/*
 * Implement stopping on assignment to a variable by adding it to
 * the variable list.
 */

private stopvar(op, exp, place, cond)
Operator op;
Node exp;
Node place;
Node cond;
{
    Symbol p, tracesym;
    Node event;
    Command action;

    if (size(exp->nodetype) > MAXTRSIZE) {
	error( catgets(scmc_catd, MS_eval, MSG_132,
	      "expression too large to trace (limit is %d bytes)"), MAXTRSIZE);
    }
    if (exp->op == O_SYM && isblock(exp->value.sym)) {
	warning( catgets(scmc_catd, MS_eval, MSG_138,
		"%s is an unusual operand for examining modification"),
		symname(exp->value.sym));
    }
    if (place == nil) {
	if (exp->op == O_LCON) {
	    p = program;
	} else {
	    p = tcontainer(exp);
	    if ((p == nil) || ismodule(p)) {
		p = program;
	    }
	}
    } else {
	p = place->value.sym;
    }
    action = build(O_STOPIFCHANGED, exp, cond);

    if (place && place->op != O_SYM)
    {
      if (op == O_STOP)
        event = build(O_EQ, build(O_SYM, linesym), place);
      else
        event = build(O_EQ, build(O_SYM, pcsym), place);

      /* Find variable being traced */
      if (exp && exp->op == O_RVAL) 
          tracesym = exp->value.arg[0]->value.sym;
      else if (exp && exp->op == O_SYM) 
          tracesym = exp->value.sym;

      /*  if the variable is active  */
      if (isvariable(tracesym) && tracesym != program
       && isactive(container(tracesym))) 
      {
        /*  save the 'initial' value  */
        initialize_trinfo(exp);
      }
    }
    else
    {
      action = build(O_TRACEON, (op == O_STOPI), buildcmdlist(action));
      event = build(O_EQ, build(O_SYM, procsym), build(O_SYM, p));
    }
    action->value.trace.event = addevent(event, buildcmdlist(action));
    action->value.trace.event->op = op;
    action->value.trace.event->exp = exp;
    action->value.trace.event->cond = cond;
    if (isstdin()) {
	printevent(action->value.trace.event);
    }
}

/*
 * NAME: assign
 *
 * FUNCTION: assign the value of an expression to a variable
 *           (or term).
 *
 * PARAMETERS:
 *      var    - Node containing variable
 *      exp    - Node containing expression
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: nothing
 */

extern Boolean call_command;

public assign (var, exp)
Node var;
Node exp;
{
    Address addr, tempaddr;
    int r, varsize, expsize;
    char c;
    short s, bad_allign;
    int i;
    LongLong l;
    union {
	float f;
	double d;
	Word w[2];
    } u;
    Boolean bitfield = false;
    unsigned int bitoffset, bitflength, bitfval, bitshift;
    unsigned int bitexpval = 0;
    Symbol svar, sexp, t;
    Boolean padvar, padexp;
    Stack *oldsp;
    extern boolean subarray_seen;

    /* if we are assigning to a c++ constant, then we put out an      */
    /* error message to indicate that this is not allowed.            */

    if (var->nodetype->language == cppLang &&
        !varIsSet("$unsafeassign") &&                        
        var->nodetype->type->isConst) 
    {
	error(catgets(scmc_catd, MS_eval, MSG_601,
              "Cannot assign a value to a const variable."));
    }

#ifdef K_THREADS
    /* if we are assigning a $ti (thread symbol ) with kernel threads */
    /* error message to indicate that this is not allowed.            */
    if (
        !varIsSet("$unsafeassign") &&
        var->nodetype->isConst)
    {
        error(catgets(scmc_catd, MS_eval, MSG_601,
              "Cannot assign a value to a const variable."));
    }
#endif /* K_THREADS */
    /* if we are assigning to or from a variable that is a c++        */
    /* reference find the size of the real variable not the size of   */
    /* the reference.                                                 */
    /* the dereferencing will occur naturally in evaluation of the    */
    /* symbols.                                                       */

    t = rtype(var->nodetype);
                                        /* if we have a subarray [..] */
    if (t->class == RANGE && subarray_seen) {
                                        /* output an error message    */
	error(catgets(scmc_catd, MS_eval, MSG_638,
              "Cannot assign to a subarray of variables."));
                                        /* if we have a ptr to member */
    } else if (t->class == PTRTOMEM) {
                                        /* output an error message    */
	error(catgets(scmc_catd, MS_eval, MSG_602,
              "Cannot assign to a pointer to member variable."));
    }
    else {
       varsize = size(var->nodetype);
    }
    if (isconst(exp->nodetype))
      expsize = size(exp->nodetype->symvalue.constval->nodetype);
    else
      expsize = size(exp->nodetype);

    /* account for fortran padded type when AUTODBL is on */
    if ((svar = var->nodetype)->class == VAR)  svar=svar->type;
    if ((sexp = exp->nodetype)->class == VAR)  sexp=sexp->type;
    padvar = is_fortran_padded(svar);
    padexp = is_fortran_padded(sexp);
    if ((padvar || padexp) && !(padvar && padexp)) {
	if (padvar)
	   varsize = svar->symvalue.field.length;
	else
	   expsize = sexp->symvalue.field.length;
    }

    svar = rtype(var->nodetype);
    sexp = rtype(exp->nodetype);
    if (var->op == O_SYM) {
	r = regnum(var->value.sym);
	if (r != -1) {
	    eval(exp);
	    if (varsize == sizeof(double)) {
                if (expsize == sizeof(char)) {
                    u.d = (double) pop(char);
                } else if (expsize == sizeof(short)) {
                    u.d = (double) pop(short);
                } else if (expsize == sizeof(float)) {
		    u.d = pop(float);
		} else if (expsize == sizeof(double)) {
		    u.d = pop(double);
		} else {
		    sp -= expsize;
                    error( catgets(scmc_catd, MS_check, MSG_1,
                                                "incompatible types"));
		}
		if (r < (NGREG+NSYS)) {
		    setreg(r, u.w[0]);
		    setreg(r+1, u.w[1]);
		}
		else {
		    setreg(r, u.w[0]);
		    setreg(-r, u.w[1]);
		}
	    } else {
                if (expsize == sizeof(char)) {
                    u.f = (Word) pop(char);
                } else if (expsize == sizeof(short)) {
                    u.f = (Word) pop(short);
                } else if (expsize == sizeof(double)) {
                    if (sexp->class == REAL)
                      u.f = pop(double);
                    else
                      u.w[0] = (Word) pop (LongLong);
		} else if (expsize == sizeof(Word)) {
		    u.w[0] = pop(Word);
		} else {
		    sp -= expsize;
                    error( catgets(scmc_catd, MS_check, MSG_1,
                                                "incompatible types"));
		}
		setreg(r, u.w[0]);
	    }
	    assign_addr = NOADDR;
	    action_mask |= ASSIGNMENT;
	    return;
	}
    }

    /* lval() should never change sp, workaround here for assignment */
    /* with casting, e.g. casting struct to int for assignment.      */
    oldsp = sp;
    addr = lval(var);
    sp = oldsp;

    assign_addr = addr;
    assign_size = varsize;
                                     /* reset pointer to member flag that*/
                                     /* was set during the evaluation of */
                                     /* the variable or expression       */
    specificptrtomember = false;

    /* adjustment for length bytes of pascal STRING */
    /* and put in new length bytes                  */
    if ((svar->class == STRING) and (sexp->class != STRING)) {
      s = (short) expsize;
      /* account for the null end of C string */
      if (istypename(sexp->type,"$char")) s--;
      dwrite((char *) &s, addr, sizeof(short)); 
      addr = addr + 2;
    }
    
    if (bitfield = isbitfield(var->nodetype)) {
	dread(&bitfval, addr, sizeof(Word)); 
	/* Clear out the assignee bits so that they may be ored in later */
	bitoffset = var->nodetype->symvalue.field.offset % BITSPERBYTE;
	bitflength = var->nodetype->symvalue.field.length;
	bitshift = BITSPERWORD - bitoffset - bitflength;
	bitfval &= ~(((1 << bitflength) - 1) << bitshift);
    }
    eval(exp);

    if (svar->class == PACKRANGE)
    {
      sp -= expsize;
      if (expsize <= varsize)
      {
        dwrite(sp, addr, expsize);
      }
      else
      {
        dwrite(sp+(expsize-varsize),addr,varsize);
      }
    }
    else if ((var->nodetype->type->class==PTR)
          && (exp->nodetype->class==ARRAY) && 
        ischartype(var->nodetype->type) &&
        !strcmp(exp->nodetype->type->name->identifier,"$char")) 
    {                            /* This section is for assigning strings */
       call_command = true;      /* to character pointers.                */
       if ( bad_allign = 4 - (expsize % 4))
       {
          for (i = 0; i < bad_allign; i++)
          {
             *sp++ = '\0';
             expsize++;
          }
       }
       tempaddr = debugee_malloc((unsigned int)expsize);  /* get new memory  */
       /* Check for bad return code here .....  */
       if (tempaddr == 0) {
	   error( catgets(scmc_catd, MS_eval, MSG_139,
			   "Unable to allocate memory for character pointer"));
	   return;
       }
       dwrite((char *)&tempaddr, addr, sizeof(tempaddr)); /* assign new mem. */
       sp -= expsize;
       dwrite(sp, tempaddr, expsize);                     /* assign string */
       call_command = false;
    } else if (isspecialpic(svar) && !ispic(sexp)) {
      cobolassign(var, exp);
    } else if (bitfield) {
    /* Value to be changed is left-justified at the byte level.
     * The expression is left justified as well, but it can be of any
     * length.  So, what to do?  An easy, though not necessarily optimal
     * method is to change the expression to be right-justified in a word,
     * shifting the expression over the appropriate number of bits, masking
     * off the excess bits, then or'ing in the new value.
     */
       sp -= expsize;
       /* Get value, right-justified */
       for (i = 0; i < expsize; i++)
	    bitexpval = (bitexpval << BITSPERBYTE) | sp[i];
       /* Mask off excess bits, and shift into position */
       bitexpval = (bitexpval & ((1 << bitflength) - 1)) << bitshift;
       /* Or in the new value, and write it out.  Done. */
       bitfval |= bitexpval;
       dwrite(&bitfval, addr, sizeof(Word));
    }
    else if ((svar->class == COMPLEX) && (sexp->class == COMPLEX))
    {
      double real_pad = 0.0, real_exp;
      double imag_pad = 0.0, imag_exp;
      float real_var, imag_var;

      switch (expsize)
      {
        case sizeof (double) :
          imag_exp = (double) pop(float);
          real_exp = (double) pop(float);
          break;

        case 2 * sizeof (double) :
          imag_exp = pop(double);
          real_exp = pop(double);
          break;

        case 4 * sizeof (double) :
          imag_pad = pop(double);
          imag_exp = pop(double);
          real_pad = pop(double);
          real_exp = pop(double);
          break;
      }

      switch (varsize)
      {
        case sizeof (double) :
          real_var = (float) real_exp;
          imag_var = (float) imag_exp;
            
          dwrite((char *)&real_var, addr, sizeof(float));         
          dwrite((char *)&imag_var, addr + sizeof (float),
                 sizeof(float));         
          break;

        case 2 * sizeof (double) :
          dwrite((char *) &real_exp, addr, sizeof (double));
          dwrite((char *) &imag_exp, addr + sizeof (double),
                 sizeof (double));
          break;

        case 4 * sizeof (double) :
          dwrite((char *) &real_exp, addr, sizeof (double));
          dwrite((char *) &real_pad, addr + sizeof(double),
                 sizeof (double));
          dwrite((char *) &imag_exp, addr + 2 * sizeof (double),
                 sizeof (double));
          dwrite((char *) &imag_pad, addr + 3 * sizeof (double),
                 sizeof (double));
          break;
      }
    }
    else
    {
      int rc;
      char *save_sp;

      expsize = size(sexp);

      save_sp = sp - expsize;

      rc = convert(save_sp, sexp, save_sp, svar);
      if (rc == 0)
      {
        dwrite(save_sp, addr, varsize);
        sp = save_sp;
      }
      else
      {
        sp -= expsize;
        if (expsize <= varsize)
        {
          if (initializestring)
          {
            initializestring = 0;
            for (i=0;i<varsize-expsize;i++)
              *(sp+expsize+i)='\0';
            expsize=varsize;
          }
          dwrite(sp, addr, expsize);
        }
        else
        {
          dwrite(sp, addr, varsize);
        }
      }
    }
    action_mask |= ASSIGNMENT;
}

/*
 * Set a debugger variable.
 */

private set (var, exp)
Node var, exp;
{
    Symbol t;

    if (var == nil) {
	defvar(nil, nil);
    } else if (exp == nil) {
	defvar(var->value.name, nil);
    } else if (var->value.name == identname("$frame", true)) {
	t = exp->nodetype;
	if (not compatible(t, t_int) and not compatible(t, t_addr)) {
	    error( catgets(scmc_catd, MS_eval, MSG_140,
						 "$frame must be an address"));
	}
	eval(exp);
	getnewregs(pop(Address));
    } else {
	defvar(var->value.name, unrval(exp));
    }
}

extern Lineno lastlinenum;
/*
 * Execute a list command.
 */

private list (p)
Node p;
{
    Symbol f;
    Address addr;
    Lineno line, l1, l2;
    Node windownode;

    if (p->value.arg[0]->op == O_LCON) {
	eval(p->value.arg[0]);
	l1 = (Lineno) (pop(long));
	eval(p->value.arg[1]);
	l2 = (Lineno) (pop(long));
	if ((l1 < 0) || (l2 < 0)) {
		beginerrmsg();
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_eval, MSG_143,
					   "line numbers must be positive\n"));
		return;
	}
	if (l2 == 0) {
	   if (l1 == 0 && p->op != O_MOVE) {  /* No line numbers specified */
		windownode = findvar(identname("$listwindow",true));
		if (windownode == nil)
			windowsize = 10;
		else {
			eval(windownode);
			windowsize = pop(integer);
		}
		l1 = cursrcline;
		l2 = cursrcline + windowsize - 1;
	   }
	   else {   	/* Only 1st line specified */
		l2 = l1;
	   }
	}  /* No ending line specified */
    }
    else {
	f = p->value.arg[0]->nodetype;

	if (nosource(f)) {
	    error(catgets(scmc_catd, MS_eval, MSG_141,
		  "no source lines for \"%s\""), symname(f));
	}
	addr = firstline(f);
	if (addr == NOADDR) {
	    error(catgets(scmc_catd, MS_eval, MSG_141,
		  "no source lines for \"%s\""), symname(f));
	}
	setsource(srcfilename(addr));
	line = srcline(addr);
	getsrcwindow(line, &l1, &l2);
    }
    if (p->op != O_MOVE)
        printlines(l1, l2);
    else {
	/*
	 * If both operands are the same, clear one so we don't try to free them
	 * both
	 */
	if( p->value.arg[0] == p->value.arg[1] ) {
	    p->value.arg[1] = NULL;
	}

	if( canReadSource() == false || lastlinenum == 0 ) {
		beginerrmsg();
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_eval, MSG_145,
						  "No file to move within\n"));
		return;
	} else if ((l1 <= 0) || (l2 > lastlinenum)) {
		beginerrmsg();
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_eval, MSG_146,
				"Line specified is not within range of %s\n"), 
				basefile(cursource));
		return;
	}
	if( isXDE ) {
	    /*
	     * The DPI debuggers expect cursrcline to be the line one past the
	     * line moved to
	     */
	    cursrcline = l1 + 1;
	} else {
	    cursrcline = l1;
	}
	action_mask |= ELISTING;
    }
}

/*
 * Execute a listi command.
 */

private listi (p)
Node p;
{
    static Address old_pc;
    Address addr1, addr2;
    Node windownode;
    Symbol f;

#ifdef KDBX
    is_diss = 1;
#endif /* KDBX */
    /*
     * Get current value of $listwindow to determine how many instructions
     * to display
     */
    windownode = findvar(identname("$listwindow",true));
    if (windownode == nil) {
	inst_windowsize = 10;
    } else {
	eval(windownode);
	inst_windowsize = pop(integer);
    }

    if (p->value.arg[0]->op == O_SYM)
    {
	f = p->value.arg[0]->nodetype;
	addr1 = firstline(f);
	if (addr1 == NOADDR) {
	    error( catgets(scmc_catd, MS_eval, MSG_141,
				    "no source lines for \"%s\""), symname(f));
	}
	printninst(inst_windowsize,addr1);
    } 
    else 
    {
	eval(p->value.arg[0]);
	addr1 = (Address) (pop(long));
	eval(p->value.arg[1]);
	addr2 = (Address) (pop(long));
	if (addr2 == 0)
        {
	   if (addr1 == 0)           /* No line numbers specified */
	   { 
	      if (prtaddr && (old_pc == pc))
	         printninst(inst_windowsize,prtaddr);
	      else
	         printninst(inst_windowsize,pc);
	   }
	   else     	             /* Only 1st line specified   */
	   {
	      printninst(inst_windowsize,addr1);
	   }
	}                            /* end if no ending line specified */
	else
	{
	   if (addr1 == 0)        /* print assembly code given source line # */
	   {
	      addr1 = objaddr(addr2,cursource);
	      if (addr1 == NOADDR) {
	          error( catgets(scmc_catd, MS_eval, MSG_149,
			      "No assembly code for that source line number"));
	      }
	      printninst(inst_windowsize,addr1);
	   }
	   else if (addr2 < addr1)
           {
	      beginerrmsg();
	      (*rpt_error)(stderr,  catgets(scmc_catd, MS_eval, MSG_144,
				"second number must be greater than first\n"));
	      return;
	   }
	   else                                /* Both lines specified */
	   {
	      if ((addr2 - addr1) < inst_windowsize)
	         printninst(inst_windowsize,addr1);
	      else
	         printinst(addr1,addr2);
	   }
	}
    }
    old_pc = pc;
}

/*
 * Execute a registers command to print out the registers in readable form.
 */

registers () {
	int p,i;
	char	**real_regnames;           	/* Used to point to the list of
						 * registers to be displayed;
						 * This defaults to using
						 * regnames
						 */

	setregs(process);

        /*  if running on a 601  */
	if (current_hardware & UNQ_601)
	  real_regnames = regnames_601;

        /*  else if running on some other powerpc machine  */
        else if (current_hardware & PPC)
	  real_regnames = regnames_ppc;
        
        else
           real_regnames = regnames;

	for (p=0; p < NGREG;) {
		for (i=0;i<4;i++) {
		     (*rpt_output)(stdout, "%5s: ", real_regnames[p]);
		     (*rpt_output)(stdout, "0x%08x  ", reg(p));
		      p++;
		}
	(*rpt_output)(stdout, "\n");
	}
	for (p = NGREG; p < NGREG+NSYS-FPSTATREG;) {
		for (i=0;i<4;i++) {
		    if( real_regnames[p] == NULL ) {
			p++;
		    } else {
			if (p >= NGREG+NSYS-FPSTATREG) {
			    (*rpt_output)(stdout, "\n");
			    break;
			}
			(*rpt_output)(stdout, "%5s: ", real_regnames[p]);
			(*rpt_output)(stdout, "0x%08x  ", reg(p));
			p++;
		    }
		}
	(*rpt_output)(stdout, "\n");
	}
	printcond();
	if (!varIsSet("$noflregs"))
	    prflregs();
	else 
	  (*rpt_output)(stdout,  catgets(scmc_catd, MS_eval, MSG_545,
		"\t[unset $noflregs to view floating point registers]"));
        (*rpt_output)(stdout, "\n");
        getpc(&pc);
        printloc();
        (*rpt_output)(stdout, "\n");
        prtaddr = printop(pc,0);
}

/*
 * Print out the values of the floating point registers.
 */
prflregs()
{
	int i;
	switch(fpregs) {
	   case 32:
	   	for (i = 0; i <= fpregs; ) {
		     (*rpt_output)(stdout, "%s$fr%d:", (i>9)?" ":"  ",i);
		      prhexflt(fpregval(i++),0);
		     (*rpt_output)(stdout, "%s$fr%d:", (i>9)?"  ":"   ",i);
		      prhexflt(fpregval(i++),0);
		      if (i == fpregs) {
#ifdef FPSCR
		       (*rpt_output)(stdout, " $fpscr: ");
		       (*rpt_output)(stdout, "0x%08x  ", reg(SYSREGNO(FPSCR)));
			i++;
#else
		        (*rpt_output)(stdout, "   $fst: ");
		         prhexflt(fpregval(i++),1);
#endif
		      }
		      else {
		      	(*rpt_output)(stdout, "%s$fr%d: ", (i>9)?"  ":"   ",i);
		         prhexflt(fpregval(i++),0);
		      }
		     (*rpt_output)(stdout, "\n");
	        }
		break;
	   case 7:
	   case 8:
	   default:
		  (*rpt_output)(stdout, "\n");
		   for (i=0;i<fpregs;i++) {
		       (*rpt_output)(stdout, "  $fr%d: ", i);
		        prhexflt(fpregval(i),0);
			if ((i+1) % 3 == 0)
			   (*rpt_output)(stdout, "\n");
		   }
		  (*rpt_output)(stdout, "  $fst: ");
		   prhexflt(fpregval(i),1);
		  (*rpt_output)(stdout, "\n");
		   break;
	}
}
	
prhexflt(d,stat_reg)
double d;
int stat_reg;
{
   int *dptr = (int *)&d;
  (*rpt_output)(stdout, "0x%8.8x",*dptr++);
   if (!stat_reg)
   (*rpt_output)(stdout, "%8.8x",*dptr);
}

printpc(pc)
Address pc;
{
	printinst(pc,pc);
}

/*
 * Execute a func command.
 */

private func (p)
Node p;
{
    Symbol s, f;
    Address addr;
    extern boolean heat_shrunk;
    struct Frame frp;

    if (p == nil) {
	if (isinline(curfunc))
	   (*rpt_output)(stdout, "unnamed block ");
        /*  if this is a heat shrunk executable and the function
              name is found in the traceback table  */
        if (heat_shrunk && (getcurframe(&frp) == 0)
         && (frp.tb.name_present && frp.name))
        {
          /*  print the function name found in the traceback table  */
          (*rpt_output)(stdout, "%s", frp.name);
        }
        else
          printname( rpt_output, stdout, curfunc, false);
	(*rpt_output)(stdout, "\n" );
    } else {
	s = p->nodetype;
	if (isroutine(s)) {
	    setcurfunc(s);
	} else {
	    find(f, s->name) where isroutine(f) endfind(f);
	    if (f == nil) {
		error( catgets(scmc_catd, MS_eval, MSG_180,
			     "%s is not a procedure or function"), symname(s));
	    }
	    setcurfunc(f);
	}
	addr = codeloc(curfunc);
	if (addr != NOADDR) {
	    if (!nosource(curfunc)) {
	         setsource(srcfilename(addr));
		 cursrcline = srcline(addr);
		 if( isXDE ) {
		     /*
		      * The DPI debuggers expect cursrcline to be one past the
		      * first line of the function
		      */
		     cursrcline++;
		 }
	    }
	}
    }
}

/*
 * Set the program counter to be at a particular place in the code.
 */
extern Boolean never_ran, noexec;
public changepc (p)
Node p;
{
    Address goaddr;
    String fn;
    long ln;
    Symbol oldfunc;

    if (noexec)
	error( catgets(scmc_catd, MS_eval, MSG_181,
						 "program is not executable"));
    oldfunc = curfunc;
    if (p->value.arg[0]->op == O_QLINE) {
	fn = p->value.arg[0]->value.arg[0]->value.scon;
	ln = p->value.arg[0]->value.arg[1]->value.lcon;
	goaddr = objaddr(ln, fn);
    } else {
/*
        Commented this out to give gotoi more capabilities.
	goaddr = (Address) p->value.arg[0]->value.lcon;	
*/
        eval(p->value.arg[0]);
        goaddr = pop(long);
    }
    if (goaddr != NOADDR) {
	setcurfunc(whatblock(goaddr));
	if ((curfunc != oldfunc) && (!(varIsSet("$unsafegoto")))) {
	    curfunc = oldfunc;
	    error( catgets(scmc_catd, MS_eval, MSG_782,
	    "Goto address is not within current function or block. \
(set $unsafegoto to override)"));
	} else {
	    setreg(SYSREGNO(PROGCTR), pc = goaddr);
	    getsrcpos();	
	    action_mask |= EXECUTION;
	    action_mask &= ~CONTEXT_CHANGE;
	    action_mask &= ~LISTING;
	    action_mask &= ~ELISTING;
	    printstatus();
	}
    } else {
        beginerrmsg();
        (*rpt_error)(stderr,  catgets(scmc_catd, MS_events,
                                      MSG_101, "no executable code at line "));
        (*rpt_error)(stderr, "%d", ln);
        enderrmsg();
    }
}
/*
 * Send a message to the current support person.
 */

public gripe()
{
#   ifdef MAINTAINER
	typedef Operation();
	Operation *old;
	int pid, status;
	extern int versionNumber;
	char subject[100];

	puts("Type control-D to end your message.  Be sure to include");
	puts("your name and the name of the file you are debugging.");
	(*rpt_output)(stdout, "\n" );
	old = signal(SIGINT, SIG_DFL);
	sprintf(subject, "dbx (version 3.%d) gripe", versionNumber);
	pid = back("Mail", stdin, stdout, "-s", subject, MAINTAINER, nil);
	signal(SIGINT, SIG_IGN);
	pwait(pid, &status);
	signal(SIGINT, old);
	if (status == 0) {
	    puts("Thank you.");
	} else {
	    puts("\nMail not sent.");
	}
#   else
	puts("Sorry, no dbx maintainer available to gripe to.");
	puts("Try contacting your system manager.");
#   endif
}


/*
 * Divert output to the given file name.
 * Cannot redirect to an existing file.
 */

private int so_fd;
Boolean notstdout;
extern enum redirect { CREATE, APPEND, OVERWRITE } stdout_mode;

public setout(filename)
String filename;
{
    File f;
    int oflag;

    switch (stdout_mode)
    {
       case CREATE:
            f = fopen(filename, "r");
            oflag = O_RDWR | O_CREAT | O_TRUNC;
            break;
       case APPEND:
            f = fopen(filename, "a");
            oflag = O_RDWR | O_CREAT | O_APPEND;
            break;
       case OVERWRITE:
            f = fopen(filename, "w");
            oflag = O_RDWR | O_CREAT | O_TRUNC;
            break;
    }
    if ((f != nil) && (stdout_mode == CREATE)) {
	fclose(f);
	error( catgets(scmc_catd, MS_eval, MSG_210,
			     "%s: file already exists, use \">!\""), filename);
    } else {
	so_fd = dup(1);
	close(1);
	if (open(filename, oflag, 0664) == -1) {
	    unsetout();
	    error( catgets(scmc_catd, MS_eval, MSG_211,
						 "cannot create %s"), filename);
	}
	notstdout = true;
    }
}

/*
 * NAME:  screen
 *
 * FUNCTION: set up an Xwindow for screen subcommand or 
 *           for child process when in multprocess debugging.
 *
 * PARAMETERS:
 *      is_screen:  input parameter,
 *                  1 for screen subcommand, 0 for multprocess debugging. 
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 when not able to setup an Xwindow.
 *
   The following diagram is the processes relationship for dbx multprocess
   debugging.  Single line indicates parent-child relationship, double line 
   indicates debugger-debuggee relationship, triple line indicates both of 
   the above relationship.

     originating dbx  ---------------  child dbx 
      fork a child                    fork a child 
   then exec debuggee               then exec aixterm
         |||                              |
         |||                              | 
         |||                              |
         |||                             dbx
         |||                             ||
         |||                             ||
         |||                             ||
       debuggee  -----------------  child debuggee


   The following diagram is the processes relationship for dbx subcommand
   screen.

     originating dbx  ---------------  child dbx
      fork a child                   exec aixterm then
   then exec debuggee         redir dbx I/O to this aixterm
         |||                              
         |||                             
         |||                            
         |||                       
       debuggee 
 */
public int screen(is_screen)
Boolean is_screen;
{
    Node p;
    int oldfd, fd, this_pid, fork_return;
    char name_buffer[23];  
    char *tmp_pipe_name = &name_buffer[0];
    int pid_open_tty = 0;  /* pid of the process which will open an Xwindow */

    /*  NOTE : mktemp modifies the input parameter - therefore, we
          cannot pass "/tmp/dbxtmppipe.XXXXXX" directly  */
    strcpy (tmp_pipe_name, "/tmp/dbxtmppipe.XXXXXX");
    tmp_pipe_name = mktemp(tmp_pipe_name);

    if(mknod(tmp_pipe_name, _S_IFIFO | S_IREAD | S_IWRITE, 0) == -1)
    {
       perror("dbx: mknod()");
       return(-1); 
    }
    oldfd = dup(0);

    /*  For the screen command, the child should create the new XWindow.
          For the multproc command, the parent should.  */

    /*  NOTE : fork returns 0 to the child process and the child's process
                 id to the parent process  */

    this_pid = getpid();
    fork_return = fork();

    /*  if this is the screen command and the parent  */
    if (is_screen && (fork_return > 0))
      pid_open_tty = ScrPid = fork_return;

    /*  else if this is multproc and the child  */
    else if (!is_screen && (fork_return == 0))
      pid_open_tty = this_pid;     

    /*  if this is the screen command and the child or
          multproc and the parent  */
    if(pid_open_tty == 0)
    {
       char *sp;
       char *pp[20]; /* store arguments to the command which open Xwindow */
       char *term_name;
       int z = 0;

       term_name = getenv("TERM");

       pp[z++] = (term_name == NULL) ? "aixterm" : term_name;

       if(!is_screen) {
          sp=(char *)(malloc(16));  /* 16 chars is big enough to hold 
                                       the int typed newdebuggee in ascii 
                                       plus the string "dbx " and null char */
          sprintf(sp,"dbx %d",newdebuggee);
          pp[z++]= "-T";
          pp[z++] = sp;
          pp[z++]= "-n";
          pp[z++] = sp;
       }
       p = findvar(identname("$xdisplay", false));

       if (p != NULL)
       {
         pp[z++] = "-display";
         pp[z++] = p->value.scon;
       }

       pp[z++] = "-name";
       pp[z++] = "dbx_xterm";
       pp[z++]= "-e";
       pp[z++]= graphical_debugger;
       pp[z++]= "-b";
       pp[z++] = tmp_pipe_name;
       pp[z] = NULL;
       execvp(pp[0],pp);
       exit(1);
    }
    else
    {
       char name_tty[30];
       char *sp;
       fd = open(tmp_pipe_name, O_RDWR);
       fcntl(fd, F_SETFL, O_NDELAY);
       
       while(1) {
          if(getpri(pid_open_tty) < 0) {  
             /* fail to open Xwindow */
             if(is_screen) 
               warning(catgets(scmc_catd, MS_eval, MSG_712,
               "dbx subcommand screen fails. dbx continued\n"));
             else 
               warning(catgets(scmc_catd, MS_eval, MSG_711,
               "dbx multproc fails. dbx continued with multproc disabled\n"));
             close(fd);
             remove(tmp_pipe_name);
             return(-1);
          }
          if(read(fd,name_tty,30) > 0) { 
             /* Xwindow is open successfully */
             close(fd);
             remove(tmp_pipe_name);
             break; 
          }
       }
       fd = open(name_tty, O_RDWR);
       /* restore the tty to the originating tty */
       ioctl(fd, TCSETA, &(ttyinfo.ttyinfo));
       (void) fcntl(fd, F_SETFL, ttyinfo.fcflags);
       close(0);                        /* reset fd 0, 1, & 2  */
       dup(fd);
       close(1);
       dup(0);
       close(2);
       dup(0);
       close(fd);
       if(is_screen) ScrUsed = true;
       return oldfd;
    }
}

/*
 * Revert output to standard output.
 */

public unsetout()
{
    /* flush if stdout is not closed */
    if (fcntl(stdout->_file, F_GETFL) != -1)
        fflush(stdout);
    close(1);
    if (dup(so_fd) != 1) {
	panic( catgets(scmc_catd, MS_eval, MSG_212,
						   "standard out dup failed"));
    }
    close(so_fd);
    notstdout = false;
}

/*
 * Determine is standard output is currently being redirected
 * to a file (as far as we know).
 */

public Boolean isredirected()
{
    return notstdout;
}

public chkenable (s)
char *s;
{
  extern int strcmpi();
  if (!strcmpi(s,"on"))
      return (int) on;
  if (!strcmpi(s,"parent"))
      return (int) parent;
  if (!strcmpi(s,"child"))
      return (int) child;
  if (!strcmpi(s,"off"))
      return (int) off;

  (*rpt_error)(stderr,"Usage: multproc { on | parent | child | off }\n");
      return 0;
}

public getcase (s)
char *s;
{
  lowercase(s);
  if (!strcmp(s,"mixed"))
      return (int) mixed;
  else if (!strcmp(s,"lower"))
      return (int) lower;
  else if (!strcmp(s,"upper"))
      return (int) upper;
  else if (!strcmp(s,"default"))
      return (int) filedep;
  else {
      (*rpt_error)(stderr,
		       "Usage: case { default | mixed | lower | upper }\n");
      return -1;
  }
}

public lowercase (s)
char *s;
{
  char *c;
  for (c = s; *c; c++)
      *c = (char) tolower(*c);
}

public uppercase (s)
char *s;
{
  char *c;
  for (c = s; *c; c++)
      *c = (char) toupper(*c);
}
