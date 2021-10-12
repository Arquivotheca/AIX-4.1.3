static char sccsid[] = "@(#)54	1.4  src/bos/kernel/ios/POWER/pio_ppc.c, sysios, bos41J, 9520B_all 5/18/95 18:39:53";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	iomem_att_ppc, iomem_det_ppc, iomem_att_601, iomem_det_601
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */                                                                   

#ifdef _POWER_PC
#include <sys/types.h>
#include <sys/ioacc.h>
#include <sys/syspest.h>
#include <sys/vmker.h>
#include <sys/machine.h>
#include <sys/mstsave.h>
#include <sys/adspace.h>
#include <sys/param.h>
#include <sys/seg.h>
#include <sys/ppda.h>
#include <sys/inline.h>

/*
 * Table of registerd BIDs.  Its index is a function of BID.
 */ 
struct businfo bid_table[MAX_BID_INDEX] = { 0 };

#define WIMG_IGM	0x7		/* move to vmker.h */

#ifdef DEBUG
/*
 * Table to convet mapping size to BAT BL bits.  Since size is
 * a power of two a count leading zero can be used to generate
 * an index into this table.
 */
static short bl_tab[] = {
	BT_128K, BT_256K, BT_512K, BT_1M, BT_2M, BT_4M,
	BT_8M, BT_16M, BT_32M, BT_64M, BT_128M, BT_256M };
#endif

/*
 * macros to set, clear, and test a bit value
 */
#define SET_BIT(value, bit) (value) = (value) | ((uint)0x80000000 >> (bit))
#define CLEAR_BIT(value, bit) \
		(value) = (value) & ~((uint)0x80000000 >> (bit))
#define BIT_VALUE(value, bit) \
		((value) & ((uint)0x80000000 >> (bit)))


#ifdef _POWER_601

/*
 * NAME: iomem_att_601
 *
 * FUNCTION:
 *	This is the 601 version of iomem_att().  It uses BUID 7F
 *	to setup addressability to IO space
 *
 *      When BUID 7F is used, iomem_att() is simply a front end to
 *      vm_att().  The segment register value and offset is
 *      calculated and vm_att() is called.  The readonly flag is
 *      ignored when using BUID 7F mapping.
 *
 *      The ioalloc word is used as a counter to track attaches and
 *      detaches.
 *
 * NOTES:
 *	This is a documented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *	This service is reached by the processor branch table
 *
 *	This may execute in the process or interupt environment
 *
 * RETURNS:
 *	32 bit effective IO address
 */
void *
iomem_att_601(
	struct io_map *iop)		/* IO region to map */
{
	struct mstsave *mstp;		/* current mst save area */
	int bid;			/* BID to map */
	int index;			/* index to BID table */
	uint raddr;			/* real address to map */

	/*
	 * track then number of IO regions mapped.  Enforce
	 * the maximum number of IO mappings allowed by Power PC
	 * BAT version of this function
	 */
	mstp = CSA;
	assert(mstp->ioalloc < NUM_KERNEL_BATS);
	mstp->ioalloc++;

	bid = iop->bid;
	index = BID_INDEX(bid);

	/*
	 * validate input parameters.  This service is performance
	 * critical, so most of checks are DEBUG only.
	 */
	ASSERT(iop->key == IO_MEM_MAP);
	ASSERT(index < MAX_BID_INDEX);
	ASSERT(BID_TYPE(bid_table[index].bid) == BID_TYPE(bid));
	ASSERT(BID_NUM(bid_table[index].bid) == BID_NUM(bid));
	ASSERT(bid_table[index].num_regions > BID_REGION(bid));
	ASSERT(iop->size >= PAGESIZE && iop->size <= SEGSIZE);
	ASSERT(__power_601());
	ASSERT(!__rs6k_smp_mca());

	/*
	 * determine real address to map, by adding bus address
	 * to the region's registered base address
	 */
	raddr = (uint)(bid_table[index].ioaddr[BID_REGION(bid)]);
	raddr += (uint)(iop->busaddr);

	/*
	 * Call vm_att to do the mapping
	 */
	return(vm_att(BUID_7F_SRVAL(raddr), raddr & (SEGSIZE-1)));
}

