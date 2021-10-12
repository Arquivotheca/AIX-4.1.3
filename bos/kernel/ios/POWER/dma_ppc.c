static char sccsid[] = "@(#)09	1.12  src/bos/kernel/ios/POWER/dma_ppc.c, sysml, bos41J, bai15 4/11/95 10:57:49";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *		d_init_ppc, d_clear_ppc, d_slave_ppc, d_master_ppc, 
 *		d_mask_ppc, d_unmask_ppc, d_move_ppc, d_cflush_ppc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * GLOBAL NOTES:
 *	These routines represent the PowerPC versions of the DMA services.
 *	At system initialization time, the machine type is identified, and
 * 	if determined to be a PowerPC, these routine addresses replace the
 *	exported service name addresses in the export list.
 *
 *	SERIALIZATION
 *
 *	These services only serialize allocating and freeing DMA
 *	channels (d_init_ppc and d_clear_ppc). The caller is responsible for 
 *	serializing the calls to the other DMA services. These services
 *	can be thought of as an extension of the device which the device
 *	driver already has to serialize its access to. These services
 *	must serialize when using registers shared by more than one channel.
 *
 *	SYSTEM INTERACTION
 *
 *	DMA bypasses much of the normal memory reference hardware. Therefore
 *	this code must provide a reliable environment in which to perform
 *	a DMA operation. Much of this work is performed by the xmemdma
 *	service.
 *
 *	For the PowerPC platform, the xmemdma service only checks access
 *	protection for the buffer, sets the mod bit if the transfer is
 *	destructive, and returns the buffer's real address. 
 *
 *	IOCC BUFFERING
 *
 *	The PowerPC I/O architecture is consistent, so there is no 
 *	buffer or cache managment necessary.
 *
 *****************************************************************************/
#ifdef _POWER_PC
 
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/errno.h>
#include <sys/errids.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include "dma_hw.h"

 

struct iocc_info iocc_info[MAX_NUM_IOCCS]={0};

struct dmaerr_ppc {			/* error log description */
	struct  err_rec0      derr;
		int           bid;
		int           channel;
		int           csr;
} dma_log_ppc;

#ifdef _RS6K_SMP_MCA
/*
 * The fd_mutex variable exists solely for a Pegasus SW workaround.
 * This WA keeps a floppy diskette DMA and a NVRAM access from
 * colliding somewhere in or around the SSGA.  When this happens
 * the NVRAM access data is corrupted.  Basically, the WA consists
 * of the setting this lock when a diskette DMA starts and resetting
 * it when it completes.  Places in the machine device driver will
 * check this lock before reading/writing NVRAM or other devices
 * outside the SSGA.  If the lock is set the MDD will kill the ongoing
 * DMA before proceeding.
 * Accesses to TOD, set_int_bu behave the same way.
 * fd_mutex can have three values:
 *	0 - No DMA active
 *	1 - DMA active on Arb lvl 0
 *	2 - DMA active on Arb lvl 1
 *
 **************************************************************************
 * Note: if the diskette channel id changes then the #define for
 *       it should also be change or this WA will just stop working.
 **************************************************************************
 */

#define DISKETTE_CHN_ID0	0x80001040
#define DISKETTE_CHN_ID1	0x40001040
#define DISKETTE_CHN_ID_MSK	0xC000FFC0
#define DMA_CHANNEL_DIS_CMD0	0x10500
#define DMA_CHANNEL_DIS_CMD1	0x10504
#define SEGREG_DMA_DIS          0x82000000
#define CHAN_DISABLE_DELAY	0x280000	/* about 250ms */

int fd_mutex = 0;
int fd_delay = CHAN_DISABLE_DELAY;
int abort_cnt = 0;
extern int pgs_SSGA_lvl;

void
d_abort_fd()
{
        int *dis_cmd;
	int dma_chan;
	volatile int i;

	abort_cnt++;

	if( fd_mutex == 1 )
		dma_chan = DMA_CHANNEL_DIS_CMD0;
	else
		dma_chan = DMA_CHANNEL_DIS_CMD1;

        dis_cmd = (int *)io_att(SEGREG_DMA_DIS, dma_chan);
        *dis_cmd = 1;
        __iospace_sync();
        io_det (dis_cmd);

	for (i = fd_delay; i > 0; i--);

	fd_mutex = 0;

}

#endif /* _RS6K_SMP_MCA */



