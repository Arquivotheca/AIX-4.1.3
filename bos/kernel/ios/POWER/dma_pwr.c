static char sccsid[] = "@(#)08        1.2.1.7  src/bos/kernel/ios/POWER/dma_pwr.c, sysml, bos411, 9438B411a 9/20/94 11:15:37";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *		d_init_pwr, d_clear_pwr, d_slave_rs1, d_slave_rsc, 
 *              d_master_pwr, d_mask_pwr, d_unmask_pwr, d_move_pwr, 
 *              d_cflush_pwr, d_bflush_pwr
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
 *
 * GLOBAL NOTES:
 *
 *	These routines represent the POWER versions of the DMA services.
 *	At system initialization time, the machine type is identified, and
 * 	if determined to be type POWER, these routine addresses replace the
 *	exported service name addresses in the export list.
 *
 *      For d_slave, there are separate versions within the POWER platform
 *	as well; one for RS1 and another for RSC.      
 *
 *	SERIALIZATION
 *
 *	These services only serialize allocating and freeing DMA
 *	channels (d_init and d_clear). The caller is responsible for 
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
 *	For a XMEM_HIDE operation, the xmemdma service makes all
 *	pages containing the DMA buffer inaccessible to the processor, 
 *	flushes the cache lines assiocated with the buffer, and returns
 *	the buffer's real address. For a XMEM_UNHIDE operation, the xmemdma
 *	service conditionally sets the pages change bit and makes them
 *	accessible to the processor.
 *
 *	Therefore any page containing a DMA buffer can not be accessed
 *	between the call to d_master and the corresponding call to 
 *	d_complete. 
 *
 *	IOCC BUFFERING
 *
 *	The IOCC buffers DMA operations. Each buffer has assiocated with
 *	it a dirty (D) bit, a buffered (B) bit, and an invalidate (I) bit.
 *	These bits describe the current state of the buffer. The B bit
 *	indicates if the buffer contains valid memory data. The D bit
 *	indicates if the buffer contains valid memory data that is more
 *	up to date than the copy in real memory. The I bit forces the
 *	IOCC to prefetch the first buffer from real memory, independent
 *	of the state of the other two bits.
 *
 *	The I bit is always set when a channel is allocated and at the
 *	start of each DMA slave transfer. Normally the IOCC clears a
 *	buffer when it is written to system memory or it prefetches
 *	the desired data from system memory. There are times that it
 *	does not do this for performance reasons. For example, when
 *	reading a complete buffers worth of data from a DMA slave.
 *	The problem here is that the DMA slave may not transfer a 
 *	complete buffers worth of data. This is really only a security
 *	concern on the first buffer in a transfer because the data
 *	in the buffer is assiocated with some other transfer. Therefore
 *	the I bit forces this first buffer to be prefetched and it 
 *	therefore contains either the old or new data for that transfer.
 *
 *	The D and B bits are always cleared when a channel is allocated,
 *	at the start of a DMA slave transfer, or at the end of a DMA
 *	slave transfer after the buffer is flushed. They can not be
 *	modified directly for DMA master transfers because another
 *	transfer for the same adapter/channel can be in progress.
 *
 *****************************************************************************/
#ifdef _POWER_RS
 
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/except.h>
#include <sys/machine.h>
#include <sys/buid.h>
#include <sys/user.h>
#include <sys/low.h>
#include <sys/systemcfg.h>
#include "dma_hw.h"
 

ulong	iocc_config = 0;
int	first_slave_tcw = 0;
int	num_ioccs = 0;


struct dma_chan dma_ch[MAX_DMA_ID];

struct dmaerr {				/* error log description */
	struct  err_rec0        derr;
		ulong           bid;
		ulong           channel;
		ulong           csr;
} dma_log ={ ERRID_DMA_ERR,"SYSDMA ", 0, 0, 0 };


#ifdef DEBUG
struct d_act {
        int     channel_id;             /* channel to use */
        int     flags;                  /* control flags */
        char    *baddr;                 /* buffer address */
        int     count;                  /* length of transfer */
        struct  xmem     *dp;            /* cross mem descrip */
        char    *daddr;                 /* bus address */
        ulong   raddr;                  /* real page number */
        int     type;                   /* type of operation */
};
struct d_act d_active[128];
struct d_act * d_active_idx = NULL;

static void
d_check(channel_id, flags, baddr, count, dp, daddr, type)
register int    channel_id;                     /* channel to use */
register int    flags;                          /* control flags */
register char   *baddr;                         /* buffer address */
register size_t count;                          /* length of transfer */
struct xmem     *dp;                            /* cross mem descrip */
register char   *daddr;                         /* bus address */
register int    type;                           /* xmemdma type */
{
        register int opri, i;
        register ulong raddr;

        opri = i_disable( INTMAX );

        /*
         *      Put this operation in the trace table.
         */

        if ( d_active_idx == NULL )
        {
                for( i = 0; i < 64; i++ )
                {
                        d_active[i].channel_id = -1;
                        d_active[i].flags = -1;
                        d_active[i].baddr = NULL;
                        d_active[i].count = -1;
                        d_active[i].dp = NULL;
                        d_active[i].daddr = NULL;
                        d_active[i].raddr = -1;
                        d_active[i].type = 0;
                }
                d_active_idx = &d_active[0];
        }
        i = d_active_idx - &d_active[0];
        d_active[i].channel_id = channel_id;
        d_active[i].flags = flags;
        d_active[i].baddr = baddr;
        d_active[i].count = count;
        d_active[i].dp = dp;
        d_active[i].daddr = daddr;
	if (flags & (DMA_WRITE_ONLY | DMA_NOHIDE)) {
		d_active[i].raddr = (ulong)xmemqra(dp, baddr);
	} else {
		d_active[i].raddr = 0xffffffff;
	}
        d_active[i].type = type;
        if ( i == 127 )
                d_active_idx = &d_active[0];
        else
                d_active_idx++;


        i_enable( opri );

}
#endif /* DEBUG */

/*
 * NAME: flush_recv
 *
 * FUNCTION: This routine performs recovery operations if 
 *	    an IO exception was experienced while flushing
 *	    IOCC buffers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on the interrupt 
 *	level.
 *
 * NOTES:
 *
 *	Exceptions may occur when accessing TCW's or TAG's. 
 *	Exception handlers have been implemented to catch 
 *	the exception, retry if possible, and log errors.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att, io_det
 */
static void
flush_recv(
	int channel_id,
	struct pio_except *infop)
{
        ulong  ioccaddr;
	int channel;
 	ulong	*csr;

	channel = ID_TO_NUM(channel_id);
	ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);
	csr = CSR_EFF_ADDR(channel);
	*csr = (( *csr ) | ( CSR_STATUS( infop->pio_csr) << 28 ));
	io_det( ioccaddr );
        return;
}

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
 */
