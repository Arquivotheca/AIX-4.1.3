/* @(#)69       1.2.1.1  src/bos/kernel/ios/POWER/dma_hw_pwr.h, sysios, bos411, 9428A410j 10/26/93 15:50:37 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_DMA_HW_PWR
#define _h_DMA_HW_PWR

#include <sys/xmem.h>
#include <sys/systemcfg.h>
#include <sys/dispauth.h>

/* Structures descibes the state of DMA channels in the system
 */

struct dma_chan{
	int	allocated;		/* 1 = allocated, 0 = free	*/
	int	flags;			/* d_init flags 		*/
	ulong	bid;			/* io_att parameter		*/
	ulong	cntchan;		/* control channel		*/
	ulong	last_t;			/* last tag/tcw	mapped		*/
	ulong	unaligned;		/* last transfer was not aligned */
};
#define MAX_DMA_ID 	2*16		/* Maximum number of dma channel IDs */
extern struct dma_chan dma_ch[MAX_DMA_ID];

/*
 *	Some dma control variables
 */
extern ulong iocc_config;	/* shadow copy of iocc config register */
extern int first_slave_tcw;	/* first tcw used for slave channel */


/*
 *			SLAVE DMA REGISTERS
 *
 *	The following are the IOCC registers and controls for 
 *	DMA slaves.
 */

/*
 *	Slave DMA register organization. These registers are used 
 *	to set up DMA slave block mode transfers. The 
 *	SLAVE_EFF_ADDR macro returns the address of the registers
 *	for the specified channel.
 */
struct slave_regs 
{
	ulong	control;		/* tag control */
	ulong	raddr;			/* next memory address */
	ulong	buf_ctl;		/* buffer control */
};
#define SLAVE_EFF_ADDR(channel) \
	((struct slave_regs volatile *) \
	(ioccaddr | (0x00400060) | ((channel) << 16)))


/* 
 *	Macros to extract fields out of slave_regs.control
 */
#define SLAVE_STATUS(v) 	((v) >> 28)		
#define SLAVE_CONTROL(v)	(((v) >> 24) & 0xf)
#define SLAVE_NEXT_TAG(v)	(((v) >> 12) & 0xfff)
#define SLAVE_COUNT(v)		((v) & 0xfff)


/*
 *	Macro to generate a slave_regs.control value.
 */
#define SLAVE_TAG(ctl,tag,count) \
	((ulong)((ctl) | ((tag) << 12) | (((count) - 1) & 0xFFF)))


/*
 *	Mask values for slave_regs.control
 */
#define SLAVE_ENABLED	0x10000000	/* DMA channel is enabled */
#define SLAVE_CHANNEL	0x08000000	/* Slave mode channel */
#define SLAVE_SYSTEM	0x04000000	/* To/from system memory */
#define SLAVE_FROM_DEV	0x01000000	/* From device to buffer */
#define SLAVE_NEXT	0xFF000FFF	/* Mask off next tag feild */


/*
 *	Mask values for slave_regs.buf_ctl and buf_regs.tcw_addr.
 */
#define BUF_DIRTY	0x80000000	/* Buffer is dirty */
#define BUF_PREFETCHED	0x40000000	/* Buffer contains data */
#define BUF_INVALID	0x20000000	/* Buffer is invalid */


/*
 *	Structure of a TAG. A TAG is the structure used to
 *	map memory for slave DMA. The TAG_EFF_ADDR macro returns
 *	the address of the specified tag.
 *
 *	The tag array is staticly managed. Each DMA channel is
 *	preassigned a part of this array. The TAGS_PER_CHANNEL
 *	constant defines the number of tags that each channel
 *	has. The first tag for a channel can be computed by:
 *
 *	first_tag_channel = first_tag + (TAGS_PER_CAHNNEL * channel);
 *
 *	The slave_regs.control macros and labels can be used to
 *	access/set tag.control.
 */
struct tag
{
	ulong	control;		/* tag control information */
	ulong	raddr;			/* memory address */
};
#define TAG_EFF_ADDR(n) \
	((struct tag volatile *)(ioccaddr | (0x00800000 ) | ((n) << 3)))


/*
 *	Tag masks and labels.
 */
#define TAGS_PER_CHANNEL 256		/* 4K tags / 16 channels */
#define LAST_TAG	0xfff		/* Last tag in list */


/*
 *			MASTER DMA REGISTERS
 *
 *	The following are the IOCC registers and controls for 
 *	DMA masters.
 */


