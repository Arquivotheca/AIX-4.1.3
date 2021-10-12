/* @(#)68       1.3  src/bos/kernel/ios/POWER/dma_hw_ppc.h, sysios, bos411, 9428A410j 7/27/93 19:06:26 */
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:  DMA hardware interface
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_DMA_HW_PPC
#define _h_DMA_HW_PPC

#include <sys/xmem.h>
#include <sys/iocc.h>
#ifdef _POWER_MP
 #include <sys/lock_def.h>
#endif


/*
 * 	The iocc_info structure is used to maintain state per IOCC.
 */

struct iocc_info {
	uint	bid;			/* this IOCCs Bus ID */
	uint	tce_offset;		/* offset in kernel ext segment */	
					/* to starting TCE for this IOCC */
	uint	num_tces;		/* # of TCEs for this IOCC */
	uint	num_slave_chans;	/* # of slave chans for this IOCC*/
	uint	first_slave_tce;	/* number of first slave TCE */
	uint	slave_ctrl_regs;	/* allocation bitmask for slaves regs */
	uint	dma_chans; 	    	/* allocation bitmask for DMA channels*/
#ifndef _POWER_MP
	uint	reserved; 	    	/* pad struct to 32 bytes */
#else
	Simple_lock iocc_lock;		/* Serialization lock */
#endif
};

extern struct iocc_info iocc_info[MAX_NUM_IOCCS];

#define	MAX_SLV_CTRL	8		/* max # of slave control regs/IOCC */
#define	MAX_DMA_CHANS	16		/* max # of DMA channels/IOCC */

/* 	Initializes slave ctrl reg table 
 */
#define INIT_SLV_CTRL_TAB(slv_chans)  (0xFFFFFFFF << (32 - (slv_chans))) 

/* 	Initializes DMA channel table 
 */
#define INIT_DMA_CHAN_TAB() (0xFFFFFFFF << (32 - MAX_DMA_CHANS))

/* 	Checks if resource is allocated 
 */
#define ALLOCATED(index, bitmask) (!((bitmask) & (1 << (31 - (index))))) 

/* 	Checks if resource is available 
 */
#define AVAILABLE(index, bitmask) ((bitmask) & (1 << (31 - (index)))) 

/* 	Marks a resource as in use 
 */
#define RESERVE(index, bitmask) ((bitmask) &= ~(1 << (31 - (index)))) 

/* 	Marks a resource as free 
 */
#define FREE(index, bitmask)	((bitmask) |= (1 << (31 - (index)))) 


/*
 *			SLAVE DMA REGISTERS
 *
 *	The following are the IOCC registers and controls for 
 *	DMA slaves.
 */

#define SLV_TCES_PER_CHAN  256		

/*
 * 	Return address of Slave Control Register
 */
#define SLAVE_CTRL_ADDR(ctrlnum) \
	((uint volatile *) \
	(ioccaddr | (0x00010380) | ((ctrlnum) << 2)))


/* 
 * 	Macros to extract fields out of CSRs
 *
 *   SLAVE CSR:
 *   ----------------------------------------------------------------
 *   | Status|1| Ctrl| Ctrl  |         Length  Count                 |
 *   |       | |     | reg # |                                       |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   ----------------------------------------------------------------
 *   MASTER CSR:
 *   ----------------------------------------------------------------
 *   | Status|0|P|            Reserved                               |
 *   |       | |r|                                                   |
 *   | | | | | |v| | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   ----------------------------------------------------------------
 */
#define SLAVE_CTRL_NUM(v)	(((v) >> 24) & 0xf)
#define SLAVE_COUNT_PPC(v)	((v) & 0xfffff)
#define CSR_STATUS_PPC(v)		((v) >> 28)
/* 
 * 	Returns true if CSR channel is enabled
 */
#define CSR_ENABLED_PPC(v)						\
	(((v) & SLAVE_MODE_PPC) ? (CSR_STATUS_PPC(v) == 1) :		\
		((CSR_STATUS_PPC(v) < 2) && ((v) & 0x10000000)))

/*
 *	This macro is used to generate a DMA slave CSR value
 */
#define SLAVE_CSR_PPC(control, ctrlnum, count) \
	((uint)((control)|((ctrlnum) << 20)| ( ((count) - 1) & (uint)0xFFFFF )))

/*
 * 	Mask values for CSR
 */
#define CHAN_ENABLE		0x10000000	/* DMA channel is enabled */
#define SLAVE_MODE_PPC		0x08000000	/* Slave mode channel */
#define SLAVE_SYSTEM_PPC	0x04000000	/* To/from system memory */
#define SLAVE_FROM_DEV_PPC	0x01000000	/* From device to buffer */


/*
 * 	Effective Address of Channel Status Register
 */
#define CSR_EFF_ADDR_PPC(channel) \
	((uint volatile *)(ioccaddr | (0x00010400) | ((channel) << 2)))

/* 	
 * 	Effective address of the IO channel disable/enable register.
 */
#define DISABLE_EFF_ADDR_PPC(channel) \
        (uint volatile *)(ioccaddr | 0x00010500|((channel)<<2))

#define ENABLE_EFF_ADDR_PPC(channel) \
        (uint volatile *)(ioccaddr | 0x00010500|((channel)<<2))


/*
 * 	Extract authority mask from segment register
 */
#define SR_AUTHORITY_PPC(segval)	(((v) >> 8) & 0x7f)

