static char sccsid[] = "@(#)71	1.70.1.33  src/bos/kernel/vmm/POWER/vmsi.c, sysvmm, bos41B, 9506B 1/25/95 15:13:11";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vmsi
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

#include "vmsys.h"
#include "vmsidata.h"
#include "vmvars.h"
#include <sys/bootsyms.h>
#include <sys/iplcb.h>
#include <sys/seg.h>
#include <sys/machine.h>
#include <sys/var.h>
#include <sys/inline.h>
#include <sys/overlay.h>
#include <sys/syspest.h>
#include "mplock.h"

#include <sys/lock_alloc.h>
#include <sys/lockname.h>
extern Simple_lock  vmm_lock_lock;

extern uint g_ksrval;
extern uint g_kxsrval;

/*
 * initialization of virtual memory manager.
 * entry point is vmsi which is called during system init.
 *
 * inputs : the following are either explicit inputs to
 *          vmsi or must be calculable in vmsi itself.
 *
 *   (1) RMAP_KERN : the address range of the module loaded
 *       by ipl which includes the loader tables but not
 *       the ram disk which follows the module. real memory
 *       is made addessable at the same virtual address
 *	 (thus this memory is assumed to be contiguous good).
 *
 *   (2) RMAP_RAMD : the ram disk is also loaded by ros ipl
 *       and follows the module. its address is assumed to be
 * 	 on a page boundary in memory the good pages are moved
 * 	 to a separate segment which can be addressed with the
 *	 segment register value stored in vmker.ramdsrval.
 *
 *   (3) RMAP_BCFG : the base config data is also loaded by ros ipl
 *       and follows the ram disk. its address is assumed to be
 * 	 on a page boundary in memory the good pages are moved
 * 	 to a separate segment which can be addressed with the
 *	 segment register value stored in vmker.ramdsrval.
 *
 *   (4) RMAP_IPLCB : the ipl control block which is assumed to
 *	 be left in contiguous good memory by ros ipl code.
 *       this is expected to be disjoint from [0,endload).
 *       on exit from vmsi the ipl control is addressable
 *       after endload, at an address stored in vmker.iplcbptr.
 *
 *   (5) RMAP_MST : the address range for the machine state save areas.
 *
 *   (6) page fix : the subset of [0,endload) which must be
 *       page fixed. this is specified by up to 16 intervals
 *       of the form [first, last] where first and last are
 *       the byte addresses of the beginning and end of each
 *       interval.
 *
 *   (7) fixed common: the address of the beginning and end
 *       of common that needs to be page fixed.
 *
 *   (8) real storage size : this is specified by the ram bit
 *       bit map computed by the POST code.
 *
 *   (9) bad memory blocks : these are specified by the same
 *       ram bit map computed by the POST code.
 *
 *  (10) initial paging device : size of initial allocation
 *       disk map for first paging device.
 *
 * execution environment:
 *
 *    this code is called with address translate disabled but
 *    otherwise enabled for interrupts. during its execution
 *    address translation is enabled and on exit all VMM services
 *    are available.
 *
 * machine hardware dependencies:
 *
 *    the file vmhwsi contains the routines required for initialization
 *    of the memory management hardware.  other machine dependencies within
 *    this file are #ifdef'd.
 *
 */

/*
 * main entry point of vmm system init code.
 */

/*
 * INITSIZE is the size of the user region.
 */
#define INITKEY         KERKEY
#define INITTYPE        V_WORKING | V_SYSTEM
#define INITSIZE        0

void vmsi()
{
	struct vmsidata si;
	struct ipl_cb *iplcb_ptr;
	struct ipl_info *iinfo_ptr;
	struct ipl_directory *idir_ptr;

	/*
	 * ram bit map
	 */
	iplcb_ptr = (struct ipl_cb *)ipl_cb;
	idir_ptr = (struct ipl_directory *)(&iplcb_ptr->s0);
	si.rmapptr  = (uint)ipl_cb + idir_ptr->bit_map_offset;
	si.rmapsize = (int) idir_ptr->bit_map_size / 4;
	iinfo_ptr = (struct ipl_info *)
		    ((uint)ipl_cb + idir_ptr->ipl_info_offset);
	si.rmapblk  = iinfo_ptr->bit_map_bytes_per_bit;

	/*
	 * fix intervals:
	 *   All of the kernel and machine-specific fix intervals are put
	 *   into the VMINT_FIXMEM array by hardinit.
	 * fixed common:
	 *   All ranges established in the VMINT_FIXCOM array by hardinit.
	 * release intervals:
	 *   Machine-specific ranges for the platform(s) we *are not* on
	 *   are placed in the VMINT_RELMEM array by hardinit.
	 */

	/*
	 * initial paging device. 16 megabytes
	 */
	si.pgdev = -1;
	si.pgsize = 4096;

	/* call vsi to do it */
	vsi(&si);
}

/*
 * vsi does the initialization after inputs have been determined.
 */

static int vsi(vp)
struct vmsidata * vp;
{

	int p,rc,sid,k;

	/*
	 * sort the page fix intervals.
	 */
	sortfix(vp);

	/*
	 * establish size of main memory and construct free list
	 * of good page frames.
	 */
	initmem(vp);

	/*
	 * calculate hash function constants and sreg values
	 * for vmmdseg, ptaseg, and paging space disk map.
	 */
	initkerdata(vp);

	/*
	 * calculate the address ranges for vmsi objects
	 */
	initranges(vp);

	/*
	 * allocate real memory to backup the ranges.
	 */
	if (rc = backreal(vp))
	{
		/*
		 * Halt with LED indicating memory shortage.
		 */
		for (;;) write_leds(HALT_NOMEM);
	}

	/*
	 * init hardware associated with VM
	 */
	vmhwinit(vp);

	/* setup srval for the kernel segment.
	 */
	ldsr(0,SRVAL(vmker.kernsrval,0,0));

	/*
	 * initialize page table.
	 */
	initpft(vp);

	/*
	 * make the ranges addressable virtually.
	 */
	backvirt(vp);

	/*
	 * transfer free page list to where VMM wants it.
	 * this is done before xlate is turned on because
	 * the free list is represented by real link list
	 * thru all of memory.
	 */
	initfree(vp);

#ifdef _POWER_RS
	if (__power_rs())
	{
		/* flush cache because of aliasing possibilities */
		vm_cflush(0,vp->memsize*PSIZE);
	}
#endif /* POWER_RS */

#if defined(_KDB)
	kdb_init_vmm(1);
#endif /* _KDB */

	/*
	 * turn on xlate. load sregs.
	 */
	xlateon();
	ldsr(VMMSR,vmker.vmmsrval);
	ldsr(PTASR,vmker.ptasrval);
	ldsr(TEMPSR,vmker.dmapsrval);
	ldsr(KERNEXSEG, vmker.kexsrval);

	/*
	 * initialize fields in vmmdseg.
	 */
	initvmdseg(vp);

	/*
	 * initialize free apm list and anchors in ptaseg.
	 */
	initapm(vp);

	/*
	 * initialize mststacks
	 */
	init_flihs();

#ifdef _SLICER
	{
		extern struct ppda *proc_arr_addr;
		extern int cur_cpu_num;
		int i;

		/* This fakes out kdb_init_slave() and vm_bs_proc()
		 * into thinking we are really on a different physical processor
		 */
		for (i = 1; i < _system_configuration.ncpus; i++) {
			ppda[i].cpuid = cur_cpu_num = i;
			proc_arr_addr = &ppda[i];
			xlateoff();
#if defined(_KDB)
			kdb_init_slave(cur_cpu_num);
#endif /* _KDB */
			vm_bs_proc();
		}

		/* Need to reset ourselves back to 0 */
		cur_cpu_num = 0;
		proc_arr_addr = &ppda[0];
	}
#endif /* _SLICER */

	/*
	 * create kernel,vmmdseg,ptaseg,diskmap and kernext seg.
	 * vms_create must work without taking a page fault.
	 */

	for (k = 0; k < 5; k++)
	{
		vms_create(&sid,INITTYPE,0,INITSIZE,SEGSIZE/2,SEGSIZE/2);
	}

	/* 
	 * initialize dmaptab array used by allocators.
	 */
	initdmaptab();

	/*
	 * finish initializing ptaseg.
	 */
	initptaseg(vp);

	/*
	 * fix up the software pft for the segments.
	 */
	initpftsw(vp);

	/*
	 * initialize paging space
	 */
	initpgspace(vp);

	/*
	 * we should now be able to take page faults.
	 * allocate xpt direct blocks for kernel and
	 * unfix pages in kernel, backing them up with
	 * disk blocks.
	 */

#if defined(_KDB)
	kdb_init_vmm(2);
#endif /* _KDB */

	 if (rc = unfixkern(vp))
		return(rc);

	/*
	 * move ram disk to its own segment
	 */
	if( rc = initramdisk(vp))
		return(rc);

	/*
	 * move base config area to its own segment
	 */
	if( rc = initbconfig(vp))
		return(rc);

	/*
	 * free memory allocated to per-frame data structures which
	 * corresponds to large memory holes.
	 */
	freerange(vp);

	/*
	 * make ipl control block addressable after endload.
	 */
	initiplcb(vp);

	/*
	 * make page 0 of kernel read-only
	 */
	vm_protect(0,PSIZE,RDONLY);

	/*
	 * pin part of common
	 */
	initcommon(vp);

	/*
	 * discard platform-dependent ranges of pinned text
	 */
	relranges(vp);

	/* Allow read access with key 1 for pages containing:
	 *   - svc interrupt entry points (0x1000- 0x1fff)
	 *   - svc handler, and other kernel code and data
	 *     that runs or is referenced with key 1
	 */
	vm_protect(PSIZE,PSIZE,UTXTKEY);
	vm_protect(&nonpriv_page,(uint)&fetchprot_page - (uint)&nonpriv_page,
		   UTXTKEY);
	vm_protect(NONPRIV_RESERV_ADDR,NONPRIV_RESERV_SIZE,UTXTKEY);

	/* initialize user kernel shadow segment. initialize
	 * storage protect key for system part of working storage
	 * segments.
	 */
	initukern();

	return(0);
}

/*
 * determine real memory size and build a free list of good
 * memory. free list consists of all good frames but NOT frames
 * assigned to ranges loaded by ROS.
 */

static initmem(vp)
struct vmsidata *vp;
{
        uint *rptr,bits,*lastptr,mask,size;
        int k,pzero,p,lastp,blksper;

	/*
	 * determine size of memory. 1's in map represent bad
	 * or non-existent memory of size vp->rmapblk. we scan
	 * map from right to left looking for first zero.
	*/
	rptr = (uint *) vp->rmapptr;  /* point to begin of map */
	rptr += vp->rmapsize - 1;   /* point to last word of map */
	size = 32*vp->rmapblk*vp->rmapsize;  /* size in bytes */
	for ( ; ; rptr += -1)
	{
		bits = *rptr;
		/* any zeros ? */
		if (bits == ONES)
		{
			size += - 32*vp->rmapblk;
			continue;
		}

		/* look for rightmost zero */
		mask = 1;
		for(k = 0; ; k += 1, mask = mask << 1) {
			if ((mask & bits) == 0)
				break;
		}

		/*
		 * express size in pages. compute rounded values
		 */
		vp->memsize = (size - k*vp->rmapblk) >> L2PSIZE;
		vp->logmsize = loground(vp->memsize);
		vp->memround = 1 << vp->logmsize;
		break;
	}

