/* @(#)89       1.14  src/bos/kernel/vmm/POWER/mplock.h, sysvmm, bos41J, 9511A_all 3/7/95 18:49:58 */
#ifndef _h_MPLOCK
#define _h_MPLOCK

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#ifdef _POWER_MP
#include <sys/lock_def.h>
#include <sys/m_intr.h>  /* for INTMAX */
#include <sys/syspest.h> /* for assert and ASSERT */
#include <sys/ppda.h>
#endif /* _POWER_MP */

/*
 * MP lock macros
 *
 * Macros are defined for handling the MP-safe lock, the SWHASH lock,
 * and various MP-efficient locks and operations.  These macros are
 * defined for all flavors (UP, MP-safe, MP-efficient) so that they
 * may be used in the code without surrounding MP #ifdef's.
 */

#ifndef _POWER_MP
/*
#########################################################################
########################### UP DEFINITIONS ##############################
#########################################################################
*/

#undef _VMM_MP_SAFE
#undef _VMM_MP_EFF

/*
 * These macros are only used within critical sections
 * and so no further serialization is required on UP.
 */
#define FETCH_AND_ADD(w,inc)		((w) += (inc))
#define FETCH_AND_ADD_R(w,inc,r)	{r = (w); (w) += (inc);}
#define FETCH_AND_LIMIT(w,inc,lim)	( (w) - (inc) > (lim) ?	\
						(w) -= (inc), 1 : 0 )

#define GET_RESERVATION(ppda,need)
#define USE_RESERVATION(ppda)

#define VMM_MPSAFE_LOCK()
#define VMM_MPSAFE_UNLOCK()
#define VMM_MPSAFE_SLOCK()
#define VMM_MPSAFE_SUNLOCK()
#define VMM_MPSAFE_LOCKHOLDER()

#define SWHASH_MPLOCK(pri,sid,pno)
#define SWHASH_MPUNLOCK(pri,sid,pno)
#define SWHASH_INT_MPLOCK(sid,pno)
#define SWHASH_INT_MPUNLOCK(sid,pno)

#define	PMAP_NFR_MPLOCK(x)
#define	PMAP_NFR_MPLOCK_TRY(x)	1
#define	PMAP_NFR_MPUNLOCK(x)
#define	PMAP_PTEG_MPLOCK(s,p)
#define	PMAP_PTEG_MPUNLOCK(s,p)
#define PMAP_NFR_LOCKHOLDER(x)
#define PMAP_PTEG_LOCKHOLDER(s,p)
#define	TLB_MPLOCK()
#define	TLB_MPUNLOCK()

#define VMM_LOCK(x,y)
#define VMM_LOCK_S(x,y)			
#define VMM_UNLOCK(x)
#define VMM_UNLOCK_S(x)
#define VMM_LOCKHOLDER(x)

#define VMM_CHKNOLOCK()
#define VMM_UNLOCKALL()
#define VMM_UNLOCKMOST(x)

#define	CAREFUL_UPDATE()

/*
 * The following macros should not be used directly without
 * a surrounding #ifdef _VMM_MP_EFF.  They still need to be
 * defined however because they are used indirectly in macros
 * that don't require the surrounding ifdef.
 */
#define VMM_LOCK_TRY(x,y)
#define VMM_LOCK_TRY_S(x,y)
#define VMM_LOCKMINE(x)


#else /* _POWER_MP */
/*
#########################################################################
############# DEFINITIONS COMMON BETWEEN MP-safe AND MP-eff #############
#########################################################################
*/

/*
 * Default MP code is MP-efficient.
 */
#ifndef _VMM_MP_SAFE
#define  _VMM_MP_EFF
#else
#undef  _VMM_MP_EFF
#endif

/*
 * Generic cache-aligned lock structure.
 * Assumes maximum cache line size is 64 bytes.
 */
struct cache_line_lock {
	Simple_lock	_lock;
	int		dummy[15];
};

