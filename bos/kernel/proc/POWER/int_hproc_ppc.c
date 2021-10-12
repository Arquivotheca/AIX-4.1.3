static char sccsid[] = "@(#)65	1.3  src/bos/kernel/proc/POWER/int_hproc_ppc.c, sysproc, bos411, 9428A410j 4/14/94 12:33:57";
/*
 * COMPONENT_NAME: (SYSPROC) Process Management
 *
 * FUNCTIONS: encode_sim_upmca mac_check_gen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _POWER_PC

#include <sys/types.h>
#include <sys/iplcb.h>
#include <sys/rosinfo.h>
#include <sys/vmker.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/errids.h>
#include <sys/dbg_codes.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>

extern ulong ipl_cb;			/* ipl control block pointer	*/
extern time_t time;			/* system time 			*/

struct {
	struct err_rec0 err_rec;
	uint code;
	uint time;
	uint srr0;
	uint status;
	uint address;
}mac_checklog = { ERRID_MACHINECHECK, "sysplanar0", 0, 0, 0, 0, 0 };

struct io_map nio_map = { 0 };

/*
 * NAME: mac_check_gen
 *
 * FUNCTION:
 *	This is intended to be model independent machine check handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This program is called from the first level interrupt handler,
 *	     mc_flih.
 *	It cannot pagefault.
 *
 * RETURNS: None
 */
void
mac_check_gen( int srr0,		/* address of failing instr     */
	   int srr1,			/* msr                          */
	   struct mstsave *mst)		/* mst save area                */
{
	/* 
	 * Log a machine check
	 */
	mac_checklog.code = (srr1 & 0xFFFF) | 0x81000000 | 'X';
	mac_checklog.time = time;
	mac_checklog.srr0 = srr0;
	mac_checklog.status = 0;
	mac_checklog.address = 0;
	errsave(&mac_checklog, sizeof(mac_checklog));

	/*
	 * halt the machine
	 */
	halt_display(mst, 0x207, (int)DBG_MCHECK);
}


/*
 * NAME: encode_sim_up_mca
 *
 * FUNCTION: converts a memory address to a sims for diagnostic
 *	purposes for the up_mca box
 *
 * RETURNS: ASCII value of sim pair where error occured
 */
int
encode_sim_up_mca(
	uint addr)		/* real memory address of fault */
{
	struct ipl_cb *iplcbp;
	struct ram46_data *ramp;
	int index;
	uint phys_addr;
	int simm_pair;

	ASSERT(__rs6k_up_mca());

	/* Get pointer to ram post information left by ROS in the IPL
	 * control block
	 */
	iplcbp = (struct ipl_cb *)vmker.iplcbptr;
	ramp = (struct ram46_data *)((uint)iplcbp +
					iplcbp->s0.ram_post_results_offset);

	/* 
	 * Check that there is memory at the failing address.
	 * The algorithm behind this is based on the SIMM addressing model
	 * for the Rainbow box.  The maximum amount of real memory allowable in
	 * the Rainbow box is 256MB, which is divided into 4 64MB extents.  We
	 * divide the real address by 64Meg to determine which extent it was
	 * accessing, and then make sure there was memory present at that
	 * extent
	 */
	index = (addr >> 26) & 0x3;	/* divide address by 64MB */
	if ((ramp->mem_config_table[index] & 0x7F) == 0)
		/*
		 * if the extent size (represented by bits 25-31 of the
		 * memory config table entry) is 0, then no memory is 
		 * present for this address
		 */
		return('X');

	/*
	 * Convert the real address to a physical address (as seen by
	 * the MCU, which may invert bits 4 and/or 5).  We use the 
	 * physical extent ID available in bits 20 and 21 of the of the 
	 * memory config table entry.
	 */
	phys_addr = (ramp->mem_config_table[index] & 0xC00) << 16;
	phys_addr |= (addr & 0xF3FFFFFF);	/* replace bits 4 and 5 */

	/*
	 * The SIMM pair is identified as follows:
	 *
	 * Phys address  SIMM Pair     Planar Location
	 *   4  28                                       
	 * ---------     ---------     ---------------
	 *   0   0         0 & 1          J1 & J2
	 *   0   1         2 & 3          J3 & J4
	 *   1   0         4 & 5          J5 & J6
	 *   1   1         6 & 7          J7 & J8
	 *
	 * So, each of the 4 states of bits 4 and 28, 
	 * yield the corresponding SIMM pair number, 0 - 3
	 */
	simm_pair = (((phys_addr >> 3) & 0x01)  | ((phys_addr >> 26) & 0x2));

	/*
	 * return the simm_pair number in ASCII form.  This becomes the
	 * second byte of sense data for the machine check error log entry.
	 * Error Analysis looks at this byte and calls out the memory slot
	 * location as follows:
	 * 
	 *   SIMM_pair		Slots
	 *   ---------		-----
	 *   '0' (0x30)		A & B
	 *   '1' (0x31)		C & D
	 *   '2' (0x32)		E & F
	 *   '3' (0x33)		G & H
	 */
	return('0' + simm_pair);
}
#endif /* _POWER_PC */