/*
 * NAME: complete_error
 *
 * FUNCTION: process d_complete error, do error loging, clean up
 *	data structures, and calculate error code
 *
 * EXECUTIONS ENVIORMENT:
 *	Called from interrupt, and process level
 *	Only called from d_complete
 *
 * NOTES: This function is not in-line to aviod bringing this code
 *	into the cache on each d_complete call
 *
 * RETURNS: d_complete return value
 *	    DMA_AUTHORITY  - access/page protection violation
 *	    DMA_EXTRA	   - Slave unsolicited request
 *	    DMA_BAD_ADDR   - Micro-channel error, CSF, address/data parity
 *	    DMA_PAGE_FAULT - Page fault
 *	    DMA_SYSTEM 	   - System error
 */
static int
complete_error(
	int channel_id,		/* channel identifier */
	int csr_val,		/* CSR value read by d_complete */
	int error,		/* Error status from CSR */
	char *baddr,		/* buffer address passed by caller */
	size_t count)		/* length of transfer */
{

	uint	trash;

	/*
	 *  Log the errors.
	 */
	dma_log_ppc.derr.error_id = ERRID_DMA_ERR;
	copystr("SYSDMA", dma_log_ppc.derr.resource_name, 7, &trash);
	dma_log_ppc.bid = CHAN_BUID_PPC(channel_id);
	dma_log_ppc.channel = CHAN_NUM( channel_id );
	dma_log_ppc.csr = csr_val;
	errsave(&dma_log_ppc, sizeof(dma_log_ppc)); 

	/*
	 *  Convert the CSR error status into a DMA error return value.
	 */
	if (SLAVE_CHAN(channel_id)) {
	   /*
	    *	If this is a Slave Channel
	    */
	   switch (error) {
		case DMA_EXTRA_REQ_PPC	: {
			/*
			 *  If this is a slave channel AND the error was
			 *  an extra request error, then we have to check to
			 *  see if we caused the extra request error due to
			 *  a previous authority violation detected in d_slave.
			 *
			 *  So, get the faulting address from the appropriate
			 *  control register and see if it was in the range
			 *  mapped by d_slave.
			 */

			uint 	fault_addr, maddr;
			struct iocc_info *d;
			uint	first_tce;
			uint	ioccaddr;
	
                        d = (struct iocc_info *)
				&iocc_info[IOCC_INDEX(channel_id)];
			ioccaddr = (uint)io_att(CHAN_BUID_PPC(channel_id) ,0);
			fault_addr = 
				*(SLAVE_CTRL_ADDR(SLV_CTRL_NUM(channel_id)));
			io_det(ioccaddr);
			first_tce = d->first_slave_tce + 
			       (SLV_TCES_PER_CHAN * SLV_CTRL_NUM(channel_id));
			maddr = (first_tce << DMA_L2PSIZE) | 
				    ((uint)baddr & (uint)(DMA_PSIZE-1));
			if ((fault_addr >= maddr) && 
			    (fault_addr < (maddr + (uint)count))) {
				/*
				 *  If the faulting address is within the range
				 *  of the original address + length (for slave
				 *  we always start with the first slave TCE for
				 *  that channel) then it was an authority 
				 *  violation.
				 */
				return(DMA_AUTHORITY);
			}
			return(DMA_EXTRA);
		}
		case DMA_MCA_ERR_PPC	: return(DMA_BAD_ADDR);
		default 	 	: return(DMA_SYSTEM);
           } /* switch */
	} else {
		/*
		 *	It is a Master Channel
		 */
		switch (error) {
			case DMA_AUTH_ERROR_PPC	: return(DMA_AUTHORITY);
			case DMA_FAULT_ERROR_PPC: return(DMA_PAGE_FAULT);
			case DMA_MCA_ERR_PPC	: return(DMA_BAD_ADDR);
			default 	 	: return(DMA_SYSTEM);
		}
	}

}

/*
 * NAME: d_init_ppc
 *
 * FUNCTION: This service allocates and initializes a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called from a program executing 
 *	either on the interrupt or process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * RETURN VALUE DESCRIPTION: 
 *	channel_id  -  On successful completion. This value is then passed
 *		       as a parameter to other DMA routines. Using this
 *		       value, these routines can determine specific channel
 *		       information.
 *
 *	DMA_FAIL    -  channel was already allocated.
 *
 * EXTERNAL PROCEDURES CALLED: 
 *	io_att, io_det
 *
 * ACCESSED:	Branch table entry d_init
 */
