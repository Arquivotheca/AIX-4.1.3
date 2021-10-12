static char sccsid[] = "@(#)76    1.27.3.26  src/bos/usr/ccs/lib/libdbx/runtime.c, libdbx, bos41J, 9521A_all 5/22/95 18:25:58";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: callproc, chkparam, curfuncframe, debugee_malloc, donext, down,
 *	      dump, dumpall, evalargs, findframe, firstline, flushoutput,
 *	      getcurfunc, isactive, lastaddr, nextfunc, popenv, printcallinfo,
 *	      procreturn, pushenv, return_addr, runtofirst, savepc, setcurfunc,
 *	      unsafe_evalargs, up, walkstack, wherecmd,
 *            procreturn_threadheld,  popenv_threadheld
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
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
 * Runtime organization dependent routines, mostly dealing with
 * activation records.
 */

#include "defs.h"
#include "envdefs.h"
#include "runtime.h"
#include "frame.h"
#include "process.h"
#include "machine.h"
#include "events.h"
#include "mappings.h"
#include "symbols.h"
#include "languages.h"
#include "tree.h"
#include "eval.h"
#include "operators.h"
#include "object.h"
#include "cplusplus.h"
#include <sys/reg.h>
#include <signal.h>
#ifdef K_THREADS
#include "k_thread.h"
#endif /* K_THREADS */

#define savepc(frp)	((frp)->save_pc)

Boolean call_command;  
public Boolean walkingstack = false;
public Frame callframe = nil;		/* frame where the call was made */
public Word caller_reg[NGREG+NSYS+1];	     /* holders of all registers */
public double caller_freg[MAXSAVEFREG];	     /* when the call was made.  */
private boolean context_change = false;
extern boolean isprolog();
extern Address minlineaddr;
extern boolean isXDE;
private Address funcAddr;
extern Language fLang;

typedef struct {
    Node callnode;
    Node cmdnode;
    boolean isfunc;
} CallEnv;

private CallEnv endproc;

/*
 * Get the current frame information in the given Frame and store the
 * associated function in the given value-result parameter.
 */

public getcurfunc (frp, fp)
Frame frp;
Symbol *fp;
{
    if (getcurframe(frp) < 0)
	return -1;
    *fp = whatblock(savepc(frp));
    return 0;
}

/*
 * Return the frame associated with the next function up the call stack, or
 * nil if there is none.  The function is returned in a value-result parameter.
 * For "inline" functions the statically outer function and same frame
 * are returned.
 */

public Frame nextfunc (frp, fp)
Frame frp;
Symbol *fp;
{
    Symbol t;
    Frame nfrp, prevframe;

    t = *fp;
 
    prevframe = (Frame)malloc(sizeof(struct Frame));
    prevframe->save_lp = frp->save_lp;
    prevframe->save_fp = frp->save_fp;
    prevframe->arsize = frp->arsize;
    prevframe->save_pc = frp->save_pc;

    checkref(t);
    if (isinline(t)) 
    {
	t = container(t);
	nfrp = frp;
    } 
    else 
    {
	nfrp = nextframe(frp, false);
	if (nfrp == nil)
	    t = nil;
	else if (frameeq(prevframe, nfrp)) 
	{
	    warning(catgets(scmc_catd, MS_runtime, MSG_262,
	          "Frame information in error at 0x%x"), savepc(nfrp));
	    t = nil;
	    nfrp = nil;
	} 
	else 
        {
	    t = whatblock(savepc(nfrp));
 
            /*  Allow frame traversal from within a "call" - this
                  "stops" the traversal  */
            if (streq(symname(t), "__dbsubc") 
             || streq(symname(t), "__dbsubg"))
            {
              t = NULL;
              nfrp = NULL;
            }
        }

    }
    *fp = t;
    free(prevframe);
    return nfrp;
}

/*
 * Return the frame associated with the given function.
 * If the function is nil, return the most recently activated frame.
 *
 * Static allocation for the frame.
 */

public Frame findframe(f)
Symbol f;
{
    register Frame frp;
    static struct Frame frame;
    Symbol p;

    frp = &frame;
    if (getcurframe(frp) < 0)
	return nil;
    if (f != nil) {
	if (f == curfunc and curframe != nil) {
	    *frp = *curframe;
	} else {
	    p = whatblock(savepc(frp));
	    for (;;) {
		if (p == f) {
		    break;
		} else if (p == program) {
		    frp = nil;
		    break;
		/* Handle 'call' situations, when we are back to */
		/* __dbsubc, we want to continue from the frame  */
		/* where the call command was made, callframe.   */
		} else if (!strcmp(symname(p),"__dbsubc")) {
		    if (callframe == nil) break;
		    memcpy(frp, callframe, sizeof(struct Frame)); 
		    p = whatblock(savepc(frp));
		    continue;		

		} else {
		    frp = nextfunc(frp, &p);
		    if (frp == nil) {
			break;
		    }
		}
	    }
	}
    }
    return frp;
}

/*
 * Find the return address of the current procedure/function.
 */

public Address return_addr ()
{
    Frame frp;
    Address addr;
    struct Frame frame;

    frp = &frame;
    if (getcurframe(frp) < 0)
	return 0;
    frp = nextframe(frp, (call_command) ? true : false);
    if (frp == nil) {
	addr = 0;
    } else {
	addr = savepc(frp);
    }
    return addr;
}