/*
 * SWHASH lock
 *
 * Used to protect insertion/deletion/lookup of s/w hash chains.
 * Also used to synchronize page table reload operations with
 * page-replacement operations to examine referenced status.
 * It is acquired both from VMM critical sections as well as
 * from interrupt handlers and so must be acquired disabled.
 * The "INT" form of the macros is used by v_reload() where
 * interrupts are already disabled.  The lock must be taken
 * recursively due to a page-fault (and reload) while already
 * holding the lock.
 * Note that reload must test pft state under this lock to serialize
 * corrrectly with operations in v_relframe.
 */
#define NUM_SWHASH_LOCKS	64	/* Must be a power-of-2 */
#define SWHASH_LOCK_HASH(sid,pno)	(((sid) ^ (pno)) & (NUM_SWHASH_LOCKS-1))

#define SWHASH_MPLOCK(pri,sid,pno)					\
	{								\
		pri = disable_ints();					\
		rsimple_lock(&swhash_lock[SWHASH_LOCK_HASH(sid,pno)]._lock);\
	}

#define SWHASH_MPUNLOCK(pri,sid,pno)					\
	{								\
		rsimple_unlock(&swhash_lock[SWHASH_LOCK_HASH(sid,pno)]._lock);\
		enable_ints(pri);					\
	}

#define SWHASH_INT_MPLOCK(sid,pno)					\
	{								\
		rsimple_lock(&swhash_lock[SWHASH_LOCK_HASH(sid,pno)]._lock);\
	}

#define SWHASH_INT_MPUNLOCK(sid,pno)					\
	{								\
		rsimple_unlock(&swhash_lock[SWHASH_LOCK_HASH(sid,pno)]._lock);\
	}

/*
 * PMAP_xxx locks
 *
 * Used to serialize machine-dependent page table operations.
 * The PMAP_NFR lock serializes access to the PVT and PVLIST
 * data structures and also ensures that any PTEs for a frame
 * are not evicted.  The PMAP_PTEG lock serializes insert/delete
 * operations for a particular PTE group (both primary and
 * secondary hash groups are serialized using the same lock).
 * These locks are acquired with interrupts disabled and
 * translation off.  The PMAP_PTEG lock does not need to be
 * recursive but we use the rsimple_lock/unlock service anyway
 * to avoid having to define non-recursive versions of these services.
 * The PTEG_LOCK_HASH must ensure that the primary and secondary hash
 * groups for a particular (sid,pno) map to the same lock.  It does this
 * by using the alternate hash formula for all primary hashes which map
 * to PTE groups beyond the mid-point "(vmker.hashmask+1)/2".
 */
#define NUM_NFR_LOCKS		64	/* Must be a power-of-2 */
#define NFR_LOCK_HASH(nfr)	((nfr) & (NUM_NFR_LOCKS-1))

#define NUM_PTEG_LOCKS		64	/* Must be a power-of-2 */
#define PTEG_LOCK_HASH(sid,pno)						\
		((((sid) ^ (pno)) & vmker.hashmask) < (vmker.hashmask+1)/2 ?\
			((sid) ^ (pno)) & (NUM_PTEG_LOCKS-1) :		\
			~((sid) ^ (pno)) & (NUM_PTEG_LOCKS-1))

#define PMAP_NFR_MPLOCK(nfr)						\
	{								\
		rsimple_lock(&pmap_nfr_lock[NFR_LOCK_HASH(nfr)]._lock);	\
	}

#define PMAP_NFR_MPLOCK_TRY(nfr)					\
		rsimple_lock_try(&pmap_nfr_lock[NFR_LOCK_HASH(nfr)]._lock)

#define PMAP_NFR_MPUNLOCK(nfr)						\
	{								\
		rsimple_unlock(&pmap_nfr_lock[NFR_LOCK_HASH(nfr)]._lock);\
	}

#define PMAP_PTEG_MPLOCK(s,p)						\
	{								\
		rsimple_lock(&pmap_pteg_lock[PTEG_LOCK_HASH(s,p)]._lock);\
	}

#define PMAP_PTEG_MPUNLOCK(s,p)						\
	{								\
		rsimple_unlock(&pmap_pteg_lock[PTEG_LOCK_HASH(s,p)]._lock);\
	}