	/*
	 * build free list of good pages by scanning ram bit map
	 * left to right. list is anchored by freemem in vmsidata.
	 * list pointers are page addresses and occupy the first
	 * word in each page. the last page has a next pointer
	 * value of 0 (NULL). good pages not loaded by ipl ros and
	 * not part of the ipl control block are put on free list.
	 */
	lastp = vp->memsize - 1;
	pzero =  0;
	blksper = vp->rmapblk >> L2PSIZE;  /* pages per bit of map */
	lastptr = (uint *) &vp->freemem;
	vp->badpages = 0;

	for (rptr = (uint *)vp->rmapptr; pzero <= lastp ; rptr += 1)
	{
		bits = *rptr;

		/* Keep track of large memory holes */
		if (bits == ONES)
			badrange(pzero, blksper * 32);

		mask = 1 << 31;
		for( ; mask != 0; mask = mask >> 1, pzero += blksper)
		{
			if(pzero > lastp)
				break;

			/* are the blocks bad ? */
			if (mask & bits)
			{
				vp->badpages += blksper;
				/*
				 * Update any range established by ROS
				 * that was loaded around this bad memory.
				 */
				shiftrange(pzero, blksper);
				continue;
			}
			/* put on free list */
			for(p = pzero; p < pzero + blksper; p ++)
			{
				/*
				 * Exclude frames already assigned to ranges
				 * loaded by ROS.
				 */
				if (isalloc(p))
					continue;
				*lastptr = p << L2PSIZE;
				lastptr = (uint *)(p << L2PSIZE);
			}
		}
	}

	/* record memory beyond lastp as a hole */
	badrange(pzero, MAXRPAGE-lastp-1);

	*lastptr = 0;    /* set end of list in last page */
}

/*
 * calculate the ranges for the various vmsi objects.
 */

static
initranges(vp)
struct vmsidata *vp;
{
	int p,q,npages,size;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set( POWER_RS1 | POWER_RSC ))
	{
		/* hardware hat.  the Power 1 CPU requires 2^15 (32K)
		 * alignment, assuming minimum memory of 4096 Pages
		 * (16 Megabytes)  See the SDR1 (HATORG) description. */
		vmrmap_id(RMAP_HAT)	= RMAP_HAT;
		vmrmap_eaddr(RMAP_HAT)	= (uint) &vmmdseg.hw.rs1.hat[0];
		size = sizeof(int) * (1 << vmker.hashbits);
		vmrmap_size(RMAP_HAT)	= size;
		vmrmap_wimg(RMAP_HAT)	= WIMG_DEFAULT;
		size = MAX(size,vmker.cachealign);
		size = MAX(size,1<<15);
		vmrmap_align(RMAP_HAT)	= size;
		vmrmap_valid(RMAP_HAT)	= 1;

		/* hardware pft.  the Power 1 CPU requires 2^16 (64K)
		 * alignment, assuming minimum memory of 4096 Pages
		 * (16 Megabytes)  See the SDR0 (PFTORG) description. */
		vmrmap_id(RMAP_PFT)	= RMAP_PFT;
		vmrmap_eaddr(RMAP_PFT)	= (uint) &vmmdseg.hw.rs1.pft[0];
		size = sizeof(struct rs1pft)*vp->memsize;
		vmrmap_size(RMAP_PFT) 	= size;
		vmrmap_wimg(RMAP_PFT)	= WIMG_DEFAULT;
		size = MAX(size,vmker.cachealign);
		size = MAX(size,1<<16);
		vmrmap_align(RMAP_PFT)	= size;
		vmrmap_valid(RMAP_PFT)	= 1;
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if (__power_rs2())
	{
		/* hashed page table */
		vmrmap_id(RMAP_PFT)	= RMAP_PFT;
		vmrmap_eaddr(RMAP_PFT)	= (uint) &vmmdseg.hw.rs2.pte[0];
		size = sizeof(struct rs2pte)*PTEGSIZE*(1 << vmker.hashbits);
		vmrmap_size(RMAP_PFT) 	= size;
		vmrmap_wimg(RMAP_PFT)	= WIMG_DEFAULT;
		vmrmap_align(RMAP_PFT)	= MAX(size,vmker.cachealign);
		vmrmap_valid(RMAP_PFT)	= 1;

		/* pvt */
		vmrmap_id(RMAP_PVT)	= RMAP_PVT;
		vmrmap_eaddr(RMAP_PVT)	= (uint) &vmmdseg.hw.rs2.pvt[0];
		vmrmap_size(RMAP_PVT) 	= sizeof(struct rs2pvt)*vp->memsize;
		vmrmap_wimg(RMAP_PVT)	= WIMG_DEFAULT;
		vmrmap_align(RMAP_PVT)	= vmker.cachealign; /* accessed real */
		vmrmap_valid(RMAP_PVT)	= 1;
	}
#endif /* POWER_RS2 */

#ifdef _POWER_PC
	if (__power_pc())
	{
		/* hashed page table */
		vmrmap_id(RMAP_PFT)	= RMAP_PFT;
		vmrmap_eaddr(RMAP_PFT)	= (uint) &vmmdseg.hw.ppc.pte[0];
		size = sizeof(struct ppcpte)*PTEGSIZE*(1 << vmker.hashbits);
		vmrmap_size(RMAP_PFT) 	= size;
		vmrmap_wimg(RMAP_PFT)	= WIMG_DEFAULT;
		vmrmap_align(RMAP_PFT)	= MAX(size,vmker.cachealign);
		vmrmap_valid(RMAP_PFT)	= 1;

		/* pvt */
		vmrmap_id(RMAP_PVT)	= RMAP_PVT;
		vmrmap_eaddr(RMAP_PVT)	= (uint) &vmmdseg.hw.ppc.pvt[0];
		vmrmap_size(RMAP_PVT) 	= sizeof(struct ppcpvt)*vp->memsize;
		vmrmap_wimg(RMAP_PVT)	= WIMG_DEFAULT;
		vmrmap_align(RMAP_PVT)	= vmker.cachealign; /* accessed real */
		vmrmap_valid(RMAP_PVT)	= 1;

		/* pvlist */
		vmrmap_id(RMAP_PVLIST)	= RMAP_PVLIST;
		vmrmap_eaddr(RMAP_PVLIST) = (uint) &vmmdseg.hw.ppc.pvlist[0];
		size = sizeof(struct ppcpvlist)*PTEGSIZE*(1 << vmker.hashbits);
		vmrmap_size(RMAP_PVLIST)  = size;
		vmrmap_wimg(RMAP_PVLIST)  = WIMG_DEFAULT;
		vmrmap_align(RMAP_PVLIST) = vmker.cachealign; /* access real */
		vmrmap_valid(RMAP_PVLIST) = 1;
	}
#endif /* _POWER_PC */

	/* software hat */
	vmrmap_id(RMAP_SWHAT)	= RMAP_SWHAT;
	vmrmap_eaddr(RMAP_SWHAT)= (uint) &vmmdseg.hat[0];
	vmrmap_size(RMAP_SWHAT) = sizeof(int) * (vmker.swhashmask + 1);
	vmrmap_wimg(RMAP_SWHAT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_SWHAT)= vmker.cachealign; /* accessed in real mode */
	vmrmap_valid(RMAP_SWHAT)= 1;

	/* software PFT */
	vmrmap_id(RMAP_SWPFT)	= RMAP_SWPFT;
	vmrmap_eaddr(RMAP_SWPFT)= (uint) &vmmdseg.pft[0];
	vmrmap_size(RMAP_SWPFT) = sizeof(struct pftsw)*vp->memsize;
	vmrmap_wimg(RMAP_SWPFT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_SWPFT)= vmker.cachealign; /* accessed in real mode */
	vmrmap_valid(RMAP_SWPFT)= 1;

	/* alias page table hat */
	vmrmap_id(RMAP_AHAT)	= RMAP_AHAT;
	vmrmap_eaddr(RMAP_AHAT) = (uint) &vmmdseg.ahat[0];
	vmrmap_size(RMAP_AHAT)  = sizeof(ushort) * (vmker.ahashmask + 1);
	vmrmap_wimg(RMAP_AHAT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_AHAT) = vmker.cachealign; /* accessed in real mode */
	vmrmap_valid(RMAP_AHAT) = 1;

	/* alias page table */
	vmrmap_id(RMAP_APT)	= RMAP_APT;
	vmrmap_eaddr(RMAP_APT)  = (uint) &vmmdseg.apt[0];
	vmrmap_size(RMAP_APT)   = sizeof(struct apt) * (vmker.ahashmask + 1);
	vmrmap_wimg(RMAP_APT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_APT)  = vmker.cachealign; /* accessed in real mode */
	vmrmap_valid(RMAP_APT)  = 1;

	/* repaging hash table */
	vmrmap_id(RMAP_RPHAT)	= RMAP_RPHAT;
	vmrmap_eaddr(RMAP_RPHAT)= (uint) &vmmdseg.rphat[0];
	size = 1 << loground(vp->memsize - vp->badpages);
	size = MIN(size,MAXREPAGE) * 2;
	vmrmap_size(RMAP_RPHAT) = size;
	vmrmap_wimg(RMAP_RPHAT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_RPHAT)= PSIZE;
	vmrmap_valid(RMAP_RPHAT)= 1;

	/* repaging table */
	vmrmap_id(RMAP_RPT)	= RMAP_RPT;
	vmrmap_eaddr(RMAP_RPT)	= (uint) &vmmdseg.rpt[0];
	size = 1 << loground(vp->memsize - vp->badpages);
	size = MIN(size,MAXREPAGE) * sizeof(struct repage);
	vmrmap_size(RMAP_RPT) 	= size;
	vmrmap_wimg(RMAP_RPT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_RPT)	= PSIZE;
	vmrmap_valid(RMAP_RPT)	= 1;

	/* pdt thru first 16 entries in scb table */
	vmrmap_id(RMAP_PDT)	= RMAP_PDT;
	vmrmap_eaddr(RMAP_PDT)	= (uint) &vmmdseg.pdt[0];
	p = (int)&vmmdseg.pdt[0];
	q = (int)&vmmdseg.scb[15];
	vmrmap_size(RMAP_PDT) 	= BTOPG(q - p + 1) << L2PSIZE;
	vmrmap_wimg(RMAP_PDT)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_PDT)	= PSIZE;
	vmrmap_valid(RMAP_PDT)	= 1;

	/* ptaseg root (root of xpt2) */
	vmrmap_id(RMAP_PTAR)	= RMAP_PTAR;
	vmrmap_eaddr(RMAP_PTAR)	= (uint) &pta_root;
	vmrmap_size(RMAP_PTAR) 	= PSIZE;
	vmrmap_wimg(RMAP_PTAR)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_PTAR)	= PSIZE;
	vmrmap_valid(RMAP_PTAR)	= 1;

	/* ptaseg first direct xpt block (i.e. xpt3) */
	vmrmap_id(RMAP_PTAD)	= RMAP_PTAD;
	vmrmap_eaddr(RMAP_PTAD)	= (uint) &pta_xptdblk[0];
	vmrmap_size(RMAP_PTAD) 	= PSIZE;
	vmrmap_wimg(RMAP_PTAD)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_PTAD)	= PSIZE;
	vmrmap_valid(RMAP_PTAD)	= 1;

	/* ptaseg initial roots -- we allow for 8 although we
	 * only need 5 now (kernel,vmmdseg,ptaseg,dmapseg,kextseg).
	 */
	vmrmap_id(RMAP_PTAI)	= RMAP_PTAI;
	vmrmap_eaddr(RMAP_PTAI)	= (uint) &pta_page[FIRSTAPM];
	vmrmap_size(RMAP_PTAI) 	= 2*PSIZE;
	vmrmap_wimg(RMAP_PTAI)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_PTAI)	= PSIZE;
	vmrmap_valid(RMAP_PTAI)	= 1;

	/* paging space disk map */
	vmrmap_id(RMAP_DMAP)	= RMAP_DMAP;
	vmrmap_eaddr(RMAP_DMAP)	= (uint) (TEMPSR << L2SSIZE);
	npages = (vp->pgsize + DBPERPAGE - 1)/DBPERPAGE;
	vmrmap_size(RMAP_DMAP) 	= PSIZE*npages;
	vmrmap_wimg(RMAP_DMAP)	= WIMG_DEFAULT;
	vmrmap_align(RMAP_DMAP)	= PSIZE;
	vmrmap_valid(RMAP_DMAP)	= 1;
}

