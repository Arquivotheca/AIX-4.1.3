static char sccsid[] = "@(#)64	1.1  src/bos/kernel/proc/POWER/int_hproc_pwr.c, sysproc, bos411, 9428A410j 3/12/93 19:09:51";
/*
 * COMPONENT_NAME: (SYSPROC) Process Management
 *
 * FUNCTIONS: encode_creg, encode_sim
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef _POWER_RS

#include <sys/types.h>
#include <sys/machine.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/sysmacros.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/lldebug.h>
#include <sys/dbg_codes.h>
#include <sys/dump.h>
#include <sys/var.h>
#include <sys/buid0.h>
#include <sys/errids.h>
#include <sys/vmker.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>

extern ulong ipl_cb;			/* ipl control block pointer	*/
extern mfmsr();				/* asm program to read MSR	*/
extern mtmsr();				/* asm program to write MSR	*/


/*
 * POWER Implementation specific defines
 */
#define MAX_SIM_RSC		0x2000000	/* Larget Salon Sim Pair*/

/* processor complex address - system registers */
#define BUID0			0x80000000 	   
#define CREG_BASE		0x1040

/*
 * Turns data translation off/on.
 *
 * At this time it is better to use the real memory copy of the
 * IPLCB, since the virtual memory copy may not be available. 
 *
 */

#define  XLATE_OFF(x, m)						\
	{								\
    		m  = mfmsr();						\
    		m &= ~MSR_DR;						\
    		x  = mtmsr(m);						\
	}

#define  XLATE_ON(x, m)							\
	{								\
    		m  = mfmsr();						\
    		m |= MSR_DR;						\
		x  = mtmsr(m);						\
	}

/*
 * Turn off all interrupts by disabling: 
 * 	FP Exceptions, Alignment Exceptions, Machine Checks,
 *	external interrupts, FP instructions
 */

#define  ALL_INTS_OFF(x, m)						\
	{								\
    		m  = mfmsr();						\
    		m &= ~(MSR_EE | MSR_FP | MSR_ME | MSR_FE);		\
		x  = mtmsr(m);						\
	}

void halt_display();


/*
 * NAME: encode_sim
 *
 * FUNCTION: converts a memory address to a sims for diagnostic
 *	purposes
 *
 * RETURNS: ASCII value of sim pair where error occured
 */
int
encode_sim(
	uint addr)
{
	struct ipl_cb *iplcbp;
	struct ram_data *ramp;
	int found;
	uint start;
	uint size;
	volatile uint *sccrp;
	int i;
	uint sccr;

	ASSERT(__power_rsc());

	/* Get pointer to ram post information left by ROS in the IPL
	 * control block
	 */
	iplcbp = (struct ipl_cb *)vmker.iplcbptr;
	ramp = (struct ram_data *)((uint)iplcbp +
					iplcbp->s0.ram_post_results_offset);

	/* Search through the memory configuration table.  Check that there
	 * is memory at the failing address
	 */
	found = 0;
	for (i = 0; i < 8; i++)
	{
		size = ramp->cfg_tbl_array[i][0] & 0xfffffff;
		start = ramp->cfg_tbl_array[i][1];

		/* set flag if the address is in logical memory range
		 */
		if (addr >= start && addr < (start + size))
			found = 1;
	}

	/* If there is not memory at the address return NO SIM
	 */
	if (!found)
		return('X');

	/* Address lines 6 and 7 can be inverted internally by RSC with
	 * the SCCR register.  If this is the case invert the bit(s) in
	 * the real address
	 */
	sccrp = (volatile uint *)io_att(BUID0, SCCR);
	sccr = *sccrp;
	io_det(sccrp);
	if (sccr & SCCR_INVA7)
		addr ^= 0x01000000;
	if (sccr & SCCR_INVA6)
		addr ^= 0x02000000;

	/* Return the sim pair number for which the error occured
	 */
	return('0' + addr/MAX_SIM_RSC);

}

/*
 * NAME: encode_creg
 *
 * FUNCTION: This program encodes the memory configurationa register.
 *
 * EXECUTION ENVIRONMENT:
 *	This program is called from the machine check handler, mac_check.
 *	It cannot pagefault.
 *
 * RETURNS: 
 *	An integer value that contains the character representation
 *	of the memory slot where the error occurred.
 */

encode_creg(int mear_val) 		/* MEAR value from buid0	*/
{
	register caddr_t buid0addr;	/* buid 0 effective address     */
	register ulong cache_size;	/* ipl key position		*/
	volatile struct ipl_cb *iplcb_ptr;/* ipl control block pointer	*/
	volatile struct ipl_info *info_ptr;/* ipl info pointer		*/
	volatile struct ipl_directory *dir_ptr;/* ipl directory pointer	*/
	int creg[16];			/* config reg save area         */
	int i;				/* index for loops		*/
	int x, m;			/* turning off data xlate - msr */
	int start, end;			/* bounds for memory extents    */
	int cpu;			/* cpu type save area           */
	int ends_8c;			/* memory loc ends in 8 or C    */
	int tabletop;			/* llano model			*/
	register ulong *conreg_ptr;	/* pointer to config regs       */

	/* turn data translation off -> using ipl_cb real memory address*/
	XLATE_OFF(m,x);

	/* get cache line size from ipl_cb */
	iplcb_ptr  = (volatile struct ipl_cb *)ipl_cb;
	dir_ptr    = &(iplcb_ptr->s0);
	info_ptr   = (struct ipl_info *)(dir_ptr->ipl_info_offset + ipl_cb);
	cache_size = (ulong)(info_ptr->cache_line_size);
	tabletop   = info_ptr->IO_planar_level_reg & 0xf0000000 == 1<<31;

	XLATE_ON(m,x);

	if (cache_size == 64)
		cpu = 1;
	else if (cache_size == 128)
		cpu = 2;
	else
		return ('X');

	/* read configuration registers from buid0 */
	buid0addr = io_att(BUID0,0);
	conreg_ptr = (ulong *) (buid0addr + CREG_BASE);
	for (i=0; i<8; i++, conreg_ptr++)
		creg[i] = *conreg_ptr;
	io_det(buid0addr);

	/* 
	 * Encode memory configuration register.
	 * 
	 * Upper half: Base Address
	 * Lower half: Extent Size Code (bit mask for valid bits in Base addr)
	 */
	for (i=0; i<16; i++)
	{  
		if (creg[i] & 0x0000FFFF)
		{
			start = creg[i] & 0xFFFF0000;
			end = start + (((creg[i] ^ 0x0000FFFF) + 1) << 16);
			if ((mear_val >= start) && (mear_val < end))
			{
				if (cpu == 1)
				{
					static char m[8] = 
					{ 'H','D','F','B','G','C','E','A' };

					if (tabletop)
					{
						static char m[8] = { 'B','C' };
						return(m[i/2]);
					}
						
					/*
					 * Extents are matched to slots: 
					 * 	0,1 = H
					 * 	2,3 = D
					 * 	etc
					 */

					return (m[i/2]);
				}
				else if (cpu == 2)
				{
					/* 
					 * Slots are matched: 
					 *     	0,1,2,3 = H or D
					 */
					ends_8c = (mear_val & 0x0008) || 
					     	  (mear_val & 0x000C);
					switch(i/4){
					case 0 : return( ends_8c ? 'H' : 'D' ); 
					case 1 : return( ends_8c ? 'F' : 'B' ); 
					case 2 : return( ends_8c ? 'G' : 'C' ); 
					case 3 : return( ends_8c ? 'E' : 'A' ); 
					}
				}
			}
		}
	}
	return('X');
}

#endif /* _POWER_RS */

