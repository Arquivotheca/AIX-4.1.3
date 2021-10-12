static char sccsid[] = "@(#)37	1.19  src/bos/kernext/disk/badiskb.c, sysxdisk, bos411, 9428A410j 2/11/91 13:03:43";

/*
 * COMPONENT_NAME: (SYSXDISK) Bus Attached Disk Device Driver
 *                            Bottom Half
 *
 * FUNCTIONS: ba_strategy(), ba_epow(), ba_intr(), ba_dump(), ba_get_specs(), 
 *            ba_get_ccs(), ba_get_dsb(), ba_get_pos(), ba_issue_cmd(),
 *            ba_log_error(), ba_process_error(), ba_sendeoi(), ba_eoirecov(),
 *            ba_piodelay(), ba_piodrcv(), ba_busystat(),
 *            ba_bstatrecov(), ba_piofunc(), ba_piorecov(),
 *            ba_start(), ba_start_nextio(), ba_dma_cleanup(), ba_piowxfer(),
 *            ba_waitirpt(), ba_cmdxfer(), ba_watch()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/adspace.h>
#include <sys/buf.h> 
#include <sys/ddtrace.h>
#include <sys/device.h> 
#include <sys/devinfo.h> 
#include <sys/dma.h> 
#include <sys/errids.h>
#include <sys/errno.h> 
#include <sys/except.h> 
#include <sys/intr.h> 
#include <sys/ioacc.h> 
#include <sys/ioctl.h> 
#include <sys/lockl.h> 
#include <sys/lvdd.h> 
#include <sys/malloc.h> 
#include <sys/param.h> 
#include <sys/pin.h> 
#include <sys/sysdma.h> 
#include <sys/sysmacros.h> 
#include <sys/timer.h> 
#include <sys/trchkid.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <sys/watchdog.h> 
#include "badiskdd.h" 

#ifdef DEBUG
#define TRCLNGTH  512 
#else
#define TRCLNGTH  32
#endif

#ifdef DEBUG
int	badebug = 0;         /* debug flag for print statements */
#endif

/*
 ****************  Variable Declarations  ****************************
 */

char	ba_count = 0;		   /* count of disks in system */
lock_t ba_lock = LOCK_AVAIL;	   /* code lock */
struct ba_info *ba_list[BA_NDISKS]; /* list of info structures */
struct ba_trace *ba_trctop = NULL;
struct ba_trace ba_trace[TRCLNGTH];
int ba_trcindex = 0;
struct cdt *ba_cdt = NULL;

/*
 ******  Interrupt Descriptions for Debug  *******
 */

static char	*bairps[16] = {
	"", "* SUCCESS ", "", "* ECC     ", "", "* RETRIES ", "* FORMATPT", 
	"* ECCRETRY", "* WARNING ", "* ABORT   ", "* RESET   ", "* DTR     ",
	"* FAILURE ", "* DMAERROR", "* CMDERROR", "* ATRERROR"}; 

/*
 ******  String declarations for Trace Buffer *****
 */

static char 	*stgyidone = "stgy IDONE";
static char 	*intridone = "intr IDONE";
static char 	*perridone = "perr IDONE";
static char 	*perrlrba  = "perr LRBA ";
static char 	*perrresid = "perr RESID";
static char 	*perrbfail = "perr BFAIL";
static char 	*perrretry = "perr RETRY";
static char 	*strtbrkup = "strt BRKUP";
static char 	*strtdslav = "strt DSLAV";
static char 	*strtnorm  = "strt NORM ";
static char 	*strtresid = "strt RESID";
static char 	*strtchain = "strt CHAIN";
static char 	*nextidone = "next IDONE";
static char 	*dmacdcomp = "dmac DCOMP";
static char 	*piowidone = "piow IDONE";


/*
 * NAME: ba_strategy
 *                                                                    
 * FUNCTION: Issues or queues I/O requests                     
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called from the process level through
 *	the devsw table, or can be called from the device driver level
 *	such as from ba_read/ba_write through uphysio. It must be
 *	pinned and cannot page fault.
 *                                                                   
 * (NOTES:) This routine validates an I/O request and if valid either
 *	starts the request or places it on the queue to be processed
 *      in turn.
 *
 * (RECOVERY OPERATION:) NONE - all errors are handled through iodone()
 *
 * (DATA STRUCTURES:) buf      - references transfer information 
 *                    ba_info  - manupulates transfer queue
 *
 * RETURNS: 0      - since no error condition is valid              
 */  

int	ba_strategy(
struct  buf *bp)                             /* pointer to buf structure */
{
	struct ba_info *ba;                  /* pointer to info structure */
	struct buf *nextbp;                  /* temporary buf pointer */
	dev_t devno;                         /* minor/major number */
	int	old_ilevel,                  /* old interrupt level */
                dev,                         /* minor */
                last_rba;                    /* last relative block address */
	char	errcode = 0,                 /* errno to return via iodone */
                zeroread = 0;                /* flag if zero read requested */

	if (bp != NULL) {
#ifdef DEBUG 
		BADBUG(("Entering ba_strategy %x %x %x %x %x\n", bp->b_dev, 
                bp, bp->b_flags, bp->b_blkno, bp->b_bcount));
#endif
		DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_STRATEGY, 0, bp->b_dev, bp, 
                bp->b_flags, bp->b_blkno, bp->b_bcount);
		devno = bp->b_dev;
	}
	while (bp != NULL) {
		dev = minor(bp->b_dev);     /* get minor number */
		if (dev >= BA_NDISKS)       /* if invalid dev, set error code*/
			errcode = ENXIO;
		else {
			ba = ba_list[dev];     /* get structure pointer */
			if ((ba == NULL) || (!ba->opened))  
				errcode = ENXIO; /* not inited or opened */
			else {
				/*
                                 *  make sure address is on 512 byte boundary 
                                 */
				if ((int) bp->b_bcount & 0x1ff)
					errcode = EINVAL;  
				else {
					last_rba = ba->ba_cfig.total_blocks-1;
					/*
                                         * make sure desired LBA is within 
					 * limit
                                         */
					if (bp->b_blkno > last_rba) {
                                                if(bp->b_blkno == 
							(last_rba+1)) {
							if (bp->b_flags & 
								B_READ) 
								/* read 
								 * starting
								 * at end of
								 * media is ok,
								 * just zero
								 * bytes read 
								 */
								zeroread=TRUE;
                                                 	else 
								/* write 
								 * starting
								 * at end
								 * of media is
								 * an error
								 */
                                                                errcode=ENXIO;
                                                } else
							/*
							 * I/O starting 
							 * beyond end of
							 * media is an error
							 */
							errcode = ENXIO; 
					} else {
                                                /* 
                                                 * Relocations not supported by
                                                 * the Bus Attached Disk 
                                                 */
						if (bp->b_options & 
							(HWRELOC | UNSAFEREL))
							errcode = EINVAL; 
					}
				} /* else good address and boundary */
			}  /* else good pointer and device open */
		} /* else valid dev */
		nextbp = bp->av_forw;               /* save next buf pointer */
                /*
                 * if we have determined this buf to be invalid
                 */
		if (errcode) {
			bp->b_flags |= B_ERROR;     /* set buf error flags */
			bp->b_error = errcode;      /* set buf error code */
			bp->b_resid = bp->b_bcount; /* set resid to bcount */
			old_ilevel = i_disable(ba->dds.intr_priority);
			BADTRC(stgyidone,(uint)bp);
			i_enable(old_ilevel); /* enable irpts*/
			iodone(bp);                 /* iodone for this buf */
			bp = nextbp;                /* set pointer to next */
			errcode = 0;                /* clear error code */
		} else {
                        /*
                         * if 0 bytes were requested or a read starting at
                         * the end of disk was requested, get rid of buf
                         */
			if (bp->b_bcount == 0 || zeroread) {
				bp->b_resid = bp->b_bcount;/* set resid */
				bp->b_flags &= ~B_ERROR;/* set flags= no err */
				bp->b_error = 0;        /* clear buf error */
				old_ilevel = i_disable(ba->dds.intr_priority);
				BADTRC(stgyidone,(uint)bp);
				i_enable(old_ilevel); /* enable irpts*/
				iodone(bp);            /* get rid of this buf*/
			} else
			 {
				/*
                                 * update statistics if read op 
                                 */
				if (bp->b_flags & B_READ) {
					if((ba->dds.byte_count += 
						bp->b_bcount) > 
						ba->dds.bucket_size) { 
						ba->dds.byte_count %= 
						ba->dds.bucket_size;
						ba->dds.bucket_count++;
					}
					/*
					 * Update IOSTAT read statistics
					 */
					ba->dkstat.dk_rblks += bp->b_bcount /
						BA_BPS;
				} else {
					/*
					 * Update IOSTAT write statistics
					 */
					ba->dkstat.dk_wblks += bp->b_bcount /
						BA_BPS;
				}
                                /*
                                 * disable interrupts to protect queue pointers
                                 */
				old_ilevel = i_disable(ba->dds.intr_priority);
				bp->av_forw = NULL;    /* set next to null*/
                                /*
                                 * if nothing is on the Q, start this request 
                                 */
				if (ba->head == NULL) {
					/*
					 * stop the idle timer
					 */
					w_stop(&ba->idle.watch); 
					/*
					 * set head, tail to this request
					 */
					ba->head = bp;
					ba->tail = bp;
					if (ba_start(ba)) { 
						bp->b_flags |= B_ERROR; 
						bp->b_error = EIO;   
						bp->b_resid = bp->b_bcount;
						BADTRC(stgyidone,(uint)bp);
						iodone(bp); /*err starting io*/
						ba->head = NULL;
						ba->tail = NULL;
						/*
						 * start the idle timer
						 */
						w_start(&ba->idle.watch); 
					}
				} else {
                                        /* 
                                         * queue not empty, so place this buf 
                                         * at end of queue
                                         */
					ba->tail->av_forw = bp;/* on end of q*/
					ba->tail = bp;     /* set tail to buf*/
				}
				i_enable(old_ilevel); /* enable irpts*/
			} /* else not zeroread or 0 bytecount */
			bp = nextbp;   /* set current buf pointer to next buf*/
		} /* else, no errors */
	} /* while(bp!=NULL) */
#ifdef DEBUG 
	BADBUG(("Exiting ba_strategy\n"));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_STRATEGY, 0, devno);
	return(0);
}


/*
 * NAME: ba_epow
 *                                                                    
 * FUNCTION: Exception Handler for Early Power Off Warning    
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by the system when it detects an
 *	Early Power Off Warning. It is called on the interrupt level,
 *	and runs at the INTMAX level. It can not page fault. 
 *                                                                   
 * (NOTES:) This routine performs one of the following options as specified
 *          in the flags field of the interrupt structure.
 *          EPOW_SUSPEND   - Suspends operation on the device by issueing
 *                           a Soft Reset and setting epow_pending flag
 *          EPOW_BATTERY   - Suspends operation only if the device is
 *                           not on battery backup
 *          EPOW_RESUME    - Resumes operation on the device by clearing 
 *                           epow_pending flag and starting any io on Q
 *                                                                   
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) intr    - interrupt structure       
 *                    ba_info - info structure
 *                    ba_list - list of info structure pointers
 *
 * RETURNS: INTR_SUCC  upon completion                              
 */  

int	ba_epow(
struct  intr *is)                     /* pointer to interrupt structure */
{
	int	i,                    /* counter */
                old_level;            /* save previous interrupt level */
	struct ba_info *ba;           /* pointer to info structure */

#ifdef DEBUG
	BADBUG(("Entering ba_epow routine\n"));
#endif
	old_level = i_disable(INTCLASS2);       /* disable my irpt level */
	if (is->flags & EPOW_SUSPEND) {
		/*
		 * if epow battery flag is not set, or it is set but the
		 * device is not on battery backup, then suspend operation
		 */
		if ((!(is->flags & EPOW_BATTERY)) || 
		((is->flags & EPOW_BATTERY) && (!ba->dds.battery_backed))) { 
			/*
			 * for each disk in the system
			 */
			for (i = 0; i < BA_NDISKS; i++) {
				ba = ba_list[i];    /* get pointer to struct */
				if (ba != NULL) {   /* issue soft reset */
					ba->epow_pend = TRUE; /*set pend flag*/
					ba->pio_address = ba->dds.base_address
						+ BA_ATR;
					ba->pio_data = BA_SOFT_RESET | 
						(BA_ATTACH >> BA_1BYTE);
					ba->pio_size = BA_1BYTE;
					ba->pio_op = BA_PIO_W;
					pio_assist((caddr_t)ba, ba_piofunc, 
						ba_piorecov);
				} /* if ba != NULL */
			} /* for loop */
		} /* if battery or not backed */
	} else {
		/*
		 * if epow battery flag set but not on battery backup
		 * then suspend operation 
		 */
		if ((is->flags & EPOW_BATTERY) && (!ba->dds.battery_backed)) {
			/*
			 * for each disk in system
			 */
			for (i = 0; i < BA_NDISKS; i++) {
				ba = ba_list[i];   /* get pointer info struct*/
				if (ba != NULL) {  /* issue soft reset */
					ba->epow_pend = TRUE;
					ba->pio_address = ba->dds.base_address
						+ BA_ATR;
					ba->pio_data = BA_SOFT_RESET | 
						(BA_ATTACH >> BA_1BYTE);
					ba->pio_size = BA_1BYTE;
					ba->pio_op = BA_PIO_W;
					pio_assist((caddr_t)ba, ba_piofunc, 
						ba_piorecov);
				} /* if ba != NULL */
			} /* for loop */
		} else /* not suspend or battery */
		 {
			/*
			 * if epow resume then clear epow pending flag and
			 * start whatever is on the I/O queue
			 */
			if (is->flags & EPOW_RESUME) {
				for (i = 0; i < BA_NDISKS; i++) {
					ba = ba_list[i];  
					if ((ba != NULL) && ba->epow_pend) {
						ba->epow_pend = FALSE;
						ba_start_nextio(ba); 
					} /* if ba != null and epow pending */
				} /* for loop */
			} /* if flags and epow resume */
		} /* else not suspend or battery */
	} /* else not suspend */

	i_enable(old_level);                         /* restore interrupts */
	return(INTR_SUCC);
}


/*
 * NAME: ba_intr
 *                                                                    
 * FUNCTION: Interrupt Handler for Bus Attached Disk          
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is invoked by the first level kernel interrupt
 *	handler. It runs at the interrupt level and can not page fault.
 *                                                                   
 * (NOTES:) This routine first determines if the Bus Attached Disk
 *	caused the interrupt, and if so performs the necessary actions
 *      corresponding to the interrupt condition, and then clears the
 *      interrupt.
 *
 * (RECOVERY OPERATION:) If a bus failure occurs while attempting to
 *	read the BSR (i.e. we aren't sure if it is our interrupt), 
 *      INTR_FAIL is returned and if it was ours we will be called to
 *      try again. If a bus failure occurs while attempting to read the
 *      ISR (i.e. we know its ours, but can't tell what its for ), then
 *      INTR_SUCC is returned and we rely on the watchdog timer to pop
 *      and retry the command. If an attachment error is detected, then
 *      INTR_SUCC is returned and we rely on the watchdog timer to pop
 *      and retry the command.
 *
 * (DATA STRUCTURES:) intr    - interrupt structure   
 *                    ba_info - info structure
 *                    buf     - buf structure for data transfer
 *
 *
 * RETURNS: INTR_SUCC    -  Successful completion of interrupt       
 *          INTR_FAIL    -  Not my interrupt, or can't tell if it is
 */  

int	ba_intr(
struct  intr *is)                    /* pointer to interrupt structure */
{
	struct ba_info *ba;          /* pointer to info structure */
	struct buf *tmp_head;        /* temporary buf pointer */
        int     drc,i=0;             /* return code from ba_dma_cleanup, cnt */
	uint	base;                /* base address */
	uchar	status,              /* value in BSR */
              	istat,               /* value in ISR */
              	cause;               /* cause of interrupt */
	char	dtr = FALSE,         /* data transfer ready flag */
		send_eoi = TRUE,     /* send end of interrupt flag */
		timer_active = TRUE; /* timer has been started flag */

#ifdef DEBUG 
	BADBUG(("Entering ba_intr %x\n", ba));
#endif
	ba = (struct ba_info *)is;            /* get pointer to my structure */
	DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_INTR, 0, ba->devno, 0, 0, 0, 0);
	if (!ba_count) {
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_FAIL, ba->devno);
#ifdef DEBUG 
		BADBUG(("Exiting 1 ba_intr : INTR_FAIL\n"));
#endif
		return(INTR_FAIL);       /* no bus attached disks in system */
	}
	if (ba == NULL) {
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_FAIL, ba->devno);
#ifdef DEBUG 
		BADBUG(("Exiting 2 ba_intr : INTR_FAIL\n"));
#endif
		return(INTR_FAIL);       /* disk not inited */
	}
	base = ba->dds.base_address;     /* get base address of this device */
	ba->pio_address = base + BA_BSR; /* read status register */
	ba->pio_size = BA_1BYTE;
	ba->pio_op = BA_PIO_R;
	/* 
	 * if bus error of some type while reading status register
	 * then return INTR_FAIL, because it may or may not have been
	 * our interrupt, if it was we will be called again 
         */
	if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov)) { 
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_FAIL, ba->devno);
#ifdef DEBUG 
		BADBUG(("Exiting 3 ba_intr : INTR_FAIL\n"));
