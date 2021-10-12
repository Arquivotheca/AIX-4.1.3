static char sccsid[] = "@(#)24	1.83.1.47  src/bos/kernel/vmm/POWER/v_getsubs.c, sysvmm, bos41J, 9520A_a 5/16/95 17:06:29";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_dpfget, v_ipfget, pfget, v_pfprot, v_makep
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * routines which bring pages into virtual memory.
 */

#include <sys/types.h>
#include "vmsys.h"
#include <jfs/inode.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/low.h>
#include <sys/syspest.h>
#include <sys/except.h>
#include <sys/trchkid.h>
#include <sys/machine.h>
#include <sys/ppda.h>
#include "vm_mmap.h"
#include "vm_map.h"
#include "mplock.h"

/*
 * v_pfget(vaddr,sregval,mst,dsisr)
 *
 * initial page fault handling routine called from flih.
 * v_dpfget is for data page faults and v_ipfget is
 * for instruction page faults.
 *
 * INPUT PARAMETERS
 *
 * (1) vaddr - the 32-bit virtual address of the page fault.
 *
 * (2) sregval - the contents of the segment register associated
 *     with vaddr.
 *
 * (3) mst - pointer to the mstsave in effect when the page fault
 *     occurred (previous  if fault occurred in backtrack state).
 *
 * (4) dsisr - contents of DSISR
 *
 *  RETURN VALUES
 *
 *      0       - ok page fault satisfied without i/o
 *
 *      VM_WAIT - ok i/o needed. process put in wait state.
 *
 *  the following conditions cause the program check slih handler
 *  to be called.
 *
 *      EFAULT  - address invalid.
 *
 *      EFBIG   - file size limit exceeded.
 *
 *      ENOSPC  - disk full.
 *
 *      ENOMEM  - no memory for xpt
 *
 *      VM_NOPFAULT - page fault not allowed (interrupt handler)
 *
 *	EXCEPT_PROT
 *
 *	various exception values that are machine dependent.
 *
 */

#ifdef _VMMDEBUG
int  vmm_mpsafe_iar[MAXCPU] = {0};
#endif /* _VMMDEBUG */

v_dpfget(vaddr,sregval,mst,dsisr,backtrack)
uint    vaddr;          /* 32 - bit virtual address of page fault */
uint    sregval;        /* contents of corresponding sreg */
struct  mstsave *mst;   /* pointer to mstsave when pfault happened */
uint    dsisr;          /* contents of DSISR */
int     backtrack;      /* backtrack flag */
{
        uint line,nfr,bad,sid,pno;
	int	rc,sidx;
	struct ppda *ppda_p = PPDA;
	int	scb_locked = 0;
	extern struct thread si_thread;

#ifdef _VMMDEBUG
	vmm_mpsafe_iar[CPUID] = mst->iar;
#endif /* _VMMDEBUG */

	/*
         * Special processing for backtrack faults.
         */
        if (backtrack)
        {
		/*
		 * We may backtrack during page-ahead so we need to
		 * clear the no_vwait flag.
		 */
		ppda_p->ppda_no_vwait = 0;

		/*
		 * Give up the adspace lock if it was acquired by this thread
		 * from a non-base level.  We can't give it up when it was
		 * acquired at base level since the adspace lock must be held
		 * across the mmap/shmat related system calls to ensure
		 * serialization.  We must give it up for non-base level
		 * acquisitions since it may have been acquired while handling
		 * a user-level fault on mmap/shmat data and we cannot return
		 * to user-level with the lock held.  We can't use the MTHREADT
		 * test here because threads may have exited since we first
		 * acquired the adspace lock leaving us single threaded now
		 * and still requiring us to release the lock.  We must avoid
		 * the call at system initialization time when there is no
		 * adspace lock (no u-block is defined).
		 */
		if (ppda_p->_curthread != &si_thread)
			v_vmm_unlockmine(&U.U_adspace_lock,0);

#ifdef _VMM_MP_EFF

		/*
                 * Release all locks.
		 * We handle a fault on a dummy parent SCB as a special case.
		 * For this case we must maintain serialization by holding
		 * on to the dummy parent SCB lock across the backtrack fault.
		 * If we didn't, a sibling could acquire the SCB lock before
		 * we proceed to handle the fault and could delete the parent
		 * SCB and then we would have an invalid fault pending.  This
		 * isn't a problem on UP or MP-safe because serialization is
		 * maintained by staying at INTPAGER or by continuing to hold
		 * the MP-safe lock.  We release all locks except the lock on
		 * the dummy parent and we tell pfget that the lock is already
		 * held.
		 */
		sidx = STOI(SRTOSID(sregval));
		if (scb_wseg(sidx) && (scb_left(sidx) || scb_right(sidx)))
		{	
			VMM_UNLOCKMOST(&scb_lock(sidx));
			scb_locked = 1;
		}
		else
		{
			VMM_UNLOCKALL();
		}

		/*
		 * We may have backtracked during page-replacement after
		 * queuing i/o so we must start these i/o's.
		 */
		if (ppda_p->ppda_sio)
		{
			ASSERT(!scb_locked);
			v_pfsio();
		}

		/*
		 * We track faults in the lru daemon in xmemok().
		 * We do not want to resolve the fault since we
		 * may need a frame and call the daemon !
		 */
		if(ppda_p->lru == VM_INLRU)
		{
			ASSERT(!scb_locked);
			ppda_p->lru = VM_NOBACKT;
			return 0;
		}

		/*
		 * Clear backtrack state.  It is set again soon by pfget
		 * but clearing it here provides a window on MP to allow
		 * lru to continue.
		 */
		ppda_p->lru = VM_NOBACKT;
#endif /* _VMM_MP_EFF */
        }

        /* is it an abnormal page fault ?
         */
        if ((DSBPR | DSIO | DSFLIO | DSLOOP | DSPROT | DSSEGB) & dsisr)
        {
		ASSERT(!scb_locked);

		/* i/o space exceptions should not get to here.
		 */
                if(DSIO & dsisr)
                {
			assert(0);
                }

		/* try to handle protection faults
		 */
		if (DSPROT & dsisr)
		{
                	rc = v_pfprot(vaddr,sregval,mst,dsisr & DSSTORE,ppda_p);
			if (rc == 0 || rc == VM_WAIT || rc == VM_NOWAIT)
				return rc;
			v_exception(rc, vaddr,sregval, mst);
                	return(rc);
		}

		/* for the rest call v_exception
		 */
		rc = (DSBPR & dsisr) ? EXCEPT_TRAP :
		     (DSFLIO & dsisr) ? EXCEPT_FP_IO :
		     (DSLOOP & dsisr) ? EXCEPT_PFTLOOP : EXCEPT_MIXED_SEGS;
		v_exception(rc, vaddr,sregval, mst);
		return rc;
        }

        /* is it a normal page fault ?
         */
        if (DSPFT & dsisr)
        {
                rc = pfget(vaddr,sregval,mst,dsisr & DSSTORE,ppda_p,scb_locked);
		if (rc == 0 || rc == VM_WAIT || rc == VM_NOWAIT)
			return rc;
		v_exception(rc, vaddr,sregval, mst);
		return rc;
        }

#ifdef _POWER_RS
	if (__power_rs())
	{
		/*
		 * Hardware locking facilities are not used --
		 * all locking is done in s/w so trap any lock exception.
		 */
		if(dsisr & DSLOCK)
		{
			assert(0);
		}
        }
#endif	/* _POWER_RS */

	/* should never get here
	 */
	assert(0);
}