static int
complete_error(
	int channel_id,
	int csr_val,
	int error)
{
	/*
	 *  Log the errors.
	 */
	dma_log.bid = dma_ch[channel_id].bid;
	dma_log.channel = ID_TO_NUM( channel_id );
	dma_log.csr = csr_val;
	errsave(&dma_log, sizeof(dma_log)); 

	if ( dma_ch[channel_id].flags & DMA_PG_PROTECT )
	{
		/*
		 *  The calling process did not have the
		 *  access authority to a page that it had expected.
		 *  This flag is set to indicate that and the
	 	 *  channel is disabled (for slave transfers) or
		 *  the direction bit restricts access (for masters).
		 *  d_complete will return DMA_AUTHORITY for both.
		 */
		error |= DMA_AUTH_ERROR;
		dma_ch[channel_id].flags &= ~DMA_PG_PROTECT;
	}

	/*
	 *  Convert the CSR error status into a DMA error return value.
	 */
	if ( dma_ch[channel_id].flags & DMA_SLAVE )
	{
		switch (error)
		{
			case DMA_AUTH_ERROR	: return(DMA_AUTHORITY);
			case DMA_EXTRA_REQ	: return(DMA_EXTRA);
			case DMA_CHANNEL_CHECK	: return(DMA_CHECK);
			case DMA_D_PARITY	: return(DMA_DATA);
			case DMA_CSF_ERROR	: return(DMA_NO_RESPONSE);
			default 	 	: return(DMA_SYSTEM);
		}
	}
	else
	{
		switch (error)
		{
			case DMA_AUTH_ERROR	: return(DMA_AUTHORITY);
			case DMA_FAULT_ERROR	: return(DMA_PAGE_FAULT);
			case DMA_TCW_EXTENT	: return(DMA_BAD_ADDR);
			case DMA_CHANNEL_CHECK	: return(DMA_CHECK);
			case DMA_D_PARITY	: return(DMA_DATA);
			case DMA_A_PARITY	: return(DMA_ADDRESS);
			case DMA_CSF_ERROR	: return(DMA_NO_RESPONSE);
			default 	 	: return(DMA_SYSTEM);
		}
	}

}

/*
 * NAME: d_init_pwr
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
 *		       value, these routines can determine which IOCC the 
 *		       channel is on.	
 *
 *	DMA_FAIL    -  channel was already allocated.
 *
 * EXTERNAL PROCEDURES CALLED: 
 *	i_disable, i_enable, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_init
 */
int 
d_init_pwr(
	int channel,
	int flags, 
	vmhandle_t bid)
{
	register int	intpri; 		/* interrupt priority */
	register ulong	channel_id;		/* channel identifier */
	register ulong  ioccaddr;		/* io_att return value */
	volatile struct slave_regs  *s_regs;	/* DMA registers */
	volatile struct buffer_regs *b_regs;	/* buffer registers */
	volatile ulong	*csr;			/* channel status register */
	register int	rc;			/* return code */
	register int	num_slave_chan;
	register int	chan;


	/*
	 *  Ensure that the operation is valid.
	 */
	ASSERT((channel <= 14) && (channel >= 0));
	ASSERT(!(flags & PC_AT_DMA));
	ASSERT((BID_TO_BUID(bid) == IOCC1_BUID) ||
				(BID_TO_BUID(bid) >= IOCC0_BUID));

       /*
	* normalize channel	
	*/
	channel_id = NUM_TO_ID (channel, bid);

	ASSERT((flags & DMA_BUS_MASK) <= DMA_BUS_MAX);
	if (flags & DMA_SLAVE)
		ASSERT(!(flags & REGION_DMA));

	/*
	 *  Serialize the allocating and freeing of DMA channels.
	 */
	intpri = i_disable(INTMAX);
	ioccaddr = (ulong)io_att(IOCC_HANDLE(bid) ,0);

	/*
	 * If the channel is in use then fail the caller
	 */
	if (dma_ch[channel_id].allocated)  
	{
		rc = DMA_FAIL;
		goto clean_up;
	}

	if (flags & DMA_SLAVE)
	{
		num_slave_chan = ICF_NUM_SLVCHN(iocc_config);

		/*
		 * if the arbitration level is grater than the number of slave
		 * channels then search for a free channel starting at
		 * highest implemented channel number
		 */
		if (channel >= num_slave_chan)
		{
			for (chan = NUM_TO_ID(num_slave_chan - 1, bid) ;
				chan >= NUM_TO_ID(0, bid) ; chan--)
			{
				if (!dma_ch[chan].allocated)
					break;
			}

			/*
			 * if all the channels have been allocated then
			 * return failure
			 */
			if (chan < NUM_TO_ID(0, bid))
			{
				rc = DMA_FAIL;
				goto clean_up;
			}

			/*
			 * set unused fields in slave channel to zero, and
			 * mark as allocated
			 */
			bzero(&dma_ch[chan], sizeof(struct dma_chan));
			dma_ch[chan].allocated = 1;
			dma_ch[chan].bid = -1;

			/*
			 * set control channel number for arbitration level
			 */
			dma_ch[channel_id].cntchan = ID_TO_NUM(chan);
		}
		else
		{
			/*
			 * This arbitration level has a coresponding slave
			 * channel so use it
			 */
			dma_ch[channel_id].cntchan = channel;
		}

		/*
		 * Mark channel as allocated and initialize the other fields
		 */
		dma_ch[channel_id].allocated = 1;
		dma_ch[channel_id].flags = flags;
		dma_ch[channel_id].bid = bid;

		/*
		 *  Personalize the DMA channel for DMA slave
		 *  operation. Also mark the assiocated buffer
		 *  as invalid to force a prefetch before the
		 *  buffer is used.
		 */
		s_regs = SLAVE_EFF_ADDR(dma_ch[channel_id].cntchan);
		assert(!(s_regs->buf_ctl & BUF_DIRTY));

		csr = CSR_EFF_ADDR(channel);

		if (iocc_config & ICF_SLAVE_TCW)
		{
			*csr = SLAVE_CSR(SLAVE_CHANNEL , 
				dma_ch[channel_id].cntchan, 0);
		}
		else
		{
			*csr = SLAVE_TAG(SLAVE_CHANNEL, LAST_TAG, 0);
		}

		BUFF_INVAL(&s_regs->buf_ctl);

	} else {

		/*
		 *
		 *  Mark this channel as allocated and initialize the
		 *  the other fields.
		 */
		dma_ch[channel_id].allocated = 1;
		dma_ch[channel_id].flags = flags;
		dma_ch[channel_id].bid = bid;
		dma_ch[channel_id].last_t = 0;
		dma_ch[channel_id].cntchan = channel;

		/*
		 *  Personalize the DMA channel for DMA master
		 *  operation. Also mark the assiocated buffer
		 *  as invalid to force a prefetch before the
		 *  buffer is used.
		 *
		 *  The default is for transfers between the
		 *  device and system with prefetch required.
		 */
		b_regs = BUF_EFF_ADDR(channel);
		csr = CSR_EFF_ADDR(channel);
		assert( ! (b_regs->tcw_addr & BUF_DIRTY) );
		*csr = MASTER_CSR(MASTER_SYSTEM|MASTER_PREFETCH,channel,
			   MASTER_FULL_AUTHORITY);
		BUFF_INVAL(&b_regs->tcw_addr);
	}

	/*
	 *  Channel successfully initialized.
	 */
	rc = channel_id;

clean_up:
	/*
	 *  Clean up and return to the caller.
	 */
	io_det(ioccaddr);
	i_enable(intpri);

	return( rc );

}

/*
 * NAME: d_clear_pwr
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
 *	i_disable, i_enable, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_clear
 */
void 
d_clear_pwr(
	int channel_id)
{
	register int	intpri; 		/* interrupt priority */
	register ulong	channel;		/* normalized channel	*/
	register ulong  ioccaddr;		/* io_att return value */
	volatile struct slave_regs *s_regs;	/* DMA registers */
	int cntchan_id;				/* Slave control chan. ID */
	register ulong	slave_chan;

	/*
	 *  Serialize the allocating and freeing of DMA channels.
	 */
	intpri = i_disable(INTMAX);

	/*
	 *  Ensure that the operation is valid.
	 */
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);

	/*
	* normalize channel	
	*/
	channel = ID_TO_NUM (channel_id);

	/*
	 *  Make sure that the channel is disabled.
	 */
	d_mask(channel_id);

	/*
	 *  Verify that the owner of the channel has called d_complete
	 *  to flush the IOCC buffers. A dirty buffer means that data
	 *  is lost for the last transfer.
	 *
	 *  Mark the buffer as invalid. This will force prefetch on the
	 *  first transfer by the next user of the channel. This is
	 *  required to ensure that there are no security exposures.
	 *
	 *  Note that the buffer control register is common to both
	 *  DMA master operation and DMA slave operation.
	 */
	ioccaddr =  (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);
	s_regs = SLAVE_EFF_ADDR(dma_ch[channel_id].cntchan);
	assert(!(s_regs->buf_ctl & BUF_DIRTY));
	BUFF_INVAL(&s_regs->buf_ctl);
	io_det( ioccaddr );

	/*
	 *  Mark the channel as free. Note that the rest of the fields
	 *  are left alone to assist in debug. The d_init service fully
	 *  initializes the channels entry when a channel is allocated.
	 */
	dma_ch[channel_id].allocated = 0;
	if (dma_ch[channel_id].cntchan != channel)
	{
		cntchan_id = NUM_TO_ID(dma_ch[channel_id].cntchan,
				dma_ch[channel_id].bid);
		dma_ch[cntchan_id].allocated = 0;
	}

	/*
	 *  Clean up and return to the caller.
	 */
	i_enable(intpri);

	return;

}

