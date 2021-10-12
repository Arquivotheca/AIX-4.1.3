static char sccsid[] = "@(#)81	1.1  src/bos/kernext/fddidiag/fddidiag_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:00:11";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_dma_acc
 *		fddi_get_trace
 *		fddi_hcr_cmd
 *		fddi_mem_acc
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/ioacc.h>
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/iocc.h>
#include <sys/adspace.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;
/*
 * NAME: fddi_mem_acc()
 *                                                                    
 * FUNCTION: allows memory and DMA access to the adapter
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *
 *	EINVAL	Indicates an invalid parameter.
 *	EFAULT	Indicates that an invalid address was specified.
 *	EIO	Indicates an error.  Status field contains
 *		detailed error information.
 */  

int
fddi_mem_acc(	fddi_acs_t	*p_acs,		/* ACS ptr */
		fddi_open_t	*p_open,	/* Open element ptr */
		fddi_mem_acc_t	*p_arg,		/* Memory access struct */
		dev_t		devflag)	/* devflag */
{
	fddi_mem_acc_t	mem;		/* local mem acc structure */
	int 		rc=0;		/* return code */
	ushort		*p_buf;		/* ptr for kernel buf mallocd */
	int		bus;		/* Access Bus Memory */
	int		i;		/* for loop counter */


	FDDI_TRACE("DmaB", p_acs, p_open, p_arg);

	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	/*
	 * copy in the ioctl memory access structure.
	 */
	
	if( rc = MOVEIN(devflag, p_arg, &mem, sizeof(fddi_mem_acc_t) ))
	{
		FDDI_TRACE("Dma1", p_acs->ctl.mode, mem.status, EFAULT);
		return(EFAULT);
	}

	mem.status = CIO_OK;

	/*
	 * check to see if the user has opened the driver in 
	 * diagnostic mode
	 */
	if ( p_acs->ctl.mode != 'D' )
	{
		rc = EIO; 
		mem.status = FDDI_NOT_DIAG_MODE;
		if(MOVEOUT(devflag, &mem, p_arg, sizeof(fddi_mem_acc_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dma2", p_acs->ctl.mode, mem.status, rc);
		return(rc);
	}	
	else if ( (p_acs->dev.state != FDDI_DWNLD) &&
		( p_acs->dev.state != FDDI_NULL) )

	{
		rc = EIO;
		mem.status = FDDI_STARTED;
		if(MOVEOUT(devflag, &mem, p_arg, sizeof(fddi_mem_acc_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dma3", p_acs->ctl.mode, mem.status, rc);
		return(rc);
	}
	/*
	 * sanity check the structure
	 */
	
	FDDI_TRACE("Dma4", mem.opcode, mem.num_transfer, mem.ram_offset);
	switch(mem.opcode)
	{
		case FDDI_WR_SHARED_RAM:
		case FDDI_RD_SHARED_RAM:

			/*
			 * sanity check the structure
			 *
			 * only one transfer is allowed
			 * for reading and writing shared ram.
			 */

			if ( mem.num_transfer != 1 )
				rc = EIO;
			else if ( (mem.buff_len1 < 1) ||
				(mem.buff_len1 > 512) )
				rc = EIO;
			else if ( ( mem.ram_offset < 0)  ||
				( (mem.buff_len1 + mem.ram_offset) > 512) )
				rc = EIO;


			if ( rc )
			{
				/* 
				 * a parameter was out of range
				 * return an error
				 */
				mem.status = CIO_BAD_RANGE;
				if(MOVEOUT(devflag, &mem, p_arg, 
						sizeof(fddi_mem_acc_t) ))
					rc = EFAULT;

				FDDI_TRACE("Dma5", p_acs->ctl.mode, 
					mem.status, rc);
				return(rc);

			}
			/*
			 * xmalloc the required memory to 
			 * do the transfer
			 */
			p_buf = xmalloc(mem.buff_len1, FDDI_WORDSHIFT,
					pinned_heap );

			if ( p_buf == NULL )
			{	
				FDDI_TRACE("Dma6", ENOMEM, 0, 0);
				return(ENOMEM);
			}

			/*
			 * get access to bus memory
			 */
			bus = BUSMEM_ATT(p_acs->dds.bus_id, 
				(ulong)p_acs->dds.bus_mem_addr);

			/*
			 * if this is a write operation,
			 * we need to copy in the data that is to
			 * be written to the adapter
			 */
			if ( mem.opcode == FDDI_WR_SHARED_RAM )
			{
				if ( rc = MOVEIN(devflag, mem.buffer_1, p_buf,
						mem.buff_len1) )
				{
					xmfree(p_buf, pinned_heap);
					FDDI_TRACE("Dma7", rc, mem.buffer_1,
						p_buf);
					BUSMEM_DET(bus);
					return(EFAULT);
				} 

				PIO_PUTSTRX(bus + mem.ram_offset,p_buf,
					mem.buff_len1);
			} /* end if write operation */
			else
			{
				PIO_GETSTRX(bus + mem.ram_offset, p_buf,
					mem.buff_len1);

				/* 
				 * copy out the data that was just
				 * read from the adapter
				 */
				
				if ( rc = MOVEOUT(devflag, p_buf, 
						mem.buffer_1, mem.buff_len1) )
				{
					FDDI_TRACE("Dma8", rc, p_buf,
						mem.buffer_1);
					rc = EFAULT; /* normalize rc */
				}
			} /* end else read operation */

			(void)xmfree(p_buf, pinned_heap);

			BUSMEM_DET(bus);

			break;
		case FDDI_WR_MEM_FDDI_RAM:
		case FDDI_RD_MEM_FDDI_RAM:
		case FDDI_WR_MEM_NP_BUS_DATA:
		case FDDI_RD_MEM_NP_BUS_DATA:
		case FDDI_WR_MEM_NP_BUS_PROGRAM:
		case FDDI_RD_MEM_NP_BUS_PROGRAM:

			mem.status = CIO_OK;
			rc = fddi_dma_acc(p_acs, p_open, &mem);
			if (MOVEOUT(devflag, &mem, p_arg, 
						sizeof(fddi_mem_acc_t) ))
					rc = EFAULT;


			break;

		default:
			/*
			 * Invalid opcode. return error
			 */
			rc = EINVAL;
			FDDI_TRACE("Dma9", mem.opcode, mem.ram_offset,
				mem.num_transfer);

			break;

	} /* end switch */

	/* !!!
	 * set the HARD FAIL carry over flag that
	 * will require microcode download to occur
	 * after we our last close for this device.
	 */
	FDDI_TRACE("DmaE", mem.status, rc, 0);
	return(rc);
} /* end fddi_mem_acc() */

/*
 * NAME: fddi_dma_acc()
 *                                                                    
 * FUNCTION: 
 *	This function handles the DMA portion of the FDDI_MEM_ACC ioctl.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *	This routine builds the ICR command parameter block as required
 *	for the passed in fddi_mem_acc_t structure.  It will xmalloc the
 *	required kernel buffers (upto 3 64K buffers).
 *
 *	DMA is setup for each kernel buffer. After the ICR command block
 *	is built and DMA all setup, this routine will call the
 *	fddi_do_dma() routine (a bottom half routine) to do the actual
 *	set up of the adapter.  The fddi_do_dma() will return when
 *	the ICR command has completed or timed out.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	This routine modifies the FDDI_MEM_ACC variables that
 *	are in the dev section of the ACS.
 *
 * RETURNS: 
 *
 *	EINVAL	Indicates an invalid parameter.
 *	EFAULT	Indicates that an invalid address was specified.
 *	EIO	Indicates an error.  Status field contains
 *		detailed error information.
 */  

int
fddi_dma_acc(	fddi_acs_t	*p_acs,		/* ACS ptr */
		fddi_open_t	*p_open,	/* Open element ptr */
		fddi_mem_acc_t	*p_mem)		/* Mem acces structure */
{
	int	rc=0, rc2=0, i, j;
	int 	opcode;
	ushort	icr;


	fddi_acs_dev_t	*p_dev;	/* ptr to dev section of acs */


	FDDI_TRACE("DdaB", p_mem->opcode, p_mem->ram_offset, 
		p_mem->num_transfer);
	
	/* 
	 * sanity check the parmeters
	 */
	if ( (p_mem->num_transfer < 1) ||
		(p_mem->num_transfer > 3) )
	{
		p_mem->status = CIO_BAD_RANGE;
		return(EIO);
	}



	p_dev = &p_acs->dev;

	/*
	 * move the mem access data into a more
	 * useful form.
	 *
	 * NB:
	 *	We round up the amount of pages to xmalloc
	 *	to include the whole page.  This is to avoid
	 *	problems when we hide the page(s) when we
	 *	set up for DMA.
	 */
	p_dev->l_ubuf[0] = p_mem->buff_len1;
	p_dev->l_kbuf[0] = ( p_mem->buff_len1 + 
			(PAGESIZE - (p_mem->buff_len1 % PAGESIZE) ) );

	p_dev->l_ubuf[1] = p_mem->buff_len2;
	p_dev->l_kbuf[1] = ( p_mem->buff_len2 +
			(PAGESIZE - (p_mem->buff_len2 % PAGESIZE) ) );

	p_dev->l_ubuf[2] = p_mem->buff_len3;
	p_dev->l_kbuf[2] = ( p_mem->buff_len3 +
			(PAGESIZE - (p_mem->buff_len3 % PAGESIZE) ) );

	p_dev->p_ubuf[0] = p_mem->buffer_1;
	p_dev->p_ubuf[1] = p_mem->buffer_2;
	p_dev->p_ubuf[2] = p_mem->buffer_3;

	/*
	 * sanity check the length of transfers
	 */
	for ( i=0; i<p_mem->num_transfer; ++i)
	{
		if ( (p_dev->l_ubuf[i] < 1) || (p_dev->l_ubuf[i] > 65534) )
		{
			p_mem->status = CIO_BAD_RANGE;
			return(EIO);
		}
	}
	/*
	 * xmalloc kernel buffers
	 */
	for( i=0; i < p_mem->num_transfer; ++i )
	{

		p_dev->p_kbuf[i] = xmalloc( p_dev->l_kbuf[i], 
					PGSHIFT, pinned_heap);

		if ( p_dev->p_kbuf[i] == NULL )
		{
			for ( j=0; j<i; ++ j)
			{
				xmfree( p_dev->p_kbuf[j], pinned_heap );
				p_dev->p_kbuf[j] = NULL;
			}
			return(ENOMEM);
		} 

	} /* end for loop to xmalloc buffers */

	/* set up the xmem descriptor for the kernel buffers just xmalloced */
	p_dev->dma_xmd.aspace_id = XMEM_GLOBAL;

	/*
	 * if it is a write operation, copy in the data 
	 * into the kernel buffers
	 */
	if ( (p_mem->opcode == FDDI_WR_MEM_FDDI_RAM) ||
		(p_mem->opcode == FDDI_WR_MEM_NP_BUS_DATA) ||
		(p_mem->opcode == FDDI_WR_MEM_NP_BUS_PROGRAM))
	{
		opcode = FDDI_WRITE;

		for ( i=0; i < p_mem->num_transfer; ++i)
		{
			if ( rc = MOVEIN(p_open->devflag, p_dev->p_ubuf[i],
					p_dev->p_kbuf[i], p_dev->l_kbuf[i]) )
			{
				/* copy in of data failed */
				for ( j=0; j < p_mem->num_transfer; ++j)
				{
					(void)xmfree(p_dev->p_kbuf[j],
						pinned_heap);
					p_dev->p_kbuf[j] = NULL;
					p_dev->l_kbuf[j] = 0;
					p_dev->p_ubuf[j] = NULL;
				}
				return(EFAULT);
			}
		} /* end for loop to copy in data */
	} /* end if write operation */
	else
		opcode = FDDI_READ;


	/*
	 * now we flush and d_master() each buffer location
	 */
	for ( i=0; i< p_mem->num_transfer; ++i)
	{
		/*
		 * we now flush the buffer so as to sync up 
		 * the cache lines
		 */
		(void)vm_cflush( p_dev->p_kbuf[i], p_dev->l_kbuf[i] );

		if ( opcode == FDDI_READ )
		{
			d_master(p_dev->dma_channel, DMA_READ,
				p_dev->p_kbuf[i], p_dev->l_kbuf[i],
				&p_dev->dma_xmd,
				p_dev->p_d_kbuf[i] );

		}
		else
		{
			d_master(p_dev->dma_channel, DMA_WRITE_ONLY,
				p_dev->p_kbuf[i], p_dev->l_kbuf[i],
				&p_dev->dma_xmd,
				p_dev->p_d_kbuf[i] );
		}
				
	} /* end for loop for d_master() calls */

	/* 
	 * build instruction command reg value
	 */
	icr = 0x0000;
	if (opcode == FDDI_READ)
		icr |= 0x8000;

	if ( p_mem->num_transfer == 2 )
		icr |= 0x2000;
	else if ( p_mem->num_transfer == 3 )
		icr |= 0x2200;


	switch( p_mem->opcode )
	{
		case FDDI_WR_MEM_FDDI_RAM:
		case FDDI_RD_MEM_FDDI_RAM:
			icr |= 0x4c00;
			break;
		case FDDI_WR_MEM_NP_BUS_DATA:
		case FDDI_RD_MEM_NP_BUS_DATA:
			icr |= 0x4800;
			break;
		default:
			/*
			 * case FDDI_WR_MEM_NP_BUS_PROGRAM: and
			 * case FDDI_RD_MEM_NP_BUS_PROGRAM: are the
			 * default hw components to access.
			 */
			icr |= 0x4400;
			break;
	} /* end switch */


	rc2 = fddi_do_dma(p_acs, icr, p_mem->num_transfer, p_mem->ram_offset);

	p_mem->status = p_acs->dev.ioctl_status;	/* return results */
	/*
	 * now we d_complete() and free each buffer location
	 */
	for ( i=0; i< p_mem->num_transfer; ++i)
	{
		if ( opcode == FDDI_READ )
		{
			d_complete(p_dev->dma_channel, DMA_READ,
				p_dev->p_kbuf[i], p_dev->l_kbuf[i],
				&p_dev->dma_xmd,
				p_dev->p_d_kbuf[i] );

			/* copy out the data that was read from
			 * the adapter card 
			 */
			if ( MOVEOUT(p_open->devflag, p_dev->p_kbuf[i], 
					p_dev->p_ubuf[i], p_dev->l_ubuf[i]) )
				rc2 = EFAULT;
				

		}
		else
		{
			d_complete(p_dev->dma_channel, DMA_WRITE_ONLY,
				p_dev->p_kbuf[i], p_dev->l_kbuf[i],
				&p_dev->dma_xmd,
				p_dev->p_d_kbuf[i] );
		}
				
		(void)xmfree(p_dev->p_kbuf[i], pinned_heap);

	} /* end for loop for d_complete() calls */

	FDDI_TRACE("DdaE", p_mem->status, rc, 0);
	return(rc2);
} /* end fddi_dma_acc() */

/*
 * NAME: fddi_hcr_cmd()
 *                                                                    
 * FUNCTION: allows the user to issue an hcr command
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *
 *	EINVAL	Indicates an invalid parameter.
 *	EFAULT	Indicates that an invalid address was specified.
 *	EIO	Indicates an error.  Status field contains
 *		detailed error information.
 */  

int
fddi_hcr_cmd(	fddi_acs_t	*p_acs,		/* ACS ptr */
		fddi_open_t	*p_open,	/* Open element ptr */
		fddi_hcr_cmd_t	*p_arg,		/* Memory access struct */
		dev_t		devflag)	/* devflag */
{
	int 		rc=0;		/* return code */
	fddi_cmd_t	*p_cmd;
	int		i;		/* for loop counter */
	extern		int fddi_hcr_cmd_cmplt();


	FDDI_TRACE("DhcB", p_acs, p_open, p_arg);

	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	/*
	 * copy in the ioctl memory access structure.
	 */
	
	if (MOVEIN(devflag,p_arg, &p_acs->dev.hcr_cmd, sizeof(fddi_hcr_cmd_t) ))
	{
		FDDI_TRACE("Dhc1", p_acs->ctl.mode, 0, EFAULT);
		return(EFAULT);
	}

	if (p_acs->dev.hcr_cmd.l_cpb > (FDDI_CPB_SIZE << 1))
	{
		p_acs->dev.hcr_cmd.status = CIO_INV_CMD;
		FDDI_TRACE("Dhc2", p_acs->dev.hcr_cmd.hcr_val, 
				p_acs->dev.hcr_cmd.l_cpb,0); 
		if (MOVEOUT(devflag, &p_acs->dev.hcr_cmd, p_arg, 
				sizeof(fddi_hcr_cmd_t)))
			return(EFAULT);
		return(EIO);
	}
	/*
	 * check to see if the user has opened the driver in 
	 * diagnostic mode
	 */
	if ( p_acs->ctl.mode != 'D' )
	{
		rc = EIO; 
		p_acs->dev.hcr_cmd.status = FDDI_NOT_DIAG_MODE;
		if(MOVEOUT(devflag, &p_acs->dev.hcr_cmd, p_arg, 
				sizeof(fddi_hcr_cmd_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dhc3", p_acs->ctl.mode, p_acs->dev.hcr_cmd.status,
				 rc);
		return(rc);
	}	
	else if ( (p_acs->dev.state != FDDI_DWNLD) &&
		( p_acs->dev.state != FDDI_NULL) )

	{
		rc = EIO;
		p_acs->dev.hcr_cmd.status = FDDI_STARTED;
		if(MOVEOUT(devflag, &p_acs->dev.hcr_cmd, p_arg, 
				sizeof(fddi_hcr_cmd_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dhc4", p_acs->ctl.mode, p_acs->dev.hcr_cmd.status,
				 rc);
		return(rc);
	}
	FDDI_TRACE("Dhc5", p_acs->dev.hcr_cmd.hcr_val, p_acs->dev.hcr_cmd.hsr_val, 0);
	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) fddi_hcr_cmd_cmplt;
	p_cmd->cmd_code = p_acs->dev.hcr_cmd.hcr_val;
	p_cmd->cpb_len = p_acs->dev.hcr_cmd.l_cpb;
	bcopy(p_acs->dev.hcr_cmd.cpb, p_cmd->cpb, p_acs->dev.hcr_cmd.l_cpb);

	for (i=0; i<(p_acs->dev.hcr_cmd.l_cpb>>1); i++)
	{
		p_acs->dev.hcr_cmd.cpb[i] =SWAPSHORT(p_acs->dev.hcr_cmd.cpb[i]);
	}
	

	if (rc = fddi_send_cmd(p_acs, p_open, p_cmd))
	{
		FDDI_TRACE("Dhc6", p_arg,0,0);
		return(rc);
	}

	p_acs->dev.hcr_cmd.status = p_acs->dev.ioctl_status;

	for (i=0; i<FDDI_CPB_SIZE; i++)
	{
		p_acs->dev.hcr_cmd.cpb[i] =SWAPSHORT(p_acs->dev.hcr_cmd.cpb[i]);
	}

	if (MOVEOUT(devflag, &p_acs->dev.hcr_cmd, p_arg,sizeof(fddi_hcr_cmd_t)))
		return(EFAULT);
	if (p_acs->dev.hcr_cmd.status != CIO_OK)
		return(EIO);
	FDDI_TRACE("DhcE", p_acs->dev.hcr_cmd.status, p_acs->dev.hcr_cmd.hsr_val, 0);
	return(0);

}

/*
 * NAME: fddi_get_trace()
 *                                                                    
 * FUNCTION: 
 *	This only ioctl returns the device driver
 *	internal trace table, the ACS, and the adapter's
 *	internal trace buffer.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *
 *	EINVAL	Indicates an invalid parameter.
 *	EFAULT	Indicates that an invalid address was specified.
 *	EIO	Indicates an error.  Status field contains
 *		detailed error information.
 */  

int
fddi_get_trace(	
	fddi_acs_t		*p_acs,		/* ACS ptr */
	fddi_open_t		*p_open,	/* Open element ptr */
	fddi_get_trace_t	*p_arg,		/* Memory access struct */
	dev_t			devflag)	/* devflag */
{
	int			curmode;	/* current mode */
	uint			curstate;	/* current state */
	fddi_get_trace_t	gt;		/* local get trace struct */
	fddi_sif_t		regs;		/* SIF registers */
	fddi_icr_cmd_t		icrs;		/* ICR command struct */
	ushort			*p_sram;	/* ptr to the SHARED RAM */
	int 			rc = 0;		/* return code */
	int			i;
	uint			iocc;
	fddi_mem_acc_t		mem_acc;	/* for shared memory */

	FDDI_DBTRACE("DgtB", p_acs->dev.state, p_acs->ctl.mode, p_arg);

	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	/*
	 * copy in the ioctl memory access structure.
	 */
	
	if( rc = MOVEIN(devflag, p_arg, &gt, sizeof(fddi_get_trace_t) ))
	{
		FDDI_DBTRACE("Dgt1", p_acs->ctl.mode, EFAULT, 0);
		return(EFAULT);
	}

	if(MOVEOUT(devflag, p_acs, gt.p_acs, sizeof(fddi_acs_t) ))
		rc = EFAULT;

	if(MOVEOUT(devflag, &fdditrace, gt.p_ddtrace, sizeof(fddi_trace_t) ))
		rc = EFAULT;

	/* 
	 * get the adapter's POS registers
	 */
	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	for (i=0;i<8;++i)
		PIO_GETCX(iocc +i, &gt.pos[i]);
	
	IOCC_DET(iocc);
	/*
	 * Get registers and shared ram
	 */
	(void) fddi_get_registers(p_acs, &regs, &icrs);

	/*
	 * save current state and mode then change so we can do diag stuff
	 */
	curstate = p_acs->dev.state;
	curmode = p_acs->ctl.mode;
	(void) fddi_chg_state (p_acs, FDDI_DWNLD, 'D');

	/*
	 * get the AMD registers, SKYLINE registers,
	 * and the DATA Store area off the adapter.
	 */
	rc = fddi_mem_acc(p_acs, p_open, gt.p_amd, devflag);
	if (rc != 0)
		gt.status = rc;
	rc = fddi_mem_acc(p_acs, p_open, gt.p_sky, devflag);
	if (rc != 0)
		gt.status = rc;
	rc = fddi_mem_acc(p_acs, p_open, gt.p_dstore, devflag);
	if (rc != 0)
		gt.status = rc;

	bzero (&mem_acc, sizeof(mem_acc));
	mem_acc.opcode = FDDI_RD_SHARED_RAM;
	mem_acc.ram_offset = 0;
	mem_acc.num_transfer = 1;
	mem_acc.buff_len1 = gt.l_sram;
	mem_acc.buffer_1 = gt.p_sram;
	fddi_mem_acc(p_acs, p_open, &mem_acc, devflag);

	rc = 0;
	if(MOVEOUT(devflag, &regs, gt.p_regs, sizeof(fddi_sif_t) ))
		rc = EFAULT;

	if(MOVEOUT(devflag, &icrs, gt.p_icrs, sizeof(fddi_icr_cmd_t) ))
		rc = EFAULT;

	if(MOVEOUT(devflag, mem_acc.buffer_1, gt.p_sram, gt.l_sram ))
		rc = EFAULT;

	if(MOVEOUT(devflag, &gt, p_arg, sizeof(fddi_get_trace_t) ))
		rc = EFAULT;

	/*
	 * change back to the 'current' state 
	 */
	(void) fddi_chg_state (p_acs, curstate, curmode);
	p_acs->dev.carryover = FALSE;
	
	FDDI_DBTRACE("DgtE", rc, gt.status, 0);
	return(rc);

} /* end fddi_get_trace() */

