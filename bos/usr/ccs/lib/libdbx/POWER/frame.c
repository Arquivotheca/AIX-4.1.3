static char sccsid[] = "@(#)91    1.30.2.17  src/bos/usr/ccs/lib/libdbx/POWER/frame.c, libdbx, bos41J, 9521A_all 5/22/95 18:25:52";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: ValidText, argcomp, argn, args_base, findtable, getcurframe,
 *	      getnewregs, inSignalHandler, isprolog, locals_base, nextframe,
 *	      passparam, preg, pushargs, pushretval, readtable, savefreg,
 *	      savereg, print_passed_args, before_alloca_frp, islastbr
 *
 * ORIGINS: 26, 27, 83
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
 * Machine-dependent runtime routines.
 */

#include "defs.h"
#include "disassembly.h"
#include "runtime.h"
#include "frame.h"
#include "process.h"
#include "execute.h"
#include "machine.h"
#include "events.h"
#include "mappings.h"
#include "symbols.h"
#include "tree.h"
#include "eval.h"
#include "operators.h"
#include "object.h"
#include "envdefs.h"
#include <sys/param.h>
#include <sys/reg.h>
#include <signal.h>
#include <sys/pseg.h>

#define ValidText(addr) ((addrtoobj(addr)) != -1)
#define inSignalHandler(addr) (0)
#define tb_length (sizeof(struct tbtable_short))

public Frame curframe = nil;
public struct Frame curframerec;
extern Address dbargs_addr;
private Address args_area;

static Address find_old_addr(Address);

/*
 * Trace table structures.
 */
typedef struct tbtable_short TraceBase;
public Boolean traceback_table = false;    

#define LastGPRsaved	15	/* last general register to save */
#define LastFPRsaved	5	/* last FP register to save (6 not preserved)*/
#define MaxGPRsave	64	/* bytes in maximum GP register save area */
#define MaxFPRsave	64	/* bytes in maximum FP register save area */

#define MAXSAVE (LINKSIZE + REGARGSIZE + 10*sizeof(Word) + 4*sizeof(double))

/*
 * Read the traceback table corresponding to a code section.
 */

private readtable (startaddr, frp, oldtable, stripped)
Address startaddr;
register Frame frp;
Boolean oldtable;
Boolean stripped;
{
    Address addr;
    register int i;
    Boolean parms_encoded;
    Boolean name_present;

    traceback_table = true;
    if (oldtable) {
	memcpy(&frp->tb, startaddr, tb_length);
        parms_encoded = (Boolean) (frp->tb.fixedparms || frp->tb.floatparms);
	if (parms_encoded) {
	    memcpy(&frp->parminfo, startaddr+tb_length,4);
	}
	memcpy(&frp->locals_reg, startaddr+tb_length+4, 4);
	if (frp->tb.name_present)
	  memcpy(&frp->name, startaddr+tb_length+8, 4);
    } else {
        iread(&frp->tb, startaddr, tb_length);
        parms_encoded = (Boolean) (frp->tb.fixedparms || frp->tb.floatparms);
	if (parms_encoded) {
	    iread(&frp->parminfo, startaddr+tb_length,4);
	}
	name_present = frp->tb.name_present;
	frp->tb.name_present = 0; /* set only if we actually read name */
	if (frp->tb.uses_alloca ||
	    (name_present && (frp->tb.lang == TB_COBOL || stripped))) {
    	    Address alloca_reg_offset;
	    char alloc_reg;
	    
	    alloca_reg_offset = startaddr+tb_length;
	    if (parms_encoded)	/* Skip parameter info */
		alloca_reg_offset += 4;
	    if (frp->tb.has_tboff) /* Skip offset from routine start */
		alloca_reg_offset += 4;
	    if (frp->tb.int_hndl) /* Skip interrupt handle info */
		alloca_reg_offset += 4;
	    if (frp->tb.has_ctl) { /* Skip controlled storage info */
		int number_of_anchors;

	        iread(&number_of_anchors, alloca_reg_offset,4);
		alloca_reg_offset += (4 + 4*number_of_anchors);
	    }
	    if (name_present) { /* Read or skip name info */
		short name_length;

	        iread(&name_length, alloca_reg_offset, 2);
		alloca_reg_offset += 2;
		if (frp->tb.lang == TB_COBOL || stripped) {
		  frp->name = (char *)malloc(name_length+1);
		  iread(frp->name, alloca_reg_offset, name_length);
		  frp->name[name_length] = '\0';
		  frp->tb.name_present = 1; /* we read name, so set it */
		}
		alloca_reg_offset += name_length;
	    }
	    if (frp->tb.uses_alloca) {
	      iread(&alloc_reg, alloca_reg_offset, 1);
	      frp->locals_reg = (unsigned int) alloc_reg;
	    }
	    else {
	      frp->locals_reg = FRP;
	    }
	} else {
	    frp->locals_reg = FRP;
	}
    }
    traceback_table = false;
}

extern Boolean checktables();

/*
 * NAME: find_old_addr
 *
 * FUNCTION: Given an address, check to see if this is a "heat-shrunk"
 *           address.  If so, return the original address.
 *
 * NOTES: Heat-shrinking is done for performance reasons.  Essentially,
 *        the code of an executable is re-ordered so that frequently
 *        executed instructions are grouped together, and less paging
 *        is necessary.  The original code is replaced by a branch
 *        into the "heat-shrink" area where the new code is located.
 *        If the user is stopped in this area, we need to determine
 *        where he "really" is (what function).  There is a table
 *        containing the address of the beginning of the original
 *        code, paired with the address of the beginning of the "heat-
 *        shrunk" code.  We use this table here to find the original
 *        code address.
 *
 * PARAMETERS: 
 *        input_addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: an address.
 *
 */