/*
 * NAME: iomem_det_601
 *
 * FUNCTION:
 *	This is the 601 version of the iomem_det kernel service.
 *      For BUID 7F mapping this service simply decrements a count
 *      and calls vm_det().
 * 	
 * NOTES:
 *	This is a documented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *	This service is reached by the processor branch table
 *
 *	This may execute in the process or interupt environment
 *
 * RETURNS:
 *	None
 */
void
iomem_det_601(
	void *busaddr)				/* effective address to unmap */
{
	struct mstsave *mstp;			/* current mst */

	mstp = CSA;
	mstp->ioalloc--;

	ASSERT(mstp->ioalloc >= 0);
	ASSERT(__power_601());
	vm_det(busaddr);
}

#endif /* _POWER_601 */

/*
 * NAME: iomem_att_ppc
 *
 * FUNCTION:
 *	This is the Power PC (non 601) version of iomem_att.
 *      Only data BATs are used by this design.  When BATs are used,
 *      iomem_att() calls vm_att() to allocate and initialize a
 *      kernel segment.  The segment is set to NULLSEGVAL.  This will
 *      produce a fault if an access is attempted in an area not
 *      covered by the BAT (size is less than 256 Meg).
 *
 *      The ioalloc word is used as an allocation mask.  Bit 0
 *      represents BAT0.  On each iomem_att(), a new BAT is
 *      allocated and initialized.  The lowest BAT number is
 *      allocated first.  If BAT 3 is allocated the system asserts.
 *      The ioalloc bit must be stored in the mst before loading BAT
 *      registers (for state_save()).
 *
 *      state_save_pc() and resume_pc() conditionally save/restore
 *      data BATS from the mst if ioalloc is non zero.
 * 
 * NOTES:
 *	This is a documented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *	This service is reached by the processor branch table
 *
 *	This may execute in the process or interupt environment
 *
 * RETURNS:
 *	32 bit Effective IO address
 */

void *
iomem_att_ppc(
	struct io_map *iop)		/* IO region to map */
{
	uint eaddr;			/* effevtive address mapped to */
	uint busaddr;			/* buss address to map */
	uint raddr;			/* real address of IO */
	uint offset;			/* offset to start of reqested memory */
	int bid;			/* BID of IO */
	struct mstsave *mstp;		/* current MST */
	int index;			/* table index of BID */
	int bat_num;			/* BAT allocated for mapping */
	uint batu;			/* upper bat contents */
	uint batl;			/* lower bat contents */
	uint bat_size;			/* size encoding for BAT */
	uint bat_addr;			/* real address to map */
	int i;
	uint size;

	mstp = CSA;

	bid = iop->bid;
	index = BID_INDEX(iop->bid);

	/*
	 * validate input parameters.  This service is performance
	 * critical, so checks are DEBUG only.
	 */
	ASSERT(iop->key == IO_MEM_MAP);
	ASSERT(index < MAX_BID_INDEX);
	ASSERT(BID_TYPE(bid_table[index].bid) == BID_TYPE(bid));
	ASSERT(BID_NUM(bid_table[index].bid) == BID_NUM(bid));
	ASSERT(bid_table[index].num_regions > BID_REGION(bid));
	ASSERT(iop->size > 0 && iop->size <= SEGSIZE);
	ASSERT(!__power_601());

	/*
	 * calculate the real address of the IO region
	 */
	raddr = (uint)bid_table[index].ioaddr[BID_REGION(bid)] +
							(ulong)iop->busaddr;


#ifdef DEBUG
	/*
	 * Find the minimum mapping size that can cover the region.
	 */
	size = MIN_BAT_SIZE;
	i = 0;
	while(1)
	{
		/*
		 * Round bus address to propper alignment.  An offset is added
		 * to the effective address retuned, to hide the rounding
		 */

		offset = raddr & (size-1);
		bat_addr = raddr - offset;

		if (((raddr + iop->size) <= (bat_addr + size)) ||
						(size == MAX_BAT_SIZE))
		{
			break;
		}
		size = size << 1;
		i++;
	}
	bat_size = bl_tab[i];

#else /* DEBUG */

