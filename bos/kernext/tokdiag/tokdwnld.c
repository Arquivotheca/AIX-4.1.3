static char sccsid[] = "@(#)75	1.25  src/bos/kernext/tokdiag/tokdwnld.c, diagddtok, bos411, 9428A410j 10/26/93 14:02:56";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  tokdnld(), dl_ldr_imag(), dl_init_mc()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>

#include <sys/tokuser.h>
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

/*
#define TOKDEBUG_0	(1)
*/


/*---------------------------  T O K D N L D  -------------------------------*/
/*									     */
/*  Downloads microcode to the Token Ring adapter.			     */
/*									     */
/*---------------------------------------------------------------------------*/

tokdnld (dds_t		*p_dds,
	 caddr_t	arg,
	 unsigned long	devflag)
{
	tok_download_t	*p_dnld_cmd;
	tok_download_t	downdata;
	char		*ldrbuf, *mcbuf;	/* bufs	for images */
	int		l_ldr_image, l_mc_image;/* image lengths */
	int		rc;			/* return codes	*/
	int		sil;			/* saved interrupt level */
	int		buf_length;

	DEBUGTRACE3 ("dnlB",(ulong)p_dds,(ulong)arg);

    /*  note: commented out
	if ((p_dds->cio.mode != 'D') && (p_dds->cio.mode != 'W'))
	{
		TRACE2("FooT", (ulong)DOWN_IOCTL_0);
		downdata.status = TOK_NOT_DIAG_MODE;
		if (copyout(&downdata, arg, sizeof(downdata)) != 0)
			return(EFAULT);
		return(ENOMSG);
	}
    */

	/*
	 *
	 * Check to see if Adapter is already activated
	 * If	so, return error.
	 */
	if ( p_dds->cio.device_state !=	DEVICE_NOT_CONN)
	{
		TRACE3("FooT", (ulong)DOWN_IOCTL_1,
			(ulong)p_dds->cio.device_state);
		return(ENOMSG);
	}


	/*
	 *   FUTURE FIX:
	 *       Add microcode version cross check to that
	 *       what is specified in VPD.
	 *
	 *       Add check to the devflag parameter,
	 *	 if from a kernel process, use bcopy
	 *	 otherwise use copyin to get microcode image
	 *
	 */

	/*
	 * Change the adapter state to show that
	 * we have microcode download in progress.
	 */
	p_dds->wrk.adap_state =	DOWNLOADING;

	p_dnld_cmd = (tok_download_t *)arg;

	/*							     */
	/*  First we must get the microcode loader image into kernel */
	/*  memory.  We	do this	by:				     */
	/*	1) allocating an area for the actual microcode	     */
	/*	   loader image	from pinned kernel memory.	     */
	/*	2) cross-memory	copying	the image into kernel memory */
	/*							     */
	ldrbuf = xmalloc(p_dnld_cmd->l_mcload, (uint)2,	pinned_heap);
	l_ldr_image = p_dnld_cmd->l_mcload;
	(void) copyin( p_dnld_cmd->p_mcload, ldrbuf, p_dnld_cmd->l_mcload);

	/*							     */
	/*  Get	the microcode image into kernel	memory;	this is	done */
	/*  by:							     */
	/*	1) allocating an area for the actual microcode	     */
	/*	   image from pinned kernel memory.		     */
	/*	2) cross-memory	copying	the image into kernel memory */
	/*	3) flushing the	cache where the	microcode image	     */
	/*	   exists in memory an possible	cache.		     */
	/*							     */
	l_mc_image = p_dnld_cmd->l_mcode;
	l_mc_image = ((l_mc_image + 79)	/ 80) *	80; /* round up	to 80 */
	buf_length = d_roundup(l_mc_image); /* round up	to cache line */
	mcbuf =	xmalloc(buf_length, PGSHIFT, pinned_heap);
	(void) copyin( p_dnld_cmd->p_mcode, mcbuf, p_dnld_cmd->l_mcode);
	(void) vm_cflush (mcbuf, l_mc_image);

	/*							     */
	/*  Now	attempt	to download the	microcode loader image	     */
	/*  to the adapter memory.  				     */
	/*							     */
	rc = dl_ldr_imag(p_dds, (short *)ldrbuf, l_ldr_image);
	if (!rc)
	{		      /* now try microcode */
	    rc = dl_init_mc(p_dds, (short *)ldrbuf, l_ldr_image,
			mcbuf, l_mc_image);
	    if (rc)
	    {
		TRACE3("FooT", (ulong)DOWN_IOCTL_2, (ulong)rc);
		logerr(p_dds, ERRID_TOK_DOWNLOAD);
	    }
	}
	else
	{
		TRACE3("FooT", (ulong)DOWN_IOCTL_3, (ulong)rc);
		logerr(p_dds, ERRID_TOK_DOWNLOAD);
	}

	(void) xmfree(ldrbuf, pinned_heap);
	(void) xmfree(mcbuf, pinned_heap);

	/*
	 * Change the adapter state to show that
	 * we have completed microcode download.
	 */
	if (!rc)
		p_dds->wrk.adap_state =	CLOSED_STATE;

	DEBUGTRACE3 ("dnlE",(ulong)p_dds,(ulong)arg);
	return(rc);
}