#endif
		return(INTR_FAIL);               /* PIO failure */
	}
	status = (uchar)ba->pio_data;            /* store status */
	if (status & BA_IRPT) {
		ba->pio_address = base + BA_ISR; /* read Irpt Status Register*/
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_R;
		/* 
		 * we know it was ours, so if a bus failure occurs while
		 * reading the Interrupt Status Register, return INTR_SUCC
		 * and rely on the watchdog timer to pop and retry command
		 */
		if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov)) {
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_SUCC, 
				ba->devno);
#ifdef DEBUG 
			BADBUG(("Exiting 4 ba_intr : INTR_SUCC\n"));
#endif
			i_reset((struct intr *)ba);   /* reset bus interrupt */
			/*
			 * send end of interrupt
			 */
			pio_assist((caddr_t)ba, ba_sendeoi, ba_eoirecov);
			return(INTR_SUCC);
		} /* if pio_assist failure */
		istat = (uchar)ba->pio_data;       /* store interrupt status */
		/*
		 * check the attachment error bit of the ISR, if it is set
		 * then reset the interrupt, send end of interrupt, and
		 * return INTR_SUCC. The watchdog timer will pop and retry
		 * whatever command failed.
		 */
		if (istat & BA_ATTACH_ERROR) {
#ifdef DEBUG
			BADBUG(("ATTACHMENT ERROR\n"));
#endif
			i_reset((struct intr *)ba);  /* reset bus interrupt */
			/*
			 * send end of interrupt
			 */
			pio_assist((caddr_t)ba, ba_sendeoi, ba_eoirecov);
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_SUCC, 
				ba->devno);
#ifdef DEBUG 
			BADBUG(("Exiting 5 ba_intr : INTR_SUCC\n"));
#endif
			return(INTR_SUCC);          /* return INTR_SUCC */
		} /* if istat and attach error */
		ba->atre = FALSE;                   /* clear ATR flag */
		cause = istat & BA_IRPT_CAUSE;      /* mask to get cause */
		ba->lint_dev = istat & BA_DEV_MASK; /* get device */
		if ( cause != BA_DTR )
 			/*
			 * stop watchdog timer only if this is not a
			 * DTR interrupt, because the transfer is not
			 * complete if this is a DTR
			 */
			w_stop(&ba->mywdog.watch); 
		i_reset((struct intr *)ba);         /* reset bus interrupt */
		ba->blk.reg = BA_PIO_R;             /* set reg flag to SIR */
		ba->blk.current = 0;                /* set current word to 0 */
		ba->piowtrb->func_data = (ulong)ba; /* store pointer */
#ifdef DEBUG
		BADBUG(("%s\n", bairps[cause]));
#endif
		BADTRC(bairps[cause],(uint)ba);
		/* 
		 * if dma transfer was initiated, finish it
		 */
		if ((ba->dma_active) && (cause != BA_DTR)) {
			drc = ba_dma_cleanup(ba);
		}
		switch (cause) {
		case BA_COMP_SUCC_ECC :  
		case BA_COMP_S_E_R :   
   			/* 
			 * complete with success with ECC applied or,
			 * complete with success with ECC and retries 
			 */
			ba->erp = BA_LOG;     /* set error recovery to log */
			ba->etype = BA_SE;    /* set error type to soft error*/
		case BA_COMP_SUCC_RET : 
		case BA_COMP_SUCC :  
	   		/* 
			 * complete with success with retries or
			 * complete with success
			 */
			switch (ba->last_cmd) {
			case BA_READ_DATA:
			case BA_WRITE_DATA:
			case BA_WRITE_VERIFY:
				/*
				 * the last command was a read or write
				 */
				if (drc == DMA_SUCC) {
					if ((!ba->chained) && (ba->target_xfer 
						- (ba->byte_count / BA_BPS)))
						/*
						 *  if more to send for this 
						 *  buf update bytes sent
						 */
						ba->bytes_sent+=ba->byte_count;
					else {
						/* 
						 * this buf request is 
						 * complete
						 */
						ba->retry_count = 0; 
						ba->bytes_sent = 0; 
						ba->buf_complete = TRUE;
					}
				} else
					/* 
					 * something wrong with dma, so 
					 * silently retry this transfer
					 */
					ba->erp = BA_SILENT_RETRY; 
				timer_active = FALSE;   /* didnt start timer */
				break;
			case BA_SEEK_CMD:
				timer_active = FALSE;   /* didnt start timer */
				ba->erp = BA_NONE;/* no error recovery needed*/
				if (ba->idle_seek_inprog) {
					ba->idle_seek_inprog = FALSE;
					ba->idle_seek_cmpl = TRUE;
				}
				break;
			case BA_GET_DEV_STAT:
			case BA_GET_CC_STAT:
			case BA_TRANS_RBA:
				/* 
				 * the last command was either a get device
				 * status, a get complete status, or a 
				 * translate relative block address, so 
				 * start timer to read the status block
				 */
				ba->erp = BA_NONE;/* no error recovery needed*/
				ba->blk.wc = 7;   /* set # wrds to read to 7 */
				/* 
				 * save address of store routine 
				 */
				ba->blk.store = (void(*)())ba_get_ccs;
				ba_piowxfer(ba->piowtrb);/* "jump start" xfer*/
				send_eoi = FALSE;   /* not finished with irpt*/
				break;
			case BA_GET_DIAG_STAT:
				/*
				 * the last command was get diagnostic status
				 * block so start timer to read the block
				 */
				ba->erp = BA_NONE;/* no error recovery needed*/
				ba->blk.wc = 7;   /* set # wrds to read to 7 */
				/* 
				 * save address of store routine 
				 */
				ba->blk.store = (void(*)())ba_get_dsb;
				ba_piowxfer(ba->piowtrb);/* "jump start" xfer*/
				send_eoi = FALSE;   /* not finished with irpt*/
				break;
			case BA_GET_DEV_CONFIG:
				/*
				 * the last command was get device config
				 * so start timer to read the data
				 */
				ba->erp = BA_NONE;/* no error recovery needed*/
				ba->blk.wc = 6;   /* set # wrds to read to 6 */
				/* 
				 * save address of store routine
				 */
				ba->blk.store = (void(*)())ba_get_specs;
				ba_piowxfer(ba->piowtrb);/* "jump start" xfer*/
				send_eoi = FALSE;   /* not finished with irpt*/
				break;
			case BA_GET_POS_INFO:
				/*
				 * the last command was get POS information
				 * so start timer to read info
				 */
				ba->erp = BA_NONE;/* no error recovery needed*/
				ba->blk.wc = 5;   /* set # wrds to read to 5 */
				/* 
				 * save address of store routine
				 */
				ba->blk.store = (void(*)())ba_get_pos;
				ba_piowxfer(ba->piowtrb);/* "jump start" xfer*/
				send_eoi = FALSE;   /* not finished with irpt*/
				break;
			default:
				/* 
				 * some other command
				 */
				ba->erp = BA_NONE; /* no err recovery defined*/
				ba->format_count = 0; /* clear cylinder count*/
				timer_active = FALSE; /* did not start timer */
				break;
			} /* switch last command */
			/*
			 * Clear the device busy flag of the IOSTAT
			 * status 
			 */
			ba->dkstat.dk_status &= ~IOST_DK_BUSY;
			break;
		case BA_ATTEN_ERROR   : 
			ba->atre = TRUE;     /* set Error writing to ATR flag*/
		case BA_DMA_ERROR     :      /* something bad wrong with DMA */
		case BA_COMMAND_ERROR :      /* command block error */
			/*
			 * interrupt was caused by either an attention error,
			 * command error, or DMA error (retry 4) , so set 
			 * error recovery to log-retry1time, and error type 
			 * to Attachment * error
			 */ 
			if (cause == BA_DMA_ERROR)
				ba->erp = BA_LR4; 
			else
				ba->erp = BA_LR1; 
			ba->etype = BA_AE;
			ba->blk.wc = 7;  /* set # word to read to 7 */
			/*
			 * save address of store routine
			 */
			ba->blk.store = (void(*)())ba_get_ccs;
			ba_piowxfer(ba->piowtrb);   /* "jump start" xfer */
			send_eoi = FALSE;           /* not finished with irpt*/
			ba->format_count = 0;       /* clear cylinder count */
			/*
			 * Clear the device busy flag of the IOSTAT
			 * status
			 */
			ba->dkstat.dk_status &= ~IOST_DK_BUSY;
			break;
		case BA_FORMAT_PCOMP:            
			/* 
			 * periodic format interrupt - each cylinder
			 */
			ba->erp = BA_NONE;       /* no error recovery needed */
			ba->format_count++;      /* increment cylinder count */
			/*
			 * read the status interface register to clear the
			 * STAT_OUT bit       
			 */
			ba->pio_address = base + BA_SIR; 
			ba->pio_size = BA_1WORD;
			ba->pio_op = BA_PIO_R;
			if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov))
				/*
				 * if bus error, reading SIR, call our
				 * watchdog routine to abort and retry
				 */
				ba_watch((struct watchdog *)ba);
			send_eoi = FALSE;      /* dont send end of interrupt */
			timer_active = FALSE;  /* did not start timer */
			break;
		case BA_ABORT_COMP :
		case BA_RESET_COMP :
			/*
			 * either ABORT or RESET complete
			 */
			if (!ba->l_waitcc)   
				/*
				 * if error recovery is not waiting on an 
				 * abort or reset to complete then no error
				 * recovery is needed.
				 */
				ba->erp = BA_NONE;  
			else 
				/* 
				 * if error recovery is waiting for abort or
				 * reset to complete then set error recovery
				 * procedure to log-retry1time
				 */
				ba->erp = BA_LR1; 
			if (ba->w_waitcc) {
				/* 
				 * if watchdog routine is waiting for an abort
				 * or reset to complete, then set error 
				 * recovery to silent retry and clear flag 
				 */
				ba->erp = BA_SILENT_RETRY; 
				ba->w_waitcc = FALSE;     
			}
			timer_active = FALSE;    /* we did not start a timer */
			break;
		case BA_DTR  :                               
			/* 
			 * ready for data transfer 
			 */
			ba->erp = BA_NONE;    /* no error recovery needed */
			dtr = TRUE;           /* set data transfer ready flag*/
			/* 
			 * enable DMA on the device 
			 */
			ba->pio_address = base + BA_BCR;
			ba->pio_data = BA_DMA_ENABLE | BA_INT_ENABLE;
			ba->pio_size = BA_1BYTE;
			ba->pio_op = BA_PIO_W;
			if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov))
				/* 
				 * if pio fails then call watchdog routine
				 * to abort and retry transfer
				 */
				ba_watch((struct watchdog *)ba);  
			timer_active = FALSE;  /* we did not start a timer */
			send_eoi = FALSE;      /* don't send end of interrupt*/
			break;
		case BA_COMP_FAIL :          
		case BA_COMP_WARN :
			/* 
			 * The command completed with Failure or Warning,
			 * but we need more information to process the error
			 * so start timer to read the command complete 
			 * status block
			 */
			ba->erp = BA_MORE_INFO; 
			ba->blk.wc = 7;          /* set # words to read to 7 */
			/* 
			 * save address of store routine
			 */
			ba->blk.store = (void(*)())ba_get_ccs; 
			ba_piowxfer(ba->piowtrb);   /* "jump start" xfer */
			send_eoi = FALSE;           /* not finished with irpt*/
			ba->format_count = 0;       /* clear cylinder count */
			/*
			 * Clear the device busy flag of the IOSTAT
			 * status
			 */
			ba->dkstat.dk_status &= ~IOST_DK_BUSY;
			break;
		} /* switch cause */
		if (((cause == BA_COMP_SUCC) || (cause == BA_COMP_SUCC_RET)) 
			&& (ba->erp != BA_SILENT_RETRY)) {   
			/* 
			 * if the cmd completed successfully, and we don't 
			 * need to retry silently
			 */
			ba->erp = BA_NONE;  /* clear error recovery procedure*/
			ba->etype = 0;      /* clear error type */
		}
		if (ba->erp == BA_LOG) { 
		        /* 
			 * if error recovery procedure is set to log, then 
			 * start a timer to read status block 
			 */
			ba->blk.wc = 7;         /* set # words to read to 7 */
			/*
			 * save address of store routine
			 */
			ba->blk.store = (void(*)())ba_get_ccs;  
			ba_piowxfer(ba->piowtrb);   /* "jump start" transfer */
			send_eoi = FALSE;           /* not finished with irpt*/
			timer_active = TRUE;        /* we DID start a timer */
		}
		if (send_eoi) {
			/*
			 * if we need to send end of interrupt now then
			 * send eoi and set command complete flag
			 */
			pio_assist((caddr_t)ba, ba_sendeoi, ba_eoirecov);
			ba->command_cmpl = TRUE;   
		}
		if (!timer_active) {
			/*
			 * if we haven't started a timer
			 */
			if (ba->erp) {
				/* 
				 * if error recovery is needed
				 */
				if (!ba->diagnos_mode) {
					/* 
					 * if not in diagnostic mode,
					 * then process the error
					 */
					ba_process_error(ba);
				} else {
					/*
					 * No logging or retrying of errors
					 * in diagnostic mode, so clear error
					 * and check if result of read/write
					 */
					ba->erp = BA_NONE;
					ba->etype = 0;
					if (ba->rwop) {
						/*
						 * if read/write then fail
						 * this buf and start the
						 * next I/O on the Q
						 */
						tmp_head = ba->head; 
						ba->head = ba->head->av_forw;
						tmp_head->b_flags |= B_ERROR;
						tmp_head->b_error = EIO;
						tmp_head->b_resid = 
							tmp_head->b_bcount;
						BADTRC(intridone,
							(uint)tmp_head);
						iodone(tmp_head);
						ba->reloc = FALSE;
						ba_start_nextio(ba);
					} /* if rwop */
				} /* else diagnos mode */
			} else {
				/*
				 * else no error recovery is needed so
				 * check if read/write
				 */
				if (ba->rwop) {
					/*
					 * it was a read or write, so check
					 * the status of the transfer
					 */
					if (ba->buf_complete) {
					   /*
					    * if this buf is complete
					    * then dequeue it and get 
					    * ready to start next I/O
					    * on queue
					    */
					   ba->buf_complete = FALSE; 
					   if (ba->chained == 0)
					   	ba->chained = 1;
					   for(i=0;i<ba->chained; i++){
						tmp_head = ba->head;
						ba->head = ba->head->av_forw;
						if (ba->reloc) {
							/*
							 * if we successfully
							 * got the data, but
							 * only after retrying
							 * then set error to
							 * ESOFT
							 */
							tmp_head->b_flags |= 
								B_ERROR;
							tmp_head->b_error = 
								ESOFT;
							tmp_head->b_resid =
								ba->resid;
							ba->resid = 0;
							ba->reloc = FALSE;
						} else {
							/*
							 * else we didn't need
							 * any retries then
							 * set no error
							 */
							tmp_head->b_flags &= 
								~B_ERROR;
							tmp_head->b_error = 0;
						}
						BADTRC(intridone,
							(uint)tmp_head);
					  	iodone(tmp_head);
					   }
					   ba_start_nextio(ba);
					} else {
						/*
						 * else this buf if not 
						 * complete so make sure irpt
						 * wasn't a data transfer ready
						 */
						if (!dtr) {
							/*
							 * if not a data 
							 * transfer ready, then
							 * start next I/O on Q
							 */
							ba_start_nextio(ba);
						}/* else dtr */
					}/* else more to transfer */
				} else  /* else rwop */
					/* 
					 * wasn't a rwop, but if it was a 
					 * seek, try and start any IO
					 */
					if (ba->idle_seek_cmpl) {
						ba->idle_seek_cmpl = FALSE;
						ba_start_nextio(ba);	
					}
			} /* else no error */
		} /* else timer_active */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_SUCC, ba->devno);
#ifdef DEBUG 
		BADBUG(("Exiting 6 ba_intr : INTR_SUCC\n"));
#endif
		return(INTR_SUCC);  /* return successful completion of INTR */
	} else {
		/*
		 * else NOT MY INTERRUPT, so return INTR_FAIL   
		 */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_INTR, INTR_FAIL, ba->devno);
#ifdef DEBUG 
		BADBUG(("Exiting 7 ba_intr (NMI) : INTR_FAIL\n"));
#endif
		return(INTR_FAIL); 
	}
}


/*
 * NAME: ba_dump  
 *                                                                    
 * FUNCTION: Perform a System Dump to the Bus Attached Disk   
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine should be considered as part of the interrupt
 *	level. It is called by the system dump when used as a dump
 *	device. This routine must not rely on interrupts or any system
 *	services, and can not page fault.
 *                                                                   
 * (NOTES:) This routine handles the following operations :   
 *	DUMPINIT   - initializes bus attached disk as dump device
 *      DUMPSTART  - prepares device for dump
 *      DUMPQUERY  - returns the maximum and minimum number of bytes than
 *                   can be transferred in a single DUMPWRITE command
 *      DUMPWRITE  - performs write to disk
 *      DUMPEND    - cleans up device on end of dump
 *      DUMPTERM   - terminates the bus attached disk as dump device
 *                                                                   
 * (RECOVERY OPERATION:) If an error occurs, the proper errno is returned
 *	and the caller is left responsible to recover from the error.
 *
 * (DATA STRUCTURES:) uio       - structure containing information about the
 *                                data to transfer 
 *                    ba_info   - info structure
 *                    dmp_query - queried transfer information is returned
 *                    xmem      - cross memory descriptor
 *
 * RETURNS: ENXIO     - Minor number out of range                       
 *                    - Device not inited
 *                    - DUMPWRITE issued when device not dump device
 *          EINVAL    - transfer length not multiple of 512
 *                    - invalid operation
 *          ENOSPC    - desired block past end of media
 *          ENOTREADY - device timeout
 *          EIO       - I/O error
 */  

