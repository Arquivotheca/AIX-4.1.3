static char sccsid[] = "@(#)80	1.2  src/bos/kernext/fddidiag/fddidiag_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:51:04";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: dnld_cmplt
 *		dnld_handler
 *		dnld_issue
 *		fddi_chg_state
 *		fddi_dnld_to
 *		fddi_do_dma
 *		fddi_download
 *		fddi_get_registers
 *		fddi_hcr_cmd_cmplt
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
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include "fddi_comio_errids.h"
#include <sys/sleep.h>
#include <sys/dma.h>

/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;

/*
 * NAME: fddi__download
 *                                                                    
 * FUNCTION: begins the process of downloading the microcode to the card.
 *                                                                    
 * EXECUTION ENVIRONMENT: called from USER process environment only
 *                                                                   
 * NOTES: fddi_download, after testing to make sure the call was set up 
 * correctly,  sets up the user memory copy of the buffer.  The set up consists
 * of pinning the buffer, creating a xmem descriptor and dmastering the data.
 * After this an enter_diagnostic command is issued through the hcr cmd 
 * structure and the download process is put to sleep.  
 *	When the command is completed, the slih will call the cmplt routine in 
 * the command structure.  In this case dnld_issue.  The dnld_issue command 
 * will set up the icr_command to download microcode in the shared memory.  
 * It will set the adapter into download/diagnostic mode using the DD bit in
 * pos reg 2.
 * 	When the card has completed the icr_cmd to download the microcode it 
 * will post the slih with DDC (or a DDA if there was a problem).  This will 
 * cause the slih to call dnld_handler.  dnld_handler will clear the DD bit from
 * pos reg 2 and then issue the test microcode test in the hcr cmd structure.
 *	When the next CCI comes in (the one for this test command) the results
 * of the test will be looked at and if all is well the exit diagnostic mode
 * will be issued.  
 * 	When this completes the dnld_cmplt will be called again and this time
 * it will wake up the original download process which will do the cleanup and
 * return the results to the user.  If at any point the download fails the 
 * result is returned to the user and the machine ends up back in the NULL 
 * state.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	The 'arg' parameter points to a ccc_download struct.
 *
 * RETURNS: EIO (and a result in the status field)
 *	    EINVAL for memory problems
 *	    EFAULT for failed memory transfers
 *
 */  