/*
 * dl_ldr_imag - download microcode loader image to adapter
 */
int dl_ldr_imag(p_dds, p_buf, len)
dds_t	*p_dds;
short	*p_buf;		/* pointer to loader image in kernel memory */
int	len;		/* Length of loader image */
{
/* local scope variable declaration	*/

    int		i;		/* loop	control	*/
    int		tmp;		/* value read from delay reg (ignored)	*/
    int		rc=1;		/* return code */
    int		retrys=0;	/* number of retrys */
    int		pio_attachment, delay_seg;
    unsigned short	tmp_status;	/* for reads from SIF registers	*/
    unsigned char	pos4, pos5;

    /* begin download loader image logic */
    
    pio_attachment = attach_bus( p_dds );

    while /* download not successfully performed and attempts are less */
	  /* than the maximum number of retries		       */
	( rc && (retrys < MAX_RETRYS))
    {
	/* first we write the loader begin command to the SIF Addr Reg */
	pio_write( p_dds, ADDRESS_REG, DL_LOADER_BEGIN );

	/* next	we issue a hardware reset to the adapter by writing    */
	/* to the reset	adapter	SIF register followed by writing to    */
	/* the enable adapter SIF register			       */

	/*
	 *   NOTE:
	 *	   If the adapter interrupts are not currently
	 *	   disabled, we	need to	disable	them.  This is
	 *	   to compensate for the spurious interrupt
	 *	   generated by	the Token-Ring adapter during
	 *	   the reset sequence.
	 */

	if (!p_dds->wrk.mask_int)
		pio_write( p_dds, IMASK_DISABLE, 0x00 );
	detach_bus(p_dds, pio_attachment);

	/*
 	*   Get the current POS 4 Setting.
 	*   Turn Off Parity.
 	*/
	pio_attachment = attach_iocc(p_dds);
	pos4 = pio_read(p_dds, POS_REG_4);
	pos4 = pos4 & ~(MC_PARITY_ON);
	pio_write( p_dds, POS_REG_4, pos4);

	/*
 	*   Get the current POS 5 Setting and disable DMA arbitration
	*   There is a timing window if the adapter is doing DMA when the
	*   reset occurs which could cause a bus timeout.
 	*/
	pos5 = pio_read(p_dds, POS_REG_5);
	pos5 = pos5 | MC_ARBITRATION;
	pio_write( p_dds, POS_REG_5, pos5);
	detach_iocc(p_dds, pio_attachment);

	/*
	 * ensure that there is no DMA going on (wouldn't expect any)
	 * wait 100 usec (somewhat arbitrary number based upon reset time)
	 */
	delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	i = 0;
	while ( ++i < 100) 
		tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
	IOCC_DET(delay_seg);

	pio_attachment = attach_bus( p_dds );
	pio_write( p_dds, RESET_REG, 0x00 );	    /* reset adapter */

	/*
	 * there is no register that can be checked to see if the reset is
	 * complete, so wait 100 usec according to the spec (actually going
	 * to wait 150+ usec)
	 */
	delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	i = 0;
	while ( ++i < 150) 
		tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
	IOCC_DET(delay_seg);

	pio_write( p_dds, ENABLE_REG, 0x00 );

	/*
	 *   NOTE:
	 *	   Since we reset the adapter,
	 *	   we can now re-enable	the adapter interrupts.
	 *	   The state in	which the Token-Ring adapter is
	 *	   known to generate spurious interrupts has
	 *	   now passed.
	 */
	pio_write( p_dds, IMASK_ENABLE, 0x00	);
	p_dds->wrk.mask_int = FALSE;

	detach_bus(p_dds, pio_attachment);
	/*
	 *   Get the current POS 4 Setting.
	 *   Turn On Parity.
	 */
	pio_attachment = attach_iocc(p_dds);
	pos4 = pio_read(p_dds, POS_REG_4);
	pos4 = pos4 | MC_PARITY_ON;
	pio_write( p_dds, POS_REG_4, pos4);

	/*
 	*   Get the current POS 5 Setting.
 	*   Allow DMA arbitration
 	*/
	pos5 = pio_read(p_dds, POS_REG_5);
	pos5 = pos5 & ~(MC_ARBITRATION);
	pio_write( p_dds, POS_REG_5, pos5);
	detach_iocc(p_dds, pio_attachment);

	pio_attachment = attach_bus( p_dds );

	/* Write the first command to the loader program */
	pio_write( p_dds, COMMAND_REG, DL_LDR_CMD1 );

	p_dds->wrk.footprint = DL_LDR_CMD1;

	/*
	 * read status register	until high order byte =	0
	 * this should occur within 20 microseconds according to the spec
	 */
	i = 0;
	delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	tmp_status = pio_read( p_dds, STATUS_REG );
	DEBUGTRACE2("ldr1", tmp_status);

	while ( ((tmp_status & 0xff00) != 0) && (++i < 50))
	{
		tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
		tmp_status = pio_read( p_dds, STATUS_REG );
		DEBUGTRACE2("ldr1", tmp_status);
	}
	IOCC_DET(delay_seg);

	if (tmp_status == 0) /* if zero status, continue download */
	{
		/* write zero to the SIF address register */
		pio_write( p_dds, ADDRESS_REG, 0x0000 );

		p_dds->wrk.footprint= DL_LOADER_IMAGE;

		/* load that image until end of image */
		while (len > 0) /* more of loader image to load */
		{
			pio_write( p_dds, AUTOINCR_REG,	*p_buf );
			len -= 2;
			/* increment pointer into LOADER IMAGE by 2 bytes */
			++p_buf;
	    	}

		/* Execute loader prog*/
		pio_write( p_dds, COMMAND_REG, DL_EXE_LDR_PRG );
	
		/*
		 * wait for results from ldr execution to be 0x0010 (good)
		 * this should occur within one second according to spec
		 */
		i = 0;
		tmp_status = pio_read( p_dds, STATUS_REG );
		DEBUGTRACE2("ldr3", tmp_status);
		while ( (tmp_status != 0x0010) && (++i < 200))
		{
			delay(HZ/100);
			tmp_status = pio_read( p_dds, STATUS_REG );
			DEBUGTRACE2("ldr3", tmp_status);
		}

		/* status is not successful */
		if (tmp_status != 0x0010 )
			TRACE4("FooT", (ulong)DOWN_LDR_IMAGE_0,	
				(ulong)tmp_status, (ulong)retrys);
		else
			rc = 0;
	}
	retrys++;
    }

    detach_bus( p_dds, pio_attachment );
    return(rc);
}	/* end function dl_ldr_imag */