v_ipfget(vaddr,sregval,mst,isisr,backtrack)
uint    vaddr;          /* 32 - bit virtual address of page fault */
uint    sregval;        /* contents of corresponding sreg */
struct  mstsave *mst;   /* pointer to mstsave when pfault happened */
uint    isisr;          /* contents of SRR1  */
int     backtrack;      /* backtrack flag */
{
	int rc;
	struct ppda *ppda_p = PPDA;

	ASSERT(!backtrack);

#ifdef _VMM_MP_EFF
	ASSERT(!ppda_p->lru);
#endif /* _VMM_MP_EFF */

        /* is it an abnormal page fault ?
         */
#ifdef	_POWER_601
	if (__power_601())
	{
		/*
		 * Check for instruction fetch to direct-store segment
		 * on 601 for which no bit is set.
		 */
		if (!(isisr & 0xffff0000))
		{
			rc = EXCEPT_IFETCH_IO;
			v_exception(rc, vaddr,sregval, mst);
			return rc;
		}
	}
#endif	/* _POWER_601 */
        if ((ISIO | ISLOOP | ISPROT | ISLOCK) & isisr)
        {
		rc = (ISLOOP & isisr) ? EXCEPT_PFTLOOP :
		     (ISPROT & isisr) ? EXCEPT_PROT :
		     (ISIO & isisr) ? EXCEPT_IFETCH_IO : EXCEPT_IFETCH_SPEC;
		v_exception(rc, vaddr,sregval, mst);
		return rc;
	}

	/* normal page fault
	 */
	vmpf_exfills += 1;
        rc = pfget(vaddr,sregval,mst,0,ppda_p,0);
	if (rc == 0 || rc == VM_WAIT || rc == VM_NOWAIT)
		return rc;
	v_exception(rc, vaddr,sregval, mst);
	return rc;
}

/*
 * pfget(vaddr,sregval,mst,store,ppda_p,scb_locked)
 *
 * page fault processing after fatal hardware errors have been
 * handled by machine specific code (v_pfget). see description
 * of v_pfget for description of parameters and results.
 *
 * RETURN VALUES
 *      0       - ok page fault satisfied without i/o
 *
 *      VM_WAIT - ok i/o needed. process put in wait state.
 *
 *      EFAULT  - address invalid.
 *
 *      EFBIG   - file size limit exceeded.
 *
 *      ENOSPC  - disk full.
 *
 *      ENOMEM  - no memory for xpt
 */