int 
d_init_ppc(
	int channel,
	int flags, 
	vmhandle_t bid)
{
	register int	intpri; 		/* interrupt priority */
	register int	channel_id;		/* channel identifier */
	register uint  ioccaddr;		/* io_att return value */
	volatile uint	*csr;			/* channel status register */
	register int	index;
	register struct iocc_info *d;		/* pointer to info struct */
	register int	slave_chan = 0;		/* flag if whether slave */
	register int	ctrl_num = 0;		/* slave control reg number */


	/*
	 *  	Ensure that the operation is valid.
	 */
	ASSERT((channel <= 15) && (channel >= 0));

	/*
	 * 	Determine which iocc_info_struct to use
	 */

	for ( index = 0; index < MAX_NUM_IOCCS; index++) 
		if ((iocc_info[index].bid & 0x1FF00000) == (bid & 0x1FF00000))
			/*
			 *	Search through the IOCC structs
			 */
			break;
	assert(index != MAX_NUM_IOCCS); 	     /* Make sure we found it */
	d = (struct iocc_info *)&iocc_info[index]; /* Set up pointer */

	GET_IOCC_LOCK( intpri, INTMAX, &d->iocc_lock );

	/*
	 * If the channel is in use then fail
	 */
	if (ALLOCATED(channel, d->dma_chans))  {
		REL_IOCC_LOCK( intpri, &d->iocc_lock );
		return(DMA_FAIL);
	}

	ioccaddr = (uint)io_att(BUID_PPC(bid) ,0);/* set IOCC addressability */

	if (flags & DMA_SLAVE)
	{
		ctrl_num = clz32(d->slave_ctrl_regs); /* find free ctrlreg */
		if (ctrl_num >= d->num_slave_chans) {
			/*
			 * Must be out of Slave Control Registers
			 */
			io_det(ioccaddr);
			REL_IOCC_LOCK( intpri, &d->iocc_lock );
			return(DMA_FAIL);
		}
		RESERVE(ctrl_num, d->slave_ctrl_regs);	/* mark as in use */
		slave_chan = SLAVE_BIT;			/* mark as slave chan*/
		
		/*
		 *  Personalize the DMA channel for DMA slave
		 *  operation. 
		 */
		csr = (volatile uint *)CSR_EFF_ADDR_PPC(channel);
		*csr = SLAVE_CSR_PPC(SLAVE_MODE_PPC, ctrl_num, 0);

	} else {

		/*
		 *  Personalize the DMA channel for DMA master
		 *  operation. 
		 */
		csr = (volatile uint *)CSR_EFF_ADDR_PPC(channel);
		*csr = CHAN_ENABLE;
	}

	/*
	 *  Channel successfully initialized. Mark as allocated.
	 */
	RESERVE(channel, d->dma_chans);
	/*
	 * Build the channel ID
	 */
	channel_id = CHAN_ID_PPC(ctrl_num, (uint)bid, slave_chan, index, 
			channel);

	/*
	 *  Clean up and return to the caller.
	 */
	io_det(ioccaddr);
	REL_IOCC_LOCK( intpri, &d->iocc_lock );

	return( channel_id );

}

/*
 * NAME: d_clear_ppc
 *
 * FUNCTION: This service frees the indicated DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless the DMA channel has been
 *	allocated with the d_init service.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED: 
 *	io_att, io_det
 *
 * ACCESSED:	Branch table entry d_clear
 */
void 
d_clear_ppc(
	int channel_id)
{
	register int	intpri; 		/* interrupt priority */
	register int	index;			/* index into IOCC struct */
	register struct iocc_info *d;	/* pointer to dma info struct*/
	void d_mask_ppc();

	/*
	 *  Ensure that the operation is valid.
	 */
	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);
	d = (struct iocc_info *) &iocc_info[index];
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */

	/*
	 *  Serialize the allocating and freeing of DMA channels.
	 */
	GET_IOCC_LOCK( intpri, INTMAX, &d->iocc_lock );

	/*
	 *  Make sure that the channel is disabled.
	 */
	d_mask_ppc(channel_id);

	/*
	 *  Mark the channel as free. 
	 */
	d->dma_chans |= (channel_id & 0xFFFF0000);
	if (SLAVE_CHAN(channel_id))
		/*
		 * Mark the control register as free
		 */
		FREE(SLV_CTRL_NUM(channel_id), d->slave_ctrl_regs);

	REL_IOCC_LOCK( intpri, &d->iocc_lock );

	return;

}

