static char sccsid[] = "@(#)34	1.9.1.31  src/bos/kernel/vmm/POWER/vmmisc.c, sysvmm, bos412, 9445C412a 10/25/94 11:41:21";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vm_handle, vm_vmid, p2vmminit, ukerncopy, vm_init,
 *		vm_config, lra, lqra, vm_allocsr, as_geth, as_puth,
 *		as_att, as_det, as_seth, base_vmm_lock, base_vmm_unlock,
 *		v_vmm_lock, v_vmm_unlock, v_vmm_slock, v_vmm_locktry,
 *		v_vmm_lockmine
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/user.h>
#include <sys/vmker.h>
#include <sys/var.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/atomic_op.h>
#include <sys/trchkid.h>
#include <sys/inline.h>
#include "vmsys.h"
#include "vmvars.h"
#include "vm_mmap.h"
#include "vm_map.h"
#include "mplock.h"	/* for define _VMM_MP_SAFE */

/*
 * nonpageable variables are declared and initialized here
 * so they do not wind up in common.
 */
struct vminfo vmminfo = { 0 };
struct cfgncb vmcfg = { 0 };
struct vmkerdata vmker = { 1 };
struct vmranges vmrmap = { 0 };
struct vmintervals vmint[VMINT_TYPES] = { { 0 },{ 0 },{ 0 },{ 0 } };
struct vmvars vmvars = { /* -1 values will be calculated at boot time */
	-1,			/* minfree */
	-1,			/* maxfree */
	-1,			/* minperm */
	-1,			/* maxperm */
	-1,			/* pfrsvdblks */
	-1,			/* npswarn */
	-1,			/* npskill */
	-1,			/* minpgahead */
	-1,			/* maxpgahead */
	-1,			/* maxpdtblks */
	-1,			/* numsched */
	-1,			/* htabscale */
	 0,                     /* aptscale */
	-1,			/* pd_npages */
};

/*
 * Per-processor V=R stack frame for machine-dependent routines.
 * Callers switch to this stack frame before invoking any of the
 * machine-dependent routines.  The stack must be large enough to
 * contain a 32-word save area plus the maximum stack usage of any
 * machine-dependent function call.
 */
char pmap_stack[PMAP_STK_SIZE * MAXCPU] = { 0 };

/*
 * When the variable noprotect is non-zero, memory is not fetch-protected.
 * This allows someone to patch the kernel to allow applications which
 * incorrectly reference no-access memory to run without core-dumping.
 */
int noprotect = 0;

/*
 * Variable to allow performance tools to obtain (via nlist) offset of
 * 'pfhdata' structure in VMMDSEG to read it via /dev/kmem.
 */
int vmm_pfhdata = 0;

Simple_lock vmm_lock_lock = {SIMPLE_LOCK_AVAIL};

/*
 * Lock routines supporting MP-safe and adspace locks.
 * Only the v_vmm_lock() and v_vmm_unlock() routines are used for the
 * adspace lock and so only these need to handle the 'baselevel' parameter.
 */
int base_vmm_lock(struct vmmlock *);
void base_vmm_unlock(struct vmmlock *);
int v_vmm_lock(struct vmmlock *, uint baselevel);
int v_vmm_unlock(struct vmmlock *, uint baselevel);
int v_vmm_unlockmine(struct vmmlock *, uint baselevel);
int v_vmm_slock(struct vmmlock *);
int v_vmm_locktry(struct vmmlock *);
int v_vmm_lockmine(struct vmmlock *);

/*
 * Bit in vmmlock lock_word to indicate lock taken at base level.
 */
#define BASE_LEVEL	0x80000000

/*
 * vm_handle(sid,key)
 *
 * returns a srval constructed from a specified sid and
 * storage protection key.
 *
 * INPUT PARAMETERS
 *
 * (1) sid - segment identifier
 *
 * (2) key - storage protection key
 *
 *
 * RETURN VALUE
 *
 *      srval constructed for sid and key.
 *
 */

vm_handle(sid,key)
int     sid;	 /* segment identifier */
int     key;	 /* protection key */
{
        return(SRVAL(sid,key,0));
}

/*
 * vm_vmid(srval)
 *
 * returns a base sid constructed from a specified segment
 * register value.
 *
 * INPUT PARAMETER
 *
 *     srval - segment register value
 *
 *
 * RETURN VALUE
 *
 *      base sid constructed from srval.
 *
 */