/*
 * NAME: d_mask_pwr
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
d_mask_pwr(
	int channel_id)				/* channel to disable */
{
	register ulong	channel;		/* normalized channel	*/
        register ulong ioccaddr;		/* virtual handle */
        volatile ulong *ptr;			/* pts. to disable register */

	/*
	 *  Ensure that the operation is valid.
	 */
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);

        /*
	 *  normalize channel	
	 */
	channel = ID_TO_NUM (channel_id);

	/* 
	 *  Disable the DMA specified channel.
	 *
	 *  Note: This command will be ignored if there is an error
	 *  pending or the channel is already disabled.
	 *
	 *  Note: on RSC the IOCC store command can't be surrounded by
	 *	word aligned fixed point stores, so the assembler routine
	 *	is used
 	 */
        ioccaddr = (ulong) io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);
        disable_chan(DISABLE_EFF_ADDR(channel), CSR_EFF_ADDR(channel));
        io_det( ioccaddr );
	 
}

/*
 * NAME: d_unmask_pwr
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
d_unmask_pwr(
	int channel_id)
{
	ulong channel;
        ulong ioccaddr;
        volatile ulong *ptr;
	ulong trash;
	int ipri;
	int csr;

	/*  Ensure that the operation is valid.
	 */
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);

	/* normalize channel	
	 */
	channel = ID_TO_NUM(channel_id);

	/*  Enable the specified channel.
	 *
	 *  Note: This command will be ignored if there is an error
	 *  	pending or the channel is already enabled.
	 *  Note: RSC will hang if the enable command is issued to an
	 *	enabled channel
	 */
        ioccaddr = (ulong) io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);
	ptr = CSR_EFF_ADDR(channel);
	ipri = i_disable(INTMAX);
	csr = *ptr;
	if (CSR_ENABLED(csr))
		goto cleanup;
	
        ptr = ENABLE_EFF_ADDR(channel);
        trash = *ptr;

cleanup:
	i_enable(ipri);
        io_det(ioccaddr);	
}

/*
 * NAME: d_slave_rsc
 *
 * FUNCTION:
 *	This is the RSC d_slave() implemenation.  calls to d_slave reach this
 *	function throuth the system branch table.  This service supports
 *	 the initialization of the DMA channel for a DMA slave block mode
 *	 transfer. This consists of mapping the transfer via the tag/tcw
 *	table and initializing the system DMA controller to perform the
 *	transfer.
 *
 *	This service supports the initialization of DMA channel for DMA
 *	slave block mode transfer using TCWs.  This consists of mapping the
 *	transfer via the TCW table and initializating the system DMA
 *	controller to perform the transfer
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
 *	The buffer must also not be accessed between the call to d_slave
 *	and the call to d_complete. See the global note on system
 *	interaction.
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
 *	xmemdma, xmemqra, xmemflush, xmemacc, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_slave
 */
void
d_slave_rsc(
	int channel_id,				/* channel to use	     */
	int flags,				/* control flags             */
	char *baddr,				/* buffer address            */
	size_t count,				/* length of transfer        */
	struct xmem *dp)			/* cross mem descrip         */
{
	ulong channel;				/* normalize channel num     */
	ulong ioccaddr;				/* io_att return value	     */
	ulong num_tcws;				/* number of tcws for trans. */
	ulong raddr;				/* DMA address */
	volatile ulong *csr;			/* address of CSR */
	ulong first_tcw;			/* first tcw number to map */
	ulong *last_tcw;			/* address to end mapping at */
	ulong *tcw_addr;			/* address of TCW to map */
	ulong maddr;				/* slave address reg value */
	ulong slave_channel;			/* channel of slave regs. */
	ulong control;				/* DMA control */
	ulong csrval;				/* channel status reg. value */
	int access_permitted;			/* page protection */
	volatile struct slave_regs *s_regs;	/* slave channel regs */
	ulong daddr;

	/* 
	 *  Ensure that the transfer is valid.
	 */
	ASSERT((count > 0) && (count <= DMA_MAX));
	ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);
	ASSERT(dma_ch[channel_id].flags & DMA_SLAVE);

#ifdef	DEBUG
       (void) d_check(channel_id, flags, baddr, count, dp, 0xdeadbeef, XMEM_HIDE);
#endif	DEBUG

	channel = ID_TO_NUM(channel_id);

	/* get addressablity to the IOCC
	 */
	ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);

	slave_channel = dma_ch[channel_id].cntchan;

	if (flags & BUS_DMA)
	{
		/* calculate values for control regs.
		 */
		control = (flags & DMA_READ) ?
			SLAVE_CHANNEL|SLAVE_ENABLED|SLAVE_FROM_DEV :
			SLAVE_CHANNEL|SLAVE_ENABLED;
		maddr = (ulong)baddr;
	}
	else
	{
		/* calculate values for control regs.
		 */
		control = (flags & DMA_READ) ?
		      SLAVE_CHANNEL|SLAVE_SYSTEM|SLAVE_ENABLED|SLAVE_FROM_DEV :
		      SLAVE_CHANNEL|SLAVE_SYSTEM|SLAVE_ENABLED;

		/* get the first TCW to use.  If DMA_CONTINUE is set continue
		 * mapping were we left off.
		 */
		if (flags & DMA_CONTINUE)
		{
			/* only page leve gather is supported with TCWs,
			 * check that last mapping ended on a page boundary,
			 * and that this transfer starts on a page boundary
			 */
			if (((uint)baddr & (DMA_PSIZE-1)) ||
				(dma_ch[channel_id].unaligned))
			{
				control &= ~SLAVE_ENABLED;
				dma_ch[channel_id].flags |= DMA_PG_PROTECT;
			}
			first_tcw = dma_ch[channel_id].last_t;
			dma_ch[channel_id].flags |= DMA_CONTINUE;
		}
		else
		{
			/* calculate the first tcw for use by this slave
			 * channel
			 */
			first_tcw = first_slave_tcw +
					TAGS_PER_CHANNEL * slave_channel;
		}

		/* calculate the number of TCWs needed to map this transfer,
		 * and update the next available TCW
		 */
		num_tcws = ((ulong)(baddr + count - 1) >> DMA_L2PSIZE) -
				((ulong)baddr >> DMA_L2PSIZE) + 1;

		/* save next tcw to be used, and if the last transfer
		 * ended on a page boundary
		 */
		dma_ch[channel_id].unaligned = (((ulong)baddr + count) &
								(DMA_PSIZE-1));
		dma_ch[channel_id].last_t = first_tcw + num_tcws;

		maddr = (first_tcw << DMA_L2PSIZE) |
					((ulong)baddr & (DMA_PSIZE-1));

		/* calculate the range of TCWs neede to map transfer
		 */
		tcw_addr = (ulong *)TCW_EFF_ADDR(first_tcw);
		last_tcw = tcw_addr + num_tcws;
		daddr = first_tcw << DMA_L2PSIZE;

		do
		{
			/* RSC is cache consistent there is no need to
			 * invalidate or hide
			 */
			raddr = xmemqra(dp, baddr);

			/* check access permission to page.  If process does
			 * not have the propper permissions, then disable
			 * the channel and set error in flags
			 */
			access_permitted = xmemacc(dp, baddr, 
							flags & DMA_NOHIDE);
			if ( (access_permitted == XMEM_NOACC) ||
				((access_permitted == XMEM_RDONLY) &&
					(flags & DMA_READ))  )
			{
				control &= ~SLAVE_ENABLED;
				dma_ch[channel_id].flags |= DMA_PG_PROTECT;
			}

			iocc_rw(IOCC_WRITE, tcw_addr, SLAVE_TCW(raddr));

			/* RSC requires that a tlbi be done on the bus address
			 * after a TCW write
			 */
			tlbi(daddr);

			daddr += DMA_PSIZE;
			tcw_addr++;
			baddr += DMA_PSIZE;
		}
		while(tcw_addr < last_tcw);

	}

	/* set up channel status register, and slave address register.
	 */
	csr = CSR_EFF_ADDR(channel);
	if (flags & DMA_CONTINUE)
	{
		csrval = *csr;
		count += (SLAVE_CSR_COUNT(csrval) + 1);
		if (!(SLAVE_CSR_CONTRL(csrval) & SLAVE_ENABLED))
			control &= ~SLAVE_ENABLED;
	}
	else
	{
		s_regs = SLAVE_EFF_ADDR(slave_channel);
		s_regs->raddr = maddr;
	}
	*csr = SLAVE_CSR(control, slave_channel, count);

	io_det(ioccaddr);

}

