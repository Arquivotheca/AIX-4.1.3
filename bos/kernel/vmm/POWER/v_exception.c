static char sccsid[] = "@(#)28	1.31  src/bos/kernel/vmm/POWER/v_exception.c, sysvmm, bos41J, 9513A_all 3/27/95 15:27:34";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_exception, v_issystem, v_isuser
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include "sys/user.h"
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/proc.h>
#include <sys/syspest.h>
#include <sys/except.h>
#include <sys/dbg_codes.h>
#include <sys/errids.h>
#include <sys/trchkid.h>
#include <sys/machine.h>
#include <sys/systm.h>
#include <sys/inline.h>

#define	HALT_DSI	0x30000000
#define	HALT_ISI	0x40000000

/* vmm error log structure
 */
struct vmmerror
{
	struct	err_rec0	vmmerr;
	uint			detail_data[4];
} vmmerrlog = { 0,"SYSVMM ", 0, 0, 0, 0 };


/*
 * v_exception(exval,vaddr, sregval, mst)
 *
 * process a synchronous exception detected by v_getsubs.
 *
 * INPUT PARAMETERS
 *
 * (1) exval - exception value
 *
 * (2) vaddr - the 32-bit virtual address of the page fault.
 *
 * (3) sregval - the contents of the segment register associated
 *               with vaddr.
 *
 * (4) mst - pointer to the mstsave in effect when the page fault
 *               occurred.
 *
 */

int
v_exception(exval,vaddr,sregval,mst)
int	exval;		/* exception value */
uint    vaddr;          /* 32 - bit virtual address of page fault */
uint    sregval;        /* contents of corresponding sreg */
struct  mstsave *mst;   /* pointer to mstsave when pfault happened */
{
	int usermode, sysaddr;
	uint sid, sidx, pno, seginfo;
	struct mstsave *exmst;

	/* get mode. use previous mst for determining
	 * mode if exception occured while backtracking.
	 */
	exmst = (mst->backt) ? mst->prev : mst;
	usermode = v_isuser(exmst);

	/* check if sid is valid.  if so, get sidx, segment info
	 * and determine if system address. if invalid sid,
	 * exception is EFAULT or VM_NOPFAULT.
	 */
	sid = SRTOSID(sregval);
	if (sid != INVLSID)
	{
		sidx = STOI(sid);
		seginfo = scb_sibits(sidx);
		pno = (vaddr & SOFFSET) >> L2PSIZE;
		sysaddr = v_issystem(sidx,pno);
	}
	else
	{
		seginfo = 0;
	}
		
	/*  trace the exception.
	 */
	TRCHKL5T(HKWD_VMM_EXCEPT,seginfo,sregval,vaddr,exval,curthread->t_tid);

	/* evalulate the exception. 
	 */
	switch(exval)
	{

	/* call p_slih() for trap instruction.
	 */
	case EXCEPT_TRAP:
#if defined(_KDB)
		if (__kdb()) {
			kdb_d_slih(mst, vaddr, EXCEPT_DSI);
			return(0);	
			break;
		}
#endif /* _KDB */
		p_slih(mst,vaddr,exval,curthread);
		return(0);	
		break;

	/* log and halt if page fault in interrupt handler
	 * or at interrupt priority other than INTBASE,
	 * INTPAGER or INTMAX (process level) and we don't
	 * have the 'excbranch' type of exception handler
	 * established (this type of handler is used by xmemin()
	 * and xmemout() which may be called by an interrupt
	 * handler and may fault in which case they will return
	 * with a failure code).  Note that other kernel routines
	 * use this type of exception handler so we will not halt for
	 * these either.  When the correct exception handler is
	 * established, we need to pass it a valid errno value so
	 * we use EFAULT.
	 */
	case VM_NOPFAULT:
		if (mst->excbranch == 0)
		{
			v_loghalt(exval,mst);
		}
		else
			exval = EFAULT;
		break;

	/* log and halt if kernel mode and fp reference 
	 * to i/o segment, segment boundry, or pft loop
	 * exception.
	 */
	case EXCEPT_FP_IO:
	case EXCEPT_MIXED_SEGS:
	case EXCEPT_PFTLOOP:

		if (!usermode)
		{
			/* log and halt.
			 */
			v_loghalt(exval,mst);
			return(0);
		}
		break;

	/* log and halt for protection exception if kernel 
	 * mode, exception on system address and priviliged
	 * access key set in sregval.
	 */
	case EXCEPT_PROT:

		if (!usermode && sysaddr && !SRTOKEY(sregval))
		{
			/* log and halt.
			 */
			v_loghalt(exval,mst);
			return(0);
		}
		break;

	/* log and halt if kernel mode and exception on system
	 * address for ifetch from i/o, ifetch from special
         * segment, ENOMEM or ENOSPC exceptions.
	 */
	case EXCEPT_IFETCH_IO:
	case EXCEPT_IFETCH_SPEC:
	case ENOMEM:
	case ENOSPC:

		if (!usermode && sysaddr)
		{
			/* log and halt.
			 */
			v_loghalt(exval,mst);
			return(0);
		}
		break;

	/* call v_vmexcept() for all other exceptions.
	 */
	case EFAULT:
	case EFBIG:
	default:
		break;
	}


	/* call v_vmexcept() to process the exception.
	 */
	v_vmexcept(exval,mst,curthread);

	return(0);
}

