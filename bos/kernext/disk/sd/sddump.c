static char sccsid[] = "@(#)95	1.8  src/bos/kernext/disk/sd/sddump.c, sysxdisk, bos411, 9428A410j 3/16/94 09:52:19";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem Device Driver
 *
 * FUNCTIONS:  sd_dump(), sd_dumpstrt(), sd_dumpend(), sd_dmp_quiesce(),
 *             sd_dumpwrt(), sd_dump_dev(), sd_dump_read(),sd_dump_complete() 
 *             sd_dump_reqsns(),and  sd_dump_retry()
 *
 * ORIGINS: 27  
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sd.h>
#include <sys/errno.h>

/*
 *                                                                       
 * NAME: sd_dump                                                         
 *                                                                       
 * FUNCTION: Determine what type of dump operation is being sought
 *
 *      Allocate necessary segment registers and parse type of
 *      dump operation.  Call the specified routine.
 *                                                                       
 *                                                                        
 * EXECUTION ENVIRONMENT:                                                 
 *                                                                        
 *      This routine must be called in the interrupt environment.  It     
 *      can not page fault and is pinned.
 *
 * (NOTES:) This routine handles the following operations :   
 *      DUMPINIT   - initializes bus attached disk as dump device
 *      DUMPSTART  - prepares device for dump
 *      DUMPQUERY  - returns the maximum and minimum number of bytes that
 *                   can be transferred in a single DUMPWRITE command
 *      DUMPWRITE  - performs write to disk
 *      DUMPEND    - cleans up device on end of dump
 *      DUMPTERM   - terminates the bus attached disk as dump device
 *
 * (DATA STRUCTURES:) uio             - structure containing information about
 *                                      the data to transfer 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    dmp_query       - queried transfer information is 
 *                                      returned
 *                    xmem            - cross memory descriptor
 *                                                          
 * INPUTS:                                                  
 *      devno   - device major/minor number                 
 *      uiop    - pointer to uio structure for data for the 
 *                specified operation code 
 *      cmd     - Type of dump operation being requested
 *      arg     - Pointer to dump query structure
 *      chan    - unused
 *      ext     - unused
 * 
 * INTERNAL PROCEDURES CALLED:
 * 
 *      sd_dumpstrt
 *      sd_dumpwrt
 *      sd_dumpend
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *
 *      IOCC_ATT
 *      BUSIO_ATT
 *      BUSIO_DET
 *      IOCC_DET
 *
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:
 *      EBUSY     -  request sense inuse
 *      EINVAL    -  Invalid iovec argumenmt,or invalid cmd
 *      EIO       -  I/O error, quiesce or scsi writefailed.
 *      ENOMEM    -  Unable to allocate resources
 *      ENXIO     -  Not inited as dump device
 *      ETIMEDOUT - adapter not responding
 */                                                                        

int sd_dump(dev_t devno,                /* device major/minor number      */
	    struct uio *uiop, 		/* data transfer information      */
	    int cmd,                    /* typ of operation requested     */
	    int  *arg,                  /* pointer dump query structure   */
	    int  chan,                  /* unused                         */
	    int  ext)                   /* unused                         */
{
	struct sd_adap_info     *ap;        /* pointer adapter 		  */
	struct sd_dasd_info     *dp;        /* pointer dasd 		  */
	struct dmp_query        *dpr;       /* dump query structure 	  */
        uint                    iocc_seg;   /* iocc segment register value*/
        uint                    seg_base;   /* base segment register value*/
	int                     ret_code = 0;/* return code 		  */




	dp = (struct sd_dasd_info *) sd_hash(devno);
	
	if (dp == NULL)
		return(EINVAL);

	ap = dp->ap;
	iocc_seg = (uint) IOCC_ATT(ap->dds.bus_id,SD_DELAY_REG);
	seg_base = (uint) BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
	switch(cmd) {
	      case DUMPINIT:              /* Initialization for dump */

		/* 
		 *code already pinned do nothing 
		 */

		break;

                          
	      case DUMPQUERY:       /* Determine max  bytes xfer in one write*/
		dpr  = (struct dmp_query *)arg;
		dpr->min_tsize = SD_BPS;
		dpr->max_tsize = dp->max_transfer;
		break;

	      case  DUMPSTART:                     /* setup device for dump */
		ret_code = sd_dumpstrt(iocc_seg,seg_base,dp);
		break;

	      case  DUMPWRITE:                 /* Write dump data to device */
		ret_code = sd_dumpwrt(iocc_seg,seg_base,dp,uiop);
		break;

	      case DUMPEND:                    /* Cleanup after dump */
		sd_dumpend(dp);
		break;
	      case DUMPTERM: /*Release resources allocated for dump support */

		/*
		 *  code already pinned do nothing 
		 */

		break;
	      default:
		ret_code = EINVAL;  
	} /* switch */

	/* 
	 * free segment registers 
	 */

	BUSIO_DET(seg_base);
	IOCC_DET(iocc_seg);
	return (ret_code);
}



/*
 *                                                                       
 * NAME: sd_dumpstrt                                                     
 *                                                                       
 * FUNCTION: Notify dasd of pending dump
 *                                                                       
 *        Prepare device for dump, let existing io finish, depending     
 *        only on the device status, then disable interrupts, and go to  
 *        polling mode for all data transfers.                           
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned   
 *
 *
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                                                          
 * INPUTS: 
 *      iocc_seg -  iocc segment register
 *      seg_base -  base segment register
 *      dp       -  dasd to which we will prepare for dump
 *
 *
 *
 * CALLED BY:
 *      sd_dump
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd__fail_adap                   sd_MB_alloc
 *      sd_cmd_alloc                   sd_free_cmd
 *      sd_dmp_quiesce  
 *     
 * EXTERNAL PROCEDURES CALLED:
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:
 *      EIO       -  Failed quiesce or scsi write
 *      ENOMEM    -  Unable to allocate resources
 *      ETIMEDOUT - adapter not responding
 */                                                                       