int	ba_dump(
dev_t   devno,                   /* Major/Minor numbers */    
struct  uio *uiop,               /* pointer to uio data */
int	cmd,                     /* command to perform */
char	*arg,                    /* pointer to dmp_query structure */
int	chan,                    /* channel unused */
int	ext)                     /* extended parameter unused */
{
	struct ba_info *ba;      /* pointer to info structure */
	struct dmp_query *dpr;   /* pointer to dmp_query structure */
	struct iovec *iovp;      /* pointer to iovec (struct within uio) */
	struct xmem txmem;       /* temporary xmem structure for xmem descrip*/
	int	i = 0,           /* counter */
		dev,             /* minor number */
		rc = OK,         /* return code */
		rba,             /* relative block address for transfer */
		last_rba;        /* last data block address on the disk */
	uint	iocc_seg,        /* iocc segment register value */
		seg_base;        /* base segment register value */
	ushort 	info;            /* place to store 16 bit word */
	uchar 	status,          /* value of BSR */
		istat,           /* value of ISR */
		cause;           /* cause of interrupt */

#ifdef DEBUG 
	BADBUG(("Entering ba_dump %x %x %x %x %x %x\n", devno, uiop, cmd, arg,
		chan, ext));
#endif
	dev = minor(devno);                 /* get minor number */
	if (dev >= BA_NDISKS) {
#ifdef DEBUG 
		BADBUG(("Exiting ba_dump  1 ENXIO\n"));
#endif
		return(ENXIO);              /* minor number out of range */
	}
	ba = ba_list[dev];                  /* get pointer to info structure */
	if (ba == NULL) {
#ifdef DEBUG 
		BADBUG(("Exiting ba_dump 2  ENXIO\n"));
#endif
		return(ENXIO);              /* device not inited */
	}
	/*
	 * reserve segment registers for iocc and bus transfers
	 */
	iocc_seg = (uint)IOCC_ATT(ba->dds.bus_id, BA_DELAY_REG ); 
	seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, ba->dds.base_address);
	switch (cmd) {
	case DUMPINIT:  
		/* 
		 * since code was pinned at open time, nothing to do 
		 */
#ifdef DEBUG 
		BADBUG(("DUMPINIT\n"));
#endif
		break;
	case DUMPSTART:
		/* 
		 * prepare device for dump, let existing io finish, depending 
		 * only on the device status, then disable interrupts, and go 
		 * to a polling mode for all data transfers
		 */ 
#ifdef DEBUG 
		BADBUG(("DUMPSTART\n"));
#endif
		i = 0;
		status = BUSIO_GETC(seg_base + BA_BSR);   /* read status */
		/* 
		 * while device is busy, delay 1 microsecond at a time
		 * by writing to the hardware delay register 
		 */
		while (((status & BA_BUSY) || (status & BA_CIPS)) && 
			(++i < 1000000)) {
			BUSIO_PUTC(iocc_seg, BA_ZERO);
			status = BUSIO_GETC(seg_base + BA_BSR); 
		}
		/*
		 * disable interrupts at the INTMAX level
		 */
		ba->dumpdev = TRUE;
		ba->dump_ilevel = i_disable(INTMAX);  
		if ((status & BA_BUSY) || (status & BA_CIPS)) {
			/*
			 * if device not quiet, quiesce
			 */
			BUSIO_PUTC(seg_base + BA_ATR, BA_SOFT_RESET | 
				(BA_ATTACH >> BA_1BYTE));
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);  
			/* 
			 * wait for interrupt, by polling
			 * at 1 microsecond intervals 
		 	 */
			while ((!(status & BA_IRPT)) && (++i < 10000000)) {
				BUSIO_PUTC(iocc_seg , BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR); 
			}
			istat = BUSIO_GETC(seg_base + BA_ISR); /*read ISR*/
			/*
			 * send end of interrupt 
			 */
			BUSIO_PUTC(seg_base + BA_ATR, ((BA_ATTACH >> BA_1BYTE) 
				| BA_EOI));
			/* 
			 * read 16 bit word from Status Interface register
			 * to clear the STAT_OUT bit of the BSR
			 */
			info = BUSIO_GETS(seg_base + BA_SIR); 
		}
		break;
	case DUMPQUERY:
		/* 
		 * Query the maximum and minimum number of bytes that can 
		 * be transferred to the device in one DUMPWRITE command.
		 */ 
#ifdef DEBUG 
		BADBUG(("DUMPQUERY\n"));
#endif
		dpr = (struct dmp_query *)arg;/* pointer to dump query struct*/
		dpr->min_tsize = BA_BPS;      /* set min size to 1 sector */
		dpr->max_tsize = DMA_MAX;     /* set max size to max dma xfer*/
		break;
	case DUMPWRITE:
		/*
		 * perform write of dump data to disk
		 */
#ifdef DEBUG 
		BADBUG(("DUMPWRITE\n"));
#endif
		if (!ba->dumpdev) {
			rc = ENXIO;   /* not inited as dump device */
			break;
		}
		/*
		 * Take the uio structure and run with it 
		 */
		iovp = uiop->uio_iov;   /* point to current iov structure */
		for (i = 0; i < uiop->uio_iovcnt; i++) {
			/*
			 * for each uio struct specified by iovcnt
			 */
			if ((int)iovp->iov_len & 0x1ff) { 
				/*
				 * if length not multiple of 512, return error
				 */
				rc = EINVAL;
				break;
			}
			/*
			 * compute last relative block address and
			 * the target relative block address
			 */
			last_rba = ba->ba_cfig.total_blocks - 1; 
			rba = uiop->uio_offset / BA_BPS;
			if (rba > last_rba) {
				/*
				 * the target RBA is past the end of media
				 */
				rc = ENOSPC;   
				break;
			}
			txmem.aspace_id = XMEM_GLOBAL; /* set up xmem descrip*/
			/*
			 *  start 3rd party dma transfer 
			 */
			d_slave((int)ba->dma_chn_id, (int)0, 
				(char *)iovp->iov_base, 
				(size_t)iovp->iov_len, 
				(struct xmem *)&txmem);
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);   
			/* 
			 * while device is busy or a command is in progress
			 * delay by writing to the 1 microsecond delay register
			 */
			while (((status & BA_BUSY) || (status & BA_CIPS)) && 
				(++i < BA_DUMPTO200)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR);
			}
			/* 
			 * request the device to accept command 
			 */
			BUSIO_PUTC(seg_base + BA_ATR, BA_ACCEPT_CMD | 
				(BA_DISK >> BA_1BYTE));
			/* 
			 * wait for device response polling at 1 microsecond
			 * intervals
			 */
			/*
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);  
			i = 0;                         
			while ((!(status & BA_BUSY)) && (++i < BA_DUMPTO10)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO); 
				status = BUSIO_GETC(seg_base + BA_BSR); 
			}
			if (!(status & BA_BUSY)) {
				/*
				 * device still hasn't responded, so timeout
				 */
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t) iovp->iov_len, 
					(struct xmem *)&txmem, (char *)NULL);
				break;
			}
			/* 
			 * write word 1 of 4 of the command block 
			 */
			BUSIO_PUTS(seg_base + BA_CIR, (BA_DISK | 
				BA_WRITE_DATA | BA_FOUR_WRDS | BA_OPTIONS));
			i = 0;                                    /* clear i */
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);   
			/*
			 * Delay 1 microsecond at a time until the device is
			 * ready to accept the next command word 
			 */
			while ((status & BA_CMD_IN) && (++i < BA_DUMPTO450)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR);  
			}
			/*
			 * If device still not ready for next command word
			 * then timeout 
			 */
			if (status & BA_CMD_IN) {
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id, (int) 0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, (char *)NULL);
				break;
			}
			/* 
			 * write word 2 of 4 of the command block 
			 */
			BUSIO_PUTS(seg_base + BA_CIR, 
				(BA_SWAPB((iovp->iov_len / BA_BPS))));
			i = 0;                                /* clear i */
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);   
			/*
			 * Delay 1 microsecond at a time until the device is
			 * ready to accept the next command word 
			 */
			while ((status & BA_CMD_IN) && (++i < BA_DUMPTO450)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR);  
			}
			/*
			 * If device still not ready for next command word
			 * then timeout 
			 */
			if (status & BA_CMD_IN) {
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, (char *)NULL);
				break;
			}
			/* 
			 * write word 3 of 4 the command block 
			 */
			BUSIO_PUTS(seg_base + BA_CIR, 
				(BA_SWAPB((ushort)(rba & BA_LWMASK))));
			i = 0;                                 /* clear i */
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);    
			/*
			 * Delay 1 microsecond at a time until the device is
			 * ready to accept the next command word 
			 */
			while ((status & BA_CMD_IN) && (++i < BA_DUMPTO450)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR);  
			}
			/*
			 * If device still not ready for next command word
			 * then timeout 
			 */
			if (status & BA_CMD_IN) {
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, (char *)NULL);
				break;
			}
			/* 
			 * write word 4 of 4 of the command block 
			 */
			BUSIO_PUTS(seg_base + BA_CIR, 
			 (BA_SWAPB ((ushort)((rba >> BA_1WORD) & BA_LWMASK))));
			i = 0;                               /* clear i */
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);  
			/* 
			 * wait for Data Transfer Ready interrupt, by polling
			 * at 1 microsecond intervals 
		 	 */
			while ((!(status & BA_IRPT)) && (++i < 10000000)) {
				BUSIO_PUTC(iocc_seg , BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR); 
			}
			if (!(status & BA_IRPT)) {
				/*
				 * if still no interrupt, then timeout 
				 */
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, (char *)NULL);
				break;
			}
			istat = BUSIO_GETC(seg_base + BA_ISR); /*read ISR*/
			cause = istat & BA_IRPT_CAUSE;     /* get cause */
			if (cause != BA_DTR) {
				/*
				 * if the interrupt was not a Data Transfer
				 * Ready, then something wrong so exit
				 */
				rc = EIO;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, 
					(char *)NULL);
				break;
			}
			/* 
			 * It was a DTR so enable DMA on device 
			 */
			BUSIO_PUTC(seg_base + BA_BCR, (char)(BA_DMA_ENABLE
				| BA_INT_ENABLE));
			i = 0;                                /* clear i */
			/* 
			 * read status 
			 */
			status = BUSIO_GETC(seg_base + BA_BSR);   
			/* 
			 * wait for command complete interrupt by polling
			 * at 1 microsecond intervals
			 */
			while ((!(status & BA_IRPT)) && (++i < 10000000)) {
				BUSIO_PUTC(iocc_seg, BA_ZERO);
				status = BUSIO_GETC(seg_base + BA_BSR); 
			}
			if (!(status & BA_IRPT)) {
				/* 
				 * if still no interrupt, then timeout
				 */
				rc = ENOTREADY;
				d_complete((int)ba->dma_chn_id,(int) 0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, 
					(char *)NULL);
				break;
			}
			istat = BUSIO_GETC(seg_base + BA_ISR); /*read ISR*/
			cause = istat & BA_IRPT_CAUSE;     /* get cause */
			if ((cause != BA_COMP_SUCC) && (cause != 
				BA_COMP_SUCC_ECC) && (cause != 
				BA_COMP_SUCC_RET) && (cause != BA_COMP_S_E_R)){
                         	/* 
				 * if command did not complete successfully, 
				 * set error, exit 
				 */
				rc = EIO;
				d_complete((int)ba->dma_chn_id, (int)0, 
					(char *)iovp->iov_base, 
					(size_t)iovp->iov_len, 
					(struct xmem *)&txmem, 
					(char *)NULL);
				break;
			}
			/* 
			 * issue dma complete and clean up 
			 */
			d_complete((int)ba->dma_chn_id, (int)0, 
				(char *)iovp->iov_base, 
				(size_t)iovp->iov_len, 
				(struct xmem *)&txmem, (char *)NULL);
			/*
			 * send end of interrupt 
			 */
			BUSIO_PUTC(seg_base + BA_ATR, BA_DISK | BA_EOI);
			/* 
			 * read 16 bit word from Status Interface register
			 * to clear the STAT_OUT bit of the BSR
			 */
			info = BUSIO_GETS(seg_base + BA_SIR); 
			/* 
			 * disable DMA on device 
			 */
			BUSIO_PUTC(seg_base + BA_BCR, 
				(char)(BA_DMA_DISABLE | BA_INT_ENABLE));
			uiop->uio_resid -= iovp->iov_len; /* compute residual*/
			iovp->iov_len = 0;                /* set length to 0 */
			/* 
			 * increment iov pointer to next structure 
			 */
			iovp =(struct iovec *)((int)iovp+sizeof(struct iovec));
		} /* for loop */
		break;
	case DUMPEND:
		/*
		 * clean up device, enable interrupts and return to
		 * a ready state
		 */
#ifdef DEBUG 
		BADBUG(("DUMPEND\n"));
#endif
		i_enable(ba->dump_ilevel);
		ba->dumpdev = FALSE;
		break;
	case DUMPTERM:
#ifdef DEBUG 
		BADBUG(("DUMPTERM\n"));
#endif
		break;
	default: 
		/* 
		 * invalid option 
		 */
		rc = EINVAL;
	}
	BUSIO_DET(seg_base);     /* Release segment register */
	IOCC_DET(iocc_seg);      /* Release segment register */
#ifdef DEBUG 
	BADBUG(("Exiting ba_dump 3  %d\n", rc));
#endif
	return(rc);
}

/*
 * NAME: ba_dump_func  
 *                                                                    
 * FUNCTION: Sets up the Dump Table Entry for the Bus Attached Disk
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine should be considered as part of the interrupt
 *	level. It is called by the system dump when this component
 *	is registered to make a dump table entry.                       
 *                                                                   
 * (NOTES:) This routine sets up the dump table and returns a pointer
 *          to the dump table.                                       
 *                                                                   
 * (RECOVERY OPERATION:) NONE
 *
 * (DATA STRUCTURES:) ba_cdt    - dump table                         
 *                    ba_info   - info structure
 *
 * RETURNS: Pointer to dump table.                                      
 */  

struct cdt *ba_dump_func(
int arg)
{
	struct ba_info *ba;
	int i;
	char only_one = FALSE;

	if (arg == 1) {
		/*
		 * Count of disks in system
		 */
		bcopy("bacount", ba_cdt->cdt_entry[0].d_name, 8);
		ba_cdt->cdt_entry[0].d_len = 1;
		ba_cdt->cdt_entry[0].d_ptr = (char *)&ba_count;
		ba_cdt->cdt_entry[0].d_segval = 0;
		/*
		 * Locking word
		 */
		bcopy("ba_lock", ba_cdt->cdt_entry[1].d_name, 8);
		ba_cdt->cdt_entry[1].d_len = 4;
		ba_cdt->cdt_entry[1].d_ptr = (char *)&ba_lock;
		ba_cdt->cdt_entry[1].d_segval = 0;
		/*
		 * List of info structures
		 */
		bcopy("infolst", ba_cdt->cdt_entry[2].d_name, 8);
		ba_cdt->cdt_entry[2].d_len = 8;
		ba_cdt->cdt_entry[2].d_ptr = (char *)ba_list;
		ba_cdt->cdt_entry[2].d_segval = 0;
		/*
		 * Top of trace table
		 */
		bcopy("tractop", ba_cdt->cdt_entry[3].d_name, 8);
		ba_cdt->cdt_entry[3].d_len = 4;
		ba_cdt->cdt_entry[3].d_ptr = (char *)&ba_trctop;
		ba_cdt->cdt_entry[3].d_segval = 0;
		/*
		 * Trace table
		 */
		bcopy("trace", ba_cdt->cdt_entry[4].d_name, 8);
		ba_cdt->cdt_entry[4].d_len = sizeof(struct ba_trace)*TRCLNGTH;
		ba_cdt->cdt_entry[4].d_ptr = (char *)ba_trace;
		ba_cdt->cdt_entry[4].d_segval = 0;
		/*
		 * Index into trace table
		 */
		bcopy("trcindx", ba_cdt->cdt_entry[5].d_name, 8);
		ba_cdt->cdt_entry[5].d_len = 4;
		ba_cdt->cdt_entry[5].d_ptr = (char *)&ba_trcindex;
		ba_cdt->cdt_entry[5].d_segval = 0;
		/*
		 * Info structure for disk 1
		 */
			
		if (ba_list[0] != NULL)
			ba = ba_list[0];
		else {
			only_one = TRUE;
			ba = ba_list[1];
		}
		bcopy(ba->dds.resource_name, ba_cdt->cdt_entry[6].d_name, 8);
		ba_cdt->cdt_entry[6].d_len = sizeof(struct ba_info);
		ba_cdt->cdt_entry[6].d_ptr = (char *)ba;
		ba_cdt->cdt_entry[6].d_segval = 0;
		/*
		 * Info structure for disk 2
		 */
		if ((ba_list[1] != NULL) && !only_one) {
			ba = ba_list[1];
			bcopy(ba->dds.resource_name, 
				ba_cdt->cdt_entry[7].d_name, 8);
			ba_cdt->cdt_entry[7].d_len = sizeof(struct ba_info);
			ba_cdt->cdt_entry[7].d_ptr = (char *)ba;
			ba_cdt->cdt_entry[7].d_segval = 0;
		}
		/*
		 * set length of entire table    
		 */
		ba_cdt->cdt_len = sizeof(struct cdt_head) + ((ba_count + 6) * 
				sizeof(struct cdt_entry));
	}

	return((struct cdt *)ba_cdt);

}