/*
 * allocate real storage for ranges. returns 0 ok and -1 if alloc
 * fails.
 */
static int backreal(vp)
struct vmsidata *vp;
{
	int rc,k,size,align;

	for (k = 0; k <= RMAP_X; k++)
	{
		/*
		 * Skip over invalid ranges, I/O ranges and
		 * ranges already backed with real memory.
		 */
		if (!vmrmap_valid(k) || vmrmap_io(k) || vmrmap_ros(k))
			continue;
		size = vmrmap_size(k);
		align = vmrmap_align(k);
		if (rc = alloc(vp,size,align,&vmrmap_raddr(k)))
			return(rc);
	}

	return(0);
}

/*
 * make ranges addressable virtually
 */

static  backvirt(vp)
struct vmsidata *vp;
{
	int k,sidx,raddr,vaddr,size,wimg;

	for(k = 0; k <= RMAP_X; k++)
	{
		/*
		 * Skip over invalid ranges and ranges which get assigned
		 * a unique segment.
		 */
		if (!vmrmap_valid(k) || vmrmap_seg(k))
			continue;

		switch(vmrmap_eaddr(k) >> L2SSIZE) {
			case KERNSR:	sidx = KERSIDX; break;
			case VMMSR:	sidx = VMMSIDX; break;
			case PTASR:	sidx = PTASIDX; break;
			case TEMPSR:	sidx = DMAPSIDX; break;
			case KERNXSR:	sidx = KEXTSIDX; break;
			default: 	sidx = KERSIDX; break;
		}
		raddr  = vmrmap_raddr(k);
		vaddr  = vmrmap_eaddr(k);
		size  = vmrmap_size(k);
		wimg  = vmrmap_wimg(k);
		insvirt(vp,sidx,vaddr,raddr,size,wimg);
	}
}

/*
 * make addressable virtually an interval in a segment.
 */

static
insvirt(vp,sidx,vaddr,raddr,nbytes,wimg)
struct vmsidata *vp;
int    sidx;    /* index in scb array */
int    vaddr;   /* virtual address first page */
int    raddr;   /* real address first page */
int    nbytes;  /* number of bytes         */
int    wimg;	/* WIMG */
{
	int npages,k,vpage,nfr;

	nfr = (uint)raddr >> L2PSIZE; /* first page frame */
	vpage = (vaddr & SOFFSET) >> L2PSIZE; /* first virtual page  */
	npages = (nbytes + PSIZE - 1) >> L2PSIZE;

	for(k = 0; k < npages; k++, nfr += 1, vpage += 1)
	{
		rinspft(vp,sidx,vpage,nfr,wimg);
	}
}

/*
 * allocate block of storage of size and alignment specified
 * and set addr to its address. align is assumed to be multiple
 * PSIZE, size is rounded up to multiple of psize. block is
 * allocated at and address which is a multiple of align and
 * the block is cleared to zeros. returns 0 ok, -1 can't allocate.
 */

static int
alloc(vp,size,align,addr)
struct vmsidata *vp;
int size;     /* number of bytes to allocate */
int align;    /* alignment constraint */
uint * addr;  /* addr of allocated block */
{

	int k,npages,n,last,p;
	uint amask,*ptr,first, *ptrprev;

	/* nothing to do ? */
	if (size == 0)
		return(0);

	/*
	 * compute alignment mask. amask will have low order bits
	 * set for masking with a page address.
	 */

	for(amask = PSIZE; amask < align; amask = amask << 1) ;
	amask = amask - 1;

	/*
	 * search for npages consecutive free pages which start
	 * with the right alignment.
	 */

	npages = BTOPG(size);
	ptrprev = (uint *) &vp->freemem;
	while(*ptrprev != 0)
	{
		/*
		 * is alignment of first page ok ?
		 */
		first = *ptrprev;
		if (amask & first)
		{
			ptrprev = (uint *)first;
			continue;
		}

		/*
		 * look for npages in a row
		 */
		last = first + (npages - 1)*PSIZE;
		for (p = first; p < last; p += PSIZE)
		{
			ptr = (uint *)p;
			if (*ptr != p + PSIZE)
			{
				ptrprev = ptr;
				break;
			}
		}

		/*
		 * found npages in a row ?
		 */
		if (p == last)
		{
#ifdef	_POWER_601
			/*
			 * try again if range spans a 256MB boundary
			 * (alignment interrupts occur on operations which
			 * cross this boundary while running translate off
			 * and we can't handle this)
			 */
			if (__power_601())
			{
				if ((first & SREGMSK) != (last & SREGMSK))
				{
					/*
					 * Move to beginning of 256MB boundary.
					 */
					for (p = first; p < last; p += PSIZE)
					{
						ptr = (uint *)p;
						if ((*ptr & SREGMSK) !=
						    (first & SREGMSK))
						{
							ptrprev = ptr;
							break;
						}
					}
					continue;
				}
			}
#endif	/* _POWER_601 */
			*addr = first;
			*ptrprev = *(uint *)last;
			clear(first,npages*PSIZE);
			return(0);
		}
	}
	return(-1);
}

/*
 * insert into page frame table info necessary
 * to address a page frame at its virtual address.
 */
static  rinspft(vp,sidx,pno,nfr,wimg)
struct vmsidata *vp;
int  sidx;  /* index in scb table     */
int  pno;     /* virtual page number    */
int  nfr;     /* real page frame number */
int  wimg;   /* WIMG */
{
	int sid,hash, *hatptr, pte;
	struct rs1pft *rs1pft;
	struct rs2pte *rs2pte;
	struct ppcpte *ppcpte;

	/* get sid from sid index  */
	sid = ITOS(sidx,0);

	/* compute hash index */
	hash = HASHF(sid,pno);

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set( POWER_RS1 | POWER_RSC ))
	{
		/* point to base of pft array */
		rs1pft = (struct rs1pft *) vmrmap_raddr(RMAP_PFT);

		/* word 0 of pft */
		rs1pft = rs1pft + nfr;
		rs1pft->u1.s1._sidpno  = SIDPNO(sid,pno);
		rs1pft->u1.s1._valid  = 1;
		rs1pft->u1.s1._key  = INITKEY;

		/* put on hash chain and set word 1 of pft */
		hatptr = (int *) (vmrmap_raddr(RMAP_HAT) + sizeof(int)*hash);
		rs1pft->_next = *hatptr;   /* word 1 of pft */
		*hatptr = nfr;

		/* words 2 and 3 are already zero */
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if (__power_rs2())
	{
		/* point to base of pte array */
		rs2pte = (struct rs2pte *) vmrmap_raddr(RMAP_PFT);

		/* find a free pte in the pteg for this hash class */
		rs2pte = rs2pte + FIRSTPTE(hash);
		for (pte = 0; pte < PTEGSIZE; pte++)
		{
			if ((rs2pte+pte)->u1.s1._valid == 0)
				break;
		}
		/* We shouldn't need to look in the alternate hash group
		 * to find a free entry but just in case, put up an LED
		 */
		if (pte == PTEGSIZE)
			for (;;) write_leds(HALT_NOMEM);

		(rs2pte+pte)->u1.s1._valid = 1;
		(rs2pte+pte)->u1.s1._sid = sid;
		(rs2pte+pte)->u1.s1._avpi = AVPI_RS2(pno);
		(rs2pte+pte)->u2.s2._rpn = nfr;
		(rs2pte+pte)->u2.s2._key = INITKEY;
	}
#endif /* _POWER_RS2 */
	
#ifdef _POWER_PC
	if (__power_pc())
	{
		/* point to base of pte array */
		ppcpte = (struct ppcpte *) vmrmap_raddr(RMAP_PFT);

		/* find a free pte in the pteg for this hash class */
		ppcpte = ppcpte + FIRSTPTE(hash);
		for (pte = 0; pte < PTEGSIZE; pte++)
		{
			if ((ppcpte+pte)->u1.s1._valid == 0)
				break;
		}
		/* We shouldn't need to look in the alternate hash group
		 * to find a free entry but just in case, put up an LED
		 */
		if (pte == PTEGSIZE)
			for (;;) write_leds(HALT_NOMEM);

		(ppcpte+pte)->u1.s1._valid = 1;
		(ppcpte+pte)->u1.s1._sid = sid;
		(ppcpte+pte)->u1.s1._avpi = AVPI_PPC(pno);
		(ppcpte+pte)->u2.s2._rpn = nfr;
		(ppcpte+pte)->u2.s2._key = INITKEY;
		(ppcpte+pte)->u2.s2._wimg = wimg;
	}
#endif /* _POWER_PC */
	
}

/*
 * compute hashmask and stoimask
 * calculate sreg values for vmmdseg,ptaseg and paging space dmap.
 */

