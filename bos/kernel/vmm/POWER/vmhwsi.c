static char sccsid[] = "@(#)81	1.20  src/bos/kernel/vmm/POWER/vmhwsi.c, sysvmm, bos412, 9445C412a 10/25/94 11:20:25";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vmhwinit
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

#include "vmsys.h"
#include "vmsidata.h"
#include <sys/inline.h>
#include "mplock.h"

/*
 * vmhwinit:
 * initialize virtual memory hardare
 *
 * inputs: size and location of page table (vmsidata)
 *	   storage underlying page table set to zeros.
 *
 * the following are initialized.
 *
 *	(1) storage description registers (SDR0 and SDR1)
 *	(2) tlb is purged.
 *	(3) segment registers - invalid except 0 to address kernel.
 *
 */

#define BCRL	0x4e800021

vmhwinit_pwr(vp)
struct vmsidata *vp;
{

	struct	rs1pft *rs1pft;
	struct	rs2pte *rs2pte;
	uint	hatmask, save, *ptr, htabmask;
	int	firstp, p, *ptrhat, tlbsize;

	/*
	 * initialize SDR's
	 */

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1|POWER_RSC))
	{
		ldsdr0(vmrmap_raddr(RMAP_PFT));
		hatmask = (1 << (vmker.hashbits - 13)) - 1;
		ldsdr1((vmrmap_raddr(RMAP_HAT) << (16 - 15)) | hatmask);
	}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
	if (__power_rs2())
	{
		/*
		 * Lock table is not used, just set SDR0 to zero.
		 */
		ldsdr0(0);
		htabmask = (1 << (vmker.hashbits - 11)) - 1;
		ldsdr1(vmrmap_raddr(RMAP_PFT) | htabmask);
	}
#endif	/* _POWER_RS2 */

	/*
	 * Load TID with a value not likely to match random memory contents
	 * so if the special bit is ever set in a segment register we will
	 * take a lock fault and halt the system.
	 */
	ldtid(0xDEADDEAD);

	/*
	 * initialize segment registers.
	 */
	(void)chgsr(0,SRVAL(0,0,0));  /* sid = 0, key 0, not special */
	for(p = 1; p <= 15; p++)
	{
		(void)chgsr(p,SRVAL(INVLSID,0,0));  /* invalid sid */
	}

	/*
	 * purge tlb. this must be done by software trickery since
	 * hardware doesn't directly support function. method is to
	 * force tlb to be loaded with known page translation and
	 * then to purge the tlb entries for them.
	 */
	firstp = (vmrmap_size(RMAP_KERN) >> L2PSIZE) + 1;
#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1|POWER_RSC))
	{
		ptrhat = (int *)vmrmap_raddr(RMAP_HAT);
		rs1pft = (struct rs1pft *) vmrmap_raddr(RMAP_PFT);
	}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
	if (__power_rs2())
	{
		rs2pte = (struct rs2pte *) vmrmap_raddr(RMAP_PFT);
	}