/*
 * NAME: ba_get_specs
 *                                                                    
 * FUNCTION: Stores the device configuration data 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the interrupt
 *	handler and by timer functions. It can not page fault.  
 *                                                                   
 * (NOTES:) This routine is called after a Get Device Configuration
 *	command has completed. The data is manupulated and stored in
 *      the badisk_specs structure
 *                                                                   
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) ba_info      - general info structure   
 *                    badisk_specs - device configuration info
 *
 * RETURNS: NONE                                                    
 */  

void ba_get_specs(
struct ba_info *ba)                    /* pointer to info structure */
{
	int	msw,                   /* most significant word */
		lsw;                   /* least significant word */
	ushort 	info;                  /* 16 bit storage word */

#ifdef DEBUG 
	BADBUG(("Entering ba_get_specs\n"));
#endif
	info = ba->blk.w[1];
	/*
	 * word 1 contains the spares per cylinder in the low byte and the
	 * special bits in the high byte
	 */
	ba->ba_cfig.spares_per_cyl = (uchar)(info & BA_LBMASK);
	ba->ba_cfig.special_bits = (uchar)((info >> BA_1BYTE) & BA_LBMASK);
	/*
	 * word 2 contains the low word of the number of rba's, and word 3
	 * contains the high word of the number of rba's, so concat these
	 * together to form the total number of rba's on the disk
	 * NOTICE: the bytes must be swapped since they come to us over
	 *         the bus byte swapped
	 */
	lsw = (uint)BA_SWAPB(ba->blk.w[2]); 
	msw = (uint)BA_SWAPB(ba->blk.w[3]);
	ba->ba_cfig.total_blocks = (lsw & BA_LWMASK) | ((msw << BA_1WORD) & 
		BA_HWMASK);
	/* 
	 * word 4 contains the number of cylinders on the disk, 
	 * REMEMBER: byte swap
	 */
	ba->ba_cfig.cylinders = (ushort)BA_SWAPB(ba->blk.w[4]);
	/*
	 * word 5 contains the number of sectors per track in the low byte
	 * and the number of tracks per cylinder in the high byte 
	 */
	info = ba->blk.w[5];
	ba->ba_cfig.sectors_per_trk = info & BA_LBMASK; 
	ba->ba_cfig.tracks_per_cyl = (info >> BA_1BYTE) & BA_LBMASK; 
#ifdef DEBUG 
	BADBUG(("Exiting ba_get_specs\n"));
#endif
	return;
}



/*
 * NAME: ba_get_ccs
 *                                                                    
 * FUNCTION: Stores the command complete status block
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the interrupt
 *	handler and by timer functions. It can not page fault.  
 *                                                                   
 * (NOTES:) This routine is called after a Get Command Complete Status,
 *      Get Device Status, or a Translate Relative Block Address Command
 *	has completed. The data is manupulated and stored in
 *      the badisk_cstat_blk structure
 *                                                                   
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) ba_info          - general info structure   
 *                    badisk_cstat_blk - command complete status block
 *
 * RETURNS: NONE                                                    
 */  

void ba_get_ccs(
struct ba_info *ba)                   /* pointer to general info structure */
{
	ushort info;                  /* 16 bit storage word */

#ifdef DEBUG 
	BADBUG(("Entering ba_get_ccs\n"));
#endif
	/*
	 * word 0 contains the last command that was completed and to which
	 * device it was on. REMEMBER: swap bytes
	 */
	ba->ba_ccs.wc_dev_cmd = (ushort)BA_SWAPB(ba->blk.w[0]);
	/*
	 * word 1 contains the command status in the low byte and the
	 * command error code int the high byte 
	 */
	info = ba->blk.w[1];
	ba->ba_ccs.cmd_status = (uchar)(info & BA_LBMASK);
	ba->ba_ccs.cmd_error = (uchar)((info >> BA_1BYTE) & BA_LBMASK); 
	/*
	 * word 2 contains the device status in the low byte and the
	 * device error code in the high byte
	 */
	info = ba->blk.w[2];
	ba->ba_ccs.dev_status = (uchar)(info & BA_LBMASK); 
	ba->ba_ccs.dev_error = (uchar)((info >> BA_1BYTE) & BA_LBMASK); 
	/*
	 * word 3 contains the number of blocks left to process
	 * REMEMBER: swap bytes
	 */
	ba->ba_ccs.blocks_left = (ushort)BA_SWAPB(ba->blk.w[3]);
	/*
	 * word 4 contains the low word of the last RBA processed and word 5
	 * contains the high word of the last RBA processed 
	 * REMEMBER: swap bytes
	 */
	ba->ba_ccs.lsw_lastrba = (ushort)BA_SWAPB(ba->blk.w[4]);
	ba->ba_ccs.msw_lastrba = (ushort)BA_SWAPB(ba->blk.w[5]); 
	/*
	 * word 6 contains the number of blocks requiring error recovery
	 */
	ba->ba_ccs.blks_err_recov = (ushort)BA_SWAPB(ba->blk.w[6]);
#ifdef DEBUG 
	BADBUG(("Exiting ba_get_ccs\n"));
#endif
	return;
}


/*
 * NAME: ba_get_dsb
 *                                                                    
 * FUNCTION: Stores the diagnostic status block       
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the interrupt
 *	handler and by timer functions. It can not page fault.  
 *                                                                   
 * (NOTES:) This routine is called after a Get Diagnostic Status Block 
 *	command has completed. The data is manupulated and stored in
 *      the badisk_dstat_blk structure
 *                                                                   
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) ba_info          - general info structure  
 *                    badisk_dstat_blk - diagnostic status block      
 *
 * RETURNS: NONE                                                    
 */  

void ba_get_dsb(
struct ba_info *ba)                    /* pointer to general info structure */
{
	ushort info;                   /* 16 bit storage word */

#ifdef DEBUG 
	BADBUG(("Entering ba_get_dsb\n"));
#endif
	/*
	 * word 1 contains the command status in the low byte and the command
	 * error code in the high byte
	 */
	info = ba->blk.w[1];
	ba->ba_dsb.cmd_status = (uchar)(info & BA_LBMASK);    
	ba->ba_dsb.cmd_error = (uchar)((info >> BA_1BYTE) & BA_LBMASK); 
	/*
	 * word 2 contains the device status in the low byte and the device
	 * error code in the high byte
	 */
	info = ba->blk.w[2];
	ba->ba_dsb.dev_status = (uchar)(info & BA_LBMASK);  
	ba->ba_dsb.dev_error = (uchar)((info >> BA_1BYTE) & BA_LBMASK); 
	/*
	 * word 3 contains the power on error code in the low byte and the
	 * test error code in the high byte
	 */
	info = ba->blk.w[3];
	ba->ba_dsb.pwron_errcode = (uchar)(info & BA_LBMASK);   
	ba->ba_dsb.test_errcode = (uchar)((info >> BA_1BYTE) & BA_LBMASK); 
	/*
	 * word 4 contains the last diagnostic command
	 * REMEMBER: byte swap
	 */
	ba->ba_dsb.diag_cmd = (ushort)BA_SWAPB(ba->blk.w[4]); 
	/*
	 * word 5 and 6 are reserved words
	 */
	ba->ba_dsb.reserved1 = (ushort)(ba->blk.w[5]);    
	ba->ba_dsb.reserved2 = (ushort)(ba->blk.w[6]);  
#ifdef DEBUG 
	BADBUG(("Exiting ba_get_dsb\n"));
#endif
	return;
}


/*
 * NAME: ba_get_pos
 *                                                                    
 * FUNCTION: Stores the POS Register Information      
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the interrupt
 *	handler and by timer functions. It can not page fault.  
 *                                                                   
 * (NOTES:) This routine is called after a Get POS Register Information
 *	command has completed. The data is manupulated and stored in
 *      the badisk_pos_regs structure
 *                                                                   
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) ba_info         - general info structure   
 *                    badisk_pos_regs - POS register information     
 *
 * RETURNS: NONE                                                    
 */  

void ba_get_pos(
struct ba_info *ba)                /* pointer to general info structure */
{
	ushort info;               /* 16 bit storage word */

#ifdef DEBUG 
	BADBUG(("Entering ba_get_pos\n"));
#endif
	/*
	 * word 1 contains POS register 0 in the low byte and POS register
	 * 1 in the high byte
	 */
	info = ba->blk.w[1];
	ba->ba_pos.posreg0 = (uchar)(info & BA_LBMASK);   
	ba->ba_pos.posreg1 = (uchar)((info >> BA_1BYTE) & BA_LBMASK);
	/*
	 * word 2 contains POS register 2 in the low byte and POS register
	 * 3 in the high byte
	 */
	info = ba->blk.w[2];
	ba->ba_pos.posreg2 = (uchar)(info & BA_LBMASK);             
	ba->ba_pos.posreg3 = (uchar)((info >> BA_1BYTE) & BA_LBMASK);
	/*
	 * word 3 contains POS register 4 in the low byte and POS register
	 * 5 in the high byte
	 */
	info = ba->blk.w[3];
	ba->ba_pos.posreg4 = (uchar)(info & BA_LBMASK);
	ba->ba_pos.posreg5 = (uchar)((info >> BA_1BYTE) & BA_LBMASK);
	/*
	 * word 4 contains POS register 6 in the low byte and POS register
	 * 7 in the high byte
	 */
	info = ba->blk.w[4];
	ba->ba_pos.posreg6 = (uchar)(info & BA_LBMASK);  
	ba->ba_pos.posreg7 = (uchar)((info >> BA_1BYTE) & BA_LBMASK);  
#ifdef DEBUG 
	BADBUG(("Exiting ba_get_pos\n"));
#endif
	return;
}


/*
 * NAME: ba_issue_cmd
 *                                                                    
 * FUNCTION: Builds a command block to be issued to the Bus Attached Disk
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called at the process level through an
 *	ioctl and at the interrupt level by the interrupt handler 
 *	and timer functions. It can not page fault.            
 *                                                                   
 * (NOTES:) This routine builds the proper command block based on values
 *	in the info structure to issue a command to the Bus Attached Disk.
 *
 * (RECOVERY OPERATION:) If an error is encountered, such as PIO error
 *	or device busy, the proper errno is returned and the caller is
 *      responsible for recovery.
 *
 * (DATA STRUCTURES:) ba_info          - info structure    
 *                    ba_cmd_blk       - command block structure
 *
 * RETURNS: EIO     - Bus error occurred during PIO transfer
 *          EBUSY   - Device cannot accept command, device busy
 *          EINVAL  - Invalid operation
 *
 */  

int	ba_issue_cmd(
struct ba_info *ba,           /* pointer to info structure */
int	op,                   /* command to issue */
ushort device)                /* device to issue command to */
{
	uint 	base;         /* base address */
	int	rc = 0,       /* return code */
		old_level,    /* save old interrupt level */
		special;      /* hold special bits of op */
	uchar 	status;       /* value in BSR */

#ifdef DEBUG  
	BADBUG(("Entering ba_issue_cmd : %X,%X\n", op, device));
#endif
	old_level = i_disable(ba->dds.intr_priority);/* disable my irpt level*/
	base = ba->dds.base_address;                 /* get base address */
	switch (op) {
		/* 
		 * The following three cases,(BA_HARD_RESET,BA_ABORT_CMD,
		 * BA_SOFT_RESET), are cmds that do not go through the 
		 * CIR(command interface register), but to the  
		 * ATR(Attention Register), or BCR(Basic Control Register) 
		 * and therefore are treated separate from the commands that 
		 * require a command block.  These commands do not care if 
		 * a command is in progress on the device.          
		 */

	case BA_HARD_RESET:                    /* issue Hardware Reset */
		ba->command_cmpl = FALSE;      /* clear command complete flag*/
		/* 
		 * write hard reset code to BCR 
		 */
		ba->pio_address = base + BA_BCR;
		ba->pio_data = BA_RESET | BA_INT_ENABLE;
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_W;
		if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov)) { 
			/*
			 * PIO Transfer Failed
			 */
			rc = EIO;
			break;
		}
		if (pio_assist((caddr_t)ba, ba_piodelay, ba_piodrcv)) { 
			/*
			 * PIO Transfer Failed
			 */
			rc = EIO;
			break;
		}
		/* 
		 * clear reset bit and enable irpts 
		 */
		ba->pio_address = base + BA_BCR; 
		ba->pio_data = BA_INT_ENABLE;
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_W;
		if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov)) { 
			/*
			 * PIO Transfer Failed
			 */
			rc = EIO;
			break;
		}
		ba->piowtrb->func_data = (ulong)ba;   /* save pointer */
		/* 
		 * since a reset completes with interrupts disabled on the 
		 * device, we must wait for interrupt and then enable
		 * interrupts, so call timer routine to wait for interrupt
		 */
		ba_waitirpt(ba->irpttrb); 
		ba->mywdog.watch.restart = 20; /* set watchdog timeout */
		w_start(&ba->mywdog.watch);    /* start the watchdog timer */
		break;
	case BA_ABORT_CMD:                  /* Abort a the command on device */
		/* 
		 * read the status register 
		 */
		ba->pio_address = base + BA_BSR;      
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_R;
		if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov)) {
			/*
			 * PIO Transfer Failure 
			 */
			rc = EIO;
			break;
		}
		if (ba->pio_data & BA_CIPS) {
			/* 
			 * Make sure that a command is in progress before 
			 * issuing abort, because if an abort is issued to 
			 * an idle device, and attention error will result.
			 */
			ba->pio_address = base + BA_ATR; 
			ba->pio_data = BA_ABORT_CMD | (device >> BA_1BYTE);
			ba->pio_size = BA_1BYTE;
			ba->pio_op = BA_PIO_W;
			if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov))
				/*
				 * PIO Transfer Failure 
				 */
				rc = EIO;    
		}
		break;
	case BA_SOFT_RESET:                    /* issue Soft Reset */
		ba->command_cmpl = FALSE;      /* clear command complete flag*/
		/* 
		 * issue soft reset code to ATR 
		 */
		ba->pio_address = base + BA_ATR;
		ba->pio_data = BA_SOFT_RESET | (BA_ATTACH >> BA_1BYTE);
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_W;
		if (pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov))
			/*
			 * PIO Transfer Failure 
			 */
			rc = EIO;
		break;
	default:
		/* 
		 * all other cases, commands requiring cmd blocks thru the CIR
		 */
		special = op & BA_LBMASK;  /* pull out special bits from op */
		op &= BA_HBMASK;           /* save base command from op */

		/* 
		 * if in diagnostic mode and the desired operation is not a 
		 * read or write then check immediately to see if the device 
		 * is busy.  If it is, fail the request with EBUSY.  This is 
		 * to avoid a poorly written diagnostic application from
		 * overrunning the state machine timers (e.g. three back to 
		 * back commands without waiting for command completion as 
		 * instructed to do in the CIS.  Read or Write commands are 
		 * issued only by this driver and are guaranteed not to have 
		 * more than one active command and one command waiting, 
		 * therefore they need not check here for device busy.   
		 */
		 
		if ((ba->diagnos_mode) && ((op != BA_READ_DATA) && 
			(op != BA_WRITE_DATA) && (op != BA_WRITE_VERIFY))) {
			if (!ba->command_cmpl) {
				rc = EBUSY;
				break;
			}
		}

		if ((op == BA_SEEK_CMD) && !ba->command_cmpl) {
			/*
			 * if this is a seek command (the idle timer popped)
			 * but there is a command that is not complete,
			 * ignore this seek
			 */
			rc = EBUSY;
			break;
		}
		ba->command_cmpl = FALSE;  /* clear command complete flag*/
		ba->lcmd_dev = device;     /* save last cmd target device */
		ba->last_cmd = op;         /* save last command issued */
		ba->rwop = FALSE;          /* clear read/write operation flag*/
		/* 
		 * set register flag to CIR meaning this block is to be
		 * written to the Command Interface Register
		 */
		ba->blk.reg = BA_PIO_W;  
		ba->blk.current = 0;       /* clear current word number */
		switch (op) {
		case BA_READ_DATA    :
		case BA_WRITE_DATA   :
		case BA_WRITE_VERIFY :  
			/*
			 * Build command block for a Read, Write or Write
			 * with Verify
			 */
			ba->rwop = TRUE;             /* set read/write flag */
			ba->blk.wc = 4;              /* 4 words to transfer */
			/* 
			 * build the 4 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_FOUR_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1]=(ushort)BA_SWAPB((ba->byte_count/BA_BPS));
			ba->blk.w[2]=(ushort)BA_SWAPB((ushort)(ba->rba & 
				BA_LWMASK));
			ba->blk.w[3] = (ushort)BA_SWAPB((ushort)((ba->rba >> 
				BA_1WORD) & BA_LWMASK));
			ba->mywdog.watch.restart = 30; /*set watchdog timeout*/
			break;
		case BA_READ_VERIFY  :
			/*
			 * build Read Verify command block
			 */
			ba->blk.wc = 4;               /* 4 words to transfer */
			/* 
			 * build the 4 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_FOUR_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1]=(ushort)BA_SWAPB((ba->byte_count/BA_BPS));
			ba->blk.w[2]=(ushort)BA_SWAPB((ushort)(ba->rba & 
				BA_LWMASK));
			ba->blk.w[3] = (ushort)BA_SWAPB((ushort)((ba->rba >> 
				BA_1WORD) & BA_LWMASK));
			ba->mywdog.watch.restart = 200;/*set watchdog timeout*/
			break;
		case BA_SEEK_CMD    :
		case BA_TRANS_RBA      :
			/*
			 * Build the Seek or Translate RBA command Blocks
			 */
			ba->blk.wc = 4;               /* 4 words to transfer */
			/* 
			 * build the 4 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_FOUR_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)BA_FILL_WORD;
			ba->blk.w[2] = (ushort)BA_SWAPB((ushort)(ba->rba & 
				BA_LWMASK));
			ba->blk.w[3] = (ushort)BA_SWAPB((ushort)((ba->rba >> 
				BA_1WORD) & BA_LWMASK));
			ba->mywdog.watch.restart = 10; /*set watchdog timeout*/
			break;
		case BA_PARK_HEADS     :
			/* 
			 * Build command block to park heads in safe place 
			 */
			ba->blk.wc = 2;               /* 2 words to transfer */
			/* 
			 * build the 2 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_POPTIONS);
			ba->blk.w[1] = (ushort)BA_FILL_WORD;
			ba->mywdog.watch.restart = 10; /*set watchdog timeout*/
			break;
		case BA_PREFORMAT      : 
			/* 
			 * Build Preformat command block
			 */
			ba->blk.wc = 2;              /* 2 words to transfer */
			/* 
			 * build the 2 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)BA_PFCODE;
			ba->mywdog.watch.restart = 10; /*set watchdog timeout*/
			break;
		case BA_FORMAT_CMD     :    
			/* 
			 * Build the format command block 
			 */
			ba->blk.wc = 2;               /* 2 words to transfer */
			/* 
			 * build the 2 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)special;
			ba->mywdog.watch.restart = 9000;/*set wdog timeout*/
			break;
		case BA_DIAG_TEST      :
			/*
			 * Build Diagnostic Test command Block
			 */
			ba->blk.wc = 2;               /* 2 words to transfer */
			/* 
			 * build the 2 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)((special << BA_1BYTE) & 
				BA_HBMASK);
			ba->mywdog.watch.restart = 400;/*set watchdog timeout*/
			break;
		case BA_GET_POS_INFO   :
		case BA_GET_CC_STAT    :
		case BA_GET_DEV_STAT   :
		case BA_GET_DIAG_STAT  :
		case BA_GET_DEV_CONFIG : 
			/* 
			 * Build the command block to Get POS info, get command
			 * complete status, get device status, get diagnostic
			 * status, or to get the device configuration
			 */
			ba->blk.wc = 2;            /* 2 words to transfer */
			/* 
			 * build the 2 command words
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)BA_FILL_WORD;
			ba->mywdog.watch.restart = 10; /*set watchdog timeout*/
			break;
		case BA_BUFFW_CMD:
		case BA_BUFFR_CMD:
		case BA_GET_MFG :
			/*
			 * Build the command block to Write to the attachment
			 * buffer, Read from the attachment buffer, or get
			 * the Manufacturers header
			 */
			ba->blk.wc = 2;               /* 2 words to transfer */
			/* 
			 * build the 2 command words 
			 */
			ba->blk.w[0] = (ushort)(device | op | BA_TWO_WRDS | 
				BA_OPTIONS);
			ba->blk.w[1] = (ushort)BA_SWAPB((ba->byte_count / 
				BA_BPS));
			ba->mywdog.watch.restart = 10; /*set watchdog timeout*/
			break;
		default:                          /* invalid command request */
			rc = EINVAL;
			break;
		} /* switch(op) (inner)*/
		ba->cmdtrb->func_data = (ulong)ba; /* save pointer */
		/* 
		 * timer shouldn't be active, but 
		 * safeguard against it 
		 */
		if (!(ba->cmdtrb->flags & T_ACTIVE))  
			/*
			 * start the command transfer and the
			 * watchdog timer
			 */
			ba_cmdxfer(ba->cmdtrb);  
		w_start(&ba->mywdog.watch);
	} /* switch(op) (outer) */
	i_enable(old_level);                    /* enable my interrupt level */