int 
pfget(vaddr,sregval,mst,store,ppda_p,scb_locked)
uint    vaddr;
uint    sregval;
struct  mstsave *mst;
int     store;  /* 0 if load non-zero for store */
struct ppda *ppda_p;
int     scb_locked;
{
        int rc,sid,pno,sidx,pagex,pagein,nfr;
        extern struct vmdebug vm_debug;

	int ssid, spno, pftpp, p, ssidx, spagex;
	int exrc;
	int object, pgobj;
	vm_offset_t offset, pgoff;
	vm_prot_t type, prot;
	int adspace_locked = 0;
	int newsregval;
	int segflag;
	int psalloc = 0;


        vmpf_pgexct += 1;

        /* check here if page faults were allowed
         */
	if (v_nopfault(mst))
		return (VM_NOPFAULT);

#ifdef _VMM_MP_EFF
	ASSERT(!ppda_p->lru);

	if (!scb_locked)
		VMM_CHKNOLOCK();
#endif /* _VMM_MP_EFF */

        /* get sid pno and sidx and pagex
         */
        sid = SRTOSID(sregval);
        sidx = STOI(sid);
        pno  = (vaddr & SOFFSET) >> L2PSIZE;
	pagex = SCBPNO(sid,pno);

	/* Check for lazy update of segment registers.  If the fault
	 * occurred while in user mode and the faulting segment
	 * register value doesn't match that in the user address
	 * space, then return success.  The resume code will load
	 * the new segment register value and the access can proceed.
	 * This does not need to be under the process user address
	 * space lock, because updates after the fault occurred don't
	 * need to be handled.  These would be application thread
	 * serialization problems.
	 */
	if (mst->msr & MSR_PR &&
		U.U_adspace.srval[((ulong)vaddr)>>SEGSHIFT] != sregval)
		return 0;

        /* Check for valid sid.
         */
        if (INVALIDSID(sid) || !scb_valid(sidx))
                return EFAULT;

	/*
	 * If multi-threaded and we faulted on a shmat segment in user mode
	 * or we faulted on an mmap segment in any mode then acquire the
	 * adspace lock.  We need the lock for shmat to prevent the segment
	 * from being detached while we handle the fault.  We need the lock
	 * for mmap to prevent changes to the address map during fault
	 * handling.  The lock may already be held if it was acquired at base
	 * level.  If it is currently held by another thread, we backtrack
	 * and wait.
	 */
	if (MTHREADT(ppda_p->_curthread))
	{
		segflag = U.U_segst[((ulong)vaddr)>>SEGSHIFT].segflag;
		if (scb_mseg(sidx) ||
		    (mst->msr & MSR_PR && (segflag & (SEG_MAPPED|SEG_SHARED))))
		{
			rc = v_vmm_lock(&U.U_adspace_lock,0);
			if (rc == VM_WAIT)
				return rc;

			/*
			 * We don't want to unlock if the lock was already
			 * held at base level.
			 */
			adspace_locked = (rc == 0);

			/*
			 * For user-mode faults check again that addressability
			 * has not been removed, now that we hold the lock.
			 */
			if (mst->msr & MSR_PR &&
			    U.U_adspace.srval[((ulong)vaddr)>>SEGSHIFT] !=
				sregval)
			{
				if(adspace_locked)
					v_vmm_unlock(&U.U_adspace_lock,0);
				return 0;
			}
		}
	}
	
#ifdef _VMM_MP_SAFE
	if (VMM_MPSAFE_PFLOCK() == VM_WAIT)
	{
		if(adspace_locked)
			v_vmm_unlock(&U.U_adspace_lock,0);
		return VM_WAIT;
	}
#endif /* _VMM_MP_SAFE */

	/*
	 * Acquire the SCB lock on the faulting segment
	 * if it's not already held.
	 */
	if (!scb_locked)
	{
		SCB_MPLOCK_S(sidx);
	}
	else
	{
		SCB_LOCKHOLDER(sidx);
	}

	/* NOTE:  After this point we can use label 'out' for returns.
	 */

	ASSERT(scb_valid(sidx));

#ifdef _VMM_MP_EFF
	/* the page fault handler runs in backtrack state
	 */
	ppda_p->lru = VM_BACKT;
#endif /* _VMM_MP_EFF */

	/* if this fault is on a text segment and the segment has not
	 * been marked as computational then do so.  The statistic numperm
	 * must be updated to account for any pages the segment currently
	 * has in memory (numperm is the number of non-computational pages).
         */
        if (((vaddr & SREGMSK) >> L2SSIZE == TEXTSR) && !scb_compseg(sidx))
	{
                scb_compseg(sidx) = 1;
		FETCH_AND_ADD(vmker.numperm, - scb_npages(sidx));
	}

	/* performance trace hook - page fault.
	 */
	TRCHKL3T(HKWD_VMM_PGEXCT,sid,scb_sibits(sidx),pno);

        /* check on limits. uplim and downlim are valid pages.
         */
        if (scb_wseg(sidx))
        {
                if (pagex > scb_uplim(sidx) && pagex < scb_downlim(sidx))
                {
			rc = EFAULT;
			goto out;
                }
        }
        else if (!scb_mseg(sidx))  /* persistent or client */
        {
                if (store && (pagex > BTOPN(U.U_limit)) && !scb_system(sidx) &&
		    !(u.u_flags & UTNOULIMIT))
                {
			rc = EFBIG;
			goto out;
                }
        }

	if (!scb_mseg(sidx))
	{
		/* try to reclaim page by its virtual address.
		 * if vreclaim doesn't find it initiate pagein
		 */
		if ((rc = vreclaim(sid,pno,store,sid,pno,0))
				== VM_NOTIN)
		{
			/* are their sufficient free blocks ?
			 *     - making check here assumes that vreclaim
			 *       doesn't call v_pagein() or anything else
			 *	 that gets a page frame
			 *     - also allows true reclaims to deplete free
			 *	 list without invoking page replacement
			 */
			if (rc = v_spaceok(sidx,1))
				goto out;

			/* Check for stack growth for segments requiring
                         * early paging space allocation.
                         */
                        if (PSEARLYALLOC(sidx,pno) &&
                               pagex >= scb_downlim(sidx) &&
                               pagex < scb_minvpn(sidx) &&
                               (v_isuser(mst) || mst->kjmpbuf ||
                                mst->excbranch))
                        {
                                psalloc = MIN(scb_minvpn(sidx),
                                              BTOPG(U_REGION_SIZE)) - pagex;

                                /* Verify that the paging space allocation
                                 * will not put the system into a low paging
                                 * space state.  If not then update the free
				 * paging space count of the system and the
				 * early allocation count of the segment.
                                 */
                                if (FETCH_AND_LIMIT(vmker.psfreeblks, psalloc,
						    pf_npswarn))
				{
					scb_npseablks(sidx) += psalloc;
					scb_minvpn(sidx) = pagex;
				}
				else
				{
					rc = EFAULT;
					goto out;
				}
                        }

			rc = v_pagein(sidx,pagex,store,sregval);
			pagein = 1;
		}
		else
		{
			pagein = 0;
		}
	}
	else
	{
		/* faulting address is a mapping address.
		 * get the source object, source offset, and protection
		 * from the corresponding address map entry.
		 */
		type = (store ? VM_PROT_READ | VM_PROT_WRITE : VM_PROT_READ);
		if ((rc = vm_map_lookup(&U.U_map, sid, vaddr, type, &object,
					&offset, &prot, &pgobj, &pgoff))
				!= KERN_SUCCESS)
		{
			switch(rc)
			{
			case KERN_PROTECTION_FAILURE:
				exrc = EXCEPT_PROT;
				/* Set bit in DSISR of exception data to
				 * indicate a protection fault.
				 */
				u.u_save.except[EISR] |= DSPROT;
				break;
			case EXCEPT_EOF:
				exrc = EXCEPT_EOF;
				break;
			default:
				exrc = EFAULT;
			}
			rc = exrc;
			goto out;
		}

		/* determine source sid, source pno, and page protection bits.
		 * if paging SCB exists then we search it first.
		 */
		if (pgobj != INVLSID)
		{
			ssid = pgobj;
			spno = pgoff >> L2PSIZE;
			ssidx = STOI(ssid);
			spagex = spno;
		}
		else
		{
			ssidx = STOI(object);
			spagex = offset >> L2PSIZE;
			ssid = ITOS(ssidx, spagex);
			spno = BASEPAGE(spagex);
		}
		pftpp = VMPROT2PP(prot);

		SCB_MPLOCK_S(ssidx);

		/* see if page is hashed at source address or at
		 * some other alias.
		 */
		if ((rc = vreclaim(sid,pno,store,ssid,spno,pftpp))
				== VM_NOTIN)
		{
			/* page isn't in memory.
			 * set sidx,pagex for source object
			 * since this is what we need to page-in
			 * (and what following code needs to set
			 * minvpn, maxvpn and to do pre-paging).
			 */
			sidx = ssidx;
			pagex = spagex;

			/* check on limits.
			 */
			if (!scb_wseg(sidx))  /* persistent or client */
			{
				if (store && (pagex > BTOPN(U.U_limit)) &&
		    		    !(u.u_flags & UTNOULIMIT))
				{
					rc = EFBIG;
					SCB_MPUNLOCK_S(ssidx);

					/* restore original sidx value
					 * for unlocking of the faulting scb.
					 */
					sidx = STOI(sid);
					goto out;
				}
			}

			/* are their sufficient free blocks ?
			 */
			if (rc = v_spaceok(sidx,1))
			{
				SCB_MPUNLOCK_S(ssidx);

				/* restore original sidx value
				 * for unlocking of the faulting scb.
				 */
				sidx = STOI(sid);
				goto out;
			}

			rc = v_pagein(sidx,pagex,store,SRVAL(ssid,0,0));
			pagein = 1;

			if (rc == ENOENT)
			{
				/* pagein couldn't find page in paging SCB.
				 * need to get page from source SCB.
				 * first touch page in source SCB.
				 */
				ssidx = STOI(object);
				spagex = offset >> L2PSIZE;
				ssid = ITOS(ssidx, spagex);
				spno = BASEPAGE(spagex);

				/* Take the source scb to avoid explicit pageout.
				 * mapping scb, paging scb and source scb locked here
				 */
				SCB_MPLOCK_S(ssidx);

				(void)chgsr(TEMPSR,SRVAL(ssid,0,0));
				p = (TEMPSR << L2SSIZE) + (spno << L2PSIZE);
				TOUCH(p);

				if (store)
				{

#ifdef _POWER_MP
					/* Remove any entries for the mapping 
					 * segment that might have been created
					 * sid <- mapping segment
					 * ssid <- source segment
					 * We did a TOUCH above and we expect
					 * the source page to be resident.
					 */

					nfr = v_lookup(ssid,spno);
					assert(nfr >= 0);
					v_delapt(APTREG,sid,pno,nfr);
					P_REMOVE(sid,pno,nfr);
#endif
					/* mapping segment is no longer involved.
					 * paging scb and source scb remain locked.
					 * paging object will be the scb unlocked at
					 * at the end of pfget.
					 */
					SCB_MPUNLOCK_S(STOI(sid));
					

					/* perform copy-on-write from source.
					 * note that this is considered a
					 * pagein for the paging SCB so
					 * sidx,pagex are still set correctly.
					 */
					rc = copysource(ssid,spno,pgobj,
							pgoff >> L2PSIZE);
				}
				else
				{
					/* Insert page with r/o access.
					 */

					/* Unlock the paging segment SCB
					 */
					SCB_MPUNLOCK_S(sidx);
					
					rc = 0;
					/* get the page frame of
					 * the source 
					 */
					nfr = v_lookup(ssid,spno);
					assert(nfr >= 0);
					v_insapt(APTREG,sid,pno,nfr,RDONLY,
						 pft_wimg(nfr));
					P_ENTER(NORMAL,sid,pno,nfr,RDONLY,
						pft_wimg(nfr));
					pagein = 0;
			
					/* no page-in so reset sidx, pagex
					 * to original fault for use below.
					 */
					sidx = STOI(sid);
					pagex = SCBPNO(sid,pno);
				}

				/* unlock source object.
				 * if store, the paging object is still locked 
				 * else mapping object is still locked (load)
				 */
				SCB_MPUNLOCK_S(ssidx);
			}
			else /* ENOENT */
			{
				/* unlock mapping segment
				 * keep source object locked
				 * (i.e. paging object if one exists
				 *  or real source)
				 */
				SCB_MPUNLOCK_S(STOI(sid));
			}
		}
		else /* vreclaim */
		{
			pagein = 0;

			/* unlock source or pgobj object
			 * mapping object remains locked.
			 */
			SCB_MPUNLOCK_S(ssidx);
		}
	}

        switch(rc)
        {
        case 0 :
		ppda_p->_curthread->t_procp->p_minflt += 1;
                break;
        case VM_WAIT:
		ppda_p->_curthread->t_procp->p_majflt += 1;
		vmpf_pendiowts += 1;
                break;
        default :
		goto out;
        }

        /* update maxvpn and minvpn (working only).
         */
        if (scb_wseg(sidx))
        {
                if (pagex <= scb_uplim(sidx))
                {
                        scb_maxvpn(sidx) = MAX(pagex,scb_maxvpn(sidx));
                }
                else
                {
                        scb_minvpn(sidx) = MIN(pagex,scb_minvpn(sidx));
                }
        }
        else if (!scb_mseg(sidx))  /* persistent or client */
        {
                scb_maxvpn(sidx) = MAX(pagex,scb_maxvpn(sidx));
        }

        /* pre-paging only for persistent or client segments
         * with pagein initiated.
         */
	if (!scb_wseg(sidx) && !scb_jseg(sidx) && !scb_logseg(sidx) && pagein)
	{
		ppda_p->ppda_no_vwait = 1;
		vpageahead(sidx,pagex,store);
		ppda_p->ppda_no_vwait = 0;
	}

out:

#ifdef _VMM_MP_EFF
	/* leaving the fault handler so clear backtrack state
	 */
	ppda_p->lru = VM_NOBACKT;
#endif /* _VMM_MP_EFF */

	/* WARNING: sidx may have changed identity!
	 */
	SCB_MPUNLOCK_S(sidx);

#ifndef _POWER_MP
        /* initiate i/o if any was queued for UP.
         */
        if (pf_iotail >= 0)
                v_pfsio();
#endif /* _POWER_MP */

#ifdef _VMM_MP_EFF
        /* initiate i/o if any was queued for MP-eff.
         */
	if(ppda_p->ppda_sio)
		v_pfsio();
#endif /* _VMM_MP_EFF */

	VMM_MPSAFE_UNLOCK();

	if (adspace_locked)
		v_vmm_unlock(&U.U_adspace_lock,0);

	VMM_CHKNOLOCK();

        return(rc);
}