#ifdef DEBUG

#define PMAP_NFR_LOCKHOLDER(nfr)					\
	{								\
		P_ASSERT((*(int *)&pmap_nfr_lock[NFR_LOCK_HASH(nfr)] & ~RECURSION) ==	\
			     (((CPUID) << 1) | LOCKBIT));		\
	}

#define PMAP_PTEG_LOCKHOLDER(s,p)					\
	{								\
		P_ASSERT(*(int *)&pmap_pteg_lock[PTEG_LOCK_HASH(s,p)] == \
			     (((CPUID) << 1) | LOCKBIT));		\
	}

#else /* DEBUG */

#define PMAP_NFR_LOCKHOLDER(nfr)
#define PMAP_PTEG_LOCKHOLDER(s,p)

#endif /* DEBUG */

/*
 * TLB lock
 *
 * The TLB lock is used implicitly when invalidating the TLB via
 * the invtlb() service.  These macros should be used when something
 * other than invtlb() is used to invalidate TLBs (such as during vmsi).
 */
#define	TLB_MPLOCK()	rsimple_lock(&tlb_lock._lock)
#define	TLB_MPUNLOCK()	rsimple_unlock(&tlb_lock._lock)


#ifdef _VMM_MP_SAFE
/*
#########################################################################
########################## MP-safe DEFINITIONS ##########################
#########################################################################
*/

/*
 * These macros are only used within critical sections
 * under the MP-safe lock and so no additional
 * serialization is required for MP-safe.
 */
#define FETCH_AND_ADD(w,inc)		((w) += (inc))
#define FETCH_AND_ADD_R(w,inc,r)	{r = (w); (w) += (inc);}
#define FETCH_AND_LIMIT(w,inc,lim)	( (w) - (inc) > (lim) ?	\
						(w) -= (inc), 1 : 0 )

#define GET_RESERVATION(ppda,need)
#define USE_RESERVATION(ppda)

/*
 * MP-safe lock
 *
 * This lock is held around each vmm critical section to preserve
 * atomicity.  The lock is hand-crafted to support waiting at
 * interrupt level (v_wait/v_ready).  The lockword is manipulated
 * under the protection of the simple lock 'vmm_lock_lock' which
 * is also used in conjunction with the adspace lock.
 */
extern Simple_lock vmm_lock_lock;
extern struct vmmlock vmm_mpsafe_lock;

/*
 * Since the mpsafe lock is blocking lock, we return if the lock is held,
 * enqueued thru v_wait on the wait list associated with the lock.
 */
#define VMM_MPSAFE_LOCK()	\
	if (v_vmm_lock(&vmm_mpsafe_lock,0) == VM_WAIT) return VM_WAIT

/*
 * In the page fault handler, we may have to release the adspace lock
 * before exiting.  Thus the return has to be done in the routines.
 */
#define VMM_MPSAFE_PFLOCK() v_vmm_lock(&vmm_mpsafe_lock,0)

/*
 * Lock try -- do not block nor spin.
 * This routine does allow recursive locking.
 */
#define VMM_MPSAFE_LOCKTRY()	v_vmm_locktry(&vmm_mpsafe_lock)

/*
 * We wait until the end of a VMM critical section to start I/O's.
 * For MP-safe it is convenient to use the unlock of the MP-safe lock
 * to perform the duties of start I/O.  The routine v_pfsio always
 * returns with the MP-safe lock released.
 */
#define VMM_MPSAFE_UNLOCK()	v_pfsio()
	
/*
 * VMM critical sections that cannot block or that treat VM_WAIT
 * in a special manner must take the MP-safe lock as a spin lock.
 */
#define VMM_MPSAFE_SLOCK()	v_vmm_slock(&vmm_mpsafe_lock)

/*
 * Simple unlock w/o start I/O.  This is used by v_pfsio() to
 * release the MP-safe lock.
 */
#define VMM_MPSAFE_SUNLOCK()	v_vmm_unlock(&vmm_mpsafe_lock,0)

/*
 * Lock mine
 * Returns 1 if the caller holds the lock, 0 otherwise.
 */