/*
 * NAME: d_slave_rs1
 *
 * FUNCTION:
 *	This is the RS1/RS2 d_slave() implementation.
 *	This service supports the initialization of DMA channel for DMA
 *	slave block mode transfer using tagss.  This consists of mapping the
 *	transfer via the tag table and initializating the system DMA
 *	controller to perform the transfer.  This is reached via the
 *	branch table.  It only executes on RS1 ans RS2 machines
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
 *	The buffer must also not be accessed between the call to d_slave
 *	and the call to d_complete. See the global note on system
 *	interaction.
 *
 *	The effect of IOCC buffering is described in the global note on
 *	IOCC buffering.
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
 *	xmemdma, xmemflush, xmemacc, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_slave
 *
 */
void
d_slave_rs1(channel_id, flags, baddr, count, dp)

register int	channel_id;			/* channel to use */	
register int	flags;				/* control flags */	
register size_t	count;				/* length of transfer */
register char	*baddr; 			/* buffer address */
struct xmem	*dp;				/* cross mem descrip */
{
	register ulong	tag;			/* tag number */
	register ulong	raddr;			/* DMA address */
	register ulong	control;		/* DMA control */
	register ulong	access_permitted;	/* page protect permision */
	register ulong	channel;		/* normalized channel num */
	register ulong  ioccaddr;		/* io_att return value */
	volatile struct slave_regs *s_regs;	/* DMA registers */

	/* 
	 *  Ensure that the transfer is valid.
	 */
	ASSERT((count > 0) && (count <= DMA_MAX));
	ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);
	ASSERT(dma_ch[channel_id].flags & DMA_SLAVE);

#ifdef	DEBUG
       (void) d_check(channel_id, flags, baddr, count, dp, 0xdeadbeef, XMEM_HIDE);
#endif	DEBUG


	channel = ID_TO_NUM(channel_id);
	ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);
	if ( flags & DMA_CONTINUE )
	{
		dma_ch[channel_id].flags |= DMA_CONTINUE;
		count = map_tag_xfer(channel_id, flags, baddr, count, dp,
					ioccaddr);

		/* disable the channel if a page protection error was
		 * detected
		 */
		if (dma_ch[channel_id].flags & DMA_PG_PROTECT)
		{
			s_regs = SLAVE_EFF_ADDR(channel);
			s_regs->control = SLAVE_TAG(SLAVE_CHANNEL,
								LAST_TAG, 0);
		}

		io_det(ioccaddr);
		return;
	}
	else
		dma_ch[channel_id].last_t = TAGS_PER_CHANNEL * channel;

	/*
	 *  Check if the transfer is totally contained within a single page.
	 *  No tags are required since the entire transfer can be described
	 *  by the DMA controller registers.
	 */
	if ( ( ( (ulong)baddr & ( DMA_PSIZE - 1 ) ) + count ) <= DMA_PSIZE )
	{
		tag = LAST_TAG;
	}

	/*
	 *  The transfer crosses a page boundary. Map all pages in the
	 *  transfer, starting with the second.
	 */
	else
	{
		count = map_tag_xfer(channel_id, flags, baddr, count, dp,
					ioccaddr);
		tag = TAGS_PER_CHANNEL * channel;
	}

	/*  
	 *  Generate the DMA address based on the type of transfer.
	 *  This is only for the first, maybe only, page in the transfer.
	 */
	if ( flags & BUS_DMA )
		raddr = (ulong)baddr;
	else
	{
		if ( flags & (DMA_WRITE_ONLY|DMA_NOHIDE) )
		{
			raddr = xmemqflush ( dp, (int)baddr );
		} else {
			/*
			 * Call xmemdma to translate the address and
			 * to Hide the page.  Notice it also checks 
			 * for access violations in which case it won't
			 * keep the page hidden
			 */
			raddr = (ulong)xmemdma_pwr(dp, baddr, XMEM_HIDE | 
				XMEM_ACC_CHK);
			if (raddr == XMEM_FAIL)
				raddr = xmemqra(dp, baddr);
		}
	}

	/*
	 *  Setup control field to indicate target memory, mode and direction.
	 *  if an earlier error was detected then don't enable the channel
	 */
	if (dma_ch[channel_id].flags & DMA_PG_PROTECT)
		control = SLAVE_CHANNEL;
	else
		control = SLAVE_CHANNEL | SLAVE_ENABLED;

	/*
	 *   Determine r/w permissions to page.
	 */
	access_permitted = xmemacc(dp, baddr, flags & DMA_NOHIDE);
	if ( access_permitted == XMEM_NOACC )
	{
		/*  
		 *  If a process does not have write access
		 *  to a page that it intended to write to,
		 *  disable its channel and indicate access
		 *  violation in the flags field.
		 */
		control &= ~SLAVE_ENABLED;
		dma_ch[channel_id].flags |= DMA_PG_PROTECT;
	}
	else if ( access_permitted == XMEM_RDONLY )
		if ( (flags & DMA_READ) )
		{	
			control &= ~SLAVE_ENABLED;
			dma_ch[channel_id].flags |= DMA_PG_PROTECT;
		}

	if (!(flags & BUS_DMA))
	   control |= SLAVE_SYSTEM;
	if (flags & DMA_READ)		
	   control |= SLAVE_FROM_DEV;

	/*
	 *  Setup the system's DMA control registers for this
	 *  slave mode transfer.
	 *
	 *  The buffer is marked as invalid so that it will be
	 *  prefetched on the first transfer. This ensures that
	 *  a security exposure is not caused when a transfer
	 *  is terminated by a means other than the length
	 *  completion. We want to force the last partial 
	 *  buffer to all 0s.
	 *
	 *  The buffer should not be dirty, the device driver should
	 *  have called d_complete after the last transfer. This is
	 *  an assert since data is most likely already lost.
	 */
	s_regs = SLAVE_EFF_ADDR(channel);
	assert(!(s_regs->buf_ctl & BUF_DIRTY));
	s_regs->control = SLAVE_TAG(control, tag, count);
	s_regs->raddr = raddr;
	BUFF_INVAL(&s_regs->buf_ctl);
	io_det( ioccaddr );

}