vm_vmid(srval)
uint    srval;	 /* segment register value */
{
	int sid;

	sid = SRTOSID(srval);
	sid = BASESID(sid);

        return(sid);
}

/*
 * lra(eaddr), lqra(sid,offset)
 *
 * These services are provided to translate an effective or long-form
 * virtual address to a real address.  This translation is done utilizing
 * software data structures rather than utilizing hardware capabilities
 * because not all platforms provide such a capability and because a frame
 * may be resident but may not be mapped in the hardware page table.
 */

lra(eaddr)
uint	eaddr;
{
	return(lqra(SRTOSID(mfsri(eaddr)),eaddr));
}

lqra(sid,offset)
uint	sid;
uint	offset;
{
	uint srsave, pno, oldpri;
	int sidx, nfr;

	/*
         * Get addressability to vmmdseg.
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

	/*
         * Check for valid sid.
         */
	sidx = STOI(sid);
        if (sid == INVLSID || sidx >= pf_hisid || !scb_valid(sidx))
        {
		(void)chgsr(VMMSR,srsave);
                return(-1);
        }

	/*
	 * Perform the lookup.
	 */
	pno = (offset & SOFFSET) >> L2PSIZE;

	/*
	 * Disable to ensure serialization while performing
	 * lookup of the page frame.
	 */
	oldpri = disable_ints();

        if ((nfr = v_lookup(sid,pno)) != -1 && PHYS_MEM(nfr))
	{
		/*
		 * Found on the software hash.  Now determine if it is h/w
		 * hashed at the specified sid (normal or I/O) given the
		 * current page state.  If not, indicate not found.
		 */
		if ((!IOVADDR(sid) && (!pft_inuse(nfr) || pft_pgahead(nfr)))
			||
		    (IOVADDR(sid) && (pft_inuse(nfr) && !pft_pgahead(nfr)))
			||
		    pft_xmemcnt(nfr))
		{
			nfr = -1;
		}
	}

	enable_ints(oldpri);
	
	/*
	 * Return real address or -1 if not found.
	 */
	(void)chgsr(VMMSR,srsave);
	return(nfr == -1 ? nfr : (nfr << L2PSIZE) + (offset & POFFSET));
}

/*
 * p2vmminit()
 *
 * called by phase2init when paging has been defined.  Adjusts
 * parameters to a run time level
 *
 * INPUT PARAMETER: NONE
 *
 * RETURN VALUE: NONE
 *
 */
void
p2vmminit()
{

}

/*
 * TEMPORARY - for fetch-protect/store problem.
 * This can be removed when we no longer need to support
 * the h/w level with this problem.
 *
 * ukerncopy(ptr,nbytes)
 * copies bytes from kernel to user-shadow kernel.
 */
int
ukerncopy(ptr,nbytes)
char * ptr;
uint nbytes; 
{
	uint target, srval;
	srval = SRVAL(SRTOSID(vmker.ukernsrval),0,0);
	target = vm_att(srval, ptr);
	bcopy(ptr,target,nbytes);
	vm_det(target);
	return 0;
}

/*
 * vm_init()
 *
 * called by main at system start up time. registers vmm
 * sysconfig routine and creates lru daemon.
 *
 * INPUT PARAMETER:
 *
 *	NONE
 *
 * RETURN VALUE:
 *
 *	NONE
 *
 */

vm_init()
{
	int vm_config();
#ifdef _VMM_MP_EFF
	int pid, kproc_wash();
#endif /* _VMM_MP_EFF */
	
	/* set pointer to vmm sysconfig routine.
	 */
	vmcfg.func = vm_config;

	/* add the config routine.
	 */
	cfgnadd(&vmcfg);

#ifdef _VMM_MP_EFF
	/*
	 * Create LRU daemon.
	 */
	pid = creatp();
	assert(pid != -1);

	initp(pid, kproc_wash, 0, 0, "lrud");

	/*
	 * XXX - set priority
	setpri(pid, 30);
	 */
#endif /* _VMM_MP_EFF */

	return(0);
}