#endif	/* _POWER_RS2 */

	/*
	 * set up entries in page table so that the interval
	 * (0,firstp) is addressed V=R. we assume that firstp
	 * is less than 2**13 so that the avpi field in the
	 * page table entries can be left as zero.
	 *
	 */
	for(p = 0; p < firstp ; p++)
	{
#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if (__power_set(POWER_RS1|POWER_RSC))
		{
			*(ptrhat + p) = p;
			(rs1pft + p)->u1.s1._valid = 1;
		}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
		if (__power_rs2())
		{
			(rs2pte + (p<<L2PTEGSIZE))->u1.s1._valid = 1;
			(rs2pte + (p<<L2PTEGSIZE))->u2.s2._rpn = p;
		}
#endif	/* _POWER_RS2 */
	}

	/*
	 * set up entries so that (firstp, firstp + tlbsize - 1)
	 * ALL point to page zero. thus all references to addresses
	 * in this range will translate to page 0 when xlate is enabled.
	 * determine maximum tlbsize from system configuration structure.
	 */
	tlbsize = MAX(_system_configuration.itlb_size *
			_system_configuration.itlb_asc,
			_system_configuration.dtlb_size *
			_system_configuration.dtlb_asc);

	for (p = firstp; p < firstp + tlbsize; p++)
	{
#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if (__power_set(POWER_RS1|POWER_RSC))
		{
			*(ptrhat + p) = 0;
		}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
		if (__power_rs2())
		{
			(rs2pte + (p<<L2PTEGSIZE))->u1.s1._valid = 1;
		}
#endif	/* _POWER_RS2 */
	}

	/*
	 * make a bcrl instruction at location zero.
	 */
	ptr = (uint *) 0;
	save = *ptr;
	*ptr = BCRL;
	vm_cflush(ptr,sizeof(uint));	/* flush the cache line */

	/*
	 * purge tlb from 0 to firstp + tlbsize - 1
	 */
	for (p = 0; p < firstp + tlbsize  ; p++) {
		tlbie((caddr_t)(p*PSIZE));
	}

	/*
	 * cause both instruction and data tlbs to be loaded
	 * with (firstp,firstp + tlbsize - 1). in the loop
	 * hyperbranch(x) is a branch to location x which translates
	 * to the bcrl at real location zero which returns.
	 */
	xlateon();
	for (p = firstp; p < firstp + tlbsize; p++)
	{
		vm_cflush(p*PSIZE, PSIZE);
		tlbie((caddr_t)(p*PSIZE));
		vm_cflush(p*PSIZE, PSIZE);
		hyperbranch(p*PSIZE);
	}
	xlateoff();

	/*
	 * purge tlb from 0 to firstp + tlbsize - 1
	 * restore word at location zero and hat and pft
	 */
	*ptr = save;
	for (p = 0; p < firstp + tlbsize; p++)
	{
		tlbie((caddr_t)(p*PSIZE));
#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if (__power_set(POWER_RS1|POWER_RSC))
		{
			*(ptrhat + p) = 0;
			(rs1pft + p)->u1.s1._valid = 0;
		}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
		if (__power_rs2())
		{
			(rs2pte + (p<<L2PTEGSIZE))->u1.s1._valid = 0;
			(rs2pte + (p<<L2PTEGSIZE))->u2.s2._rpn = 0;
		}
#endif	/* _POWER_RS2 */
	}
}

#ifdef	_POWER_PC
vmhwinit_ppc(vp)
struct vmsidata *vp;
{
	uint	htabmask, pno;
	int	p, tlbsize;

	/*
	 * Initialize SDR1.
	 */
	htabmask = (1 << (vmker.hashbits - 10)) - 1;
	ldsdr1(vmrmap_raddr(RMAP_PFT) | htabmask);

	/*
	 * Initialize segment registers.
	 */
	(void)chgsr(0,SRVAL(0,0,0));  /* sid = 0, key 0 */
	for(p = 1; p <= 15; p++)
	{
		(void)chgsr(p,SRVAL(INVLSID,0,0));  /* invalid sid */
	}

	/*
	 * Invalidate the entire TLB for the local Master Processor.
	 * This code does assume that the TLB invalidate entry
	 * instruction works by index and invalidates
	 * both entries in the congruence class.
	 *
	 * A sync instruction is required after the gang-invalidate
	 * of the TLB in order to ensure that all of the preceding
	 * "tlbie" instructions, themselves are completed now, before 
	 * relocate gets turned on. Note that 'sync' does not kill
	 * prefetching, so this does not preclude the possibility of
	 * fetching stale data from an old TLB entry. Since relocate
	 * is off, however, here, that is not an issue, and 'mtmsr'
	 * which will turn relocate on later is execute synchronizing,
	 * this is ok.
	 */
	tlbsize = MAX(_system_configuration.itlb_size,
		      _system_configuration.dtlb_size);
	for (pno = 0; pno < tlbsize; pno++)
		tlbie((caddr_t)(pno*PSIZE));
	__iospace_sync();