#define is_branch_instruction(inst) \
   ((bits((inst), 0, 5) == 18) && (bits((inst), 30, 31) == 0))

/*  the algorithm for getting the address from the
      branch instruction is to concatenate the 24 bit
      LI field (bits 6 through 29) with b'00', sign
      extending the result to 32 bits.  Add this to the
      address of the branch instruction  */

#define branch_target(inst, addr) \
  (((long)((long)((inst) & ~3) << 6) >> 6) + (addr))

static Address find_old_addr(Address input_addr)
{
    int i, low, high, mid;
    struct reloc_table *map;
    extern struct heat_shrink *hs_table;
    extern integer loadcnt;
    extern struct ldinfo *loader_info;
    extern boolean heat_shrunk;
    Word instruction;
    Address addr, orig_addr;

    if (!heat_shrunk)
      return input_addr;

    for (i = 0; i < loadcnt; i++)
    {
      /*  if input_addr is in this loaded object  */
      if ((loader_info[i].textorg <= input_addr)
       && (input_addr <= loader_info[i].textorg + loader_info[i].textsize))
      {
        addr = input_addr - loader_info[i].textorg;

        /*  if there is no heat-shrink table for this loaded object  */
        if ((map = hs_table[i].map) == NULL)
          return input_addr;

        /*  the first element of array map contains the number
              of elements in the array in the first field (old_addr)
              and -1 in the second field (new_addr)  */
              
        low = 1;
        high = hs_table[i].num_elements;

        /*  if this address is not in the heat-shrink area  */
        if (addr < map[low].new_addr)
          return input_addr;

        /*  use a binary search to find the original address  */
        do
        {
          mid = (low + high) / 2;

          if (addr < map[mid].new_addr)
            high = mid - 1;
          else if (map[mid].new_addr == 0)
          {
            /*  may have blank entries at the bottom - handle them  */
            high = mid - 1;
            /*  update table to ignore elements below here in
                  future searches  */
            hs_table[i].num_elements = mid;
          }
          else if (addr > map[mid].new_addr)
            low = mid + 1;
          else
            break;
        } while (low <= high);

        /*  NOTE: we should never enter both of the following loops  */

        /*  if we ended up past the correct entry  */
        while (addr < map[mid].new_addr)
          /*  back up until the correct entry is found  */
          mid--;
   
        /*  there may be duplicate new_addr entries - the last one
              is correct  */
        while (map[mid].new_addr == map[mid+1].new_addr)
          mid++;

        /*  the size of the reordered "chunk" can be:
         *    1.  the same size as the original "chunk" or
         *    2.  1 instruction larger than the original "chunk" or
         *    3.  2 instructions larger than the original "chunk"
         *
         *  all instructions in the original "chunk", except for
         *    the last instruction have been replaced by branches
         *    to the corresponding address.  If the reordered
         *    "chunk" is larger, the extra instruction(s) will be
         *    at the bottom.  We can use this knowledge to get the
         *    original address.  Since the last address of the
         *    original chunk is unchanged, it can be bit tricky.
         */
    
        orig_addr = map[mid].old_addr + input_addr - map[mid].new_addr;
        addr = input_addr;

        /*  i = 0 covers all instructions in the original chunk
                    except the last one.
            i = 1 covers the last "real" instruction.
            i = 2 covers the first extra instruction.
            i = 3 covers the second extra instruction.  */

        for (i = 0; i < 4; i++)
        {
          iread (&instruction, orig_addr, INST_SZ);

          /*  if this is a branch and the target of the branch is
                addr  */
          if (is_branch_instruction(instruction)
           && branch_target(instruction, orig_addr) == addr)
          {
              /*  if this is the first time through the loop,
                    return orig_addr, otherwise return the address
                    of the instruction following orig_addr.  */
              return ((i == 0) ? orig_addr : orig_addr + INST_SZ);
          }
          orig_addr -= INST_SZ;
          addr -= INST_SZ;
        }
        /*  don't think you should be able to get here, but...  */
        return input_addr;
      }
    }
    return input_addr;
}

/*
 * Find the address of the trace table following the procedure
 * at the given address.  Return true if there is one, and read the table
 * information into the given frame.
 */

