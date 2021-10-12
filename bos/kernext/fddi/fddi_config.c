static char sccsid[] = "@(#)68	1.14  src/bos/kernext/fddi/fddi_config.c, sysxfddi, bos41J, 9519B_all 5/11/95 14:55:33";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: cfg_pos_regs
 *		disable_card
 *		dnld_handler
 *		dnld_issue
 *		fddi_cdt_add
 *		fddi_cdt_del
 *		fddi_cdt_func
 *		fddi_cdt_init
 *		fddi_cdt_undo_init
 *		fddi_config
 *		fddi_dnld_to
 *		fddi_gen_crc
 *		free_services
 *		get_fr_xcard_vpd
 *		get_sc_xcard_vpd
 *		get_vpd
 *		hcr_dnld_cmplt
 *		init_acs
 *		init_services
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

#include "fddiproto.h"
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/sleep.h>

/* 
 * Declare the device driver table structue.  This is the major global structure
 * for the fddi device driver.  It contains the internal trace table, the config
 * lock, and a table of acs structures.  
 * 
 */
fddi_tbl_t	fddi_tbl = 
{
	LOCK_AVAIL,	/* table_lock : locks config */
	0,		/* acs_cnt */
	0,		/* open_cnt */
	{
		0
	},
	0,
	{
		0,		/* reserved */
		0,		/* reserved */
		0,		/* reserved */
	        0,              /* trace.next */
        	0x46444449,     /* "FDDI" */
        	0x54524143,     /* "TRAC" */
        	0x4554424c,     /* "ETBL" */
        	0x21212121      /* "!!!!" */
	}
};

/*
 * FDDI component dump table objects:
 */

struct cdt	*p_fddi_cdt = NULL;
int		l_fddi_cdt = 0;

/*
 * NAME: fddi_config()
 *                                                                    
 * FUNCTION: Handles the CFG_INIT, CFG_TERM, CFG_QVPD, CFG_UCODE sysconfig cmds
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 	This routine executes on the process thread only.  It assumes that
 * 	interrupts have not been disabled.
 *                                                                   
 * NOTES:
 *	
 *
 * RETURNS: 
 *	CFG_INIT:
 *		0	- Successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *		EBUSY	- Device is already configured
 *		ENOMEM	- Unable to allocate required memory
 *
 *	CFG_TERM:
 *		0	- Successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *		EBUSY	- Device is open configured
 *		ENODEV	- Indicates that the device is not configured
 *
 *	CFG_QVPD:
 *		0	- successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *
 *	CFG_UCODE:
 *		0	- Successful
 *		EINVAL	- Indicates that an invalid parameter was passed
 *		EIO 	- Indicates a permanent I/O error
 *		ENODEV	- Indicates that the device is not configured
 *		ENETDOWN - Indicates that there was a pio failure.
 */  

int
fddi_config(int cmd,		
	    struct uio *p_uio)	
{
	fddi_acs_t	*p_acs;
	ndd_config_t 	ndd_cfg;
	fddi_dds_t	tmp_dds;
	int		badrc;
	int		i;
	int		iocc;
	int		rc;

	
	/*
	 * Lock to prevent other config calls from running.  This must be a 
	 * lockl because other locks need calls to lock alloc and lock free 
	 * and these calls would need to be locked. 
	 */
	rc = lockl(&fddi_tbl.table_lock, LOCK_SHORT);

	if ( rc != LOCK_SUCC )
                return(EINVAL);

	rc = pincode(fddi_config);
	if (rc != 0)
	{
		return(ENOMEM);
	}

	if ( fddi_tbl.acs_cnt == 0)
	{
                lock_alloc(&fddi_tbl.trace_lock, LOCK_ALLOC_PIN,
                        FDDI_TRACE_LOCK, -1);

                simple_lock_init(&fddi_tbl.trace_lock);
	}

	/* grab ACS Table lock */
	FDDI_TRACE("cfcB", cmd, fddi_tbl.acs_cnt, 0);


	uiomove (&ndd_cfg, (int)sizeof(ndd_config_t), UIO_WRITE, p_uio);

	/*
	 * At present there is a hard coded (FDDI_MAX_ACS) limit to the number
	 * adapters that can be configured at one time.  This is needed for 
	 * the table of acs structures.  The seq_number is guarenteed to be 
	 * the lowest non used number.
	 */
	if (ndd_cfg.seq_number >= FDDI_MAX_ACS)
	{
		FDDI_ETRACE("cfc1",ndd_cfg.seq_number, FDDI_MAX_ACS, 0);
		unpincode(fddi_config);
		unlockl(&fddi_tbl.table_lock);
		if ( fddi_tbl.acs_cnt == 0)
			lock_free(&fddi_tbl.trace_lock);
		return(EINVAL);
	}