/*
 * NAME: d_mask_ppc
 *
 * FUNCTION: This service disables a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless the DMA channel has been
 *	allocated with the d_init service.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * ACCESSED:	Branch table entry d_mask
 */
void 
d_mask_ppc(
	int channel_id)				/* channel to disable */
{
	register uint	channel;		/* normalized channel	*/
	register int	index;			/* index into IOCC struct */
        register uint ioccaddr;			/* virtual handle */
	register struct iocc_info *d;	/* pointer to dma info */

	/*
	 *  Ensure that the operation is valid.
	 */
	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);
	d = (struct iocc_info *) &iocc_info[index];
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */

        /*
	 *  Get Channel Number from Channel ID
	 */
	channel = CHAN_NUM(channel_id);

	/* 
	 *  Disable the specified channel.
	 *
	 *  Note: This command will be ignored if there is an error
	 *  pending or the channel is already disabled.
	 *
 	 */
        ioccaddr = (uint) io_att(BUID_PPC(d->bid), 0);
	/*
	 * Store to the disable address, which will disable the channel
	 */
	*(uint volatile *)(DISABLE_EFF_ADDR_PPC(channel)) = 0;

        io_det( ioccaddr );
}

/*
 * NAME: d_unmask_ppc
 *
 * FUNCTION: This service enables a DMA channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless the DMA channel has been
 *	allocated with the d_init service.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * ACCESSED:	Branch table entry d_unmask
 */
void 
d_unmask_ppc(
	int channel_id)
{
	uint channel;
	register int	index;			/* index into IOCC struct */
        uint ioccaddr;
	uint trash;
	register struct iocc_info *d;	/* pointer to dma info */

	/* 
	 *  Ensure that the operation is valid.
	 */
	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);
	d = (struct iocc_info *) &iocc_info[index];
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */

	channel = CHAN_NUM(channel_id);	/* get channel number from ID */

	/*  Enable the specified channel.
	 *
	 *  Note: This command will be ignored if there is an error
	 *  	pending or the channel is already enabled.
	 */
        ioccaddr = (uint) io_att(BUID_PPC(d->bid), 0);
	/*
	 *  Read from the Enable Effective Address, which will enable
	 *  the channel
	 */	
        trash = *(volatile uint *)ENABLE_EFF_ADDR_PPC(channel);

        io_det(ioccaddr);	
}

/*
 * NAME: d_slave_ppc
 *
 * FUNCTION:
 *	This is the PPC d_slave() implemenation.  calls to d_slave reach this
 *	function throuth the system branch table.  This service supports
 *	the initialization of the DMA channel for a DMA slave block mode
 *	transfer. This consists of mapping the transfer via the TCE
 *	table and initializing the system DMA controller to perform the
 *	transfer.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	The memory buffer must remain pinned from the time this service is
 *	called until the DMA transfer is completed and d_complete is called.
 *
 *	This service should not be called unless the DMA channel has been
 *	allocated with the d_init service.
 *
 *	Note: When DMA_READ flag is specified, the data will be
 *	moved from the device to memory.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma, xmemacc, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_slave
 */
void
d_slave_ppc(
	int channel_id,				/* channel to use	     */
	int flags,				/* control flags             */
	char *baddr,				/* buffer address            */
	size_t count,				/* length of transfer        */
	struct xmem *dp)			/* cross mem descrip         */
{
	register uint channel;			/* normalize channel num     */
	register uint num_tces;		/* number of tcws for trans. */
	register uint *tce_addr;		/* address of TCW to map */
	register uint slave_channel;		/* channel of slave regs. */
	register struct iocc_info *d;	/* pointer to dma info */
	register int page_no;			/* count of pages mapped */
	uint ioccaddr;				/* io_att return value	     */
	uint raddr;				/* DMA address */
	volatile uint *csr;			/* address of CSR */
	uint first_tce;			/* first tcw number to map */
	uint maddr;				/* slave address reg value */
	uint control;				/* DMA control */
	int	index;				/* index into IOCC struct */

