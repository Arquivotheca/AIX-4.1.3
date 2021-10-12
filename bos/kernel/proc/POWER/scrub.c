static char sccsid[] = "@(#)39  1.7  src/bos/kernel/proc/POWER/scrub.c, sysproc, bos411, 9428A410j 4/13/94 14:32:11";
/*
 * COMPONENT_NAME: (SYSPROC) Process Management
 *
 * FUNCTIONS: scrubinit, scrubcfg, scrubclock 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/buid0.h>
#include <sys/errids.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/iplcb.h>
#include <sys/machine.h>
#include <sys/sysconfig.h>
#include <sys/systemcfg.h>
#include <sys/trchkid.h>
#include <sys/var.h>
#include <sys/vmker.h>


/* LOCAL VARIABLES */

#define SBSSR_ADDR 0x000010A0	/* Single Bit Signature/Syndrome Register */
#define SBSR_ADDR  0x000010A4	/* Single Bit Status Register		  */
#define	SBSR_HE	   0x80000000	/* Scrub - Hard Correctable ECC   	  */
#define	SBSR_SE	   0x40000000	/* Scrub - Soft Correctable ECC   	  */
#define	SBSR_CE	   0x20000000	/* DMA   - Correctable ECC 	   	  */
#define SBAR_ADDR  0x000010A8	/* Single Bit Address Error	          */
#define STVR_ADDR  0x000010B4	/* Scrub Timer Value register 		  */
#define SSAR_ADDR  0x000010B8	/* Scrub Start Address Register 	  */
#define SEAR_ADDR  0x000010BC	/* Scrub End Address Register 		  */

#define DIVIDE_BY_BYTES			3
#define BYTE_REMAINDER			0x7

enum bitmapmode { get, set };

static struct errlog {
	struct err_rec0 hdr;
	uint sbssr;
	uint sbsr;
	uint sbar;
} sb_errlog = { ERRID_CORRECTED_SCRUB, "SYSPROC", 0, 0, 0 };

#define LOG_THRESHOLD	3
#define SCRUB_ARRAY	16

static struct {
	ulong	addr;
	int	cnt;
} sb_cnt[SCRUB_ARRAY];

static void set_next_interval();
static void check_soft_errors();
static int process_bitmap();
static struct cfgncb scrub_cfgcb;	/* config notification ctl blk */
static struct ipl_cb *ipl_cb_ptr;
static struct ipl_directory *dir_ptr;
static uint bitmapsize, bitmapaddr;
static uint bitmapblocksize, bmblksizebitct;


/*
 * MODULE DESCRIPTION:
 *
 *	Memory scrubbing is used to remove soft errors from storage on a
 *	periodic basis.  Memory scrubbing is performed by the hardware
 *	when requested via the Scrubbing Control Registers.  The minimum
 * 	amount of memory that can be scrubbed is equivalent to one cache
 *	line in size.
 *
 *	Memory scrubbing can be done on any portion of storage except ROS 
 *	and will not alter the contents of storage which is also the target 
 *	of DMA or cache activity.  Memory scrubbing is the lowest priority
 *	activity, therefore, if DMA or cache activity is high, scrubbing 
 *	may be delayed signficantly.
 *
 *	Any errors other than single bit errors detected while scrubbing
 *	will set the External Error registers, stop scrubbing and raise
 *	an External Check.  Single bit errors will set the SBSR, SBAR, and
 *	SBSSR registers.  Single bit errors will not stop scrubbing or
 *	cause an interrupt unless in diagnostic mode.  Note we do not run
 *	in diagnostic mode.
 *
 *	The following are the Scrubbing Control Registers:
 *
 *	Scrub Start Address Register (SSAR) specifies the address of the 
 *	first line of storage to begin scrubbing.  The address is incremented
 *	by the hardware each time a line is successfully scrubbed until that
 *	address exceeds the value in the SEAR.
 *
 *	Scrub End Address Register (SEAR) specifies the address of the last
 *	line of storage to be scrubbed during the active scrubbing cycle.  The
 *	least signficant bit is used to start the scrubbing activity.  It is 
 *	turned off by the hardware when the specified segment has been scrubbed
 *	and by software to disable scrubbing.
 *
 *	Scrub Timer value Register (STVR) contains a 16 bit value indicating 
 *	the number of clock cycles to wait between lines, while scrubbing.
 *
 *	Monitoring For Single Bit Errors.
 *
 *	By periodically examining the Single Bit Status Register for some
 *	single bit errors, some future double bit errors may be avoided via
 *	Bit Steering, scrubbing, or marking the pages bad.  Our current 
 *	implementation marks the pages bad in the bad block Map (iplcb) so  
 *	we don't stumble on the same errors each pass of scrubbing.  We don't
 *	bit steer, remove pages from the free list, or kill processes 
 *	containing bad memory.
 *
 *	The Single Bit Status Register (SBSR) is a 32 bit read only register
 *	that indicates the source and type of the highest priority single bit 
 *	error which has occurred sincle this register was last read.   
 *
 *	SBSR_HE		resulted from scrubbing 	Hard Correctable ECC	
 *	SBSR_SE		resulted from scrubbing 	Soft Correctable ECC	
 *	SBSR_SE		resulted from dma	 	Soft Correctable ECC	
 *
 *	The Single Bit Address Register (SBAR) is a 32 bit read only register
 *	that contains the 32 bit address of the highest priority single bit 
 *	error reflected in the SBSR since it was last cleared.
 *
 *	The Single Bit Signature/Syndrome Register (SBSSR) is a 32 bit read 
 *	only register that contains the signature or syndrome of the error
 *	reflected in SBSR.  The SBSR is cleared when it is read following a
 *	read of the SBSSR.
 *
 */