int
fddi_download(
	fddi_acs_t		*p_acs, 
	fddi_open_t		*p_open, 
	fddi_dwnld_t		*p_arg, 
	ulong			devflag)
{
	int			rc,saved_rc;
	fddi_dwnld_t		dnld;
	int			tx_cnt;
	int			i,j,len[3];
	int			addr_spc;
	int 			iocc;
	int 			ioa;
	int			bus;
	int			ipri;
	fddi_cmd_t 		*p_cmd;
	ushort			pgoffset;
	ushort			type_aspc;
	extern	void		dnld_issue();


	FDDI_TRACE("DdlB", p_acs->ctl.mode, p_open, p_arg);

	/*
	 * check to see if the user has opened the driver in diagnostic 
	 *  mode
	 */
	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	if ((p_acs->ctl.mode != 'D') && (p_acs->ctl.mode != 'C'))
	{
		dnld.status = FDDI_NOT_DIAG_MODE;
		FDDI_TRACE("Ddl1", dnld.status, EIO, 0);
		if (MOVEOUT(devflag, &dnld, p_arg,sizeof(fddi_dwnld_t)))
		{
			/* if the copyout failed, return */
			return (EFAULT);
		}
		return(EIO);
	}
		
	if ((p_acs->dev.state != FDDI_NULL) &&
		(p_acs->dev.state != FDDI_DWNLD))
	{
		dnld.status = FDDI_STARTED;
		FDDI_TRACE("Ddl2", p_acs->dev.state, 0, 0);
		if (MOVEOUT(devflag, &dnld, p_arg, sizeof(fddi_dwnld_t)))
		{
			/* if copyout failed, return */
			return (EFAULT);
		}
		return (EIO);
	}

	/*
	 * copy in the ioctl download structure.  This will allow the size
	 * to be checked and the xmem descriptors to be set up.
	 */

	if (rc = MOVEIN(devflag, p_arg, &dnld, sizeof(fddi_dwnld_t)))
	{
		return(EFAULT);
	}

	/* 
	 * The max available range for a microcode program is 128k (0x20000).
	 * Make sure the user has passed in a valid size and that the length is
	 * even.  It must be even as the adapter is word oriented as opposed
	 * to byte oriented.
	 */
	
	if ((dnld.l_mcode < 1) || (dnld.l_mcode > 0x20000) ||
		(dnld.l_mcode & 0x1))
	{
		FDDI_TRACE("Ddl3", dnld.l_mcode, EINVAL, 0);
		return(EINVAL);
	}

	/*
	 * If the call is from a kernel user then the addr_spc var needs to
	 *be set to the value of SYS_ADSPACE for the call to xmattach otherwise
	 * it needs to be set to USER_ADSPACE.
	 */
	if (p_open->devflag & DKERNEL)
	{
		addr_spc = SYS_ADSPACE;
		type_aspc = UIO_USERSPACE;
	}
	else 
	{
		addr_spc = USER_ADSPACE;
		type_aspc = UIO_SYSSPACE;
	}
	
	/*
	 * Pin the user's memory containing the microcode (dnld.p_mcode)
	 */
	if (pinu(dnld.p_mcode, dnld.l_mcode, type_aspc))
	{
		FDDI_TRACE("Ddl4", dnld.p_mcode, dnld.l_mcode, EFAULT);
		return(EFAULT);
	}

	p_acs->dev.dma_xmd.aspace_id = XMEM_INVAL;

	/* 
	 * attach to the microcode image.
	 */
	if (xmattach(dnld.p_mcode, dnld.l_mcode, &p_acs->dev.dma_xmd, addr_spc)
		 != XMEM_SUCC)
	{
		/*
		 * Trace and error log and exit.  The dma set up
		 * should stay as it is used elsewhere and the only
		 * clean up I know of right know is unpinning user
		 * memory and any attached memory previous to this point
		 */
		unpinu(dnld.p_mcode, dnld.l_mcode, type_aspc);
		FDDI_TRACE("Ddl5", dnld.p_mcode, dnld.l_mcode, EINVAL);
		return(EINVAL);
	}

	/*
	 * Flush the whole microcode image to get cache lines
	 * in sync.  Then we d_master the whole mcode image with the hide
	 * option to prevent cache inconsistency during the DMA of
	 * the mcode image.
	 */
		/* 
		 * !!! check the last parameter on the d_master()
		 */

	d_master(p_acs->dev.dma_channel, 
		DMA_READ, 
		dnld.p_mcode,
		dnld.l_mcode, 
		&p_acs->dev.dma_xmd,
		p_acs->dev.p_d_kbuf[0]);

	p_acs->dev.l_ubuf[0] = dnld.l_mcode;
	p_acs->dev.p_ubuf[0] = dnld.p_mcode;

	/*
	 * NB:
	 *	Here we calculate the offset into the page of the
	 *	microcode image.  We need to use this offset to align 
	 *	the DMA address that we give to the adapter for the
	 *	microcode image.  If we do not account for the
	 *	offset, the adapter will get an invalid
	 *	microcode image.
	 */

	pgoffset = (ushort)((uint)dnld.p_mcode & 0xFFF);

	/*
	 * We only need to put the page offset of the microcode
	 * image into the low address of the ICR cmd block addrs.
	 */
	p_acs->dev.icr_cmd.local_addr = 0;
	p_acs->dev.icr_cmd.len1 = SWAPSHORT(FDDI_MAX_TX_SZ);
	p_acs->dev.icr_cmd.hi_addr1 =SWAPSHORT(ADDR_HI(p_acs->dev.p_d_kbuf[0] +
					pgoffset));
	p_acs->dev.icr_cmd.lo_addr1 =SWAPSHORT(
					ADDR_LO(p_acs->dev.p_d_kbuf[0] 
							+ pgoffset));
	p_acs->dev.icr_cmd.len2 = SWAPSHORT(FDDI_MAX_TX_SZ);
	p_acs->dev.icr_cmd.hi_addr2 = SWAPSHORT(ADDR_HI(p_acs->dev.p_d_kbuf[0] +
					pgoffset + FDDI_MAX_TX_SZ));
	p_acs->dev.icr_cmd.lo_addr2 = SWAPSHORT(ADDR_LO(p_acs->dev.p_d_kbuf[0] +
					FDDI_MAX_TX_SZ + pgoffset));
	p_acs->dev.icr_cmd.cmd = 0x6400;
	if (dnld.l_mcode % FDDI_MAX_TX_SZ)
	{
		p_acs->dev.icr_cmd.cmd |= 0x0200;
		p_acs->dev.icr_cmd.len3 = SWAPSHORT(dnld.l_mcode - 
			(2*FDDI_MAX_TX_SZ));
		p_acs->dev.icr_cmd.hi_addr3 = SWAPSHORT(
			ADDR_HI(p_acs->dev.p_d_kbuf[0] + 2*FDDI_MAX_TX_SZ +
			pgoffset));
		p_acs->dev.icr_cmd.lo_addr3 = SWAPSHORT(
			ADDR_LO(p_acs->dev.p_d_kbuf[0] + pgoffset 
				+ 2*FDDI_MAX_TX_SZ));
	}
	else
	{
		p_acs->dev.icr_cmd.len3 = 0;
		p_acs->dev.icr_cmd.hi_addr3 = 0;
		p_acs->dev.icr_cmd.lo_addr3 = 0;
	}
	p_acs->dev.icr_cmd.cmd = SWAPSHORT(p_acs->dev.icr_cmd.cmd);

	ipri = i_disable(INTOFFL1);

	p_acs->dev.state = FDDI_DWNLDING;

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	PIO_PUTSRX(ioa + FDDI_HMR_REG, FDDI_HMR_DWNLD);

	BUSIO_DET(ioa);

	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (void(*)()) dnld_issue;
	p_cmd->cmd_code = FDDI_HCR_DIAG_MODE;
	p_cmd->cpb_len = 0;
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	fddi_issue_cmd(p_acs, p_cmd, bus);
	BUSMEM_DET(bus);

	/* 
	 * save the values for microchannel errors and to communicate back a 
	 * reason for failing 
	 */
	p_acs->dev.ioctl_status = CIO_OK;

	
	e_sleep(&p_acs->dev.ioctl_event, EVENT_SHORT);

	i_enable(ipri);
	rc = d_complete(p_acs->dev.dma_channel, 
			DMA_READ,
			dnld.p_mcode, 
			dnld.l_mcode, 
			&p_acs->dev.dma_xmd,
			p_acs->dev.p_d_kbuf[0]);

	unpinu(dnld.p_mcode, dnld.l_mcode, type_aspc);
			
	if ( rc	!= DMA_SUCC )
		saved_rc = EINVAL;
	else 
		saved_rc = 0;
	
	if (xmdetach(&p_acs->dev.dma_xmd) != XMEM_SUCC)
	{
		saved_rc = EINVAL;
	}

	if (p_acs->dev.ioctl_status != CIO_OK)
	{
		dnld.status = p_acs->dev.ioctl_status;
		if (MOVEOUT(devflag,&dnld,p_arg, sizeof(fddi_dwnld_t)))
		{
			return(EFAULT);
		}
		return(EIO);
	}
		

	if (p_acs->dev.state != FDDI_DWNLD)
	{
		if (p_acs->dev.state == FDDI_DWNLDING)
			p_acs->dev.state = FDDI_NULL;
		return(EFAULT);
	}

	if (saved_rc)
		return(saved_rc);

	dnld.status = CIO_OK;
	p_acs->dev.carryover = FALSE;

	if (MOVEOUT(devflag,&dnld,p_arg, sizeof(fddi_dwnld_t)))
		return(EFAULT);

	FDDI_TRACE("DdlE", 0, 0, 0);
	return(0);
}