int sd_dumpstrt(uint iocc_seg,            /* iocc segment register      */
		uint seg_base,            /* base segment register      */
		struct sd_dasd_info *dp)  /* pointer to effected DASD   */
{    
	struct sd_adap_info  *ap;         /* adapter 			*/
	struct sd_cmd        *cmd;        /* command jacket 		*/
	int                  ret_code = 0;/* return code 		*/
	


	ap = dp->ap;
	
	/*
	 * No need to disable or lock, since dump dd disables
	 * at INTMAX and runs only on master CPU.
	 */

	ap->dumpdev = TRUE;
	ap->status |= SD_SUSPEND;


	/*
	 * Quiesce the dasd by getting a cmd jacket and build a mailbox
	 * if unavailable then fail dump. 
	 */

	sd_fail_adap(ap);            /* fail all cmds for the given adapter */
	cmd = sd_cmd_alloc(ap);
	if (cmd == NULL) {

		ap->dumpdev = FALSE;
		ap->status &= ~SD_SUSPEND;
		ret_code = ENOMEM;        
	}
	else {
		cmd->type = SD_ADAP_CMD;
		cmd->ap = ap;
		cmd->cp = dp->cp;
		cmd->dp = dp;
		if (sd_MB_alloc(cmd)) {
			sd_free_cmd(cmd);
			ret_code = ENOMEM;

			ap->dumpdev = FALSE;
			ap->status &= ~SD_SUSPEND;
		}
		else {
			ret_code = sd_dmp_quiesce(cmd,dp,seg_base,iocc_seg);
			if (ret_code) {

				ap->dumpdev = FALSE;
				ap->status &= ~SD_SUSPEND;
			}

		}  /* else: if sd_MB_alloc */

	} /* else: if cmd == NULL */

	return (ret_code);
}


/*
 *                                                                        
 * NAME: sd_dmp_quiesce                                                   
 *                                                                        
 * FUNCTION: Quiesce adapter
 *                                                                        
 *       issue a quiesce mailbox to the adapter and wait for its          
 *	 completion                                                       
 *                                                                        
 * EXECUTION ENVIRONMENT:                                                 
 *                                                                        
 *      This routine must be called in the interrupt environment.  It     
 *      can not page fault and is pinned.
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    xmem            - cross memory descriptor
 *                    sd_mbox         - mail box for quiesce command
 *                                                          
 * INPUTS: 
 *      cmd      -  allocated command jacket
 *      dp       -  dasd to which we will prepare for dump
 *      seg_base -  base segment register
 *      iocc_seg -  iocc segment register
 *
 *
 *
 * CALLED BY:
 *      sd_dumpstrt
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_cmd
 *      sd_free_MB                   
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      dc_flush                       BUSIO_PUTC
 *      BUSIO_GETC
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:
 *      EIO       -  Failed quiesce or scsi write
 *      ETIMEDOUT - adapter not responding
 */                                                                        