#define VMM_MPSAFE_LOCKMINE() v_vmm_lockmine(&vmm_mpsafe_lock)

/*
 * Lock holder
 * Assert that the caller holds the lock.
 */
#define VMM_MPSAFE_LOCKHOLDER() ASSERT(VMM_MPSAFE_LOCKMINE());

#define VMM_LOCK(x,y)
#define VMM_LOCK_S(x,y)			
#define VMM_UNLOCK(x)
#define VMM_UNLOCK_S(x)
#define VMM_LOCKHOLDER(x)

#define VMM_CHKNOLOCK()
#define VMM_UNLOCKALL()
#define VMM_UNLOCKMOST(x)

#define	CAREFUL_UPDATE()

/*
 * The following macros should not be used directly without
 * a surrounding #ifdef _VMM_MP_EFF.  They still need to be
 * defined however because they are used indirectly in macros
 * that don't require the surrounding ifdef.
 */
#define VMM_LOCK_TRY(x,y)
#define VMM_LOCK_TRY_S(x,y)
#define VMM_LOCKMINE(x)

#endif /* _VMM_MP_SAFE */

#ifdef _VMM_MP_EFF
/*
#########################################################################
########################## MP-eff DEFINITIONS ###########################
#########################################################################
*/

/*
 * For MP-eff, disabled critical sections are not sufficient
 * to protect updates and so atomic updates must be used.
 */
#define FETCH_AND_ADD(w,inc)		fetch_and_add(&(w),(inc))
#define FETCH_AND_ADD_R(w,inc,r)	r = fetch_and_add(&(w),(inc))
#define FETCH_AND_LIMIT(w,inc,lim)	fetch_and_limit(&(w),(inc),(lim))

/*
 * For MP-eff, free page frames are reserved by v_spaceok,
 * and used by v_getframe.  Reservations are not cumulative,
 * ie. any call to GET_RESERVATION must include any previous
 * reservation that is still required.
 */
#define GET_RESERVATION(ppda,need)					\
	{								\
		vmker.numfrb += -(need - ppda->ppda_reservation);	\
		ppda->ppda_reservation = need;				\
	}

#define USE_RESERVATION(ppda)					\
	{							\
		assert(ppda->ppda_reservation > 0);		\
		ppda->ppda_reservation += -1;			\
	}

#ifndef _VMMDEBUG

#define VMM_MPSAFE_LOCK()
#define VMM_MPSAFE_UNLOCK()

#else /* _VMMDEBUG */

/*
 * For debug we use the MP-safe lock and unlock to
 * do some sanity checking at the beginning and end
 * of every critical section.
 */
#define VMM_MPSAFE_LOCK()				\
	{						\
		v_chknolock();				\
		assert(PPDA->ppda_sio == 0);		\
	}

#define VMM_MPSAFE_UNLOCK()				\
	{						\
		v_chknolock();				\
	}

#endif /* _VMMDEBUG */

#define VMM_MPSAFE_SLOCK()	VMM_MPSAFE_LOCK()
#define VMM_MPSAFE_SUNLOCK()
#define VMM_MPSAFE_LOCKMINE() 0	
#define VMM_MPSAFE_LOCKHOLDER() 

/*
 * All MP-eff locks use VMM_LOCK/VMM_UNLOCK.
 *
 * The VMM_LOCK_S/VMM_UNLOCK_S macros are used when a page-fault
 * and backtrack may occur so that the lock is scoreboarded and
 * can be released.
 */

#ifndef _VMMDEBUG

#define VMM_LOCK(x,y)		simple_lock(x)
#define VMM_UNLOCK(x)		simple_unlock(x)
#define VMM_LOCK_TRY(x,y)	simple_lock_try(x)
#define VMM_LOCK_TRY_S(x,y)					\
	(							\
		simple_lock_try(x) ? v_scoreboard(x),1 : 0	\
	)				
#define VMM_LOCK_S(x,y)			\
	{				\
		simple_lock(x);		\
		v_scoreboard(x);	\
	}				