/*
 * NAME: dnld_cmplt
 *                                                                    
 * FUNCTION: completes the command sequence for a download of microcode
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: dnld_cmplt is called by the interrupt handler (fddi_slih) to handle 
 * the results from a test of microcode (test 9).  dnld_cmplt will test the 
 * results and then issue an exit diagnostic's command.  It will be called 
 * again to complete the exit diag command and wakeup the original process with
 * the results.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: p_acs
 *
 * RETURNS: none
 */  


int 
dnld_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd, 
	int		bus)
{
	ushort test_res;

	FDDI_TRACE("DdcB", p_acs, p_cmd, p_cmd->stat);

	/* check for errors */
	if (p_cmd->stat != FDDI_HCR_SUCCESS)
	{
		/*
		 * command failed: wake up the caller 
		 * 	(invalid setcount handled in fddi_cmd_handler())
		 */
		fddi_logerr(p_acs, ERRID_FDDI_DWNLD,
			__LINE__, __FILE__);
		p_cmd->cmd_code = 0;
		p_acs->dev.state = FDDI_NULL;
		p_acs->dev.ioctl_status = CIO_HARD_FAIL;
		e_wakeup(&p_acs->dev.ioctl_event);

		/* err path return */
		return ;
	}
	
	if (p_cmd->cmd_code == FDDI_HCR_TEST9)
	{
		/*
		 * the TEST9 command was just issued
		 */
		PIO_GETSRX(bus+0x12,&test_res);

		if (test_res == 0x0000)
		{
			FDDI_TRACE("Ddc1", p_acs, p_cmd->cmd_code, test_res);
			
			p_acs->dev.state = FDDI_DWNLD;
			/*
			 * Setup for the START MICROCODE COMMAND
			 *	for fddi_cmd_handler to issue
			 */
			p_cmd->cmd_code = FDDI_HCR_START_MCODE;
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cpb_len = 0;

			return;
		}
		if (test_res == 0xFFFF)
		{
			/* 
			 * handle any clean up needed, the card had a 
			 *	fatal error while trying to run a test 
			 *	of the download code 
			 */
			fddi_logerr (p_acs, ERRID_FDDI_SELFT_ERR,
				__LINE__, __FILE__);
			p_acs->dev.state = FDDI_DEAD;
			FDDI_TRACE("Ddc2", p_acs,  p_cmd->cmd_code, test_res);
		}
		else 
		{
			fddi_logerr(p_acs, ERRID_FDDI_DWNLD,
				__LINE__, __FILE__);
			p_acs->dev.state = FDDI_NULL;

			FDDI_TRACE("Ddc3", p_acs,  p_cmd->cmd_code, test_res);
		}
	}

	/* 
	 * we are done
	 */
	p_cmd->cmd_code = 0;
	e_wakeup(&p_acs->dev.ioctl_event);

	/* the cmd block is set */
	FDDI_TRACE("DdcE", p_acs, p_acs->dev.state, 0);

	return ;
}