/*
 * dl_init_mc - download initialization parameters and microcode to adapter
 */
int dl_init_mc ( p_dds, p_ldr_buf, l_ldr, p_mc_buf, l_mc )
dds_t		*p_dds;		/* pointer to dds for current adapter */
short		*p_ldr_buf;	/* pointer to loader image in kernel memory */
int		l_ldr;		/* Length of loader image */
unsigned char	*p_mc_buf;	/* ptr to microcode image in kernel memory */
int		l_mc;		/* Length of microcode image */
{
					/* template for	adapter	PIO regs */
    t_dwnld_ccb		*p_ccb;		/* ptr to command control block	 */
    unsigned short	*p_cb_addr;	/* ptr to dma bus memory addr	 */
    struct xmem		xmd_cb;		/* ctrl blk cross memory descriptor */
    struct xmem		xmd_mc;		/* ucode cross memory descriptor */
    int			rc;		/* generic return code	*/
    int			retrys=0;	/* number of init parms	retrys	*/
    unsigned short	temp[5];	/* for temporary cmd block jiggling */
    int			tmp_dma_addr;	/* temporary dma address variable */
    int			pio_attachment, delay_seg, align, buf_length;
    int			i;		/* loop	control	*/
    int			tmp;		/* value from delay reg (ignored) */
    unsigned short	tmp_status;	/* for reads from SIF registers	*/

    unsigned int	done=FALSE, fatal_err=FALSE;

    /* set pointer to Systems Interface register set on token ring adapter */
    pio_attachment = attach_bus( p_dds );

    /* now we allocate the area for the command control block	*/
    align = d_align();		/* align on a processor cache line boundary */
    buf_length = d_roundup(sizeof(t_dwnld_ccb)); /* allocate full cache line*/
    p_ccb = xmalloc(buf_length, align, pinned_heap);

    /* set up stuff for loader initialization parameters...this is    */
    /* a bit of magic as follows.  We use the base of our bus memory  */
    /* dma addresses and one TCW for the command control block	      */
    /* Since the initialization parameters must include that address, */
    /* burst size, options and a check sum, all written byte reversed */
    /* into the PIO address register, we just figure it all out up    */
    /* front and stick it into a temporary array of unsigned shorts   */
    /* that way we can write byte reversed into PIO and in case of    */
    /* retries we will not have to calculate it again.		      */

    tmp_dma_addr = (uint)p_dds->wrk.dl_tcw_base;
    tmp_dma_addr += (uint)p_ccb & 0xFFF; /* + p_ccb offset from page boundary*/

    /* view tmp_dma_addr as two	shorts */
    p_cb_addr =	(unsigned short	*) (&tmp_dma_addr);

    temp[0] = *p_cb_addr;	/* First half Bus Master address */
    p_cb_addr++;
    temp[1] = *p_cb_addr;	/* Second half Bus Master address */
    temp[2] = 80;		/* segment length */
    temp[3] = 0x8000;		/* options */
    temp[4] = -(temp[0]+temp[1]+temp[2]+temp[3]);	/* checksum */

    /* set up values for dma control block pointer		      */
    /* allocate and place initial values in cross memory descriptors  */
    /*
     *   FUTURE FIX:
     *	   Add error logic for when xmalloc fails to
     *	   allocate the	needed memory.
     */

    /* set up the command block to request a write program segment */

    tmp_dma_addr = (unsigned int)p_dds->wrk.dl_tcw_base;
    tmp_dma_addr += 2*PAGESIZE;

    /* view tmp_dma_addr as two shorts */

    p_cb_addr =	(unsigned short	*) (&tmp_dma_addr);
    p_ccb->segment[0] =	*p_cb_addr;	/* set up first	half of	image */
					/* segment location */
    p_cb_addr++;
    p_ccb->segment[1] =	*p_cb_addr;	/* set up second half of image */
					/* segment location */
    p_ccb->recs	= (l_mc	+ 79) /	80;	/* set number of 80 byte records */


    /* now we issue d_master calls for both the command block and the	     */
    /* microcode image...later we'll kick all this stuff off and it will     */
    /* all magically do its thing */

    xmd_cb.aspace_id = XMEM_INVAL;
    xmd_mc.aspace_id = XMEM_INVAL;

    rc = xmattach( p_ccb, sizeof(t_dwnld_ccb), &xmd_cb, SYS_ADSPACE);
    if (rc != XMEM_FAIL)
    {   /* good attach */
	rc = xmattach( p_mc_buf, l_mc, &xmd_mc, SYS_ADSPACE);
	if (rc != XMEM_FAIL)
	{   /* good attach */
	    d_master(p_dds->wrk.dma_chnl_id, DMA_WRITE_ONLY, p_mc_buf,
		     l_mc, &xmd_mc, (char *) tmp_dma_addr );
	}
	else
	{
	    (void) xmdetach(&xmd_cb);
	    xmfree(p_ccb, pinned_heap);
	    TRACE3("FooT", (ulong)DOWN_INIT_MC_0, (ulong)rc);
	    logerr(p_dds, ERRID_TOK_ERR15);
	    return(EFAULT);
	}
    }
    else
    {
	xmfree(p_ccb, pinned_heap);
	TRACE3("FooT", (ulong)DOWN_INIT_MC_1, (ulong)rc);
	logerr(p_dds, ERRID_TOK_ERR15);
	return(EFAULT);
    }

    /*
     *  initialization not successfully performed and attempts are less
     * than the maximum number of retries
     */
    while (!done && !fatal_err && (retrys < MAX_RETRYS) )
    {
	retrys++;

	/* now what we do is write the initialization parameters */
	/* generated into the temp array into the PIO registers	 */
	/* on the adapter.  First we write the address into the	 */
	/* addr	SIF reg	and then the five half words into the	 */
	/* auto	increment register... Finally, write 0x2000 to	 */
	/* the SIF command register to process those babies	 */

	p_ccb->cmd = 0x0001;		       /* command */

	/* at this point we should flush the cache lines...	*/
	(void) vm_cflush (p_ccb, sizeof(t_dwnld_ccb));

	d_master(p_dds->wrk.dma_chnl_id, DMA_READ + DMA_NOHIDE, p_ccb, 8,
		 &xmd_cb, p_dds->wrk.dl_tcw_base);

	pio_write( p_dds, ADDRESS_REG,  DL_INIT_ADDR );
	pio_write( p_dds, AUTOINCR_REG,	temp[0]	);
	pio_write( p_dds, AUTOINCR_REG,	temp[1]	);
	pio_write( p_dds, AUTOINCR_REG,	temp[2]	);
	pio_write( p_dds, AUTOINCR_REG,	temp[3]	);
	pio_write( p_dds, AUTOINCR_REG,	temp[4]	);
	pio_write( p_dds, COMMAND_REG,  DL_PROC_INIT );

	/*
	 * wait for results from ldr initialization to be 0x0020 (good)
	 */
	i = 0;
	delay_seg = (uint)IOCC_ATT(p_dds->ddi.bus_id, DL_DELAY_REG ); 
	tmp_status = pio_read( p_dds, STATUS_REG );
	DEBUGTRACE2("ldr4", tmp_status);
	while ( (tmp_status != 0x0020) && (++i < 1000))
	{
	    tmp = BUSIO_GETC(delay_seg); /* delay 1 microsecond */
	    tmp_status = pio_read( p_dds, STATUS_REG );
	    DEBUGTRACE2("ldr4", tmp_status);
	}   /* end while	*/
	IOCC_DET(delay_seg);

	if (tmp_status != 0x0020) /* initialization parms were not processed */
	{
	    TRACE3("FooT", (ulong)DOWN_INIT_MC_2, (ulong)tmp_status);
	    rc = dl_ldr_imag ( p_dds, p_ldr_buf, l_ldr);

	    if (rc) /* downloading the loader image failed */
	    {
		TRACE3("FooT", (ulong)DOWN_INIT_MC_3, (ulong)rc);
		fatal_err = TRUE;
	    }

	    /*
	     *   FUTURE FIX:
	     *       Add error logic to handle a bad return code from
	     *       the following d_complete() call
	     */
	    (void) d_complete(p_dds->wrk.dma_chnl_id, MICRO_CHANNEL_DMA,
			    p_ccb, 8, &xmd_cb, p_dds->wrk.dl_tcw_base);

	}   /* end if init. parameters were not processed */
	else
	{
	    /*
	     * previous ldr download and initialization parameter processing
	     * were successful, we will continue to do microcode download
	     */

	    /* now we can finally download the microcode...this part is easy */
	    pio_write( p_dds, COMMAND_REG, 0x2000 );

	    /*
	     * wait for results from ucode download to be 0x0030 (good)
	     */
	    i = 0;
	    tmp_status = pio_read( p_dds, STATUS_REG );
	    DEBUGTRACE2("ldr5", tmp_status);
	    while ( (tmp_status != 0x0030) && (++i < 100))
	    {
		delay(HZ/100);
		tmp_status = pio_read( p_dds, STATUS_REG );
		DEBUGTRACE2("ldr5", tmp_status);
	    }   /* end while	*/

	    if ( tmp_status == 0x0030)
	    {   /* read the command control block */
		(void) d_complete(p_dds->wrk.dma_chnl_id, MICRO_CHANNEL_DMA,
				p_ccb, 8, &xmd_cb, p_dds->wrk.dl_tcw_base);

		tmp_status = p_ccb->cmd;
		/* return code is good */
		if ( tmp_status == 0x8001 )
		{
		    p_ccb->cmd = 0x0002; /* set command to check ucode load */

		    /* at this point we should flush the cache lines... */
		    (void) vm_cflush(p_ccb, sizeof(t_dwnld_ccb));

		    d_master(p_dds->wrk.dma_chnl_id, DMA_READ + DMA_NOHIDE,
			     p_ccb, 8, &xmd_cb, p_dds->wrk.dl_tcw_base);

		    pio_write( p_dds, COMMAND_REG, 0x2000 );

		    /*
		     * wait for results from ucode check to be 0x0030 (good)
		     */
		    i = 0;
		    tmp_status = pio_read( p_dds, STATUS_REG );
		    DEBUGTRACE2("ldr6", tmp_status);
		    while ( (tmp_status != 0x0030) && (++i < 100))
		    {
			delay(HZ/100);
			tmp_status = pio_read( p_dds, STATUS_REG );
			DEBUGTRACE2("ldr6", tmp_status);
		    }   /* end while	*/

		    (void) d_complete(p_dds->wrk.dma_chnl_id,
				  MICRO_CHANNEL_DMA, p_ccb, 8, &xmd_cb,
				  p_dds->wrk.dl_tcw_base);

		    /* attempt to check microcode download succeeded */
		    if (tmp_status == 0x0030)
		    {
			/* microcode download was correct */
			if ((tmp_status = p_ccb->cmd) == 0x8002)
			    done = TRUE;	/* set exit while loop flag */
			else
			{
			    TRACE3("FooT", (ulong)DOWN_INIT_MC_B,
				   (ulong)tmp_status);
			    rc = dl_ldr_imag (p_dds, p_ldr_buf, l_ldr);
			
			    if (rc) /* loader image download failed */
			    {
				TRACE3("FooT", (ulong)DOWN_INIT_MC_5,
					(ulong)rc);
				fatal_err = TRUE;
			    }
			}
		    }
		    else
		    {      /* microcode download check failed */

			TRACE3("FooT", (ulong)DOWN_INIT_MC_4,
				(ulong)tmp_status);
			rc = dl_ldr_imag ( p_dds, p_ldr_buf, l_ldr);
			
			if (rc) /* downloading the loader image failed */
			{
			    TRACE3("FooT", (ulong)DOWN_INIT_MC_5, (ulong)rc);
			    fatal_err = TRUE;
			}
		    }      /* end microcode download check failed */

		}
		else
		{  /* microcode download failed */

		    TRACE3("FooT", (ulong)DOWN_INIT_MC_6, (ulong)tmp_status);
		    rc = dl_ldr_imag ( p_dds, p_ldr_buf, l_ldr);

		    if (rc) /* downloading the loader image failed */
		    {
			TRACE3("FooT", (ulong)DOWN_INIT_MC_7, (ulong)rc);
			fatal_err = TRUE;
		    }
		} /* end microcode download check failed */
	    } /* end if command block is available */
	    else 
	    {
		TRACE3("FooT", (ulong)DOWN_INIT_MC_8, (ulong)tmp_status);
		(void) d_complete(p_dds->wrk.dma_chnl_id, MICRO_CHANNEL_DMA,
				p_ccb, 8, &xmd_cb, p_dds->wrk.dl_tcw_base);

		rc = dl_ldr_imag ( p_dds, p_ldr_buf, l_ldr);

		if (rc) /* downloading the loader image failed */
		{
		    TRACE3("FooT", (ulong)DOWN_INIT_MC_9, (ulong)rc);
		    fatal_err = TRUE;
		}

	    }  /* end if command block not available */
	}  /* end if initialization parameters were taken successfully */


    } /* end of while loop */

    TRACE5("FooT", (ulong)p_dds, (ulong)DOWN_INIT_MC_A, (ulong)tmp_status, rc);

    (void) d_complete(p_dds->wrk.dma_chnl_id, DMA_WRITE_ONLY,
		      p_mc_buf, l_mc, &xmd_mc, (char *) tmp_dma_addr);
    (void) xmdetach(&xmd_cb);
    (void) xmdetach(&xmd_mc);
    xmfree(p_ccb, pinned_heap);
    detach_bus( p_dds, pio_attachment );
    if (done)
	return(0);
    else
	return(-1);
}  /* end function dl_init_mc()	*/