/*
 *	Each DMA channel has a channel status register. This register
 *	is the same physical register as the slave_regs.control
 *	register for DMA slaves. The SLAVE_CHANNEL bit determines how
 *	the hardware processes the register.
 */
#define CSR_EFF_ADDR(channel) \
	((ulong volatile *)(ioccaddr | (0x00400060) | ((channel) << 16)))

/* 	
 *	Effective address of the IO channel disable/enable register.
 */
#define DISABLE_EFF_ADDR(channel) \
        (ulong volatile *)(ioccaddr | 0x00400070|((channel)<<16))

#define ENABLE_EFF_ADDR(channel) \
        (ulong volatile *)(ioccaddr | 0x00400070|((channel)<<16))


/*
 *	These macros extract fields from the csr.
 */
#define CSR_STATUS(v)		((v) >> 28)
#define CSR_BUFF(v)		(((v) >> 16) & 0xf)
#define CSR_AUTHORITY(v)	(((v) >> 8) & 0xff)

/* Returs true if csr channel is enabled
 */
#define CSR_ENABLED(v)							\
	(((v) & SLAVE_CHANNEL) ? (CSR_STATUS(v) == 1) :			\
		((CSR_STATUS(v) <= 3) && ((v) & 0x10000000)))

/*
 *	Thes macros will extract fields from a CSR that is used for
 *	DMA slave with TCWs
 */
#define SLAVE_CSR_CONTRL(c)	((c) & 0xff000000)
#define SLAVE_CSR_COUNT(c)	((c) & 0x000fffff)


/*
 *	This macro is used to generate a DMA master CSR value.
 */
#define MASTER_CSR(flags,buff,auth) \
	((ulong)((flags) | ((buff) << 16) | ((auth) << 8)))

/*
 *	This macro is used to generate a DMA slave CSR value, for slave
 *	arbitration levels using TCWs
 */
#define SLAVE_CSR(control, channel, count) \
	(  (ulong)((control)|((channel) << 20)| ( ((count) - 1) & 0xFFFFF )))


/*
 *	Mask values for csr.
 */
#define MASTER_SYSTEM	0x30000000	/* Enabled to system memory */
#define MASTER_BUS	0x10000000	/* Enabled to bus memory */
#define MASTER_PREFETCH 0x04000000	/* Prefetch required */
#define MASTER_BYPASS   0xFFFFFFDF      /* Bypass the TCW's */

/*
 *	Access authority values for csr.
 */
#define MASTER_FULL_AUTHORITY 0xFF	/* Full access authority */
#define MASTER_NO_AUTHORITY   0		/* No access authority */


/*
 *	The IOCC has several buffers that it performs DMA into and
 *	out of. The contents of a buffer are described by the
 *	buffer control registers. A DMA master can have one or
 *	more buffers assiocated with it.
 *
 *	Currently only one buffer is assigned to a DMA master. The
 *	buffer assigned has the same number as the DMA channel.
 *
 *	The BUF_DIRTY, etc masks can be used with buffer_regs.tcw_addr
 *	to determine the state of the buffer. Also, the TCW masks can
 *	be used with buffer_regs.tcw_image.
 */
struct buffer_regs 
{
	ulong	tcw_image;		/* Copy of TCW last used */
	ulong	tcw_addr;		/* Status/address */
};
#define BUF_EFF_ADDR(buffer) \
	((struct buffer_regs volatile *) \
	(ioccaddr | (0x00400064) | ((buffer) << 16)))


/*
 *	Macros to extract fields from buffer_regs.
 */
#define BUF_BUS_ADDR(t) 	((t) & 0x03ffffff)


/*
 *	Each entry in the TCW array maps a page of bus addresses.
 *	d_master uses this array to map a DMA transfer for a DMA
 *	master. The TCW_EFF_ADDR macro can be used to generate the
 *	address of an entry in the TCW array given the TCW number.
 *	the CALC_TCW_NUM macro can be used to calculate the TCW
 *	number given a bus address.
 *
 *	Each channel is staticly allocated a window into system
 *	memory via the TCWs. The dma.h (sysdma.h) header file
 *	defines the size and location of this window. Larger
 *	windows can be assigned to a device as part of the ODM
 *	system configuration process.
 *
 *	The first 16 TCWs are always mapped to cause an exception.
 *	This part of the address space is reserved for bus I/O accesses.
 */
#define TCW_EFF_ADDR(tcw_num) \
	((uint volatile *)(ioccaddr | 0x00C00000 | ((tcw_num)<<2)))
#define CALC_TCW_NUM(daddr)	(((uint)(daddr)) >> DMA_L2PSIZE)