/*
 * vm_config(cmd,curvar,newvar)
 *
 * VMM configuration routine called by sysconfig (SETPARM) to
 * set configurable VMM values.  Currently, the only configurable
 * values supported are those related to i/o pacing (pageout limit
 * and pageout level).
 *	
 * INPUT PARAMETER:
 *
 *	cmd	- config command (CFGV_PREPARE or CFGV_COMMIT) 
 *	curvar	- pointer to current var structure
 *	newvar	- pointer to new var structure
 *
 * RETURN VALUE:
 *
 *	0			- success
 *	VM_CFGERR_MAXPOUT	- invalid maxpout value
 *	VM_CFGERR_MINPOUT	- invalid minpout value
 *
 */

#define VM_CFGERR_MAXPOUT  ((int)&((struct var *)0)->v_maxpout)
#define VM_CFGERR_MINPOUT  ((int)&((struct var *)0)->v_minpout)

int
vm_config(cmd,curvar,newvar)
int cmd;
struct var *curvar;
struct var *newvar;
{
	struct vmconfig cfg;

	switch(cmd)
	{
	
	/* validate the new values.
	 */
	case CFGV_PREPARE:

		/* check for a change in i/o pacing values.
		 */
		if (curvar->v_maxpout == newvar->v_maxpout &&
		    curvar->v_minpout == newvar->v_minpout)
			return(0);

		/* maxpout and minpout must be positive.
		 */
		if (newvar->v_maxpout < 0)
			return(VM_CFGERR_MAXPOUT);

  		if (newvar->v_minpout < 0)
			return(VM_CFGERR_MINPOUT);

		/* ok if both values are zero.
		 */
		if (newvar->v_maxpout == 0 && newvar->v_minpout == 0)
			return(0);

		/* minpout must be less than maxpout.
		 */
		if (newvar->v_maxpout <= newvar->v_minpout)
			return(VM_CFGERR_MINPOUT);

		/* new values ok.
		 */
		return(0);

	/* set new values.
	 */
	case CFGV_COMMIT:

		/* check for a change in i/o pacing values.
		 */
		if (curvar->v_maxpout == newvar->v_maxpout &&
		    curvar->v_minpout == newvar->v_minpout)
			return(0);

		/* set the values in a critical section.
		 */
		cfg.maxpout = newvar->v_maxpout;
		cfg.minpout = newvar->v_minpout;
		vcs_config(&cfg);

		return(0);	

	default:
		assert(0);
	}
}

/*
 *
 *  NAME: vm_allocsr
 *
 *  FUNCTION:  Allocate a specific segment register.
 *
 *       int vm_allocsr(adspace_t *adsp, caddr_t addr)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure
 *     addr -	address at which to allocate (bits 0..3 indicate SR number)
 *
 *  RETURNED VALUE:
 *     0      if allocation successful
 *     ENOMEM if requested segment reg not available
 *
 *  EXECUTION ENVIRONMENT:
 *     must be called from base level
 *
 *  NOTE:
 *     This routine is invoked by the following macros in <sys/adspace.h>:
 *         vm_ralloc, as_ralloc
 */
int
vm_allocsr(adspace_t *adsp, caddr_t addr)
{
	unsigned	mask;
	int		rc = 0;
	int		waslocked;
	int		mthread;

	/* Calculate the mask for segment register bit in allocation mask. */
	mask = ((unsigned)1 << 31) >> ((ulong)addr >> SEGSHIFT);

	/* Hold the adspace lock while checking and setting the allocation
	 * if the process is multithreaded.
	 */
	mthread = MTHREADT(curthread);
	if (adsp == &U.U_adspace && mthread)
		waslocked = base_vmm_lock(&U.U_adspace_lock);

	/* Check and set bit in allocation mask. */
	if (!(adsp->alloc & mask))
		adsp->alloc |= mask;
	else
		rc = ENOMEM;

	if (adsp == &U.U_adspace && mthread && !waslocked)
		base_vmm_unlock(&U.U_adspace_lock);

	return rc;
}

/*
 *
 *  NAME: as_geth
 *
 *  FUNCTION:  Get segment register handle from address space.
 *
 *       vmhandle_t as_geth(adspace_t *adsp, caddr_t addr)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure
 *     addr -	32-bit virtual address, from which segment register is deduced
 *
 *  RETURNED VALUE:
 *	vmhandle_t -	segment register handle from address space
 *
 *  EXECUTION ENVIRONMENT:
 *     must be called from base level
 *
 * NOTE:
 *     If the segment register handle is derived from the user
 *     address space and refers to a VMM segment, the cross memory
 *     count of the segment is incremented so that the caller can
 *     use the segment without the threat of it being deleted.
 *     as_puth is used to decrement the cross memory count after
 *     the last use of the segment register value.
 */