#ifdef DEBUG 
	BADBUG(("Exiting ba_issue_cmd\n"));
#endif
	return(rc);
}


/*
 * NAME: ba_log_error
 *                                                                    
 * FUNCTION: Logs an Error to the System Error Log            
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called from the process level as well as
 *	the interrupt level to log an error into the system error
 *	log. It can not page fault.                          
 *                                                                   
 * (NOTES:) This routine builds an error record based on the type
 *	of error that occurred and then saves it in the system 
 *      error log.
 *
 * (RECOVERY OPERATION:) NONE 
 *
 * (DATA STRUCTURES:) ba_info         - general info structure     
 *                    ba_error_log_df - error record structure for Bus
 *                                      Attached Disk
 *
 * RETURNS: NONE
 */  

void ba_log_error(
struct ba_info *ba)              /* pointer to general info structure */
{
	switch (ba->etype) {
	case BA_SE: 
		/* 
		 * soft read error 
		 */
		ba->elog.error_type = ERRID_BADISK_ERR1;    
		break;
	case BA_HE: 
		/* 
		 * hard read error 
		 */
		ba->elog.error_type = ERRID_BADISK_ERR2;    
		break;
	case BA_SC: 
		/*
		 * soft equipment check 
		 */
		ba->elog.error_type = ERRID_BADISK_ERR3;
		break;
	case BA_HC: 
		/*
		 * hard equipment check 
		 */
		ba->elog.error_type = ERRID_BADISK_ERR4;
		break;
	case BA_AE: 
		/*
		 * device attach error
		 */
		ba->elog.error_type = ERRID_BADISK_ERR5;  
		break;
	case BA_SK: 
		/* 
		 * seek fault 
		 */
		ba->elog.error_type = ERRID_BADISK_ERR8;         
		break;
	default: 
		break;
	};
	/* 
	 * store the current transfer statistics 
	 */
	ba->elog.bucket_count = ba->dds.bucket_count;   /* store bucket count*/
	ba->elog.byte_count = ba->dds.byte_count; /* store current byte count*/
	/* 
	 * copy the command complete status block 
	 */
	bcopy(&(ba->ba_ccs), &(ba->elog.sense_data), 14); 
	/* 
	 * log the error in system error log 
	 */
	errsave(&(ba->elog), sizeof(struct ba_error_log_df ));      
}


/*
 * NAME: ba_process_error
 *                                                                    
 * FUNCTION: Process Error Recovery for the Bus Attached Disk 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the   
 *	interrupt handler and system timers, and it can not page
 *	fault.                                               
 *                                                                   
 * (NOTES:) This routine performs the proper error recovery procedure
 *	as defined by the BADISK Problem Determination Guide.
 *
 * (RECOVERY OPERATION:) NONE                                     
 *
 * (DATA STRUCTURES:) ba_info           - general info structure   
 *                    buf               - transfer info structure
 *                    badisk_cstat_blk  - command complete status block
 *
 * RETURNS: NONE
 */  

void ba_process_error(
struct ba_info *ba)               /* pointer to info structure */
{
	char	retries,          /* number of retries for this recovery proc*/
		buf_found=FALSE,  /* flag whether buf with error found */
		last_rba_valid=TRUE;  /* flag whether last rba is valid */
	uint 	last_rba;         /* last relative block address of disk */
	struct buf *tmp_head;     /* temporary buf struct pointer */
	int	i=0;              /* counter */

#ifdef DEBUG 
	BADBUG(("Entering ba_process_error\n"));
#endif

	if (ba->erp == BA_MORE_INFO) {         
		/* 
		 * if an error recovery procedure is defined, but we need 
		 * more information....The following is prioritized to base 
		 * the error recovery on a cmd error code first, a device 
		 * error code second and a device status third. So, even if 
		 * the error recovery procedure is defined on a device status, 
		 * it can be changed if an error recovery procedures exists 
		 * for a cmd error as the code falls thru. 
		 */

		/* 
		 * check device status code 
		 */
		if (ba->ba_ccs.dev_status & BA_WRITE_FAULT_EM) {
			ba->erp = BA_LR1;  /* set erp to log, retry 1 time */
			ba->etype = BA_HC; /* set errtype to hard equip check*/
		}
		/* 
		 * check device error codes 
		 */
		switch (ba->ba_ccs.dev_error) {
		case BA_BLK_NOT_FOUND_ID:
		case BA_BLK_NOT_FOUND_AM:
		case BA_DATA_ECC_ERROR:
		case BA_ID_CRC_ERROR:
		case BA_NO_DATA_AM:
		case BA_NOIDAM_OR_IDECCERR:
		case BA_NO_ID_FOUND:
			ba->erp = BA_LR2B;  /* erp = log, retry 2 ,relocate */
			ba->etype = BA_HE;  /* set errtype to hard read error*/
			break;
		case BA_WRITE_FAULT:
		case BA_READ_FAULT:
		case BA_NO_INDEX_SECPULSE:
		case BA_DEVICE_NOT_READY:
		case BA_SEEK_ERROR:
			ba->erp = BA_LR1;  /* set erp to log, retry 1 time */
			ba->etype = BA_HC; /* set errtype to hard equip check*/
			break;
		case BA_INTERFACE_FAULT:
		case BA_RBA_OUTOF_RANGE:
		case BA_SELECT_ERR:
			ba->erp = BA_LAR3;  /* erp = log, abort, and retry 3 */
			ba->etype = BA_AE;  /* set error type to attach error*/
			break;
		case BA_SEEK_FAULT:
			ba->erp = BA_LR2B;   /* set erp to log, retry 1 time */
			ba->etype = BA_SK;  /* set error type to seek error */
			break;
		default: 
			break;
		} /* switch device error */
		/* 
		 * check command error codes 
		 */
		switch (ba->ba_ccs.cmd_error) {
		case BA_INVALID_PARM:
		case BA_CMD_NOT_SUPPORTED:
		case BA_FORMAT_REJECTED:
		case BA_FORMAT_ERROR_HCS:
			ba->erp = BA_LR1;   /* set erp to log, retry 1 time */
			ba->etype = BA_AE;  /* set error type to attach error*/
			break;
		case BA_CMD_ABORTED:
		case BA_FORMAT_WARNING_VEF:
			ba->erp = BA_LR1;  /* set erp to log, retry 1 time */
			ba->etype = BA_SC; /* set errtype to soft equip check*/
			break;
		case BA_FORMAT_ERROR_DE:
		case BA_FORMAT_WARNING_PTO:
		case BA_FORMAT_WARNING_15P:
			ba->erp = BA_LR1;  /* set erp to log, retry 1 time */
			ba->etype = BA_HC; /* set errtype to hard equip check*/
			break;
		case BA_CMD_REJECTED:
			ba->erp = BA_LHR1; /* erp = log, hard reset, retry 1 */
			ba->etype = BA_HC; /* set error type hard equip check*/
			break;
		case BA_INVALID_DEV:
			ba->erp = BA_LAR3; /* erp = log, abort, retry 3 */
			ba->etype = BA_AE; /* set error type attachment error*/
			break;
		default :
			break;
		} /* switch cmd error */
	} /* erp = more info */
	switch (ba->erp) {           
	/* 
	 * depending on the error recovery procedure .. 
	 */
	case BA_LOG: 
		/* 
		 * Log Error Only 
		 */
		ba_log_error(ba); 
		break;
	case BA_LR1: 
		/* 
		 * Log and Retry 1 time 
		 */
	case BA_LHR1: 
		/* 
		 * Log, Hard Reset and Retry 1 time 
		 */
		if (ba->l_waitcc) 
			/* 
			 * if error recovery waiting, (e.g. we issued a 
			 * hard reset and this is our second pass   
			 */
			ba->l_waitcc = FALSE;    /* then clear the flag */
		else
			ba_log_error(ba);        /* else log the error */
		if (ba->erp == BA_LHR1) {
			/*
			 * Need to issue Hard Reset
			 */
#ifdef DEBUG
			BADBUG(("Issuing Hard Reset\n"));
#endif
			ba_issue_cmd(ba, (int)BA_HARD_RESET, (ushort)BA_DISK);
			/* 
			 * set flag stating we are waiting for reset 
			 * to complete 
			 */
			ba->l_waitcc = TRUE;           
		}
		retries = BA_LR1_MAX_RETRIES;      /* set maximum retries */
		break;
	case BA_LR2B:  
		/* 
		 * Log, Retry up to 2 times, Put RBA in Bad Block Map 
		 */
		ba_log_error(ba);                   /* log the error */
		retries = BA_LR2_MAX_RETRIES;       /* set maximum retries */
		break;
	case BA_LAR3:  
		/* 
		 * Log, Abort, Retry up to 3 times 
		 */
		ba_log_error(ba);                   /* log the error */
#ifdef DEBUG
		BADBUG(("Aborting Command\n"));
#endif
		ba_issue_cmd(ba, (int)BA_ABORT_CMD, (ushort)ba->lcmd_dev);
		retries = BA_LR3_MAX_RETRIES;       /* set maximum retries */
		break;
	case BA_LR4:  
		/* 
		 * Log, Retry up to 4 times
		 */
		ba_log_error(ba);                   /* log the error */
		retries = BA_LR4_MAX_RETRIES;       /* set maximum retries */
		break;
	case BA_SILENT_RETRY:
		/*
		 * silently retry ( don't log error, but retry )
		 */
		retries = BA_LR3_MAX_RETRIES;       /* set maximum retries */
		break;
	} /* switch erp */
	if (ba->erp == BA_LOG) { 
		/* 
		 * if all we had to do was log the error, we have already 
		 * done this, so continue i/o  
		 */
		if (ba->buf_complete) {
			/*
			 * the transfer is complete, so iodone it and
			 * start next I/O
			 */
			ba->buf_complete = FALSE; /* clear buf complete flag */
			if (ba->chained == 0)
			   	ba->chained = 1;
			for(i=0;i<ba->chained; i++) {
				tmp_head = ba->head; /* save ptr to this buf*/
				ba->head = ba->head->av_forw; /* set next buf*/
				if (ba->reloc) {
					/* 
					 * We got the data, but only after 
					 * retries, so request relocation 
					 * by returning ESOFT
					 */
					tmp_head->b_flags |= B_ERROR;       
					tmp_head->b_error = ESOFT; 
					tmp_head->b_resid = ba->resid;
					ba->reloc = FALSE; /* clear relocate */
					ba->resid = 0;
				} else
				 {
					/*
					 * We got the data with no problems, so
					 * return no error
					 */
					tmp_head->b_flags &= ~B_ERROR;
					tmp_head->b_error = 0;
				}
				BADTRC(perridone,(uint)tmp_head);
				iodone(tmp_head); /* get rid of this buf */
			}
			ba_start_nextio(ba);      /* start next I/O */
		} else {
			/*
			 * else this transfer is not complete, (it was
			 * broken into chunks), so start the next chunk
			 */
			ba_start_nextio(ba);     /* start next I/O */
		}/* else more to transfer */

	} else
	 {
		/*
		 * else we need to do more than just
		 * log the error
		 */
		if (ba->erp != BA_LHR1) {
			/*
			 * if the error recovery is not defined to be
			 * a log, hard reset, and retry 1 time, then continue
			 * else if we did issue a hard reset, then we need to
			 * not do anything but wait for it to complete
			 */
			if (ba->rwop) {
				/*
				 * if all of this is a result of a read or
				 * write operation then ...
				 */
				/* 
				 * get last rba processed from the 
				 * command complete status block 
				 */
				last_rba = ba->ba_ccs.lsw_lastrba | 
					((ba->ba_ccs.msw_lastrba << 
					BA_1WORD) & BA_HWMASK);
				if ((ba->etype == BA_SK) || (last_rba < 
					ba->rba) || (last_rba >= (ba->rba + 
					(ba->byte_count / BA_BPS)))) 
					last_rba_valid = FALSE;
				BADTRC(perrlrba,(uint)last_rba);
				if (ba->chained && last_rba_valid && (ba->erp 
					== BA_LR2B)) {
					/*
					 * if we had coalesced bufs, check to
					 * see if any bufs completed before the
					 * error
					 */
					buf_found = FALSE;
					tmp_head = ba->head;
					while (!buf_found) {
						if ((tmp_head->b_blkno <= 
					   	   last_rba) && 
						   ((tmp_head->b_blkno +
						   (tmp_head->b_bcount / 
						   BA_BPS)) > last_rba))
						   buf_found = TRUE;
						else {
						   ba->head=ba->head->av_forw;
						   if (ba->reloc) {
							/* 
							 * We got the data, but
						 	 * only after retries,
							 * so request relocate 
							 * by returning ESOFT
							 */
							tmp_head->b_flags |= 
								B_ERROR;       
							tmp_head->b_error = 
								ESOFT;  
							tmp_head->b_resid = 
								ba->resid;
							ba->reloc = FALSE;
							ba->resid = 0;
						   } else {
							/*
							 * We got the data with
							 * no problems, so 
							 * return no error
							 */
							tmp_head->b_flags &= 
								~B_ERROR;
							tmp_head->b_error = 0;
						   }
					 	   BADTRC(perridone,
							(uint)tmp_head);
						   iodone(tmp_head);
						   tmp_head = ba->head;
						}
					}
				}
				if (ba->erp == BA_LR2B) {
					/*
					 * if we need to request relocation 
					 */
					ba->reloc = TRUE;/* set relocate flag*/
					/* 
					 * calculate the resid, since the LVM 
					 * will depend on the resid value to 
					 * determine what needs relocation 
					 */
					if (last_rba_valid)
						ba->resid = 
							ba->head->b_bcount - 
							( (last_rba - 
							ba->head->b_blkno) * 
							BA_BPS);
					else
						ba->resid = 
							ba->head->b_bcount;
					BADTRC(perrresid,
						(uint)ba->resid);
#ifdef DEBUG
					BADBUG(("LAST RBA : %X, resid : %X\n",
						 last_rba, ba->resid));
#endif
				} /* if erp = BA_LR2b */

				if (++ba->retry_count > retries) {
					/*
					 * we have exceeded our allowed
					 * number of retries, so fail this
					 * buf and start the next one
					 */
#ifdef DEBUG
					BADBUG(("FAILING BUF\n"));
#endif
					ba->retry_count = 0;/*clear retry cnt*/
					tmp_head = ba->head;/* save head of q*/
					ba->head = ba->head->av_forw;
					tmp_head->b_flags |= B_ERROR;
					if (ba->erp == BA_LR2B) { 
						/* 
						 * if the error involved a
						 * bad block, set error to 
						 * EMEDIA 
						 */
						tmp_head->b_error = EMEDIA;
						tmp_head->b_resid = ba->resid;
						ba->resid = 0;
					} else {
						/*
						 * else other error, set to
						 * EIO ( basic I/O error )
						 */
						tmp_head->b_error = EIO;
						tmp_head->b_resid = 
							tmp_head->b_bcount - 
							ba->bytes_sent;
					}
					BADTRC(perrbfail,(uint)tmp_head);
					BADTRC(perridone,(uint)tmp_head);
					iodone(tmp_head); /* get rid of  buf */
					ba->byte_count = 0;/* clear byte cnt */
					ba->target_buff = NULL;/*clear bufadd*/
					ba->buf_complete = FALSE;
					ba->reloc = FALSE;
					ba->rba = 0; 
					ba_start_nextio(ba);/* start next I/O*/
				} else { 
					/* 
					 * We havn't exhausted retries, so 
					 * start this one again 
					 */
#ifdef DEBUG
					BADBUG(("RETRYING\n"));
#endif
					BADTRC(perrretry,(uint)ba->head);
					ba_start_nextio(ba);/* start next I/O*/
				} /* else haven't used all our retries */
			} else {
				/*
				 * else this is not a result of a read or
				 * write operation, but some other command 
				 */
				if (++ba->retry_count <= retries) {
					/*
					 * if we haven't exhausted retries
					 * reissue the command 
					 */
#ifdef DEBUG
					BADBUG(("RETRYING CMD\n"));
#endif
					ba_issue_cmd(ba, (int)ba->last_cmd, 
						(ushort)ba->lcmd_dev);
				} else {
					/*
					 * else nothing else we can do
					 */
					if (ba->idle_seek_inprog) {
						/*
						 * if this was a seek 
						 * then try to start more
						 * IO
						 */
						ba->idle_seek_inprog = FALSE;
						ba_start_nextio(ba);	
					}
#ifdef DEBUG
					BADBUG(("GIVING UP ON CMD\n"));
#endif
				} /* else retries exhausted */
			} /* else not read/write operation */
		} /* if erp not BA_LHR1 */
	} /* else not BA_LOG */
}