/*
 * NAME: dnld_handler
 *                                                                    
 * FUNCTION: called in result to a DDC (Download/Diagnostic Complete) bit set in
 * the interrupt handler.
 *                                                                    
 * EXECUTION ENVIRONMENT: called only on the interrupt thread
 *                                                                   
 * NOTES: called to continue the path of download microcode.  It removes the DD
 * bit from pos reg 2 (unmapping icr_cmd structure and the shared memory).  Then
 * issuing the test microcode self test (test 9).
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  
int
dnld_handler (
	fddi_acs_t 	*p_acs, 
	int		bus)
{
	fddi_cmd_t 	*p_cmd;
	int		iocc;
	extern int 	ddc_cmplt();
	uint	ioa;

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
	PIO_PUTSRX (ioa + FDDI_HMR_REG, 0x807f);
	BUSIO_DET (ioa);

	w_stop (&(p_acs->dev.dnld_wdt));
	FDDI_TRACE("DdhB", p_acs->dev.p_cmd_prog,
			p_acs->dev.state, 0);
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_PUTCX( iocc + FDDI_POS_REG2, (p_acs->dev.pos2 | FDDI_POS2_CEN) & 
					~FDDI_POS2_DD);
	IOCC_DET(iocc);
	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) dnld_cmplt;
	p_cmd->cmd_code = FDDI_HCR_TEST9;
	p_cmd->cpb_len = 0;
	fddi_issue_cmd(p_acs, p_cmd, bus);

	FDDI_TRACE("DdhE", p_cmd->stat, p_cmd->cmd_code, p_cmd);
	return;
}

/*
 * NAME: dnld_issue
 *                                                                    
 * FUNCTION: initial completion routine for the download microcode command
 * sequence.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES:  This is called as the completion of the enter diagnostic command that
 * started the download microcode command sequence.  The command will set the
 * DD bit in pos reg 2 mapping the shared memory to the icr command structure.
 * Then the structure is filled out with the address of the user's buffer saved
 * from the ioctl call.  The final transfer is that of the icr_cmd.  This kicks
 * off the download.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  


void
dnld_issue(
	fddi_acs_t *p_acs, 
	fddi_cmd_t	*p_cmd,
	int		bus)
{
	int 			iocc;
	int 			ioa;
	int			error;
	int			i;
	int			badrc;

	FDDI_TRACE("DdiB", p_acs, p_acs->dev.state, p_cmd);
	
	/* 
	 * get access to the the IOCC to access the pos registers.
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));

	/*
	 * Get Access to the I/O bus to access I/O registers
	 */
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/*
	 * Update POS reg. 2 to turn on the D/D bit (download/diagnostic
	 * MUST BE in D/D mode so that the ICR is accessed through the
	 * shared RAM - else we're modifying the TX/RX list stuff.
	 */

	PIO_PUTCX( iocc + FDDI_POS_REG2, p_acs->dev.pos2 | 
		FDDI_POS2_DD | FDDI_POS2_CEN);

	/*
	 *  initialize the possible interrupts
	 */
	PIO_PUTSRX(ioa + FDDI_HMR_REG, FDDI_HMR_DWNLD);

	w_start (&(p_acs->dev.dnld_wdt));
	/*
	 * issue the command
	 */
	PIO_PUTSTRX(bus,&p_acs->dev.icr_cmd, sizeof(struct fddi_icr_cmd));

	p_cmd->cmd_code = 0;

	BUSIO_DET(ioa);
	IOCC_DET(iocc);

	FDDI_TRACE("DdiE", p_acs, p_acs->dev.state, p_cmd);
	return;
}