int sd_dmp_quiesce(struct sd_cmd *cmd,        /* quiesce command jacket */
		   struct sd_dasd_info *dp,   /* effected DASD          */
		   uint seg_base,	      /* base segment register  */
		   uint iocc_seg)	      /* iocc segment register  */
{
	struct sd_adap_info *ap;	 /* adapter pointer                */
	uchar               quiesce_tag; /* mailbox tag of quiesce command */
	uint                tags;	 /* contents completion reg 	   */
	char                *tag;        /* one byte of completion reg	   */
	uint                alert_word;  /* contents pf alert register	   */
	uchar               *alert;      /* one byte of alert register	   */
	int                 i,j;         /* general counter	           */
	int                 loop;        /* controls exist from while loop */
	int                 ret_code = 0;/* return code 	    	   */
	int                 read_tags;   /* number of times completion reg */
	                                 /* has been read.		   */
	caddr_t             daddr;       /* dma address 		   */
	caddr_t             eaddr;       /* effective dma address	   */
	uchar               status;      /* value of adapter control reg   */


	ap = dp->ap;

	/* 
	 * fill in cmd jacket and mailbox for quiesce adapter command
	 */

	cmd->mbox_copy.mb6.reset_type = SD_QUIESCE_ADAP_MB;
	cmd->mb->mb6.reset_type = SD_QUIESCE_ADAP_MB;
	cmd->mbox_copy.op_code = SD_RSTQSC_OP;
	cmd->mb->op_code = SD_RSTQSC_OP;

	cmd->mbox_copy.mb7.dev_address = 0;
	cmd->mb->mb7.dev_address = 0;
	quiesce_tag = cmd->mb->tag; 
	cmd->mbox_copy.tag = cmd->mb->tag;


	/*
	 * flush MB cache lines.
	 */

	eaddr = (caddr_t)cmd->mb;
	daddr = (caddr_t)SD_MB_ADDR((uint)ap->base_MB_dma_addr,
		(uint)cmd->mb->tag);
	d_cflush((int) (ap->dma_channel),eaddr,(int) SD_MB_SIZE,daddr);
   
	/* 
	 * write ap->last_tag to LAST TAG reg 
	 * ie issue quiesce command 
	 */
	ap->last_tag = cmd->mb->tag;
	if (SD_PUTC(seg_base + SD_LTAG,ap->last_tag)) {
		sd_free_MB(cmd, (char)SD_NOT_USED);
		sd_free_cmd(cmd);
		return(EIO);

	}
	i = 0;
	read_tags = 0;
	loop = TRUE;
	while ((loop) && (++i < SD_MAX_DMP_LOOPS)) {
		/*
		 * delay 1 micro
		 * second 
		 */

		BUSIO_PUTC(iocc_seg,0x00);

		/* 
		 * poll for interrupt 
		 */

		if (SD_GETC(seg_base + SD_CTRL,&status)) {
			loop = FALSE;
			sd_free_MB(cmd, (char)SD_USED);
			sd_free_cmd(cmd);
		}
		if (status & 0x40) {  /* device sets interrupt bit */

			/* 
			 * read completion reg 
			 */ 

			if (SD_GETL(seg_base + SD_CMPL,&tags)) {
				loop = FALSE;
				sd_free_MB(cmd, (char)SD_USED);
				sd_free_cmd(cmd);
			}
			tag = (char *) &tags;

			/*
			 * process first byte of comp reg 
			 */

			for(j = 0;j < SD_CMPL_REG_SIZE; j++ ) {
				read_tags++; /* increment number of read tags*/

				/* 
				 * quiesce completed successfully
				 */

				if (tag[j] == quiesce_tag) {   
					loop = FALSE;
					sd_free_MB(cmd, (char)SD_USED);
					sd_free_cmd(cmd);
					break;
				}
				else if (tag[j] == 0x00) {
					break;
				}
				else if (tag[j] == 0xFF) {

					/* 
					 * deal with errors 
					 */

					ret_code = EIO;
					loop = FALSE;
					sd_free_MB(cmd, (char)SD_USED);
					sd_free_cmd(cmd);
					break;
				}   /* else: if tag[j] = quiesce */
				else if ((tag[j] < SD_MIN_TAG) || 
					(tag[j] > SD_MAX_TAG) ) {

					/* 
					 * deal with errors 
					 */

					ret_code = EIO;
					loop = FALSE;
					sd_free_MB(cmd, (char)SD_USED);
					sd_free_cmd(cmd);
					break;
				}
				if (read_tags > (SD_MAX_TAG + 1)) {
					/*
					 * should have read quiesce by now
					 * so fail dump if reach this
					 */

					ret_code = EIO;
					loop = FALSE;
					sd_free_MB(cmd, (char)SD_USED);
					sd_free_cmd(cmd);
					break;
				}
			}
		}  /* if status & x40 */
	} /* while */
	if (i == SD_MAX_DMP_LOOPS) {       /* if interrupt bit never reset by device */
		ret_code = ETIMEDOUT;
		sd_free_MB(cmd, (char)SD_USED);
		sd_free_cmd(cmd);
	}
	return (ret_code);
}


/*
 *                                                                       
 * NAME: sd_dumpwrt                                                      
 *                                                                       
 * FUNCTION: Write to the dump device.
 *                                                                       
 *          issue as many scsi commands as neccessry to write uiop's     
 *          data to dasd using the SCSI write command.  If any errors    
 *	    then fail return w/ error.                                   
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                                                          
 * INPUTS: 
 *      iocc_seg -  iocc segment register
 *      seg_base -  base segment register
 *      dp       -  dasd to which we will dump
 *      uiop     -  pointer to uio structure for data for the 
 *                  specified operation code
 *      base     -  base register for reading POS registers
 *
 *
 *
 * CALLED BY:
 *      sd_dump
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_dump_dev                   sd_dump_read
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      BUSIO_PUTC                    BUSIO_GETC
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:
 *      EBUSY     -  request sense inuse
 *      EINVAL    -  Invalid iovec argument
 *      EIO       -  I/O error
 *      ENOMEM    -  Unable to allocate resources
 *      ENXIO     -  Not inited as dump device
 *      ETIMEDOUT - timed out because no interrupt
 *     
 */                                                                       

int sd_dumpwrt(uint iocc_seg,		/* iocc segment register 	 */
	       uint seg_base,		/* base segment register	 */
	       struct sd_dasd_info *dp,	/* effected DASD	      	 */
	       struct uio *uiop)	/* Contains data to be written   */
{
	struct sd_adap_info *ap;       /* dasd's adapter 		*/
	uint                tags;      /* completion register contents 	*/
	uchar               *tag;      /* one byte of completion reg 	*/
	int                 ret_code = 0;/* return code 		*/
	int                 lba;       /* logical block address 	*/
	struct iovec        *iovp;     /* pointer iovec structure	*/
	int                 i,j,k;       /* general counter 		*/
	int                 loop;      /* flag controlling exit from 	*/
				       /* while loop			*/
	uchar               status;    /*adapter control register 	*/
				       /* contents 			*/


	/* 
	 * if not inited as dump device 
	 */

	if (!(dp->ap->dumpdev)) 
		return (ENXIO);
	ap = dp->ap;
	dp->reqsns_tag = 0x0;
	/*
	 * build mailbox(es) for write scsi command(s) 
	 */