static  initkerdata(vp)
struct vmsidata *vp;
{
	int minhash;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1|POWER_RSC))
	{
		/*
		 * The hardware supports a HAT mask of 13 to 21 bits.
		 * We restrict the maximum size to 20 bits so the ITOS
		 * hash function produces at most a 20-bit SID.
		 */
		minhash = vp->logmsize + 1;	/* "r+1" bits */
		vmker.hashbits = MIN(MAX(minhash, 13), 20);
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if (__power_rs2())
	{
		/*
		 * The hardware supports an HTAB mask of 11 to 19 bits.
		 * We restrict the maximum size to 18 bits because we use
		 * a 21-bit PTE index (so that PVNULL is an invalid index).
		 * The size is scalable based on vmvars.htabscale and defaults
		 * to the recommended size of one PTE group for every 2 frames
		 * (vmvars.htabscale is statically initialized to -1).
		 */
		minhash = loground(vp->memsize - vp->badpages);
		minhash += vmvars.htabscale;
		vmker.hashbits = MIN(MAX(minhash, 11), 18);
	}
#endif /* _POWER_RS2 */

#ifdef _POWER_PC
	if (__power_pc())
	{
		/*
		 * The hardware supports an HTAB mask of 10 to 19 bits.
		 * We restrict the maximum size to 18 bits because we use
		 * a 21-bit PTE index (so that PVNULL is an invalid index).
		 * The size is scalable based on vmvars.htabscale and defaults
		 * to the recommended size of one PTE group for every 2 frames
		 * (vmvars.htabscale is statically initialized to -1).
		 */
		minhash = loground(vp->memsize - vp->badpages);
		minhash += vmvars.htabscale;
		vmker.hashbits = MIN(MAX(minhash, 10), 18);
	}
#endif /* _POWER_PC */

	/*
	 * Masks for HASHF and ITOS/STOI macros.
	 */
	vmker.hashmask = (1 << vmker.hashbits) - 1;
	vmker.stoibits = vmker.hashbits - STOIBITS;
	vmker.stoimask = (1 << STOIBITS) - 1;

	/*
	 * Mask for software hash.
	 */
	minhash = loground(vp->memsize - vp->badpages);
	vmker.swhashmask = (1 << (minhash - 1)) - 1;	/* "r-1" bits */

	/*
	 * Mask for alias page table hash.
	 * The default size is based on the platform and is scalable based
	 * on vmvars.aptscale.
	 */
#ifdef _POWER_RS
	if (__power_rs())
	{
		/* For _POWER_RS the APT is used as a fast-lookup cache
		 * for alias addresses (to improve ping-pong performance).
		 * The default size of 8 provides for a 1MB cache of aliases.
		 */
		minhash = 8;
	}
#endif /* POWER_RS */
#ifdef _POWER_PC
	if (__power_pc())
	{
		/* For _POWER_PC which supports aliasing in h/w, the APT is
		 * used to hold T=0 mappings to I/O space.  The default size
		 * of 6 provides for 256KB of T=0 I/O mappings.  Note that
		 * some space is still required to perform raw I/O operations
		 * on alias addresses (to handle pinning of such addresses).
		 */
		minhash = 6;
	}
#endif /* _POWER_PC */
	minhash += vmvars.aptscale;
	minhash = MIN(MAX(minhash,6), loground(MAXAPT));
	vmker.ahashmask = (1 << (minhash)) - 1;

	/*
	 * Virtual address byte alignment to avoid cache aliasing.
	 */
	vmker.cachealign = (1 << (L2PSIZE + _system_configuration.cach_cong));

#ifdef _POWER_RS1
	/*
	 * H/W workaround, RS1 with 2 Data Cache Units (RS.9)
	 */
	if (__power_rs1())
		if ( _system_configuration.dcache_size == 32768 )
			vmker.cachealign <<= 1;
#endif /* _POWER_RS1 */

	/*
	 * set the number of reserved paging space blocks.
	 */
	vmker.psrsvdblks = NRSVDBLKS;

	/*
	 * compute sreg values
	 */
	vmker.kernsrval = g_ksrval = SRVAL(ITOS(KERSIDX,0),0,0);
	vmker.vmmsrval = SRVAL(ITOS(VMMSIDX,0),0,0);
	vmker.ptasrval = SRVAL(ITOS(PTASIDX,0),0,0);
	vmker.dmapsrval = SRVAL(ITOS(DMAPSIDX,0),0,0);
	vmker.kexsrval = g_kxsrval = SRVAL(ITOS(KEXTSIDX,0),0,0);
}

/*
 * initialize page table
 */
static  initpft(vp)
struct vmsidata *vp;
{
	int  *p, *p0, *p1;
	struct rs2pte *rs2pte, *rs2pte0, *rs2pte1;
	struct rs2pvt *rs2pvt, *rs2pvt0, *rs2pvt1;
	struct ppcpte *ppcpte, *ppcpte0, *ppcpte1;
	struct ppcpvt *ppcpvt, *ppcpvt0, *ppcpvt1;
	ushort  *s, *s0, *s1;
	struct pftsw *pftsw, *pftsw0, *pftsw1;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1|POWER_RSC))
	{
		/* init hat
		 */
		p0 = (int *) vmrmap_raddr(RMAP_HAT);
		p1 = p0 + vmrmap_size(RMAP_HAT)/sizeof(int);
		for ( p = p0; p < p1; p += 1)
		{
			*p = -1;
		}
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if (__power_rs2())
	{
		/* init PVT
		 */
		rs2pvt0 = (struct rs2pvt *) vmrmap_raddr(RMAP_PVT);
		rs2pvt1 = rs2pvt0 +
			  (vmrmap_size(RMAP_PVT)/sizeof(struct rs2pvt));
		for ( rs2pvt = rs2pvt0; rs2pvt < rs2pvt1 ; rs2pvt += 1)
		{
			rs2pvt->u1.s1._ptex = PVNULL;
		}
	}
#endif /* _POWER_RS2 */

#ifdef _POWER_PC
	if (__power_pc())
	{
		/* init PVT
		 */
		ppcpvt0 = (struct ppcpvt *) vmrmap_raddr(RMAP_PVT);
		ppcpvt1 = ppcpvt0 +
			  (vmrmap_size(RMAP_PVT)/sizeof(struct ppcpvt));
		for ( ppcpvt = ppcpvt0; ppcpvt < ppcpvt1 ; ppcpvt += 1)
		{
			ppcpvt->u1.s1._ptex = PVNULL;
		}
	}
#endif /* _POWER_PC */

	/* init s/w hat
	 */
	p0 = (int *) vmrmap_raddr(RMAP_SWHAT);
	p1 = p0 + vmrmap_size(RMAP_SWHAT)/sizeof(int);
	for ( p = p0; p < p1; p += 1)
	{
		*p = -1;
	}

	/* init alias page table hat
	 */
	s0 = (ushort *) vmrmap_raddr(RMAP_AHAT);
	s1 = s0 + vmrmap_size(RMAP_AHAT)/sizeof(ushort);
	for ( s = s0; s < s1; s += 1)
	{
		*s = APTNULL;
	}

	/* init s/w PFT alias chain
	 */
	pftsw0 = (struct pftsw *) vmrmap_raddr(RMAP_SWPFT);
	pftsw1 = pftsw0 + vmrmap_size(RMAP_SWPFT)/sizeof(struct pftsw);
	for ( pftsw = pftsw0; pftsw < pftsw1 ; pftsw += 1)
	{
		pftsw->u7.s1._alist = APTNULL;
	}
}

/*
 * return smallest integer m such that 2**m >= input
 */
static int loground(k)
int k;
{
	int log,m;

	log = 0;
	for(m = 1; m < k; m = m << 1 ) {
		log = log + 1;
	}
	return(log);
}

/*
 * clear block of memory to zero
 */
static  clear(start,nbytes)
uint * start; /* assumed to start on word boundary */
int    nbytes; /* assumed to be multiple of 4 */
{
	uint * ptr;
	for(ptr = start; ptr < start + nbytes/4; ptr ++)
		*ptr = 0;
}

/*
 * initialize page fixed parts of vmmdseg (but not pft and hat)
 */
static  initvmdseg(vp)
struct vmsidata *vp;
{
	int k;
	ushort a, last;

	/*
	 * init pfhdata (mostly already zeroed)
	 */
	vmker.nrpages = vp->memsize;
	vmker.badpages = vp->badpages;

	/*
	 * Set up LRU cursors.
	 */
	pf_firstnf = vmint_end(VMINT_FIXMEM,0) + 1;
	pf_syncptr = pf_lruptr = pf_firstnf;
	pf_lruidx = 0;
	pf_skiplru = vmint_start(VMINT_BADMEM,pf_lruidx);
	
	pf_pfavail = vmker.numfrb = vp->numfrb;
	pf_iotail = -1;

	initvars();

	/* init repaging tables.
	 */
	initrpaging();

	/*
	 * init scb free list
	 */
	pf_sidfree = 0;
	pf_hisid  = 16;   /* one bigger than biggest index on free list */
	for (k = 0; k < 15; k++)
	{
		scb_free(k) = k + 1;
#ifdef _VMM_MP_EFF
		/* Initialize lock with no instrumentation, which is not
		 * available yet.  The locks will really be initialized
		 * in init_vmmlocks()
		 */
		*((simple_lock_data *) &scb_lock(k)) =  SIMPLE_LOCK_AVAIL;
#endif /* _VMM_MP_EFF */
	}
	scb_free(15) = -1;  /* use -1 to denote end of list */
#ifdef _VMM_MP_EFF
	*((simple_lock_data *) &scb_lock(15)) =  SIMPLE_LOCK_AVAIL;
#endif /* _VMM_MP_EFF */

	/*
	 * init ame free list
	 */
	pf_amefree = -1;
	pf_hiame  = 0;

	/*
	 * init apt free list
	 */
	pf_aptfree = 0;
	pf_aptlru = 0;
	last = (vmrmap_size(RMAP_APT) / sizeof(struct apt)) - 1;
	for (a = 0; a < last; a++)
	{
		apt_free(a) = a + 1;
	}
	apt_free(last) = APTNULL;
}

/*
 * initialize the apm free list.
 */
static  initapm()
{
	int k,last;

	last = PSIZE/sizeof(struct apm) - 1; /* last apm index page 0 */

	for(k = FIRSTAPM; k <= last; k ++)
	{
		pta_apm[k].fwd = k + 1;
		pta_apm[k].bwd = k - 1;
	}

	pta_freetail = last;
	pta_freecnt = 0;
	pta_hiapm = last +1;
	pta_apm[FIRSTAPM].bwd = NULL;
	pta_apm[last].fwd = NULL;
	pta_anchor[XPT4K] = FIRSTAPM;
}

/*
 * initialize ptaseg exclusive of apm free list.
 */
static  initptaseg()
{
	int k;

	/*
	 * give back the root which was allocated by vms_create.
	 * We must lock for asserts!
	 */
	ALLOC_MPLOCK();
	v_freexpt(XPT1K,scb_vxpto(PTASIDX));
	ALLOC_MPUNLOCK();
	scb_vxpto(PTASIDX) = (uint) &pta_root;
	scb_ptaseg(PTASIDX) = 1;

	/*
	 * initialize the root
	 */
	for (k = 0; k < XPTENT; k++)
	{
		pta_root.xptdir[k]  = &pta_xptdblk[k];
	}

}

/*
 * put the page frames that remain on the freelist anchored
 * by freemem onto the VMM freelist anchored by the pft
 * entry FBANCH.
 */
static  initfree(vp)
struct vmsidata *vp;
{
	int nfr,nxt;
	uint *ptr;
	struct pftsw * pftptr;