/*
 * vreclaim(sid,pno,store,ssid,spno,prot)
 *
 * Searches page table to see if page is in memory.  If not
 * it returns VM_NOTIN.  If it is in memory but the process
 * has to wait then VM_WAIT is returned.  If it is in memory
 * and the process does not need to wait then this routine
 * returns 0.
 *
 * RETURN VALUES
 *      0  - page found no wait
 *	VM_WAIT - page found caller must wait.
 *      VM_NOTIN - page not found. 
 */

static
vreclaim(sid,pno,store,ssid,spno,prot)
int     sid;    /* faulting segment id */
int     pno;    /* faulting page number */
int     store;  /* 0 if load non-zero for store */
int	ssid;	/* source sid */
int	spno;	/* source page number */
int	prot;	/* mapping page protection key */
{
        int hash,nfr,msr,ssidx,rc;
	struct ppda *ppda_p = PPDA;

	SCB_LOCKHOLDER(STOI(sid));
	SCB_LOCKHOLDER(STOI(ssid));

	/* see if page frame is in memory
	 */
	if ((nfr = v_lookup(ssid,spno)) < 0)
		return(VM_NOTIN);

	ssidx = STOI(ssid);

	/* is the page hidden due to pageahead ?
	 */
	if (pft_pgahead(nfr))
	{
		/* call vpageahead().
	 	 */
		ppda_p->ppda_no_vwait = 1;
		vpageahead(ssidx,pft_pagex(nfr),store);
		ppda_p->ppda_no_vwait = 0;

#ifndef _VMM_MP_EFF
		/* check if nfr has been stolen and reused (additional
		 * page-ins initiated by page-ahead can caused nfr to be
		 * stolen when page-replacement is done in-line).
		 * if so, return.
		 */
		if (v_lookup(ssid,spno) != nfr)
			return(0);	
#endif /* _VMM_MP_EFF */

		/* clear pageahead and unhide page if inuse.
	 	 */
		pft_pgahead(nfr) = 0;
		if (pft_inuse(nfr))
		{
			P_ENTER(IODONE,ssid,spno,nfr,
				pft_key(nfr),pft_wimg(nfr));
			return(0);
		}
	}

	/*
	 * We should not fault on log pages that are already
	 * resident because they are kept addressible at all times.
	 * Unfortunately, on MP we may get here due to race conditions
	 * involving multiple processors faulting on a page or due to
	 * the fact that endpagein() allows a thread to re-fault on a
	 * paged-in page before it has been made addressible.
	 * So this assert is left as a comment only.
	 *
	ASSERT(!scb_logseg(STOI(pft_ssid(nfr))));
	 */

	/* performance trace hook - page reclaim.
	 */
	TRCHKL4T(HKWD_VMM_RECLAIM,sid,scb_sibits(ssidx),pno,nfr);

	/* if io state just return
	 */
	if (pft_pagein(nfr) || pft_pageout(nfr))
	{
		v_wait(&pft_waitlist(nfr));
        	return(VM_WAIT);
	}


#ifdef _POWER_RS
	if (__power_rs())
	{
		/* for RS1, RS2 pages are hidden for dma i/o (see xmemdma)
		 * and this check must be serialized at INTMAX.
		 */
		msr = disable();

		/* check that page is still unavailable
		 */
		if (pft_xmemcnt(nfr) > 0)
		{
			v_wait(&pft_waitlist(nfr));
			mtmsr(msr);
			return(VM_WAIT);
		}

		mtmsr(msr);
	}
#endif /* _POWER_RS */

	assert(pft_inuse(nfr));
	
	/*
	 * Page is In-Use so enter new mapping into page table.
	 */
	if (scb_mseg(STOI(sid)))
	{
		prot = RDONLYPP(nfr,prot);
		if (rc = v_insapt(APTREG,sid,pno,nfr,prot,pft_wimg(nfr)))
		{
			/*
			 * We must wait for an outstanding pin operation on
			 * the alias to complete before re-assigning it.
			 */
			ASSERT(rc == EAGAIN);
			v_wait(&pft_waitlist(nfr));
			return(VM_WAIT);
		}
	}
	else
	{
		prot = pft_key(nfr);
	}
	P_ENTER(NORMAL,sid,pno,nfr,prot,pft_wimg(nfr));
	
	return(0);
}
 
/*
 * vpageahead(sidx,pagex,store)
 *
 * performs prepaging for non-journalled persistent segments.
 *
 * prepaging will be performed if the last page faulted on
 * within the segment is the page previous to pagex, or pagex
 * is both a hidden page and the last prepaged page to be
 * hidden within the segment.  the number of pages prepaged
 * will start at pf_minpgahead and grow to pf_maxpgahead as a 
 * sequentail page reference pattern continues.  Each time
 * prepaging occurs the first prepaged page will be hidden and
 * recorded to allow prepaging to be triggered before a page
 * is required.
 *
 * prepaging will only occur when pagex is the target of a
 * load operation and will not occur for page zero.
 *
 * this routine should only be called for non-journalled
 * persistent segments or client segments.
 *
 * RETURN VALUES
 *      None
 *
 */

static
vpageahead(sidx,pagex,store)
int sidx;	/* index in segment table */
int pagex;	/* original page fault page number */
int store;	/* true if op was a store */
{
	int firstp,nextp,nfr,sid,nump;
	union xptentry *xpt, *v_findiblk();

	SCB_LOCKHOLDER(sidx);

	/* don't do anything if op was a store.
	 */
	if (store)
		return;

	/* if no page frame (page-in returned VM_WAIT or some error
	 * without allocating a page frame) just return.
	 */
	if ((nfr = v_lookup(ITOS(sidx,pagex),BASEPAGE(pagex))) < 0)
		return;

	/* is pagex a hidden pageahead page ?
	 */
	if (pft_pgahead(nfr))
	{
		/* is pagex the last hidden page ?
		 * if so, get first page to be prepaged.    
		 */
		if (scb_lstpagex(sidx) != pagex)
		{
			scb_lstpagex(sidx) = pagex;
			scb_npgahead(sidx) = 0;
			return;
		}
		firstp = pagex + (1 << scb_npgahead(sidx));
	}
	else
	{
		/* check for sequential pattern or page 0.
		 */
		if (scb_lstpagex(sidx) != pagex - 1 || pagex == 0)
		{
			scb_lstpagex(sidx) = pagex;
			scb_npgahead(sidx) = 0;
			return;
		}
		firstp = pagex + 1;
	}

	/* get the number of pages to prepage.
	 * scb_npgahead is log base 2 of number of pages and we assume
	 * that pf_maxpgahead is never so large as to allow scb_npgahead
	 * to wrap.
	 */
	do {
		scb_npgahead(sidx)++;
		nump = 1 << scb_npgahead(sidx);
	} while (nump < pf_minpgahead);

	while (nump > pf_maxpgahead)
	{
		scb_npgahead(sidx)--;
		nump = 1 << scb_npgahead(sidx);
	}

	/* prepage the page interval [firstp, firstp + nump - 1].
	 */
	for (nextp = firstp; nextp < firstp + nump; nextp++)
	{

		/* don't prepage if next page is already in memory.
	 	 */
		if (v_lookup(ITOS(sidx,nextp),BASEPAGE(nextp)) >= 0)
			return;

		/* check xpt of next page if persistent segment.
	 	 */
		if (scb_pseg(sidx))
		{
			xpt = v_findiblk(sidx,nextp);
			if (xpt == NULL || xpt->fragptr == 0)
				return;
		}

		/* are their sufficient free blocks ?
		 */
		if (v_spaceok(sidx,1))
			return;

		/* prepage next page.
	 	 */
		sid = ITOS(sidx,nextp);
		v_pagein(sidx,nextp,store,SRVAL(sid,0,0));

		/* first page will be hidden to trigger next readahead.
	 	 */
		if (nextp == firstp)
		{
			/* if no page frame just return.
			 */
			if ((nfr = v_lookup(ITOS(sidx,nextp),BASEPAGE(nextp)))
					< 0)
				return;
			pft_pgahead(nfr) = 1;
			scb_lstpagex(sidx) = nextp;
		}

        	scb_maxvpn(sidx) = MAX(nextp,scb_maxvpn(sidx));
	}
}