/*
 * NAME: map_tag_xfer
 *
 * FUNCTION: This routine maps the transfer, except for the first page, to
 *	to the tags assigned to that channel.
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
 *	The buffer must also not be accessed between the call to d_slave
 *	and the call to d_complete. See the global note on system
 *	interaction.
 *
 *	The effect of IOCC buffering is described in the global note on
 *	IOCC buffering.
 *
 *	Exceptions may occur when accessing TCW's or TAG's. 
 *	Exception handlers have been implemented to catch 
 *	the exception, retry if possible, and log errors.
 *
 * RETURN VALUE DESCRIPTION: DMA_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma, xmemqflush, xmemacc, io_att, io_det
 */
static int
map_tag_xfer (
	int channel_id,				/* channel id	*/	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	ulong ioccaddr)				/* effective address of iocc */
{
	register int	rest;			/* part mapped in d_slave */
	register ulong	raddr;			/* DMA address */
	register ulong	control;		/* DMA control */
	register ulong	csr_control;		/* DMA control */
	register ulong	channel;		/* channel to use	*/
	register ulong	tag_num;		/* tag number */
	register ulong	access_permitted;	/* page protect permision */
	volatile struct slave_regs *s_regs;	/* DMA registers */
	volatile struct tag *t; 		/* tag */

	/*
	 * convert channel id to channel number.
	 */
	channel = ID_TO_NUM(channel_id);

	/*
	 *  Setup access to the IOCC.
	 */
	if ( !( flags & DMA_CONTINUE ) )
	{
		/*
		 *  Move past the first page of the transfer. It is processed in
		 *  d_slave.
		 */
		rest = (int)( DMA_PSIZE - ( (ulong)baddr & ( DMA_PSIZE - 1 )));
		count -= ( DMA_PSIZE - ( (ulong)baddr & ( DMA_PSIZE - 1 ) ) );
		ASSERT( count > 0 );
		baddr += ( DMA_PSIZE - ( (ulong)baddr & ( DMA_PSIZE - 1 ) ) );
	}

	/*
	 *  Setup control field to indicate target memory, mode and direction.
	 */
	control = 0;
	if (!(flags & BUS_DMA))
	   control |= SLAVE_SYSTEM;
	if (flags & DMA_READ)		
	   control |= SLAVE_FROM_DEV;

	/*
	 *  Update tag pointers.
	 */
	tag_num = dma_ch[channel_id].last_t;
	dma_ch[channel_id].last_t++;

	if ( flags & DMA_CONTINUE )
	{
		/*
		 *  Set the next tag pointer appropriately.
	 	 */
		if ( tag_num == ( TAGS_PER_CHANNEL * channel ))
		{
			/*
			 *  No tags have been mapped, must update the next 
			 *  tag pointer in the channel status register.
			 */
			s_regs = SLAVE_EFF_ADDR(channel);
			csr_control = s_regs->control;
			assert( SLAVE_NEXT_TAG(csr_control ) == 0xFFF );
			csr_control &=  SLAVE_NEXT;
			s_regs->control = csr_control | ( tag_num << 12 );
		}
		else
		{
			/*
			 *  Update the next tag pointer in the TAG.
			 */
			t = TAG_EFF_ADDR( tag_num - 1 );
			csr_control = iocc_rw( IOCC_READ, &(t->control), 0 );
			csr_control &=  SLAVE_NEXT;
			iocc_rw( IOCC_WRITE, &(t->control),
					 (csr_control | (tag_num << 12)) );
		}
	}

	/*
	 * Map each page after the first via a tag.
	 */
	for ( ;; tag_num++, dma_ch[channel_id].last_t++)
	{
		/*
		 *  Map a bus to bus transfer into the tag. In this case the
		 *  input address is the address to perform the DMA transfer to.
		 *  Note that the cache does not need to be flushed since it
		 *  is not in system memory.
		 */
		t = TAG_EFF_ADDR(tag_num);

		if ( flags & BUS_DMA )
		{
		     iocc_rw(IOCC_WRITE, &(t->raddr), (ulong)baddr);
		}
		/*
		 *  Map a transfer to system memory. The system memory address
		 *  must be converted from either a global or non-global
		 *  virtual address into a real address. Also, the system
		 *  memory cache must be flushed for this page so that data
		 *  will not be lost.
		 */
		else
		{
			if (flags & (DMA_WRITE_ONLY|DMA_NOHIDE))
			{
				raddr = xmemqflush (dp, (int)baddr);
			}
			else {
				/*
				 * Call xmemdma to translate the address and
				 * to Hide the page.  Notice it also checks 
				 * for access violations in which case it won't
				 * keep the page hidden
				 */
				raddr = (ulong)xmemdma_pwr(dp, baddr, XMEM_HIDE|
					XMEM_ACC_CHK);
				if (raddr == XMEM_FAIL)
					raddr = xmemqra(dp, baddr);
			}

		     	iocc_rw( IOCC_WRITE, &(t->raddr), (ulong)raddr);
		}

		/*
		 *   Determine r/w permissions to page.
		 */
		access_permitted = xmemacc(dp, baddr, flags & DMA_NOHIDE);
		if ( access_permitted == XMEM_NOACC )
		{
			/*  
			 *  If a process does not have write access
			 *  to a page that it intended to write to,
			 *  disable its channel and indicate access
			 *  violation in the flags field.
			 */
			dma_ch[channel_id].flags |= DMA_PG_PROTECT;
		}
		else if ( access_permitted == XMEM_RDONLY )
			if ( flags & DMA_READ )
			{	
				dma_ch[channel_id].flags |= DMA_PG_PROTECT;
			}

		if (( (ulong)baddr & ( DMA_PSIZE - 1 )) == 0 )
		{
			/*
			 *  Write the control information to the tag.
			 */
			if ( count <= DMA_PSIZE )
			{
				iocc_rw( IOCC_WRITE, &(t->control),
					 SLAVE_TAG(control, LAST_TAG, count) );
				break;		    /* End of the transfer */
			}

			iocc_rw( IOCC_WRITE, &(t->control),
				SLAVE_TAG(control, tag_num+1, DMA_PSIZE));
			/*
			 *  Move to the next page.
			 */
			count -= DMA_PSIZE;
			baddr += DMA_PSIZE;
		}
		else
		{
			/*
			 *  The DMA_CONTINUE flag was passed to d_slave
			 *  and the buffer was not aligned on a page
			 *  boundary. 
			 *  Write the control information to the tag.
			 */
			rest = (int)(DMA_PSIZE-((ulong)baddr & (DMA_PSIZE-1)));
			baddr += rest;
			if ( count <= rest )
			{
				iocc_rw( IOCC_WRITE, &(t->control),
					 SLAVE_TAG(control, LAST_TAG, count) );
				break;		    /* End of the transfer */
			}
			else
			{
				iocc_rw( IOCC_WRITE, &(t->control),
					SLAVE_TAG(control, tag_num+1, rest) );
				count -= rest;
			}
		}
	}

	/*
	 *  Clean up and return to the caller.
	 */
	return(rest);
}


/*
 * NAME: d_master_pwr
 *
 * FUNCTION: This service supports the initialization of the DMA
 *	channel for a DMA master block mode transfer.
 *	This only consists of mapping the transfer via the TCWs.
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
 *	The buffer must also not be accessed between the call to d_master
 *	and the call to d_complete. See the global note on system
 *	interaction.
 *
 *	The effect of IOCC buffering is described in the global note on
 *	IOCC buffering.
 *
 *	This service should not be called unless the DMA channel has been 
 *	allocated with the d_init service.
 *
 *	Exceptions may occur when accessing  TCW's or TAG's.
 *	Exception handlers have been implemented. 
 *	See PIO error recovery.
 *
 *	This routine can not alter the channel's CSR. This is because a
 *	transfer can be in progress for one device while a transfer is
 *	being mapped for another device. The exception to this is bus
 *	transfers. Here the caller must ensure that there are no transfers
 *	in progress because of the mapped by channel limitation.
 *	(The problem is that error information could be lost.)
 *
 *	This routine also can not check/alter the buffer control registers
 *	because a transfer can be in progress.
 *
 * RETURN VALUE DESCRIPTION: none
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma,  xmemqflush, xmemacc, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_master
 */