/*
 * NAME: fddi_dnld_to
 *                                                                    
 * FUNCTION: handles a timeout during the download process
 *                                                                    
 * EXECUTION ENVIRONMENT: called from timer process
 *                                                                   
 * NOTES: returns the device to a non-downloaded state (NULL) and returns the
 * failure in the ioctl_status field
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	the 'p_wdt' points to the dnld_wdt structure inside the acs
 *
 * RETURNS:  void function
 *
 */  

void
fddi_dnld_to (
	struct	watchdog *p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;
	int		iocc;
	uint	ioa;

	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_dev_t, dnld_wdt) + 
		 (uint) offsetof (fddi_acs_t, dev)));

	ipri = i_disable ( INTCLASS2 );
	FDDI_TRACE("DdtB", p_wdt, 0, 0);
	p_acs->dev.oflv_events |= FDDI_DNLD_WDT_IO;
	if (p_acs->dev.oflv_running == FALSE)
	{
		/*
		 * schedule offlevel process and set running flag.
		 *	oflv_running is set to FALSE after the last
		 *	check of the hsr in fddi_oflv();
		 */
		i_sched (&p_acs->dev.ihs);
		p_acs->dev.oflv_running = TRUE;
	}

	FDDI_TRACE("DdtE", p_acs, 0, 0);
	i_enable (ipri);
	return;
}

/*
 * NAME: fddi_hcr_cmd_cmplt
 *                                                                    
 * FUNCTION: completes and interprets the results of a set addr command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  


int
fddi_hcr_cmd_cmplt(
	fddi_acs_t 	*p_acs, 
	fddi_cmd_t	*p_cmd,
	int		bus)
{

	FDDI_DBTRACE("DccB", p_acs, p_acs->dev.state, p_cmd);
	
	/* 
	 * Command specific action taken below
	 */
	p_cmd->cmd_code = 0;
	PIO_GETSTRX(bus + FDDI_CPB_SHARED_RAM, &p_acs->dev.hcr_cmd.cpb, 
				(FDDI_CPB_SIZE << 1));
	e_wakeup(&p_acs->dev.ioctl_event);
	FDDI_DBTRACE("DccE", p_acs, p_acs->dev.state, p_cmd);
	return;
}