	/*
	 * For the master processor, the following code is really
	 * probably not necessary, since no other processor is running,
	 * but it is in for defensiveness. See comments in vm_bs_proc()
	 * for what this does.
	 */
	if (__power_mp() && !__power_601())
	{
	        tlbsync();
		__iospace_sync();
	}    

}
#endif	/* _POWER_PC */

#ifdef _POWER_MP

/*
 * NAME: vm_bs_proc
 *
 * FUNCTION: Boot-slave processor VMM initialization
 *
 *	Initialize VMM associated registers
 *	(1) storage description registers (SDR1)
 *	(2) segment registers - invalid except KERN and KERNX to address kernel.
 *	(3) tlb is purged.
 *      Translation on.
 *
 *
 * NOTES:
 *      This code only works for PowerPC processors.
 *      This code should never be called on a UP.
 *      
 *      POWER processors require a different, and much more complex
 *      tlb invalidation mechanism, since there is no direct way on POWER
 *      to invalidate the instruction TLB, for example.
 *      Slave POWER CPUs are not supported in this code.
 *
 *
 * INPUT: 
 *
 * EXECUTION ENVIRONMENT:
 *	Runs on a special stack: start_bs_stk (set up in start_bs_proc)
 *
 * RETURNS:
 */
vm_bs_proc()
{
	uint	htabmask, pno;
	int	p, tlbsize;
	extern struct cache_line_lock tlb_lock;

	/* Don't need to do the following when SLICER is enabled.  Hardware
	 * has already been initialized */
#ifndef _SLICER
	/*
	 * Initialize SDR1.
	 */
	htabmask = (1 << (vmker.hashbits - 10)) - 1;
	ldsdr1(vmrmap_raddr(RMAP_PFT) | htabmask);

	/*
	 * Initialize segment registers.
	 */
	(void)chgsr(KERNELSEG, vmker.kernsrval);
	for(p = 1; p <= 15; p++) {
		(void)chgsr(p,SRVAL(INVLSID,0,0));  /* invalid sid */
	}
        (void)chgsr(KERNEXSEG, vmker.kexsrval);


	/*
	 * Invalidate the entire TLB for each Slave Processor.
	 * This code does assume that the TLB invalidate entry
	 * instruction works by index and invalidates
	 * both entries in the congruence class.
	 * A sync instruction is required after the gang-invalidate
	 * of the TLB in order to ensure that all of the preceding
	 * "tlbie" instructions, themselves are completed before any
	 * other storage ops are kicked off, which could reference stale
	 * TLB entries.
	 */
	tlbsize = MAX(_system_configuration.itlb_size,
		      _system_configuration.dtlb_size);
	TLB_MPLOCK();
	for (pno = 0; pno < tlbsize; pno++)
		tlbie((caddr_t)(pno*PSIZE));
	__iospace_sync();

	/*
	 * The following code is very defensive, but ensures architecture
	 * compliance. The slave processors are locked out from performing
	 * simultaneous tlb invalidates via the simple lock. The meaning of
	 * the 'tlbsync' instruction (and the 'sync' instruction after tlbie
	 * on 601-only) is that it should wait for all tlb-invalidates on
	 * remote processors to complete. A sync after tlbsync guarantees 
	 * completion of tlbsync itself. This code makes sure that all
	 * tlb-invalidates in the 'world' are done before unlocking the lock,
	 * and permitting another processor to perform tlb-invalidates.
	 */
	if (__power_mp() && !__power_601())
	{
	        tlbsync();
		__iospace_sync();
	}    
	TLB_MPUNLOCK();
#endif /* _SLICER */

#if defined(_KDB)
	kdb_init_vmm(1);
#endif /* _KDB */

	xlateon();

#if defined(_KDB)
	kdb_init_vmm(2);
#endif /* _KDB */
}
#endif	/* _POWER_MP */