void 
d_master_pwr(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	register ulong	channel;		/* channel to use	*/
	register ulong	num_tcws;		/* number of TCWs to map */
	register ulong	raddr;			/* DMA address */
	register ulong  ioccaddr;		/* io_att return value */
	register ulong  access_permitted;	/* r/w access to a page */
	register ulong  tcw_sys_acc;		/* R/W access permissions */
	ulong  access;				/* R/W access permissions */
	volatile ulong	*csr;			/* channel status register */
	volatile ulong	*t;			/* tcw */
	ulong invaddr;				/* bus address for tlbi */

	/*
       	 * normalize channel	
	 */
	channel = ID_TO_NUM (channel_id);

	/*
	 *  Ensure that the transfer is valid.
	 */
	ASSERT(count > 0);
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);
	ASSERT(!(dma_ch[channel_id].flags & DMA_SLAVE));
	ASSERT(!(dma_ch[channel_id].flags & REGION_DMA));

	/*
	 *  Setup access to the IOCC.
	 */
	ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);

	/*
	 *  We must map one TCW for each page that contains part of 
	 *  the input buffer.
	 */
	num_tcws = ((ulong)(baddr + count - 1 ) >> DMA_L2PSIZE ) -
		   ((ulong)baddr >> DMA_L2PSIZE ) + 1;

#ifdef DEBUG
        (void) d_check(channel_id, flags, baddr, count, dp, daddr, XMEM_HIDE);
#endif	DEBUG


	/*
	 *  Map a bus to bus transfer into the TCWs. In this case the
	 *  input address is the address to perform the DMA transfer to.
	 *  Note that the buffer does not need to be flushed since it
	 *  is not in system memory.
	 *
	 *  The IOCC does not support mapping by page. It only supports
	 *  mapping by channel. Therefore a device can not have a transfer
	 *  mapped to system memory and another transfer mapped to the bus.
	 */
	if ( flags & BUS_DMA )
	{
		csr = CSR_EFF_ADDR(channel);
		*csr = MASTER_CSR(MASTER_BUS, channel, MASTER_NO_AUTHORITY);
		LOAD_IOCC_CONFIG();
	}

	/*
	 *  The buffer is in system memory. Map each page
	 *  containing a part of the buffer from the bus
	 *  to system memory starting with the bus address
	 *  supplied by the caller.
	 */
	else
	{
		if ( flags & DMA_WRITE_ONLY )
			tcw_sys_acc = TCW_SYS_RO;		
		else
			tcw_sys_acc = TCW_SYS_RW;		

		invaddr = daddr;

		for ( t = (volatile ulong *)TCW_EFF_ADDR(CALC_TCW_NUM(daddr));
		      t < (volatile ulong *)TCW_EFF_ADDR(
				CALC_TCW_NUM(daddr)+num_tcws); t++)
		{
			if ( flags & (DMA_WRITE_ONLY|DMA_NOHIDE) )
			{
				raddr = xmemqflush ( dp, (int)baddr );
			}
			else
			{
				/*
				 * Call xmemdma to translate the address and
				 * to Hide the page.  Notice it also checks 
				 * for access violations in which case it won't
				 * keep the page hidden
				 */
				raddr = (ulong)xmemdma_pwr(dp, baddr, XMEM_HIDE|
					XMEM_ACC_CHK);
				if (raddr == XMEM_FAIL)
					raddr = xmemqra(dp, baddr);
			}

			/*
			 *  Check this process's access permissions to
		  	 *  this real address. If the process does not have
			 *  read/write access to the page and it intended
			 *  to do a write operation, set the DMA_PAGE_PROTECT
			 *  bit in the dma_ch[i].flags field to indicate
		    	 *  access violation and allow read only access.
			 */
			access = tcw_sys_acc; /* reset default access */
			access_permitted = xmemacc(dp, baddr,
							flags & DMA_NOHIDE);
			if ( access_permitted == XMEM_NOACC )
			{
				/*  disable channel  */
				*csr &= ~MASTER_BUS;
				dma_ch[channel_id].flags |= DMA_PG_PROTECT;
			}
			else
				if ((access_permitted == XMEM_RDONLY) && 
				   ( !(flags & DMA_WRITE_ONLY)))
				{
				     access = TCW_SYS_RO;
				}

			iocc_rw( IOCC_WRITE, t, MASTER_TCW(raddr,channel,0,
				TCW_REFERENCED|TCW_CHANGED|access) );

			/* RSC requires a tlbi to be done on the effective
			 * bus address after every tcw write
			 */
			if (MACH_RSC())
				tlbi(invaddr);
			invaddr += DMA_PSIZE;
			baddr += DMA_PSIZE;
		}

		/* conditionally perform next buffer
		 * invalidate, if dual buffering is supported
		 */
		if (iocc_config & ICF_DUAL_BUF)
			NEXT_BUFF_INVAL(CALC_TCW_NUM(daddr));

	}

	/*
	 *  Clean up and return to the caller.
	 */
	io_det( ioccaddr );
}

/*
 * NAME: d_complete_pwr
 *
 * FUNCTION: This service is called upon completion of a DMA transfer.
 *	    It will flush the IOCC buffers and report any errors.
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
 *	The buffer must also not be accessed between the call to d_slave
 *	and the call to d_complete. See the global note on system
 *	interaction.
 *
 *	The effect of IOCC buffering is described in the global note on
 *	IOCC buffering.
 *
 *	This service should not be called unless the DMA channel has been 
 *	allocated with the d_init service.
 *
 *	Exceptions may occur when accessing TCW's or TAG's.
 *	Exception handlers will be implemented when the system method
 *	of handling them is defined. An exception can also occur
 *	when flushing an IOCC buffer.
 *
 * RETURN VALUE DESCRIPTION: see dma.h
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemdma, io_att, io_det, errsave
 *
 * ACCESSED:	Branch table entry d_complete
 */
int 
d_complete_pwr(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	register int	error;			/* DMA error status */
	register int	tcw_num;		/* TCW number  */
	volatile struct slave_regs *s_regs;	/* DMA registers */
	volatile struct buffer_regs *b_regs;	/* buffer registers */
	register ulong	channel;		/* normalized channel value */
	register ulong  ioccaddr;		/* io_att return value */
	register int	ipri;			/* interrupt priority */
	volatile ulong	*csr;			/* channel status register */
        register ulong  save_csr;               /* save/restore csr */
	register ulong	csr_val;		/* contents of CSR */
	int		buf_ctl;
	int		raddr;			/* contents of mem addr reg */


	/*
	 *  Ensure that the transfer is valid.
	 */
	ASSERT(count > 0);
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);

	/*
	* normalize channel	
	*/
	channel = ID_TO_NUM (channel_id);

#ifdef	DEBUG
       (void) d_check(channel_id, flags, baddr, count, dp, daddr, XMEM_UNHIDE);