	/* 
	 *  Ensure that the operation is valid.
	 */
#ifdef _RS6K_SMP_MCA
	/* NVRAM WA */
        if (__rs6k_smp_mca() && pgs_SSGA_lvl == 2 ) {

		index = channel_id & DISKETTE_CHN_ID_MSK;
		if(index == DISKETTE_CHN_ID0)
		{
			fd_mutex = 1;
		}
		else if(index == DISKETTE_CHN_ID1)
		{
			fd_mutex = 2;
		}
	}
#endif /* _RS6K_SMP_MCA */

	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);
	d = (struct iocc_info *) &iocc_info[index];
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */
	ASSERT((count > 0) && (count <= DMA_MAX));
	ASSERT(channel_id & SLAVE_BIT);

/*
#ifdef	DEBUG
       (void) d_check(channel_id, flags, baddr, count, dp, 0xdeadbeef, XMEM_HIDE);
#endif	DEBUG
*/
	channel = CHAN_NUM(channel_id); 	/* get channel number */

	/* get addressablity to the IOCC
	 */
	ioccaddr = (uint)io_att(BUID_PPC(d->bid), 0);

	slave_channel = SLV_CTRL_NUM(channel_id);

	if (flags & BUS_DMA)
	{
		/* calculate values for control regs.
		 */
		control = (flags & DMA_READ) ?
			SLAVE_MODE_PPC|CHAN_ENABLE|SLAVE_FROM_DEV_PPC :
			SLAVE_MODE_PPC|CHAN_ENABLE;
		maddr = (uint)baddr;
	}
	else
	{
		/* calculate values for control regs.
		 */
		control = (uint)((flags & DMA_READ) ?
		   SLAVE_MODE_PPC|SLAVE_SYSTEM_PPC|CHAN_ENABLE|
			SLAVE_FROM_DEV_PPC :
		   SLAVE_MODE_PPC|SLAVE_SYSTEM_PPC|CHAN_ENABLE);

		/* calculate the first tcw for use by this slave
		 * channel
		 */
		first_tce = d->first_slave_tce + 
				(SLV_TCES_PER_CHAN * slave_channel);

		/* calculate the number of TCEs needed to map this transfer.
		 */
		num_tces = ((uint)(baddr + count - 1) >> DMA_L2PSIZE) -
				((uint)baddr >> DMA_L2PSIZE) + 1;

		/* calculate memory address
		 */
		maddr = (first_tce << DMA_L2PSIZE) |
					((uint)baddr & (DMA_PSIZE-1));

		/* Compute the starting TCE address
		 */
		tce_addr = (uint *)TCE_EFF_ADDR_PPC(d->tce_offset, first_tce);
		page_no = 0;

		while(page_no < num_tces) {
			/*
			 * For each TCE in the range
			 */

			/* Get the real address and perform access checking
			 */
			raddr = xmemdma_ppc(dp, baddr, XMEM_HIDE|XMEM_ACC_CHK| 
			     ((flags & DMA_READ) ? 0 : XMEM_WRITE_ONLY ));

			if ((int)raddr == XMEM_FAIL) {
				/*
				 * Something failed 
				 */
				if (page_no == 0) {
					/*
					 * if this was the first page, then
					 * set control to disable the channel
					 */
					control &= ~CHAN_ENABLE;
				} else {
					/*
					 * We've already mapped something, so
					 * adjust count to what we've already
					 * mapped
					 */
					count = (int)
						((int)(page_no * DMA_PSIZE) - 
					  	(int)((uint)baddr & 
						(uint)(DMA_PSIZE - 1)));
				}
				break;	/* break out of the loop */
			}

			*tce_addr = SLAVE_TCE_PPC(raddr);

			tce_addr++;
			baddr += DMA_PSIZE;
			page_no++;
		}
		/*
		 * Perform SYNC 
	 	 */
		__iospace_sync();
	}

	/* set up control register with address and channel status register
	 */
	*(uint volatile *)SLAVE_CTRL_ADDR(slave_channel) = maddr;
	csr = (uint volatile *)CSR_EFF_ADDR_PPC(channel);
	*csr = SLAVE_CSR_PPC(control, slave_channel, count);

	io_det(ioccaddr);

}


/*
 * NAME: d_master
 *
 * FUNCTION: This service supports the initialization of the DMA
 *	channel for a DMA master block mode transfer.
 *	This only consists of mapping the transfer via the TCEs.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either 
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	The memory buffer must remain pinned from the time this service is
 *	called until the DMA transfer is completed and d_complete is called.
 *
 *	This service should not be called unless the DMA channel has been 
 *	allocated with the d_init service.
 *
 *	This routine can not alter the channel's CSR. This is because a
 *	transfer can be in progress for one device while a transfer is
 *	being mapped for another device. 
 *	(The problem is that error information could be lost.)
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma,  xmemacc, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_master
 */