/*
 *	Each entry in the TCE table maps a page of bus addresses.
 *	d_master uses this array to map a DMA transfer for a DMA
 *	master. The TCE_EFF_ADDR_PPC macro can be used to generate the
 *	address of an entry in the TCE array given the TCE number.
 *	the CALC_TCE_NUM_PPC macro can be used to calculate the TCE
 *	number given a bus address.
 *
 *	Each channel is staticly allocated a window into system
 *	memory via the TCEs. The dma.h (sysdma.h) header file
 *	defines the size and location of this window. Larger
 *	windows can be assigned to a device as part of the ODM
 *	system configuration process.
 *
 *	The first 16 TCEs are always mapped to cause an exception.
 *	This part of the address space is reserved for bus I/O accesses.
 */
#define TCE_EFF_ADDR_PPC(tce_base_addr, tce_num) \
	((uint volatile *)(tce_base_addr + ((tce_num)<<2)))

#define CALC_TCE_NUM_PPC(daddr)	(((uint)(daddr)) >> DMA_L2PSIZE)

/*
 *   MASTER TCE:
 *   +---------------------------------------+-------------+-----+---+
 *   |          Real Page Number             | Reserved    | Key |Ctl|
 *   |                                       |             |     |   |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------+-------------+-----+---+
 *   SLAVE TCE:
 *   +---------------------------------------+-------------+-----+---+
 *   |          Real Page Number             | Reserved    | Key |Rsv|
 *   |                                       |             |     |   |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------+-------------+-----+---+
 */

/*
 *	Macro to generate a master TCE value.
 */
#define MASTER_TCE_PPC(raddr,key,control) \
	(((uint)(raddr) & 0xfffff000) | ((key)<<2) | (control)) 

/*
 *	Macro to generate a slave TCE value
 */
#define SLAVE_TCE_PPC(raddr)  ((uint)(raddr) & 0xfffff000)

/*
 *	The following macros extract fields out of a TCE.
 */
#define TCE_RPN_PPC(tce)	(tce) >> 12)
#define TCE_KEY_PPC(tce)	(((tce) >> 2) & 0x7)
#define TCE_CTL_PPC(tce)	((tce) & 0x3)


/*
 *	The following are TCE Page mapping and Control bits.
 */
#define TCE_SYS_RW_PPC	3		/* System memory read/write */
#define TCE_SYS_RO_PPC	2		/* System memory read only */
#define TCE_BUS_PPC 	0		/* Bus memory */
#define TCE_PAGE_FAULT	1		/* page fault - no access */

/*
 *			DMA ERROR VALUES.
 *
 *	The following define the IOCC DMA error status values.
 */
#define	DMA_NO_ERROR_PPC		2	/* No DMA error */
#define DMA_EXTRA_REQ_PPC		3	/* Extra slave DMA request */
#define DMA_AUTH_ERROR_PPC		4	/* Authority error */
#define DMA_FAULT_ERROR_PPC		5	/* TCE page fault */
#define DMA_MCA_ERR_PPC			6	/* Micro-Channel Error */
#define DMA_SYS_ACC_ERR_PPC		7	/* System Access Error */


/*
 *			MISCELLANEOUS COMMANDS
 *
 */

/*
 *	Build a channel ID
 *
 *   ----------------------------------------------------------------
 *   |         DMA channel mask      |      BUID       |S|slave|IOCC |
 *   | (bit number indicates channel)|                 |M|ctr #|index|
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   ----------------------------------------------------------------
 * 
 */

#define CHAN_ID_PPC(slvctrl, buid, slv_mast, iocc_ind, chan_num)	\
	(((slvctrl) << 3) | (((buid) & 0x1FF00000) >> 13) | 		\
	(slv_mast) | (iocc_ind) | (1 << (31 - (chan_num))))

/*
 *	Slave Bit
 */
#define SLAVE_BIT	0x00000040

/*
 *	Get the Slave Control Number from the Channel ID
 */ 
#define SLV_CTRL_NUM(channel_id)	(((channel_id) >> 3) & 0x7)
/*
 *	Get the BUID from the Channel ID
 */ 
#define CHAN_BUID_PPC(channel_id)   \
	(((channel_id << 13) & 0x1FF00000) | 0x800C00E0)
/*
 *	Create a BUID segment register value
 */
#define BUID_PPC(bid)   (((bid) & 0x1FF00000) | 0x800C00E0)
/*
 *	Test if this is a Slave Channel (bit 12)
 */ 
#define SLAVE_CHAN(channel_id)	((channel_id) & SLAVE_BIT)
/*
 *	Get the IOCC array index from the Channel ID
 */ 
#define IOCC_INDEX(channel_id) 	((channel_id) & 0x7)
/*
 *	Get the DMA Channel Number from the Channel ID
 *	(counts leading zeroes of bit mask)
 */ 
#define	CHAN_NUM(channel_id)	clz32(channel_id)

/*
 *	Reset a channel status register, clearing error status
 */
#define RESET_STATUS_PPC(c)						\
{									\
	volatile ulong *ptr;						\
	ptr = (ulong *)(c);						\
	*ptr = (*ptr & 0x0FFFFFFF);					\
}


#ifndef _POWER_MP
 #define GET_IOCC_LOCK( oP, nP, pL )	(oP = i_disable( nP ))
#else
 #define GET_IOCC_LOCK( oP, nP, pL )	(oP = disable_lock( nP, pL ))
#endif

#ifndef _POWER_MP
 #define REL_IOCC_LOCK( oP, pL )	(i_enable( oP ))
#else
 #define REL_IOCC_LOCK( oP, pL )	(unlock_enable( oP, pL ))
#endif

#endif /* _h_DMA_HW_PPC */