/*
 * Print out the information about a call, i.e.,
 * routine name, parameter values, and source location.
 */

private printcallinfo (f, frp)
Symbol f;
Frame frp;
{
    Lineno line;
    Address caller;
    String srcfile;
    extern boolean heat_shrunk;

    caller = frp->save_pc;
    if (caller != reg(SYSREGNO(PROGCTR)) && caller != minlineaddr) {
	/*
	 * Unless we are in the current procedure, we should use pc-n
	 * for the traceback so that we see the call as the current location
	 * in the caller.  The pc often points to the line following the call.
	 */
	 /* Let's at least back it up to the privous instruction. */
	caller -= MIN_INST_SZ;
    }
    if (isinline(f)) {
	(*rpt_output)(stdout, "unnamed block ");
    }
    if (frp->tb.name_present && frp->name != nil && frp->name[0] != '\0')
        (*rpt_output)(stdout, "%s", frp->name);
    else {
        printname(rpt_output, stdout, f, false);
    }
    if ((not isinline(f)) && (!(varIsSet("$noargs")))) {
	printparams(f, frp);
    }
    line = srcline(caller);
    srcfile = srcfilename(caller);
    if (line != 0 && srcfile != nil) {
	(*rpt_output)(stdout, ", line %d", line);
	(*rpt_output)(stdout, " in \"%s\"\n", basefile(srcfile));
    } else {
	(*rpt_output)(stdout, " at 0x%x", caller);
        if (heat_shrunk && (frp->save_pc != frp->orig_loc))
          /*  print out the original location (adjusted)  */
          (*rpt_output)(stdout, " = fdpr[0x%x]",
                        frp->orig_loc - (frp->save_pc - caller));
        (*rpt_output)(stdout, "\n");
    }
}

/*
 * Walk the stack of active procedures printing information
 * about each active procedure.
 */

private walkstack(dumpvariables)
Boolean dumpvariables;
{
    Frame frp;
    boolean save;
    Symbol f;
    struct Frame frame;
    cases savecase;
#ifdef KDBX
    Address prev_fp;
    extern void kdbx_show_stack();	/* print stack in user mode */
    extern int lldb_kdbx_lvl;		/* Set if the debugger supports */
					/* tracing thru system calls */
#endif

    if (notstarted(process) or isfinished(process)) {
	error( catgets(scmc_catd, MS_runtime, MSG_265,
						     "program is not active"));
    } else {
	savecase = symcase;	/* Save case mode; shouldn't change here. */
	save = walkingstack;
	walkingstack = true;
	showaggrs = dumpvariables;
	frp = &frame;
	if (getcurfunc(frp, &f) < 0) {
	     warning( catgets(scmc_catd, MS_runtime, MSG_269,
				      "Could not determine current function"));
		/* initial frame corrupt, check next frame */ 
             frp->save_fp = reg(STKP);
             frp->save_lr = reg(SYSREGNO(LR));
             dread(&frp->caller_fp,frp->save_fp,4);
             frp->prolog=false;
             frp->tb.saves_lr=1;
             f = whatblock(frp->save_fp);
             frp = nextfunc(frp,&f);
	     if (frp == nil or endofstack(frp)) 
		return;
	}
	for (;;) {
	    printcallinfo(f, frp);
	    if (dumpvariables) {
		dumpvars(f, frp);
		(*rpt_output)(stdout, "\n" );
	    }
#ifdef KDBX
	    prev_fp = frp->save_fp;
#endif
	    frp = nextfunc(frp, &f);
	    if (frp == nil or endofstack(frp)) {
		break;
	    }
#ifdef KDBX
	    if ( (lldb_kdbx_lvl < 1) || !varIsSet("$where_thru_sc")) {
              if (frp->save_fp < prev_fp)
                break;
            } else {
              if (frp->save_fp < prev_fp) {
		(*rpt_output)(stdout,
		    "\n	Continuing the stack trace before the system call\n");
		Dprintf("save_fp=0x%x, save_pc= 0x%x, save_lr= 0x%x\n",
			frp->save_fp, frp->save_pc, frp->save_lr);
                kdbx_show_stack ( FALSE,
			frp->save_fp, frp->save_pc, frp->save_lr);
                break;
              }
	    }
#endif
	}
	if (dumpvariables) {
	    (*rpt_output)(stdout, "in \"%s\":\n", symname(program));
	    dumpvars(program, nil);
	    (*rpt_output)(stdout, "\n" );
	}
	walkingstack = save;
	symcase = savecase;	/* Restore case mode. */
    }
}

#ifdef KDBX

/*
 * do a kernel stack trace, taking into account exceptions mst.
 */

static void kdbx_wherecmd ()
{
  extern int is_lldb_kernel_debugger;
  unsigned long prev_mst (), save_mst, pt;
  extern unsigned long kern_good_mst;
  extern unsigned long kern_mst;
  boolean done_once, i;

  walkstack(false);
  if (varIsSet("$where_thru_exc")) {
    save_mst = kern_good_mst;
    done_once = false;
    if (kern_good_mst == 0) kern_good_mst= kern_mst;
    while ((pt = prev_mst()) != NULL ) {
        (*rpt_output)(stdout, "\n	Interrupt taken (mst: 0x%.8x)\n",pt);
        kern_good_mst = pt;
        flush_cache ();
        for (i = 0; i <= regword(NGREG+NSYS+MAXFREG+1-1); i++) {
          process->valid[i] = 0;
          process->dirty[i] = 0;
        }
        done_once = true;
        walkstack(false);
    }
    if (done_once) {
      flush_cache ();
      for (i = 0; i <= regword(NGREG+NSYS+MAXFREG+1-1); i++) {
        process->valid[i] = 0;
        process->dirty[i] = 0;
      }
      cacheflush(process);
    }
    kern_good_mst = save_mst;
    flush_cache ();
  }
}
#endif /* KDBX */