/*
 *	Macro to generate a master TCW value.
 *	NOTE: The c-bit must be set as RSC work around
 */
#define MASTER_TCW(raddr,buff,key,control) \
   (((uint)(raddr) & 0xfffff000) | ((buff)<<8) | ((key)<<4) | (control) | 0x4)

/*
 *	Macro to generate a slave TCW value
 */
#define SLAVE_TCW(raddr) \
	((uint)(raddr) & 0xfffff000)


/*
 *	The following macros extract fields out of a TCW.
 */
#define TCW_RPN(tcw)		((tcw) >> 12)
#define TCW_BUFF(tcw)		(((tcw) >> 8) & 0xf)
#define TCW_KEY(tcw)		(((tcw) >> 4) & 0xf)
#define TCW_CTL(tcw)		((tcw) & 0x3)


/*
 *	The following are TCW masks and labels.
 */
#define TCW_REFERENCED	8		/* TCW used in reference */
#define TCW_CHANGED	4		/* TCW used in store */
#define TCW_SYS_RW	3		/* System memory read/write */
#define TCW_SYS_RO	2		/* System memory read only */
#define TCW_BUS 	0		/* Bus memory */
#define TCW_NO_ACCESS	1		/* No access is allowed */

/*
 *	This structure will contain the parameter
 *	list for functions invoked by the pio_assist 
 *	exception handler.
 */

struct parmlist 
{
	volatile ulong	*t;			/* destination  address */
	volatile ulong	data;			/* data         	*/
	volatile ulong	flags;			/* flush = 1, pio = 0   */
	volatile ulong	channel;		/* IO  channel          */
};



/*
 *			DMA ERROR VALUES.
 *
 *	The following define the IOCC DMA error status values.
 */
#define	DMA_NO_ERROR		3	/* No DMA error */
#define DMA_EXTRA_REQ		4	/* Extra slave DMA request */
#define DMA_AUTH_ERROR		5	/* Authority error */
#define DMA_FAULT_ERROR		6	/* TCW page fault */
#define DMA_TCW_EXTENT		7	/* No TCW for bus address */
#define DMA_CHANNEL_CHECK	8	/* Channel check */
#define DMA_D_PARITY		9	/* Inbound data parity error */
#define DMA_A_PARITY		10	/* Address parity error */
#define DMA_CSF_ERROR		11	/* No card selected feedback */
#define DMA_ECC_ERROR		12	/* System memory ECC error */
#define DMA_SYSTEM_ADDRESS	13	/* Bad system memory address */
#define DMA_TAG_PARITY		14	/* Tag parity error */
#define DMA_TCW_PARITY		14	/* TCW parity error */
#define DMA_IOCC_ERROR		15	/* Internal IOCC error */


/*
 *			DMA COMMANDS
 *
 *	The following define the IOCC DMA commands.
 */



/*
 *	Flush the IOCC buffer assiocated with a DMA slave transfer.
 *	NOTE:
 *		RSC bug prevents this command from being issued
 */
#define SLAVE_FLUSH()							\
{									\
	if (!__power_rsc())						\
	{								\
		int rc;							\
		volatile ulong *ptr;					\
		ptr = (ulong volatile *)(ioccaddr | 0x00400078|		\
						((channel)<<16));	\
		if (rc = busputl(ptr, 0))				\
		{							\
			assert(rc == EXCEPT_IO);			\
			flush_recv(channel_id, csa->except);		\
		}							\
	}								\
}


/*
 *	Restore csr15 value for graphics threads.
 *	This macro is used in MASTER_FLUSH(), io_exception(), d_move(),
 *      and d_bflush().
 *	Interrupts must be disabled before using this macro.
 */
#define RESTORE_CSR15(buid)                                             \
{                                                                       \
	register struct gruprt  *gp;                                    \
	for (gp = curthread->t_graphics; gp; gp = gp->gp_next)          \
	{                                                               \
	    /* make sure csr15 is set up correctly (even when this      \
	       thread does not currently own any display on the bus)    \
	     */						                \
	    if (BID_TO_BUID(GRUPRT_IOCC_SR(gp)) == buid)                \
	    {                                                           \
	        setup_display_acc(gp);                                  \
	        break;                                                  \
            }                                                           \
       }                                                                \
}									


/*
 *	Flush the IOCC buffer assiocated with a DMA master transfer.
 *	NOTES:
 *		On RSC CSR 15 read returns 0 in b5-b31.  The authority
 *		mask in CSR 15 must be reconstructed for processes
 */