	/*
	 * init free list to NULL
	 */
	vp->numfrb = 0;
	pftptr = (struct pftsw *)vmrmap_raddr(RMAP_SWPFT);

	(pftptr + FBANCH)->u4.s1._freefwd = FBANCH;
	(pftptr + FBANCH)->u4.s1._freebwd = FBANCH;

	for(ptr = &vp->freemem; *ptr != 0; ptr = (uint *)*ptr)
	{
		vp->numfrb += 1;
		nfr = *ptr >> L2PSIZE;   /* its page frame number */
		/*
		 * insert nfr on free list
		 */
		nxt = (pftptr + FBANCH)->u4.s1._freefwd;
		(pftptr + nfr)->u1.s1._free = 1;
		(pftptr + nfr)->u4.s1._freefwd = nxt;
		(pftptr + nxt)->u4.s1._freebwd = nfr;
		(pftptr + nfr)->u4.s1._freebwd = FBANCH;
		(pftptr + FBANCH)->u4.s1._freefwd = nfr;
	}
}

/*
 * initialize paging space.
 * this happens before the first paging device is
 * known. the size should be set to some nominal size
 * say 4096 ( 16 megabytes) in vmsi and dev_t to -1;
 */

static initpgspace(vp)
struct vmsidata *vp;
{
	/* 0 is necessary for nbufstr args at this point */
	return(vm_defineps(vp->pgdev,vp->pgsize,0));
}

/*
 * fill in software pft for pages previously allocated,
 * chain them to scbs. mark them all as page fixed.
 */

static initpftsw(vp)
struct vmsidata *vp;
{
	int k,sidx,vaddr,raddr,size,wimg;

	for(k = 0; k <= RMAP_X; k++)
	{
		/*
		 * Skip over invalid ranges and ranges which get
		 * assigned to a unique segment.
		 */
		if (!vmrmap_valid(k) || vmrmap_seg(k))
			continue;
		switch(vmrmap_eaddr(k) >> L2SSIZE) {
			case KERNSR:	sidx = KERSIDX; break;
			case VMMSR:	sidx = VMMSIDX; break;
			case PTASR:	sidx = PTASIDX; break;
			case TEMPSR:	sidx = DMAPSIDX; break;
			case KERNXSR:	sidx = KEXTSIDX; break;
			default: 	sidx = KERSIDX; break;
		}
		raddr	= vmrmap_raddr(k);
		vaddr	= vmrmap_eaddr(k);
		size	= vmrmap_size(k);
		wimg	= vmrmap_wimg(k);

		/*
		 * Mappings to I/O space get added to the alias page table
		 * so they can be reloaded in the event of a PTE overflow.
		 * Non-I/O mappings corresond to real memory and get added
		 * to the software PFT.
		 */
#ifdef _DEBUG
		/* for asserts */
		SCB_MPLOCK(sidx);
#endif
		if (vmrmap_io(k))
		{
			insapt(vp,sidx,vaddr,raddr,size,wimg);
		}
		else
		{
			inspftsw(vp,sidx,vaddr,raddr,size,wimg);
		}
#ifdef _DEBUG
		SCB_MPUNLOCK(sidx);
#endif
	}
}

/*
 * initialize alias page table for a range of pages.
 */
static insapt(vp,sidx,vaddr,raddr,nbytes,wimg)
struct vmsidata *vp;
int    sidx;    /* index in scb array */
int    vaddr;   /* virtual address first page */
int    raddr;   /* real address first page */
int    nbytes;  /* number of bytes	 */
int    wimg;	/* WIMG */
{
	int npages,k,vpage,nfr,sid;
	ushort aptx, next, hash;

	nfr = (uint)raddr >> L2PSIZE; /* first real page frame */
	vpage = (vaddr & SOFFSET) >> L2PSIZE; /* first virt page number */
	npages = (nbytes + PSIZE - 1) >> L2PSIZE;
	sid = ITOS(sidx,0);

	for(k = 0; k < npages; k++, nfr += 1, vpage += 1)
	{
		/*
		 * Allocate a free entry (put up an LED if none).
		 */
		if (pf_aptfree == APTNULL)
			for (;;) write_leds(HALT_NOMEM);
		next = apt_free(pf_aptfree);
		aptx = pf_aptfree;
		pf_aptfree = next;

		/*
		 * Initialize the entry.
		 */
		apt_sid(aptx) = sid;
		apt_pno(aptx) = vpage;
		apt_key(aptx) = INITKEY;
		apt_wimg(aptx) = wimg;
		apt_nfr(aptx) = nfr;
		apt_pinned(aptx) = 1;
		apt_valid(aptx) = 1;
		apt_free(aptx) = APTNULL;

		/*
		 * Finally, add it to the alias hash chain.
		 */
		hash = AHASH(sid,vpage);
		apt_next(aptx) = *(volatile ushort *)&ahattab[hash];
		ahattab[hash] = aptx;
	}
}

/*
 * initialize software pft for a range of pages in a segment.
 */
static inspftsw(vp,sidx,vaddr,raddr,nbytes,wimg)
struct vmsidata *vp;
int    sidx;    /* index in scb array */
int    vaddr;   /* virtual address first page */
int    raddr;   /* real address first page */
int    nbytes;  /* number of bytes	 */
int    wimg;	/* WIMG */
{
	int npages,k,vpage,nfr,sid;
	int hash, pte;
	uint ptex;
	struct rs1pft *rs1pft;
	struct rs2pte *rs2pte;
	struct rs2pvt *rs2pvt;
	struct ppcpte *ppcpte;
	struct ppcpvt *ppcpvt;
	struct ppcpvlist *ppcpvlist;

	nfr = (uint)raddr >> L2PSIZE; /* first real page frame */
	vpage = (vaddr & SOFFSET) >> L2PSIZE; /* first virt page number */
	npages = (nbytes + PSIZE - 1) >> L2PSIZE;
	sid = ITOS(sidx,0);

	for(k = 0; k < npages; k++, nfr += 1, vpage += 1)
	{
		pft_ssid(nfr) = sid;
		pft_spage(nfr) = vpage;
		pft_key(nfr) = INITKEY;
		pft_wimg(nfr) = wimg;
		pft_inuse(nfr) = 1;

		/*
		 * Insert on software hash chain.
		 */
		v_inspft(sid,vpage,nfr);

		/*
		 * Initialize PVT.
		 * Translate is on so we must access machine-dependent
		 * structures without using macros since the macros
		 * expand to xlate off accesses.
		 */
#if defined(_POWER_RS1) || defined(_POWER_RSC)
		if (__power_set(POWER_RS1|POWER_RSC))
		{
			rs1pft = (struct rs1pft *) vmrmap_eaddr(RMAP_PFT);
			(rs1pft+nfr)->u2.s1._page = vpage;
		}
#endif /* _POWER_RS1 || _POWER_RSC */
#ifdef _POWER_RS2
		if (__power_rs2())
		{
			/* point to base of pte array */
			rs2pte = (struct rs2pte *) vmrmap_eaddr(RMAP_PFT);

			/* find pte for this (sid,pno) */
			hash = HASHF(sid,vpage);
			rs2pte = rs2pte + FIRSTPTE(hash);
			for (pte = 0; pte < PTEGSIZE; pte++)
			{
				if ((rs2pte+pte)->u1.s1._valid &&
				    (rs2pte+pte)->u1.s1._sid == sid &&
					(rs2pte+pte)->u1.s1._avpi ==
							AVPI_RS2(vpage))
					break;
			}
			/* We shouldn't need to look in the alternate hash group
			 * to find a free entry but just in case, put up an LED
			 */
			if (pte == PTEGSIZE)
				for (;;) write_leds(HALT_NOMEM);

			/* point to base of pvt array */
			rs2pvt = (struct rs2pvt *) vmrmap_eaddr(RMAP_PVT);
			(rs2pvt+nfr)->u1.s1._ptex = FIRSTPTE(hash) + pte;
		}
#endif /* _POWER_RS2 */
#ifdef _POWER_PC
		if (__power_pc())
		{
			/* point to base of pte array */
			ppcpte = (struct ppcpte *) vmrmap_eaddr(RMAP_PFT);

			/* find pte for this (sid,pno) */
			hash = HASHF(sid,vpage);
			ppcpte = ppcpte + FIRSTPTE(hash);
			for (pte = 0; pte < PTEGSIZE; pte++)
			{
				if ((ppcpte+pte)->u1.s1._valid &&
				    (ppcpte+pte)->u1.s1._sid == sid &&
					(ppcpte+pte)->u1.s1._avpi ==
							AVPI_PPC(vpage))
					break;
			}
			/* We shouldn't need to look in the alternate hash group
			 * to find a free entry but just in case, put up an LED
			 */
			if (pte == PTEGSIZE)
				for (;;) write_leds(HALT_NOMEM);

			/* point to base of pvt array */
			ppcpvt = (struct ppcpvt *) vmrmap_eaddr(RMAP_PVT);
			ptex = FIRSTPTE(hash) + pte;
			(ppcpvt+nfr)->u1.s1._ptex = ptex;
	
			/* init pvlist entry for this mapping */
			ppcpvlist = (struct ppcpvlist *)
					vmrmap_eaddr(RMAP_PVLIST);
			(ppcpvlist+ptex)->u1.s1._next = PVNULL;
			(ppcpvlist+ptex)->u1.s1._pno = PVPNO_PPC(vpage);
		}
#endif /* _POWER_PC */

		/*
		 * except for FBANCH set pincount to maxpin
		 * pincount overlays free block lists so
		 * we can't set it for FBANCH.
		 */
		if (nfr != FBANCH)
			pft_pincount(nfr) = MAXPIN;
		/*
		 * update maxvpn minvpn
		 */
		if (vpage <= scb_uplim(sidx))
		{
			scb_maxvpn(sidx) = MAX(vpage,scb_maxvpn(sidx));
		}
		else
		{
			scb_minvpn(sidx) = MIN(vpage,scb_minvpn(sidx));
		}
		v_insscb(sidx,nfr);   /* put on scb list */

	}
}

/*
 * unfixkern
 * allocate xpt direct blocks to cover (0,endload).
 * unfix the pages which are not supposed to be page fixed.
 * and allocate disk blocks to back them up.
 * machine-dependent regions that will be released are still covered
 * in the FIXMEM intervals so that disk blocks are not allocated.
 */

static  unfixkern(vp)
struct vmsidata *vp;
{
	int k,n,p,mempgsz,nxpt,nfix,first,last,rc;
	struct xptroot *ptr;

	/*
	 * allocate xpt direct blocks to back up (0,endload).
	 */

	nxpt = (vmrmap_size(RMAP_KERN) + BPERXPT1K - 1)/BPERXPT1K;
	ptr = (struct xptroot *) scb_vxpto(KERSIDX);

	for(k = 0; k < nxpt; k++)
	{
		p = vcs_getxpt(XPT1K,V_NOTOUCH);
		if (p == 0)
			return(-1);
		ptr->xptdir[k] = (struct xptdblk *) p;
	}

	/*
	 * the fixed intervals are in sort order. unfix
	 * the pages in the gaps.
	 */