/*
 * v_pfprot(vaddr,sregval,mst,store,ppda_p)
 *
 * handles storage protection interrupt.
 *
 * a store into a read-only page is allowed if the storage
 * access key in the segment register is ok and either
 * the page is associated with a vmap block in a working
 * storage segment or the page is in a persistent segment.
 * in the vmap block case the storage protect key is changed
 * a disk block allocated, and the association with the
 * vmap block broken (see v_unvmap()). 
 * 
 * for persistent storage, there are two cases to handle.
 * if the page has no-disk block allocated for it, it is
 * a page of zeros previously materialized by a load. in
 * this case, we release the page and re-execute the store.
 * otherwise the page has a disk-block. we change the key
 * to read-write and if this is the right-most page in the
 * file, the size in the inode is set to the end of the 
 * page.
 *
 * for client segments, v_cpfprot() will be called for a
 * store into a readonly page. v_cpfprot() will set the     
 * page to read_write and send a message (buf struct)
 * to the client strategy routine to indicate the page has
 * been made read-write.
 *
 *
 * loads are never allowed, i.e the address is invalid.
 *
 * Return values
 *
 *      0 - ok
 *
 *      ENOSPC  - disk is full.
 *
 *      EXCEPT_PROT  - address invalid..protection violation.
 *
 *	EFBIG	- ulimit exceeded
 */

int
v_pfprot(vaddr,sregval,mst,store,ppda_p)
uint    vaddr;
uint    sregval;
struct mstsave * mst;
int     store;  /* 0 if load, non-zero for store */
struct ppda *ppda_p;
{
        int sid, sidx, ssidx, spagex, pno, mapx, nfr, pagex, oldsize;
	int rc, fperpage, mmap;
        union xptentry * xpt, * v_findxpt(), * v_findiblk();
	struct inode *ip;
	int object, pgobj, pgpno;
	vm_offset_t offset, pgoff;
	vm_prot_t type, prot;
	int ssid, spno, p;
	int fsid, fpno, fkey, bsid;
	int segflag;
	int adspace_locked = 0;

	/* if execution state of mst doesn't allow a page
	 * fault just return EXCEPT_PROT.
	 */
	if (v_nopfault(mst))
		return (EXCEPT_PROT);

        /* get sid pno and sidx etc.
         */
        fsid = sid = SRTOSID(sregval);
        sidx = STOI(sid);
        fpno = pno  = (vaddr & SOFFSET) >> L2PSIZE;

	/*
	 * If multi-threaded and we faulted on a shmat segment in user mode
	 * or we faulted on an mmap segment in any mode then acquire the
	 * adspace lock.  We need the lock for shmat to prevent the segment
	 * from being detached while we handle the fault.  We need the lock
	 * for mmap to prevent changes to the address map during fault
	 * handling.  The lock may already be held if it was acquired at base
	 * level.  If it is currently held by another thread, we backtrack
	 * and wait.
	 */
	if (MTHREADT(ppda_p->_curthread))
	{
		segflag = U.U_segst[((ulong)vaddr)>>SEGSHIFT].segflag;
		if (scb_mseg(sidx) ||
		    (mst->msr & MSR_PR && (segflag & (SEG_MAPPED|SEG_SHARED))))
		{
			rc = v_vmm_lock(&U.U_adspace_lock,0);
			if (rc == VM_WAIT)
				return rc;

			/*
			 * We don't want to unlock if the lock was already
			 * held at base level.
			 */
			adspace_locked = (rc == 0);

			/*
			 * For user-mode faults check again that addressability
			 * has not been removed, now that we hold the lock.
			 */
			if (mst->msr & MSR_PR &&
			    U.U_adspace.srval[((ulong)vaddr)>>SEGSHIFT] !=
				sregval)
			{
				if(adspace_locked)
					v_vmm_unlock(&U.U_adspace_lock,0);
				return 0;
			}
		}
	}

#ifdef _VMM_MP_SAFE
	if (VMM_MPSAFE_PFLOCK() == VM_WAIT)
	{
		if(adspace_locked)
			v_vmm_unlock(&U.U_adspace_lock,0);
		return VM_WAIT;
	}
#endif /* _VMM_MP_SAFE */

	SCB_MPLOCK_S(sidx);

	/* NOTE:  After this point we can use label 'out' for returns.
	 */

	ASSERT(scb_valid(sidx));

#ifdef _VMM_MP_EFF
	/* the protection fault handler runs in backtrack state
	 */
	ppda_p->lru = VM_BACKT;
#endif /* _VMM_MP_EFF */

        /* if its a load no good.
         */
        if(store == 0)
	{
		/* if machine has h/w problem then assert
		 * rather than allowing effects of random store.
		 */
		assert(vmker.nofetchprot == 0);
                rc = EXCEPT_PROT;
		goto out;
	}

	if (scb_mseg(sidx))
	{
		mmap = 1;

		type = (store ? VM_PROT_READ | VM_PROT_WRITE : VM_PROT_READ);
		if ((rc = vm_map_lookup(&U.U_map, sid, vaddr, type, &object,
					&offset, &prot, &pgobj, &pgoff))
				!= KERN_SUCCESS)
		{
			assert(rc == KERN_PROTECTION_FAILURE);
			rc = EXCEPT_PROT;
			goto out;
		}

		/* If we get here the protection key must be RDONLY which
		 * means one of the following cases apply:
		 * (1) unmodified page of a private mapping
		 * (2) page has no disk block
		 * (3) page is last page in a file
		 * (4) page has a partial disk block in a compressed segment.
		 * The only case that we get a RDONLY fault on a paging sid
		 * is a store to an unmodified page of a private mapping.
		 * In any case, we can use object,offset to look up the frame.
		 */
		ssidx = STOI(object);
		spagex = offset >> L2PSIZE;
		ssid = ITOS(ssidx, spagex);
		spno = BASEPAGE(spagex);

		if (pgobj != INVLSID)
			SCB_MPLOCK_S(STOI(pgobj));

		SCB_MPLOCK_S(ssidx);

		nfr = v_lookup(ssid,spno);
#ifndef _POWER_MP
		assert(nfr >= 0);
#else /* _POWER_MP */
		/* another thread just did the relframe we would have
	 	 * done and won the race.
		 * Page can also be in i/o state.
		 */
		if(nfr < 0 || !pft_inuse(nfr))
		{
			rc = 0;
			SCB_MPUNLOCK_S(ssidx);
			if (pgobj != INVLSID)
				SCB_MPUNLOCK_S(STOI(pgobj));
			goto out;
		}
#endif /* _POWER_MP */

		/* no longer need to keep lock on the mapping segment
		 */
		SCB_MPUNLOCK_S(sidx);

		/* If a paging object exists then this is a store to
		 * an unmodified page of a private mapping.  The r/o
		 * mapping is removed and we establish addressibility
		 * to the frame at its source address so we can perform
		 * the copy-on-write operation.  This logic should be
		 * equivalent to that in the copy-on-write path in pfget.
		 * We need to perform the copy-on-write here without
		 * backtracking to avoid deadlocks related to copy-on-write
		 * and self-modifying code.
		 */
		if (pgobj != INVLSID)
		{

			sidx = STOI(pgobj);
			pgpno = pgoff >> L2PSIZE;
#ifdef _POWER_MP
			/* If a race occurs another thread may already have
			 * a frame for this page in the paging object. We need
			 * to check this and if the frame is present, 
			 * we return. The original fault will be resolved
			 * by backtracking.
			 */


			if (v_lookup(pgobj, pgpno) >= 0)
			{
				SCB_MPUNLOCK_S(ssidx);
				goto out;
			}
#endif /* _POWER_MP */
			v_delapt(APTREG,fsid,fpno,nfr);
			P_ENTER(STARTIO,ssid,spno,nfr,pft_key(nfr),
				pft_wimg(nfr));

			/* Are their sufficient free blocks ?
			 */
			if (rc = v_spaceok(sidx,1))
			{
				SCB_MPUNLOCK_S(ssidx);
				goto out;
			}

			/*
			 * Touch the source address in case page-replacement
			 * got to the frame.  This may fault and backtrack but
			 * at some point we will perform the copy-on-write
			 * without faulting.
			 */
			(void)chgsr(TEMPSR,SRVAL(ssid,0,0));
			p = (TEMPSR << L2SSIZE) + (spno << L2PSIZE);
			TOUCH(p);

			if ((rc = copysource(ssid,spno,pgobj,pgpno)) == 0)
			{
				/* Copy succeeded so update fault stat
				 * and segment limit for paging SCB.
				 */
				curproc->p_minflt += 1;
				if (pgpno <= scb_uplim(sidx))
				{
					scb_maxvpn(sidx) =
						MAX(pgpno,scb_maxvpn(sidx));
				}
				else
				{
					scb_minvpn(sidx) =
						MIN(pgpno,scb_minvpn(sidx));
				}
			}
			SCB_MPUNLOCK_S(ssidx);
			goto out;
		}

		/* Reset sid,pno,sidx,sregval to source segment for use below.
		 */
		sid = ssid;
		sidx = STOI(sid);
		pno = spno;
		sregval = SRVAL(sid,0,0);
	}
	else
	{
		mmap = 0;

		/* get nfr using software hash lookup.
		 */
		nfr = v_lookup(sid,pno);
#ifndef _POWER_MP
		assert(nfr >= 0);
#else /* _POWER_MP */
		/* another thread just did the relframe we would have
	 	 * done and won the race.
		 * Page can also be in i/o state.
		 */
		if(nfr < 0 || !pft_inuse(nfr))
		{
			rc = 0;
			goto out;
		}
#endif /* _POWER_MP */
	}