#endif	DEBUG

	if ( flags & DMA_CONTINUE )
	{
		dma_ch[channel_id].flags &= ~DMA_CONTINUE;
		/*
		 *  Make the buffer accessible to processor accesses. The
		 *  buffer is hidden from the processor by d_slave and d_master
		 *  because a memory reference to a cache line that also
		 *  contains a DMA buffer can cause data to be lost.
		 */
		if (!(flags & (BUS_DMA|DMA_WRITE_ONLY|DMA_NOHIDE)))
		{
			register char	*naddr;
			naddr = (char *)((ulong)baddr & ( ~ ( DMA_PSIZE - 1 ) ));
			while( naddr < ( baddr + count ) )
			{
				(void)xmemdma_pwr(dp, naddr, XMEM_UNHIDE);
				naddr += DMA_PSIZE;
			}
		}
		return(0);
	}		


	/*
	 *  Setup access to the IOCC.
	 */
	ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid), 0);

	/*
	 *  Flush the IOCC buffer for a third party transfer.
	 */
	if (dma_ch[channel_id].flags & DMA_SLAVE)
	{
		/*
		 *  The IOCC buffer used for the transfer does not need to be
		 *  flushed if the transfer was a bus to bus transfer since
		 *  the IOCC buffers are not used for that type of transfer.
		 *  It also does not need to be flushed if it is not dirty.
		 *  It is later set to invalid so that a subsequent transfer
		 *  will have to refetch it from real memory.
		 */
		s_regs = SLAVE_EFF_ADDR(dma_ch[channel_id].cntchan);
                buf_ctl = s_regs->buf_ctl; /* read buf_control register */
                raddr   = s_regs->raddr; /* read mem addr */
                if (((buf_ctl & 0xE0000000) == 0x80000000) &&
                    ((raddr & 0x0000003F) == 0)) {
                        /*
                         * !!!WORKAROUND for XIO problem!!!
                         * If the buffer control bits 0..2 indicate b'100
                         * and the real address is at a 64byte boundary, we
                         * Need to decrement the raddr by 1 so the XIO
                         * will be sure to flush the buffers correctly
                         */
                        s_regs->raddr = raddr - 1;
                }
		if (( ! ( flags & BUS_DMA )) && (buf_ctl & BUF_DIRTY))
		{

			/*
			 *  Care must be taken when flushing the buffer because
			 *  an exception can be reported in response to the
			 *  IOCC flush command if an error occurs during the
			 *  flush.  Set up exception handler and flush 
			 *  IOCC buffer for a DMA slave transfer.  
			 *  The exception handler has been added
			 *  that can catch this exception and move the error
			 *  status from the PIO CSR to the DMA channel's CSR.
			 *  
			 */

			SLAVE_FLUSH();		

		}

		/*
		 *  Get the error status for this transfer and mark the
		 *  buffer as invalid.
		 */
		csr = CSR_EFF_ADDR(channel);
		csr_val = *csr;
		error = (int)SLAVE_STATUS(csr_val);

		/* The correct channel number nust be stored in the CSR
		 * even if the arbitration level is disabled.  Failing
		 * to do this causes bus-timeouts on RSC
		 */
		if (iocc_config & ICF_SLAVE_TCW)
		{
			*csr = SLAVE_CSR(SLAVE_CHANNEL,
					dma_ch[channel_id].cntchan, 0);
		}
		else
		{
			*csr = SLAVE_TAG(SLAVE_CHANNEL, LAST_TAG, 0);
		}
		ASSERT(!(s_regs->buf_ctl & BUF_DIRTY));
		BUFF_INVAL(&s_regs->buf_ctl);

	}
	else
	{

		/*
		 *  The IOCC buffer used for the transfer does not need to be
		 *  flushed if the transfer was a bus to bus transfer since
		 *  the IOCC buffers are not used for that type of transfer.
		 */
		if ( ! ( flags & BUS_DMA ) )
		{

			/*
			 *  Care must be taken when flushing the buffer because
			 *  an exception can be reported in response to the
			 *  IOCC flush command if an error occurs during the
			 *  flush. An exception handler has been added
			 *  that can catch this exception and move the error
			 *  status from the PIO CSR to the DMA channel's CSR.
			 */
			tcw_num = CALC_TCW_NUM( daddr );
			MASTER_FLUSH( tcw_num );
		}

		/*
		 *  Get the error status for this transfer. The buffer can
		 *  not be checked or marked as invalid because another
		 *  transfer can be in progress for the device.
		 */
		csr = CSR_EFF_ADDR(channel);
		csr_val = *csr;
		LOAD_IOCC_CONFIG();
		error = (int)CSR_STATUS(csr_val);

		/*
		 *  Re-initialize the DMA channel as it was last mapped
		 *  if an error has occurred. Must remap to allow bus
		 *  to system or system to bus transfers.
		 */
		if ( error > DMA_NO_ERROR )
			if ( flags & BUS_DMA )
			{
				*csr = MASTER_CSR(MASTER_BUS,channel,
					   MASTER_NO_AUTHORITY);
				LOAD_IOCC_CONFIG();
			}
			else
			{
				*csr = MASTER_CSR(MASTER_SYSTEM|MASTER_PREFETCH
					   , channel, MASTER_FULL_AUTHORITY);
				LOAD_IOCC_CONFIG();
			}

	}

	/*
	 *  Restore state for return to the caller.
	 */
	io_det( ioccaddr );

	/*
	 *  Make the buffer accessible to processor accesses. The
	 *  buffer is hidden from the processor by d_slave and d_master
	 *  because a memory reference to a cache line that also
	 *  contains a DMA buffer can cause data to be lost.
	 */
	if (!(flags & (BUS_DMA|DMA_WRITE_ONLY|DMA_NOHIDE)))
	{
		register char	*naddr;
		naddr = (char *)((ulong)baddr & ( ~ ( DMA_PSIZE - 1 ) ));
		while( naddr < ( baddr + count ) )
		{
			(void)xmemdma_pwr(dp, naddr, XMEM_UNHIDE);
			naddr += DMA_PSIZE;
		}
	}

	/*
	 *  All done if no errors.
	 */
	if (error <= DMA_NO_ERROR)
		return(DMA_SUCC);
	else
		return(complete_error(channel_id, csr_val, error));


}

/*
 * NAME: d_cflush_pwr
 *
 * FUNCTION: This service flushes the processor cache for the virtual 
 * 	    address area defined by baddr and count length. For XIO 
 *	    (the follow on IOCCi) it also executes the next buffer invalidate
 *           command. This is done to invalidate the XIO read ahead buffer when
 *           long term mapping of dma buffers is performed using the
 *	    DMA_NOHIDE style of dma which does not hide the pages involved.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing the interrupt level.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	vm_cflush	
 *
 * RETURN VALUE DESCRIPTION: 0  for no error or
 *			    EINVAL for invalid parameters (from vm_cflush)
 *
 * ACCESSED:	Branch table entry d_cflush
 */
int  
d_cflush_pwr(
	int channel_id,				/* channel to use */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	char *daddr)				/* bus address */
{
	int rc;
	ulong ioccaddr;

	/*  Ensure that the transfer is valid.
	 */
	ASSERT(count > 0);
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);

	/* flush the processor cache
	 */
	rc = vm_cflush(baddr, (int) count);

	/* conditionally perform next buffer invalidate command, if
	 * dual buffering is supported
	 */
	if (iocc_config & ICF_DUAL_BUF)
	{
		ioccaddr = (ulong)io_att(
				IOCC_HANDLE(dma_ch[channel_id].bid), 0);
		NEXT_BUFF_INVAL(CALC_TCW_NUM(daddr));
		io_det(ioccaddr);
	}

	return(rc);
}

/*
 * NAME: d_move_pwr
 *
 * FUNCTION:  This services provides consistent access to memory
 *	when a device and its driver both have access to system
 *	memory.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service can be called by a program executing on either 
 *	the interrupt level or the process level.
 *
 *	It only page faults on the stack when called under a process.
 *
 * NOTES:
 *	The RSC CSR 15 read problem does not need to be applied here
 *	since this function is not supported on RSC
 *
 * RETURN VALUE DESCRIPTION:
 *	return from xmemin/xmemout
 *	EINVAL if non-buffered IOCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *	xmemin,	xmemout, io_att, io_det
 *
 * ACCESSED:	Branch table entry d_move
 */