	iovp = uiop->uio_iov;
	dp->xmem_buf.aspace_id = XMEM_GLOBAL;   /* set cross mem descrip */
	lba = uiop->uio_offset/SD_BPS;
	for (i = 0; i < uiop->uio_iovcnt; i++) {


		/* 
		 * build send scsi mailbox for iovec 
		 */

		ret_code = sd_dump_dev(dp,seg_base,iocc_seg,iovp,lba);
		if (ret_code)
			break;

		/* 
		 * if no errors so far 
		 * issue above command by 
		 * writing to last tag register 
		 */

		if (SD_PUTC(seg_base+SD_LTAG,ap->last_tag)) {
		        ret_code = EIO;
			break;
		} 
		k = 0;
		loop = TRUE;

		/* 
		 * loop until command completes or an
		 * error is detected 
		 */

		while ((loop) && (++k < SD_MAX_DMP_LOOPS)) {

			/* 
			 * delay 1 micro
			 * second 
			 */

			BUSIO_PUTC(iocc_seg,0x00);

			/* 
			 * poll for interrupt
			 */

			if (SD_GETC(seg_base + SD_CTRL,&status)) {
				loop = FALSE;
				ret_code = EIO;
			}
			if (status & 0x40) {       /* if device interrupts */

				/* 
				 * read completion reg 
				 */

				k =0;
				if (SD_GETL(seg_base + SD_CMPL,&tags)) {
					loop = FALSE;
					ret_code = EIO;
				}
				tag = (uchar *) &tags;
				ret_code =sd_dump_read(tag,dp,seg_base,
						       iocc_seg,&loop,
						       iovp);
				if (ret_code)
					loop = FALSE;
			} /* if */
		} /* while */
		if (ret_code)
			break;

		if (k == SD_MAX_DMP_LOOPS) {
			ret_code = ETIMEDOUT;
			break;
		}

		/* 
		 * get lba for next iovec struct 
		 */

		lba += iovp->iov_len/ SD_BPS;

		/* 
		 * get next iovec struct 
		 */
		uiop->uio_resid -= iovp->iov_len;
		iovp->iov_len = 0;
		iovp = (struct iovec *) ((int)iovp + sizeof(struct iovec));
		
	} /*for loop */

	return (ret_code);
}

/*
 *                                                                        
 * NAME: sd_dump_dev                                                      
 *                                                                        
 * FUNCTION: Build scsi write mailboxes for the indicated dasd
 *                                                                        
 *      This routine fills in mailboxes for the write scsi command for    
 *	all data in one iovec. It also fills in the scsi command block for the 
 *	scsi write.                                                       
 *                                                                        
 * EXECUTION ENVIRONMENT:                                                 
 *                                                                        
 *      This routine must be called in the interrupt environment.  It     
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_mbox         - mail box for scsi write command
 *                                                          
 * INPUTS: 
 *      dp       -  dasd to which we will dump
 *      seg_base -  base segment register
 *      iocc_seg -  iocc segment register
 *      iovec    -  present scsi write info for dump device
 *      xmem     -  cross memory descriptor for dma
 *
 *
 *
 * CALLED BY:
 *      sd_dumpwrt
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_cmd_alloc                  sd_free_cmd
 *      SD_LUNTAR                        sd_TCW_alloc
 *      SD_DMA_ADDR
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      d_master                      d_cflush
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:
 *      EINVAL -  Invalid iovec argument
 *      EIO    -  I/O error
 *      ENOMEM -  Unable to allocate resources
 */                                                                        
int sd_dump_dev(struct sd_dasd_info *dp, /* Effected DASD		 */
		uint seg_base,		 /* base segment register	 */
		uint iocc_seg,		 /* iocc segment register	 */
		struct iovec *iovp,	 /* Contains data to be written	 */
		int lba)		 /* Logical block address	 */
{
	struct sd_cmd        *cmd;     /* command jacket  		*/
	struct sd_adap_info  *ap;      /* adapter pointer 		*/
	struct sc_cmd        *cdb;     /* scsi command block 		*/
	struct sc_cmd        *cdb2;    /* scsi command block 		*/
	int                  ret_code = 0;/* return code 		*/
	caddr_t              daddr;    /* dma address 			*/
	caddr_t              eaddr;    /* effective dma address 	*/
	uchar                tarlun;   /* the target and lun id of the 	*/
				       /* dasd 				*/
	uchar                target;   /*the target id of the controller*/
	uchar                *lba_byte;/* one byte of lba 		*/
	int                  i;        /* general counter 		*/
	int                  transfer_length; /* size of transfer in blocks */

	ap = dp->ap;
	if ( (iovp->iov_len/SD_BPS) > dp->max_transfer) {   
		return(EINVAL);
	}
	cmd = sd_cmd_alloc(ap);
	if (cmd == NULL) {
		return(ENOMEM);
	}
	cmd->type = SD_DASD_CMD;
	cmd->ap = ap;
	cmd->cp = dp->cp;
	cmd->dp = dp;
	if (sd_MB_alloc(cmd)) {
		sd_free_cmd(cmd);
		return(ENOMEM);
	}

	/* 
	 * fill in cmd and mailbox 
	 */

	cmd->mbox_copy.op_code = SD_SEND_SCSI_OP;
	cmd->mb->op_code = SD_SEND_SCSI_OP;
	cmd->mbox_copy.mb6.qc_scsiext = 0x00;
	cmd->mb->mb6.qc_scsiext = 0x00;

	/* 
	 * fill in tarlun address 
	 */

	target = dp->cp->dds.target_id;
	tarlun = SD_LUNTAR(target,dp->dds.lun_id,SD_LUNDEV);

	cmd->mbox_copy.mb7.dev_address = tarlun;
	cmd->mb->mb7.dev_address = tarlun;

	/* 
	 * create write scsi command 
	 */

	cdb = (struct sc_cmd *)&(cmd->mbox_copy.mb8.fields.scsi_cmd);
	cdb2 = (struct sc_cmd *)&(cmd->mb->mb8.fields.scsi_cmd);
	cdb->scsi_op_code = SCSI_WRITE_EXTENDED;
	cdb2->scsi_op_code = SCSI_WRITE_EXTENDED;
    
	/* 
	 * Serial DASD subsystem does not use the lun field of sc_cmd 
	 */
    

	/* 
	 * insert lba and shift over  so can insert length for
	 * extended write scsi command
	 */
	lba_byte = (uchar *)&lba;
	cdb->lun = 0;
	cdb2->lun = 0;
	cdb->scsi_bytes[0] = lba_byte[0];
	cdb2->scsi_bytes[0] = lba_byte[0];
	cdb->scsi_bytes[1] = lba_byte[1];
	cdb2->scsi_bytes[1] = lba_byte[1];
	cdb->scsi_bytes[2] = lba_byte[2];
	cdb2->scsi_bytes[2] = lba_byte[2];
	cdb->scsi_bytes[3] = lba_byte[3];
	cdb2->scsi_bytes[3] = lba_byte[3];
	cdb->scsi_bytes[4] = 0;
	cdb2->scsi_bytes[4] = 0;
	transfer_length = iovp->iov_len/ SD_BPS;
	cdb->scsi_bytes[5] = (transfer_length >> 8) & 0xFF; 
	cdb2->scsi_bytes[5] = cdb->scsi_bytes[5];
	cdb->scsi_bytes[6] = transfer_length & 0xFF;
	cdb2->scsi_bytes[6] = cdb->scsi_bytes[6];
	cdb->scsi_bytes[7] = 0;   
	cdb2->scsi_bytes[7] = 0;


	/*
	 * allocate the TCW's for this cmd 
	 */
	
	cmd->b_addr = (char *) iovp->iov_base;
	cmd->b_length = (uint) iovp->iov_len;
	cmd->dma_flags = 0;

	if (sd_TCW_alloc(cmd) == FALSE) {
		sd_free_MB(cmd, (char)SD_NOT_USED);
		sd_free_cmd(cmd);
		return(ENOMEM);
	}

	/* 
	 * Fill in dma fields of mail box 
	 */

	
	daddr = (caddr_t)(SD_DMA_ADDR(cmd->ap->dds.tcw_start_addr, 
		cmd->tcws_start) + ((uint)cmd->b_addr & (SD_TCWSIZE - 1)));
	cmd->mbox_copy.mb8.fields.dma_addr = (uint) daddr;
	cmd->mb->mb8.fields.dma_addr = (uint) daddr;
	cmd->mbox_copy.mb8.fields.dma_length = (uint)iovp->iov_len;
	cmd->mb->mb8.fields.dma_length = (uint) iovp->iov_len;
        cmd->mbox_copy.tag = cmd->mb->tag;
        cmd->mbox_copy.nextmb = cmd->mb->nextmb;

	d_master((int)ap->dma_channel,(int) cmd->dma_flags,(caddr_t)iovp->iov_base,
		 (size_t)iovp->iov_len,
		 &dp->xmem_buf,daddr);


	/*
	 * flush MB cache lines.
	 */

	eaddr = (caddr_t)cmd->mb;
	daddr = (caddr_t)SD_MB_ADDR((uint)ap->base_MB_dma_addr,
		(uint)cmd->mb->tag);
	d_cflush((int) (ap->dma_channel),eaddr,(int) SD_MB_SIZE,daddr);

   
	ap->last_tag = cmd->mb->tag;
	return (ret_code);
}