private boolean findtable (addr, frp)
Address addr;
Frame frp;
{
    register Address a;
    char buf[4];
    int *bufptr;
    int badread;
    unsigned int **tbcontents;
    Address *beginaddr, *endaddr;
    unsigned searchend = 0xffffffff;
    Boolean isold;
    Boolean stripped;


    int looksbad = 0;

    bufptr = (int *) buf;
    stripped = (nlhdr[addrtoobj(addr)].nsyms == 0);
    isold = checktables(addr, &beginaddr, &endaddr, &tbcontents);
    /* Ptrace returns a -1 if address is out of user's space.  However, it
       also returns a -1 if address is in user's space, but = 0xffffffff.  So,
       the heuristic here is 128 straight 0xffffffff in what we expect to be
       code is bad space.  Ought to use errno instead. */
    if (isold) {
        if (addr >= *beginaddr) {  /* Past or at previous search point */
		if (addr <= *endaddr) {  /* Within previous search */
	    		readtable((Address *) tbcontents, frp, true, stripped);
	    		return true;
		} else {	/* Past previous found trace table */
			isold = false;
		}
	} else {	/* Preceding previous search point */
		isold = false;
		searchend = *beginaddr;
	}
    }
    for (a = WORDALIGN(addr); a < searchend; a+=sizeof(Word)) {
#ifdef KDBX
	/* dont check addresses for validity, in case we are in user space */
	/* due to where_thru_sc */
	if ( looksbad > 127) {
	    warning( catgets(scmc_catd, MS_frame, MSG_383,
	     "could not locate trace table from starting address 0x%x"), addr);
	    return false;
	}
        badread = iread(buf, a, sizeof(buf));
#else /* KDBX */
	if ((!ValidText(a)) || (looksbad > 127)) {
	    warning( catgets(scmc_catd, MS_frame, MSG_383,
	     "could not locate trace table from starting address 0x%x"), addr);
	    return false;
	}
	if (badread = iread(buf, a, sizeof(buf))) {
	    warning( catgets(scmc_catd, MS_frame, MSG_383,
	     "could not locate trace table from starting address 0x%x"), addr);
	    return false;
	}
#endif /* KDBX */
	if (*bufptr == 0) {
	    readtable(a+4, frp, false, stripped);
	    if ((!isold) && (beginaddr != (Address *) 0)) {
		*beginaddr = addr;
		*endaddr = a;
		memcpy(tbcontents, &frp->tb, tb_length);
		memcpy(tbcontents+2, &frp->parminfo, 4);
		memcpy(tbcontents+3, &frp->locals_reg, 4);
		memcpy(tbcontents+4, &frp->name, 4);
	    }
	    return true;
	}
	if (*bufptr == -1)
		looksbad++;
	else 
		looksbad = 0;
    }
    /* NOTREACHED unless getting updated search point */
    *beginaddr = addr;
    readtable((Address *) tbcontents, frp, true, stripped);
    return true;
}

/*
 * argcomp: compute arguments of caller given return pc
 */
argcomp (tbp)
Frame tbp;
{
    struct Frame frame;

    if (tbp == nil) {
	if (getcurframe(&frame) < 0)
	    return -1;
	else {
	    tbp = &frame;
	}
    }
    return (tbp->tb.fixedparms + tbp->tb.floatparms);
}

/* Had to change this from ACIS version.  AIX has no way of telling you
 * that you are absolutely in the prolog.
 */

public boolean isprolog (addr, frp) 
Address addr;
Frame frp;
{
	unsigned long estprollen = 0; /* Estimated length of prolog */
	Symbol framefunc;

	framefunc = whatblock(addr);
	if (!framefunc)
	     return true;
	if (prolloc(framefunc) == codeloc(framefunc)) { /* Not compiled -g? */ 
	    if (frp) {
		estprollen = INST_SZ * (
			frp->tb.saves_cr + frp->tb.saves_lr +
			(frp->tb.stores_bc * 2) + /* stack drops too */
			(frp->tb.gpr_saved ? 1 : 0) +
			(frp->tb.fpr_saved ? 1 : 0) + 
			(frp->tb.parmsonstk ? frp->tb.fixedparms +
				frp->tb.floatparms : 0));
	    } else {
		estprollen = 24;    /* Just an educated average prollength */
	    }
	    return (boolean) ((addr - prolloc(framefunc)) <= estprollen);
	} else
	    return (addr < codeloc(framefunc));
}


/*
 * NAME: islastbr
 *
 * FUNCTION: Checks and see if the instruction at specified
 *           address is the last branch instruction in a function.
 *
 * NOTES: Used by function getcurframe and nextframe to determine
 *        if users had stopped at the last branch instruction of a
 *        function. Function getcurframe and nextframe need this
 *        information in determining the current stack pointer (sp).
 *        If dbx is stopped at the last "br" of a function, it will
 *        not need to dereference the content of r1 (sp) since the
 *        stack pointer is already restored to point back at the
 *        previous frame.
 *
 * PARAMETERS:
 *      addr      - address to check
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: true if addr points to last branch instruction of a function,
 *          false if otherwise.
 */
private boolean islastbr (addr)
Address addr;
{
   unsigned int buf, opcode, ext_op;
   boolean badread = false;

   /* check next instruction after addr and see if */
   /* it is start of the traceback table.          */
   badread = iread(&buf, addr + INST_SZ, sizeof(buf));
   if (badread || buf != 0)
      return false;
   else {
      /* then check current instruction and see if */
      /* it is a branch instruction.               */
      badread = iread(&buf, addr, sizeof(buf));
      opcode = bits(buf,0,5);
      ext_op = bits(buf,21,30);
      /* check for unconditional branch (16), conditional */
      /* branch (18), and branch register (19).           */
      return (!badread && (opcode == 16 || opcode == 18 ||
                           (opcode == 19 && (ext_op == 16 || ext_op == 528))));
   }
}


/*
 * Set a frame to the current activation record.
 */