/*
 * NAME:	scrubinit
 *
 * FUNCTION:	
 *	Register the memory scrubbing's config notification 
 *	routine with sysconfig() so that the scrubbing can 
 *	be enabled/disabled by the user.
 *
 * EXECUTION ENVIRONMENT:  
 *	This routine is called at system initialization	by an entry 
 *	in a pointer-to-function array defined in <sys/init.h>
 *	This routine can pagefault.
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 */

void
scrubinit()
{
	int scrubcfg();
	ulong buid0_addr; 
	ulong *scrub_start_addr, *scrub_end_addr, *scrub_timer_addr;
	int intpri;
        struct ipl_info *iinfo_ptr;

	/* setup pointer to bitmap */
	ipl_cb_ptr = (struct ipl_cb *)vmker.iplcbptr;
	dir_ptr    = &(ipl_cb_ptr->s0);
	bitmapsize = dir_ptr->bit_map_size;
	bitmapaddr = (uint)ipl_cb_ptr + dir_ptr->bit_map_offset;
        iinfo_ptr = (struct ipl_info *)
                    ((uint)ipl_cb_ptr + dir_ptr->ipl_info_offset);
        bitmapblocksize  = iinfo_ptr->bit_map_bytes_per_bit;
	for (bmblksizebitct=0, intpri=bitmapblocksize>>1;
	     intpri != 0;
	     bmblksizebitct++, intpri >>= 1);

	/* only POWER_RS1 and POWER_RS2 support memory scrubbing */
	if (__power_set(POWER_RS1 | POWER_RS2))
	{

		/*
	 	 * Initialize scrubbing control registers.
	 	 */
		intpri = i_disable(INTMAX);
		buid0_addr = (ulong)io_att(BUID0, 0);

		scrub_start_addr = (ulong *)(buid0_addr + SSAR_ADDR);
		scrub_end_addr = (ulong *)(buid0_addr + SEAR_ADDR);
		scrub_timer_addr = (ulong *)(buid0_addr + STVR_ADDR);

		/* 
		 * First initialize end addr to ensure that scrubbing is 
		 * disabled. Registers are readonly while scrubbing is enabled.
		 */

		*scrub_end_addr = 0;
		*scrub_start_addr = 0;
		*scrub_timer_addr = 0xFFFF; 	/* set maximum timer value */

		io_det(buid0_addr);
		i_enable(intpri);
	}


	/*
	 * Register memory scrubbing's config notification routine on the
	 * cfgncb list.  This will cause the cfgncb.func routine to be 
	 * called when the sysconfig(SYS_SETPARMS) system call is invoked, 
	 * and will allow scrubbing to enable/disabled by the user. 
	 */
	scrub_cfgcb.cbnext = NULL;
	scrub_cfgcb.cbprev = NULL;
	scrub_cfgcb.func = scrubcfg;
	cfgnadd(&scrub_cfgcb);
}  

/*
 * NAME:  scrubcfg
 *
 * FUNCTION: 
 *	This routine is called as part of the system config notifcation
 *	scheme.  It validates all changes to the global variable "v".
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the sysconfig SYS_SETPARMS system call.
 *	This routine cannot pagefault, since it disables interrupts.
 *
 * DATA STRUCTURES:
 *	v is defined in <sys/var.h>
 *
 *  	v.v_memscrub 	 0 = disable, non-zero = enable
 *	
 *	In buid0:
 *
 *	scrub start addr	
 *	scrub end addr	
 *
 * RETURNS:	
 *	Always returns success, 0. 
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att()
 *	io_det()
 */