#define VMM_UNLOCK_S(x)			\
	{				\
		v_descoreboard(x);	\
		simple_unlock(x);	\
	}				
#define VMM_UNLOCKALL()		v_descoreboard_all()
#define VMM_UNLOCKMOST(x)	v_descoreboard_most(x)
#define VMM_CHKNOLOCK()

#else  /* _VMMDEBUG */

/*
 * VMMNUMLOCKS > vmm number of MP-eff locks
 */
#define VMMNUMLOCKS	12
#ifdef _SLICER
extern ulong slicer_msts_init;   
#endif

/*
 * Debugging structures
 */
extern char scoreboard_file[][VMM_MAXLOCK][256];
extern int  scoreboard_line[][VMM_MAXLOCK];
extern int  scoreboard_lock[][VMM_MAXLOCK];
extern int  scoreboard_vertex[VMMNUMLOCKS][VMMNUMLOCKS];
extern char *vmm_locks[];
extern int  v_backt[][16];
extern int  v_backt_save[][16];

#ifndef _SLICER
#define VMM_LOCK_S(x,y)			\
	{				\
		simple_lock(x);		\
		v_scoreboard(x, __LINE__, __FILE__,y);	\
	}				
#else /* _SLICER */
extern ulong slicer_msts_init;
extern int pseudo_count;
extern ulong slicer_wait_proc_init;
#define VMM_LOCK_S(x,y)						\
	{							\
		if(!slicer_msts_init || slicer_wait_proc_init != MAXCPU)  \
		{						\
			simple_lock(x);				\
			v_scoreboard(x, __LINE__, __FILE__,y);	\
		}						\
		else 	{					\
			while (!simple_lock_try(x))			\
			{						\
				pseudo_count = pseudo_count + 1;	\
				pseudo_slicer_int();  			\
			}						\
			v_scoreboard(x, __LINE__, __FILE__,y);  	\
			pseudo_count = pseudo_count + 1;		\
			pseudo_slicer_int();	   			\
		}							\
	}
#endif /* _SLICER */

#define VMM_UNLOCK_S(x)						\
	{							\
		v_descoreboard(x, __LINE__, __FILE__);		\
		simple_unlock(x);				\
	}				
#define VMM_LOCK_TRY_S(x,y)					\
	(							\
		simple_lock_try(x) ?				\
		v_scoreboard(x, __LINE__, __FILE__,y),1 : 0	\
	)				
#define VMM_LOCK(x,y)						\
	{							\
		if(v_backt[CPUID][0] == 0)			\
		{						\
			v_backt_save[CPUID][0] = CSA->backt;	\
			CSA->backt = 0;				\
		}						\
		v_backt[CPUID][0] += 1;				\
		VMM_LOCK_S(x,y);				\
	}
#define VMM_UNLOCK(x)						\
	{							\
		VMM_UNLOCK_S(x);				\
		v_backt[CPUID][0] -= 1;				\
		if(v_backt[CPUID][0] == 0)			\
			CSA->backt = v_backt_save[CPUID][0];	\
	}
#define VMM_LOCK_TRY(x,y)	VMM_LOCK_TRY_S(x,y)
#define VMM_UNLOCKALL()		v_descoreboard_all(__LINE__, __FILE__)
#define VMM_UNLOCKMOST(x)	v_descoreboard_most(x, __LINE__, __FILE__)
#define VMM_CHKNOLOCK()		v_chknolock()

#endif /* _VMMDEBUG */

#define	CAREFUL_UPDATE()

#define VMM_LOCKMINE(x)		lock_mine(x)
#define VMM_LOCKHOLDER(x)	ASSERT(VMM_LOCKMINE(x))

#endif /* _VMM_MP_EFF */

#endif /* _POWER_MP */

/*
 * MP-eff per-data structure locks
 */