/*
 *                                                                       
 * NAME: sd_dump_read                                                    
 *                                                                       
 * FUNCTION: Read adapters completion and alert registers.
 *                                                                       
 *          Check contents of completion register and if necessary read  
 *	    the alert register                                           
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned.
 *
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_cmd          - command jacket
 *                                                          
 * INPUTS: 
 *      tag      -  tag (one byte) of completion register
 *      dp       -  dasd to which we will dump
 *      seg_base -  base segment register
 *      iocc_seg -  iocc segment register
 *      loop     -  controls while loop back in sd_dumpwrt
 *      iovec    -  present scsi write info for dump device
 *      xmem     -  cross memory descriptor for dma
 *
 *
 * CALLED BY:
 *      sd_dumpwrt
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_MB                    sd_free_cmd
 *      sd_free_TCW                   sd_dump_retry
 *      sd_dump_reqsns
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      d_complete                     BUSIO_PUTC                     
 *      BUSIO_GETC
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 *
 * RETURNS:
 *      EBUSY  -  request sense inuse
 *      EIO    -  I/O error
 *      ENOMEM -  Unable to allocate resources
 */                                                                       

int sd_dump_read(uchar *tag,		   /* completion register 	 */
		 struct sd_dasd_info *dp,  /* effected DASD 		 */
		 uint seg_base,		   /* base segment register    	 */
		 uint iocc_seg,		   /* iocc segment register 	 */
		 int *loop,		   /* control loop in sd_dumpwrt */
		 struct iovec *iovp)	   /* data being written 	 */
{
	struct sd_adap_info *ap;            /* dasd's adapter 		*/
	int                 ret_code = 0;   /* return code 		*/
	int                 j;              /* general counter 		*/
	uint                alert_word;     /* alert register contents  */
	uchar               *alert;         /* one byte of alert reg 	*/
	uchar               pos4;           /* POS register 4 		*/
	struct sd_cmd       *cmd;           /* command jacket 		*/
	caddr_t             mb_addr;        /* mailbox dma address 	*/
	caddr_t             daddr;          /* dma address 		*/
	uint                dma_length;     /* length of dma 		*/
	struct sd_cmd       *cmd2;


	ap = dp->ap;

	/*
	 * read completion register and if necessary the alert register
	 */