scrubcfg(register int cmd, struct var *cur, struct var *new)
{
	int intpri;
	ulong buid0_addr;
	ulong *scrub_start_addr, *scrub_end_addr;
	
	/* only POWER_RS1 and POWER_RS2 support memory scrubbing */
	if (!__power_set(POWER_RS1 | POWER_RS2))
	{
		if (cmd == CFGV_PREPARE)
		{
			if (new->v_memscrub)
			{
				return((int)&new->v_memscrub - (int)new); 
			}
		}
		return(0);
	}

	/*
	 * This routine is called twice every time one of the system
	 * global variables "v" is changed.  The first time to approve
	 * the changes, the second to commit.  Only take action, if 
	 * we are changing states.  
	 */
	if (cmd == CFGV_COMMIT)
	{

		/* scrubbing is on and request is to disable */
		if (cur->v_memscrub && !new->v_memscrub)		
		{
			TRCHKT(HKWD_KERN_SCRUB_DISABLE);

			intpri = i_disable(INTMAX);

			/*
		  	 * The least significant bit enables or
		 	 * disables memory scrubbing.
		 	 */
			buid0_addr = (ulong)io_att(BUID0, 0);
			scrub_end_addr = (ulong *)(buid0_addr + SEAR_ADDR);
			*scrub_end_addr = 0;

			/* 
		 	 * Check correctable single bit ecc errors.
		 	 * Reads SBSSR, SBSR, SBAR.
		 	 */
			check_soft_errors(buid0_addr);

			io_det(buid0_addr);

			i_enable(intpri);

		}
		/* scrubbing is off and request is to enable */
		else if (!cur->v_memscrub && new->v_memscrub)
		{
			TRCHKT(HKWD_KERN_SCRUB_ENABLE);

			intpri = i_disable(INTMAX);

			buid0_addr = (ulong)io_att(BUID0, 0);
			scrub_start_addr = (ulong *)(buid0_addr + SSAR_ADDR);
			scrub_end_addr   = (ulong *)(buid0_addr + SEAR_ADDR);

			/* 	
		 	 * Get next segment to scrub, skip bad blocks 
		 	 * listed in the bad block map.  Assign values
		 	 * to scrub control registers (start & end ).
		 	 * This also turns on memory scrubbing.  
		 	 */
			set_next_interval(scrub_start_addr, scrub_end_addr);

			io_det(buid0_addr);

			i_enable(intpri);
		}

	}
	return(0);
} 

/*
 * NAME:  scrubclock
 *
 * FUNCTION: 
 *	This routine is called once a second from sys_timer() to 
 *	perform memory scrubbing.  
 *
 * EXECUTION ENVIRONMENT:
 *	This routine cannot pagefault, since it disables interrupts.
 *
 * DATA STRUCTURES:
 *	v is defined in <sys/var.h>
 *
 *  	v.vscrub_mem 	 0 = disable, non-zero = enable
 *	
 *	In buid0:
 *
 *	scrub start addr	
 *	scrub end addr	
 *
 * RETURNS:  none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att()
 *	io_det()
 */

void
scrubclock()
{
	ulong buid0_addr;
	ulong *scrub_start_addr, *scrub_end_addr;
	int intpri;

	intpri = i_disable(INTMAX);

	buid0_addr = (ulong)io_att(BUID0, 0);

	scrub_start_addr = (ulong *)(buid0_addr + SSAR_ADDR);
	scrub_end_addr   = (ulong *)(buid0_addr + SEAR_ADDR);

	/* 
	 * The first time scrubbing is enabled, the two 
	 * registers are equal.  When start > end, the 
	 * last designated segment has been scrubbed. 
	 */ 
	if (*scrub_start_addr > *scrub_end_addr)
	{
		/* 
		 * Check correctable single bit ecc errors.
		 * Reads SBSSR, SBSR, SBAR.
		 */
		check_soft_errors(buid0_addr);

		/* 	
		 * Get next segment to scrub, skip bad blocks 
		 * listed in the bad block map.  Assign values
		 * to scrub control registers (start & end ).
		 * This also turns on memory scrubbing.  The
		 * low order bit turned on in the scrub_end_addr.
		 */
		set_next_interval(scrub_start_addr, scrub_end_addr);
	}

	io_det(buid0_addr);

	i_enable(intpri);
}

/*
 * NAME:  check_soft_errors
 *
 * FUNCTION: 
 *	This routine reads the memory scrubbing registers in buid0 
 *	to determine what soft error were corrected in the last 
 *	memory scrubbing session.  It then records this information
 *	in a local array so that the errors can be remembered.  This 
 *	allows thresholds to be applied before logging errors.	
 *
 * EXECUTION ENVIRONMENT:
 *	This routine cannot pagefault.  Interrupts are disabled by caller.
 *
 * DATA STRUCTURES:
 *	In buid0:
 *
 *	single bit status register
 *	single bit address error 
 *
 * RETURNS:  none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att()
 *	io_det()
 */