/*
 * Print a list of currently active blocks starting with most recent.
 */

public wherecmd ()
{
#ifdef KDBX
    kdbx_wherecmd();
#else  /* KDBX */
    walkstack(false);
#endif /* KDBX */
}

/*
 * Print the variables in the given frame or the current one if nil.
 */

public dump (func)
Symbol func;
{
    Symbol f;
    Frame frp;

    if (func == nil) {
	f = curfunc;
	if (curframe != nil) {
	    frp = curframe;
	} else {
	    frp = findframe(f);
	}
    } else {
	f = func;
	frp = findframe(f);
    }
    showaggrs = true;
    printcallinfo(f, frp);
    dumpvars(f, frp);
}

/*
 * Dump all values.
 */

public dumpall ()
{
    walkstack(true);
}

/*
 * Set the current function to the given symbol.
 * We must adjust "curframe" so that subsequent operations are
 * not confused; for simplicity we simply clear it.
 */

public setcurfunc (f)
Symbol f;
{
    extern Boolean lazy;

    curfunc = f;
    curframe = nil;
    if (lazy) {
        touch_file(f);
    }
}

/*
 * Return the frame for the current function.
 * The space for the frame is allocated statically.
 */

public Frame curfuncframe (storage)
Frame storage;
{
    static struct Frame frame;
    Frame frp;

    if (pc == 0) {		/* make sure process still exists... */
	return nil;
    }
    if (curframe == nil) {
	frp = findframe(curfunc);
	if (frp != nil) {
	    curframe = &curframerec;
	    *curframe = *frp;
	}
    } else if (storage != nil) {
	frp = storage;
	*frp = *curframe;
    } else {
	frp = &frame;
	*frp = *curframe;
    }
    return frp;
}

/*
 * Execute up to a source line, skipping over intermediate procedure calls.
 * The tricky part here os recursion; we might single step to the next line,
 * but be within a recursive call.  So we compare frame pointers to make sure
 * that execution is at the same or outer level.
 */

public donext ()
{
	Address oldfrp, newfrp;

	if (isprolog(pc,0)) {
	  dostep(true);
	  pc = reg(SYSREGNO(PROGCTR));
	} else {
	  oldfrp = reg(FRP);
	  do {
	 	dostep(true);
		pc = reg(SYSREGNO(PROGCTR));
		newfrp = reg(FRP);
	  } while (newfrp < oldfrp and newfrp != 0);
	}
}

/*
 * Set curfunc to be N up/down the stack from its current value.
 */

public up (n)
integer n;
{
    Address addr;
    integer i;
    Symbol f, prevfunc;
    Frame prevframe, frp;
    boolean done;
    extern Boolean lazy;

    if (not isactive(program)) {
	error( catgets(scmc_catd, MS_runtime, MSG_265,
						     "program is not active"));
    } else if (curfunc == nil) {
	error( catgets(scmc_catd, MS_runtime, MSG_276,
						       "no current function"));
    } else {
	i = 0;
	f = curfunc;
	frp = curfuncframe(nil);
        prevframe = (Frame) malloc(sizeof(struct Frame));
	done = false;
	do {
	    if (frp == nil) {
		done = true;
		error( catgets(scmc_catd, MS_runtime, MSG_298,
						      "not that many levels"));
	    } else if (i >= n) {
		done = true;
		curfunc = f;
		/* read info of new func automatically after up/down */
		if (lazy)
		   touch_sym(curfunc);
		curframe = &curframerec;
		*curframe = *frp;
		showaggrs = false;
		context_change = true;
		printcallinfo(curfunc, curframe);
		context_change = false;
	    } else if (f == program) {
		done = true;
		error( catgets(scmc_catd, MS_runtime, MSG_298,
						      "not that many levels"));
	    } else {
        	prevframe->arsize = frp->arsize;
        	prevframe->save_pc = frp->save_pc;
                prevframe->save_fp = frp->save_fp;
                prevframe->save_lp = frp->save_lp;
		prevfunc = f;
		frp = nextfunc(frp, &f);
		if (frameeq(prevframe,frp) && (!isinline(prevfunc))) {
		   error( catgets(scmc_catd, MS_runtime, MSG_300,
			       "Frame information in error, %d levels up."),i);
		   i = n;  /* To exit loop next time. */
		}
	    }
	    ++i;
	} while (not done);
        action_mask |= CONTEXT_CHANGE;
    	addr = curframe->save_pc;
    	if (addr != reg(SYSREGNO(PROGCTR)) && addr != minlineaddr) 
	    addr -= MIN_INST_SZ;
	cursource = srcfilename(addr);
    	cursrcline = srcline(addr);
	cursrclang = srclang(cursource);
	free((char *)prevframe);
	if (isXDE) {
	  (*rpt_ctx_level)(n);
	  /*
	   * The DPI debuggers expect cursrcline to be one past the first line
	   * of the function
	   */
	  cursrcline++;
	}
    }
}

