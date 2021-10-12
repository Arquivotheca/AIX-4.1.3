static char sccsid[] = "@(#)11	1.8  src/bos/kernext/tok/trmon_ucode.c, sysxtok, bos411, 9428A410j 4/19/94 08:56:47";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: dl_init_mc
 *		dl_ldr_imag
 *		tokdnld
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tokpro.h"

/*
 *	tokdnld - downloads microcode to the Token-ring adapter
 *
 *	The process is:
 *	1.  Download the microcode loader program into the adapter memory
 *	    and start running the program.
 *	2.  Transfer initialization parameters to the microcode loader program.
 *	3.  Provide the program commands for the adapter to download and check
 *	    the RAM in the adapter.
 */

tokdnld (
	dds_t		*p_dds,
	ndd_config_t	*p_ndd_config)
{
	tok_download_t	*p_dnld_cmd;
	tok_download_t	downdata;
	char		*ldrbuf, *mcbuf;	/* bufs	for images */
	uchar		pos2;
	int		iocc;
	int		l_ldr_image, l_mc_image;/* image lengths */
	int		rc;			/* return codes	*/
	int		buf_length;

   	TRACE_DBG(MON_OTHER, "dnlB", (int)p_dds, 0, 0);

	p_dnld_cmd = (tok_download_t *)p_ndd_config->ucode;

	/*
	 *  First we must get the microcode loader image into kernel
	 *  memory.  We do this by:
	 *	1) allocating an area for the actual microcode
	 *	   loader image	from pinned kernel memory.
	 *	2) cross-memory	copying	the image into kernel memory
	 */
	if ( (ldrbuf = xmalloc( p_dnld_cmd->l_mcload, (uint)2, pinned_heap ))
	    == NULL) {
		logerr( p_dds, ERRID_CTOK_MEM_ERR, __LINE__, __FILE__);
		return (ENOMEM);
	}
	l_ldr_image = p_dnld_cmd->l_mcload;
	if ( (rc = copyin( p_dnld_cmd->p_mcload, ldrbuf,
			p_dnld_cmd->l_mcload ) ) ) {
		logerr( p_dds, ERRID_CTOK_DOWNLOAD, __LINE__, __FILE__);
		(void) xmfree(ldrbuf, pinned_heap);
		return (rc);
	}

	/*
	 *  Get the microcode image into kernel memory; this is done
	 *  by:
	 *	1) allocating an area for the actual microcode
	 *	   image from pinned kernel memory.
	 *	2) cross-memory	copying	the image into kernel memory
	 *	3) flushing the	cache where the	microcode image
	 *	   exists in memory and possible cache.
	 */
	l_mc_image = p_dnld_cmd->l_mcode;
	l_mc_image = ((l_mc_image + 79)	/ 80) *	80; /* round up	to 80 */
	buf_length = d_roundup( l_mc_image ); /* round up to cache line */
	if ( (mcbuf = xmalloc( buf_length, PGSHIFT, pinned_heap ))
	    == NULL) {
		(void) xmfree(ldrbuf, pinned_heap);
		logerr( p_dds, ERRID_CTOK_MEM_ERR, __LINE__, __FILE__);
		return (ENOMEM);
	}
	if ( (rc = copyin( p_dnld_cmd->p_mcode, mcbuf,
			p_dnld_cmd->l_mcode ) ) ) {
		logerr( p_dds, ERRID_CTOK_DOWNLOAD, __LINE__, __FILE__);
		(void) xmfree(ldrbuf, pinned_heap);
		(void) xmfree(mcbuf, pinned_heap);
		return (rc);
	}
	vm_cflush ( mcbuf, l_mc_image );

	/* 
	 * Enable the card. 
	 */
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	TOK_GETPOS( iocc + POS_REG_2, &pos2 );
	TOK_PUTPOS( iocc + POS_REG_2, (pos2 | (CARD_ENABLE) ) );
	IOCC_DET( iocc );

	if (WRK.pio_rc) {
	    (void) xmfree(ldrbuf, pinned_heap);
	    (void) xmfree(mcbuf, pinned_heap);

            iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	    TOK_GETPOS( iocc + POS_REG_2, &pos2 );
	    TOK_PUTPOS( iocc + POS_REG_2, (pos2 & (~CARD_ENABLE)) );
	    IOCC_DET( iocc );

	    return(WRK.pio_rc);
	}

	/*
	 *  Initialize the DMA channel via the d_init() kernel
	 *  service routine.  If the d_init() completes successfully
	 *  call the d_unmask() kernel service to enable the channel.
	 */
	WRK.dma_chnl_id =
		d_init( DDI.dma_arbit_lvl, MICRO_CHANNEL_DMA, DDI.bus_id );

	if (WRK.dma_chnl_id == DMA_FAIL) {
		rc = EIO;
	} else {
		d_unmask( WRK.dma_chnl_id );
	
		/*
		 *  Now attempt to download the microcode loader image
		 *  to the adapter memory.
		 */
		rc = dl_ldr_imag(p_dds, (short *)ldrbuf, l_ldr_image);
		if (!rc) {
			/*
			 * now download the microcode
			 */
			rc = dl_init_mc(p_dds, (short *)ldrbuf, l_ldr_image,
				mcbuf, l_mc_image);
			if (rc) {
   				TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds,
					(ulong)TOKDWNLD_0, (ulong)rc);
				logerr(p_dds, ERRID_CTOK_DOWNLOAD,
					__LINE__, __FILE__);
			}
		} else {
   			TRACE_BOTH(MON_OTHER, "FooT", (int)p_dds,
				(ulong)TOKDWNLD_1, (ulong)rc);
			logerr(p_dds, ERRID_CTOK_DOWNLOAD, __LINE__, __FILE__);
		}
	}
    
	(void) xmfree(ldrbuf, pinned_heap);
	(void) xmfree(mcbuf, pinned_heap);

	/*
	 *  Clear the dma channel
	 */
	if (WRK.dma_chnl_id != DMA_FAIL) {
		d_clear( WRK.dma_chnl_id );
	}

	/* 
	 * Disable the card. 
	 */
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	TOK_GETPOS( iocc + POS_REG_2, &pos2 );
	TOK_PUTPOS( iocc + POS_REG_2, (pos2 & (~CARD_ENABLE)) );
	IOCC_DET( iocc );
  
   	TRACE_DBG(MON_OTHER, "dnlE", (int)p_dds, rc, 0);
 
	return(rc);
}