void 
d_master_ppc(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	register uint	num_tces;		/* number of TCWs to map */
	register uint	raddr;			/* DMA address */
	register uint  ioccaddr;		/* io_att return value */
	register uint  tce_sys_acc;		/* R/W access permissions */
	register uint  *tce_addr;		/* TCE effective address */
	register uint  *last_addr;		/* last TCE address */
	volatile uint	*t;			/* tcw */
	int	index;				/* index into IOCC struct */
	uint  access_permitted;			/* r/w access to a page */
	uint  access;				/* R/W access permissions */
	register struct iocc_info *d;	/* pointer to dma info */

	/* 
	 *  Ensure that the operation is valid.
	 */
	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);		/* valid IOCC struct */
	d = (struct iocc_info *) &iocc_info[index]; /* point to struct */
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */
	ASSERT(count > 0);			/* valid transfer length */
	ASSERT(!(channel_id & SLAVE_BIT));	/* not a slave channel */
	ASSERT(!(flags & BUS_DMA));		/* PPC doesn't support this, */
						/* bus to bus must be done */
						/* with the Bus Mapping Reg */
	/*
	 *  Setup access to the IOCC.
	 */
	ioccaddr = (uint)io_att(BUID_PPC(d->bid), 0);

/*
#ifdef DEBUG
        (void) d_check(channel_id, flags, baddr, count, dp, daddr, XMEM_HIDE);
#endif	DEBUG
*/
	/*
	 *  Compute the total number of TCEs needed for this transfer
	 */
	num_tces = ((uint)(baddr + count - 1 ) >> DMA_L2PSIZE ) -
		   ((uint)baddr >> DMA_L2PSIZE ) + 1;

	if ( flags & DMA_WRITE_ONLY )
		/*
		 * If this is a DMA Write Only, then set
		 * system page access to Read-Only, i.e. don't
		 * allow the device to write into system memory
		 */
		tce_sys_acc = TCE_SYS_RO_PPC;		
	else
		/*
		 * else, set to full read-write access
		 */
		tce_sys_acc = TCE_SYS_RW_PPC;		

	/*
	 * Compute starting TCE address and ending TCE address
	 */
	tce_addr = (uint *)TCE_EFF_ADDR_PPC(d->tce_offset, 
			CALC_TCE_NUM_PPC(daddr)); 	
	last_addr = (uint *)TCE_EFF_ADDR_PPC(d->tce_offset, 
				(CALC_TCE_NUM_PPC(daddr) + num_tces)); 	


	for ( t = tce_addr; t < last_addr; t++) {
		/*
		 *	For each TCE in the range of the transfer
		 */

		access = tce_sys_acc;	/* reset access to original */
		/* 	Get the real address and perform access checking
		 */
		raddr = xmemdma_ppc(dp, baddr, XMEM_HIDE|XMEM_ACC_CHK| 
			     ((flags & DMA_WRITE_ONLY) ? XMEM_WRITE_ONLY : 0));

		if ((int)raddr == XMEM_FAIL) {
			/*
			 *	Something failed. See if it was an 
			 *	access violation.
			 */
			raddr = xmemqra(dp, baddr);
			access_permitted = xmemacc(dp, baddr, flags&DMA_NOHIDE);
			if ( access_permitted == XMEM_NOACC ) 
				/*
				 * if no access, set TCE to Page Fault
				 */
				access = TCE_PAGE_FAULT;		
			else 
				if ((access_permitted == XMEM_RDONLY) && 
			   		( !(flags & DMA_WRITE_ONLY)))
					/*
					 * else if the page is read-only and
					 * this could be a write, set the 
					 * TCE control to Read-only
					 */
			     		access = TCE_SYS_RO_PPC;
		}

		*t = MASTER_TCE_PPC(raddr,0,access); /* Write the TCE */
		baddr += DMA_PSIZE;	       /* increment buffer address */
	}

	/*
	 * perform SYNC
	 */
	__iospace_sync();

	/*
	 *  Clean up and return to the caller.
	 */
	io_det( ioccaddr );
}