public down (n)
integer n;
{
    Address addr;
    integer i, depth;
    Frame frp, curfrp;
    Symbol f;
    struct Frame frame;
    extern Boolean lazy;

    if (not isactive(program)) {
	error( catgets(scmc_catd, MS_runtime, MSG_265,
						     "program is not active"));
    } else if (curfunc == nil) {
	error( catgets(scmc_catd, MS_runtime, MSG_276,
						       "no current function"));
    } else {
	depth = 0;
	frp = &frame;
	if (getcurfunc(frp, &f) < 0) {
	     error( catgets(scmc_catd, MS_runtime, MSG_269,
				      "Could not determine current function"));
	     return;
	}
	if (curframe == nil) {
	    curfrp = findframe(curfunc);
	    curframe = &curframerec;
	    *curframe = *curfrp;
	}
	while ((f != curfunc or !frameeq(frp, curframe)) and f != nil) {
	    frp = nextfunc(frp, &f);
	    ++depth;
	}
	if (f == nil or n > depth) {
	    error( catgets(scmc_catd, MS_runtime, MSG_298,
						      "not that many levels"));
	} else {
	    depth -= n;
	    frp = &frame;
	    if (getcurfunc(frp, &f) < 0) {
	         error( catgets(scmc_catd, MS_runtime, MSG_269,
				      "Could not determine current function"));
	         return;
	    }
	    for (i = 0; i < depth; i++) {
		frp = nextfunc(frp, &f);
		assert(frp != nil);
	    }
	    curfunc = f;
            /* read info of new func automatically after up/down */
	    if (lazy)
	       touch_sym(curfunc);
	    *curframe = *frp;
	    showaggrs = false;
	    context_change = true;
	    printcallinfo(curfunc, curframe);
	    context_change = false;

    	    addr = curframe->save_pc;
    	    if (addr != reg(SYSREGNO(PROGCTR)) && addr != minlineaddr) 
		addr -= MIN_INST_SZ;
	    cursource = srcfilename(addr);
    	    cursrcline = srcline(addr);
	    cursrclang = srclang(cursource);
	    action_mask |= CONTEXT_CHANGE;
	    if (isXDE) {
	      (*rpt_ctx_level)(-n);
	      /*
	       * The DPI debuggers expect cursrcline to be one past the first
	       * line of the function
	       */
	      cursrcline++;
	    }
	}
    }
}

/*
 * Return the address corresponding to the first line in a function.
 */

public Address firstline (f)
Symbol f;
{
    Address addr;

    addr = codeloc(f);
    while (linelookup(addr) == 0 and addr < objsize) {
	++addr;
    }
    if (addr == objsize) {
	addr = -1;
    }
    return addr;
}

/*
 * Catcher drops strike three ...
 */

public runtofirst ()
{
    Address addr, endaddr;

    addr = pc;
    endaddr = objsize + CODESTART;
    while (linelookup(addr) == 0 and addr < endaddr) {
	++addr;
    }
    if (addr < endaddr) {
	stepto(addr);
    }
}

/*
 * Return the address corresponding to the end of the program.
 *
 * We look for the entry to "_exit".
 */

public Address lastaddr ()
{
    Symbol s;

    s = lookup(identname("_exit", true));
    if (s == nil) {
	warning(catgets(scmc_catd, MS_runtime, MSG_307, "cannot find _exit"));
	return CODESTART;
    }
    return codeloc(s);
}

/*
 * Decide if the given function is currently active.
 *
 * We avoid calls to "findframe" during a stack trace for efficiency.
 * Presumably information evaluated while walking the stack is active.
 */

public Boolean isactive (f)
Symbol f;
{
    Boolean b;
    Boolean isCall;

    if (isfinished(process)) {
	b = false;
    } else {
	if (walkingstack or f == program or f == nil
	    or (f->language == cppLang and f->class == TAG)
	    or (ismodule(f) and isactive(container(f)))) {
	    b = true;
	} else {
	    /* We need to turn off call_command before we call findframe */
	    isCall = call_command;
	    call_command = false;
	    b = (Boolean) (findframe(f) != nil);
	    call_command = isCall;
	}
    }
    return b;
}

/*
 * Evaluate a call to a procedure.
 */

#ifdef K_THREADS
tid_t  tid_func_thread;  /* running_thread when  command  call func() is done*/
extern tid_t tid_running_thread;
#endif /* K_THREADS */
extern boolean specificptrtomember;
extern boolean ptrtomemberfunction;
extern boolean notderefed;