/*
 * !!! this routine goes in a bottom have .c file
 *
 * NAME: fddi_do_dma()
 *                                                                    
 * FUNCTION: 
 *	This function kicks off the DMA for the FDDI_MEM_ACC ioctl
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread
 *                                                                   
 * NOTES: 
 *	This routine will set up the Instruction Command Registers
 *	for a DMA transfer(s).  POS register 2 is modified to put the
 *	adapter into Download/Diagnostic mode (this causes shared RAM
 *	to be re-mapped to the ICR regs.  This routine sleeps waiting for
 *	for the ICR command to complete or timeout.
 *
 *	POS register 2 is restored to it's original value upon the 
 *	completion of the ICR command.
 *
 * RECOVERY OPERATION: 
 *	This routine has no error recovery.
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
fddi_do_dma(	fddi_acs_t	*p_acs,		/* ACS ptr */
		ushort		icr,		/* icr command */
		uint		cnt,		/* # of transfer locations */
		uint		offset)		/* RAM offset */
{
	int 	iocc;		/* access to IOCC */
	int	bus;		/* access to bus memory */
	int	ioa;		/* access to bus I/O */
	int	i, ipri, rc=0;
	uchar	tmp;
	ushort	tmp2;
	ushort	hmr;		/* host mask register saved value */

	FDDI_DBTRACE("DddB", icr, cnt, offset);

	/* 
	 * attach to the bus for accessing the POS regs, shared ram,
	 * and the I/O regs.
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot<<16));
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	ipri = i_disable(INTOFFL1);

	/*
	 * Turn on the D/D bit (download/diagnostic) in POS 2.
	 * This causes the shared RAM to be remapped.  We do this
	 * so we can write out the ICR and not wipe out the TX and 
	 * RCV descriptors
	 *
	 * make sure that the adapter reset bit is NOT set and
	 * that the card is enabled.
	 */

	/*
	 * get the current value of POS register 2
	 */
	p_acs->dev.carryover = TRUE;
	PIO_GETCX(iocc + FDDI_POS_REG2, &tmp);
	PIO_PUTCX(iocc + FDDI_POS_REG2, ( (tmp | FDDI_POS2_DD | FDDI_POS2_CEN)
		&~(FDDI_POS2_AR) ) );

	for ( i=0; i < cnt; ++ i )
	{
		/* !!! what do 14 and 6 represent */
		PIO_PUTSRX(bus + (14-(i*6)), p_acs->dev.l_ubuf[i] );

		/*
		 * write the the high 2 bytes of the transfer 
		 * address to the ICR.
		 */
		tmp2 = ADDR_HI(p_acs->dev.p_d_kbuf[i]);
		/* !!! what dows 16 and 6 represent */
		PIO_PUTSRX( bus+(16-(i*6)), tmp2);


		/* 
		 * write the low 2 bytes of the transfer
		 * address to the ICR
		 */
		
		tmp2 = ADDR_LO(p_acs->dev.p_d_kbuf[i]);
		/* !!! what dows 18 and 6 represent */
		PIO_PUTSRX( bus+(18-(i*6)), tmp2);
	}

	/*
	 * write the RAM offset to the ICR local address
	 */
	PIO_PUTSRX( bus, offset );

	/*
	 * clear out the HSR.
	 * !!! is there some valid reason for doing
	 * this besides the ENG driver does it.
	 */
	PIO_GETSRX(ioa + FDDI_HSR_REG, &tmp2);

	/*
	 * get the current value of the Host Mask Register 
	 * save the value for restoration
	 */
	PIO_GETSRX( ioa + FDDI_HMR_REG, &hmr);

	/*
	 * enable interrupts from the adapter
	 * !!! determine why the ENG driver does a 0x007e
	 */
	PIO_PUTSRX( ioa + FDDI_HMR_REG, 0x007e );
	
	/*
	 * write the ICR command. This kicks the transfer(s) off.
	 */
	
	PIO_PUTSRX( bus + 20, icr );

	/* start watch dog timer */
	w_start(&p_acs->dev.dnld_wdt);
	 
	if ( e_sleep( &p_acs->dev.ioctl_event, EVENT_SIGRET) != EVENT_SUCC )
	{
		rc = EINTR;
	}
	else if ( p_acs->dev.ioctl_status != CIO_OK )
		rc = EIO;

	w_stop(&p_acs->dev.dnld_wdt);

	/* 
	 * restore the original Host Mask register
	 */
	PIO_PUTSRX( ioa + FDDI_HMR_REG, hmr );
	/*
	 * restore POS REG 2.  We turn off the adapter
	 * reset bit so as to not reset the adapter card.
	 */
	PIO_PUTCX(iocc + FDDI_POS_REG2, (tmp & ~(FDDI_POS2_AR)) );

	i_enable(ipri);

	BUSMEM_DET(bus);
	BUSIO_DET(ioa);
	IOCC_DET(iocc);
	FDDI_DBTRACE("DddE", rc, p_acs->dev.ioctl_status, 0);
	return(rc);

} /* end fddi_do_dma() */