public getcurframe (frp)
register Frame frp;
{
    register Address a;
    register int i;
    Address backchain;
    Address callpc;
    Address orig_a;

    a = reg(SYSREGNO(IAR));
    orig_a = find_old_addr(a);

    if (findtable(orig_a, frp)) {
	frp->prolog = (Boolean) ((isprolog(a, frp)) || (frp->tb.globallink) ||
				 (frp->tb.is_eprol));
	frp->save_pc = a;
        frp->orig_loc = orig_a;
	frp->save_fp = reg(STKP);
	frp->save_lp = reg(frp->locals_reg);
	if (!frp->prolog) {
	   /* A prolog can have only one instruction (4 bytes). For example, */
	   /* "stu r1,-64(r1)" will update the sp and put in the backchain   */
           /* all in one simple instruction.     			     */
           /* Also, no need to deference saved frame point if we are stopped */
           /* on the last branch, since the SP is already restored to point  */
           /* back at the previous frame.                                    */
           if ((!frp->tb.stores_bc) || (a - prolloc(whatblock(a)) < 4) ||
                islastbr(a)) {
	       backchain = frp->save_fp;
           } else {
    	       dread(&backchain,frp->save_fp,4);
           }
	} else {
    	   dread(&backchain,frp->save_fp,4);
	}

	frp->caller_fp = frp->prolog ? frp->save_fp : backchain;
	frp->arsize = frp->caller_fp - frp->save_fp; 
	frp->save_lr = reg(SYSREGNO(LR));
	for (i = 0; i < NSAVEREG; i++) {
	    frp->save_reg[i] = reg(i);
	}
	for (i = 0; i < fpregs; i++) {
	    frp->save_freg[i] = fpregval(i);
	}
	/* Get call address */
	if (frp->tb.globallink || frp->prolog || (!frp->tb.saves_lr)) {
		   callpc = reg(SYSREGNO(LR));
	} else {
		   dread(&callpc,frp->save_fp + 8,4);
	}
    	frp->nparams = argcomp(frp);
	return 0;
    } else {
	return -1;
    }
}

/*
 * Return a pointer to the next activation record up the stack or
 * nil if there is none.  Normally, this routine writes over
 * the given Frame space; therefore it is important to perform
 * the field assignments in the correct order.
 */