	for (j = 0;j < SD_CMPL_REG_SIZE;j++) {                            
		if (tag[j] == 0x00) {
			*loop = FALSE;
			break;
		}
		else if (tag[j] == 0xFF) {           /* read alert register */

			if (SD_GETL(seg_base + SD_ALRT,&alert_word)) {
				*loop = FALSE;
				ret_code = EIO;
				break;

			}
			alert = (uchar *) &alert_word;
            
			/* 
			 * check alert reg and respond accordingly 
			 */

			if (alert[2] == 0x00) {        /* scsi bus status */

				if (alert[3] == 0x02) {  /* check condition */

					/* 
					 * issue  request sense and retry 
					 * cmd 
					 */
					sd_dump_complete(ap->cmd_map[alert[0]],
							 ap,iovp,dp);
					ret_code=sd_dump_reqsns(dp,seg_base);
					if (ret_code) 
						break;
					else 
						dp->old_tag = alert[0];
					break;
                
				}
				else if (alert[3] == 0x28) {  /* queue full */

					/* 
					 * retry cmd several times 
					 */

					sd_dump_complete(ap->cmd_map[alert[0]],
							 ap,iovp,dp);
					ret_code =sd_dump_retry(alert[0],dp,
								seg_base,iovp); 
					
					break;
					
				}
				else {

					/*
					 * fail dump if good or 
					 * reservation conflict
					 */
					sd_dump_complete(ap->cmd_map[alert[0]],
							 ap,iovp,dp);
					ret_code = EIO;
					break;
				}

			}
			else if (alert[0] == 0xFF)  {    /* error detected */

				if (alert[2] == 0x1D) {
                                                      
					/*
					 * reenable dma bit
					 */
					if (sd_write_POS(ap,SD_POS4,ap->pos4)) 
						{
							ret_code = EIO;
							break;
						}

				}
				else {            /* fail dump write */
					

					ret_code = EIO;
					break;
                
				}
			}
			else {        /* fail dump write */

				ret_code = EIO;
				break;
			}
		}
		else if ((tag[j] >= SD_MIN_TAG) && (tag[j] <= SD_MAX_TAG) ){ 

			/* 
			 * cmd successful 
			 * get cmd jacket  
			 */

			cmd = dp->ap->cmd_map[tag[j]];
			if ((cmd == NULL) || (!(cmd->status & SD_ACTIVE))) {
				ret_code = EIO;
				break;
			}
			cmd2 = dp->checked_cmd;
			sd_dump_complete(cmd,ap,iovp,dp);

		       
			
			/* 
			 * free cmd and 
			 *  mail box 
			 */

			sd_free_cmd(cmd);
			if (tag[j] == dp->reqsns_tag) { 

				/* 
				 * if request sense completed
				 * successfully  then retry old command 
				 */
			        dp->reqsns_tag = 0;
				dp->checked_cmd = cmd2;
				dp->reqsns.status = SD_FREE;
				ret_code=sd_dump_retry(dp->old_tag,dp,
						       seg_base,iovp);

				
				break;
			}
			else
				*loop = FALSE;
		}
		else { /* invalid tag */
			ret_code = EIO;
			break;

		}
	}
	return (ret_code);
}

/*
 *                                                                       
 * NAME: sd_dump_complete                                                  
 *                                                                       
 * FUNCTION: Issue d_complete for mailbox and any dma transfers
 *                                                                       
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_cmd          - command jacket
 *                    iovp            - pointer to sent command
 *                                                          
 * INPUTS: 
 *      ap       -  adapter to which command was sent
 *      cmd      -  command jacket
 *      iovec    -  present scsi write info for dump device
 *      xmem     -  cross memory descriptor for dma
 *
 *
 * CALLED BY:
 *      sd_dump_read
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_TCW                       SD_DMA_ADDR
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      d_complete
 *                                                                   
 * (RECOVERY OPERATION:)
 *        None
 *
 * RETURNS:
 *      nothing
 */   
void sd_dump_complete(struct sd_cmd *cmd,      /* command jacket      	*/
		      struct sd_adap_info *ap, /* effected adatper      */
		      struct iovec *iovp,      /* data being written	*/
		      struct sd_dasd_info *dp)
{
	uint       dma_length;        /* to check if dma was done 	*/
	caddr_t    daddr;             /* dma address 			*/
	caddr_t    mb_addr;           /* mail box dma address 		*/

	if ((cmd == NULL) || (!(cmd->status & SD_ACTIVE)))
	    return;

	dma_length = (cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_bytes[5] << 8);
	dma_length |= cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_bytes[6];
	if (dma_length) { /* dma was used in tag's command */
		daddr = (caddr_t)(SD_DMA_ADDR(cmd->ap->dds.tcw_start_addr, 
			cmd->tcws_start) + 
			((uint)cmd->b_addr & (SD_TCWSIZE - 1)));
		d_complete((int) ap->dma_channel,(int) cmd->dma_flags,
			   (caddr_t) iovp->iov_base,(size_t)iovp->iov_len,
			   &dp->xmem_buf,daddr);
		sd_TCW_dealloc(cmd);
	}

	/*
	 * d_complete the mailbox 
	 */
	daddr = (caddr_t)SD_MB_ADDR((uint)ap->base_MB_dma_addr,
		(uint)cmd->mb->tag);

        (void)d_complete((int)ap->dma_channel,(int)(SD_DMA_TYPE | 
		DMA_NOHIDE), (char *)cmd->mb, (size_t)SD_MB_SIZE, 
		&ap->xmem_buf, (char *)cmd->mb_dma_addr);
	dp->checked_cmd = cmd;
	sd_free_MB(cmd, (char)SD_USED);
	return;
}

/*
 *                                                                       
 * NAME: sd_dump_reqsns                                                  
 *                                                                       
 * FUNCTION: Send request sense mailbox to the indicated dasd
 *                                                                       
 *  Issue a request sense to the dasd by filling in a send scsi command  
 *  mailbox.  If a request sense is in use then fail dump.               
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_cmd          - command jacket
 *                                                          
 * INPUTS: 
 *      dp       -  dasd to which we will dump
 *      seg_base -  base segment register
 *
 *
 * CALLED BY:
 *      sd_dump_read
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_MB                    sd_MB_alloc
  *     
 * EXTERNAL PROCEDURES CALLED:
 *      bcopy                         d_cflush                     
 *      BUSIO_PUTC
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:
 *      EBUSY  -  request sense inuse
 *      ENOMEM -  Unable to allocate resources
 */                                                                       