#define SCB_MPLOCK(x)                   VMM_LOCK(&scb_lock(x),1)
#define SCB_MPLOCK_S(x)                 VMM_LOCK_S(&scb_lock(x),1)
#define SCB_MPUNLOCK(x)                 VMM_UNLOCK(&scb_lock(x))
#define SCB_MPUNLOCK_S(x)               VMM_UNLOCK_S(&scb_lock(x))
#define SCB_MPLOCK_TRY(x)               VMM_LOCK_TRY(&scb_lock(x),1)
#define SCB_MPLOCK_TRY_S(x)             VMM_LOCK_TRY_S(&scb_lock(x),1)
#define SCB_LOCKHOLDER(x)               VMM_LOCKHOLDER(&scb_lock(x))
#define SCB_LOCKMINE(x)                 VMM_LOCKMINE(&scb_lock(x))

#define AME_MPLOCK()                    VMM_LOCK(&ame_lock,2)
#define AME_MPUNLOCK()                  VMM_UNLOCK(&ame_lock)
#define AME_MPLOCK_S()                  VMM_LOCK_S(&ame_lock,2)
#define AME_MPUNLOCK_S()                VMM_UNLOCK_S(&ame_lock)
#define AME_LOCKHOLDER()                VMM_LOCKHOLDER(&ame_lock)

#define VMAP_MPLOCK()                   VMM_LOCK(&vmap_lock,3)
#define VMAP_MPUNLOCK()                 VMM_UNLOCK(&vmap_lock)
#define VMAP_MPLOCK_S()                 VMM_LOCK_S(&vmap_lock,3)
#define VMAP_MPUNLOCK_S()               VMM_UNLOCK_S(&vmap_lock)
#define VMAP_LOCKHOLDER()               VMM_LOCKHOLDER(&vmap_lock)

#define FS_MPLOCK(x)                   VMM_LOCK(&lv_lock(x),4)
#define FS_MPUNLOCK(x)                 VMM_UNLOCK(&lv_lock(x))
#define FS_MPLOCK_S(x)                 VMM_LOCK_S(&lv_lock(x),4)
#define FS_MPUNLOCK_S(x)               VMM_UNLOCK_S(&lv_lock(x))
#define FS_LOCKHOLDER(x)               VMM_LOCKHOLDER(&lv_lock(x))

#define ALLOC_MPLOCK()                  VMM_LOCK(&alloc_lock,5)
#define ALLOC_MPUNLOCK()                VMM_UNLOCK(&alloc_lock)
#define ALLOC_MPLOCK_S()                VMM_LOCK_S(&alloc_lock,5)
#define ALLOC_MPUNLOCK_S()              VMM_UNLOCK_S(&alloc_lock)
#define ALLOC_LOCKHOLDER()              VMM_LOCKHOLDER(&alloc_lock)

#define LW_MPLOCK()                     VMM_LOCK(&lw_lock,6)
#define LW_MPUNLOCK()                   VMM_UNLOCK(&lw_lock)
#define LW_MPLOCK_S()                   VMM_LOCK_S(&lw_lock,6)
#define LW_MPUNLOCK_S()                 VMM_UNLOCK_S(&lw_lock)
#define LW_LOCKHOLDER()                 VMM_LOCKHOLDER(&lw_lock)

#define PDT_MPLOCK()                    VMM_LOCK(&pdt_lock,7)
#define PDT_MPUNLOCK()                  VMM_UNLOCK(&pdt_lock)
#define PDT_LOCKHOLDER()                VMM_LOCKHOLDER(&pdt_lock)

#define PG_MPLOCK(x)                   VMM_LOCK(&lv_lock(x),8)
#define PG_MPUNLOCK(x)                 VMM_UNLOCK(&lv_lock(x))

#define RPT_MPLOCK()                    VMM_LOCK(&rpt_lock,9)
#define RPT_MPUNLOCK()                  VMM_UNLOCK(&rpt_lock)

#define APT_MPLOCK()                    VMM_LOCK(&apt_lock,10)
#define APT_MPUNLOCK()                  VMM_UNLOCK(&apt_lock)

#define VMKER_MPLOCK()                  VMM_LOCK(&vmker_lock,11)
#define VMKER_MPUNLOCK()                VMM_UNLOCK(&vmker_lock)
#define VMKER_LOCKHOLDER()              VMM_LOCKHOLDER(&vmker_lock)

#endif /* _h_MPLOCK */