	switch(cmd)
	{
		case CFG_INIT:
		{
			/* 
			 * Check to see if this number has already been 
			 * allocated.  This is probably unnecessary 
			 * as the config method should also be checking.
			 */
			if (fddi_tbl.p_acs[ndd_cfg.seq_number] != 0)
			{
				FDDI_ETRACE("cfi1",ndd_cfg.seq_number, 0, 0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
			 	return(EBUSY);
			}
			
			/* 
	 		 * get ACS memory from the pinned heap, page aligned
	 		 */
			p_acs = xmalloc(sizeof(fddi_acs_t), FDDI_WORDSHIFT, 
					pinned_heap );	
	
			if ( p_acs == NULL )
			{
				FDDI_ETRACE("cfi2",0,0,0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(ENOMEM);
			}
	
			bzero(p_acs, sizeof(fddi_acs_t) );

			if ((rc = copyin(ndd_cfg.dds, &p_acs->dds, 
				sizeof(fddi_dds_t))) != 0)
			{
				FDDI_ETRACE("cfi5",rc,0,0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree(p_acs, pinned_heap);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			/*
			 * Initialize all of the one time values of the acs.
			 */
			init_acs(p_acs);

    			if (p_acs->dds.intr_priority != CFDDI_OPLEVEL) 
			{
 				FDDI_ETRACE("cfi6",p_acs,
					p_acs->dds.intr_priority,
					CFDDI_OPLEVEL);

				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree(p_acs, pinned_heap);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
    			}

			/* 
			 * The adapter needs to be reset so that the adapter 
			 * is guaranteed to have run the self tests.
			 */
			iocc = IOCC_ATT(p_acs->dds.bus_id, 
				(IO_IOCC+(p_acs->dds.slot<<16)) );
			PIO_PUTPOS2((iocc+FDDI_POS_REG2), 
				(p_acs->dev.pos[2] | FDDI_POS2_AR ) );
			IOCC_DET(iocc);

			delay(10*HZ);
				
			/* 
	 		 * set in POS what was just given to us
	 		 * in the DDS.
	 		 */

			if ((rc = cfg_pos_regs(p_acs)) != 0)
			{
				FDDI_ETRACE("cfi7",rc,0,0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree( p_acs, pinned_heap );	
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

	
			if (get_stest(p_acs) == NDD_PIO_FAIL)
			{
				FDDI_ETRACE("cfi9",NDD_PIO_FAIL,0,0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree(p_acs, pinned_heap);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			badrc = 0;
			for (i=0;i<8;++i)
			{
				if (p_acs->dev.stest[i] !=0)
				{
					badrc = p_acs->dev.stest[i];
				}
			}

			if (p_acs->dev.stest[8] == 0x0000) 
			{
				p_acs->dev.attach_class = FDDI_ACLASS_DAS;
				p_acs->ndd.ndd_flags |= CFDDI_NDD_DAC;
			}
			else if (p_acs->dev.stest[8] == 0x80ea) 
			{
				p_acs->dev.attach_class = FDDI_ACLASS_SAS;
			}
			else
			{
				badrc = p_acs->dev.stest[8];
			}
	
			if ( (p_acs->dev.stest[9] != 0x0000) &&
		 		(p_acs->dev.stest[9] != 0x9001) )
			{
				badrc = p_acs->dev.stest[9];
			}

			if ((p_acs->dev.card_type == FDDI_SC) && 
				(p_acs->dev.stest[10] != 0x0000)  &&
				(p_acs->dev.stest[10] != 0xA0EA))
			{
				badrc = p_acs->dev.stest[10];
			} /* self tests failed */

			if (badrc != 0)
			{
				FDDI_ETRACE("cfia",badrc,0,0);
				fddi_logerr(p_acs, ERRID_CFDDI_SELFT_ERR,
					__LINE__,__FILE__, badrc, 0, 0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree( p_acs, pinned_heap );
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EIO);
			} /* self tests failed */

			get_vpd(p_acs);	/* get the VPD from adap */
		
			if (p_acs->vpd.status != FDDI_VPD_VALID)
			{
				FDDI_ETRACE("cfib",p_acs->vpd.status,
					FDDI_VPD_VALID,0);
				if ( fddi_tbl.acs_cnt == 0)
					lock_free(&fddi_tbl.trace_lock);
				xmfree( p_acs, pinned_heap );
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}


			if (p_acs->dev.stest[8] == 0x0000)
			{
				if (p_acs->dev.card_type == FDDI_FR)
				{
					get_fr_xcard_vpd(p_acs);
				}
				else
					if (p_acs->dev.card_type == FDDI_SC)
					{
						get_sc_xcard_vpd(p_acs);
					}
					else 
					{
						FDDI_ETRACE("cfic",
							p_acs->dev.card_type,
							0,0);
						if ( fddi_tbl.acs_cnt == 0)
							lock_free(&fddi_tbl.trace_lock);
						xmfree( p_acs, pinned_heap );
						unpincode(fddi_config);
						unlockl(
							&fddi_tbl.table_lock);
						return(EIO);
					}

				p_acs->vpd.xc_status = FDDI_VPD_VALID;
			}
			else
			{
				p_acs->vpd.xc_status = FDDI_VPD_NOT_READ;
			}

			/* 
	 		 * Set the source address up.  It should be the one
			 * from the vpd unless use of the alternate address is 
			 * configured.
	 		 */
			if (p_acs->dds.use_alt_addr)
			{
				bcopy (p_acs->dds.alt_addr, 
					p_acs->addrs.src_addr,
					CFDDI_NADR_LENGTH);
			}
			else
			{
				bcopy (p_acs->dev.vpd_addr, 
					p_acs->addrs.src_addr,
					CFDDI_NADR_LENGTH);
			}

			fddi_tbl.p_acs[ndd_cfg.seq_number] = p_acs;
			fddi_tbl.acs_cnt++;
			p_acs->dev.state = INIT_STATE;
		} /* end CFG_INIT */	
		break;

		case CFG_QVPD:
		{
			/* 
			 * The vpd was read from the adapter in CFG_INIT.  This
			 * call will just copy out what has already been read
			 * in.
			 */
			if ((p_acs = fddi_tbl.p_acs[ndd_cfg.seq_number]) == 0)
			{
				FDDI_ETRACE("cfq1",ndd_cfg.seq_number,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(ENODEV);
			}

			if (ndd_cfg.l_vpd < sizeof(fddi_vpd_t))
			{
				FDDI_ETRACE("cfq2",ndd_cfg.l_vpd,
					sizeof(fddi_vpd_t),0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(ENOMEM);
			}

			copyout(&p_acs->vpd, ndd_cfg.p_vpd, sizeof(fddi_vpd_t));
			uiomove (&ndd_cfg, (int)sizeof(ndd_config_t), 
				UIO_READ, p_uio);
			
		} /* end CFG_QVPD */
		break;

		case CFG_UCODE:
		{
			int		rc,saved_rc=0;
			fddi_dnld_t	dnld;
			int		addrs[3];
			int		tx_cnt;
			int		i,j,len[3];
			int		addr_spc;
			int 		iocc;
			int		bus;
			int		ipri;
			fddi_cmd_t 	p_cmd;
			ushort		pgoffset;
			ushort		type_aspc;
			extern	void	dnld_issue();

			/*
			 * Check to verify that the acs has been allocated.
			 */
			if ((p_acs = fddi_tbl.p_acs[ndd_cfg.seq_number]) == 0)
			{
				FDDI_ETRACE("cfu1",ndd_cfg.seq_number,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(ENODEV);
			}

			if ((rc = copyin(ndd_cfg.ucode,&dnld,
				sizeof(fddi_dnld_t))) != 0)	
			{
				FDDI_ETRACE("cfu2",rc,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			/*
			 * Check the length of the microcode.
			 */
			if ((dnld.l_mcode < 1) || (dnld.l_mcode > 0x20000) ||
				(dnld.l_mcode & 0x1))
			{
				FDDI_ETRACE("cfu3",dnld.l_mcode,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			if (p_acs->dev.state != INIT_STATE)
			{
				FDDI_ETRACE("cfu4",p_acs->dev.state,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			p_acs->dev.state = DNLDING_STATE;
	
			/*
			 * Allocate the system services the driver needs.
			 */
			if ((rc = init_services(p_acs)) != 0)
			{
				FDDI_ETRACE("cfu5",rc,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}
			
			addr_spc = USER_ADSPACE;
			type_aspc = UIO_SYSSPACE;
	
			/*
	 		 * Pin the user's memory containing the microcode 
			 * (dnld.p_mcode)
	 		 */
			if ((rc = pinu(dnld.p_mcode, dnld.l_mcode, type_aspc))
				!= 0)
			{
				FDDI_ETRACE("cfu6",rc,0,0);
				p_acs->dev.state = INIT_STATE;
				free_services(p_acs);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EIO);
			}

			p_acs->dev.dma_xmd.aspace_id = XMEM_INVAL;

			/* 
	 		 * attach to the microcode image.
	 		 */
			if ((rc = xmattach(dnld.p_mcode, dnld.l_mcode, 
				&p_acs->dev.dma_xmd, addr_spc)) != XMEM_SUCC)
			{
				FDDI_ETRACE("cfu7",rc,0,0);
				p_acs->dev.state = INIT_STATE;
				unpinu(dnld.p_mcode, dnld.l_mcode, type_aspc);
				free_services(p_acs);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EINVAL);
			}

			/*
	 		 * Dd_master the whole mcode image 
			 * with the hide option to prevent cache inconsistency 
			 * during the DMA of the mcode image.
	 		 */

			d_master(p_acs->dev.dma_channel, 
				DMA_READ, 
				dnld.p_mcode,
				dnld.l_mcode, 
				&p_acs->dev.dma_xmd,
				p_acs->dev.p_d_kbuf);

			/*
	 		 * Here we calculate the offset into the page of 
			 * the microcode image.  We need to use this offset
			 * to align the DMA address that we give to the 
			 * adapter for the microcode image.  If we do not 
			 * account for the offset, the adapter will get an 
			 * invalid microcode image.
	 		 */

			pgoffset = (ushort)((uint)dnld.p_mcode & 0xFFF);

			/*
	 		 * We only need to put the page offset of the microcode
	 		 * image into the low address of the ICR cmd block addrs
	 		 */
			addrs[0] = p_acs->dev.p_d_kbuf+pgoffset;
			addrs[1] = addrs[0]+FDDI_MAX_TX_SZ;
			addrs[2] = addrs[1]+FDDI_MAX_TX_SZ;

			p_acs->dev.icr.local_addr = 0;
			p_acs->dev.icr.len1 = SWAPSHORT(FDDI_MAX_TX_SZ);
			p_acs->dev.icr.hi_addr1 = 
				SWAPSHORT(ADDR_HI(addrs[0]));
			p_acs->dev.icr.lo_addr1 =
				SWAPSHORT(ADDR_LO(addrs[0]));

			p_acs->dev.icr.len2 = SWAPSHORT(FDDI_MAX_TX_SZ);
			p_acs->dev.icr.hi_addr2 = 
				SWAPSHORT(ADDR_HI(addrs[1]));
			p_acs->dev.icr.lo_addr2 = 
				SWAPSHORT(ADDR_LO(addrs[1]));
			p_acs->dev.icr.cmd = 0x6400;

			if (dnld.l_mcode % FDDI_MAX_TX_SZ)
			{
				p_acs->dev.icr.cmd |= 0x0200;
				p_acs->dev.icr.len3 = 
					SWAPSHORT(dnld.l_mcode - 
					(2*FDDI_MAX_TX_SZ));
				p_acs->dev.icr.hi_addr3 = 
					SWAPSHORT(ADDR_HI(addrs[2]));
				p_acs->dev.icr.lo_addr3 = 
					SWAPSHORT(ADDR_LO(addrs[2]));
			}
			else
			{
				p_acs->dev.icr.len3 = 0;
				p_acs->dev.icr.hi_addr3 = 0;
				p_acs->dev.icr.lo_addr3 = 0;
			}
			p_acs->dev.icr.cmd = 
				SWAPSHORT(p_acs->dev.icr.cmd);

			if (!(p_acs->dev.pio_rc))
			{
				/* 
				 * Start the download process.  The first step
				 * is to put the adapter into download/diag mode
				 * The dnld_issue routine will issue the icr
				 * cmd that has just been set up when the 
				 * enter diagnostic command has completed.
				 *
				 * When the whole download process has completed
				 * this process will be woken up and return from
				 * the sleep in send_cmd.
				 */
				p_cmd.stat = 0;
				p_cmd.pri = 0;
				p_cmd.cmplt = (int(*)()) dnld_issue;
				p_cmd.cmd_code = FDDI_HCR_DIAG_MODE;
				p_cmd.cpb_len = 0;

				ipri = disable_lock (CFDDI_OPLEVEL, 
					&p_acs->dev.cmd_lock);

				send_cmd(p_acs, &p_cmd);

				unlock_enable(ipri, &p_acs->dev.cmd_lock);

			}
			else
			{
				FDDI_ETRACE("cfu8",ENETDOWN,0,0);
				saved_rc = ENETDOWN;
			}

			/* 
	 		 * save the values of the last error while releasing
			 * services.  As they all need to be released any
			 * errors are saved and if one occured the 
			 * command is failed.
	 		 */

			rc = d_complete(p_acs->dev.dma_channel, 
					DMA_READ,
					dnld.p_mcode, 
					dnld.l_mcode, 
					&p_acs->dev.dma_xmd,
					p_acs->dev.p_d_kbuf);

			if ( rc	!= DMA_SUCC )
			{
				FDDI_ETRACE("cfu9",rc,0,0);
				saved_rc = EINVAL;
			}

			
			if ((rc = xmdetach(&p_acs->dev.dma_xmd)) != XMEM_SUCC)
			{
				FDDI_ETRACE("cfua",rc,0,0);
				saved_rc = EINVAL;
			}

			unpinu(dnld.p_mcode, dnld.l_mcode, type_aspc);

			if (p_acs->dev.cmd_status != 0)
			{
				FDDI_ETRACE("cfub",p_acs->dev.cmd_status,0,0);
				saved_rc = EIO;
			}

			free_services(p_acs);

			if (saved_rc != 0)
			{
				p_acs->dev.state = INIT_STATE;
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(saved_rc);
			}

			if ((rc = ns_attach(p_acs)) != 0)
			{
				FDDI_ETRACE("cfuc",rc,0,0);
				p_acs->dev.state = INIT_STATE;
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(EIO);
			}

			p_acs->dev.state = DNLD_STATE;
		}
		break;

		case CFG_TERM:
		{
			int 		iocc;

			if ((p_acs = fddi_tbl.p_acs[ndd_cfg.seq_number]) == 0)
			{
				FDDI_ETRACE("cft1",0,0,0);
				unpincode(fddi_config);
				unlockl(&fddi_tbl.table_lock);
				return(ENODEV);
			}

			/* 
			 * If we are still in the INIT_STATE we can close
			 * without the detach as the driver hasn't called
			 * ns_attach yet.
			 * Otherwise, the ns_detach must be called first to 
			 * prevent opens from coming down and check for
			 * any outstanding opens.
			 */
			if (p_acs->dev.state != INIT_STATE)
			{
				if ((rc = ns_detach (p_acs)) != 0)
				{
					FDDI_ETRACE("cft2",rc,0,0);
					unpincode(fddi_config);
					unlockl(&fddi_tbl.table_lock);
					return(EBUSY);
				}
			}

			xmfree( p_acs, pinned_heap );

			fddi_tbl.p_acs[ndd_cfg.seq_number] = 0;
			fddi_tbl.acs_cnt--;
		}  /* end case CFG_TERM */
		break;

		default:
			rc = EINVAL;

	} /* end switch (cmd) */

	FDDI_TRACE("cfcE", cmd, fddi_tbl.acs_cnt, 0);

	if ((cmd == CFG_TERM) && (fddi_tbl.acs_cnt == 0))
	{
		lock_free(&fddi_tbl.trace_lock);
	}

	/* unpin the driver code */
	unpincode(fddi_config);

	unlockl(&fddi_tbl.table_lock);

	return(rc);

} /* end fddi_config() */


/*
 * NAME: init_acs
 *                                                                    
 * FUNCTION: Initialize the ACS structure.  Services are allocated in 
 * 		init_services.  This routine is intended to just initialize 
 *		the acs the first time.  init_services will also initialize 
 *		those variables which need to be reset on close and such.
 *		init_tx, init_rx will take care of the receive and transmit
 *		parts of the acs themselves (they are only called in fddi_open).
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *		
 */  
void
init_acs(fddi_acs_t *p_acs)
{
	FDDI_TRACE("ciaB",p_acs, 0,0);

	p_acs->ndd.ndd_name = p_acs->dds.lname;
	p_acs->ndd.ndd_alias = p_acs->dds.alias;
	p_acs->ndd.ndd_flags = NDD_BROADCAST | NDD_SIMPLEX;
	p_acs->ndd.ndd_open = fddi_open;
	p_acs->ndd.ndd_close = fddi_close;
	p_acs->ndd.ndd_output = fddi_output;
	p_acs->ndd.ndd_ctl = fddi_ctl;
	p_acs->ndd.ndd_mtu = CFDDI_MAX_LLC_PACKET;
	p_acs->ndd.ndd_mintu = CFDDI_MIN_PACKET;
	p_acs->ndd.ndd_type = NDD_FDDI;
	p_acs->ndd.ndd_addrlen = CFDDI_NADR_LENGTH;
	p_acs->ndd.ndd_hdrlen = CFDDI_HDRLEN;
	p_acs->ndd.ndd_physaddr = p_acs->addrs.src_addr;
	p_acs->ndd.ndd_specstats = (caddr_t) &p_acs->ls;
	p_acs->ndd.ndd_speclen = sizeof (p_acs->ls);
	
	p_acs->dev.cmd_event = EVENT_NULL;
	p_acs->dev.cmd_wait_event = EVENT_NULL;

	p_acs->dev.smt_event_mask = FDDI_SMT_EVNT_MSK;
	p_acs->dev.smt_error_mask = FDDI_SMT_ERR_MSK;

	p_acs->dev.pio_rc = FALSE;

	p_acs->dev.state = NULL_STATE;

	p_acs->rx.l_adj_buf = d_roundup (CFDDI_MAX_LLC_PACKET);
	p_acs->tx.p_d_sf = p_acs->dds.dma_base_addr + 
			   (FDDI_MAX_RX_DESC * p_acs->rx.l_adj_buf);

	if (p_acs->tx.p_d_sf & (PAGESIZE - 1))
	{
		/* round up */
		p_acs->tx.p_d_sf &= (~(PAGESIZE - 1));
		p_acs->tx.p_d_sf += PAGESIZE;
	}
	p_acs->tx.p_d_base = p_acs->tx.p_d_sf + FDDI_SF_CACHE;
	p_acs->dev.p_d_kbuf = p_acs->tx.p_d_base;
	FDDI_TRACE("ciaE",p_acs, 0,0);
}


/*
 * NAME: init_services
 *                                                                    
 * FUNCTION: Initialize the system services and those variables in the acs which
 *		need to be re-initialized every open call.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *		0	- successful
 *		EIO 	- failed to initialize a service
 *		ENETDOWN - there was a pio failure
 *		
 */  

int
init_services(fddi_acs_t *p_acs)
{
	int 	rc = 0;
	int 	iocc;
	int 	ioa;
	ushort	hsr;

	FDDI_TRACE("cisB",p_acs, p_acs->dev.state,0);

	/* This needs to be initialized each open to clear any old errors */
	p_acs->dev.pio_rc = FALSE;

	/* 
	 * initialize DMA via d_init(), d_unmask() 
	 */
	p_acs->dev.dma_channel = d_init(p_acs->dds.dma_lvl, 
					MICRO_CHANNEL_DMA, 
					p_acs->dds.bus_id);

	if (p_acs->dev.dma_channel == DMA_FAIL)
	{
		FDDI_ETRACE("cis1",DMA_FAIL,0,0);
		return(EIO);
	}

	d_unmask(p_acs->dev.dma_channel);

	/* 
	 * register slih  
	 */
	p_acs->ihs.next = NULL;
	p_acs->ihs.handler = (int (*)())fddi_slih; 
	p_acs->ihs.bus_type = p_acs->dds.bus_type;
	p_acs->ihs.flags = INTR_MPSAFE;
	p_acs->ihs.level = p_acs->dds.bus_intr_lvl;
	p_acs->ihs.priority = CFDDI_OPLEVEL;
	p_acs->ihs.bid = p_acs->dds.bus_id;
	if (i_init ((struct intr *) (&p_acs->ihs)) != INTR_SUCC)
	{
		FDDI_ETRACE("cis2",p_acs,0,0);
		d_clear(p_acs->dev.dma_channel);
		return(EIO);
	}

	/* 
	 * initialize the DOWNLOAD watchdog timer 
	 */
	p_acs->dev.dnld_wdt.next = NULL;
	p_acs->dev.dnld_wdt.prev = NULL;
	p_acs->dev.dnld_wdt.restart = 0;
	p_acs->dev.dnld_wdt.count = 0;
	p_acs->dev.dnld_wdt.restart = FDDI_DNLD_WDT_RESTART;
	p_acs->dev.dnld_wdt.func = (void(*)())fddi_dnld_to; 
	while (w_init( &p_acs->dev.dnld_wdt));

	/* 
	 * initialize the COMMAND watchdog timer 
	 */
	p_acs->dev.cmd_wdt.next = NULL;
	p_acs->dev.cmd_wdt.prev = NULL;
	p_acs->dev.cmd_wdt.restart = FDDI_CMD_WDT_RESTART;
	p_acs->dev.cmd_wdt.count = 0;
	p_acs->dev.cmd_wdt.func = (void(*)())fddi_cmd_to;
	while (w_init( &p_acs->dev.cmd_wdt));

	/* 
	 * initialize the reset watchdog timer 
	 */

	p_acs->dev.limbo_wdt.next = NULL;
	p_acs->dev.limbo_wdt.prev = NULL;
	p_acs->dev.limbo_wdt.restart = FDDI_RESET_WDT_RESTART;
	p_acs->dev.limbo_wdt.count = 0;
	p_acs->dev.limbo_wdt.func = (void(*)())fddi_reset_to;
	while (w_init( &p_acs->dev.limbo_wdt));

	/*  
	 * enable the card 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_PUTPOS2(iocc+FDDI_POS_REG2, (p_acs->dev.pos[2] | FDDI_POS2_CEN));
	IOCC_DET(iocc);

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/*
	 * Read in the hsr to clear it of the misc. interrupts the 
	 * card generates when first enabled.
	 */
	PIO_GETSRX(ioa, &hsr);

	/*
	 * Unmask  interrupts for CCI (activation) and for ERRORS
	 * If we had an interrupt waiting it will happen inbetween
	 *	the TRACE calls.
	 */
	PIO_PUTSRX (ioa + FDDI_HMR_REG, FDDI_HMR_BASE_INTS);
	BUSIO_DET (ioa);

	p_acs->dev.smt_control = 0;

	p_acs->dev.stime = lbolt;

	/* initialize the counters in the statistics */
	p_acs->ls.mcast_tx_ok = 0;
	p_acs->ls.bcast_tx_ok = 0;
	p_acs->ls.mcast_rx_ok = 0;
	p_acs->ls.bcast_rx_ok = 0;

	bzero(&p_acs->ndd.ndd_genstats, sizeof (struct ndd_genstats));
	
	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("cis3",p_acs->dev.iox,0,0);
		i_clear( (struct intr *)(&p_acs->ihs) );
		while(w_clear( &p_acs->dev.dnld_wdt ));
		while(w_clear( &p_acs->dev.limbo_wdt));
		while(w_clear( &p_acs->dev.cmd_wdt));
		d_clear(p_acs->dev.dma_channel);
		disable_card(p_acs);
		p_acs->dev.pio_rc = FALSE;
		return(ENETDOWN);
	}

	lock_alloc(&p_acs->dev.cmd_lock, LOCK_ALLOC_PIN, 
		FDDI_CMD_LOCK, p_acs->dds.slot);
	simple_lock_init(&p_acs->dev.cmd_lock);

	lock_alloc(&p_acs->dev.slih_lock, LOCK_ALLOC_PIN, 
		FDDI_SLIH_LOCK, p_acs->dds.slot);
	simple_lock_init(&p_acs->dev.slih_lock);

	FDDI_TRACE("cisE",p_acs, p_acs->dev.state,0);
	return(0);
}

/*
 * NAME: free_services
 *                                                                    
 * FUNCTION: release the system resources allocated by the device driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		0	- successful
 *		
 */  
void
free_services(fddi_acs_t *p_acs)
{
	int	iocc;
	
	FDDI_TRACE("cfsB",p_acs,0,0);

	/* 
	 * Stop and de-allocate timers
	 */
	w_stop( &(p_acs->dev.dnld_wdt));
	w_stop( &p_acs->dev.cmd_wdt );
	w_stop( &p_acs->dev.limbo_wdt );

	while (w_clear( &p_acs->dev.dnld_wdt));
	while (w_clear( &p_acs->dev.cmd_wdt));
	while (w_clear( &p_acs->dev.limbo_wdt));

	/* un-initialize DMA via d_clear()  */
	d_clear(p_acs->dev.dma_channel);

	/* un register slih  */
	i_clear( (struct intr *)(&p_acs->ihs) );

	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_PUTPOS2((iocc+FDDI_POS_REG2), (p_acs->dev.pos[2] | FDDI_POS2_AR ) );
	IOCC_DET(iocc);

	delay(10*HZ);
			
	cfg_pos_regs(p_acs);

	lock_free(&p_acs->dev.cmd_lock);
	lock_free(&p_acs->dev.slih_lock);
	FDDI_TRACE("cfsE",p_acs,0,0);

}

/*
 * NAME: hcr_dnld_cmplt
 *                                                                    
 * FUNCTION: completes the command sequence for a download of microcode
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: 
 * This routine completes the hcr command sequence following the completion
 * of the microcode download (issued by dnld_issue).  It will issue the hcr
 * command to test the microcode, exit the diagnostic mode and then wake up 
 * the user with the results of the commands/test.
 *
 * RETURNS: other commands for the sequence.
 */  


void 
hcr_dnld_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd, 
	int		bus,
	int		ipri) 		/* not used */
{
	ushort test_res;

	FDDI_TRACE("cdcB",p_acs,p_cmd->cmd_code,p_cmd->stat);

	/* check for errors */
	if (p_cmd->stat != FDDI_HCR_SUCCESS)
	{
		/*
		 * command failed: wake up the caller 
		 * 	(invalid setcount handled in fddi_cmd_handler())
		 */
		FDDI_ETRACE("cdc1",p_cmd->cmd_code,p_cmd->stat,0);
		fddi_logerr(p_acs, ERRID_CFDDI_DWNLD,
			__LINE__, __FILE__, p_cmd->cmd_code, p_cmd->stat,0);

		p_cmd->cmd_code = 0;
		p_acs->dev.cmd_status = EIO;
		e_wakeup(&p_acs->dev.cmd_event);

		/* err path return */
		return ;
	}
	
	if (p_cmd->cmd_code == FDDI_HCR_TEST9)
	{
		/*
		 * the TEST9 command was just issued, get the results.
		 */
		PIO_GETSRX(bus+0x12,&test_res);

		if (p_acs->dev.pio_rc)
		{
			FDDI_ETRACE("cdc2",p_cmd->cmd_code,0,0);
			p_cmd->cmd_code = 0;
			p_acs->dev.cmd_status = EIO;
			e_wakeup(&p_acs->dev.cmd_event);

			/* err path return */
			return ;
		}

		if (test_res == 0x0000)
		{
			
			p_acs->dev.state = DNLD_STATE;
			/*
			 * Setup for the START MICROCODE COMMAND
			 *	for fddi_cmd_handler to issue
			 */
			p_cmd->cmd_code = FDDI_HCR_START_MCODE;
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cpb_len = 0;

			FDDI_TRACE("cdcD",p_acs,p_cmd->cmd_code,p_cmd->stat);
			return;
		}
		if (test_res == 0xFFFF)
		{
			/* 
			 * handle any clean up needed, the card had a 
			 *	fatal error while trying to run a test 
			 *	of the download code 
			 */
			FDDI_ETRACE("cdc3",test_res,p_cmd->cmd_code,0);
			fddi_logerr (p_acs, ERRID_CFDDI_SELFT_ERR,
				__LINE__, __FILE__, test_res,p_cmd->cmd_code,0);

			p_cmd->cmd_code = 0;
			p_acs->dev.cmd_status = EIO;
			e_wakeup(&p_acs->dev.cmd_event);

			/* err path return */
			return ;
		}
		else 
		{
			FDDI_ETRACE("cdc4",test_res,p_cmd->cmd_code,
				p_cmd->stat);

			fddi_logerr(p_acs, ERRID_CFDDI_DWNLD,
				__LINE__, __FILE__, test_res,p_cmd->cmd_code, 
				p_cmd->stat);

			p_cmd->cmd_code = 0;
			p_acs->dev.cmd_status = EIO;
			e_wakeup(&p_acs->dev.cmd_event);

			/* err path return */
			return ;
		}
	}

	/* 
	 * we are done
	 */
	p_cmd->cmd_code = 0;
	e_wakeup(&p_acs->dev.cmd_event);

	FDDI_TRACE("cdcE",p_acs,0,0);
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
 * NOTES: 
 * This routine is called when the download for the microcode is complete.
 * It removes the DD bit from pos reg 2 (unmapping icr structure and the 
 * shared memory).  Then issuing the test microcode self test (test 9).
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
	uint		ioa;

	FDDI_TRACE("cdhB",p_acs,0,0);
	w_stop (&(p_acs->dev.dnld_wdt));

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
	PIO_PUTSRX (ioa + FDDI_HMR_REG, FDDI_HMR_BASE_INTS);
	BUSIO_DET (ioa);

	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_PUTPOS2(iocc + FDDI_POS_REG2, (p_acs->dev.pos[2] | FDDI_POS2_CEN) & 
					~FDDI_POS2_DD);
	IOCC_DET(iocc);

	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) hcr_dnld_cmplt;
	p_cmd->cmd_code = FDDI_HCR_TEST9;
	p_cmd->cpb_len = 0;
	issue_cmd(p_acs, p_cmd, bus, FALSE);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("cdh1",p_acs,0,0);
		p_cmd->cmd_code = 0;
		p_acs->dev.cmd_status = EIO;
		e_wakeup(&p_acs->dev.cmd_event);

		/* err path return */
		return(0);
	}

	FDDI_TRACE("cdhE",p_acs,0,0);
	return(0);
}

/*
 * NAME: dnld_issue
 *                                                                    
 * FUNCTION: initial completion routine for the download microcode command
 * sequence.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES:  
 * This is called as the completion of the enter diagnostic command that
 * started the download microcode command sequence.  The command will set the
 * DD bit in pos reg 2 mapping the shared memory to the icr command structure.
 * Then the structure is filled out with the address of the user's buffer saved
 * from the ioctl call.  The final transfer is that of the icr.  This kicks
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

	FDDI_TRACE("cdhB",p_acs,p_cmd->cmd_code,p_cmd->stat);
	/* 
	 * get access to the the IOCC to access the pos registers.
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));

	/*
	 * Get Access to the I/O bus to access I/O registers
	 */
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/*
	 *  initialize the possible interrupts
	 */
	PIO_PUTSRX(ioa + FDDI_HMR_REG, FDDI_HMR_DWNLD);

	/*
	 * Update POS reg. 2 to turn on the D/D bit (download/diagnostic
	 * MUST BE in D/D mode so that the ICR is accessed through the
	 * shared RAM - else we're modifying the TX/RX list stuff.
	 */

	PIO_PUTPOS2( iocc + FDDI_POS_REG2, p_acs->dev.pos[2] | 
		FDDI_POS2_DD | FDDI_POS2_CEN);


	w_start (&(p_acs->dev.dnld_wdt));
	/*
	 * issue the command
	 */
	PIO_PUTSTRX(bus,&p_acs->dev.icr, sizeof(struct fddi_icr_cmd));

	p_cmd->cmd_code = 0;

	BUSIO_DET(ioa);
	IOCC_DET(iocc);

	if (p_acs->dev.pio_rc)
	{
		FDDI_TRACE("cdh1",p_acs,p_cmd->cmd_code,p_cmd->stat);
		p_acs->dev.cmd_status = EIO;
		e_wakeup(&p_acs->dev.cmd_event);
		w_stop(&(p_acs->dev.dnld_wdt));
		/* err path return */
		return ;
	}
	FDDI_TRACE("cdhE",p_acs,p_cmd->cmd_code,p_cmd->stat);
	return;
}

/*
 * NAME: fddi_dnld_to
 *                                                                    
 * FUNCTION: handles a timeout during the download process
 *                                                                    
 * EXECUTION ENVIRONMENT: called from timer process
 *                                                                   
 * NOTES: 
 * This routine is called when the icr command to download the microcode fails
 * to return.  It will fail the download and wake up the user sleeping on the
 * download.
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

	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_dev_t, dnld_wdt) + 
		 (uint) offsetof (fddi_acs_t, dev)));

	ipri = disable_lock ( CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
	FDDI_ETRACE("cdt ",p_acs,0,0);
	p_acs->dev.cmd_status = EIO;

	e_wakeup(&p_acs->dev.cmd_event);
	unlock_enable(ipri, &p_acs->dev.cmd_lock);
	return;
}


/*
 * NAME: cfg_pos_regs()
 *                                                                    
 * FUNCTION: Configure the POS registers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine decodes the configuration values in the DDS
 *	and sets the POS registers accordingly.
 *
 * RETURNS: 
 *		0	- successful
 *		EINVAL  - If the dds has an illegal value in it.
 *		
 */  

int 
cfg_pos_regs( fddi_acs_t *p_acs )
{
	int	acc;		/* for access to IOCC to modify POS regs */

	FDDI_TRACE("cprB",p_acs,0,0);
	/*
	 * POS Register 2
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |   Interrupt (INTR)    | DD  | not | AR  | CEN |
	 * |              Level    |     | used |    |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   INTR = Interrupt Level.  These bits contain the binary 
	 *	representation of the adapter card interrupt level.  
	 *
	 *   DD = Download/Diagnostic bit.  When set (1), this bit places the
	 *	adapter in to Download/Diagnostic state.  in this state, the 
	 *	ALISA chip is the master of the Node Processor bus and the 
	 *	Host has access to the ALISA Instruction Cmd registers.  The 
	 *	Node Processor is placed in a bus hold condition.
	 *
	 *   AR = Adapter Reset.   When this bit is written to a one (1), an
	 *	adapter reset pulse is will be generated.  All ALISA regs will
	 *	return to their default settings.  This bit will will always
	 *	read as a one (1).  A write of zero (0) has no effect.
	 *
	 *   CEN = Card Enable Bit.  When this bit is cleared (0), all ALISA
	 *	MC drivers will be tri-stated during normal MC cycles.  The
	 *	When programmed high (1), the adapter will support all MC
	 *	activity.  This bit will default to to a cleared (0) condition
	 *	after the occurance of a channel reset.
	 */


	p_acs->dev.pos[2] = 0;
	/*
	 * These are NOOPs but here to reflect intentions
	 */
	p_acs->dev.pos[2] &= ~(FDDI_POS2_CEN);	/* disable the card */
	p_acs->dev.pos[2] &= ~(FDDI_POS2_DD);	/* put adapter in normal op */
	p_acs->dev.pos[2] &= ~(FDDI_POS2_AR);	/* do not reset adapter */

	/*
	 * decode and set the interrupt level 
	 */
	switch (p_acs->dds.bus_intr_lvl)
	{
		case (FDDI_INTR_LVL_9):		
			p_acs->dev.pos[2] |=0xe0;	
			break;
		case (FDDI_INTR_LVL_10):
			p_acs->dev.pos[2] |=0xd0;	
			break;
		case (FDDI_INTR_LVL_14):
			p_acs->dev.pos[2] |=0xb0;	
			break;
		case (FDDI_INTR_LVL_15):
			p_acs->dev.pos[2] |=0x70;	
			break;
		default :
			FDDI_ETRACE("cpr1",p_acs,p_acs->dds.bus_intr_lvl,0);
			return(EINVAL);

	} /* end bus_intr_lvl switch */

	/*
	 * POS Register 3
	 *
	 *	Non-extension Mode
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | SDE | MSE | not |  F  | Arbitration  (ARB)    |
	 * |     |     | used|     |              Level    |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   SDE = Streaming Data Enable.  If set (1), then
	 *	the MC Streaming Data mode is disabled.  If reset (0),
	 *	then the Streaming Data mode is enabled using 10Mhz
	 *	(100nSec).  The default is set.
	 *
	 *   MSE = Streaming Data Enable.  If set (1), then 64bit
	 *	MC Streaming Data will not be attempted regardless of the 
	 *	MSDR indication.  If reset (0), then the 64 bit Streaming Data
	 *	mode is enabled.  The default value is set.
	 *
	 *   F = Fairness Bit.  If set (1), then the MC fairness feature
	 *	is enabled for Burst mode and Nibble mode transfers.  If 
	 *	reset (0), then fairness is not supported.  The default 
	 *	condition is set (1).
	 *
	 *   ARB = Arbitration Level.  These bits define the adapter's
	 *	MC arbitration level.
	 *
	 *
	 * 	Extension Mode
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |  d  |  d  |  d  |  d  |  d  |  d  |  d  |  d  |
	 * |     |     |     |     |     |     |     |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *	d = Data interface with the POS extension register or mem.
	 *
	 */
	p_acs->dev.pos[3] = 0;
	p_acs->dev.pos[3] &= ~(FDDI_POS3_SDE);
	p_acs->dev.pos[3] &= ~(FDDI_POS3_MSE);
	p_acs->dev.pos[3] |= FDDI_POS3_FAIR;
	p_acs->dev.pos[3] |= (p_acs->dds.dma_lvl & FDDI_POS3_ARB_MASK);

	/*
	 * POS Register 4
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | not | not | NSE | AIM |    ABM    |   P SEL   |
	 * | used| used|     |     |           |           |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   NSE = No Selected Feedback interrupt enable.  If set (1),
	 *	then ALISA will automatically set the NSF bit in SIF 
	 *	registers HSR and NS2 when a bus master operation has been
	 *	attempted with no SFDBK RTN or CHCK returned by the selected
	 *	slave.  If cleared (0), then the HSR and NS2 bits will not
	 *	be set by ALISA (default setting).
	 *
	 *   AIM = Auto Increment Mode for POS extensions.  If set (1),
	 *	then the index values in POS register 6 will be auto 
	 *	incremented after each POS extension access.  The default
	 *	is reset (0).
	 *
	 *   ABM = Address Burst Managment field.  This field will be used
	 *	to minimize the number of bus arbitration cycles.  Enabling 
	 *	ABM will cause the adapter to terminate a bus transfer when 
	 *	an address boundary crossing occurs.  ABM shall be controlled
	 *	by two bits in POS 4 as follows:
	 *
	 *		00 - ABM disabled (terminate transfer on bus
	 *			interface FIFO empty or preempt)
	 *		01 - 16 byte address boundary crossing
	 *		10 - 64 byte address boundary crossing
	 *		11 - 256 byte address boundary crossing
	 *
	 *   P SEL = Parity Select Field.  These bits enable and disable
	 *	the parity checking capability of ALISA.  The decode is
	 *	as follows:
	 *
	 *		00 - Parity is enabled on the MC data bus and the
	 *			MC address bus.
	 *		x1 - Parity is disabled for the MC data bus (default).
	 *		1x - Parity is disabled for the MC addr bus (default).
	 *
	 */

	p_acs->dev.pos[4] = 0;
	p_acs->dev.pos[4] |= FDDI_POS4_NSE;
	p_acs->dev.pos[4] |= FDDI_POS4_AIM;
	p_acs->dev.pos[4] |= FDDI_POS4_ABM;
	p_acs->dev.pos[4] |= FDDI_POS4_PSEL;

	/*
	 * POS Register 5
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | CHK | STA |       IAF       |       MAF       |
	 * |     |     | I/O adap addr   | Mem adap addr   |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   CHK = Channel check indicator.  When set (1), the Micro
	 *	channel check signal (CHCK) has not been driven by the
	 *	adapter.  When reset (0), the CHCK signal has been drivn by
	 *	the adapter and a MC address bus parity error 
	 *
	 *   STA = Status Field.  When set high (1), indicates that no 
	 *	status is reported for the encountered channel check 
	 *	exception condition.  When set low (0), this bit indicates 
	 *	that channel check status is reported via bytes XXX6 and XXX7.
	 *
	 *   IAF - I/O adapter address field.  This field will alter the 
	 *	MC I/O memory address for the ALISA system interface registers.
	 *	The default value is 0.  Possible I/O address are:
	 *
	 *		000 = 0x7140 
	 *		001 = 0x7150 
	 *		010 = 0x7540
	 *		011 = 0x7550
	 *		100 = 0x7940
	 *		101 = 0x7950
	 *		110 = 0x7d40
	 *		111 = 0x7d50
	 *		
	 *
	 *   MAF = Memory adapter address field.  This field will alter the
	 *	MC memory address for the the ALISA shared RAM buffer.
	 *	The default value is 0.  Possible RAM address are:
	 *
	 *		000 = 0x00100000
	 *		001 = 0x00300000
	 *		010 = 0x00500000
	 *		011 = 0x00700000
	 *		100 = 0x00900000
	 *		101 = 0x00b00000
	 *		110 = 0x00d00000
	 *		111 = 0x00f00000
	 *
	 */


	p_acs->dev.pos[5] = FDDI_POS5_CHK | FDDI_POS5_STAT;

	switch ( (int)p_acs->dds.bus_mem_addr )
	{
		case FDDI_POS5_MEM_1:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_000;
			break;
		case FDDI_POS5_MEM_3:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_001;
			break;
		case FDDI_POS5_MEM_5:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_010;
			break;
		case FDDI_POS5_MEM_7:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_011;
			break;
		case FDDI_POS5_MEM_9:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_100;
			break;
		case FDDI_POS5_MEM_b:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_101;
			break;
		case FDDI_POS5_MEM_d:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_110;
			break;
		case FDDI_POS5_MEM_f:
			p_acs->dev.pos[5] |= FDDI_POS5_MAF_111;
			break;
		default :
			FDDI_ETRACE("cpr2",p_acs,p_acs->dds.bus_mem_addr,0);
			return(EINVAL);
	} /* end switch on bus_mem_addr */

	switch ( (int)p_acs->dds.bus_io_addr )
	{
		case FDDI_POS5_IO_7140:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_000;
			break;
		case FDDI_POS5_IO_7150:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_001;
			break;
		case FDDI_POS5_IO_7540:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_010;
			break;
		case FDDI_POS5_IO_7550:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_011;
			break;
		case FDDI_POS5_IO_7940:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_100;
			break;
		case FDDI_POS5_IO_7950:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_101;
			break;
		case FDDI_POS5_IO_7d40:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_110;
			break;
		case FDDI_POS5_IO_7d50:
			p_acs->dev.pos[5] |= FDDI_POS5_IAF_111;
			break;
		default :
			FDDI_ETRACE("cpr3",p_acs,p_acs->dds.bus_io_addr,0);
			return(EINVAL);
	} /* end switch on bus_io_addr */

	/*
	 * POS Register 6
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |  a  |  a  |  a  |  a  |  a  |  a  |  a  |  a  |
	 * |     |     |     |     |     |     |     |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *    a = Address field.  The address field will be the index into
	 *	the VPD ROM, depending on the decode of POS register 7 
	 *	D SEL field.  The default value of POS register 7 is 0.
	 *
	 */

	/*
	 * save the POS register 6 configuration value
	 */
	p_acs->dev.pos[6] = 0;

	/*
	 * POS Register 7
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | MDP | not | not | not | VPS |     D SEL       |
	 * |     | used| used| used|     |                 |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   MDP = Micro Channel Data Bus Parity Error.  This bit is
	 *	set (1) when a parity error is detected on the MC data bus.
	 *	This bit can be cleared by a write to this register by the
	 *	Host processor.  The Node processor does not have write access.
	 *
	 *   VPS = Vital Products ROM Select. When this bit is set (1), then
	 *	the D SEL field can be used to select the VPD ROM on the
	 *	Class A extender card.  When this bit is cleared (0), then the
	 *	D SEL field can be used to select the VPD ROM on the base
	 *	adapter card.  A value of 000 in the D SEL field will select
	 *	a VPD ROM.
	 *
	 *   D SEL = Device Select bit.
	 *		000 - VPD ROM
	 *		001 - I/O device address field low (bits 1-8)
	 *		010 - I/O device address field high (bits 0)
	 *		011 - Memory device address field 1 (bits 9-16)
	 *		100 - Memory device address field 2 (bits 1-8)
	 *		101 - Memory device address field 3 (bits 0)
	 *		110 - reserved
	 *		111 - reserved
	 */

	/*
	 * save the POS register 7 configuration value
	 */
	p_acs->dev.pos[7] = 0;

	/*
	 * write out the POS register settings.
	 */
	acc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));
	PIO_PUTPOS( (acc + FDDI_POS_REG6), p_acs->dev.pos[6] );
	PIO_PUTPOS( (acc + FDDI_POS_REG7), p_acs->dev.pos[7] );
	PIO_PUTPOS( (acc + FDDI_POS_REG5), p_acs->dev.pos[5] );
	PIO_PUTPOS( (acc + FDDI_POS_REG4), p_acs->dev.pos[4] );
	PIO_PUTPOS( (acc + FDDI_POS_REG3), p_acs->dev.pos[3] );
	PIO_PUTPOS2( (acc + FDDI_POS_REG2), p_acs->dev.pos[2] );
	IOCC_DET(acc); 

	FDDI_TRACE("cprE",p_acs,0,0);
	return(0);

} /* end cfg_pos_regs */

/*
 * NAME: get_vpd()
 *                                                                    
 * FUNCTION: Gets the primary adapter's VPD
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine reads in the VPD for the primary adapter 
 *
 *	The VPD is crc checked. 
 *
 * RETURNS: 
 *	0		- successful
 *		
 */  

void
get_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	int 		vpd_found = FALSE;
	int 		na_found = FALSE;
	int 		crc_valid = FALSE;
	int 		fn_found = FALSE;
	ushort 		cal_crc;
	ushort 		vpd_crc;
	uchar		tmp1, tmp2;

	
	FDDI_TRACE("cgvB",p_acs,0,0);

	/*
	 * get access to the IOCC.  
	 */
	
	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	PIO_GETPOS( (iocc + FDDI_POS_REG0) ,&tmp1 );
	PIO_GETPOS( (iocc + FDDI_POS_REG1) ,&tmp2 );

	if ( (tmp1 != FDDI_POS_REG0_VAL) ||
		(tmp2 != FDDI_POS_REG1_VAL) )
	{
		/* 
		 * not a valid card 
		 * we don't read the VPD.
		 */
		p_acs->vpd.status=FDDI_VPD_NOT_READ;
	}
	else
	{
		p_vpd = &(p_acs->vpd);

		/* Initialize POS reg 6 and 7 */
		PIO_PUTPOS( (iocc + FDDI_POS_REG7), p_acs->dev.pos[7] );
		PIO_PUTPOS( (iocc + FDDI_POS_REG6), p_acs->dev.pos[6] );

		for ( i = 0; i < FDDI_VPD_LENGTH; ++i )
		{
			/*
			 * set up the correct addr for the VPD read byte 
			 */
			tmp1=(uchar)i+1;
			PIO_PUTPOS( (iocc + FDDI_POS_REG6), tmp1 );

			PIO_GETPOS( (iocc + FDDI_POS_REG3), &p_vpd->vpd[i] );
		} 

		p_vpd->vpd[FDDI_VPD_LENGTH - 1] = 0x00;

		/* Initialize POS reg 6 and 7 */
		PIO_PUTPOS( (iocc + FDDI_POS_REG7), p_acs->dev.pos[7] );
		PIO_PUTPOS( (iocc + FDDI_POS_REG6), p_acs->dev.pos[6] );

		/* 
		 * Test the VPD fields the driver uses.
		 */

		if ( (p_vpd->vpd[0] == 'V') &&
		     (p_vpd->vpd[1] == 'P') &&
		     (p_vpd->vpd[2] == 'D') &&
		     (p_vpd->vpd[7] == '*'))
		{
			vpd_found = TRUE;
			/* 
			 * calculate the VPD length
			 */
			p_vpd->l_vpd = ( (2 * ((p_vpd->vpd[3] << 8) |
						   p_vpd->vpd[4])) + 7);


			if (p_vpd->l_vpd > FDDI_VPD_LENGTH)
			{
				p_vpd->l_vpd = FDDI_VPD_LENGTH;
				/*
				 * mismatch on the length - can not test crc
				 * assume good crc
				 */
				crc_valid = TRUE;

			}
			else
			{
				vpd_crc = ( (p_vpd->vpd[5] << 8) |
						p_vpd->vpd[6]);

				/*
				 * verify that the crc is valid
				 */

				cal_crc = fddi_gen_crc( &(p_vpd->vpd[7]), 
						((ushort)p_vpd->l_vpd -7) );

				if ( vpd_crc == cal_crc )
					crc_valid = TRUE;
			} 

			/*
			 * get the network address 
			 */
			for ( i=0; i < p_vpd->l_vpd; ++ i)
			{
				/* test for the Network Address header */
				if ( (p_vpd->vpd[i+0] == '*' ) && 
				     (p_vpd->vpd[i+1] == 'N' ) &&
				     (p_vpd->vpd[i+2] == 'A' ) &&
				     (p_vpd->vpd[i+3] == 5) )
				{
					na_found = TRUE;
					/* save the net address */
					for (j=0; j < CFDDI_NADR_LENGTH; ++j)
						p_acs->dev.vpd_addr[j]=
							p_vpd->vpd[i+4+j];

					i += j-1;
				} /* end test for net addr */
				else 
				{
					if ((p_vpd->vpd[i] == 'F') &&
						(p_vpd->vpd[i+1] == 'N'))
					{
						if (!strncmp(&p_vpd->vpd[i+4],
							FDDI_FR_FRU1, 
							FDDI_FN_LEN))
						{
							p_acs->dev.card_type =	
								FDDI_FR;
							fn_found = TRUE;
						}
						else
						if (!strncmp(&p_vpd->vpd[i+4],
							FDDI_FR_FRU2, 
							FDDI_FN_LEN))
						{
							p_acs->dev.card_type =	
								FDDI_FR;
							fn_found = TRUE;
						}
						else 
						if ((!strncmp(&p_vpd->vpd[i+4],
							FDDI_SC_FRU, 
							FDDI_FN_LEN)) ||
						(!strncmp(&p_vpd->vpd[i+4],
							FDDI_FC_FRU, 
							FDDI_FN_LEN)))
						{
							p_acs->dev.card_type =	
								FDDI_SC;
							fn_found = TRUE;
						}
					}

				}
			} /* end for loop search for net addr */
		}

		if ( !p_acs->dev.pio_rc && vpd_found && na_found && 
			crc_valid && fn_found)
			p_vpd->status = FDDI_VPD_VALID;
		else
			p_vpd->status = FDDI_VPD_INVALID;
	} /* end else VPD read */

	IOCC_DET(iocc);

	FDDI_TRACE("cgvE",p_vpd,p_vpd->status,p_vpd->vpd);
	return;
} /* end get_vpd() */

/*
 * NAME: get_sc_xcard_vpd()
 *                                                                    
 * FUNCTION: get the extender card's vpd for the scarborough type of card
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *	0		- successful
 *		
 */  


void
get_sc_xcard_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	ushort 		cal_crc;
	ushort 		vpd_crc;
	uchar		tmp1, tmp2;

	/*
	 * get access to the IOCC.  
	 */
	
	FDDI_TRACE("cgsB",p_acs,0,0);

	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	p_vpd = &(p_acs->vpd);

	/* Initialize POS reg 6 and 7 */
	PIO_PUTPOS( (iocc + FDDI_POS_REG7), p_acs->dev.pos[7] | FDDI_POS7_VPS );
	PIO_PUTPOS( (iocc + FDDI_POS_REG6), p_acs->dev.pos[6] );

	for ( i = 0; i < FDDI_VPD_LENGTH; ++i )
	{
		/*
		 * set up the correct addr for the VPD read byte 
		 */
		tmp1=(uchar)i+1;
		PIO_PUTPOS( (iocc + FDDI_POS_REG6), tmp1 );

		PIO_GETPOS( (iocc + FDDI_POS_REG3), &p_vpd->xcvpd[i] );
	} 

	p_vpd->xcvpd[FDDI_VPD_LENGTH - 1] = 0x00;
	p_vpd->l_xcvpd = FDDI_XCVPD_LENGTH;

	/* Initialize POS reg 6 and 7 */
	PIO_PUTPOS( (iocc + FDDI_POS_REG7), p_acs->dev.pos[7] );
	PIO_PUTPOS( (iocc + FDDI_POS_REG6), p_acs->dev.pos[6] );

	IOCC_DET(iocc);

	FDDI_TRACE("cgsE",p_acs,0,0);
	return;
} /* end get_sc_xcard_vpd() */

/*
 * NAME: get_fr_xcard_vpd()
 *                                                                    
 * FUNCTION: 
 * 	Get the vpd for front royal extender adapters.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine reads in the VPD for the extender adapter 
 *
 *
 * RETURNS: 
 *	0		- successful
 *		
 */  

void
get_fr_xcard_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	uchar		tmp1, tmp2;

	/*
	 * get ioccess to the IOCC.  
	 */

	
	FDDI_TRACE("cgfB",p_acs,0,0);
	iocc = IOCC_ATT( p_acs->dds.bus_id,(IO_IOCC+((p_acs->dds.slot-1)<<16)));

	p_vpd = &(p_acs->vpd);

	/* Initialize POS reg 6 and 7 */
	BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos[6] );

	for ( i = 0; i < FDDI_XCVPD_LENGTH; ++i )
	{
		/*
		 * set up the correct addr for the VPD read byte 
		 */
		tmp1=(uchar)i+1;
		BUS_PUTCX( (iocc + FDDI_POS_REG6), tmp1 );

		PIO_GETPOS( (iocc + FDDI_POS_REG3), &p_vpd->xcvpd[i] );
	} 

	p_vpd->xcvpd[FDDI_XCVPD_LENGTH - 1] = 0x00;
	p_vpd->l_xcvpd = FDDI_XCVPD_LENGTH;

	IOCC_DET(iocc);
	FDDI_TRACE("cgfE",p_acs,0,0);
	return;
} /* end get_fr_xcard_vpd() */

/*
 * NAME: fddi_gen_crc()
 *                                                                    
 * FUNCTION: Generate a 16-bit CRC value
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine calculates a 16-bit crc value used to check
 *	the validity of VPD.
 *
 * RETURNS: 
 *		A 16-bit crc value per specs for MCA bus VPD area CRC.
 *		
 */  

ushort
fddi_gen_crc(	register uchar 	*p_buf,
		register int	l_buf)
{
	register uchar	work_msb;
	register uchar	work_lsb;
	register uchar	value_msb;
	register uchar	value_lsb;
	ushort rc;

	FDDI_TRACE("cgcB",0,0,0);

	for (value_msb = 0xff, value_lsb = 0xff; l_buf > 0; l_buf-- )
	{
		value_lsb ^= *p_buf++;
		value_lsb ^= (value_lsb << 4);

		work_msb = value_lsb >> 1;
		work_lsb = (value_lsb << 7) ^ value_lsb;

		work_lsb = ( work_msb << 4 ) | (work_lsb >> 4);
		work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

		value_lsb = work_lsb ^ value_msb;
		value_msb = work_msb;
	} 

	rc = ((ushort)value_msb << 8) | value_lsb;
	FDDI_TRACE("cgcE",0,0,0);
	return(rc);

} /* end fddi_gen_crc() */

/*
 * NAME: fddi_cdt_init
 *                                                                    
 * FUNCTION: initialize component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: 
 *	0 	if successful
 *	ENOMEN 	if no memory available
 */  

int
fddi_cdt_init ()
{
	FDDI_TRACE("cciB",0,0,0);
	/* 
	 * allocate dump table with initial size
	 */
	p_fddi_cdt = (struct cdt *) xmalloc(sizeof(struct cdt_head) + 
		(FDDI_CDT_ENTRIES * sizeof(struct cdt_entry)), 
		FDDI_WORDSHIFT, pinned_heap);

	if (p_fddi_cdt == NULL)
	{
		FDDI_ETRACE("cci1",0,0,0);
		/* fail */
		return (ENOMEM);
	}
	/* 
	 * initialize component dump table 
	 * l_fddi_cdt is the total size of the allocated dump table
	 * cdt_len is the current size of the actual entries in the sump table
	 */
	l_fddi_cdt = FDDI_CDT_ENTRIES;
	p_fddi_cdt->cdt_magic = DMP_MAGIC;
	bcopy (FDDI_DD_STR, p_fddi_cdt->cdt_name, sizeof(FDDI_DD_STR));
	p_fddi_cdt->cdt_len = sizeof (struct cdt_head);

	/* 
	 * add static data structures: trace table and fddi control struct
	 *	also, add the allocated open table at this time.
	 */
	fddi_cdt_add ("fddi_tbl", (char *) &fddi_tbl,  sizeof (fddi_tbl));

	/* 
	 * register for dump 
	 */
	dmp_add (((void(*)())fddi_cdt_func));

	FDDI_TRACE("cciE",p_fddi_cdt,p_fddi_cdt->cdt_len,l_fddi_cdt);
	/* ok */
	return (0);
}

/*
 * NAME: fddi_cdt_undo_init
 *                                                                    
 * FUNCTION: remove the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */  

int
fddi_cdt_undo_init ()
{
	FDDI_TRACE("ccuB",0,0,0);
	/* Delete global structures */
	fddi_cdt_del ((char *) &fddi_tbl);

	/* 
	 * If our number of entries is not zero we have an internal 
	 * programming error 
	 */
	FDDI_ASSERT(NUM_ENTRIES(p_fddi_cdt) == 0);

	dmp_del (((void(*)())fddi_cdt_func));
	xmfree(p_fddi_cdt, pinned_heap);
	p_fddi_cdt = NULL;
	l_fddi_cdt = 0;
	
	FDDI_TRACE("ccuE",0,0,0);
	return(0);
}

/*
 * NAME: fddi_cdt_add
 *                                                                    
 * FUNCTION: add an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */  

void
fddi_cdt_add (
	char	*p_name,
	char	*ptr,
	int	len)
{
	int		rc;
	int		num_entries;
	int		new_size;
	struct cdt	*p_old;
	struct cdt	*p_new;

	 
	FDDI_TRACE("ccaB",NUM_ENTRIES(p_fddi_cdt),0,0);
	/* get number of entries */
	num_entries = NUM_ENTRIES (p_fddi_cdt);

	if (num_entries == l_fddi_cdt)
	{
		/* 
		 * enlarge the entries by FDDI_CDT_ENTRIES
		 */
		new_size = sizeof (struct cdt_head) + 
			((l_fddi_cdt + FDDI_CDT_ENTRIES) * 
			sizeof (struct cdt_entry));

		/* grow dump table */
		p_new = (struct cdt *) xmalloc (new_size, FDDI_WORDSHIFT, 
						pinned_heap);

		if (p_new == NULL)
		{
			/* 
			 * can't expand dump table drop entry
			 */
			FDDI_ETRACE("cca1",0,0,0);
			return;
		}
		/* 
		 * initialize new dump table with old values
		 * (cdt_len is copied as well)
		 */
		bcopy (p_fddi_cdt, p_new, p_fddi_cdt->cdt_len);

		/* 
		 * swap curent (old) dump table with new 
		 * 	Notice p_fddi_cdt will always have a valid 
 		 *	pointer to avoid errors if the dump function
		 *	were called during the swap
		 */
		p_old = p_fddi_cdt;
		p_fddi_cdt = p_new;
		l_fddi_cdt += FDDI_CDT_ENTRIES;
		xmfree (p_old, pinned_heap);

	}
	/*
	 * Add entry 
	 */
	bcopy (p_name, p_fddi_cdt->cdt_entry[num_entries].d_name, 
			sizeof (((struct cdt_entry *) 0)->d_name));
	p_fddi_cdt->cdt_entry[num_entries].d_len = len;
	p_fddi_cdt->cdt_entry[num_entries].d_ptr = ptr;
	p_fddi_cdt->cdt_entry[num_entries].d_segval = 0;
	p_fddi_cdt->cdt_len += sizeof (struct cdt_entry);


	FDDI_TRACE("ccaE",NUM_ENTRIES(p_fddi_cdt),0,0);
	/* ok */
	return ;
}
/*
 * NAME: fddi_cdt_del
 *                                                                    
 * FUNCTION: deletes an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RETURNS: none
 */
void
fddi_cdt_del (
	char	*ptr)
{
	int	i;
	int	rc;
	int	num_entries;


	FDDI_TRACE("ccdB",NUM_ENTRIES(p_fddi_cdt),0,0);
	/*
	 * find entry that matches this ptr 
	 */
	num_entries = NUM_ENTRIES(p_fddi_cdt);
	for (i = 0; i < num_entries; i++)
	{
		if (ptr == p_fddi_cdt->cdt_entry[i].d_ptr)
		{
			/* found it */
			break;
		}
	}
	if (i < num_entries)
	{
		/* 
		 * re-pack entries 
		 */
		for ( ; i < num_entries; i++)
		{
			p_fddi_cdt->cdt_entry[i] = p_fddi_cdt->cdt_entry[i+1];
		}
		p_fddi_cdt->cdt_len -= sizeof (struct cdt_entry);
	}


	FDDI_TRACE("ccdE",NUM_ENTRIES(p_fddi_cdt),0,0);
	/* ok */
	return ;
}

/*
 * NAME: disable_card
 *                                                                    
 * FUNCTION: disables the adapter by removing the CEN bit from pos reg 2.
 *                                                                    
 * EXECUTION ENVIRONMENT: any enviroment.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * RETURNS: none
 */
void
disable_card (
	fddi_acs_t	*p_acs)
{
	int	iocc;

	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_PUTPOS2( (iocc+FDDI_POS_REG2), 
		(p_acs->dev.pos[2] & ~(FDDI_POS2_CEN) & (~FDDI_POS2_AR)));
	IOCC_DET(iocc);
}

/*
 * NAME: fddi_cdt_func
 *
 * FUNCTION: component dump function called at dump time
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *      The function is called during a component dump.  The buffer used for the
 * small frame cache (see tx section) was previously marked to be dumped and
 * at this point the sf_cache will be filled with the available data from the
 * adapter.  First the 8 bytes of pos regs are saved.  Then the 16 bytes of
 * sif registers.  Finally the 0x100 bytes of shared ram is dumped.
 *
 * RETURNS: ptr to the current component dump table
 */

struct cdt *
fddi_cdt_func(
        int     pass )
{

        int             iocc;
        int             ioa;
        int             bus;
        int             i,cnt;
        int             loop;
        fddi_acs_t      *p_acs;
	fddi_adap_dump_t	*p_dump;

        if (pass == 1)
        {
                i=0;
		cnt = 0;

                while ((i<FDDI_MAX_ACS) && (cnt < fddi_tbl.acs_cnt))
                {
                        if ((p_acs = fddi_tbl.p_acs[i]) != NULL)
                        {
				cnt++;

				p_dump = (fddi_adap_dump_t *)
					p_acs->tx.p_sf_cache;

                                bzero(p_dump, sizeof(fddi_adap_dump_t));

                                iocc =IOCC_ATT(p_acs->dds.bus_id,IO_IOCC+
                                        (p_acs->dds.slot<<16));

				/* Get the pos registers for the dump */
                                for (loop = 0; loop < 8; loop++)
                                        BUS_GETCX(iocc + loop, 
						&p_dump->pos[loop]);
				
                                IOCC_DET(iocc);

				if (p_dump->pos[2] & FDDI_POS2_CEN)
				{
                                	ioa = BUSIO_ATT(p_acs->dds.bus_id,
	                                        p_acs->dds.bus_io_addr);

					BUS_GETSRX(ioa + FDDI_HSR_REG,
						&p_dump->hsr);
					
					BUS_GETSRX(ioa + FDDI_HCR_REG,
						&p_dump->hcr);
				
					BUS_GETSRX(ioa + FDDI_NS1_REG,
						&p_dump->ns1);
				
					BUS_GETSRX(ioa + FDDI_NS2_REG,
						&p_dump->ns2);
				
					BUS_GETSRX(ioa + FDDI_HMR_REG,
						&p_dump->hmr);
				
					BUS_GETSRX(ioa + FDDI_NM1_REG,
						&p_dump->nm1);
					
					BUS_GETSRX(ioa + FDDI_NM2_REG,
						&p_dump->nm2);
				
					BUS_GETSRX(ioa + FDDI_ACL_REG,
						&p_dump->acr);
				
                                	BUSIO_DET(ioa);

                                	bus=BUSMEM_ATT(p_acs->dds.bus_id,
	                                        (uint)p_acs->dds.bus_mem_addr);

					/* Get the tx and rx descriptors */
                                	for (loop = 0; 
						loop < FDDI_MAX_TX_DESC; loop++)
                                	{
						BUS_GETSRX(bus + 
							p_acs->tx.desc[loop].offset +
							offsetof(fddi_adap_t, addr_hi),
							&p_dump->tx[loop].addr_hi);
						
						BUS_GETSRX(bus + 
							p_acs->tx.desc[loop].offset +
							offsetof(fddi_adap_t, addr_lo),
							&p_dump->tx[loop].addr_lo);

						BUS_GETSRX(bus + 
							p_acs->tx.desc[loop].offset +
							offsetof(fddi_adap_t, cnt),
							&p_dump->tx[loop].cnt);

						BUS_GETSRX(bus + 
							p_acs->tx.desc[loop].offset +
							offsetof(fddi_adap_t, ctl),
							&p_dump->tx[loop].ctl);

						BUS_GETSRX(bus + 
							p_acs->tx.desc[loop].offset +
							offsetof(fddi_adap_t, stat),
							&p_dump->tx[loop].stat);
                                	}

                                       	for (loop = 0; 
						loop < FDDI_MAX_RX_DESC; loop++)

                                	{
						BUS_GETSRX(bus + 
							p_acs->rx.desc[loop].offset +
							offsetof(fddi_adap_t, addr_hi),
							&p_dump->rx[loop].addr_hi);
							
						BUS_GETSRX(bus + 
							p_acs->rx.desc[loop].offset +
							offsetof(fddi_adap_t, addr_lo),
							&p_dump->rx[loop].addr_lo);
	
						BUS_GETSRX(bus + 
							p_acs->rx.desc[loop].offset +
							offsetof(fddi_adap_t, cnt),
							&p_dump->rx[loop].cnt);
	
						BUS_GETSRX(bus + 
							p_acs->rx.desc[loop].offset +
							offsetof(fddi_adap_t, ctl),
							&p_dump->rx[loop].ctl);
	
						BUS_GETSRX(bus + 
							p_acs->rx.desc[loop].offset +
							offsetof(fddi_adap_t, stat),
							&p_dump->rx[loop].stat);
                                	}
                                	BUSMEM_DET(bus);
                        	}
			}
                        i++;
                }
        }

        return (p_fddi_cdt);
}