vmhandle_t
as_geth(adspace_t *adsp, caddr_t addr)
{
	vmhandle_t	srval;
	int		sid, sidx;
	int		waslocked;
	uint		srsave;

	/* Non-user address spaces don't need to be protected.
	 */
	if (adsp != &U.U_adspace)
		return (adsp)->srval[((ulong)addr)>>SEGSHIFT];

	/* as_geth should be called at base level for the user
	 * address space.
	 */
	ASSERT(CSA->intpri == INTBASE);

	/* Get addressability to vmmdseg.
         */
        srsave = chgsr(VMMSR, vmker.vmmsrval);

	/* Hold the adspace lock while getting srval and "holding" segment.
	 */
	waslocked = base_vmm_lock(&U.U_adspace_lock);

	srval = adsp->srval[((ulong)addr)>>SEGSHIFT];
	sid = SRTOSID(srval);
	sidx = STOI(sid);

	/* Check for invalid and I/O space segreg values.
	 */
	if ((srval & IOSEGMENT) || INVALIDSID(sid) || !scb_valid(sidx))
	{
		if (!waslocked)
			base_vmm_unlock(&U.U_adspace_lock);
		(void)chgsr(VMMSR, srsave);
		return srval;
	}

	/* Increment the cross-memory attach count to prevent the segment
	 * from being deleted at detach time.  The caller is responsible
	 * for calling as_puth() after last use to decrement the count.
	 * This needs to be serialized with interrupt handlers.
	 */
	fetch_and_add_h((atomic_p)&scb_xmemcnt(sidx), 1);
	ASSERT(scb_xmemcnt(sidx) != 0);

	(void)chgsr(VMMSR, srsave);

	if (!waslocked)
		base_vmm_unlock(&U.U_adspace_lock);

	return srval;
}

/*
 *
 *  NAME: as_getsrval
 *
 *  FUNCTION:  Get segment register handle from address space.
 *
 *       vmhandle_t as_getsrval(adspace_t *adsp, caddr_t addr)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure
 *     addr -	32-bit virtual address, from which segment register is deduced
 *
 *  RETURNED VALUE:
 *	vmhandle_t -	segment register handle from address space
 *
 *  EXECUTION ENVIRONMENT:
 *     must be called from base level
 *
 *  NOTE:
 *     The routine is identical to as_geth, except that the cross
 *     memory count is not altered, so as_puth need not be called
 *     later.  This routine does no serialization with modifiers
 *     of the address space.  It should only be called for segment
 *     registers that are not modified in a multithreaded
 *     environment.
 */
vmhandle_t
as_getsrval(adspace_t *adsp, caddr_t addr)
{
	return adsp->srval[((ulong)addr)>>SEGSHIFT];
}

/*
 *
 *  NAME: as_puth
 *
 *  FUNCTION:  Release "hold" on segment register handle from address space.
 *
 *       void as_puth(adspace_t *adsp, vmhandle_t srval)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure
 *     srval -	segment register handle originally from address space
 *
 *  RETURNED VALUE:
 *	none
 *
 *  EXECUTION ENVIRONMENT:
 *     may be called from a critical section
 */
void
as_puth(adspace_t *adsp, vmhandle_t srval)
{
	uint		srsave;
	int		sid, sidx;

	/* Non-user address spaces don't need to be protected.
	 */
	if (adsp != &U.U_adspace)
		return;

	/*
         * Get addressability to vmmdseg.
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

	/* Check for I/O space or invalid segreg values. */
	if (!(srval & IOSEGMENT))
	{
		sid = SRTOSID(srval);
		sidx = STOI(sid);

		if (!INVALIDSID(sid) && scb_valid(sidx))
		{
			/* Decrement the cross-memory attach count to allow
			 * the segment to be reclaimed.  This needs to be
			 * serialized with interrupt handlers.
			 */
			ASSERT(scb_xmemcnt(sidx) != 0);
			fetch_and_add_h((atomic_p)&scb_xmemcnt(sidx), -1);
		}
	}

	(void)chgsr(VMMSR,srsave);
}