public callproc (exprnode, isfunc)
Node exprnode;
boolean isfunc;
{
    Node procnode, arglist;
    Symbol proc = nil;
    integer argc, i;
    static struct Frame frame;
    Address thisptr = nil;
    Boolean isVirtualCall = false;
    Boolean isQualVirtualCall = false;

    funcAddr = nil;
    if (call_command) 
    {
	beginerrmsg();
	(*rpt_error)(stderr, catgets(scmc_catd, MS_runtime, MSG_308,
		             "Nesting of \"call\" commands is illegal."));
	enderrmsg();
    }
    procnode = exprnode->value.arg[0];
    arglist = exprnode->value.arg[1];

    if (procnode->op == O_SYM)
        proc = procnode->value.sym;
    else if (procnode->op == O_CPPREF)
    {
        eval(procnode->value.arg[0]);
	rpush(pop(Address), sizeof(Address));
	rpush(pop(Address), sizeof(Address));
	funcAddr = pop(Address);
	proc = whatblock(funcAddr);
    }
    else if (procnode->nodetype->language == cppLang)
    {
        if (procnode->op == O_DOT) 
	{
	    Symbol classType;
	    AccessList path;

	    if (cpp_isVirtual(procnode->value.arg[1], &classType, &path))
	    {
		isVirtualCall = true;
		procnode->value.arg[0] = buildAccess(path, classType, 
					     procnode->value.arg[0]);
	    }
	    else
	    {
		if (procnode->nodetype->class == MEMBER)
		    error(catgets(scmc_catd, MS_runtime, MSG_636,
			  "Cannot call a pure virtual function."));
		if (procnode->nodetype->symvalue.funcv.u.memFuncSym->
		                        symvalue.member.attrs.func.isVirtual)
		    isQualVirtualCall = true;
		proc = procnode->value.arg[1]->value.sym;
	    }

            eval(procnode->value.arg[0]);
            if ((thisptr = pop(Address)) == nil)
		error(catgets(scmc_catd, MS_runtime, MSG_618,
		      "\"this\" pointer evaluates to nil."));
        }
        else 
	{
	    assert(procnode->op == O_DOTSTAR &&
                   rtype(procnode->nodetype)->class == FFUNC);
            eval(procnode);
            ptrtomemberfunction = false;
            if ((thisptr = pop(Address)) == nil)
		error(catgets(scmc_catd, MS_runtime, MSG_618,
		      "\"this\" pointer evaluates to nil."));
            funcAddr = pop(Address);
            proc = whatblock(funcAddr);

	    /* all calls via pointers to virtual member functions are */
	    /* never considered qualified.                            */
	    isVirtualCall = proc->symvalue.funcv.u.memFuncSym->
				  symvalue.member.attrs.func.isVirtual; 
        }
    }
    if (thisptr && (isVirtualCall || isQualVirtualCall))
    {
        Symbol memFuncSym;
	int vtblEntrySize;
	int memIndex;
	char *vtblPtr;
	Address descAddr;

        if (isVirtualCall)
            memFuncSym = procnode->nodetype;
	else /* qualified virtual call - we must still adjust "this" pointer */
            memFuncSym = procnode->nodetype->symvalue.funcv.u.memFuncSym;

	/* Modify the "this" pointer by the offset given in the virtual */
	/* table (at address given by "thisptr") and, if the function   */
	/* call was not explicitly qualified, get the address of the    */
	/* virtual function being called.                               */

	vtblEntrySize = sizeof(Address) + sizeof(long int);
	memIndex = memFuncSym->symvalue.member.attrs.func.funcIndex;

	rpush(thisptr, sizeof(Address));
	vtblPtr = pop(Address);
	rpush(vtblPtr + memIndex * vtblEntrySize, vtblEntrySize);
	thisptr += pop(long int);
	descAddr = pop(Address);

	if (isVirtualCall)
	{
	    rpush(descAddr, sizeof(Address));
	    funcAddr = pop(Address);
	    proc = whatblock(funcAddr);
        }
    }

    if (proc == nil)
    {
	beginerrmsg();
	(*rpt_error)(stderr, "cannot call \"");
	prtree(rpt_error, stderr, procnode);
	(*rpt_error)(stderr, "\"");
	enderrmsg();
    }
    if (!funcAddr && not isblock(proc)) 
    {
	error(catgets(scmc_catd, MS_runtime, MSG_314,
		      "\"%s\" is not a procedure or function"), 
		      symname(proc));
    }
    if (isfunc and (proc->class == PROC or istypename(proc->type, "void")))
    {
        error(catgets(scmc_catd, MS_runtime, MSG_323,
                      "\"%s\" has no return value. Use call instead of print."),
                      symname(proc));
    }

    /*
     * Make sure we can access this symbol with fast-load
     */
    touch_sym( proc );

    endproc.isfunc = isfunc;
    endproc.callnode = exprnode;
    endproc.cmdnode = topnode;

    /* Save off the current frame before we start the call */
    callframe = &frame;
    getcurframe(callframe);
    pushenv();

    /* Also save off all registers before the call */
    for (i = 0; i < NGREG + NSYS + 1; i++) {
	caller_reg[i] = reg(i);
    }
    for (i = 0; i < fpregs; i++) {
	caller_freg[i] = fpregval(i);
    }
    if (funcAddr) {
       pc = funcAddr;
    }
    else {
       pc = codeloc(proc);
    }

    argc = pushargs(proc, arglist, thisptr);

    call_command = true;
#ifdef K_THREADS
    tid_func_thread = tid_running_thread;
#endif /* K_THREADS */
    beginproc(proc, argc, funcAddr);
    event_once(
	build(O_EQ, build(O_SYM, pcsym), build(O_SYM, retaddrsym)),
	buildcmdlist(build(O_PROCRTN, proc))
    );
    isstopped = false;
    if (not bpact()) {
        notderefed = true;
        specificptrtomember = false;
	isstopped = true;
	cont(0);
    }

    /*
     * bpact() won't return true, it will call printstatus() and go back
     * to command input if a breakpoint is found.
     */
    /* NOTREACHED */
}