/*
 * NAME: ba_sendeoi
 *                                                                    
 * FUNCTION: Function Routine passed to pio_assist for sending
 *           End of Interrupt via PIO.                                
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform the specific programmed I/O to send an End of Interrupt
 *      to the Bus Attached Disk
 *
 * (RECOVERY OPERATION:) If an error occurs while writing over the bus,
 *	an exception is generated and handled by the pio_assist exception
 *      handler.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info    
 *
 * RETURNS: 0     - Successful completion                           
 */  

int	ba_sendeoi(
caddr_t parms)                    /* pointer to info data */
{
	struct ba_info *ba;       /* pointer to info structure */
	ushort info;              /* 16 bit storage word */

	ba = (struct ba_info *)parms;        /* type cast to get my pointer */
	/* 
	 * Reserve segment register 
	 */
	ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, ba->dds.base_address);
	ba->state = 0;                            /* set a state variable */
	/* 
	 * send end of interrupt 
	 */
	BUSIO_PUTC(ba->seg_base + BA_ATR, ba->lint_dev | BA_EOI);
	ba->state++;                              /* increment state variable*/
	/* 
	 * read 16 bit Status Interface Register to  clear STAT_OUT bit of
	 * Basic Status Register 
	 */
	info = BUSIO_GETS(ba->seg_base + BA_SIR);               
	ba->state++;                              /* increment state variable*/
	/* 
	 * disable DMA on the device
	 */
	BUSIO_PUTC(ba->seg_base + BA_BCR, BA_DMA_DISABLE | BA_INT_ENABLE);    
	BUSIO_DET(ba->seg_base);                  /* Release segment register*/
	return(0);
}



/*
 * NAME: ba_eoirecov
 *                                                                    
 * FUNCTION: Recovery Routine passed to pio_assist for sending
 *           End of Interrupt via PIO.                                
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform recovery of an exception caused by the ba_sendeoi PIO  
 *      function routine.       
 *
 * (RECOVERY OPERATION:) This routine retries the failed transfer only if
 *	the PIO_RETRY op is passed, else is returns the proper errno, and
 *      pio_assist is responsible for passing this errno back up the call
 *      list.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info      
 *                    pio_except - info about the PIO failure
 *
 * RETURNS: 0      - Successful completion                           
 *          EIO    - Invalid State
 *                 - No more Retries
 *          EINVAL - Invalid Operation
 */  

int	ba_eoirecov(
caddr_t parm,                          /* pointer to info */
int	op,                            /* operation to perform */
struct pio_except *infop)              /* pointer to exception data */
{
	struct ba_info *ba;            /* pointer to info structure */
	int	rc = 0;                /* return code */
	ushort info;                   /* 16 bit storage word */

	ba = (struct ba_info *)parm;   /* type cast to get my pointer */
	BUSIO_DET(ba->seg_base);       /* Release segment register */
	switch (op) {
	case PIO_RETRY :               /* retry */
		ba->etype = BA_AE;     /* set error type to attachment error */
		ba_log_error(ba);      /* log the error */
		/* 
		 * reserve segment register 
		 */
		ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, 
			ba->dds.base_address);
		switch (ba->state) {       
		/* 
		 * depending on how far we got before the exception, 
		 * as told by our state variable, continue where we left off 
		 */
		case 0:
			/*
			 * resend End of Interrupt 
			 */
			BUSIO_PUTC(ba->seg_base+BA_ATR,ba->lint_dev | BA_EOI);
			ba->state++;             /* increment state variable */
		case 1:
			/*
			 * clear STAT_OUT bit of BSR
			 */
			info = BUSIO_GETS(ba->seg_base + BA_SIR);
			ba->state++;             /* increment state variable */
		case 2:
			/* 
			 * disable DMA on device 
			 */
			BUSIO_PUTC(ba->seg_base + BA_BCR, BA_DMA_DISABLE | 
				BA_INT_ENABLE);
			break;
		default: 
			rc = EIO;         /* invalid state */
			break;
		}
		BUSIO_DET(ba->seg_base);  /* release segment register */
		break;
	case PIO_NO_RETRY :               /* don't retry */
		ba->etype = BA_AE;        /* set error type to attach error */
		ba_log_error(ba);         /* log the error */
		rc = EIO;                 /* set error code */
		break;
	default:                          /* invalid request */
		rc = EINVAL;
		break;
	}
	return(rc);
}


/*
 * NAME: ba_piodelay
 *                                                                    
 * FUNCTION: Function Routine passed to pio_assist for delaying 
 *           10 microseconds.                                         
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform the specific programmed I/O to delay 10 microseconds,   
 *      one microsecond at a time, by writing to the hardware delay
 *      register.
 *
 * (RECOVERY OPERATION:) If an error occurs while writing over the bus,
 *	an exception is generated and handled by the pio_assist exception
 *      handler.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info   
 *
 * RETURNS: 0     - Successful completion                           
 */  

int	ba_piodelay(
caddr_t parms)                      /* pointer to data */
{
	struct ba_info *ba;         /* pointer to info structure */
	int	i = 0;              /* counter */

	ba = (struct ba_info *)parms;       /* type cast to get my pointer */
	/*
	 * Reserve segment register
	 */
	ba->seg_base = (uint)IOCC_ATT(ba->dds.bus_id, BA_DELAY_REG);
	/* 
	 * for 10 iterations , thus 10 microseconds
	 */
	for (i = 0; i < 10; i++) 
		BUSIO_PUTC(ba->seg_base, BA_ZERO);/* write to delay register */
	IOCC_DET(ba->seg_base);                   /* release segment register*/
	return(0);
}



/*
 * NAME: ba_piordcv
 *                                                                    
 * FUNCTION: Recovery Routine passed to pio_assist for delaying
 *           10 microseconds.                                          
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform recovery of an exception caused by the ba_piodelay PIO  
 *      function routine.       
 *
 * (RECOVERY OPERATION:) This routine retries the failed transfer only if
 *	the PIO_RETRY op is passed, else is returns the proper errno, and
 *      pio_assist is responsible for passing this errno back up the call
 *      list.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info   
 *                    pio_except - info about the PIO failure
 *
 * RETURNS: 0      - Successful completion                           
 *          EIO    - No more Retries
 *          EINVAL - Invalid Operation
 */  

int	ba_piodrcv(
caddr_t parm,                          /* pointer to info */
int	op,                            /* operation to perform */
struct pio_except *infop)              /* pointer to exception info */
{
	struct ba_info *ba;            /* pointer to general info structure */
	int	rc = 0;                /* return code */

	ba = (struct ba_info *)parm;   /* type cast to get my pointer */
	IOCC_DET(ba->seg_base);        /* release segment register */
	switch (op) { 
	case PIO_RETRY :               /* retry */
		ba->etype = BA_AE;     /* set error type to attachment error */
		ba_log_error(ba);      /* log the error */
		ba_piodelay((caddr_t)ba); /* retry the delay */
		break;
	case PIO_NO_RETRY :            /* don't retry */
		ba->etype = BA_AE;     /* set error type to attachment error */
		ba_log_error(ba);      /* log the error */
		rc = EIO;              /* set error code */
		break;
	default:                       /* invalid request */
		rc = EINVAL;
		break;
	}
	return(rc);
}


/*
 * NAME: ba_busystat
 *                                                                    
 * FUNCTION: Function Routine passed to pio_assist for waiting up to
 *           10 microseconds for a Busy Status.                       
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform the specific programmed I/O to wait up to 10 microseconds
 *      for a BUSY status to appear in the Basic Status Register.  This
 *      is used to determine when the device is ready to receive a command
 *	after an ATTENTION code (Request Accept Command) has
 *      been issued to the device.
 *
 * (RECOVERY OPERATION:) If an error occurs while writing over the bus,
 *	an exception is generated and handled by the pio_assist exception
 *      handler.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info  
 *
 * RETURNS: 0     - Successful completion                           
 */  

int	ba_busystat(
caddr_t parms)                     /* pointer to info data */
{
	struct ba_info *ba;        /* pointer to general info structure */
	int	i = 0;             /* counter */ 
	uchar status;              /* value of the Basic Status Register */

	ba = (struct ba_info *)parms;        /* type cast to get my pointer */
	/* 
	 * reserve segment register for busio 
	 */
	ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, ba->dds.base_address);
	/* 
	 * reserve segment register for iocc io 
	 */
	ba->iocc_seg = (uint)IOCC_ATT(ba->dds.bus_id, BA_DELAY_REG);
	status = BUSIO_GETC(ba->seg_base + BA_BSR);     /* Read Status */
	i = 0;                                          /* clear i */
	/* 
	 * wait for BUSY status by polling every 1 usec 
	 */
	while ((!(status & BA_BUSY)) && (i++ < BA_TO10)) {
		BUSIO_PUTC(ba->iocc_seg, BA_ZERO);         /* write delay reg*/
		status = BUSIO_GETC(ba->seg_base + BA_BSR); /* Read Status */
	}
	IOCC_DET(ba->iocc_seg);                 /* release segment register */
	BUSIO_DET(ba->seg_base);                /* release segment register */
	ba->pio_data = status;                  /* store the last status */
	return(0);
}

/*
 * NAME: ba_bstatrecov
 *                                                                    
 * FUNCTION: Recovery Routine passed to pio_assist for waiting for
 *           BUSY status.                                              
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform recovery of an exception caused by the ba_busystat PIO  
 *      function routine.       
 *
 * (RECOVERY OPERATION:) This routine retries the failed transfer only if
 *	the PIO_RETRY op is passed, else is returns the proper errno, and
 *      pio_assist is responsible for passing this errno back up the call
 *      list.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info   
 *                    pio_except - info about the PIO failure
 *
 * RETURNS: 0      - Successful completion                           
 *          EIO    - No more Retries
 *          EINVAL - Invalid Operation
 */  

int	ba_bstatrecov(
caddr_t parm,                           /* pointer to info data */
int	op,                             /* operation to perform */
struct pio_except *infop)               /* pointer to exception info */
{
	struct ba_info *ba;             /* pointer to general info structure */
	int	rc = 0;                 /* return code */

	ba = (struct ba_info *)parm;    /* type cast to get my pointer */
	IOCC_DET(ba->iocc_seg);         /* release segment register */
	BUSIO_DET(ba->seg_base);        /* release segment register */
	switch (op) {
	case PIO_RETRY :                /* retry */
		ba->etype = BA_AE;      /* set error type to attachment error*/
		ba_log_error(ba);       /* log the error */
		ba_busystat((caddr_t)ba);  /* retry the wait for busy stat */
		break;
	case PIO_NO_RETRY :             /* no retries */
		ba->etype = BA_AE;      /* set error type to attachment error*/
		ba_log_error(ba);       /* log the error */
		rc = EIO;               /* set error */
		break;
	default:                        /* invalid operation */
		rc = EINVAL;
		break;
	}
	return(rc);
}


/*
 * NAME: ba_piofunc
 *                                                                    
 * FUNCTION: Function Routine passed to pio_assist for performing Programmed
 *           I/O.                                                     
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform the specific programmed I/O. Either an 8 bit or 16
 *      bit word is either read or written over the bus.
 *
 * (RECOVERY OPERATION:) If an error occurs while writing over the bus,
 *	an exception is generated and handled by the pio_assist exception
 *      handler.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info  
 *
 * RETURNS: 0      - Successful completion                           
 *          EINVAL - Invalid Request
 */  

int	ba_piofunc(
caddr_t parm)                         /* pointer to info data */
{
	struct ba_info *ba;           /* pointer to general info structure */
	int	rc = 0;               /* return code */

	ba = (struct ba_info *)parm;  /* type cast to get pointer */
	switch (ba->pio_op) {
	case BA_PIO_W:                      
		/* 
		 * PIO write request 
		 */
		switch (ba->pio_size) {
		case BA_1BYTE:  
			/* 
			 * 8 bit write 
			 *
			 * reserve segment register 
			 */
			ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, 
				ba->pio_address);
			BUSIO_PUTC(ba->seg_base, ba->pio_data); /* write data*/
			BUSIO_DET(ba->seg_base);  /* release the seg register*/
			break;
		case BA_1WORD:  
			/* 
			 * 16 bit write 
			 *
			 * reserve segment register 
			 */
			ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, 
				ba->pio_address);
			BUSIO_PUTS(ba->seg_base, ba->pio_data); /* write data*/
			BUSIO_DET(ba->seg_base);  /* release the seg register*/
			break;
		default:                          /* invalid request */
			rc = EINVAL;
		}
		break;
	case BA_PIO_R: 
		/* 
		 * PIO read request 
		 */
		switch (ba->pio_size) {
		case BA_1BYTE: 
			/* 
			 * 8 bit read 
			 *
			 * reserve segment register 
			 */
			ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, 
				ba->pio_address);
			/*
			 * read data 
			 */
			ba->pio_data = (uchar)BUSIO_GETC(ba->seg_base);
			/* 
			 * release the segment register 
			 */
			BUSIO_DET(ba->seg_base);    
			break;
		case BA_1WORD:  
			/* 
			 * 16 bit read 
			 *
			 * reserve segment register 
			 */
			ba->seg_base = (uint)BUSIO_ATT(ba->dds.bus_id, 
				ba->pio_address);
			ba->pio_data = BUSIO_GETS(ba->seg_base); /* read data*/
			BUSIO_DET(ba->seg_base);  /* release the seg register*/
			break;
		default:                     /* invalid request */
			rc = EINVAL;
		}
		break;
	default:                             /* invalid request */
		rc = EINVAL;
	}
	return(rc);
}