/*
 *  NAME: as_att
 *
 *  FUNCTION:  convert (srval,offset) to 32-bit address by allocating
 *             a segment register entry in an address space structure,
 *             loading it with "srval", and returning the 32-bit value
 *             that will selects the sr with "offset" as the offset.
 *	      If the adspace being attached to is the current adspace
 *	      then load the segmenent register.
 *
 *             Never allocate sr's 0, 1, 2, or 14
 *
 *       caddr_t = as_att(adspace_t *adsp, vmhandle_t srval, caddr_t offset)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure ("adspace_t" type)
 *     srval -	segment register value ("vmhandle_t" type)
 *     offset -	offset within segment ("caddr_t" type, hi 4 bits IGNORED)
 *
 *  NOTES:
 *     This function corrects key bit values for the platform it
 *     executes on
 *
 *  RETURNED VALUE:
 *     32-bit value made up of 4-bit sr number || 28-bit offset
 *     NULL if not successful
 *
 *  EXECUTION ENVIRONMENT:
 *     must be called from base level
 *
 *  NOTE:
 *     This routine is also invoked via the following macros in
 *     <sys/adspace.h>:
 *         vm_att, io_att
 */
caddr_t
as_att(adspace_t *adsp, vmhandle_t srval, caddr_t offset)
{
	caddr_t rv;
	int	waslocked;
	int	mthread;

	/* Hold the adspace lock while checking and setting the allocation
	 * if the process is multithreaded.
	 */
	mthread = MTHREADT(curthread);
	if (adsp == &U.U_adspace && mthread)
		waslocked = base_vmm_lock(&U.U_adspace_lock);

	rv = (caddr_t)ml_as_att(adsp, srval, offset);

	if (adsp == &U.U_adspace && mthread && !waslocked)
		base_vmm_unlock(&U.U_adspace_lock);

	return rv;
}

/*
 *  NAME: as_det
 *
 *  FUNCTION:  Given an address space structure and a 32-bit virtual
 *             address, release the segment register associated with
 *             that address.
 *	      Will invalidate the segment register if detaching
 *	      from current segment register
 *
 *             Never release sr's 0, 1, 2, or 14.
 *
 *       int as_det(adspace_t *adsp, caddr_t addr)
 *
 *  INPUT STATE:
 *     adsp -	ptr to address space structure (adspace_t)
 *     addr -	32-bit virtual address (caddr_t)
 *
 *  RETURNED VALUE:
 *     0 = successful
 *     EINVAL = detaching invalid address
 *
 *  EXECUTION ENVIRONMENT:
 *  	must be called from base level
 */
int
as_det(adspace_t *adsp, caddr_t addr)
{
	caddr_t 	rv;
	int		waslocked;
	int		mthread;
	extern void	cs_mpc_issue(void);

	ASSERT(CSA->intpri == INTBASE);

	/* Hold the adspace lock while checking and setting the allocation. */
	mthread = MTHREADT(curthread);
	if (adsp == &U.U_adspace && mthread)
		waslocked = base_vmm_lock(&U.U_adspace_lock);

	rv = (caddr_t)ml_as_det(adsp, addr);

	if (adsp == &U.U_adspace && mthread && !waslocked)
		base_vmm_unlock(&U.U_adspace_lock);

#ifdef _POWER_MP
	/* For an MP system we must shoot down the sid in other threads
	 * of this process potentially running on other processors.  We
	 * do this by interrupting all other processors, which reloads
	 * the segment registers in user mode from the user address space.
	 */
	if (adsp == &U.U_adspace && mthread)
		cs_mpc_issue();
#endif /* _POWER_MP */

	return rv;
}

/*
 *  NAME: as_seth
 *
 *  FUNCTION:  Put a "handle" into a adspace, given a handle
 *             and a 32-bit virtual address.  The high order 4 bits of
 *             the 32-bit VA specify which segment register is to be loaded.
 *	      if the adspace is the calling process's current adspace
 *	      then the handle will be loaded into a segment register
 *
 *       void as_seth(adspace_t *adsp, vmhandle_t srval, caddr_t addr)
 *
 *  NOTES:
 *	Corrects key bit values for the platform it executes on
 *
 *  INPUT STATE:
 *     adsp -	pointer to user address space
 *     srval -	handle (new contents for segment register)
 *     addr -	32-bit virtual address
 *
 *  RETURNED VALUE:
 *     none
 *
 *  EXECUTION ENVIRONMENT:
 *       must be run from base level
 */