int sd_dump_reqsns(struct sd_dasd_info *dp, /* Effected DASD		 */
		   uint seg_base)	    /* base segment register 	 */
{
	struct sd_adap_info   *ap;       /* adpater pointer 		*/
	int                   ret_code = 0;/* return code 		*/
	struct sd_cmd         *cmd;      /* command jacket 		*/
	caddr_t               eaddr;     /* effective dma address 	*/
	caddr_t               daddr;     /* dma address 		*/
	struct sc_cmd        *cdb;     /* scsi command block 		*/
	struct sc_cmd        *cdb2;    /* scsi command block 		*/
	int            transfer_length; /* size of transfer in blocks */

	ap = dp->ap;
	/* 
	 * issue request sense 
	 */

	if (dp->reqsns.status != (uchar)SD_FREE) {    /*fail dump */
		return(EBUSY);
	}
	cmd = &dp->reqsns;

	cmd->type = SD_DASD_CMD;
	cmd->ap = ap;
	cmd->cp = dp->cp;
	cmd->dp = dp;
	if (sd_MB_alloc(cmd)) {
		return(ENOMEM);
	}
	cmd->mbox_copy.op_code = SD_SEND_SCSI_OP;
	cmd->mb->op_code = SD_SEND_SCSI_OP;
	cmd->mbox_copy.mb7.dev_address = SD_LUNTAR(dp->cp->dds.target_id, 
						   dp->dds.lun_id,SD_LUNDEV);	
	cmd->mb->mb7.dev_address = cmd->mbox_copy.mb7.dev_address;
	cmd->mbox_copy.mb6.qc_scsiext = 0x00;
	cmd->mb->mb6.qc_scsiext = 0x00;

	cdb = (struct sc_cmd *)&(cmd->mbox_copy.mb8.fields.scsi_cmd);
	cdb2 = (struct sc_cmd *)&(cmd->mb->mb8.fields.scsi_cmd);
	cdb->scsi_op_code = SCSI_REQUEST_SENSE;
	cdb2->scsi_op_code =  SCSI_REQUEST_SENSE;

	cdb->lun = 0;
	cdb2->lun = 0;
	cdb->scsi_bytes[0] = 0;
	cdb2->scsi_bytes[0] = 0;
	cdb->scsi_bytes[1] = 0;
	cdb2->scsi_bytes[1] = 0;
	
	transfer_length = 255;
	cdb->scsi_bytes[2] = transfer_length;
	cdb2->scsi_bytes[2] = transfer_length;
	cdb->scsi_bytes[3] = 0;
	cdb2->scsi_bytes[3] = 0;
	cdb->scsi_bytes[4] = 0;
	cdb2->scsi_bytes[4] = 0;	
	cdb->scsi_bytes[5] = 0;
	cdb2->scsi_bytes[5] = 0;

	cmd->b_addr = (char *) dp->sense_buf;
	cmd->b_length = (uint) 255;
	cmd->dma_flags = DMA_READ;

	if (sd_STA_alloc(cmd) == FALSE) {
		/*
		 * if could not get STA for this command
		 * free the mailbox and the command will remain on
		 * the queue
		 * Don't Want start to make another pass
		 */
		sd_free_MB(cmd, (char)SD_NOT_USED);
		sd_free_cmd(cmd);
		return(ENOMEM);
	}
	/* 
	 * remember command tag 
	 */

	dp->reqsns_tag = cmd->mb->tag;


	/* 
	 * Fill in dma fields of mail box 
	 */
	daddr = (caddr_t)(SD_DMA_ADDR(ap->dds.tcw_start_addr, 
		ap->sta_tcw_start)) + ((uint)(ap->STA[cmd->sta_index].stap)
		- ((uint)(ap->STA[0].stap) & ~(SD_TCWSIZE - 1)));

	d_cflush(ap->dma_channel, (char *)ap->STA[cmd->sta_index].stap,
		(size_t)SD_STA_SIZE, daddr);
	
	cmd->mbox_copy.mb8.fields.dma_addr = (uint) daddr;
	cmd->mb->mb8.fields.dma_addr = (uint) daddr;
	cmd->mbox_copy.mb8.fields.dma_length = (uint) 255;
	cmd->mb->mb8.fields.dma_length = (uint) 255;
        cmd->mbox_copy.tag = cmd->mb->tag;
        cmd->mbox_copy.nextmb = cmd->mb->nextmb;


	/*
	 * flush MB cache lines.
	 */

	eaddr = (caddr_t)cmd->mb;
	daddr = (caddr_t)SD_MB_ADDR((uint)ap->base_MB_dma_addr,
		(uint)cmd->mb->tag);
	d_cflush((int) (ap->dma_channel),eaddr,(int) SD_MB_SIZE,daddr);

   
	/* 
	 * write ap->last_tag to last tag reg;
	 */
	ap->last_tag = cmd->mb->tag;
	if (SD_PUTC(seg_base+SD_LTAG,ap->last_tag)) {
		ret_code = EIO;
		sd_free_MB(cmd,(char)SD_NOT_USED);
		sd_free_cmd(cmd);
	}
	return (ret_code);
}