/*
 * NAME: ba_piorecov
 *                                                                    
 * FUNCTION: Recovery Routine passed to pio_assist for Programmed I/O
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the interrupt level by   
 *	the pio_assist kernel service. It can not page fault.   
 *                                                                   
 * (NOTES:) This routine is called by the pio_assist kernel service
 *	to perform recovery of an exception caused by the ba_piofunc PIO  
 *      function routine.       
 *
 * (RECOVERY OPERATION:) This routine retries the failed transfer only if
 *	the PIO_RETRY op is passed, else is returns the proper errno, and
 *      pio_assist is responsible for passing this errno back up the call
 *      list.
 *
 * (DATA STRUCTURES:) ba_info    - contains PIO transfer info  
 *                    pio_except - info about the PIO failure
 *
 * RETURNS: 0      - Successful completion                           
 *          EIO    - No more Retries
 *          EINVAL - Invalid Operation
 */  

int	ba_piorecov(
caddr_t parm,                     /* pointer to info data */
int	op,                       /* operation to perform */
struct pio_except *infop)         /* pointer to exception info */
{
	struct ba_info *ba;       /* pointer to general info structure */
	int	rc = 0;           /* return code */

#ifdef DEBUG
	BADBUG(("Entering ba_piorecov %x %x %x\n", parm, op, infop));
	BADBUG(("Pio Except Structure : \n"));
	BADBUG(("Channel Status register = %x \n", infop->pio_csr));
	BADBUG(("Data Storage ISR        = %x \n", infop->pio_dsisr));
	BADBUG(("I/O Segment Register    = %x \n", infop->pio_srval));
	BADBUG(("Effective Address Used  = %x \n", infop->pio_dar));
#endif

	ba = (struct ba_info *)parm;    /* type cast to get pointer */
	BUSIO_DET(ba->seg_base);        /* release the segment register */
	switch (op) {
	case PIO_RETRY:                 /* retry */
		ba->etype = BA_AE;      /* set error type to attachment error*/
		ba_log_error(ba);       /* log the error */
		ba_piofunc((caddr_t)ba);/* retry the pio transfer */
		break;
	case PIO_NO_RETRY :             /* don't retry */
		ba->etype = BA_AE;      /* set error type to attachment error*/
		ba_log_error(ba);       /* log the error */
		rc = EIO;               /* set error */
		break;
	default:                        /* invalid request */
		rc = EINVAL;
		break;
	}
	return(rc);
}


/*
 * NAME: ba_start 
 *                                                                    
 * FUNCTION: Starts an I/O request on the Bus Attached Disk Device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine can be called at the process or interrupt 
 *	levels.  It is called by ba_start_nextio, and ba_strategy.
 *                                                                   
 * (NOTES:) This routine takes a buf structure and breaks it into
 *	allowable transfer sizes if necessary, and then issues the
 *      necessary commands to the device to perform the transfer.
 *
 * (RECOVERY OPERATION:) If an error occurs during the issue of the
 *	command, (PIO error), the proper errno is returned and the caller
 *      is left responsible for recovery.
 *
 * (DATA STRUCTURES:) buf         - transfer information   
 *                    ba_info     - general info structure
 *
 * RETURNS:  Returns result of call to ba_issue_cmd() 
 */  

int	ba_start(
struct ba_info *ba)
{
	struct buf *bp,*curr,*next;
	int	last_rba, rc = 0;
	char 	coalesce = FALSE;

#ifdef DEBUG 
	BADBUG(("Entering ba_start %x %x %x %x %x\n", ba->devno, bp, 
		bp->b_flags, bp->b_blkno, bp->b_bcount));
#endif
	if ((!ba->epow_pend)&&(!ba->dumpdev)&&(!ba->idle_seek_inprog)) { 
		/* if there is an epow pending, or we have been initialized
		 * as a dump device or idle seek in progress then don't do 
		 * anything and the I/O request will remain on the queue, so 
		 * if we recover without shutdown, we can continue unharmed
		 * else continue as normal ... 
		 */
		bp = ba->head;         /* set pointer to current I/O request */
		/* 
		 * get last RBA on disk 
		 */
		last_rba = ba->ba_cfig.total_blocks - 1;         
		/* 
		 * set cross memory descriptor 
		 */
		ba->mp = (struct xmem *) & (bp->b_xmemd); 
		if (bp->b_flags & B_READ) 
			/*  
			 *  READ 
			 */
			ba->dma_flags = DMA_READ;
		else 
			/* 
			 *  WRITE 
			 */
			ba->dma_flags = 0;
		if ( bp->b_bcount > DMA_MAX ) {
			BADTRC(strtbrkup,(uint)bp);
			/*
			 * this was a big transfer that we had to break
			 * up
			 */
			ba->chained = 0; /* no bufs in the coalesce chain */
			/* 
			 * set address of data to buf address plus 
			 * what we've already sent 
			 */
			ba->target_buff = (char *)((uint)bp->b_un.b_addr + 
				ba->bytes_sent);
			/* 
			 * compute transfer in blocks, based on byte count 
			 * and what we've sent 
			 */
			ba->target_xfer = 
				(bp->b_bcount - ba->bytes_sent) / BA_BPS;
			/*
			 * compute Relative Block Address ,based on 
			 * bufs RBA and what we've sent 
			 */
			ba->rba = bp->b_blkno + (ba->bytes_sent / BA_BPS);
			if ( (ba->rba + ba->target_xfer - 1) > last_rba) { 
				/* 
				 * the desired RBA is past end of disk, 
				 * so set resid and send what we can 
				 */
				bp->b_resid = BA_BPS * (ba->target_xfer - 
					(last_rba - ba->rba + 1));
				ba->target_xfer = last_rba - ba->rba + 1;
			} else /* It will all fit on disk, set resid to 0 */
				bp->b_resid = 0;
			/* 
			 * if desired transfer > maximum DMA transfer, break up
			 */
			if (ba->target_xfer > (int)(DMA_MAX / BA_BPS))
				ba->byte_count = DMA_MAX;  
			else 
				/* 
				 * we can send it all at once 
				 */
				ba->byte_count = 
					(int)(ba->target_xfer * BA_BPS); 
			/*
			 * Check if we are not to hide this page
			 */
			if (bp->b_flags & B_NOHIDE)
				ba->dma_flags |= DMA_WRITE_ONLY;
			ba->dma_active = TRUE;      /* set dma active flag */
			/* 
			 * call dma slave to initiate third party dma transfer 
			 */
			BADTRC(strtdslav,(uint)ba->target_buff);
			d_slave((int)ba->dma_chn_id, (int)ba->dma_flags, 
				(char *)ba->target_buff,(size_t)ba->byte_count,
				(struct xmem *)ba->mp);
			DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_BSTART, 0, 
				ba->devno, bp, bp->b_flags, bp->b_blkno, 
				bp->b_bcount);
			/*
			 * clear NOHIDE from dma flags
			 */
			ba->dma_flags &= ~DMA_WRITE_ONLY;
		} else {
			BADTRC(strtnorm,(uint)bp);
			/*
			 * not a huge transfer, so try to coalesce
			 */
			coalesce = TRUE;
			/* 
			 * at least 1 buf in the coalesce chain, so go ahead
			 * and map this first one 
			 */
			ba->chained = 1; 
			curr = bp;
			next = bp->av_forw;
			ba->byte_count = curr->b_bcount;
			ba->rba = curr->b_blkno;
			/*
			 * Check if not to hide this page
			 */
			if (curr->b_flags & B_NOHIDE)
				ba->dma_flags |= DMA_WRITE_ONLY;
			if ((ba->rba + (ba->byte_count / BA_BPS) - 1) 
				> last_rba) { 
				/* 
				 * this buf will take us past end of disk, 
				 * so set resid and send what we can 
				 */
				curr->b_resid = curr->b_bcount - 
					(BA_BPS * (last_rba - curr->b_blkno
					+ 1));
				ba->byte_count -= curr->b_resid;
				BADTRC(strtresid,(uint)curr->b_resid);
			} else 
				/* 
				 * It will all fit on disk, set resid to 0 
				 */
				curr->b_resid = 0;
			ba->dma_active = TRUE;  
			/* 
			 * call dma slave to initiate 
			 * third party dma transfer 
			 */
			BADTRC(strtdslav,(uint)curr);
			d_slave((int)ba->dma_chn_id, (int)ba->dma_flags, 
				(char *)curr->b_un.b_addr, 
				(size_t)curr->b_bcount - curr->b_resid, 
				(struct xmem *)&curr->b_xmemd);
			DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_BSTART, 0, 
				ba->devno, curr, curr->b_flags, curr->b_blkno, 
				curr->b_bcount);
			/*
			 * clear NOHIDE from dma flags
			 */
			ba->dma_flags &= ~DMA_WRITE_ONLY;
			while ( coalesce ) {
				/* 
				 * while the next buf is not null, its
				 * sequential on the disk, its the same (read/
				 * write) request, and if it won't put us over
				 * the maximum DMA transfer limit, then
				 * coalesce it and try the next one
				 */
				if ((next != NULL) && 
				   ((curr->b_blkno + (curr->b_bcount / BA_BPS))
 					== next->b_blkno) &&
				   ((curr->b_flags & B_READ) ==
					(next->b_flags & B_READ)) &&
				   ((curr->b_options & WRITEV) ==
					(next->b_options & WRITEV)) &&
				   ((ba->byte_count + next->b_bcount) 
					<= ba->dds.max_coalesce) &&   
				   ((ba->byte_count + next->b_bcount) 
					< DMA_MAX)) {
					BADTRC(strtchain,(uint)next);
					ba->chained++;
					ba->byte_count += next->b_bcount;
					curr = next;
					next = curr->av_forw;
					if ((ba->rba + 
						(ba->byte_count / BA_BPS) - 1) 
						> last_rba) { 
						/* 
						 * this buf will take us past 
						 * end of disk, so set resid 
						 * and send what we can 
						 */
						curr->b_resid = curr->b_bcount 
							- (BA_BPS * (last_rba 
							- curr->b_blkno + 1));
						ba->byte_count-=curr->b_resid;
						BADTRC(strtresid,
							(uint)curr->b_resid);
					} else 
						/* 
						 * It will all fit on disk, 
						 * set resid to 0 
						 */
						curr->b_resid = 0;
					/*
					 * Check if not to hide this page
					 */
					if (curr->b_flags & B_NOHIDE)
						ba->dma_flags|=DMA_WRITE_ONLY;
					/*
					 * set flag to tell d_slave to append
					 * this to the previous
					 */
					ba->dma_flags |= DMA_CONTINUE;
					/* 
					 * call dma slave to initiate 
					 * third party dma transfer 
					 */
					BADTRC(strtdslav,(uint)curr);
					d_slave((int)ba->dma_chn_id, 
						(int)ba->dma_flags, 
						(char *)curr->b_un.b_addr, 
						(size_t)curr->b_bcount - 
							curr->b_resid,
						(struct xmem *)&curr->b_xmemd);
					DDHKWD5(HKWD_DD_BADISKDD, 
						DD_ENTRY_BSTART, 0, ba->devno,
						curr, curr->b_flags, 
						curr->b_blkno, curr->b_bcount);
					/*
			 		 * clear NOHIDE from dma flags
			 		 */
					ba->dma_flags &= ~DMA_WRITE_ONLY;
					/*
			 		 * clear CONTINUE from dma flags
			 		 */
					ba->dma_flags &= ~DMA_CONTINUE;
				} else 
					/*
					 * Can't coalesce any more
					 */
					coalesce = FALSE;
			} /* while coalesce */
		} /* else not a huge transfer */
		if (bp->b_flags & B_READ) {
			/*
			 * Issue READ
			 */
			if (rc = ba_issue_cmd(ba, (int)BA_READ_DATA, 
				(ushort)BA_DISK)) {
				/*
				 * issue command failed, so clean up
				 */
				ba_dma_cleanup(ba);
				return(rc);
			} /* if ba_issue */
		} else {
			/* 
			 * Issue Write
			 */
			if (bp->b_options & WRITEV) {  
				/* 
				 * issue write with verify 
				 */
				if (rc = ba_issue_cmd(ba, (int)BA_WRITE_VERIFY,
					(ushort)BA_DISK)) {
					/*
					 * issue command failed, so clean up
					 */
					ba_dma_cleanup(ba);
					return(rc);
				} /* if ba_issue */
			} else {
				/* 
				 * issue WRITE  
				 */
				if (rc = ba_issue_cmd(ba, (int)BA_WRITE_DATA, 
					(ushort)BA_DISK)) {
					/*
					 * issue command failed, so clean up
					 */
					ba_dma_cleanup(ba);
					return(rc);
				} /* if ba_issue */
			} /* else not write verify */
		}/* else write operation */
		/*
		 * Either a Read or Write has been issued to the device,
		 * so update the transfer statistics and set the device 
		 * busy flag in the IOSTAT status 
		 */
		ba->dkstat.dk_xfers++;
		ba->dkstat.dk_status |= IOST_DK_BUSY;
	} /* if not Early Power off Warning */
#ifdef DEBUG 
	BADBUG(("Exiting ba_start %x \n", ba->devno));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_BSTART, 0, ba->devno);
	return(0);
}


/*
 * NAME: ba_start_nextio
 *                                                                    
 * FUNCTION: Starts the next I/O on the Bus Attached Disk I/O queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the
 *	interrupt handler and system timer functions. It can not
 *	page fault.                                           
 *                                                                   
 * (NOTES:) This routine check the I/O queue to determine if it can
 *	start a waiting request.                          
 *
 * (RECOVERY OPERATION:) NONE - Error recovery is not passed as return        
 *                              parameters, but via iodone().
 *
 * (DATA STRUCTURES:) buf        - transfer information  
 *                    ba_info    - general information structure
 *
 * RETURNS: NONE
 */  

void ba_start_nextio(
struct ba_info *ba)                 /* pointer to ba_info structure */
{
	char	iostarted = FALSE;  /* flag whether I/O has been started */
	struct buf *tmp_head;       /* temporary buf structure pointer */

#ifdef DEBUG
	BADBUG(("Entering ba_start_nextio \n"));
#endif

	iostarted = FALSE;                       /* clear iostarted flag */
	/* 
	 * while the I/O Queue is not empty and we have not 
	 * successfully started a request
	 */
	while ((ba->head != NULL) && (!iostarted)) {
		if (ba_start(ba)) { 
			/* 
			 * start failed, which means PIO transfer failure
			 * which has already been retried 3 times, so fail
			 * this request.
			 */
			tmp_head = ba->head;     /* save pointer to this buf */
			ba->head = ba->head->av_forw; /*set head  to next buf*/
			tmp_head->b_flags |= B_ERROR;  /* set buf error flags*/
			tmp_head->b_error = EIO;        /* set buf error code*/
			tmp_head->b_resid = tmp_head->b_bcount;  /* set resid*/
			BADTRC(nextidone,(uint)tmp_head);
			iodone(tmp_head);          /* get rid of this request*/
		} else 
			/* 
			 * else start succeeded 
			 */
			iostarted = TRUE;               /* set iostarted flag*/
	} /* while head not null and not started */
	if (ba->head == NULL)
		/* 
		 * Q has gone empty, so start the 
		 * device idle watchdog timer
		 */
		w_start(&ba->idle.watch); 
}


/*
 * NAME: ba_dma_cleanup
 *                                                                    
 * FUNCTION: Cleans up after a DMA transfer                        
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the
 *	interrupt handler and system timer functions. It can not
 *	page fault.                                           
 *                                                                   
 * (NOTES:) This routine calls d_complete to complete a DMA transfer
 *
 * (RECOVERY OPERATION:) If d_complete returns an error, this error is        
 *                              passed to the caller to handle the error.
 *
 * (DATA STRUCTURES:) buf        - transfer information  
 *                    ba_info    - general information structure
 *
 * RETURNS: Result of d_complete call(s)
 */  

int ba_dma_cleanup(
struct ba_info *ba)		 /* pointer to ba_info structure */
{
	struct buf *curr,*next;
	int i=0,rc=0;

	if (ba->chained) {
		/*
		 * clean up coalesced bufs
		 */
		curr = ba->head;
		next = ba->head->av_forw;
		for (i=0;i<ba->chained;i++) {
			/*
			 * Check for nohide 
			 */
			if (curr->b_flags & B_NOHIDE)
				ba->dma_flags |= DMA_WRITE_ONLY;
			if ( i > 0 ) {
				/*
				 * if not first mapping then set continue
				 * flag and don't check return code.
				 */
				ba->dma_flags |= DMA_CONTINUE;
				BADTRC(dmacdcomp,(uint)curr);
				d_complete((int)ba->dma_chn_id, 
				   (int)ba->dma_flags, 
				   (char *) curr->b_un.b_addr,
				   (size_t)curr->b_bcount - curr->b_resid,
		    		   (struct xmem *)&curr->b_xmemd,(char *)NULL);
			} else {
				/*
				 * first mapping , so check return code
				 */
				BADTRC(dmacdcomp,(uint)curr);
				rc = d_complete((int)ba->dma_chn_id, 
				   (int)ba->dma_flags, 
				   (char *) curr->b_un.b_addr,
				   (size_t)curr->b_bcount - curr->b_resid,
		    		   (struct xmem *)&curr->b_xmemd,(char *)NULL);
			}
			/*
	 		 * clear NOHIDE from dma flags
	 		 */
			ba->dma_flags &= ~DMA_WRITE_ONLY;
			/*
	 		 * clear CONTINUE from dma flags
	 		 */
			ba->dma_flags &= ~DMA_CONTINUE;
			curr = next;
			next = curr->av_forw;
		}
	} else {
		BADTRC(dmacdcomp,(uint)ba->head);
		rc = d_complete((int)ba->dma_chn_id, (int)ba->dma_flags,
			(char *)ba->target_buff, (size_t)ba->byte_count,
	    		(struct xmem *)ba->mp, (char *)NULL);
	}
	ba->dma_active = FALSE;
	return(rc);
}