/*
 * NAME: fddi_get_registers
 *                                                                    
 * FUNCTION: called to get an internal dump of some adapter objects
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment with interrupts disabled
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  0
 */  
int
fddi_get_registers ( 
	fddi_acs_t  	*p_acs,
	fddi_sif_t	*p_regs,
	fddi_icr_cmd_t	*p_icrs)
{
	uint		bus; 
	uint		iocc;
	uint		ioa;
	uchar		pos2;
	int		ipri;

	ipri = i_disable(INTCLASS2);
	/*
	 * Get the SHARED RAM and the SIF regesters from the adapter
	 * attach to the bus for accessing the POS regs, shared ram,
	 * and the I/O regs.
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot<<16));
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/* get the SIF regs */
	PIO_GETSRX( ioa + FDDI_HSR_REG, &p_regs->hsr);
	PIO_GETSRX( ioa + FDDI_HCR_REG, &p_regs->hcr);
	PIO_GETSRX( ioa + FDDI_NS1_REG, &p_regs->ns1);
	PIO_GETSRX( ioa + FDDI_NS2_REG, &p_regs->ns2);
	PIO_GETSRX( ioa + FDDI_HMR_REG, &p_regs->hmr);
	PIO_GETSRX( ioa + FDDI_NM1_REG, &p_regs->nm1);
	PIO_GETSRX( ioa + FDDI_NM2_REG, &p_regs->nm2);
	PIO_GETSRX( ioa + FDDI_ACL_REG, &p_regs->acl);

	/*
	 * we now flip the D/D bit in POS reg two to remap the
	 * adapters memory so we can get at ALISA's Instruction
	 * cmd regs.
	 * after we get the ICRs, we will map back to the
	 * original state
	 * NB:
	 *	We will be calling the fddi_mem_acc() routine
	 *	to get the adapter trace table.  In order for
	 *	it to work, we must trick the fddi_mem_acc() routine
	 *	into thinking that the device is opened in diagnostic
	 *	mode and that the device state is in the correct state.
	 */
	PIO_GETCX( iocc + FDDI_POS_REG2, &pos2);
	PIO_PUTCX( iocc + FDDI_POS_REG2, ((pos2 | FDDI_POS2_DD | FDDI_POS2_CEN)
						& ~(FDDI_POS2_AR)) );
	PIO_GETSTRX( bus, p_icrs, sizeof(fddi_icr_cmd_t) );
	PIO_PUTCX( iocc + FDDI_POS_REG2, ((pos2 | FDDI_POS2_CEN) 
						& ~(FDDI_POS2_AR)) );
	IOCC_DET(iocc);
	BUSIO_DET(ioa);
	BUSMEM_DET(bus);

	i_enable(ipri);

	return (0);
}
/*
 * NAME: fddi_chg_state
 *                                                                    
 * FUNCTION: called when getting an internal dump of some adapter objects
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment with interrupts disabled
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  0
 */  
int
fddi_chg_state (
	fddi_acs_t	*p_acs,
	int		state,
	char		mode)
{
	int		ipri;

	ipri = i_disable(INTOFFL1);
	p_acs->dev.state = state;
	p_acs->ctl.mode = mode;
	i_enable(ipri);

	return (0);
}