	mempgsz = ( vmrmap_size(RMAP_KERN) - 1 ) >> L2PSIZE;
	nfix = vmint_num(VMINT_FIXMEM);
	for(n = 0; n < nfix; n++)
	{
		/*
		 * determine first and last pages to unfix
		 */
		first  = vmint_end(VMINT_FIXMEM,n);
		last   = (n == nfix - 1) ? mempgsz
					 : vmint_start(VMINT_FIXMEM,n+1);

		/*
		 * unfix them.
		 */
		for(p = first; p < last; p += 1)
		{
			if( rc = unfix(vp,p))
				return(rc);
		}
	}
	return(0);
}

/*
 * unfix a page in the kernel and allocate a disk block
 * for it. set modbit. returns 0 ok, -1 if couldn't
 * allocate paging space disk block.
 */

static unfix(vp,nfr)
struct vmsidata *vp;
int  nfr;    /* page frame number = virtual pno */
{

	int dblock,k,n;
	union xptentry *xpt;
	struct xptroot * ptr;
	int srsave, rc;
	struct vmdmap *p0;

	/*
	 * set modbit and unpin page.
	 */
	SETMOD(nfr);
	pft_pincount(nfr) = 0;
	pf_pfavail += 1;

	/*
	 * allocate disk block and update scb disk block count
	 */
	srsave = chgsr(TEMPSR,vmker.dmapsrval);
	p0 = (struct vmdmap *) (TEMPSR << L2SSIZE);
	rc = v_alloc(p0,pdt_fperpage(0),p0->lastalloc,&dblock);
	(void)chgsr(TEMPSR,srsave);
	if(rc != 0)
		return(-1);

	vmker.psfreeblks += -1;
	scb_npsblks(KERSIDX) += 1;

	/*
	 * get addr of xpt entry. note that nfr = virtual page
	 * number.
	 */

	ptr = (struct xptroot *) scb_vxpto(KERSIDX);
	k = nfr >> L2XPTENT;    /* index in root */
	n = nfr & (XPTENT - 1); /* index in direct block */
	xpt = &ptr->xptdir[k]->xpte[n];

	/*
	 * put dblock in pft and xpt entry. set key.
	 */
	pft_devid(nfr) = 0;
	pft_dblock(nfr) = dblock;
	xpt->spkey = INITKEY;
	xpt->cdaddr = dblock;
	return(0);
}

/*
 * sort the fix intervals smallest first.
 */
static sortfix(vp)
struct vmsidata *vp;
{
	struct vmintervals temp;
	int k,n,top,nfix;

	/* bubble sort */
	nfix = vmint_num(VMINT_FIXMEM);
	for(k = 0; k < nfix; k++)
	{
		top = vmint_start(VMINT_FIXMEM,k);
		for(n = k + 1; n < nfix; n++)
		{
			if (top < vmint_start(VMINT_FIXMEM,n))
				continue;
			top = vmint_start(VMINT_FIXMEM,n);
			temp     = vmint[k];
			vmint[k] = vmint[n];
			vmint[n] = temp;
		}
	}
}

/*
 * initramdisk
 *
 * create a segment for ram disk. move the good pages of the
 * ram disk loaded by ipl to it.
 */

static
initramdisk(vp)
struct vmsidata *vp;
{

	int sid,rc,sp,tp,firstp,npages,savetempsr;
	int sidx,plast,p,last;
	unsigned int ram_eaddr;     /* Effective address ram disk image */
	int ram_size;	/* size of ram disk image */

	/*
	 * create a segment for ram disk.
	 * fill in sreg val to address it in vmker structure.
	 */
	if (rc = vms_create(&sid,V_WORKING|V_SYSTEM,0,0,SEGSIZE/2,SEGSIZE/2))
		return(rc);
	vmker.ramdsrval = SRVAL(sid,0,0);
	sidx = STOI(sid);

	tp = 0;
	firstp = vmrmap_raddr(RMAP_RAMD) >> L2PSIZE;
	npages = BTOPG(vmrmap_size(RMAP_RAMD));

	for(sp = firstp; sp < firstp + npages; sp ++)
	{
		if (isbad(vp,sp))
		{
			pft_pincount(sp) = MAXPIN;
			continue;
		}

		/*
		 * make sp addressable in kernel segment
		 * we always make it addressable at MAXVPN.
		 * copy the page to the ramdisk segment page tp.
		 */
		pft_key(sp) = INITKEY;
		P_ENTER(NORMAL,vmker.kernsrval,MAXVPN,sp,INITKEY,WIMG_DEFAULT);
		copypage(sid,tp,MAXVPN);
		tp++;

		/*
		 * remove addressing to firstp. move sp to free list.
		 * also correct pf_pfavail count.
		 */
		P_REMOVE_ALL(sp);
#ifdef _DEBUG
		/* for asserts */
		SCB_MPLOCK(0);
#endif
		v_insfree(sp);
#ifdef _DEBUG
		SCB_MPUNLOCK(0);
#endif
		pf_pfavail++;
	}

	savetempsr = chgsr(TEMPSR, vmker.ramdsrval);
	ram_eaddr = TEMPSR << L2SSIZE;

	/* query RAM disk driver for size of RAM disk to pin.
	 * routine ram_disk_size() must not depend on anything that
	 * hasn't been initialized yet by vmsi.
	 */
	ram_size = ram_disk_size((void *) ram_eaddr);

	/* pin pages of boot ram disk
	 */
	pin(ram_eaddr, ram_size);
	(void)chgsr(TEMPSR, savetempsr);

	/* free disk blocks which were allocated on copypage faults
	 * in order to conserve paging space.
	 * V_NOKEEP must be used to prevent setting the page
	 * to logically zero.
	 */
	plast =BTOPG(ram_size)-1;
	for (p = 0; p <= plast; p += XPTENT)
	{
		last = MIN((p + XPTENT - 1), plast);
#ifdef _DEBUG
		/* for asserts */
		SCB_MPLOCK(sidx);
#endif
		v_releasexpt(sidx,p,last,V_NOKEEP);
#ifdef _DEBUG
		SCB_MPUNLOCK(sidx);
#endif
	}
	
	return(0);
}

/*
 * initbconfig
 *
 * create a segment for base config area. move the good pages of the
 * base config area loaded by ipl to it.
 */

static
initbconfig(vp)
struct vmsidata *vp;
{

	int sid,rc,sp,tp,firstp,npages,savetempsr;
	int sidx,pfirst,plast,p,last;

	/*
	 * check for the existence of the base config area.
	 */
	if (vmrmap_size(RMAP_BCFG) == 0)
	{
		vmker.bconfsrval = 0;
		return(0);
	}

	/*
	 * create a segment for base config.
	 * fill in sreg val to address it in vmker structure.
	 */
	if (rc = vms_create(&sid,V_WORKING|V_SYSTEM,0,0,SEGSIZE/2,SEGSIZE/2))
		return(rc);
	vmker.bconfsrval = SRVAL(sid,0,0);
	sidx = STOI(sid);

	tp = 0;
	firstp = vmrmap_raddr(RMAP_BCFG) >> L2PSIZE;
	npages = BTOPG(vmrmap_size(RMAP_BCFG));

	for(sp = firstp; sp < firstp + npages; sp ++)
	{
		if (isbad(vp,sp))
		{
			pft_pincount(sp) = MAXPIN;
			continue;
		}

		/*
		 * make sp addressable in kernel segment
		 * we always make it addressable at MAXVPN.
		 * copy the page to the base config segment page tp.
		 */
		pft_key(sp) = INITKEY;
		P_ENTER(NORMAL,vmker.kernsrval,MAXVPN,sp,INITKEY,WIMG_DEFAULT);
		copypage(sid,tp,MAXVPN);
		tp = tp + 1;

		/*
		 * remove addressing to firstp. move sp to free list.
		 * also correct pf_pfavail count.
		 */
		P_REMOVE_ALL(sp);
#ifdef _DEBUG
		/* for asserts */
		SCB_MPLOCK(0);
#endif
		v_insfree(sp);
#ifdef _DEBUG
		SCB_MPUNLOCK(0);
#endif
		pf_pfavail += 1;
	}

	/* pin pages of base config segment.
	 */
	savetempsr = chgsr(TEMPSR, vmker.bconfsrval);
	pin(TEMPSR << L2SSIZE, tp << L2PSIZE);
	(void)chgsr(TEMPSR, savetempsr);

	/* free disk blocks which were allocated on copypage faults
	 * in order to conserve paging space.
	 * V_NOKEEP must be used to prevent setting the page
	 * to logically zero.
	 */
	pfirst = 0;
	plast = pfirst + npages - 1;
	for (p = pfirst; p <= plast; p += XPTENT)
	{
		last = MIN((p + XPTENT - 1), plast);
#ifdef _DEBUG
		/* for asserts */
		SCB_MPLOCK(sidx);
#endif
		v_releasexpt(sidx,p,last,V_NOKEEP);
#ifdef _DEBUG
		SCB_MPUNLOCK(sidx);
#endif
	}

	return(0);
}

/*
 * isbad(vp,pno)
 *
 * returns 1 if real page pno is bad and 0 if good.
 */

static
isbad(vp,pno)
struct vmsidata *vp;
int     pno;
{
	int bit,word, *ptr;

	/*
	 * get pointer to word in ram bit map
	 */
	bit = (pno * PSIZE) / vp->rmapblk;
	word = bit/32;
	ptr = (int *)((vp->rmapptr & SOFFSET) + 4*word);

	/*
	 * move bit to bit position 31 in word, mask and return it.
	 */
	bit = bit - word*32;
	bit = (*ptr >> (31 - bit)) & 0x1;
	return(bit);
}

/*
 * badrange(badnfr, badsize)
 *
 * Keep track of large memory holes so we can make software data
 * structures sparse.  Intervals cover [start, end).
 */

static
badrange(badnfr, badsize)
uint	badnfr, badsize;
{
	uint r;

	/*
	 * If this hole is contiguous with the previous one just update
	 * the current range.  Otherwise create a new range.
	 */
	r = vmint_num(VMINT_BADMEM);
	if (r != 0 && badnfr == vmint_end(VMINT_BADMEM,r-1))
	{
		vmint_end(VMINT_BADMEM,r-1) += badsize;
	}
	else if (r < VMINT_MAX)
	{
		vmint_num(VMINT_BADMEM)++;
		vmint_start(VMINT_BADMEM,r) = badnfr;
		vmint_end(VMINT_BADMEM,r) = badnfr + badsize;
	}
	else
	{
                /*
		 * Halt with LED indicating memory shortage.
		 */
		for (;;) write_leds(HALT_NOMEM);
	}
}

/*
 * freerange()
 *
 * Release memory in software data structures corresponding to memory holes.
 */

static
freerange(vp)
struct vmsidata *vp;
{
	uint r, start, end, p, pfts, pfte, pvts, pvte;
	struct pftsw *pftptr;
	struct rs2pvt *rs2pvt;
	struct ppcpvt *ppcpvt;

        pftptr = (struct pftsw *)vmrmap_raddr(RMAP_SWPFT);