void
as_seth(adspace_t *adsp, vmhandle_t srval, caddr_t addr)
{
	int		waslocked;
	int		oldsrval;
	extern void	cs_mpc_issue(void);

	/* We only need to hold the adspace lock if the process is
	 * multithreaded and we are modifying the user address space.
	 */
	if (!MTHREADT(curthread) || adsp != &U.U_adspace)
	{
		ml_as_seth(adsp, srval, addr);
		return;
	}
	
	waslocked = base_vmm_lock(&U.U_adspace_lock);

#ifdef _POWER_MP
	oldsrval = adsp->srval[((ulong)addr)>>SEGSHIFT];
#endif /* _POWER_MP */

	/* Replace current mapping atomically.
	 */
	ml_as_seth(adsp, srval, addr);

	if (!waslocked)
		base_vmm_unlock(&U.U_adspace_lock);

#ifdef _POWER_MP
	/* Do segment ID shootdown for any segment register value that
	 * would not necessarily cause a fault.
	 * For an MP system we must shoot down the sid in other threads
	 * of this process potentially running on other processors.  We
	 * do this by interrupting all other processors, which reloads
	 * the segment registers in user mode from the user address space.
	 */
	if (!INVALIDSID(oldsrval))
		cs_mpc_issue();
#endif /* _POWER_MP */
}

/*
 *  NAME: base_vmm_lock
 *
 *  FUNCTION:  Hold a vmm lock, possibly waiting for it to freed.
 *
 *       int base_vmm_lock(struct vmmlock *lock)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *
 *  RETURNED VALUE:
 *     0 = lock not held before call, now lock is held
 *     1 = lock was held before call, lock is still held
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 */
int
base_vmm_lock(struct vmmlock *lock)
{
	int	lockword;
	int	intpri;

	/* Try to get lock at base level first. */
	intpri = disable_lock(INTPAGER, &vmm_lock_lock);

	lockword = lock->lock_word;
	if (lockword == -1)
	{
		/* get the lock */
		lock->lock_word = curthread->t_tid | BASE_LEVEL;
		unlock_enable(intpri, &vmm_lock_lock);
		return 0;
	}
	else if ((lockword & OWNER_MASK) == curthread->t_tid)
	{
		/* This thread already holds the lock. */
		unlock_enable(intpri, &vmm_lock_lock);
		return 1;
	}

	unlock_enable(intpri, &vmm_lock_lock);
	
	return vcs_vmm_lock(lock,BASE_LEVEL);	
}

/*
 *  NAME: base_vmm_unlock
 *
 *  FUNCTION:  Release a vmm lock, waking up any waiting threads.
 *
 *       void base_vmm_unlock(struct vmmlock *lock)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *
 *  RETURNED VALUE:
 *	none
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 */
void
base_vmm_unlock(struct vmmlock *lock)
{
	int	intpri;
	int	lockword;

	/* Try to release lock at base level. */
	intpri = disable_lock(INTPAGER, &vmm_lock_lock);

	if (lock->vmm_lock_wait == NULL)
	{
#ifdef DEBUG
		/* Assure ourselves that no other thread owns the lock. */
		lockword = lock->lock_word;
		assert((lockword & OWNER_MASK) == curthread->t_tid ||
			lockword == -1);

		/* The lock must have been acquired from base level */
		assert(lockword & BASE_LEVEL);
#endif

		lock->lock_word = -1;
		unlock_enable(intpri, &vmm_lock_lock);
		return;
	}
	
	/* There are other threads waiting for the lock.
	 * we must release it in the critical section.
	 */
	unlock_enable(intpri, &vmm_lock_lock);

	vcs_vmm_unlock(lock,BASE_LEVEL);
}

/*
 *  NAME: v_vmm_lock
 *
 *  FUNCTION:  Hold a vmm lock, possibly waiting for it to freed.
 *
 *       int v_vmm_lock(struct vmmlock *lock, uint baselevel)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *	baselevel - indicates if lock being acquired at base level
 *
 *  RETURNED VALUE:
 *     0 = lock not held before call, now lock is held
 *     1 = lock was held before call, lock is still held
 *     VM_WAIT = lock is held by another thread, must backtrack
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.  It is protected
 *      by the critical section in UP, and the vmm_lock_lock in MP.
 */