/*
  dl_ldr_imag - download microcode loader image to adapter
 */
int
dl_ldr_imag( dds_t *p_dds,
	short	*p_buf,		/* pointer to loader image in kernel memory */
	int	len)		/* Length of loader image */
{
	int	i;		/* loop	control	*/
	int	tmp;		/* value read from delay reg (ignored) */
	int	rc=EIO;		/* return code */
	int	retrys=0;	/* number of retrys */
	int	ioa, delay_seg;
	ushort	tmp_status;	/* for reads from SIF registers	*/
	uchar	pos4, pos5;

	/*
	 * loop while the download has not worked and we haven't
	 * exhausted the maximum number of retries
	 */
	while( rc && (retrys < MAX_RETRYS))
	{
		/*
		 * The sequence of activities to download the microcode
		 * loader program is: 
		 * 1 - write the "download loader begin" cmd to the adapter
		 * 2 - issue a hardware reset to the adapter 
		 * 3 - write the "ready to download loader" cmd to the adapter
		 * 4 - PIO the loader program to the adapter 
		 * 5 - write the "start" command to the loader program
		 */

		/*
		 * 1 - write "download loader begin" cmd to the SIF Addr Reg
		 */

        	ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
		TOK_PUTSRX( ioa + ADDRESS_REG, DL_LOADER_BEGIN);
		BUSIO_DET( ioa );

		/*
	 	 * 2 - issue a hardware reset to the adapter
	 	 */
	
		hwreset( p_dds);
		if (WRK.pio_rc) {
			return(WRK.pio_rc);
		}
	
		/*
		 * 3 - write the "ready to download loader" cmd to the adapter
		 * - read status register until high order byte = 0 or 50
		 *   microseconds pass (should occur within 20 microseconds
		 *   according to the spec)
		 */
        	ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );

		TOK_PUTSRX( ioa + COMMAND_REG, DL_LDR_CMD1);
	
		WRK.footprint = DL_LDR_CMD1;
	
		i = 0;
		delay_seg = (uint)IOCC_ATT( DDI.bus_id, IOCC_DELAY ); 
		TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   		TRACE_BOTH(MON_OTHER, "ldr1", tmp_status, 0, 0);

		while ( ((tmp_status & 0xff00) != 0) && (++i < 50))
		{
			TOK_GETCX(delay_seg, &tmp); /* delay 1 microsecond */
			TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   			TRACE_BOTH(MON_OTHER, "ldr1", tmp_status, 0, 0);
		}
		IOCC_DET( delay_seg );

		if (WRK.pio_rc) {
			return(WRK.pio_rc);
		}
	
		if (tmp_status == 0) /* if zero status, continue download */
		{
			/*
			 * 4 - PIO the loader program to the adapter 
			 */
	    		WRK.footprint= DL_LOADER_IMAGE;

			TOK_PUTSRX( ioa + ADDRESS_REG, 0x0000);
	    		while (len > 0) /* more of loader image to load */
	    		{
				/* 2 bytes of loader are done at a time */
				/*
				 * if PIO errors are retried, the wrong
				 * data will get loaded and later checks
				 * will fail
				 */
				TOK_PUTSRX( ioa + AUTOINCR_REG, *p_buf );
				len -= 2;
				++p_buf;
	    		}
			/*
			 * 5 - write the "start" command to the loader program
			 * - read status register until it is 0x0010 or two
			 *   seconds pass (should occur within one second
			 *   according to the spec)
			 */
	    
			TOK_PUTSRX( ioa + COMMAND_REG, DL_EXE_LDR_PRG );
	    
			i = 0;
			TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   			TRACE_BOTH(MON_OTHER, "ldr2", tmp_status, 0, 0);
			while ( (tmp_status != 0x0010) && (++i < 200))
			{
				delay( HZ/100);
				TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   				TRACE_BOTH(MON_OTHER,"ldr2", tmp_status, 0, 0);
			}
	    
			/* status is not successful */
			if (tmp_status != 0x0010 ) {
   				TRACE_BOTH(MON_OTHER, "FooT", DOWN_LDR_IMAGE_0,
					tmp_status, retrys);
			} else {
				rc = 0;
			}
		}
		BUSIO_DET( ioa );

		if (WRK.pio_rc) {
			return(WRK.pio_rc);
		}

		retrys++;
	}
	return(rc);
}	/* end function dl_ldr_imag */