/*
 * Check to see if an expression is correct for a given parameter.
 * If the given parameter is false, don't worry about type inconsistencies.
 *
 * Return whether or not it is ok.
 */

static boolean ellipsesfound = false;

private boolean chkparam (actual, formal, chk)
Node actual;
Symbol formal;
boolean chk;
{
    boolean b;
    Symbol actualVar;

    b = true;
    if (chk) {
	if (formal == nil) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_runtime, MSG_315,
						       "too many parameters"));
	    b = false;
	} else {
          Symbol actualnodetype;
          actualnodetype = actual->nodetype;
          if (rtype(formal)->class == ELLIPSES) 
              ellipsesfound = b = true;
          else if (not compatible(formal->type, actualnodetype)) 
	  {
	      beginerrmsg();
	      (*rpt_error)(stderr, catgets(scmc_catd, MS_runtime, MSG_316,
		           "type mismatch for %s"), symname(formal));
	      b = false;
          } else {
            actualVar = actual->nodetype;
	    /* If the param is an address, check what it's pointing at */
	    /* if we have an enumeration constant, then it is active.  */
            if (actualVar == t_addr && actual->op == O_SYM)  
               actualVar = actual->value.sym;  
            if (!(actualVar->class == CONST && actualVar->type->class == SCAL))
               if (actualVar->storage != EXT && 
				not isactive(container(actualVar)) ) {
                 beginerrmsg();
                 (*rpt_error)(stderr,  catgets(scmc_catd, MS_eval, MSG_82,
                        "\"%s\" is not active"), symname(actualVar));
                 b = false;
               }
	  }
	}
    }
    if (b and formal != nil and
	isvarparam(formal) and
	not (
	    /* fortran allows passing builtin constants to var parameters */
            ( strcmp(actual->nodetype->language->name,"$builtin symbols") or
	      (formal->language == fLang) ) or
	    actual->op == O_RVAL or actual->nodetype == t_addr or
	    (
		actual->op == O_TYPERENAME and
		(
		    actual->value.arg[0]->op == O_RVAL or
		    actual->value.arg[0]->nodetype == t_addr
		)
	    )
	)
    ) {
	beginerrmsg();
	(*rpt_error)(stderr, "expected variable, found \"");
	prtree( rpt_error, stderr, actual);
	(*rpt_error)(stderr, "\"");
	b = false;
    }
    return b;
}

/*
 * Evaluate an argument list left-to-right.
 */

public integer evalargs (proc, arglist, fparmregs, fpargcnt, thisptr)
Symbol proc;
Node arglist;
double *fparmregs;
int *fpargcnt;
Address thisptr;
{
    Node p, actual;
    Symbol formal;
    Stack *savesp;
    integer count;
    boolean chk;
    int fparmno = 0;

    savesp = sp;
    count = 0;
    chk = (boolean) (not nosource(proc));

    formal = proc->chain;
    
    if (thisptr != nil) {
        ++count;
        if (formal->name != this) {
	    sp = savesp;
	    popenv();
	    error(catgets(scmc_catd, MS_runtime, MSG_619,
                  "\"this\" object not required!"));
        }
        push(Address, thisptr);
        formal = formal->chain;
    }

    for (p = arglist; p != nil; p = p->value.arg[1]) {
	assert(p->op == O_COMMA);
	actual = p->value.arg[0];
        if (!ellipsesfound) {
	    if (not chkparam(actual, formal, chk)) {
	        (*rpt_error)(stderr," in call to %s", symname(proc));
	        sp = savesp;
	        popenv();
	        enderrmsg();
	    }
            if (ellipsesfound) 
		formal = nil;
        }
	passparam(actual, formal, fparmregs, fpargcnt);
       
	if (formal != nil) {
	    formal = formal->chain;
	}
	++count;
    }

    ellipsesfound = false;
    if (chk && formal != nil && rtype(formal)->class != ELLIPSES)
    {
	sp = savesp;
	popenv();
	    error(catgets(scmc_catd, MS_runtime, MSG_321,
		  "not enough parameters to %s"), symname(proc));
    }
    return count;
}

/*
 * Evaluate an argument list without any type checking.
 * This is only useful for procedures with a varying number of
 * arguments that are compiled -g.
 */

public integer unsafe_evalargs (proc, arglist, fparmregs, fpargcnt, thisptr)
Symbol proc;
Node arglist;
double fparmregs[];
int *fpargcnt;
Address thisptr;
{
    Node p;
    integer count = 0;
    Symbol	formal = proc->chain;

    if (thisptr != nil) {
       ++count;
       push(Address,thisptr);
    }
    for (p = arglist; p != nil; p = p->value.arg[1]) {
	assert(p->op == O_COMMA);
	passparam(p->value.arg[0], formal, fparmregs, fpargcnt);

	if( formal != NULL ) {
	    formal = formal->chain;
	}
	++count;
    }
    return count;
}

#ifdef K_THREADS
/*
 * NAME: procreturn_threadheld
 *
 * FUNCTION: return after  hold the running thread, the context of this 
 *           thread has to be restored.
 *           This thread is executing __funcblock_np().
 *
 * NOTES: Called by routine procreturn()
 *
 * PARAMETERS:
 *      f   - symbol 
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 
 */