int
v_vmm_lock(struct vmmlock *lock, uint baselevel)
{
	int	lockval;
	int	rc;
        register int            link_register;

        /* Get caller of this routine */
        GET_RETURN_ADDRESS(link_register);

#ifdef _POWER_MP
	simple_lock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	lockval = lock->lock_word;

	if (lockval == -1)
	{
		/* get the lock */
		lock->lock_word = curthread->t_tid | baselevel;
		rc = 0;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	else if ((lockval & OWNER_MASK) == curthread->t_tid)
	{
		/* This thread already holds the lock. */
		rc = 1;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	else
	{
		/* block waiting for lock to free */
		v_wait(&lock->vmm_lock_wait);
		rc = VM_WAIT;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_MISS,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	
#ifdef _POWER_MP
	simple_unlock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	return rc;
}

/*
 *  NAME: v_vmm_unlock
 *
 *  FUNCTION:  Release a vmm lock, waking up any waiting threads.
 *
 *       void v_vmm_unlock(struct vmmlock *lock, uint baselevel)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *	baselevel - indicates whether to unlock if lock acquired at base level
 *
 *  RETURNED VALUE:
 *	none
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.  It is protected
 *      by the critical section in UP, and the vmm_lock_lock in MP.
 */
int
v_vmm_unlock(struct vmmlock *lock, uint baselevel)
{
	int	intpri;
	int	lockword;
        register int            link_register;

        /* Get caller of this routine */
        GET_RETURN_ADDRESS(link_register);

#ifdef _POWER_MP
	simple_lock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	lockword = lock->lock_word;
#ifdef DEBUG
	assert((lockword & OWNER_MASK) == curthread->t_tid || lockword == -1);

	/* If unlocking from base level then base level bit should be set */
	if (baselevel)
		assert(lockword & BASE_LEVEL);
#endif

	/*
	 * Unlock only if thread is current owner AND lock was
	 * acquired at specified level.
	 */
	if (lockword == (curthread->t_tid | baselevel))
	{
		lock->lock_word = -1;
		while (lock->vmm_lock_wait != NULL)
			v_ready(&lock->vmm_lock_wait);

		TRCHKL3T(HKWD_KERN_UNLOCK,lock,lock->lock_word,link_register);
	}
	
#ifdef _POWER_MP
	simple_unlock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	return 0;
}

/*
 *  NAME: v_vmm_unlockmine
 *
 *  FUNCTION:  Release a vmm lock, waking up any waiting threads.
 *	       For this version of unlock it is legal for the lock to
 * 	       be held by a thread other than the caller.
 *
 *       void v_vmm_unlockmine(struct vmmlock *lock, uint baselevel)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *	baselevel - indicates whether to unlock if lock acquired at base level
 *
 *  RETURNED VALUE:
 *	none
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.  It is protected
 *      by the critical section in UP, and the vmm_lock_lock in MP.
 */
int
v_vmm_unlockmine(struct vmmlock *lock, uint baselevel)
{
	int	intpri;
	int	lockword;
        register int            link_register;

        /* Get caller of this routine */
        GET_RETURN_ADDRESS(link_register);

#ifdef _POWER_MP
	simple_lock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	lockword = lock->lock_word;

	/*
	 * Unlock only if thread is current owner AND lock was
	 * acquired at specified level.
	 */
	if (lockword == (curthread->t_tid | baselevel))
	{
		lock->lock_word = -1;
		while (lock->vmm_lock_wait != NULL)
			v_ready(&lock->vmm_lock_wait);

		TRCHKL3T(HKWD_KERN_UNLOCK,lock,lock->lock_word,link_register);
	}
	
#ifdef _POWER_MP
	simple_unlock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	return 0;
}

#ifdef _VMM_MP_SAFE
/*
 *  NAME: v_vmm_slock
 *
 *  FUNCTION:  Hold a vmm lock, spin lock version.
 *
 *       int v_vmm_slock(struct vmmlock *lock)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *
 *  RETURNED VALUE:
 *     0 = lock not held before call, now lock is held
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.
 *	This lock is not recursive when called spin.
 *	Note that this routine is defined in mp only since the
 * 	state of the lock cannot change on a up when we spin
 *	at INTPAGER.
 */
int
v_vmm_slock(struct vmmlock *lock)
{
	int	lockval;
	volatile struct vmmlock *ptr = lock;
        register int            link_register;

        /* Get caller of this routine */
        GET_RETURN_ADDRESS(link_register);

	ASSERT(CSA->intpri != INTMAX);

	while(1)
	{
		simple_lock(&vmm_lock_lock);

		lockval = lock->lock_word;
		if (lockval == -1)
		{
			/* get the lock */
			lock->lock_word = curthread->t_tid;
			TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lock,lockval,LOCK_SWRITE_TRACE,link_register);
			simple_unlock(&vmm_lock_lock);
			return 0;
		}
		simple_unlock(&vmm_lock_lock);

		/* spin on the lockword unprotected
		 */
                while(ptr->lock_word != -1)
                        ptr +=0;
	}
}

/*
 *  NAME: v_vmm_locktry
 *
 *  FUNCTION:  Try to grab a lock, do not block nor spin.
 *
 *       int v_vmm_locktry(struct vmmlock *lock)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *
 *  RETURNED VALUE:
 *     2 = the caller already hold the lock
 *     1 = lock not held before call, now lock is held
 *     0 = lock is held by another thread
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.  It is protected
 *      by the critical section in UP, and the vmm_lock_lock in MP.
 *	This lock is recursive when called spin.
 */
int
v_vmm_locktry(struct vmmlock *lock)
{
	int	lockval;
	int	rc;
        register int            link_register;

        /* Get caller of this routine */
        GET_RETURN_ADDRESS(link_register);

	simple_lock(&vmm_lock_lock);

	lockval = lock->lock_word;

	if (lockval == -1)
	{
		/* get the lock */
		lock->lock_word = curthread->t_tid;
		rc = 1;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	else if ((lockval & OWNER_MASK) == curthread->t_tid)
	{
		/* This thread already holds the lock. */
		rc = 2;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_RECURSIVE,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	else
	{
		/* lock is held by another thread. */
		rc = 0;
		TRCHKL4T(HKWD_KERN_LOCK|hkwd_LOCK_BUSY,lock,lockval,LOCK_SWRITE_TRACE,link_register);
	}
	
	simple_unlock(&vmm_lock_lock);

	return rc;
}
#endif /* _VMM_MP_SAFE */

/*
 *  NAME: v_vmm_lockmine
 *
 *  FUNCTION:  test if the caller hold the lock
 *
 *       int v_vmm_lockmine(struct vmmlock *lock)
 *
 *  INPUT STATE:
 *	lock -	(struct vmmlock *) vmm lock structure
 *
 *  RETURNED VALUE:
 *     1 = the caller already hold the lock
 *     0 otherwise
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine is not protected by the MP safe lock.  It is protected
 *      by the critical section in UP, and the vmm_lock_lock in MP.
 */
int
v_vmm_lockmine(struct vmmlock *lock)
{
	int	rc;

#ifdef _POWER_MP
	simple_lock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	rc = ((lock->lock_word & OWNER_MASK) == curthread->t_tid);

#ifdef _POWER_MP
	simple_unlock(&vmm_lock_lock);
#endif	/* _POWER_MP */

	return rc;
}

/*
 *  NAME: vmm_fork
 *
 *  FUNCTION:  Handle all VMM requirements at process fork time
 *
 *       void vmm_fork(vm_map_t old_map, struct user *uchild)
 *
 *  INPUT STATE:
 *	old_map - address map from forking process
 *	uchild -  user block of child process to be filled in
 *
 *  RETURNED VALUE:
 *	none
 *
 *  EXECUTION ENVIRONMENT:
 *      may be run with interrupts disabled
 *
 *  NOTE:
 *	This routine must be serialized with respect to other
 *	threads of this process, which may be changing the user
 *	address space.
 */
void
vmm_fork(vm_map_t old_map, struct user *uchild)
{
	int waslocked;
	int mthread;

	/* Hold the adspace lock only if we are multithreaded.
	 */
	mthread = MTHREADT(curthread);
	if (mthread)
	{
		waslocked = base_vmm_lock(&U.U_adspace_lock);
		ASSERT(!waslocked);
	}

	/* Update shared memory data.
	 */
	shmfork(uchild);

	/* Fork address map for mmap() -- must be done after shmfork.
	 */
	if (old_map)
		uchild->U_map = (char *)vm_map_fork(old_map, uchild);

	/* Initialize vmm fields in u block.
	 */
	uchild->U_lock_word = -1;
	uchild->U_vmm_lock_wait = NULL;

	if (mthread)
		base_vmm_unlock(&U.U_adspace_lock);
}