/*
 * dl_init_mc - download initialization parameters and microcode to adapter
 */
int
dl_init_mc ( dds_t	*p_dds,		/* ptr to dds for current adapter */
	short		*p_ldr_buf,	/* ptr to loader image in kernel */
	int		l_ldr,		/* Length of loader image */
	uchar		*p_mc_buf,	/* ptr to microcode image in kernel */
	int		l_mc)		/* Length of microcode image */
{
	t_dwnld_ccb	*p_ccb;		/* ptr to command control block	 */
	unsigned short	*p_cb_addr;	/* ptr to dma bus memory addr	 */
	struct xmem	xmd_cb;		/* ctrl blk cross memory descriptor */
	struct xmem	xmd_mc;		/* ucode cross memory descriptor */
	int		rc;		/* generic return code	*/
	int		retrys=0;	/* number of init parms	retrys	*/
	unsigned short	temp[5];	/* for temporary cmd block jiggling */
	int		tmp_dma_addr;	/* temporary dma address variable */
	int		ioa, delay_seg, align, buf_length;
	int		i;		/* loop	control	*/
	int		tmp;		/* value from delay reg (ignored) */
	u_short		tmp_status;	/* for reads from SIF registers	*/

	u_int		done=FALSE, fatal_err=FALSE;
	u_int		local_err;
    
	/*
	 * allocate the area for the command control block
	 * - aligned on a processor cache line boundary
	 * - the size of a cache line
	 */
	align = d_align();
	buf_length = d_roundup( sizeof(t_dwnld_ccb) );
	p_ccb = xmalloc( buf_length, align, pinned_heap );

	if (!p_ccb) {
		logerr( p_dds, ERRID_CTOK_MEM_ERR, __LINE__, __FILE__);
		return (ENOMEM);
	}

	/*
	 * set up stuff for loader initialization parameters...this is
	 * a bit of magic as follows.  We use the base of our bus memory
	 * dma addresses and one TCW for the command control block
	 * Since the initialization parameters must include that address,
	 * burst size, options and a check sum, all written byte reversed
	 * into the PIO address register, we just figure it all out up
	 * front and stick it into a temporary array of unsigned shorts
	 * that way we can write byte reversed into PIO and in case of
	 * retries we will not have to calculate it again.
	 */
    
	tmp_dma_addr = (uint)DDI.tcw_bus_mem_addr;
	tmp_dma_addr += (uint)p_ccb & 0xFFF; /* + p_ccb offset from page bdy*/
    
	/* view tmp_dma_addr as two	shorts */
	p_cb_addr =	(unsigned short	*) (&tmp_dma_addr);
    
	temp[0] = *p_cb_addr;	/* First half Bus Master address */
	p_cb_addr++;
	temp[1] = *p_cb_addr;	/* Second half Bus Master address */
	temp[2] = 80;		/* segment length */
	temp[3] = 0x8000;	/* options (check parity) */
	temp[4] = -(temp[0]+temp[1]+temp[2]+temp[3]);	/* checksum */
    
	/* set up values for dma control block pointer		      */
	/* allocate and place initial values in cross memory descriptors  */
    
	tmp_dma_addr = (unsigned int)DDI.tcw_bus_mem_addr;
	tmp_dma_addr += 2*PAGESIZE;
    
	/* view tmp_dma_addr as two shorts */
    
	p_cb_addr = (unsigned short *) (&tmp_dma_addr);
	p_ccb->segment[0] = *p_cb_addr;	/* set up first	half of	image */
					/* segment location */
	p_cb_addr++;
	p_ccb->segment[1] = *p_cb_addr;	/* set up second half of image */
					/* segment location */
	p_ccb->recs = (l_mc + 79) / 80;	/* set number of 80 byte records */
    
	/* now we issue d_master calls for both the command block and the */
	/* microcode image...later we'll kick all this stuff off and it will */
	/* all magically do its thing */
    
	xmd_cb.aspace_id = XMEM_INVAL;
	xmd_mc.aspace_id = XMEM_INVAL;
    
	rc = xmattach( p_ccb, sizeof(t_dwnld_ccb), &xmd_cb, SYS_ADSPACE);
	if (rc != XMEM_FAIL) {   /* good attach */
		rc = xmattach( p_mc_buf, l_mc, &xmd_mc, SYS_ADSPACE);
		if (rc != XMEM_FAIL) {   /* good attach */
			d_master(WRK.dma_chnl_id, DMA_WRITE_ONLY, p_mc_buf,
				l_mc, &xmd_mc, (char *) tmp_dma_addr );
		} else {
			(void) xmdetach(&xmd_cb);
			xmfree(p_ccb, pinned_heap);
   			TRACE_BOTH(MON_OTHER, "FooT", DOWN_INIT_MC_0, rc, 0);
			/* error logged in main routine */
			return(EIO);
		}
	} else {
		xmfree(p_ccb, pinned_heap);
   		TRACE_BOTH(MON_OTHER, "FooT", DOWN_INIT_MC_1, rc, 0);
		/* error logged in main routine */
		return(EIO);
	}

        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );

	/*
	 * Loop while the microcode was not successfully downloaded and the
	 * attempts are less than the maximim number of retries
	 */    
	while (!done && !fatal_err && (retrys < MAX_RETRYS) ) {
		local_err = FALSE;
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

		d_master(WRK.dma_chnl_id, DMA_READ | DMA_NOHIDE, p_ccb, 8,
			 &xmd_cb, DDI.tcw_bus_mem_addr);

		TOK_PUTSRX( ioa + ADDRESS_REG, DL_INIT_ADDR );
		TOK_PUTSRX( ioa + AUTOINCR_REG, temp[0] );
		TOK_PUTSRX( ioa + AUTOINCR_REG, temp[1] );
		TOK_PUTSRX( ioa + AUTOINCR_REG, temp[2] );
		TOK_PUTSRX( ioa + AUTOINCR_REG, temp[3] );
		TOK_PUTSRX( ioa + AUTOINCR_REG, temp[4] );
		TOK_PUTSRX( ioa + COMMAND_REG, DL_PROC_INIT );

		/*
		 * wait for results from ldr initialization to be 0x0020 (good)
		 */
		i = 0;
		delay_seg = (uint)IOCC_ATT( DDI.bus_id, IOCC_DELAY ); 
		TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   		TRACE_BOTH(MON_OTHER, "ldr3", tmp_status, 0, 0);
		while ( (tmp_status != 0x0020) && (++i < 1000)) {
			TOK_GETCX(delay_seg, &tmp); /* delay 1 microsecond */
			TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   			TRACE_BOTH(MON_OTHER, "ldr3", tmp_status, 0, 0);
		}
		IOCC_DET(delay_seg);

		if (tmp_status != 0x0020) { /* init parms were not processed */
			local_err = DOWN_INIT_MC_2;
		}

		if (!local_err) {
			/*
			 * now download the microcode itself
			 */
			TOK_PUTSRX( ioa + COMMAND_REG, 0x2000 );
	    
			/*
			 * wait for results from ucode download to be 0x0030
			 * (good)
			 */
			i = 0;
			TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   			TRACE_BOTH(MON_OTHER, "ldr4", tmp_status, 0, 0);
			while ( (tmp_status != 0x0030) && (++i < 100)) {
				delay(HZ/100);
				TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   				TRACE_BOTH(MON_OTHER,"ldr4", tmp_status, 0, 0);
			}   /* end while	*/

			/* 
			 * read the command control block
			 */
			if ( tmp_status != 0x0030) {
				local_err = DOWN_INIT_MC_3;
			}
		}

		rc = d_complete(WRK.dma_chnl_id, DMA_READ,
				p_ccb, 8, &xmd_cb, DDI.tcw_bus_mem_addr);
		if ( rc != DMA_SUCC ) {
			local_err = rc;;
			logerr( p_dds, ERRID_CTOK_DEVICE_ERR,
				__LINE__, __FILE__);
		}

		if (!local_err) {
			if ( p_ccb->cmd != 0x8001 ) {
				local_err = DOWN_INIT_MC_4;
			}
		}

		if (!local_err) {
			/*
			 * check microcode
			 */
			p_ccb->cmd = 0x0002;

			(void) vm_cflush(p_ccb, sizeof(t_dwnld_ccb));

			d_master(WRK.dma_chnl_id, DMA_READ | DMA_NOHIDE,
					p_ccb, 8, &xmd_cb,
					DDI.tcw_bus_mem_addr);

			TOK_PUTSRX( ioa + COMMAND_REG, 0x2000 );

			/*
			 * wait for results from ucode check
			 * to be 0x0030 (good)
			 */
			i = 0;
			TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   			TRACE_BOTH(MON_OTHER, "ldr5", tmp_status, 0, 0);
			while ( (tmp_status != 0x0030) && (++i < 100)) {
				delay(HZ/100);
				TOK_GETSRX( ioa + STATUS_REG, &tmp_status);
   				TRACE_BOTH(MON_OTHER,"ldr5", tmp_status, 0, 0);
			}   /* end while	*/
		    
			rc = d_complete(WRK.dma_chnl_id, DMA_READ,
					p_ccb, 8, &xmd_cb,
					DDI.tcw_bus_mem_addr);
			if ( rc != DMA_SUCC ) {
				local_err = rc;;
				logerr( p_dds, ERRID_CTOK_DEVICE_ERR,
					__LINE__, __FILE__);
			}
		    
			if (tmp_status == 0x0030) {
				/* microcode download was correct */
				if ((tmp_status = p_ccb->cmd) == 0x8002) {
					done = TRUE;
				} else {
					local_err = DOWN_INIT_MC_5;
				}
			}
		}
		
		
		if (local_err) {
   			TRACE_BOTH(MON_OTHER,"FooT", local_err, tmp_status, 0);
		}

		if (!done && (retrys < MAX_RETRYS) ) {
			if ( rc = dl_ldr_imag ( p_dds, p_ldr_buf, l_ldr) ) {
   			    TRACE_BOTH(MON_OTHER,"FooT", DOWN_INIT_MC_6, rc,0);
			    fatal_err = TRUE;
			}
		}

	} /* end of while loop */

   	TRACE_BOTH(MON_OTHER, "FooT", DOWN_INIT_MC_7, tmp_status, rc);
    
	rc = d_complete(WRK.dma_chnl_id, DMA_WRITE_ONLY,
		      p_mc_buf, l_mc, &xmd_mc, (char *) tmp_dma_addr);
	if ( rc != DMA_SUCC ) {
		logerr( p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__, __FILE__);
	}

	(void) xmdetach(&xmd_cb);
	(void) xmdetach(&xmd_mc);
	xmfree(p_ccb, pinned_heap);
	BUSIO_DET( ioa );

	if (WRK.pio_rc) {
		return(WRK.pio_rc);
	}

	if (done) {
		return(0);
	} else {
		return(EIO);
	}
}  /* end function dl_init_mc()	*/