/*
 * NAME: d_complete_ppc
 *
 * FUNCTION: This service is called upon completion of a DMA transfer.
 *	    It will report any errors.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 *	This service should not be called unless the DMA channel has been 
 *	allocated with the d_init service.
 *
 * RETURN VALUE DESCRIPTION: see dma.h
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma, io_att, io_det, errsave
 *
 * ACCESSED:	Branch table entry d_complete
 */
int 
d_complete_ppc(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	register int	error;			/* DMA error status */
	register uint	channel;		/* normalized channel value */
	register uint  ioccaddr;		/* io_att return value */
	volatile uint	*csr;			/* channel status register */
	register uint	csr_val;		/* contents of CSR */
	int	index;				/* index into IOCC struct */
	register struct iocc_info *d;	/* pointer to dma info */

	/* 
	 *  Ensure that the operation is valid.
	 */
	index = IOCC_INDEX(channel_id);
        ASSERT(index < MAX_NUM_IOCCS);		/* valid IOCC struct */
	d = (struct iocc_info *) &iocc_info[index]; /* point to struct */
	ASSERT(!(channel_id & d->dma_chans));   /* make sure its allocated */
	ASSERT(count > 0);			/* valid transfer length */

	channel = CHAN_NUM(channel_id);		/* get channel number from ID */

/*
#ifdef	DEBUG
       (void) d_check(channel_id, flags, baddr, count, dp, daddr, XMEM_UNHIDE);
#endif	DEBUG
*/

	/*
	 *  Setup access to the IOCC.
	 */
	ioccaddr = (uint)io_att(BUID_PPC(d->bid), 0);

	/*
	 *  Get the error status for this transfer 
	 */
	csr = CSR_EFF_ADDR_PPC(channel);
	csr_val = *csr;
	error = (int)CSR_STATUS_PPC(csr_val);

	if (!(SLAVE_CHAN(channel_id)) && (error > DMA_NO_ERROR_PPC)) {
		/*
		 *  If there was an error AND this is a Master
		 *  channel, then re-initialize the DMA channel 
		 *  (slaves get setup each time)
		 */
		*csr = CHAN_ENABLE;
	}

	/*
	 *  Restore state for return to the caller.
	 */
	io_det( ioccaddr );

#ifdef _RS6K_SMP_MCA
	/* NVRAM WA */
        if (__rs6k_smp_mca() && pgs_SSGA_lvl == 2 ) {

		index = channel_id & DISKETTE_CHN_ID_MSK;
		if((index == DISKETTE_CHN_ID0) || (index == DISKETTE_CHN_ID1))
		{
			fd_mutex = 0;
		}
	}
#endif /* _RS6K_SMP_MCA */

	if (error <= DMA_NO_ERROR_PPC)
		/*
		 * if no error, return success
		 */
		return(DMA_SUCC);
	else
		/*
		 * else go figure out what the error was and
		 * return the proper code.
		 */
		return(complete_error(channel_id,csr_val,error,baddr,count));
}

/*
 * NAME: d_cflush_ppc
 *
 * FUNCTION: This service is a no-operation on PowerPC, since the PowerPC
 *	     platform is cache consistent.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing the interrupt level.
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * RETURN VALUE DESCRIPTION: 0  
 *
 * ACCESSED:	Branch table entry d_cflush
 */
int  
d_cflush_ppc(
	int channel_id,				/* channel to use */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	char *daddr)				/* bus address */
{
	return(0);
}               
               
/*
 * NAME: d_bflush_ppc
 *
 * FUNCTION: This service is a no-operation on PowerPC, since the PowerPC
 *	     platform is cache consistent.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing the interrupt level.
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * RETURN VALUE DESCRIPTION: 0  
 *
 * ACCESSED:	Branch table entry d_bflush
 */
int  
d_bflush_ppc(
	int channel_id,				/* channel to use */	
	vmhandle_t bid,				/* Bus ID handle */
	char *daddr)				/* bus address */
{
	return(0);
}               
               
/*
 * NAME: d_move_ppc
 *
 * FUNCTION:  This service is not supported on coherent memory platforms,
 *	      which PowerPC is.  
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either 
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * RETURN VALUE DESCRIPTION:
 *	EINVAL - not supported on this platform
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * ACCESSED:	Branch table entry d_move
 */
int 
d_move_ppc(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	return(EINVAL);
}

#endif /* _POWER_PC */