	/*
	 * Round bus address to propper alignment.  An offset is added
	 * to the effective address retuned, to hide the rounding
	 */
	bat_size = BT_256M;
	offset = raddr & (MAX_BAT_SIZE-1);
	bat_addr = raddr - offset;

#endif /* !DEBUG */

	/*
	 * allocate effective address slot for mapping.  Initialize
	 * the segment register to NULLSEGVAL (to force a fault if
	 * accessed)
	 */
	eaddr = vm_att(NULLSEGVAL, offset);

	/*
	 * allocate a BAT to use.  setting ioalloc to non zero
	 * causes state_save()/resume() to begin saving and restoring
	 * BAT values.
	 */
	bat_num = clz32(~mstp->ioalloc);
	SET_BIT(mstp->ioalloc, bat_num);



	/*
	 * load the appropriate bat registers to set up mapping.
	 * The compiler must (will) have update ioalloc before
	 * loading registers
	 */
	batu = DBATU(eaddr, bat_size, BT_VS);
	batl = DBATL(raddr, WIMG_IGM,
		(iop->flags & IOM_RDONLY) ? BT_RDONLY : BT_WRITE);

	switch(bat_num)
	{
		case 0:
			mtdbat0l(batl);
			mtdbat0u(batu);
			break;
		case 1:
			mtdbat1l(batl);
			mtdbat1u(batu);
			break;
		case 2:
			mtdbat2l(batl);
			mtdbat2u(batu);
			break;
		default:
			assert(0);
	}

	isync();			/* synchronization */

	return(eaddr);
}

/*
 * NAME: iomem_det_ppc
 *
 * FUNCTION:
 *	This is the Power PC (non 601) version of iomem_det.
 *      When BATs are used, iomem_det() first must find the correct
 *      BAT to invalidate.  This is accomplished by examining BATs
 *      (smallest to largest) and comparing the effective address
 *      mapped, to the effective address being detached.  If no
 *      match is found the kernel will assert.
 *
 *      The service next invalidates the BAT and zeros its
 *      allocation bit in ioalloc.  Last iomem_det() clears the
 *      segment register allocation bit in mst.adspace (this is
 *      equivalent to calling vm_det().
 *
 * NOTES:
 *	This is a documented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *	This service is reached by the processor branch table
 *
 *	This may execute in the process or interupt environment
 *
 * RETURNS:
 *	None
 */
void
iomem_det_ppc(
	uint eaddr)			/* IO address to free */
{
	int segno;			/* segment number to free */
	int bat_num;			/* BAT to free */
	struct mstsave *mstp;		/* current save area */

	ASSERT(!__power_601());
	isync();			/* synchronization */

	segno = eaddr >> SEGSHIFT;	/* get segment number to free */

	/*
	 * Find the bat to free by checking effective address contained
	 * in the BAT.  Invalidate the BAT.  Also check that the BAT
	 * is used as a kernel BAT.
	 */
	if (BAT_ESEG(mfdbat0u()) == segno)
	{
		ASSERT((mfdbat0u() & (BT_VS|BT_VP)) == BT_VS);
		bat_num = 0;
		mtdbat0u(0);
	}
	else if (BAT_ESEG(mfdbat1u()) == segno)
	{
		ASSERT((mfdbat1u() & (BT_VS|BT_VP)) == BT_VS);
		bat_num = 1;
		mtdbat1u(0);
	}
	else if (BAT_ESEG(mfdbat2u()) == segno)
	{
		ASSERT((mfdbat2u() & (BT_VS|BT_VP)) == BT_VS);
		bat_num = 2;
		mtdbat2u(0);
	}
	else
	{
		assert(0);
	}

	/*
	 * mark the BAT and segment register as free.  Since NULLSEGVAL
	 * was loaded at iomem_att() time there is nothing more to do.
	 * 
	 */
	mstp = CSA;
	ASSERT(BIT_VALUE(mstp->ioalloc, bat_num));
	ASSERT(BIT_VALUE(mstp->as.alloc, segno));
	CLEAR_BIT(mstp->ioalloc, bat_num);
	CLEAR_BIT(mstp->as.alloc, segno);

}

#endif /* _POWER_PC */