/*
 * v_vmexcept(exval,mst,procp)
 *
 * process synchronous (v_getsubs) and asynchronous (v_pfend)
 * exceptions.
 *
 * INPUT PARAMETERS
 *
 * (1) exval - exception value
 *
 * (2) mst - pointer to the mstsave in effect when the page
 *	     fault occured.
 *
 * (3) procp - process table pointer for process that received
 *		the exception.
 *
 */

v_vmexcept(exval,mst,threadp)
int exval;
struct mstsave * mst;
struct thread * threadp;
{
	int pftype, usermode, sigexcept;
	uint vaddr;
	struct mstsave *exmst;

 	/* if synchronous exception within a backtracking
	 * page fault use previous mst.
	 */
	exmst = (mst->backt) ? mst->prev : mst;

	/* get page fault type, vaddr and sidx on which the
	 * exception occured, and mode.
	 */
	pftype = exmst->except[EPFTYPE];
	vaddr = exmst->except[EVADDR];
	usermode = v_isuser(exmst);

	/* save the vaddr for handling by vmexception().  this
	 * is necessary in case another page fault overwrites
	 * exmst->except[EORGVADDR].
	 */
	exmst->o_vaddr = exmst->except[EORGVADDR];

	/* check if exception occured on a instruction storage page
	 * fault. if so, call p_slih() if the fault occured while in 
	 * user mode; otherwise log the error and halt (kernel mode).
	 */
	if (pftype == EXCEPT_ISI)
	{

		if (usermode)
		{
			p_slih(exmst,vaddr,exval,threadp);
		}
		else
		{
			v_loghalt(exval,mst);
		}
	}
	/* exception occured on a data storage page fault. log
	 * and halt if kernel mode and no exception handler is
	 * defined; otherwise call p_slih().
	 */
	else
	{
		ASSERT(pftype == EXCEPT_DSI);

		sigexcept = ((mst->backt) && (threadp->t_atomic & TSIGDELIVERY));

		if (!usermode && exmst->kjmpbuf == NULL &&
		    exmst->excbranch == 0 && sigexcept == 0)
		{
			v_loghalt(exval,mst);
		}
		else
		{
			p_slih(exmst,vaddr,exval,threadp);
		}
	}
		
	return(0);

}

/*
 * v_loghalt(exval,mst)
 *
 * save exception information and halt.
 *
 * INPUT PARAMETERS
 *
 * (1) exval - exception value.
 *
 * (2) mst - pointer to the mstsave in effect when the page
 *	     fault occured.
 *
 */

v_loghalt(exval,mst)
int exval;
struct mstsave * mst;
{
		int dbgcode, haltcode;
		struct mstsave *exmst;

		/* exception info resides in previous
		 * mst for backtracking exceptions.
		 */
		exmst = (mst->backt) ? mst->prev : mst;

		/* determine halt code, debug code and error id.
		 */
		if (exmst->except[EPFTYPE] == EXCEPT_DSI)
		{
			haltcode = HALT_DSI;
			dbgcode = DBG_DSI_PROC;
			vmmerrlog.vmmerr.error_id = ERRID_DSI_PROC;
		}
		else
		{
			haltcode = HALT_ISI;
			dbgcode = DBG_ISI_PROC;
			vmmerrlog.vmmerr.error_id = ERRID_ISI_PROC;
		}

		/* fill out the error record.
		 */
		vmmerrlog.detail_data[0] = exmst->except[EISR];
		vmmerrlog.detail_data[1] = exmst->except[ESRVAL];
		vmmerrlog.detail_data[2] = exmst->except[EVADDR];
		vmmerrlog.detail_data[3] = exval;
			
		/* log the exception.
		 */
		errsave(&vmmerrlog,sizeof(vmmerrlog));

		/* halt.
		 */
		halt_display(mst,haltcode,dbgcode);


		return(0);
}


/* 
 * v_issystem(sidx,pno)
 *
 * returns 1 if (sidx,pno) refers to a kernel address and
 * 0 if not. a kernel address is one in any of the following
 * segments : kernel, kernel extension, VMM data , VMM page table,
 * or VMM paging space disk map, or an address in the privileged
 * part of the process-private segment.
 */
v_issystem(sidx,pno)
uint sidx;
int pno;
{

	/* is it in process-private area
	 */
	if (scb_privseg(sidx))
	{
		return ( (pno > scb_sysbr(sidx)) ? 1 : 0);
	}

	/* is it in one of the system segments?
	 */
	if (scb_system(sidx) && scb_wseg(sidx))
		return 1;

	/* not a system address.
	 */
	return 0;
}

/*
 * v_isuser(mst)
 *
 * returns true if mstsave is in problem-state and 0 if in supervisor
 * state.
 */
v_isuser(mst)
struct  mstsave *mst;   /* pointer to mstsave when pfault  happened */
{
	return(mst->msr & MSR_PR);
}

/* 
 * chk_issystem(srval,vaddr)
 *
 * Interface to the program check handler to determine if the
 * faulting address is a system address or not.
 */
chk_issystem(srval,vaddr)
ulong srval;
ulong vaddr;
{
	ulong sidx, pno;
	ulong savevmm;
	int rc;

        /* 
	 * load sregs to address vmmdseg and segment to change
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	sidx = STOI(SRTOSID(srval));
	pno = (vaddr & SOFFSET) >> L2PSIZE;
	rc =  v_issystem(sidx,pno);

        (void)chgsr(VMMSR,savevmm);
	return(rc);
}