	sidx = STOI(sid);	/* Work-around for 154934 */
	ASSERT(!scb_mseg(sidx));

	/* performance trace hook - protection fault.
	 */
	TRCHKL4T(HKWD_VMM_PROTEXCT,sid,scb_sibits(sidx),pno,nfr);

        /* is it a working storage segment ?
         */
        if (scb_wseg(sidx))
        {
		xpt = v_findxpt(sidx,pno);
		
		/* check if store allowed.
		 */
                if (nostore(sregval,xpt))
		{
			rc = EXCEPT_PROT;
			goto out;
		}

		/* if vmmapped, unvmap the page.
		 */
                if (xpt->mapblk)
		{
			rc = v_unvmap(sidx,pno,xpt,nfr);
			goto out;
		}

		/* if no disk block allocated, release the
		 * page.
		 */
                if (xpt->cdaddr == 0)
		{
			v_relframe(sidx,nfr);
			rc = 0;
			goto out;
		}

#ifndef _POWER_MP
		rc = EXCEPT_PROT;
		goto out;
#else /* _POWER_MP */
		/*
		 * the page may already have been made
		 * writeable.
		 */
		if(pft_key(nfr) == UDATAKEY)
		{
			rc = 0;
			goto out;
		}
		else
		{
			rc = EXCEPT_PROT;
			goto out;
		}
#endif /* _POWER_MP */
        }
	else	/* persistent or client */
	{
		if (SRTOKEY(sregval))
		{
			rc = EXCEPT_PROT;
			goto out;
		}
	}

	/* if client segment call v_cpfprot() to change
	 * key to FILEKEY and to inform the client segment.
	 * SMP: this works without change because this operation
	 * is idempotent for NFS and DCE.
	 * NOTE that the mpsafe lock and scb lock are released in
	 * v_cpfprot() before the call to the strategy routine !
	 */
	pagex = pft_pagex(nfr);
	if (scb_clseg(sidx))
	{
#ifdef _VMM_MP_EFF
		/* we should not page-fault any more and will be leaving
		 * the protection fault handler so clear backtrack state
		 */
		ppda_p->lru = VM_NOBACKT;
#endif /* _VMM_MP_EFF */

		rc = v_cpfprot(sidx,pagex,nfr);

#ifdef _VMM_MP_EFF
		ASSERT(!VMM_LOCKMINE(scb_lock(sidx)));
		ASSERT(!ppda_p->ppda_sio && !ppda_p->lru);
#endif /* _VMM_MP_EFF */

		if (adspace_locked)
			v_vmm_unlock(&U.U_adspace_lock,0);

		VMM_CHKNOLOCK();
		return(rc);
	}
			
	/* persistent storage case. 
	 * if pno not backed with disk, release page and make
	 * program re-execute store.
 	 */
	if (pagex > BTOPN(U.U_limit) && !scb_system(sidx) &&
	    !(u.u_flags & UTNOULIMIT))
	{
		rc = EFBIG;
		goto out;
	}

	xpt = v_findiblk(sidx,pagex);
	if(xpt == NULL || xpt->fragptr == 0)
	{
		v_relframe(sidx,nfr);
		rc = 0;
		goto out;
	}

	/* if not an mmap fault extend the file size to the end of
	 * the page if it is the last page of the file. and if the
	 * page is partially backed by disk resources, promote its
	 * allocation to a full block as part of extending the file.
	 * for an mmap() fault we don't extend the file. if it is
	 * not a compressed page we just make the page r/w. for
	 * compression, it is always necessary to back the page with
	 * a full disk block.
	 */

	if (mmap && scb_compress(sidx) == 0)
	{
		fkey = UDATAKEY;
	}
	else
	{
		/* extend the page's allocation to a full block if
		 * it is partially backed. (either mmap is false or
		 * compression applies).
		 */
		if (xpt->fptr.nfrags)
		{
			/* cannot extend the allocation if the combit
			 * is set. we'll have to wait.
			 */
			if (scb_combit(sidx))
			{
				vmpf_extendwts += 1;
				v_wait(&pf_extendwait);
				rc = VM_WAIT;
				goto out;
			}

			/* if mmap the file size doesn't change. remember
			 * the file size here.
			 */
			if (mmap)
			{
				ip = GTOIP(scb_gnptr(sidx));
				oldsize = ip->i_size;
			}

			/* in addition to extending the page to a full block,
			 * v_makefrag() will move the file size to the end
			 * of the page and make the page read/write.
			 */

			fperpage = pdt_fperpage(scb_devid(sidx));
			bsid = BASESID(pft_ssid(nfr));
			rc = v_makefrag(bsid,pagex,fperpage);

			/* file size doesn't change if mmap
			 */
			if (mmap && rc == 0)
			{
				if (BTOPN(ip->i_size) == pagex)
					ip->i_size = oldsize;
			}
			
			goto out;
                }

		/* was fully backed. if not mmap, set file size to
		 * end of the page and set the protection key to r/w.
		 */
		if (mmap)
		{
			fkey = UDATAKEY;
		}
		else
		{
			fkey = FILEKEY;
			ip = GTOIP(scb_gnptr(sidx));
			if (BTOPN(ip->i_size) == pagex)
				ip->i_size = (pagex + 1) << L2PSIZE;
		}
	}

	/*
	 * Raise the protection to r/w for this mapping (AND for
	 * the underlying source file unless it is not fully backed
	 * and must remain RDONLY)
	 */
	if (xpt->fptr.nfrags == 0)
		pft_key(nfr) = FILEKEY;
	else
	{
		ASSERT(mmap); 
		ASSERT(xpt == &(ip->i_rdaddr[BTOPN(ip->i_size)])); 
		ASSERT(pft_key(nfr) == RDONLY);
	}
	v_aptkey(fsid,fpno,nfr,fkey);
	P_PROTECT(fsid,fpno,nfr,fkey);
	rc =0;

out:

#ifdef _VMM_MP_EFF
	/* leaving the protection fault handler so clear backtrack state
	 */
	ppda_p->lru = VM_NOBACKT;
	ASSERT(!ppda_p->ppda_sio);
#endif

	/* Warning: we don't exit thru this path when v_cpfprot is called !
	 */
	SCB_MPUNLOCK_S(sidx);

	VMM_MPSAFE_UNLOCK();

	if (adspace_locked)
		v_vmm_unlock(&U.U_adspace_lock,0);

	VMM_CHKNOLOCK();

	return(rc);
}

/*
 * nostore(srval,xpt)
 *
 * returns 1 if a store is not allowed for the srval and xpt
 * specified. segment is working storage.
 */

int nostore(srval,xpt)
int srval;
union xptentry * xpt;
{
        int srkey;
        srkey = SRTOKEY(srval);
        return(xpt->spkey == RDONLY || srkey && xpt->spkey != UDATAKEY);
}