#define MASTER_FLUSH(tcw_num)						\
{									\
	volatile ulong *csr;						\
	struct {							\
		uint oldpri;						\
		uint oldcsr;						\
	}errstate;							\
									\
	csr = CSR_EFF_ADDR(15);						\
	if (master_flush(csr, MASTER_CSR(0, channel, MASTER_NO_AUTHORITY), \
		ioccaddr|0x00C00002|((tcw_num) << 2), ioccaddr|0x0400010, \
		&errstate))						\
	{								\
		flush_recv(channel_id, csa->except);			\
		*csr = errstate.oldcsr;					\
		LOAD_IOCC_CONFIG();					\
		i_enable(errstate.oldpri);				\
	}								\
	else {                                                          \
                /* Only need to restore csr15 for RSC when i/o exception \
		   does not occur.  This is because if i/o exception    \
		   occurs, csr15 will have been set up correctly by     \
		   io_exception() for graphics threads.                 \
		 */                                                     \
		if (__power_rsc())                                      \
		{                                                       \
		    int  oldpri;					\
	            int  buid1;						\
		    extern int disable_ints();				\
		    extern void enable_ints();				\
	       								\
		    buid1 = BID_TO_BUID(dma_ch[channel_id].bid);	\
	       								\
	       	    /* disable external interrupts */			\
	       	    oldpri = disable_ints();				\
		    RESTORE_CSR15(buid1);                               \
		    /* re-enable external interrupts */			\
	       	    enable_ints(oldpri);				\
		}                                                       \
	}                                                               \
}

/*
 * Invalidate buffer. bufctl is address of buffer control register
 */
#define BUFF_INVAL(bufctl)						\
		*(volatile ulong *)(bufctl) = BUF_INVALID;		\

/*
 * Invalidate the next buffer, for dual buffer support
 */
#define NEXT_BUFF_INVAL(tcw_num)					\
	*(volatile ulong *)(ioccaddr | 0x01800000 | ((tcw_num) << 2)) = 0; 

/*
 *			MISCELLANEOUS COMMANDS
 *
 */

/*
 *	Convert the channel id into a channel number (0 - 15).
 */
#define ID_TO_NUM(channel_id)						\
	((channel_id) & 0xF)

/*
 *	Convert the channel number into a channel id.
 */
#define NUM_TO_ID(channel, bid)						\
	((channel) + ((((bid) & 0x00F00000)) >> 20) * 16)

/*
 *	Creates a virtual memory handle.
 *	Preserve the given buid and set up for I/O address space,
 * 	address check and increment, IOCC select, 24-bit mode and
 *	bypass mode.
 */
#define IOCC_HANDLE(bid)						\
                ((0x0FF00000 & bid) | 0x800C00E0)

/*
 *	Creates a virtual memory handle.
 *	Preserve the given buid, set up for virtual address space
 *	and turn off the bypass bit.
 */
#define VIRTUAL_HANDLE(bid)						\
                (((0x0FF00000 & bid) | 0x800C0000) & MASTER_BYPASS)


/*
 *    Performs a load from the IOCC Configuration Register.
 *
 *    A problem was found in the IOCC design which has potential to affect
 *    error recovery.  When an Invalid Operation occurs following a Load/Store
 *    to an IOCC Channel Status Register, the error will not be logged.
 *    An Invalid Operation is defined as any Load/Store operation which is
 *    returned with a '0001'B status code in Channel Status Register 15.
 *    Load/Stores to the CSR will be followed by another Supervisor
 *    State instruction  (LOAD_IOCC_CONFIG) to rectify the problem.
 */


#define LOAD_IOCC_CONFIG()						\
{									\
	volatile ulong *ptr;						\
        ptr = ( ioccaddr | 0x0400010);					\
	ptr = *ptr;							\
}

/*
 *	Reset a channel status register, clearing error status
 */
#define RESET_STATUS(c)							\
{									\
	volatile ulong *ptr;						\
	ptr = (ulong *)(c);						\
	*ptr = (*ptr & 0x0FFFFFFF);					\
}

/*
 *	Performs loads and stores to the iocc.  
 *	Warning do not change the
 *	values of IOCC_WRITE ore IOCC_READ without also changing
 *	R2/sys/ml/dma.s
 */
int iocc_rw( int flag, unsigned long *addr, unsigned long data );
#define IOCC_READ 0		/* return iocc address specified by *addr */
#define IOCC_WRITE 1		/* write value of data to *addr		  */

#endif /* _h_DMA_HW_PWR */