private procreturn_threadheld (f)
Symbol f;
{
    extern Boolean just_started;
    integer r;
    integer i = 0;
    Node n;
    char *data;
    Symbol retType;
    Boolean notrunthread;
    union {
        double d;
        Word w[2];
    } u;
    /*in case of hold running_thread, when we return from kernel, we have not */
    /* to restore the context : it's the context of the new running thread    */
    /* threadheld() doesn't call pushenv() */
    popenv_threadheld();
    if (endproc.isfunc) {
        if (f->type->class == CPPREF)
            r = size(retType = f->type->type);
        else
            r = size(retType = f->type);

        pushretval(r, r > sizeofLongLong || f->type->class == CPPREF,
                   rtype(f->type));

        if (r > sizeof(long)) {
            data = newarr(char, r);
            popn(r, data);
            n = build(O_SCON, data, 0);
        } else {
            n = build(O_LCON, (long)popsmall(retType));
        }
       /* flushoutput();*/
        n->nodetype = retType;

/*   This appears to cause some problems
 *   when applied to library calls and
 *   doesn't get us that much so....

        while (endproc.callnode->value.arg[i] != nil)
        {
           dispose(endproc.callnode->value.arg[i]->value.scon);
           dispose(endproc.callnode->value.arg[i]);
           i++;
        }
*/
        *endproc.callnode = *n;
        dispose(n);
        eval(endproc.cmdnode);
    } else {
        /*flushoutput();*/
        (*rpt_output)(stdout, "\n" );
        if (funcAddr) {
           printname(rpt_output, stdout, whatblock(funcAddr), false);
           funcAddr = nil;
        }
      /*  isstopped = true;
        printstatus();
      */
        
    }
/* if the running_thread is not thread running when the call proc() had been */
/* done we have to restore registers of the tid_func_thread                  */
    if ( tid_running_thread != tid_func_thread ){
        notrunthread = true;
    }
    else
        notrunthread = false;

    /* restore general and floating point registers after a call return */
    for (i=0; i < NGREG+NSYS+1; i++) {
        /* if kernel threads and the running thread had changed */
        /* restore the registers                                */
        /* serThreadregs_k restore all registers                */
        if ( notrunthread && (lib_type == KERNEL_THREAD)) {
           setThreadregs_k(tid_func_thread , i, &caller_reg[i]);
           setThreadregs_k(tid_func_thread , NGREG , &caller_reg[NGREG]);
           break;
        } else
        setreg(i, caller_reg[i]);
    }
    for (i=0; i < fpregs; i++) {
        if ( notrunthread && (lib_type == KERNEL_THREAD)) {
           setThreadregs_k (tid_func_thread, -(NGREG+NSYS+i), &caller_freg[i]);
           break;
        } else {
           u.d = caller_freg[i];
           setreg(NGREG+NSYS+i, u.w[0]);
           setreg(-(NGREG+NSYS+i), u.w[1]);
        }
    }
/*  if call func() : we want to keept the current_thread */
    if (notrunthread)
       current_thread = running_thread;
    call_command = false;
    callframe = nil;
    isstopped = true;
    printstatus();
    erecover();

}


/*
 * Pop back to the real world.
 * pop in local variables : not modify the variables
 * just used in case of we have held the running thread
 */

public popenv_threadheld ()
{
    Boolean just_started;
    Boolean isstopped;
    String filename;
    Word frp;
    CallEnv endproc;
    Frame curframe;
    Symbol curfunc;
    Lineno curline;
    Address pc;

    frp = pop(Word);
#ifdef _POWER
    frp = pop(Word);
#endif
    frp = pop(Word);
    endproc = pop(CallEnv);
    curframerec = pop(struct Frame);
    curframe = pop(Frame);
    curfunc = pop(Symbol);
    just_started = (Boolean) pop(int);
    isstopped = (Boolean) pop(int);
    filename = pop(String);
    curline = pop(Lineno);
    pc = pop(Address);
}
#endif /* K_THREADS */