public Frame nextframe (frp, use_dbsubn_addr)
register Frame frp;
Boolean use_dbsubn_addr;
{
    extern Address dbsubn_addr;
    register Word *r;
    struct Frame frame;
    Address callpc, off, backchain;
    struct sigcontext *sc;
    int i;
    Address orig_pc;

#ifdef KDBX
	char cmd[40];
	extern int is_lldb_kernel_debugger;
#endif /* KDBX */


    if (frp->caller_fp == 0) /* End of the line. */
	return nil;
    if (use_dbsubn_addr)
        callpc = dbsubn_addr;
    else if ((frp->prolog) || (!frp->tb.saves_lr))
        /* if this isn't the second to the top frame, what is in */
        /* frp->save_lr is not for this frame, so set it to zero */
        if (frp->save_pc != reg(SYSREGNO(IAR)))
           callpc = 0;
        else
	   callpc = frp->save_lr;
    else 
	dread(&callpc, frp->caller_fp + 8, 4);
    if (callpc == 0) /* The End of the line. */
	return nil;
#ifndef KDBX /* for kdbx, in order to get to user space,
                     we need to go to places where callpc is not 'valid' !! */
    if (!ValidText(callpc)) {
	   /* Trying to get traceback thru a signal handler by    */
	   /* locating the sigcontext structure...		  */
	   /* if we have a signal handler, the stack should       */
	   /* look like the following:		    	          */
	   /*							  
           		SP ->| SP+frame |  (backchain)     <-- frame added
           		     |          |		       after prolog
           		     |          |  (saved lr)
           		         ...
           		     |----------|
           	     SP+frame|    0     |  (backchain)
             		     |          |
           		     |   0x27bc |  (saved lr)
                   		 ...
      		     SP+frame|----------|  (sigcontext)
                      +STKMIN|   sig-   |
                             | context  |
                                 ...
                     SP+frame|----------|
                      +STKMIN|          |
                      +sizeof|          |
                 (sigcontext)|          |
                                 ...
                     SP+frame|----------|     <---usp value in kernel 
                       +STKMIN                        func sig_slih()
                       +sizeof(sigcontext)          
                       +STACK_FLOOR 
	   */

           /* if callpc is not valid text and backchain is zero,  */
           /* assume it's a signal handler and get the sigcontext */
           dread(&backchain,frp->save_fp,4);
           /* A new frame is created after the prolog code */
           /* need to dereference again if outside prolog. */
           /* Unless we are at the last branch, in which   */
           /* case the SP is also restored to point at the */
           /* preview frame.                               */
           if (backchain && (!frp->prolog) && !islastbr(frp->save_pc))
              dread(&backchain,backchain,4);
           if (backchain != 0) {
	      /* if the backchain isn't nil, we are not dealing with */
	      /* a signal handler...				     */
              return nil;
           } else {
	      /* A ministack was placed here before the user stack begin */
	      /* so the sigcontext is STKMIN bytes off the top of this   */
	      /* frame, pointed at by r1				 */
              sc = (struct sigcontext *) (frp->caller_fp + STKMIN);

	      /* read the callpc of the sigcontext structure...          */
              dread(&callpc, &sc->sc_jmpbuf.jmp_context.iar, sizeof(callpc));

              /* One final check to make sure the (callpc) sigcontext    */
              if (!ValidText(callpc))
                 return nil;

	      /* everything likes fine, read the other needed info...    */
              dread(&(frp->save_lr), &sc->sc_jmpbuf.jmp_context.lr,
                                                sizeof(frp->save_lr));
              dread(frp->save_reg, &sc->sc_jmpbuf.jmp_context.gpr,
                                                sizeof(frp->save_reg));
              dread(frp->save_freg, &sc->sc_jmpbuf.jmp_context.fpr,
                                                sizeof(frp->save_freg));
              frp->caller_fp = frp->save_reg[1];
              frp->prolog = isprolog(callpc);
           }
    } else if (!frp->prolog) {
#else /* KDBX */
    if (!frp->prolog) {
#endif /* KDBX */
	/* Restore the saved registers from the stack. */
	if ((frp->tb.fpr_saved != 0) && (frp->tb.fpr_saved <= MAXFREG)) {
	   frp->fregoff = 8*frp->tb.fpr_saved;
	   off = frp->caller_fp - frp->fregoff;
	   dread(&frp->save_freg[MAXSAVEFREG-frp->tb.fpr_saved],
						      off,frp->tb.fpr_saved*8);
	} else {
	   frp->fregoff = 0;
	}
	if ((frp->tb.gpr_saved != 0) && (frp->tb.gpr_saved <= NGREG)) {
	   frp->regoff = frp->fregoff + 4*frp->tb.gpr_saved;
	   off = frp->caller_fp - frp->regoff;
	   dread(&frp->save_reg[MAXSAVEREG-frp->tb.gpr_saved],
						      off,frp->tb.gpr_saved*4);
	}
    } else {
	frp->prolog = false;
    }
    
    /* if KDBX, may follow pc to user space, so has to switch before */

#ifdef KDBX 
	if ((callpc >= 0x10000000)){ /* out of kernel text segment */
		if (is_lldb_kernel_debugger)
			sprintf (cmd, "switch %x\n", INT_MAX);
		else
			strcpy (cmd, "switch u\n");
		dbg_write (cmd);
	}
#endif /* KDBX */
		
    orig_pc = find_old_addr(callpc);

    if (!findtable(orig_pc, &frame)) {
#ifdef KDBX 
		if ((callpc >= 0x10000000)){ /* back to kernel text segment */
			if (is_lldb_kernel_debugger)
				sprintf (cmd, "switch %x\n", INT_MAX);
			else
				strcpy (cmd, "switch k\n");
			dbg_write (cmd);
		}
#endif /* KDBX */
	return nil;
    }
	
    memcpy(&frp->tb,&frame.tb,tb_length);
    if (frp->tb.name_present)
      frp->name = frame.name;
    frp->parminfo = frame.parminfo;
    frp->save_fp = frp->caller_fp;
    frp->save_pc = callpc;
    frp->orig_loc = orig_pc;
    frp->save_lp = (frp->tb.uses_alloca) ?
				frp->save_reg[frp->locals_reg] : frp->save_fp;
    frp->nparams = argcomp(frp);
    if (!frp->prolog) {
       if ((callpc - prolloc(whatblock(callpc))) < 4) {
	   backchain = frp->save_fp;
       } else {
           if (dread(&backchain,frp->save_fp,4)){
#ifdef KDBX 
			   if ((callpc >= 0x10000000)){ /* back to kernel text segment */
				   if (is_lldb_kernel_debugger)
					   sprintf (cmd, "switch %x\n", INT_MAX);
				   else
					   strcpy (cmd, "switch k\n");
				   dbg_write (cmd);
			   }
#endif /* KDBX */
			   return nil;
		   }
       }
   }
#ifdef KDBX 
	if ((callpc >= 0x10000000)){ /* back to kernel text segment */
		if (is_lldb_kernel_debugger)
			sprintf (cmd, "switch %x\n", INT_MAX);
		else
			strcpy (cmd, "switch k\n");
		dbg_write (cmd);
	}
#endif  /* KDBX */
    if (backchain == (Address) -1)	/* Error in accessing backchain */
	return nil;
    frp->caller_fp = frp->prolog ? frp->save_fp : backchain;
    frp->arsize = frp->caller_fp - frp->save_fp; 
    return frp;
}

/*
 * If the given parameter is currently in a register,
 * return the register number; otherwise return -1.
 *
 * Different compilers (hc, pcc) use different offsets for the parameters
 * (-16, 0), so we adjust the parameter offset such that the first parameter
 * is at offset 0 (just by subtracting first parameter's offset).
 * The resulting offset, when divided by 4, is a register increment.
 * Since the calling sequence puts parameters starting with register 3,
 * we return 3 plus the computed offset.
 */

public int preg (param, optfrp)
Symbol param;
Frame optfrp;
{
    register Symbol p;
    register int r, off;
    Frame frp;

    r = -1;
    if (param->block == nil) {
	error( catgets(scmc_catd, MS_frame, MSG_403,
		     "[internal error: nil function containing parameter %s]"),
							       symname(param));
    }
    p = param->block->chain;
    if (p == nil) {
	error( catgets(scmc_catd, MS_frame, MSG_404,
		 "[internal error: nil paramlist for function containing %s]"),
	    						       symname(param));
    }
    off = 3;
    for (; off <= 10; off++) {
	if (p == param) {
	    if (optfrp == nil) {
		frp = findframe(p->block);
	    } else {
		frp = optfrp;
	    }
	    if (frp == nil) break;
	    if (frp->prolog) {
		r = off;
	    }
	    break;
	} else if (p == nil) {	/* Somehow missed the parameter on chain */
		r = -1;
		break;
	}
	p = p->chain;
    }
    return r;
}

/*
 * Return the base save address for arguments in the caller's frame.
 */

public Address saved_args_base (frp)
register Frame frp;
{
    return frp->caller_fp + 24;	/* backchain, cr, lr, reserve(2), toc */
}

/*
 * Return the base address for arguments in the given frame.
 */

public Address args_base (frp)
register Frame frp;
{
    return frp->save_fp;
}

/*
 * Return the base address for locals in the given frame.
 */

public Address locals_base (optfrp)
Frame optfrp;
{
    struct Frame frame;
    Frame frp;

    if (optfrp == nil) {
	if (getcurframe(&frame) < 0)
	    return (Address) -1;
	else {
	    frp = &frame;
	}
    } else {
	frp = optfrp;
    }
    return frp->save_lp;
}

/*
 * NAME: before_alloca_frp
 *
 * FUNCTION: return the stack (frame) pointer value before it was
 *	     changed by alloca().
 *
 * PARAMETERS: None.  
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: zero if function does not use alloca().
 *          Otherwise the stack pointer value before it was changed
 *	    by alloca().
 */
public Address before_alloca_frp()
{
   struct Frame frame;
	
   if ((getcurframe(&frame) != -1) && frame.tb.uses_alloca)
	return frame.save_lp;
   else 
	return (Address) 0;
}

/*
 * Return saved register n from the given frame.
 */

public Word savereg (n, frp)
integer n;
Frame frp;
{
    Word w;

    if (frp == nil) {
	w = reg(n);
    } else {
	switch (n) {
	    case STKP:
		w = reg(STKP);
		break;

	    case IAR:
		w = frp->save_pc;
		break;

	    default:
		assert(n >= 0 and n < NSAVEREG);
		w = frp->save_reg[n];
		break;
	}
    }
    return w;
}

/*
 * Return saved floating point register n from the given frame.
 */

public double savefreg (n, frp)
integer n;
Frame frp;
{
    double d;

    if (frp == nil) {
	d = fpregval(n);
    } else {
	d = frp->save_freg[n];
    }
    return d;
}

/*
 * Set the registers according to the given frame pointer.
 */

public getnewregs (addr)
Address addr;
{
    struct Frame frame;

    dread(&frame, addr, sizeof(frame));
    setreg(FRP, frame.save_fp);
    setreg(SYSREGNO(IAR), frame.save_pc);
    pc = frame.save_pc;
    setcurfunc(whatblock(pc));
}

/*
 * Return the nth argument of the given frame.
 * If the frame is nil, use the current one.
 */

public Word argn (n, optfrp)
int n;
Frame optfrp;
{
    Word w = 0;
    register Frame frp;
    struct Frame frame;

    if (optfrp == nil) {
	if (getcurframe(&frame) < 0)
	    return -1;
	else {
	    frp = &frame;
	}
    } else {
	frp = optfrp;
    }
    if (n == 0) {
	w = frp->nparams;
	if (frp->prolog && (w > 8))
	    w = 8;    /* First eight words of parms are passed in registers */
#ifdef NOTRACETABLES
    } else if (n >= 1 && n <= 8 && (frp->prolog)) {
#else
    } else if (n >= 1 && n <= 8 && (frp->prolog || (!frp->tb.stores_bc))) {
#endif
			w = reg(n+2);
    } else if (n >= 1 && n <= 8) {
#ifndef NOTRACETABLES
			w = reg(MAXSAVEREG-n+1);
#else
			w = reg(n+2);
#endif
    } else if (!frp->prolog) {
	dread(&w, frp->caller_fp + ((n-1) * sizeof(Word)) - 16, sizeof(w));
    }
    return w;
}

/*
 * Print arguments from the given frame.
 * If the frame is nil, use the current one.
 */

public print_passed_args (optfrp, begin_parmno, end_parmno)
Frame optfrp;
unsigned int begin_parmno;
unsigned int end_parmno;
{
    register Frame frp;
    struct Frame frame;
    int fixed_regno = 3;	/* Fixed regs stored in $r3-$r10 */
    int float_regno = 1;	/* Float regs stored in $fr1-$fr13 */
    int fixed_remaining;
    int float_remaining;
    unsigned bitcount = 0;
    unsigned int parmindex;
    unsigned int parm_encode, reverse_encode;
    parm_type passed_as;
    unsigned int stack_offset = 0;
    Word fixed_parm;
    double double_parm;
    float float_parm;
    extern parm_type get_parm_type();

    if (optfrp == nil) {
	if (getcurframe(&frame) < 0)
	    return -1;
	else {
	    frp = &frame;
	}
    } else {
	frp = optfrp;
    }
    if (begin_parmno > frp->nparams)	/* Can't show what isn't there */
	return;
    if (end_parmno > frp->nparams)	/* Still cannot show what isn't there */
	end_parmno = frp->nparams;

    parm_encode = frp->parminfo;
    reverse_encode = 0;
    fixed_remaining = frp->tb.fixedparms;
    float_remaining = frp->tb.floatparms;

    if (parm_encode) { /* Reverse bit ordering for ease of use later. */
	for (bitcount = 0;bitcount < 32;bitcount++) {
		reverse_encode <<= 1;	/* Shift up */
		reverse_encode |= (parm_encode & 1);
		parm_encode >>= 1;
	}
    }

    for (parmindex = 1; parmindex < begin_parmno; parmindex++) {
	 passed_as = get_parm_type(&reverse_encode);
	 switch(passed_as) {
	     case FIXED_PARM:
		 fixed_regno++;
		 stack_offset+=4;
		 fixed_remaining--;
		 break;
		
	     case FLOAT_PARM:
		 fixed_regno++;
		 float_regno++;
		 stack_offset+=4;
		 float_remaining--;
		 break;

	     case DOUBLE_PARM:
		 fixed_regno += 2;
		 float_regno++;
		 stack_offset+=8;
		 float_remaining--;
		 break;
	 }
    }
    for (; parmindex <= end_parmno; parmindex++) {
	 passed_as = get_parm_type(&reverse_encode);
	 switch(passed_as) {
	     case FIXED_PARM:
                 if ((frp->prolog) && (fixed_regno <= 10))
                 {
                   /*  print the value in the register used to 
                         pass the parameter  */
                   frp->save_reg[fixed_regno];
                   (*rpt_output)(stdout, "0x%x",
                                 frp->save_reg[fixed_regno]);
                 }
                 else
                 {
                   /*  if the parameter is stored on the stack  */
                   if (frp->tb.parmsonstk)
                   {
                     dread(&fixed_parm, saved_args_base(frp) +
                           stack_offset, 4);
                     (*rpt_output)(stdout, "0x%x", fixed_parm);
                   }
                   else
                     (*rpt_output)(stdout, "??");
                 }
		 fixed_regno++;
		 stack_offset+=4;
		 fixed_remaining--;
		 break;
		
	     case FLOAT_PARM:
                 if ((frp->prolog) && (float_regno <= 13))
                 {
                   /*  print the value in the register used to 
                         pass the parameter  */
                   float_parm = (float) frp->save_freg[float_regno];
                   (*rpt_output)(stdout, "%.9g", float_parm);
                 }
                 else
                 {
                   /*  if the parameter is stored on the stack  */
                   if (frp->tb.parmsonstk)
                   {
                     dread(&float_parm, saved_args_base(frp) +
                           stack_offset, 4);
                     (*rpt_output)(stdout, "%.9g", float_parm);
                   }
                   else
                     (*rpt_output)(stdout, "??");
                 }
		 fixed_regno++;
		 float_regno++;
		 stack_offset+=4;
		 float_remaining--;
		 break;

	     case DOUBLE_PARM:
                 if ((frp->prolog) && (float_regno <= 13))
                 {
                   /*  print the value in the register used to 
                         pass the parameter  */
                   double_parm = frp->save_freg[float_regno];
                   (*rpt_output)(stdout, "%.17g", double_parm);
                 }
                 else
                 {
                   /*  if the parameter is stored on the stack  */
                   if (frp->tb.parmsonstk)
                   {
                     dread(&double_parm, saved_args_base(frp) +
                           stack_offset, 8);
                     (*rpt_output)(stdout, "%.17g", double_parm);
                   }
                   else
                     (*rpt_output)(stdout, "??");
                 }
		 fixed_regno += 2;
		 float_regno++;
		 stack_offset+=8;
		 float_remaining--;
		 break;
	 }
	 if (fixed_remaining || float_remaining) {
	     (*rpt_output)(stdout, ", ");
	 }
    }
}

parm_type get_parm_type (parm_encode)
unsigned int *parm_encode;
{
    unsigned int parm_indicator;

    parm_indicator = (*parm_encode) & 1;
    (*parm_encode) >>= 1;
    if (parm_indicator) {		/* Float or double parameter */
        parm_indicator = (*parm_encode) & 1;
	(*parm_encode) >>= 1;
	return ((parm_indicator) ? DOUBLE_PARM : FLOAT_PARM);
     } else {			/* Fixed parameter */
	return FIXED_PARM;
     }
}

/*
 * Push the arguments on the process' stack.  We do this by first
 * evaluating them on the "eval" stack, then copying into the process'
 * space.
 */

public integer pushargs (proc, arglist, thisptr)
Symbol proc;
Node arglist;
Address thisptr;
{
    Stack *savesp;
    Word paramreg[8];
    double fparamreg[13];
    int fpargcnt = 0;
    int n, r, argc, args_size;

    savesp = sp;
    args_area = dbargs_addr;
    if (varIsSet("$unsafecall")) {
	argc = unsafe_evalargs(proc, arglist, fparamreg, &fpargcnt, thisptr);
    } else {
	argc = evalargs(proc, arglist, fparamreg, &fpargcnt, thisptr);
    }
    args_size = sp - savesp;
    if (args_size > 0) {
	if (args_size > sizeof(paramreg)) {
	    n = sizeof(paramreg) / sizeof(Word);
	    mov(savesp, paramreg, sizeof(paramreg));
	    dwrite(savesp, reg(STKP)+24, args_size);
/*
	    args_size -= sizeof(paramreg);
	    setreg(STKP, reg(STKP) - args_size);
	    dwrite(savesp + sizeof(paramreg), reg(STKP), args_size);
*/
	} else {
            n = args_size / sizeof(Word);
	    mov(savesp, paramreg, args_size);
	}
	for (r = 1; (r <= fpargcnt) && (r <= 13); r++) {
	    process->freg[r] = fparamreg[r-1];
	    writeflreg(process, FREGTOSYS(r), process->freg[r]);
	}
	for (r = 0; r < n; r++) {
	    setreg(r+ARG1, paramreg[r]);
	}
    }
    sp = savesp;
    return argc;
}

/*
 * Pass an expression to a particular parameter.
 *
 * Normally we pass either the address or value, but in some cases
 * (such as C strings) we want to copy the value onto the stack and
 * pass its address.
 *
 * Another special case raised by strings is the possibility that
 * the actual parameter will be larger than the formal, even with
 * appropriate type-checking.  This occurs because we assume during
 * evaluation that strings are null-terminated, whereas some languages,
 * notably Pascal, do not work under that assumption.
 */

public passparam (actual, formal, fpr, fprindex)
Node actual;
Symbol formal;
double fpr[];
int *fprindex;
{
    Address addr;
    Stack *savesp;
    integer actsize, formsize;
    Symbol actnode;
    Symbol formal_type = rtype(formal);

    actnode = rtype(actual->nodetype);
    if (formal and isvarparam(formal) 
        and (strcmp(actual->nodetype->language->name,"$builtin symbols")))
    {
	addr = lval(actual->value.arg[0]);
	push(Address, addr);
    } 
    else if (passaddr(formal, actual->nodetype)) 
    {
       if (actual->op == O_RVAL)
       {
	  addr = lval(actual->value.arg[0]);
	  push(Address, addr);
       }
       else
       {
	  savesp = sp;
	  eval(actual);
	  actsize = sp - savesp;
	  /* Put in check for writing past end of args area */
	  dwrite(savesp, args_area, actsize);
	  sp = savesp;
	  push(Address, args_area);
	  args_area += actsize;
       }
    }
    else if (actnode->class == CLASS && 
              !(actnode->symvalue.class.passedByValue)){
        savesp = sp;
        eval(actual);
        actsize = sp - savesp;
        /* Put in check for writing past end of args area */
        dwrite(savesp, args_area, actsize);
        sp = savesp;
        push(Address, args_area);
        args_area += actsize;
    }
    else if ((formal) && ((int)(actual->value.sym) <= (sbrk(0))) &&
             (strcmp(actual->nodetype->language->name,"$builtin symbols")) &&
             ((actual->value.sym->class == FUNC) ||
              (actual->value.sym->class == PROC) ||
              (actual->value.sym->class == CSECTFUNC)) )
    {
        Name n;
        Symbol sym;
        char funcname[30];
        sym = actual->value.sym;
        sprintf(funcname,"_%s",symname(sym));
        n = identname(funcname,true);
        if (lookup(n) == nil) {
          /* In case of doing a dbx call with function as parameter, */
          /* Not only do we need to pass the entry address of the    */
          /* function (proladdr), we also have to take care of the   */
          /* toc. As a result, here we try to construct a function   */
          /* descriptor (entry address following by toc) in our args */
          /* area, and pass that address (kind of like passing a     */
          /* function desciptor). In case of local functions and     */
          /* referenced out-of-module function (have local glue      */
          /* code), the local toc is being used. For unreferenced    */
          /* out-of-module function, the out-of-module entry point   */
          /* is used with local toc (doesn't really matter since     */
          /* it's going to fail because the real toc couldn't be     */
          /* found, which is still a problem.                        */
          Address toc;
          Address o_pc = reg(SYSREGNO(IAR));
          /* prolloc means 0x1... for local func and 0xd.. for lib func */
          Address a = prolloc(sym);
          int local_module;
          Desclist *dptr;
          /* Try to find local glue code entry address for out-of- */
          /* module finction.                                      */
          if (textobj(a) != (local_module = textobj(o_pc))) {
               for (dptr = sym->symvalue.funcv.fcn_desc;
                     (dptr != nil) && (textobj(dptr->descaddr) != local_module);
                     dptr = dptr->next_desc);
               if (dptr != nil)                 /* If glue code found, use */
                  a = dptr->descaddr;           /* glue code address.      */
          }
          dwrite((char *) &a, args_area, sizeof(Address));
          toc = reg(2);                         /* find toc value */
          dwrite((char *) &toc, args_area + sizeof(Address), sizeof(Address));
          push(Address, args_area);
          args_area += 2*sizeof(Address);
        } else {
          sym = lookup(n);
          push(Address, sym->symvalue.offset);
        }
    } 
    else if (formal) 
    {
	formsize = size(formal);
	savesp = sp;
	eval(actual);
	actsize = sp - savesp;

        if (formsize < sizeof(Word))
        {
          /*  parameters less than the sizeof a word need
                to be right justified in a word.  Therefore,
                we need to convert to an int  */
          convert (savesp, actnode, savesp, rtype(t_int));
          formsize = sizeof(Word);
        }
        else
          convert (savesp, actnode, savesp, formal_type);
        
        /*  adjust the stack pointer appropriately  */
        /*  NOTE : if convert "failed", we are simply left-
                     justifying whatever is on the stack.
                     This was the "old" behavior for all
                     types of parameters.  Now we perform
                     conversion on "base" types.  */
        sp += (formsize - actsize);

	/* Do float-double stuff here. */
        if (formal_type->class == REAL)
        {
          if (*fprindex < 13)
          {
            switch (formal_type->symvalue.size)
            {
              case sizeof(float) :
                fpr[(*fprindex)++] = (double) *(float *) savesp;
                break;
              case sizeof(double) :
                fpr[(*fprindex)++] = *(double *) savesp;
                break;
              case 2 * sizeof(double) :
                if (*fprindex < 12)
                {
                  fpr[(*fprindex)++] = *(double *) savesp;
                  fpr[(*fprindex)++] = *(((double *) savesp) + 1);
                }
                break;
            }
          }
        }
    } 
    else {
        savesp = sp;
        eval(actual);
        actsize = sp - savesp;
        if (actsize < sizeof(Word)) {
          int param_val = 0;
          char *vptr;
          vptr = (char *) &param_val;
          vptr += sizeof(Word) - actsize;
          popn(actsize, vptr);
          push(int, param_val);
        }
   }
}

/*
 * Push the value associated with the current function.
 */

public pushretval (len, isindirect, rsym)
integer len;
boolean isindirect;
Symbol rsym;
{
    Word retval;

    retval = reg(ARG1);
    if (rsym->class == REAL)
    {
	if (len == sizeof(float))
	{
	    push(float, (float)fpregval(1));
	}
	else if (len == sizeof(double))
	{
	    push(double, (double)fpregval(1));
	}
	else
	{
	    push(double, (double)fpregval(1));
	    push(double, (double)fpregval(2));
	}
    }
    else if (isindirect) {
	rpush((Address) retval, len);
    } else {
	switch (len) {
	    case sizeof(char):
		push(char, retval);
		break;

	    case sizeof(short):
		push(short, retval);
		break;

	    default:
		if (len == sizeof(Word)) {
		    push(Word, retval);
		} else if (len == 2*sizeof(Word)) {
		    push(Word, retval);
		    push(Word, reg(4));
		} else {
		    error( catgets(scmc_catd, MS_frame, MSG_406,
			 "[internal error: bad size %d in pushretval]") , len);
		}
		break;
	}
    }
}