	/*
	 * Free memory corresponding to each hole (excluding the last
	 * interval since it represents non-existing memory).
	 */
	for (r = 0; r < vmint_num(VMINT_BADMEM) - 1; r++)
	{
		start = vmint_start(VMINT_BADMEM,r);
		end = vmint_end(VMINT_BADMEM,r);

		/*
		 * Free full pages in software PFT covering bad memory
		 * in interval [start, end).
		 */
		pfts = ((uint)(pftptr + start) + PSIZE - 1) >> L2PSIZE;
		pfte = ((uint)(pftptr + end) >> L2PSIZE) - 1;

		for (p = pfts; p <= pfte; p++)
		{
#ifdef _DEBUG
			/* for asserts */
			SCB_MPLOCK(VMMSIDX);
#endif
			v_relframe(VMMSIDX,p);
#ifdef _DEBUG
			SCB_MPUNLOCK(VMMSIDX);
#endif
		}

		/*
		 * Free pages in PVT.
		 */
#ifdef _POWER_RS2
		if (__power_rs2())
		{
			rs2pvt = (struct rs2pvt *) vmrmap_raddr(RMAP_PVT);
			pvts = ((uint)(rs2pvt + start) + PSIZE - 1) >> L2PSIZE;
			pvte = ((uint)(rs2pvt + end) >> L2PSIZE) - 1;
			for (p = pvts; p <= pvte; p++)
			{
				v_relframe(VMMSIDX,p);
			}
		}
#endif /* _POWER_RS2 */

#ifdef _POWER_PC
		if (__power_pc())
		{
			ppcpvt = (struct ppcpvt *) vmrmap_raddr(RMAP_PVT);
			pvts = ((uint)(ppcpvt + start) + PSIZE - 1) >> L2PSIZE;
			pvte = ((uint)(ppcpvt + end) >> L2PSIZE) - 1;
			for (p = pvts; p <= pvte; p++)
			{
#ifdef _DEBUG
				/* for asserts */
				SCB_MPLOCK(VMMSIDX);
#endif
				v_relframe(VMMSIDX,p);
#ifdef _DEBUG
				SCB_MPUNLOCK(VMMSIDX);
#endif
			}
		}
#endif /* _POWER_PC */

	}
}

/*
 * shiftrange(badnfr, badsize)
 *
 * RMAP ranges loaded by ROS that are beyond the first 2MB of memory
 * may contain bad memory (or memory holes).  The starting address and
 * size of these ranges are determined by variables initialized by the
 * tool that creates the boot image.
 * VMM code which references these ranges assumes that the starting real
 * address refers to good memory and that the length includes any
 * contained bad memory pages.
 * If the boot image is compressed, the VMM assumptions are satisfied
 * because the expand code updates the addresses to account for bad
 * memory.  If not, the VMM must do its own accounting.  The decision
 * is based upon the setting of vmrmap_holes.
 */

static
shiftrange(badnfr, badsize)
uint	badnfr, badsize;
{
	uint k, ebadnfr, rmapnfr, ermapnfr;

        /*
	 * Determine if the given real memory range is contained in an
	 * RMAP range which was established by ROS and may contain
	 * bad memory.
         */
        for (k = 0; k <= RMAP_X; k++)
        {
		if (vmrmap_holes(k) && vmrmap_valid(k) && vmrmap_ros(k) &&
			!vmrmap_io(k) && vmrmap_size(k))
		{
			ebadnfr = badnfr + badsize - 1;
			rmapnfr = vmrmap_raddr(k) >> L2PSIZE;
			ermapnfr = (vmrmap_raddr(k) + vmrmap_size(k) - 1)
					>> L2PSIZE;
			/*
			 * If the bad block proceeds the RMAP range or has
			 * the same starting address then update the starting
			 * address of the RMAP range to account for the bad
			 * block. Otherwise, if the bad block range is at least
			 * partially contained in the RMAP range then adjust
			 * the size of the RMAP range to account for it.
			 */
			if (rmapnfr >= badnfr)
			{
				vmrmap_raddr(k) += badsize << L2PSIZE;
			}
			else if (badnfr <= ermapnfr)
			{
				vmrmap_size(k) += badsize << L2PSIZE;
			}
		}
	}
}

/*
 * isalloc(nfr)
 *
 * returns 1 if page frame is allocated to a SI range and 0 if not.
 */

static
isalloc(nfr)
uint	nfr;
{
	uint k, snfr, enfr;

	/*
	 * Determine if the given page frame is allocated to an rmap entry
	 * which was established by ROS.
	 */
	for (k = 0; k <= RMAP_X; k++)
	{
		if (vmrmap_ros(k) && vmrmap_valid(k) && !vmrmap_io(k) &&
			vmrmap_size(k))
		{
			snfr = vmrmap_raddr(k) >> L2PSIZE;
			enfr = (vmrmap_raddr(k)+vmrmap_size(k)-1) >> L2PSIZE;
			if (nfr >= snfr && nfr <= enfr)
				return(1);
		}
	}
	return(0);
}

/*
 * copypage(tsid,tpno,spno)
 * copy page spno in kernel segment to page (tsid,tpno)
 */

static
copypage(tsid,tpno,spno)
int     tsid;
int     tpno;
int     spno;
{
	int  savetempsr, *tptr, *sptr, *sptr0;

	/*
	 * get addressabilty to target
	 */
	savetempsr = chgsr(TEMPSR,SRVAL(tsid,0,0));
	tptr  = (int * )( (TEMPSR << L2SSIZE) + (tpno << L2PSIZE) );

	/*
	 * set sptr to start of page in kernel segment
	 */
	sptr  = (int * )(spno << L2PSIZE);

	/*
	 * copy the page
	 */

	sptr0 = sptr + PSIZE/4;
	for ( ; sptr < sptr0; sptr += 8, tptr += 8)
	{
		*(tptr + 0)  = * (sptr + 0);
		*(tptr + 1)  = * (sptr + 1);
		*(tptr + 2)  = * (sptr + 2);
		*(tptr + 3)  = * (sptr + 3);
		*(tptr + 4)  = * (sptr + 4);
		*(tptr + 5)  = * (sptr + 5);
		*(tptr + 6)  = * (sptr + 6);
		*(tptr + 7)  = * (sptr + 7);
	}

	/*
	 * restore TEMPSR
	 */

	(void)chgsr(TEMPSR,savetempsr);
}

/*
 * initiplcb
 *
 * make ipl control block addressable at first page after
 * endload.
 */

static
initiplcb(vp)
struct vmsidata *vp;
{
	int sp;				/* source page */
	int tp;				/* target page */
	int npages,k,nfr;

	npages = BTOPG(vmrmap_size(RMAP_IPLCB));
	sp = vmrmap_raddr(RMAP_IPLCB) >> L2PSIZE;
	tp = vmrmap_size(RMAP_KERN) >> L2PSIZE;
	tp = tp + 2; /* the 2 is to avoid NLS rubbish */
	/*
	 * Add starting page offset to address in order to handle
	 * non page-aligned iplcb.
	 */
	vmker.iplcbptr = (tp << L2PSIZE) + (vmrmap_raddr(RMAP_IPLCB) & POFFSET);

	/*
	 * remove source (sp) from hash chain and move to
	 * hash chain for target (tp). invalidate tlb for sp.
	 */

#ifdef _DEBUG
	/* for asserts */
	SCB_MPLOCK(0);
#endif
	for (k = 0; k < npages; k += 1, sp += 1, tp += 1)
	{
		nfr = sp;
		P_REMOVE_ALL(nfr);
		v_delpft(nfr);
		pft_key(sp) = INITKEY;
		P_ENTER(NORMAL,vmker.kernsrval,tp,nfr,INITKEY,WIMG_DEFAULT);
		pft_spage(nfr) = tp;
		v_inspft(vmker.kernsrval,tp,nfr);
	}

	/*
	 * Now place the Extended IPL Control Block immediately following
	 * the primary one.  This position is not required, simply an
	 * implementation choice.
	 * The vmrmap_raddr(RMAP_IPLCBX) is zero (0) if none exists.
	 */
	if (vmrmap_raddr(RMAP_IPLCBX) == 0)
	{
		vmker.iplcbxptr = 0;
#ifdef _DEBUG
		SCB_MPUNLOCK(0);
#endif
		return;
	}

	/*
	 * On exit from the primary IPLCB loop, tp (target page) is ready
	 * for use here.
	 */
	npages = BTOPG(vmrmap_size(RMAP_IPLCBX));
	sp = vmrmap_raddr(RMAP_IPLCBX) >> L2PSIZE;
	vmker.iplcbxptr = (tp << L2PSIZE) +
			  (vmrmap_raddr(RMAP_IPLCBX) & POFFSET);

	for (k = 0; k < npages; k += 1, sp += 1, tp += 1)
	{
		nfr = sp;
		P_REMOVE_ALL(nfr);
		v_delpft(nfr);
		pft_key(sp) = INITKEY;
		P_ENTER(NORMAL,vmker.kernsrval,tp,nfr,INITKEY,WIMG_DEFAULT);
		pft_spage(nfr) = tp;
		v_inspft(vmker.kernsrval,tp,nfr);
	}
#ifdef _DEBUG
	SCB_MPUNLOCK(0);
#endif
}

/*
 * initcommon(vp)
 *
 * pins interval of addresses in common.
 * this always includes pin_com and at least one machine-specific
 * common range.
 * ltpin will free the backing storage allocated by its touch.
 */

static
initcommon(vp)
struct vmsidata *vp;
{
	unsigned long	 start;
	int		 nbytes;
	int		 p,rc;

	for (p = 0; p < vmint_num(VMINT_FIXCOM); p += 1)
	{
		start  = vmint_start(VMINT_FIXCOM,p) << L2PSIZE;
		nbytes = ( vmint_end(VMINT_FIXCOM,p) << L2PSIZE ) - start;

		if ( (rc = ltpin(start,nbytes) ) != 0)
			return(rc);
	}

	return(0);
}

/*
 * relranges(vp)
 *
 * frees machine-dependent memory ranges containing text for hardware
 * platforms we aren't on.
 * All RELMEM ranges are stored [FIRST,LAST) where FIRST may be and LAST may
 * not be freed.  There are cases with the region size less than or equal to
 * one page where LAST will be equal to or one less than FIRST.  That's OK.
 */

static
relranges(vp)
struct vmsidata *vp;
{
	int	k;
	uint	p, pfts, pfte;

#ifdef _DEBUG
	/* for asserts */
	SCB_MPLOCK(0);
#endif
	for (k = 0; k < vmint_num(VMINT_RELMEM); k += 1)
	{
		pfts = vmint_start(VMINT_RELMEM,k);
		pfte = vmint_end(VMINT_RELMEM,k);

		for (p = pfts; p < pfte; p++)		/* [FIRST,LAST) */
		{
			v_relframe(KERSIDX,p);
		}
	}
#ifdef _DEBUG
	SCB_MPUNLOCK(0);
#endif

	return(0);
}