/*
 *                                                                        
 * NAME: sd_dump_retry                                                    
 *                                                                        
 * FUNCTION: 
 *                                                                        
 *          Retry command with cmd->tag = tag SD_DUMP_RETRIES times,          
 *                                                                        
 * EXECUTION ENVIRONMENT:                                                 
 *                                                                        
 *      This routine must be called in the interrupt environment.  It     
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_cmd          - command jacket
 *                                                          
 * INPUTS: 
 *      tag      -  mail box tag of command to retry 
 *      dp       -  dasd to which we will dump
 *      seg_base -  base segment register
 *
 *
 * CALLED BY:
 *      sd_dump_read
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_free_MB                    sd_MB_alloc
  *     
 * EXTERNAL PROCEDURES CALLED:
 *      bcopy                         d_cflush                     
 *      BUSIO_PUTC                    d_complete
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:
 *      EIO    -  I/O error
 *      ENOMEM -  Unable to allocate resources
 */                                                                        

int sd_dump_retry(uchar tag,		  /* tag to be retried 		*/
		  struct sd_dasd_info *dp,/* effected DASD		*/
		  uint seg_base,	  /* base segment register 	*/
		  struct iovec *iovp)	   /* data being written 	 */
{
	struct sd_adap_info *ap;           /* dasd's adapter 		*/
	struct sd_cmd       *cmd;          /* command jacket 		*/
	int                 ret_code = 0;  /* return code 		*/
	caddr_t             daddr;         /* dma address 		*/
	caddr_t             eaddr;         /* effective dma address 	*/
	caddr_t             mb_addr;       /* dma address of mail box 	*/
	struct sd_mbox *nextmb = NULL;
	uchar mbtag  = 0x0;

	ap = dp->ap;
	cmd = dp->checked_cmd;
	if (cmd == NULL) {
		return(EIO);
	}

	cmd->type = SD_DASD_CMD;
	cmd->ap = ap;
	cmd->cp = dp->cp;
	cmd->dp = dp;
	if (sd_MB_alloc(cmd)) {
		sd_free_cmd(cmd);
		return(ENOMEM);
	}


	/* 
	 * fail dump if a command 
	 * has been retried too many times
	 */

	if (++(cmd->retry_count) > SD_DUMP_RETRIES)
		return(EIO);


	/*
	 * fill in cmd->mb from values in cmd->mbox_copy
	 */
	mbtag = cmd->mb->tag;
	nextmb = cmd->mb->nextmb;
	bcopy((char *)&(cmd->mbox_copy),(char *)cmd->mb,sizeof(struct sd_mbox));

	cmd->mb->tag = mbtag;
	cmd->mb->nextmb = nextmb;
	if (sd_TCW_alloc(cmd) == FALSE) {
		sd_free_MB(cmd, (char)SD_NOT_USED);
		sd_free_cmd(cmd);
		return(ENOMEM);
	}

	/* 
	 * Fill in dma fields of mail box 
	 */
	
	daddr = (caddr_t)(SD_DMA_ADDR(cmd->ap->dds.tcw_start_addr, 
		cmd->tcws_start) + ((uint)cmd->b_addr & (SD_TCWSIZE - 1)));
	cmd->mbox_copy.mb8.fields.dma_addr = (uint) daddr;
        cmd->mbox_copy.tag = cmd->mb->tag;
        cmd->mbox_copy.nextmb = cmd->mb->nextmb;

	d_master((int)ap->dma_channel,(int) cmd->dma_flags,(caddr_t)iovp->iov_base,
		 cmd->mb->mb8.fields.dma_length,
		 &dp->xmem_buf,daddr);


	/*
	 * flush MB cache lines.
	 */

	eaddr = (caddr_t)cmd->mb;
	daddr = (caddr_t)SD_MB_ADDR((uint)ap->base_MB_dma_addr,
		(uint)cmd->mb->tag);

	d_cflush((int) (ap->dma_channel),eaddr,(int) SD_MB_SIZE,daddr);
   
                
	/* 
	 * write ap->last_tag to last tag reg; 
	 */
	ap->last_tag = cmd->mb->tag;
	if (SD_PUTC(seg_base+SD_LTAG,ap->last_tag)) {
		ret_code = EIO;
		sd_free_MB(cmd,(char)SD_NOT_USED);
		sd_free_cmd(cmd);
	}

	return (ret_code);
}
	/*
	 * flush MB cache lines.
	 */



/*
 *                                                                       
 * NAME: sd_dumpend                                                      
 *                                                                       
 * FUNCTION: Notify end of dump, try to recover
 *
 *                                                                       
 *       Cleanup after the dump, reenable interrupts
 *                                                                       
 * EXECUTION ENVIRONMENT:                                                
 *                                                                       
 *      This routine must be called in the interrupt environment.  It    
 *      can not page fault and is pinned.
 *
 *    
 * (DATA STRUCTURES:) 
 *                    sd_adap_info    - adapter info structure
 *                    sd_ctrl_info    - controller info structure
 *                    sd_dasd_info    - dasd info structure
 *                    sd_cmd          - command jacket
 *                                                          
 * INPUTS: 
 *      dp       -  dasd to which we will prepare for dump
 *
 *
 * CALLED BY:
 *      sd_dump
 *
 * INTERNAL PROCEDURES CALLED:
 *      sd_start
 *     
 * EXTERNAL PROCEDURES CALLED:
 *      unlock_enable
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:
 *      nothing
 */                                                                       

void sd_dumpend(struct sd_dasd_info *dp)
{
	struct sd_adap_info   *ap;         /* adapter pointer 		*/
	int                   ret_code = 0;/* return code 		*/
	int			i;



	dp->ap->dumpdev = FALSE;
	dp->ap->status &= ~SD_SUSPEND;
	for (i=0; i < SD_ADAP_TBL_SIZE; i++) {
		ap = sd_adap_table[i];
		while (ap != NULL) { 
			sd_start(ap);
			ap = ap->hash_next;
		}
	}
	return;
}