/*
 * v_unvmap(sidx,pno,xpt,nfr)
 *
 * allocates a disk block for a vmapped page and
 * disassociates the page from the vmap block.
 * 
 * the page's modify bit will be set and the 
 * storage protection key for the page will be
 * set from the xpt.
 *
 * Return values
 *
 *      0 - ok
 *
 *      ENOSPC  - disk is full.
 * 
 */

v_unvmap(sidx,pno,xpt,nfr)
int sidx;	/* index in segment table */
int pno;	/* page number */
union xptentry * xpt;	/* xpt pointer */
int nfr;	/* real page number */
{
        int 	sid,mapx,prevcnt;

	SCB_LOCKHOLDER(sidx);

	/* get sid.
	 */
	sid = ITOS(sidx,pno);

	/* touch vmap block and then allocate a disk block.
	 */
	mapx = xpt->cdaddr;
	TOUCH(&pta_vmap[mapx]);

	if (v_dalloc(sidx,pno,xpt))
		return(ENOSPC);

	/* put disk address in pft and set modified bit.
	 */
	pft_devid(nfr) = PDTIND(xpt->word);
	pft_dblock(nfr) = PDTBLK(xpt->word);
	SETMOD(nfr);

	/* the xpt entry is no longer used as a pointer
	 * to the vmapblk. decrement use count on vmapblk
	 * and free it if count is now zero (previous count
	 * was 1).
	 */
	xpt->mapblk = 0;

	FETCH_AND_ADD_R(pta_vmap[mapx].count, -1, prevcnt);
	ASSERT(prevcnt > 0);

	if (prevcnt == 1)
	{
		VMAP_MPLOCK();
		pta_vmap[mapx]._u.next = pta_vmapfree;
		pta_vmapfree = mapx;
		VMAP_MPUNLOCK();
	}

	/*
	 * We want to raise the protection on the page but we
	 * only know the correct key for the base address.
	 * Entering the base address using STARTIO causes
	 * other aliases to be removed so that a re-fault gets
	 * the correct protection.  This is more efficient than
	 * simply removing all page table entries since we
	 * avoid the fault for the base address and since no
	 * cache flush occurs (the page is read-only and thus
	 * is not modified).
	 */
	pft_key(nfr) = xpt->spkey;
	P_ENTER(STARTIO,pft_ssid(nfr),pft_spage(nfr),nfr,pft_key(nfr),
		pft_wimg(nfr));

	return 0;
}

/* 
 * v_makep(sid,pagex)
 *
 * makes a page in a client or persistent segment.
 * parameters assumed to have been checked in vm_makep.
 *
 * (1) client segment: the page is materialized but the
 *     contents of the page are initialized to zero.
 *
 * (2) persistent segment: the page is initialized to 
 *     zeros and a disk block will be allocated if none
 *     exists.  EEXIST is returned if the page already
 *     is in real memory.
 *
 * this routine should not be used for partially backed
 * pages.  for pages of this nature v_makefrag() should
 * be used.
 *
 * INPUT PARAMETERS
 *
 *	sid	- base segment id
 *	pagex	- page number in SCB
 *
 * RETURN VALUES:
 *
 *	0	- ok
 *	EFBIG   - file size limit exceeded.
 *	ESTALE  - segment inactive.
 *      VM_WAIT - ( never makes it back to caller).
 *
 * the following return values occur only for persistent
 * segments.
 *
 *	EEXIST - the page already is defined.
 *	ENOSPC	- out of disk space.
 *
 * this procedure executes at VMM interrupt level with 
 * back-tracking enabled and is only called via vcs_makep.
 */

int 
v_makep1(sid , pagex, nfrp)
int	sid;
int	pagex;
int 	*nfrp;
{
        int rc, sidx, nfr, pno, lw, pdtx, dblk, temp;
	union xptentry * xpt;
	uint srval, vaddr;
	int sidio, pnoio;

	/* check limit on file size
	 */
	if (pagex > BTOPN(U.U_limit) && !(u.u_flags & UTNOULIMIT))
		return(EFBIG);

	sidx = STOI(sid);

	SCB_LOCKHOLDER(sidx);
#ifdef _VMMDEBUG
	assert(sid != INVLSID);
        assert(scb_valid(sidx));
#endif /* _VMMDEBUG */

	if (scb_inactive(sidx))
		return(ESTALE);
       
        /* are their sufficient free blocks ?
         */
        if (rc = v_spaceok(sidx,1))
        {
		return(rc);
        }

	/* check if it is in memory. check is necessary for
	 * persistent because it could be a new page materialized
	 * for a load instruction but for which no disk resources
	 * have been allocated.
         */
	sid = ITOS(sidx, pagex);
	pno = BASEPAGE(pagex);
        if  (v_lookup(sid,pno) >= 0) 
		return ((scb_clseg(sidx)) ? 0 : EEXIST );

	pdtx = scb_devid(sidx);

	/* page is not in memory 
	 */
        if (scb_pseg(sidx))
	{
		/* if its journalled or deferred update must check for
	 	 * page being in extension memory.
	 	 */
		if (scb_jseg(sidx) || scb_defseg(sidx))
		{
			/* lockword has both home and paging space disk
		 	 * address.
		 	 */
			LW_MPLOCK_S();
			if (lw = v_findlw(sid,pno))
			{
				LW_MPUNLOCK_S();

				/* pagex should not have a partial
				 * allocation.
				 */
				assert(!(lword[lw].home & ~(NEWBIT|CDADDR)));

				if (temp = lword[lw].extmem)
				{
					pdtx = PDTIND(temp);
					dblk = PDTBLK(temp);
				}
				else
				{
					dblk = lword[lw].home;
				}
				goto common;
			}
			LW_MPUNLOCK_S();
		}

		if (scb_combit(sidx) && !scb_jseg(sidx))
		{
			v_wait(&pf_extendwait);
			vmpf_extendwts += 1;
			return(VM_WAIT);
		}

		/* check if the page exists on disk.  if not,
		 * call v_fpagein() to make the page.
		 */
		xpt = v_findiblk(sidx,pagex);
		if (xpt == NULL || xpt->fragptr == 0)
		{
			srval = SRVAL(sid,0,scb_jseg(sidx));
			rc = v_fpagein(sidx,pagex,1,srval);

			switch(rc)
			{
			case 0:
        			scb_maxvpn(sidx) = MAX(pagex,scb_maxvpn(sidx));
				break;
			case VM_WAIT:
				break;
			default:
				break;
			}

			return(rc);
		}
		/* if pagex has a partial allocation, it should be
		 * be a compressed page. to avoid complications, 
		 * we just touch the page to bring it in.
		 */
		if (xpt->fptr.nfrags)
		{
			(void)chgsr(TEMPSR, SRVAL(sid, 0,0));
			vaddr = SR13ADDR + (pno << L2PSIZE);
			TOUCH(vaddr);
			assert(0);
		}

		/* pagex does not have a partial allocation.
		 */
		dblk = xpt->fragptr;

	}
	else
	{
		/* client segment
		 */
		assert(scb_clseg(sidx));
		dblk = 0;
	}


	common:

	/* allocate a page frame. fill in pft entry
	 */
#ifdef _VMMDEBUG
	CSA->backt = 0;	/* no more faults */
#endif /* _VMMDEBUG */
	nfr = v_getframe(sid,pno);
	pft_devid(nfr) = pdtx;
	pft_dblock(nfr) = dblk;

	/* Record correct protection key for the page.
	 * Insert the page at its I/O address to initialize the page.
	 */
	pft_key(nfr) = FILEKEY;
        sidio = IOSID(sid);
        pnoio = pno;

	P_ENTER(STARTIO,sidio,pnoio,nfr,pft_key(nfr),pft_wimg(nfr));

	/* Insert on SCB list.
	 * before setting pft_pagein to avoid
	 * inserting on the scb iolist.
	 */
	v_insscb(sidx,nfr);
	pft_pagein(nfr) = 1;
	
	*nfrp = nfr;

       	scb_maxvpn(sidx) = MAX(pagex,scb_maxvpn(sidx));

	return(0);
}

/*
 * v_makep per se , calls v_makep1
 */