static 
void
check_soft_errors(ulong buid0_addr)
{
	ulong *sbssr, *sbsr, *sbar;
	int i;

	sbssr = (ulong *)(buid0_addr + SBSSR_ADDR);
	sbsr = (ulong *)(buid0_addr + SBSR_ADDR);
	sbar = (ulong *)(buid0_addr + SBAR_ADDR);

	TRCHKL3T(HKWD_KERN_SCRUB_SOFT, *sbssr, *sbsr, *sbar);

	/* Any errors reported */
	if ((ulong)*sbsr & (SBSR_HE | SBSR_SE | SBSR_CE))
	{
		/* add to past count of single bit errors */
		for (i=0; i < SCRUB_ARRAY; i++)
		{
			if ((sb_cnt[i].addr == *sbar) ||
			    (sb_cnt[i].addr == 0))
			{
				sb_cnt[i].addr = *sbar;
				/* only log, if error has repeated itself */
				if (++sb_cnt[i].cnt >= LOG_THRESHOLD)
				{
					ulong bad_addr;

					/* 
					 * Must read in this order to 
					 * clear registers 
					 */
					sb_errlog.sbssr = *sbssr;
					sb_errlog.sbsr  = *sbsr;
					sb_errlog.sbar  = *sbar;

					errsave(&sb_errlog, 
						sizeof(struct errlog));

					/* 
					 * This prevents this block from
					 * being scrubbed again.
					 */
					bad_addr = *sbar; 
					process_bitmap(set, &bad_addr, 
						bitmapaddr, bitmapsize);
				}
				break;
			}
		}
	}
}


/*
 * NAME:  set_next_interval
 *
 * FUNCTION: 
 *	This routine identifies the next segment to be scrubbed.  A 
 *	maximum of 16k will be scrubbed at any given time.  The amount 
 *	may be less since the bad block are skipped. 
 *
 * EXECUTION ENVIRONMENT:
 *	This routine cannot pagefault.  Interrupts are disabled by caller.
 *
 * DATA STRUCTURES:
 *	In buid0:
 *
 *	single bit status register
 *	single bit address error 
 *
 * RETURNS:  none.
 *
 * EXTERNAL PROCEDURES CALLED:
 */

static 
void
set_next_interval(ulong *start, ulong *end)
{
	/* 
	 * Get next good block from "start".  Wrap around to zero
	 * when we run over the end of the bit map.
	 */
	if (process_bitmap(get, start, bitmapaddr, bitmapsize) == -1)
	{
		*start = 0;
		process_bitmap(get, start, bitmapaddr, bitmapsize);
	}
	/* 
	 * If the low order bit in "end" is set, memory scrubbing is enabled. 
	 */
	*end = *start + (ulong)bitmapblocksize - (ulong)1;

	TRCHKL2T(HKWD_KERN_SCRUB_SEG, *start, *end);
}

static 
int
process_bitmap( enum bitmapmode mode, ulong *from, ulong *bitaddr, int size )
{
	char *bitmap;
	int num_bits, byte_index;
	uchar bit_mask;

	/* Treat bit map as char map since size is the number of bytes. */
	bitmap = (char *)bitaddr;

	/* 
	 * Each bit in mask represents xxk where xx is a power of 2
	 * (default is 16k).  0x80 = addr 0x0000, 0x40 = addr xxk, ...
	 *  
	 * 1. Truncate input address to xxk boundary.
	 * 2. Calculate "from"/xxk (size of bad block segments)  to 
	 *    determine the bit offset in the bitmap.
	 * 3. Divide by the number of bits in a word to determine 
	 *    the word index into the bitmap.
	 * 4. Create bit mask for that word. 
	 */ 

	*from &= ~(bitmapblocksize - 1);			/* step 1 */
	num_bits = *from >> bmblksizebitct;			/* step 2 */
	byte_index = num_bits >> DIVIDE_BY_BYTES; 	    	/* step 3 */
	bit_mask = 0x80 >> (num_bits & BYTE_REMAINDER);		/* step 4 */

	if (byte_index >= size)
		return(-1);

	/* set bit in mask */
	if (mode == set)					
	{
		bitmap[byte_index] |= bit_mask;
		return(0);
	}

	/* calculate next good address to scrub */
	for ( ; bitmap[byte_index] & bit_mask; bit_mask >>= 1)
	{
		if (bit_mask == 0)
		{
			bit_mask = 0x80;
			byte_index++;
		}
		*from += bitmapblocksize;
	} 
	return(0);
}