/*
 * NAME: ba_piowxfer
 *                                                                    
 * FUNCTION: System Timer Function routine to perform 16 bit Read/
 *           Write PIO Transfers                                      
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by the system timer service on the
 *	interrupt level and can not page fault.                 
 *                                                                   
 * (NOTES:) This routine relies on the system timer service to 
 *	transfer 1 16 bit word at a time.  This method was used
 *      to eliminate interrupt delays such as polling.
 *
 * (RECOVERY OPERATION:) If an error were to occur, recovery is dependent 
 *	on the watchdog timer service.
 *
 * (DATA STRUCTURES:) trb       - system timer structure   
 *                    buf       - transfer information
 *                    ba_info   - general info structure 
 *
 * RETURNS: NONE
 */  

void ba_piowxfer(
struct trb *t)                   /* pointer to timer structure */
{
	struct ba_info *ba;      /* pointer to info structure */
	uint 	base;            /* base address of device */    
	int	old_level,i=0;   /* old interrupt level before disable, cnter*/
	struct buf *tmp_head;    /* temporary buf structure pointer */
	char	skip=FALSE;      /* flag to skip rest of this routine */

	ba = (struct ba_info *)(t->func_data);      /* get my pointer */
	if ((!ba->epow_pend)&&(!ba->dumpdev)) {
		/*
		 * Make sure we don't have an epow pending and 
		 * that we haven't been initialized as a dump device
		 * before continuing
		 */
		/* 
		 * disable my irpt level 
		 */
		old_level = i_disable(ba->dds.intr_priority); 
		base = ba->dds.base_address;            /* get base address */
		if (ba->blk.reg == BA_PIO_W) {
			/*
			 * Write 16 bit word, via pio_assist 
			 * and Command Interface Register
			 */
			/*
			 * Check CMD IN to see if we can send
			 * another word
			 */
			ba->pio_address = base + BA_BSR;  
			ba->pio_size = BA_1BYTE;
			ba->pio_op = BA_PIO_R;
			pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);
			if (ba->pio_data & BA_CMD_IN) {
				/*
				 * Not Ready, start timer
				 */
				ba->piowtrb->timeout.it_value.tv_sec = 0;
				ba->piowtrb->timeout.it_value.tv_nsec = 30000;
				ba->piowtrb->func_data = (ulong)ba;
				tstart(ba->piowtrb);              
				skip = TRUE;
			} else {
				/*
				 * Ready, write this word
				 */
				ba->pio_address = base + BA_CIR;  
				ba->pio_data = ba->blk.w[ba->blk.current];
				ba->pio_size = BA_1WORD;
				ba->pio_op = BA_PIO_W;
				pio_assist((caddr_t)ba,ba_piofunc,ba_piorecov);
			}
		} else {
			/*
			 * Read 16 bit word, via pio_assist and Status
			 * Interface Register
			 */
			/*
			 * Check STAT OUT to see if we can read
			 * another word
			 */
			ba->pio_address = base + BA_BSR;  
			ba->pio_size = BA_1BYTE;
			ba->pio_op = BA_PIO_R;
			pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);
			if (!(ba->pio_data & BA_STAT_OUT)) {
				/*
				 * Not Ready, start timer
				 */
				ba->piowtrb->timeout.it_value.tv_sec = 0;
				ba->piowtrb->timeout.it_value.tv_nsec = 30000;
				ba->piowtrb->func_data = (ulong)ba;
				tstart(ba->piowtrb);              
				skip = TRUE;
			} else {
				/*
				 * Ready, read this word
				 */
				ba->pio_address = base + BA_SIR; 
				ba->pio_size = BA_1WORD;
				ba->pio_op = BA_PIO_R;
				pio_assist((caddr_t)ba,ba_piofunc,ba_piorecov);
				/* 
				 * store status
				 */
				ba->blk.w[ba->blk.current] = ba->pio_data; 
			}
		}
		if(!skip) {
		   if (++ba->blk.current < ba->blk.wc) {
			/*
			 * if another word to read or write, set timeout
			 * to 1 usec and start the timer
			 */
			ba->piowtrb->timeout.it_value.tv_sec = 0;
			ba->piowtrb->timeout.it_value.tv_nsec = 30000;
			ba->piowtrb->func_data = (ulong)ba;  /* store pointer*/
			tstart(ba->piowtrb);                 /* start timer */
		   } else {
			/*
			 * else we have finished reading/writing this block
			 * of 16 bit words ...
			 */
			if (ba->blk.reg == BA_PIO_R) {
				/*
				 * if we just got finished reading a status
				 * block, call the proper store routine to
				 * store this information
				 */
				(*ba->blk.store)(ba);
				if (!ba->atre) 
					/* 
					 * if irpt was not attention error 
					 * then send End of Interrupt
					 */
					pio_assist((caddr_t)ba, ba_sendeoi, 
						ba_eoirecov);
				/* 
				 * set command complete flag 
				 */
				ba->command_cmpl = TRUE;  
				if (ba->erp) {
					/* 
					 * if an error recovery procedure was
					 * defined ...
					 */
					if (!ba->diagnos_mode) {
						/* 
						 * if not in diagnostic mode
						 * go ahead and process the
						 * error 
						 */
						ba_process_error(ba);
					} else {
						/*
						 * we don't retry or log errors
						 * in diagnostic mode, so just
						 * fail this request
						 */
						ba->erp = BA_NONE;
						ba->etype = 0;
						if (ba->rwop) {
							/*
							 * if I/O was involved
							 * fail the buf, and
							 * start next io, else
							 * it was just a failed
							 * command so don't do
							 * anything
							 */
							tmp_head = ba->head;
							ba->head = ba->
								head->av_forw;
							tmp_head->b_flags |= 
								B_ERROR; 
							tmp_head->b_error =EIO;
							tmp_head->b_resid =
							  tmp_head->b_bcount;
							BADTRC(piowidone,
							   (uint)tmp_head);
							iodone(tmp_head);
							ba->reloc = FALSE;
							ba_start_nextio(ba);
						} /* if rwop */
					} /* else diagnostic mode */
				} else {
					/*
					 * else no error recovery procedure
					 * was defined, so see what needs to 
					 * be done to continue I/O
					 */
					if (ba->rwop) {
						/*
						 * if last command was
						 * read or write
						 */
						if (ba->buf_complete) {
						  /*
						   * if this transfer is
						   * is complete, iodone
						   * this request and
						   * start the next
						   */
						  ba->buf_complete = FALSE;
						  if (ba->chained == 0)
							ba->chained = 1;
						  for(i=0;i<ba->chained; i++){
							tmp_head = ba->head;
							ba->head = ba->
								head->av_forw;
							if (ba->reloc) {
								/*
								 * if we need
								 * to request
								 * relocation
								 * because we
								 * succeeded
								 * only after
								 * retries
								 */
								tmp_head->
								   b_flags |= 
								   B_ERROR;
								tmp_head->
								   b_error =
								   ESOFT;
								tmp_head->
								   b_resid =
								   ba->resid;
								ba->reloc = 
									FALSE;
								ba->resid = 0;
							} else {
								/*
								 * else don't
								 * need to 
								 * request
								 * relocation
								 */
								tmp_head->
								   b_flags &= 
								   ~B_ERROR; 
								tmp_head->
								   b_error = 0;
							}
							BADTRC(piowidone,
							   (uint)tmp_head);
							iodone(tmp_head);
						   }
						   ba_start_nextio(ba);
						} /* if buf_complete */
					} else  /* if read/write op */
						/*
						 * if this was a seek that 
						 * completed, try to start
						 * more IO
						 */
						if (ba->idle_seek_cmpl) {
							ba->idle_seek_cmpl =
								FALSE;
							ba_start_nextio(ba);
						}
				} /* if process error */
			} /* if reg == BA_PIO_R */
		   } /* else no more to read */
		}
		i_enable(old_level);                  /* enable interrupts */
	} /* if ! ba->epow_pend */
	return;
}


/*
 * NAME: ba_waitirpt
 *                                                                    
 * FUNCTION: System Timer Function routine to wait for an interrupt
 *           Condition.                                               
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by the system timer service on the
 *	interrupt level and can not page fault.                 
 *                                                                   
 * (NOTES:) This routine relies on the system timer service to 
 *	Poll for an interrupt condition.   This method was used
 *      to eliminate interrupt delays caused by spin loop polling.
 *
 * (RECOVERY OPERATION:) If an error were to occur, recovery is dependent 
 *	on the watchdog timer service.
 *
 * (DATA STRUCTURES:) trb       - system timer structure  
 *                    buf       - transfer information
 *                    ba_info   - general info structure 
 *
 * RETURNS: NONE
 */  

void ba_waitirpt(
struct trb *t)                   /* pointer to timer structure */
{
	struct ba_info *ba;      /* pointer to general info structure */
	uint 	base;            /* base address of device */
	int	old_level;       /* old interrupt level before i_disable */
	uchar 	status;          /* contents of Basic Status Register */

	ba = (struct ba_info *)(t->func_data);       /* get my pointer */
	old_level = i_disable(ba->dds.intr_priority);/* disable my irpt level*/
	base = ba->dds.base_address;                 /* get base address */
	ba->pio_address = base + BA_BSR;             /* read the status */
	ba->pio_size = BA_1BYTE;
	ba->pio_op = BA_PIO_R;
	if (!(pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov))) {
		/*
		 * if we did not have any kind of PIO error while
		 * reading the status, ...
		 */
		status = (uchar)ba->pio_data;            /* store the status */
		if (((status & BA_INT_PEND) || (status & BA_CIPS) || 
			(status & BA_BUSY)) && (!(status & BA_IRPT))) {
			/*
			 * if still no interrupt condition,
			 * set timeout to 1 second and start timer 
			 */
			ba->irpttrb->timeout.it_value.tv_sec = 1; 
			ba->irpttrb->timeout.it_value.tv_nsec = 0;
			ba->irpttrb->func_data = (ulong)ba;/* store pointer */
			tstart(ba->irpttrb);               /* start the timer*/
		} else {
			/*
			 * either status is clear or there is
			 * an interrupt condition, so if status
			 * is clear set command complete flag, then
			 * enable interrupts, so if there is an interrupt
			 * condition, our interrupt handler will take over
			 */
			if (!(status & BA_IRPT))
				ba->command_cmpl = TRUE;
			/* 
			 * enable device interrupts 
			 */
			ba->pio_address = base + BA_BCR;
			ba->pio_data = BA_INT_ENABLE;
			ba->pio_size = BA_1BYTE;
			ba->pio_op = BA_PIO_W;
			pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);
		}
	}
	i_enable(old_level);        /* enable interrupt level */
}


/*
 * NAME: ba_cmdxfer
 *                                                                    
 * FUNCTION: System Timer Function routine to wait while device is 
 *           busy to start a command transfer.                        
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by the system timer service on the
 *	interrupt level and can not page fault.                 
 *                                                                   
 * (NOTES:) This routine relies on the system timer service to 
 *	Poll for device not BUSY and not Command in Progress. This method
 *      was used to eliminate interrupt delays caused by spin loop polling.
 *
 * (RECOVERY OPERATION:) If an error were to occur, recovery is dependent 
 *	on the watchdog timer service.
 *
 * (DATA STRUCTURES:) trb       - system timer structure   
 *                    buf       - transfer information
 *                    ba_info   - general info structure 
 *
 * RETURNS: NONE
 */  

void ba_cmdxfer(
struct trb *t)                  /* pointer to timer structure */
{
	struct ba_info *ba;     /* pointer to general info structure */
	uint 	base;           /* base address of device */
	int	old_level;      /* old interrupt level before i_disable */
	uchar 	status;         /* contents of Basic Status Register */

	ba = (struct ba_info *)(t->func_data);       /* get my pointer */
	old_level = i_disable(ba->dds.intr_priority);/* disable my irpt level*/
	base = ba->dds.base_address;                 /* get base address */
	ba->pio_address = base + BA_BSR;             /* read the status */
	ba->pio_size = BA_1BYTE;
	ba->pio_op = BA_PIO_R;
	pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);
	status = (uchar)ba->pio_data;
	if ((status & BA_BUSY) || (status & BA_CIPS)) {
		/*
		 * if the device is showing a BUSY status or
		 * that a command is in progress, set the timeout
		 * to 1 microsecond and start the timer 
		 */
		ba->cmdtrb->timeout.it_value.tv_sec = 0;
		ba->cmdtrb->timeout.it_value.tv_nsec = 1000; 
		ba->cmdtrb->func_data = (ulong)ba;        /* store pointer */
		tstart(ba->cmdtrb);                       /* start the timer */
	} else {
		/*
		 * the device is no longer busy, so start
		 * the command transfer
		 */
		/* 
		 * request to accept command 
		 */
		ba->pio_address = base + BA_ATR; 
		ba->pio_data = BA_ACCEPT_CMD | (ba->lcmd_dev >> BA_1BYTE);
		ba->pio_size = BA_1BYTE;
		ba->pio_op = BA_PIO_W;
		pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);
		/* 
		 * wait until device ready for cmd 
		 */
		pio_assist((caddr_t)ba, ba_busystat, ba_bstatrecov);
		ba->command_cmpl = FALSE;      /* clear command complete flag*/
		ba->piowtrb->func_data = (ulong)ba; /* save pointer to struct*/
		if (!(ba->piowtrb->flags & T_ACTIVE)) 
			/* 
			 * timer shouldn't be active, but 
			 * safeguard against it anyway before
			 * manually calling timer routine to
			 * start the command block transfer 
			 */
			ba_piowxfer(ba->piowtrb);            
	}
	i_enable(old_level);             /* enable interrupt level */
}

/*
 * NAME: ba_idle 
 *                                                                    
 * FUNCTION: Bus Attached Disk Device Idle Watchdog Handler                
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the system
 *	watchdog timer service. It can not page fault.          
 *                                                                   
 * (NOTES:) This routine issues a seek to the device when it has been
 *	idle for at least 90 seconds.                                   
 *
 * (RECOVERY OPERATION:) NONE
 *
 * (DATA STRUCTURES:) watchdog    - watchdog timer structure  
 *                    ba_info     - general info structure
 *                    ba_watchdog - local watchdog structure for Bus Attached
 *                                  Disk
 *
 * RETURNS: NONE
 */  
void ba_idle(
struct watchdog *wp)                /* pointer to system watchdog structure */
{
	struct ba_info *ba;         /* pointer to general info structure */
	struct ba_watchdog *mw;     /* pointer to local watchdog structure */
	int rc = 0;

	mw = (struct ba_watchdog *)wp;  /* type cast to local watchdog struct*/
	ba = (struct ba_info *)(mw->ba_pointer);/* get pointer to info struct*/
	if (ba->head == NULL) {
		ba->idle_seek_inprog = TRUE;
		ba->idle_seek_rba += 10;
		if (ba->idle_seek_rba >= ba->ba_cfig.total_blocks)
			ba->idle_seek_rba = 0;
		ba->rba = ba->idle_seek_rba;
		rc = ba_issue_cmd(ba, (int)BA_SEEK_CMD, (ushort)BA_DISK);
		if (rc)
			ba->idle_seek_inprog = FALSE;
	}
	return;
}

/*
 * NAME: ba_watch 
 *                                                                    
 * FUNCTION: Bus Attached Disk Watchdog Handler                
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called at the interrupt level by the system
 *	watchdog timer service. It can not page fault.          
 *                                                                   
 * (NOTES:) This routine attempts to clean up and retry any existing
 *	I/O or commands that have not completed within the time allowed.
 *
 * (RECOVERY OPERATION:) This routine is a recovery routine which
 *	issues a Hard Reset to the device and relies on the interrupt
 *      handler to respond to the reset and retry the last failed command.
 *
 * (DATA STRUCTURES:) watchdog    - watchdog timer structure  
 *                    ba_info     - general info structure
 *                    ba_watchdog - local watchdog structure for Bus Attached
 *                                  Disk
 *
 * RETURNS: NONE
 */  

void ba_watch(
struct watchdog *wp)                /* pointer to system watchdog structure */
{
	struct ba_info *ba;         /* pointer to general info structure */
	struct ba_watchdog *mw;     /* pointer to local watchdog structure */

#ifdef DEBUG 
	BADBUG(("Entering ba_watch\n"));
#endif
	mw = (struct ba_watchdog *)wp;  /* type cast to local watchdog struct*/
	ba = (struct ba_info *)(mw->ba_pointer);/* get pointer to info struct*/
	tstop(ba->piowtrb);                     /* stop the timer */
	tstop(ba->irpttrb);                     /* stop the timer */
	tstop(ba->cmdtrb);                      /* stop the timer */
	if (!ba->w_waitcc) {
		/*
		 * if we are not already waiting for a
		 * Hard Reset to complete, then issue the Hard
		 * Reset now, and set flag stating we are waiting
		 * for a reset to complete 
		 */
		ba_issue_cmd(ba, (int)BA_HARD_RESET, (ushort)BA_DISK);
		ba->w_waitcc = TRUE;    
	}

#ifdef DEBUG 
	BADBUG(("Exiting ba_watch\n"));
#endif
	return;
}