int 
d_move_pwr(
	int channel_id,				/* channel to use */	
	int flags,				/* control flags */	
	char *baddr,				/* buffer address */
	size_t count,				/* length of transfer */
	struct xmem *dp,			/* cross mem descrip */
	char *daddr)				/* bus address */
{
	register ulong	ioccaddr;		/* iocc base address */
	register ulong  busaddr;		/* i/o base address */
	register ulong	channel;		/* channel to use */
	register ulong	rc;			/* return code */
        register ulong  save_csr;               /* save/restore csr */
	register int	ipri;			/* interrupt priority */
	volatile ulong	*csr;			/* channel status register */
	register int    blks;                   /* number of 4K pages */
	register ulong  busaddr1;
	register int    buid;
	register int    i;

	/*
	 * This opperation is invalide on non-buffered IOCC
	 */
	if (!ICF_BUFFERED(iocc_config))
		return(EINVAL);

	/*
       	 * normalize channel	
	 */
	channel = ID_TO_NUM (channel_id);

	/*
	 *  Ensure that the transfer is valid.
	 */
	ASSERT(count > 0);
        ASSERT((channel_id < MAX_DMA_ID) && (channel_id >= 0));
	ASSERT(dma_ch[channel_id].allocated);
	ASSERT(!(dma_ch[channel_id].flags & DMA_SLAVE));
	ASSERT(!(dma_ch[channel_id].flags & REGION_DMA));

	if (curthread->t_graphics == NULL)
	{
		/*
		 *  Setup access to the IOCC.
		 *  Disable to INTIODONE to support task switching.
		 */
		ipri = i_disable( INTIODONE );	
		ioccaddr =
		  (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid) ,0);
		csr = CSR_EFF_ADDR(15);
		save_csr = *csr;
		*csr = MASTER_CSR(0,channel,MASTER_NO_AUTHORITY);
		LOAD_IOCC_CONFIG();
		busaddr =
		  (ulong)io_att(VIRTUAL_HANDLE(dma_ch[channel_id].bid), daddr);
		
		if (flags & DMA_READ)
			rc = xmemout(busaddr,baddr,count,dp);
		else
			rc = xmemin(baddr,busaddr,count,dp);
		
		/*
		 *  Clean up and return.
		 */
		io_det(busaddr);	/* detach from segment register */
		*csr = save_csr;	/* restore channel status register */
		LOAD_IOCC_CONFIG();	/* ensures invalid operation logged */
		io_det(ioccaddr);	/* detach from segment register */
		i_enable ( ipri );	/* enable interrupts to old pri */
		return( rc );
	}	
	else {

	    /* the current thread is a graphics thread
	     */
	    ioccaddr = (ulong)io_att(IOCC_HANDLE(dma_ch[channel_id].bid) ,0);
	    csr = CSR_EFF_ADDR(15);
	    busaddr1 = busaddr =
	    (ulong)io_att(VIRTUAL_HANDLE(dma_ch[channel_id].bid), daddr);
	    
	    blks = (count + DMA_PSIZE - 1) / DMA_PSIZE;

	    for (i = 0; i < blks; i++)
	    {
		/* disable interrupts to prevent RCM from changing csr15
	         */  
		 ipri = i_disable(INTMAX);
		 *csr = MASTER_CSR(0,channel,MASTER_NO_AUTHORITY);
		 LOAD_IOCC_CONFIG();
		 if (flags & DMA_READ)
		 {	 
			 if (count < DMA_PSIZE)
			 {
			     rc = xmemout(busaddr1,baddr,count,dp);
		         }
		         else {
			     rc = xmemout(busaddr1,baddr,DMA_PSIZE,dp);
		         }
		 }
		 else {
		         if (count < DMA_PSIZE)
			 {
			     rc = xmemin(baddr,busaddr1,count,dp);
		         }
			 else {
		             rc = xmemin(baddr,busaddr1,DMA_PSIZE,dp);
			 }
	         }		 
		 /* enable interrupts to allow task switching
		  */ 
		 i_enable(ipri);

		 /* break if failed in xmemout()/xmemin()
		  */
		 if (rc != XMEM_SUCC)  break;

		 busaddr1 += DMA_PSIZE;
		 baddr += DMA_PSIZE;
		 count -= DMA_PSIZE;
	    }

	     /*  Clean up  */
	    io_det(busaddr);		/* detach from segment register */
	    io_det(ioccaddr);		/* detach from segment register */

	    /* Set up access to displays in the IOCC bus with the channel
	     * for a graphics thread.  
	     * Make sure csr15 is set up correctly (even when it does not
	     * currently own any display on the bus)
	     */
	    buid = BID_TO_BUID(dma_ch[channel_id].bid);
	    ipri = disable_ints();
	    RESTORE_CSR15(buid);
	    enable_ints(ipri);
	}                                                       
	return( rc );
}

/*
 *
 * NAME: d_bflush_pwr
 *
 * FUNCTION: This service flushes the IOCC buffer associated with a TCE number.
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This service can be called by a program executing on either
 *       the interrupt level or the process level.
 *
 *       It only page faults on the stack when called under a process.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: 0 on successful completion.
 *                           DMA_BAD_ADDR due to I/O exception
 *
 * ACCESSED:  Branch table entry d_bflush
 *
 *
 */

int 
d_bflush_pwr( int  channel_id,  /* Channel ID as returned by d_init() */
	      vmhandle_t buid,	  /* Bus Unit ID */
	      char *daddr)	  /* bus address (dd managed TCE_address) */
{
	struct {
		uint oldpri;
		uint oldcsr;
	}errstate;
	volatile uint *csr;
	int	rc = DMA_SUCC;
	uint	ioccaddr;
	int	tcw_num;

	ioccaddr = (uint)io_att(IOCC_HANDLE(buid), 0); /* IOCC access*/
	tcw_num = CALC_TCW_NUM(daddr);	/* compute TCW number */
	csr = (volatile uint *)CSR_EFF_ADDR(15);     /* get CSR15 address */
	/*
	 * 	Perform flush - calls master_flush() of ml/dma_pwr.s
	 * 	which takes the CSR address, CSR value, Buffer flush
	 *	address, IOCC config register address, and a reference
	 *	to an error state storage
	 */
	if (master_flush(csr, MASTER_CSR( 0, ID_TO_NUM(channel_id), 
		MASTER_NO_AUTHORITY), ((int)TCW_EFF_ADDR(tcw_num)) 
		| 0x2, ioccaddr|0x0400010, &errstate)) {
		/*
		 * 	if error (I/O exception)
		 * 	set return code, and enable interrupts
		 */
		rc = DMA_BAD_ADDR;
		i_enable(errstate.oldpri);
	}
#ifdef _POWER_RSC
	else {
           /* Only need to restore csr15 for RSC when i/o exception
   	      does not occur.  This is because if i/o exception    
	      occurs, csr15 will have been set up correctly by     
	      io_exception() for graphics threads.               
	    */                                                     
	   if (__power_rsc())
	   {                                                     
	       int  oldpri;
	       int  buid1;
	       
	       buid1 = BID_TO_BUID(dma_ch[channel_id].bid);
	       
	       /* disable external interrupts */
	       oldpri = disable_ints();

	       /* Set up access to displays in the IOCC bus with the channel
		* for a graphics thread.  
		* Make sure csr15 is set up correctly (even when it does not
		* currently own any display on the bus)
		*/
	       RESTORE_CSR15(buid1);

	       /* re-enable external interrupts */
	       enable_ints(oldpri);
	   }
        }
#endif /* _POWER_RSC */

	io_det(ioccaddr);
	return(rc);
}

#endif /* _POWER_RS */