int 
v_makep(sid , pagex)
int	sid;
int	pagex;
{
	int nfr, rc, pno, sidio, sidx;

	SCB_LOCKHOLDER(STOI(sid));

	nfr = 0;
	if (rc = v_makep1(BASESID(sid),pagex,&nfr))
		return rc;
	else
	/*
	 * if the page is from the free list we must zero it
	 * before making the page addressable at its normal address
	 */ 
	if(nfr)
	{
		ASSERT(rc == 0);
                pno = BASEPAGE(pagex);
		sidx = STOI(sid);
                sid = ITOS(sidx, pagex);
                sidio = IOSID(sid);

                /* zero the page.
                 */
                ZEROPAGE(sidio,pno);

                /* Insert page at normal address with correct key.
                 * This also clears the modified bit so we need to
                 * turn it back on.
                 */
                pft_pagein(nfr) = 0;
                P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
                SETMOD(nfr);
                pft_inuse(nfr) = 1;
        }
	return 0;
		
}

/* 
 * v_movep(sid,src,dest)
 *
 * copies a page of data from a source segment to a page
 * align target within a destination segment.
 * 
 * the target page will be created (via a call to v_makep())
 * if it does not exist.
 *
 * v_movep() updates information in exception structure of
 * the process' mst to indicate the progress made.  this
 * information is also updated by v_movefrag() and the page
 * fault handler and is used by the various VMM copy routines
 * to determine the amount of data copied in the event of an
 * exception.
 *
 * INPUT PARAMETERS:
 *
 *	sid	- relative segment id for destination
 *	sp	- pointer to a vmsrclist structure describing the source
 *	dest	- page aligned target of copy
 *
 * RETURN VALUES:
 *
 *	0	- ok
 *      VM_WAIT - (never makes it back to caller).
 *
 * this procedure executes at VMM interrupt level with 
 * back-tracking enabled and is only called via vmpcopy.
 */

v_movep(sid,sp,dest)
uint sid;
struct vmsrclist *sp;
uint dest;
{
	int pno, pagex, i;
	uint src, src2;
	struct mstsave *mst = CSA->prev;
	int rc, sidx, sidio;
	int nfr = 0;
	char *tptr;

	SCB_LOCKHOLDER(STOI(sid));

	/* touch the source pages described by the vmsrclist and set 
	 * the exception address.
	 */
	for (i = 0; i < sp->nvecs; i++)
	{
		/* set exception address and touch first page of
		 * the vmsrclist element.
		 */
		mst->except[EORGVADDR] = src = (uint) sp->vecs[i].iov_base;
		TOUCH(src);

		/* if vmsrclist element spans a page boundry update
		 * exception address with address of the first byte
		 * of the second page and touch this byte.
		 */
		src2 = (src + sp->vecs[i].iov_len - 1) & ~POFFSET;
		if ((src >> L2PSIZE) != (src2 >> L2PSIZE))
		{
			mst->except[EORGVADDR] = src2;
			TOUCH(src2);
		}
	}

	/* make the destination page.
	 */
	mst->except[EORGVADDR] = dest;
	pno = (dest & SOFFSET) >> L2PSIZE;
	pagex = SCBPNO(sid,pno);
	if ((rc = v_makep1(BASESID(sid),pagex,&nfr)) == VM_WAIT
		|| rc == VM_NOWAIT)
		return(rc);

	/*
	 * if the page is from the free list we must copy
	 * before making the page addressable at its normal address
	 * this is made here (avoid zeroing it, which is carried
	 * in v_premakep() if needed).
	 */ 
	if(nfr)
	{
		ASSERT(rc == 0);

        	sidio = IOSID(sid);
        	(void)chgsr(TEMPSR,SRVAL(sidio,0,0));
        	tptr  = (char * )( (TEMPSR << L2SSIZE) + (pno << L2PSIZE) );

		/* copy the PSIZE bytes of data described by the vmsrclist 
		 * to the destination page.
		 */
		for (i = 0; i < sp->nvecs; i++)
		{
			bcopy(sp->vecs[i].iov_base, tptr, sp->vecs[i].iov_len);
			tptr += sp->vecs[i].iov_len;
		}

		/* Insert page at normal address with correct key.
	 	 * This also clears the modified bit so we need to
		 * turn it back on.
		 */
		pft_pagein(nfr) = 0;
		P_ENTER(IODONE,sid,pno,nfr,pft_key(nfr),pft_wimg(nfr));
		SETMOD(nfr);
		pft_inuse(nfr) = 1;
	}
	/* copy the source page to the destination page.
	 * We can fault on the bcopy and longjmp to vmpcopy
	 * if the page was there w/ a bad protection or if
	 * no disk block available, ...
	 */
	else 
	{
		/* copy the PSIZE bytes of data described by the vmsrclist 
		 * to the destination page.
		 */
		for (i = 0; i < sp->nvecs; i++)
		{
			bcopy(sp->vecs[i].iov_base, dest, sp->vecs[i].iov_len);
			dest += sp->vecs[i].iov_len;
		}
	}

	ASSERT(!rc || rc == EEXIST);

	return(0);
}

/*
 * v_nopfault(mst)
 *
 * returns 0 if mst is a state for which a page-fault
 * is allowed and 1 if not.
 */
static
v_nopfault(mst)
struct mstsave * mst;
{
	/* ok if normal process level or set up for back-tracking
	 */
	if (mst->intpri == INTBASE || mst->backt)
		return 0;

	/* no pfault if executing as an interrupt handler.
	 */
        if (mst->prev != NULL )
                return(1);

	/* ok if running as process at INTMAX
	 */
	if (mst->intpri == INTMAX)
		return 0;

	/* not ok otherwise (process mst between INTBASE and INTMAX)
	 */
	return 1;
}

/*
 * copysource(ssid,spno,tsid,tpno)
 *
 * Copies page specified by ssid,spno to page specified by tsid,tpno.
 * Used for mmap() support when a store occurs to a private mapping and
 * the page must be copied from the source SCB (copy-on-write).
 *
 * RETURN VALUES
 *
 *      0       - page copied successfully
 *
 *      ENOSPC  - no disk space
 *
 * Assumes that source page exists
 */

static
copysource(ssid,spno,tsid,tpno)

int	ssid;	/* sid of source segment */
int	spno;	/* page number in source */
int     tsid;   /* sid of target segment */
int     tpno;   /* page number target */
{
        int  tsidx, p, newf, key;
        union xptentry * xpt, * v_findxpt();
	int sidio, pnoio;
	int rc;

	tsidx = STOI(tsid);

	SCB_LOCKHOLDER(STOI(ssid));
	SCB_LOCKHOLDER(tsidx);

	/*
	 * Get xpt entry for target page
	 * grow if it doesn't exist
	 */
        xpt = v_findxpt(tsidx,tpno);
        if (xpt == NULL)
        {
                if (rc = v_growxpt(tsidx,tpno))
                {
			return(rc);
                }
                xpt = v_findxpt(tsidx,tpno);
        }

	/*
	 * Allocate a disk block for target page.
	 */
	if (v_dalloc(tsidx,tpno,xpt))
		return(ENOSPC);

        /*
         * Take a page frame from the free list.
         */
        newf = v_getframe(tsid,tpno);
	pft_devid(newf) = PDTIND(xpt->word);
	pft_dblock(newf) = PDTBLK(xpt->word);

        /*
         * Set protection key (default key is fine since correct
	 * protection for mapping is used when page is aliased).
         */
        key = xpt->spkey = scb_defkey(tsidx);

	/* Record correct protection key for the page (this is a r/w key).
	 * Insert the page at its I/O address to initialize the page.
         */
	pft_key(newf) = key;
        sidio = IOSID(tsid);
        pnoio = tpno;
	P_ENTER(STARTIO,sidio,pnoio,newf,pft_key(newf),pft_wimg(newf));

	/* Insert on SCB list.
	 * before setting pft_pagein to avoid 
	 * inserting on the scb iolist.
	 */
        v_insscb(tsidx,newf);
        pft_pagein(newf) = 1;

        COPYPAGE(sidio,pnoio,ssid,spno);

	/* Insert page at normal address.
	 * This also clears the modified bit so we need to turn it back on.
	 */
        pft_pagein(newf) = 0;
	P_ENTER(IODONE,tsid,tpno,newf,pft_key(newf),pft_wimg(newf));
        pft_inuse(newf) = 1;
	SETMOD(newf);

	return(0);
}