/* 
 * initializes dmaptab.
 * the kth bit of dmaptab[x] is a one if the byte x contains
 * a  sequence of k consecutive zeros, k = 1,2,..8 (where
 * leftmost bit of char is bit 1).
 */
initdmaptab()
{
	extern unsigned char dmaptab[256];
	unsigned int mask, tmask;
	int k,n,j,shift;

	dmaptab[255] = 0;
	dmaptab[0] = ONES;

	/* other than  0 and 255, all have sequences of zeros
	 * between 1 and 7.
	 */
	for (k = 1; k < 255; k++)
	{
		for (n = 7; n > 1; n--)
		{
			shift = 8 - n;
			mask = (ONES << shift) & 0xff;
			tmask = mask;
			for (j = 0; j <= shift; j++) 
			{
				if ((tmask & k) == 0)
					goto next;
				tmask = tmask >> 1;
			}
		}
		next : dmaptab[k] = (j <= shift) ? mask : 0x80;
	}

}

/*
 * initialize tunable variables. tunable variables with negative or
 * invalid values will be initialized as a function of the memory size.
 */
static
initvars()
{
	int goodpages;
	extern int vmm_pfhdata;

	goodpages = vmker.nrpages - vmker.badpages;

	if (vmvars.minfree <= 0 || vmvars.maxfree <= 0 ||
	    vmvars.minfree > vmvars.maxfree) {
		vmvars.maxfree = MIN(goodpages / 128, 128);
		vmvars.minfree = vmvars.maxfree - 8;
	}
	pf_maxfree = vmvars.maxfree;
	pf_minfree = vmvars.minfree;

	/* we calculate maxperm to be 80% and minperm to be 20%
	 * of the non-long-term pinned pages.  We swag # of 
	 * long-term pinned pages to be 4MB.
	 */
	if (vmvars.minperm < 0 || vmvars.maxperm < 0) {
		vmvars.minperm = (goodpages - 1024) / 5;  
		vmvars.maxperm = (4 * (goodpages - 1024)) / 5;  
	}
	vmker.maxperm = vmvars.maxperm;
	pf_minperm = vmvars.minperm;

	/* set maxclient to 80% of maxperm.
	 * vmker.maxclient = (4 * vmker.maxperm) / 5;
	 * NOTE: maxperm is no longer used as an upper limit
	 *	 on the number of file pages in memory (repaging is
	 *	 used).  we set maxclient to 80% of the non-long-term
	 *	 pinned pages so that machines w/o a local filesystem
	 *	 (diskless clients) have a reasonably large file cache.
	 */
	vmker.maxclient = vmvars.maxperm;

	/* set the number of reserved non-pinable page frames
	 * (20% of real memory). 
	 */
	if (vmvars.pfrsvdblks <= 0)
		vmvars.pfrsvdblks = goodpages / 5;
	vmker.pfrsvdblks = vmvars.pfrsvdblks;

	if (vmvars.npswarn <= 0 || vmvars.npskill <= 0 ||
	    vmvars.npswarn <= vmvars.npskill) {
		vmvars.npswarn = 512;
		vmvars.npskill = 128;
	}
	pf_npswarn = pf_nextwarn = vmvars.npswarn;
	pf_adjwarn = MAX(pf_npswarn/99,8);
	pf_npskill = vmvars.npskill;
	pf_adjkill = MAX(pf_npskill/16,8);

	if (vmvars.minpgahead < 0 || vmvars.maxpgahead < 0 ||
	    vmvars.minpgahead > vmvars.maxpgahead) {
		vmvars.minpgahead = MINPGAHEAD;
		vmvars.maxpgahead = MAXPGAHEAD;
	}
	pf_minpgahead = vmvars.minpgahead;
	pf_maxpgahead = vmvars.maxpgahead;

	/* set the maximum number of blocks allocated from
	 * a single paging device before allocating from
	 * a new device.  larger values give more sequential
	 * disk block allocation while smaller numbers
	 * schedule paging i/o over more devices.
	 */
	if (vmvars.maxpdtblks < 0)
		vmvars.maxpdtblks = 4;
	pf_maxpdtblks = vmvars.maxpdtblks;

	/* set the minimum number of blocks to steal in v_fblru().
	 */
	if (vmvars.numsched < 0)
		vmvars.numsched = 4;
	pf_numsched = vmvars.numsched;

	/* set configurable values for i/o pacing.
	 */
	vmker.maxpout = v.v_maxpout;
	vmker.minpout = v.v_minpout;

	/* set offset of pfdhata for performance tools.
	 */
	vmm_pfhdata = (int)&vmmdseg.pf & 0x0fffffff;

	/*
	 * Set maximum number of pages to delete in one critical section.
 	 */
	if (vmvars.pd_npages <= 0 || vmvars.pd_npages > MAXFSIZE/PSIZE)
		vmvars.pd_npages = MAXFSIZE/PSIZE;
	vmker.pd_npages = vmvars.pd_npages;
}

/* TEMPORARY routine for fetch-protect/store problem.
 * This can be removed when we no longer need to support the
 * hardware level that has this problem.
 *
 * initukern()
 * For machines with the h/w problem we can't fetch protect anything
 * in the user's address space so we create a shadow of the kernel
 * segment that consists of just the user-readable pages of the
 * kernel segment.  The variable vmker.ukernsrval contains the key 1
 * segment register value for this shadow segment.
 * Any data in the kernel segment that is readable with key 1 must be
 * copied to the shadow segment using routine ukerncopy() -- currently
 * this data consists of the first 3 pages, the svc table, and the
 * trace variable Trconflag. 
 * We also set pf_kerkey to UTXTKEY so that the system part of a
 * working storage segment is not fetch protected.
 *
 * For machines without the problem, we don't create the shadow
 * segment and we just set vmker.ukernsrval to the key 1 value of the
 * normal kernel segment, and we set pf_kerkey to KERKEY.
 * Calls to ukerncopy() just copy the data to itself in the kernel segment.
 */
int
initukern()
{
	int sid;
	uint bytes, target;
	extern int noprotect;

	/* The variable vmker.nofetchprot is set by init_config()
	 * to a non-zero value if we are running on a machine
	 * with the h/w problem.
	 * The variable noprotect is provided as a means of patching
	 * the kernel to allow applications which incorrectly reference
	 * no-access memory to run without core-dumping (i.e. by setting
	 * noprotect to a non-zero value nothing is fetch-protected).
	 */
	if (vmker.nofetchprot || noprotect)
	{
		/* init key for system part of a working storage
		 * segment to user-readable.
		 */
		pf_kerkey = UTXTKEY;

		/* create segment with user key 1 readable pages.
		 * put sreg val for it in vmker.
		 */
		vms_create(&sid,V_WORKING|V_SYSTEM|V_UREAD, NULL,
			   SEGSIZE, SEGSIZE, 0);
		vmker.ukernsrval = SRVAL(sid,1,0);

		/* copy pages up before fetchprot_page.
		 */
		ldsr(TEMPSR,SRVAL(sid,0,0));
		bytes = (uint)&fetchprot_page;
		target = TEMPSR << L2SSIZE;
		bcopy(0,target,bytes);

		/* page fix the pages 
		 */
		pin(target,bytes);
	}
	else
	{
		/* Machine does not have h/w problem.
		 */
		vmker.ukernsrval = SRVAL(SRTOSID(vmker.kernsrval),1,0);
		pf_kerkey = KERKEY;
	}
}

/*
 * initrpaging()
 *
 * initializes repaging data structures.
 *
 * No lock needed, single execution thread.
 */

int
initrpaging()
{
	int k, size;

	/* determine size of repaging tables.
	 */
	size = 1 << loground(vmker.nrpages - vmker.badpages);
	size = MIN(size,MAXREPAGE);

	/* put all entries on hash chain . entry 0 and size - 1 not used.
	 */
	for (k = 0; k < size - 1; k++)
	{
		rphtab[k] = k;
		rpt_tag(k) = 0;
		rpt_pno(k) = k;
		rpt_next(k) = 0;
	}
	rphtab[size - 1] = 0;

	/* init control values and counters.
	 */
	vmker.rptfree = 0;
	vmker.sysrepage = 0;
	vmker.rptsize = size;
	for (k = 0; k < RPTYPES; k++)
		pf_rpgcnt[k] = 0;

	/* set decay rate (percent) for repaging
	 * counts.
	 */
	vmker.rpdecay = 90;
}

/*
 * init_vmmlocks()
 *
 * Initialize VMM MP locks now that lock instrumentation is available.
 * We assume that the lockwords can safely be overwritten at this point.
 */
init_vmmlocks()
{
	int sidx;

	/*
	 * Initialize vmm_lock_lock.
	 */
	lock_alloc(&vmm_lock_lock, LOCK_ALLOC_PIN, VMM_LOCK_VMM, -1);
        simple_lock_init(&vmm_lock_lock);

#ifdef _VMM_MP_EFF
	/*
	 * Initialize global locks.
	 */
	lock_alloc(&vmker_lock, LOCK_ALLOC_PIN, VMM_LOCK_VMKER, -1);
        simple_lock_init(&vmker_lock);
	lock_alloc(&lw_lock, LOCK_ALLOC_PIN, VMM_LOCK_LOCKWORD, -1);
        simple_lock_init(&lw_lock);
	lock_alloc(&pdt_lock, LOCK_ALLOC_PIN, VMM_LOCK_PDT, -1);
        simple_lock_init(&pdt_lock);
	lock_alloc(&vmap_lock, LOCK_ALLOC_PIN, VMM_LOCK_VMAP, -1);
        simple_lock_init(&vmap_lock);
	lock_alloc(&ame_lock, LOCK_ALLOC_PIN, VMM_LOCK_AME, -1);
        simple_lock_init(&ame_lock);
	lock_alloc(&rpt_lock, LOCK_ALLOC_PIN, VMM_LOCK_RPT, -1);
        simple_lock_init(&rpt_lock);
	lock_alloc(&alloc_lock, LOCK_ALLOC_PIN, VMM_LOCK_ALLOC, -1);
        simple_lock_init(&alloc_lock);
	lock_alloc(&apt_lock, LOCK_ALLOC_PIN, VMM_LOCK_APT, -1);
        simple_lock_init(&apt_lock);

	/*
	 * Initialize PG lock for initial paging device (this
	 * wasn't done in vm_defineps() because instrumentation
	 * isn't available during vmsi at that point).
	 */
	lock_alloc(&lv_lock(0), LOCK_ALLOC_PIN, VMM_LOCK_LV, 0);
	simple_lock_init(&lv_lock(0));

	/*
	 * Initialize SCB locks for pre-allocated SCBs (this wasn't
	 * done in initvmdseg() because instrumentation isn't available
	 * during vmsi at that point).  We pass the REINIT flag to
	 * lock_alloc() so it will allocate a lock instrumentation
	 * structure if needed.
	 */
        for (sidx = 0; sidx < pf_hisid; sidx += 1)
        {
		lock_alloc(&scb_lock(sidx), LOCK_ALLOC_PIN | LOCK_ALLOC_REINIT,
			   VMM_LOCK_SCB, sidx);
        }

#endif /* _VMM_MP_EFF */

}