public procreturn (f)
Symbol f;
{
    extern Boolean just_started;
    integer r;
    integer i = 0;
    Node n;
    char *data;
    Symbol retType;
    union {
        double d;
        Word w[2];
    } u;
#ifdef K_THREADS
    Boolean notrunthread;
    extern Address addr_dbsubn;
    if (addr_dbsubn) {
        addr_dbsubn = nil;/* not recall procreturn on bpact (see flushoutput) */
        procreturn_threadheld(f);
    }

#endif /* K_THREADS */

    popenv();
    if (endproc.isfunc) {
	if (f->type->class == CPPREF)
	    r = size(retType = f->type->type);
	else
	    r = size(retType = f->type);

	pushretval(r, r > sizeofLongLong || f->type->class == CPPREF,
                   rtype(f->type));

	if (r > sizeof(long)) {
	    data = newarr(char, r);
	    popn(r, data);
	    n = build(O_SCON, data, 0);
	} else {
	    n = build(O_LCON, (long)popsmall(retType));
	}
	flushoutput();
	n->nodetype = retType;

/*   This appears to cause some problems
 *   when applied to library calls and
 *   doesn't get us that much so....

        while (endproc.callnode->value.arg[i] != nil)
        {
           dispose(endproc.callnode->value.arg[i]->value.scon);
           dispose(endproc.callnode->value.arg[i]);
           i++;
        }
*/
	*endproc.callnode = *n;
	dispose(n);
	eval(endproc.cmdnode);
    } else {
	flushoutput();
	(*rpt_output)(stdout, "\n" );
        if (funcAddr) {
           printname(rpt_output, stdout, whatblock(funcAddr), false);
           funcAddr = nil;
        }
        else {
	   printname( rpt_output, stdout, f, false);
        }
	(*rpt_output)(stdout, catgets(scmc_catd, MS_runtime, MSG_322,
						   " returns successfully\n"));
    }
#ifdef K_THREADS
/* if the running_thread is not thread running when the call proc() had been */
/* done we have to restore registers of the tid_func_thread                  */
    if ( tid_running_thread != tid_func_thread ){
        notrunthread = true;
    }
    else
        notrunthread = false;
#endif /* K_THREADS */

    /* restore general and floating point registers after a call return */
    for (i=0; i < NGREG+NSYS+1; i++) {
#ifdef K_THREADS
        /* if kernel threads and the running thread had changed */
        /* restore the registers                                */
        /* serThreadregs_k restore all registers                */
        if ( notrunthread && (lib_type == KERNEL_THREAD)) {
           setThreadregs_k(tid_func_thread , i, &caller_reg[i]);
           setThreadregs_k(tid_func_thread , NGREG , &caller_reg[NGREG]);
           break;
        } else
#endif /* K_THREADS */
	setreg(i, caller_reg[i]);
    }
    for (i=0; i < fpregs; i++) {
#ifdef K_THREADS
        if ( notrunthread && (lib_type == KERNEL_THREAD)) {
           setThreadregs_k (tid_func_thread, -(NGREG+NSYS+i), &caller_freg[i]);
           break;
        } else
#endif /* K_THREADS */
        {
           u.d = caller_freg[i];
           setreg(NGREG+NSYS+i, u.w[0]);
           setreg(-(NGREG+NSYS+i), u.w[1]);
        }
    }
#ifdef K_THREADS
/*  if call func() : we want to keept the current_thread */
    if (notrunthread)
       current_thread = running_thread;
#endif /* K_THREADS */
    call_command = false;
    callframe = nil;
    erecover();
    
}

/*
 * Push the current environment.
 */

public pushenv ()
{
    extern Boolean just_started;
    push(Address, pc);
    push(Lineno, curline);
    push(String, cursource);
    push(int, (int) isstopped);
    push(int, (int) just_started);
    push(Symbol, curfunc);
    push(Frame, curframe);
    push(struct Frame, curframerec);
    push(CallEnv, endproc);
    push(Word, reg(SYSREGNO(PROGCTR)));
#ifdef _POWER
    push(Word, reg(SYSREGNO(LR)));
#endif
    push(Word, reg(FRP));
}

/*
 * Pop back to the real world.
 */

public popenv ()
{
    extern Boolean just_started;
    String filename;

    setreg(FRP, pop(Word));
#ifdef _POWER
    setreg(SYSREGNO(LR), pop(Word));
#endif
    setreg(SYSREGNO(PROGCTR), pop(Word));
    endproc = pop(CallEnv);
    curframerec = pop(struct Frame);
    curframe = pop(Frame);
    curfunc = pop(Symbol);
    just_started = (Boolean) pop(int);
    isstopped = (Boolean) pop(int);
    filename = pop(String);
    curline = pop(Lineno);
    pc = pop(Address);
    setsource(filename);
}

/*
 * Flush the debuggee's standard output.
 *
 * This is VERY dependent on the use of stdio.
 */

public flushoutput()
{
    Symbol p, iob;

    p = lookup(identname("fflush", true));
    while (p != nil and not isblock(p)) {
	p = p->next_sym;
    }
    /* handle case of many fflush() in different libraries */
    if (p->symvalue.funcv.fcn_desc == nil) {
        Symbol q = p;
        while (q != nil) {
             /* locate the fflush() we have fcn_desc for... */
             if (q->name == p->name && isroutine(q) && 
		 		q->symvalue.funcv.fcn_desc != nil)
                 p = q;
             q = q->next_sym;
        }
    }
    if (p != nil) {
	iob = lookup(identname("_iob", true));
	if (iob != nil) {
	    pushenv();
	    pc = codeloc(p) - FUNCOFFSET;
	    setreg(ARG1, address(iob, nil) + sizeof(*stdout));
	    beginproc(p, 1, nil);
	    stepto(return_addr());
	    popenv();
	}
    }

}

/*
 *   Allocate memory for debuggee.
 */

public debugee_malloc( size )
unsigned size;
{
    Symbol p, iob;

    p = lookup(identname("malloc", true));
    while (p != nil and not isblock(p)) {
	p = p->next_sym;
    }
    /* handle case of many malloc() in different libraries */
    if (p->symvalue.funcv.fcn_desc == nil) {
        Symbol q = p;
        while (q != nil) {
             /* locate the malloc() we have fcn_desc for... */
             if (q->name == p->name && isroutine(q) && 
		 		q->symvalue.funcv.fcn_desc != nil)
                 p = q;
             q = q->next_sym;
        }
    }
    if (p != nil) {
	pushenv();
	pc = codeloc(p) - FUNCOFFSET;
	setreg(ARG1, size);
	beginproc(p, 1, nil);
	stepto(return_addr());
	popenv();
        return(reg(ARG1));
    }
    return(0);    /* some sort of error */
}
