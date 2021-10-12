static char sccsid[] = "@(#)45  1.19.6.9  src/bos/kernext/disk/scdiskb.c, sysxdisk, bos41J, 9521A_all 5/23/95 22:11:09";
/*
 * COMPONENT_NAME: (SYSXDISK) SCSI Disk Device Driver Bottom Half Routines
 *
 * FUNCTIONS:	scdisk_build_cmd		scdisk_build_error
 * 		scdisk_cdt_func 		scdisk_cmd_alloc
 * 		scdisk_cmd_alloc_disable 	scdisk_coalesce
 * 		scdisk_d_q_cmd 			scdisk_dump
 *		scdisk_dump_write 		scdisk_process_dmp_error
 *		scdisk_process_dmp_sns 		scdisk_dmp_start_unit
 * 		scdisk_fail_disk 		scdisk_format_mode_data
 * 		scdisk_free_cmd 		scdisk_free_cmd_disable
 * 		scdisk_iodone 			scdisk_log_error
 * 		scdisk_mode_data_compare 	scdisk_mode_select
 * 		scdisk_mode_sense 		scdisk_pending_dequeue
 * 		scdisk_pending_enqueue 		scdisk_prevent_allow
 * 		scdisk_prevent_allow_disable 	scdisk_process_adapter_error
 * 		scdisk_process_buf 		scdisk_process_buf_error
 * 		scdisk_process_diagnostic_error scdisk_process_error
 * 		scdisk_process_good 		scdisk_process_ioctl_error
 * 		scdisk_process_reqsns_error 	scdisk_process_reset
 * 		scdisk_process_reset_error 	scdisk_process_scsi_error
 * 		scdisk_process_sense 		scdisk_process_special_error
 * 		scdisk_q_cmd 			scdisk_q_mode
 * 		scdisk_read_cap 		scdisk_recover_adap_q
 * 		scdisk_release 			scdisk_release_disable
 * 		scdisk_request_sense 		scdisk_reserve
 * 		scdisk_sleep 			scdisk_start
 * 		scdisk_start_disable 		scdisk_start_unit
 * 		scdisk_start_unit_disable 	scdisk_start_watchdog
 * 		scdisk_strategy 		scdisk_test_unit_ready
 * 		scdisk_test_unit_ready_disable 	scdisk_trc
 * 		scdisk_trc_disable 		scdisk_watchdog
 * 		scdisk_write_verify 		scdisk_raw_iodone
 *		scdisk_read_disc_info		scdisk_pm_handler
 *		scdisk_dmp_reqsns		scdisk_pm_watchdog
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Included System Files: */


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/i_machine.h>
#include <sys/systm.h>
#include <sys/m_param.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/devinfo.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/watchdog.h>
#include <sys/lvdd.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/iostat.h>

#include <sys/scdisk.h>
#include <sys/scdiskdd.h>
#include <sys/pm.h>




/* End of Included System Files */

/************************************************************************/
/*									*/
/*			Static Structures				*/
/*									*/
/*	The scdisk_list data structure is an array of pointers to	*/
/*	the first diskinfo structure of a NULL terminated linked	*/
/*	list of diskinfo structures describing each of the disk		*/
/*	device's currently configured in the system.			*/
/*									*/
/*	The scdisk_open_list data structure is similar to the disk_list	*/
/*	the only exception being that it contains only those disk	*/
/*	devices currently open on the system.				*/
/*									*/
/*	The scdisk_info data structure is a structure used for storing	*/
/*	general disk driver information, including global states,	*/
/*	configuration counts, global locks, etc.			*/
/*									*/
/*	The diskinfo data structures are allcoated on a one per device	*/
/*	basis and are used for storing device specific information.	*/
/*	These structures are dynamically allocated when a device is	*/
/*	configured into the system, and deallocated when the device	*/
/*	is deleted. These structures are linked into the disk_list	*/
/*	upon configuration and linked into the disk_open_list during	*/
/*	any outstanding opens.						*/
/*									*/
/************************************************************************/

struct scdisk_info	scdisk_info = {
	0, 0, LOCK_AVAIL, NULL };
struct scdisk_diskinfo	*scdisk_list[DK_HASHSIZE] = {
	NULL };
struct scdisk_diskinfo	*scdisk_open_list[DK_HASHSIZE] = {
	NULL };

/*
 * To allow hardware folks to turn off device driver thresholding
 */
int scdisk_threshold;
/* 
 * offset into diskinfo structure where our formatted mode data is located.
 * This is also for the hardware folks.
 */
int scdisk_mode_data_offset; 

   
#ifdef DEBUG
int			scdisk_debug = 0;



uint *scdisk_trace;


/*
 ******  strings for Internal Debug Trace Table *******
 * To use compile with the flags SC_GOOD_PATH and SC_ERROR_PATH
 * the variable scdisk_trace will be the beginning of the trace table in
 * memory.  The variable scdisk_trctop will point to location where the
 * next entry in the trace table will go.
 */

char      *topoftrc     = "*DKTOP**";

char     *strategy      = "STRATEGY";
char     *insertq       = "INSERT_Q";
char     *dequeue       = "DEQUEUE ";
char     *start         = "START   ";
char     *coales        = "COALESCE";
char     *relocate      = "RELOCATE";
char     *interrupt     = "INTR    ";
char     *good          = "GOOD	   ";
char     *error         = "ERROR   ";
char     *sdiodone      = "IODONE  ";
char     *rw_iodone     = "RAWIODON";
char     *bufretry      = "BUFRETRY";
char     *failbuf       = "FAIL_BUF";
char     *faildisk      = "FAILDISK";
char     *scsierror     = "SCSI_ERR";
char     *adaperror     = "ADAP_ERR";
char     *processbuf    = "BUFCMPLT";
char     *processreset  = "RESETCMP";
char     *qcmd          = "Q_CMD   ";
char     *dqcmd         = "D_Q_CMD ";
char     *cmdalloc      = "CMDALLOC";
char     *cmdfree       = "FREE_CMD";
char     *cmdfail       = "FAIL_CMD";
char     *prevent       = "PRV_ALLOW";
char     *startunit     = "STRTUNIT";
char     *testunit      = "TESTUNIT";
char     *reqsns        = "REQSNS  ";
char     *reserve       = "RESERVE ";
char     *release       = "RELEASE ";
char     *modesense     = "M_SENSE ";
char     *modeselect    = "M_SELECT";
char     *readcap       = "READ_CAP";
char	 *rdiscinfo	= "READINFO";
char     *inquiry       = "INQUIRY ";
char     *scsisense     = "SCSISENS";
char     *logerr        = "LOG ERR ";
char     *cmdtimer	= "TIMER   ";
char     *writeverify   = "WRIT_VER";
char     *specialerr	= "SPEC_ERR";
char     *reqsnserror   = "REQS_ERR";
char     *buildcmd	= "BUILDCMD";
char	 *diagerr	= "DIAGERR ";
char	 *ioctlerr	= "IOCTLERR";
char     *reseterr	= "RESETERR";
char     *sttimer	= "STRT_TMR";
char	 *issue		= "ISSUE   ";
char	 *dump		= "DUMP    ";
char	 *dmpreqsns	= "DMPREQSN";
char	 *dmpstart	= "DMPSTRTU";
char	 *dmpsns	= "DMPSENSE";
char	 *dmperr	= "DMPERROR";
char	 *dmpwrt	= "DMPWRITE";
char	 *dmpissue	= "DMPISSUE";
char	 *pmh_trc	= "PM_HANDL";
char	 *pmh_timer	= "PM_TIMER";

char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */
#endif


/************************************************************************/
/*									*/
/*	NAME:	scdisk_strategy						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Strategy Routine.		*/
/*		This routine is called with a NULL terminated linked	*/
/*		list of buf structs. It scans the incoming list in an	*/
/*		attempt to coalesce contiguous write operations to the	*/
/*		same device together in a single scsi operation.	*/
/*		Coalesced requests are placed in an IN_PROGRESS queue	*/
/*		where they can be processed by scdisk_start. Requests	*/
/*		that are not coalesced are placed in a PENDING queue	*/
/*		where they will be processed by scdisk_start in future	*/
/*		IO operations.						*/
/*									*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use disable_lock, unlock_enable.			*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*		The device's busy flag is set and cleared during	*/
/*		coalescing to insure the request queues integrity	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		scdiskinfo	General disk driver information		*/
/*		scdisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		bp	- Address of first buf struct in list		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned in the	*/
/*		buf struct b_error field:				*/
/*		EFAULT	- Device not open				*/
/*			- Illegal transfer request			*/
/*		ENXIO	- Invalid disk block address			*/
/*		EINVAL	- Attempt to reassign on read operation		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		unlock_enable	disable_lock				*/
/*		iodone							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_strategy(
struct buf	*bp)
{
	int			hash_key, blkno, count,rc;
	int			opri, last_blk;
	dev_t			devno, first_devno;
	struct buf		*next_bp;
	struct scdisk_diskinfo	*diskinfo, *last_diskinfo = NULL;
	struct scdisk_stats	*stats;






	first_devno = bp->b_dev;
	last_diskinfo = NULL;




	DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_STRATEGY, 0, first_devno, bp,
	    bp->b_flags, bp->b_blkno, bp->b_bcount);
		
	while (bp != NULL)  {
		devno = bp->b_dev;
		count = bp->b_bcount;
		next_bp = bp->av_forw;


		/* Locate diskinfo structure in scdisk_open_list */
		hash_key = (minor(devno) & DK_HASH);
		diskinfo = scdisk_open_list[hash_key];
		while (diskinfo != NULL) {
			if (diskinfo->devno == devno)
				break;
			diskinfo = diskinfo->next_open;
		}
 
		if (diskinfo == NULL) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			bp->b_resid = bp->b_bcount;

			iodone(bp);
		}
		else {
	
#ifdef _POWER_MP
			opri = disable_lock(INTTIMER,
					    &(diskinfo->spin_lock));	 
#else
			opri = i_disable(INTTIMER);
#endif

#ifdef DEBUG
#ifdef SC_GOOD_PATH
			scdisk_trc(diskinfo,strategy, entry,(char)0, 
				   (uint)bp, (uint)bp->b_dev,
				   (uint)bp->b_blkno, (uint)bp->av_forw,
				   (uint)0);
#endif	
#endif
			last_blk = diskinfo->capacity.lba;
			diskinfo->cmd_pending = TRUE;


		  	SCDISK_GET_LBA(blkno,diskinfo,bp,rc);
			if (rc) {
			  	/*
			   	 * Request does not start on device
				 * block boundary.  
                                 * Fail any buf if diskinfo->block_size
                                 * is not a multiple of DK_BLOCKSIZE,
                                 * since there is no way to convert
                                 * it to an integral block number
                                 * for this device's block size.
                                 */
                                bp->b_flags |= B_ERROR;
                                bp->b_error = EINVAL;
                                bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if (bp->b_bcount > diskinfo->max_request) {
				/* 
				 * Fail any buf that is greater then our 
				 * maximum transfer size or
				 * 
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if (diskinfo->mode == DK_DIAG) {
				bp->b_flags |= B_ERROR;
				bp->b_error = EACCES;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if (blkno > (last_blk+1)) {
				/*
				 * Verify block address on device
				 */
				bp->b_resid = bp->b_bcount;
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);
			}
			else if (blkno == (last_blk+1)) {
				bp->b_resid = bp->b_bcount;
				if (!(bp->b_flags & B_READ)) {
					bp->b_flags |= B_ERROR;
					bp->b_error = ENXIO;
				}
				iodone(bp);
			}
			else if (bp->b_bcount == 0) {
				/*
				 * Check for zero length read or write
				 */
				bp->b_resid = 0;
				iodone(bp);
			}
			else if ((diskinfo->state == DK_OFFLINE) ||
				 (diskinfo->starting_close)) {
				/*
				 * Verify transfer validity
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EIO;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if ((count & (diskinfo->block_size -1 )) 
				 || ((bp->b_flags & B_READ) &&
				     (bp->b_options & (HWRELOC | UNSAFEREL)))) {
				/*
				 * Only allow reassign on write requests 
				 * and multiples of our block size 
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else if ((bp->b_options & (HWRELOC | UNSAFEREL)) &&
				 ((bp->b_bcount > diskinfo->block_size) ||
				  (diskinfo->safe_relocate == DK_NO_RELOCATION) 
				  || ((bp->b_options & HWRELOC) && 
				      (diskinfo->safe_relocate == 
				       DK_UNSAFE_RELOCATION)))) {
				bp->b_flags |= B_ERROR;
				bp->b_error = EINVAL;
				bp->b_resid = bp->b_bcount;
				iodone(bp);
			}
			else {
				/*
				 * If we got here then we passed all 
				 *  of the above checks
				 */


				/* 
				 * Filter out operation beyond last block 
				 * of disk 
				 */
				if ((blkno + (count / diskinfo->block_size)) > 
				    (last_blk + 1)) {

					bp->b_resid = 
					  ((blkno + 
					    (count / diskinfo->block_size))
					   - (last_blk + 1)) * 
					     diskinfo->block_size;

				} else {
					bp->b_resid = 0x00;
				}
				
				/* Update transfer statistics for device */
				stats = &(diskinfo->stats);
				if ((bp->b_flags & B_READ) &&
				    ((stats->byte_count += bp->b_bcount) >
				     stats->segment_size)) {

					stats->segment_count++;
					stats->byte_count %= 
						stats->segment_size;
				}
				
				
				if ((diskinfo != last_diskinfo) &&
				    (last_diskinfo != NULL)) {
					scdisk_start(last_diskinfo);
				}
				
				
				/*
				 * Put buf on this disk's pending queue
				 */
				scdisk_pending_enqueue(diskinfo, 
						       bp);
				
				

			}
			/* 
			 * Enable intrs and unlock for next 
			 * pass thru 
			 */

#ifdef _POWER_MP
			unlock_enable(opri,&(diskinfo->spin_lock));
#else
			i_enable(opri);
#endif
		} /* else: diskinfo != NULL */


		last_diskinfo = diskinfo;
		bp = next_bp;

		if (diskinfo != NULL) {
			diskinfo->cmd_pending = FALSE;
		}
	} /* while */

	if (diskinfo != NULL) {
		/*
		 * Call start for the last disk processed above
		 */
		scdisk_start_disable(diskinfo);

	
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_STRATEGY, 0, first_devno);
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
		scdisk_trc_disable(diskinfo,strategy, exit,(char)0, 
			   (uint)diskinfo, (uint)0,
			   (uint)0,(uint)0,(uint)0);
#endif	
#endif
#endif



	}
	return(0);
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_pending_enqueue					*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Pending Enqueue Routine	*/
/*		This routine is called with a diskinfo structure pointer*/
/*		and a pointer to a buf structure to insert into the    	*/
/*		disk's pending queue. The insertion into the list is    */
/*		based on the target logical block address of the request*/
/*		The list is ordered in ascending logical block address  */
/*		to provide seek optimization when processing disk 	*/
/*		requests.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use disable_lock, unlock_enable.			*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		buf		System I/O Request Buffer Structure	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	- Pointer to diskinfo structure		*/
/*		bp		- Pointer to buf to insert		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:		None				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:	None				*/
/*									*/
/*									*/
/************************************************************************/

void scdisk_pending_enqueue(
struct scdisk_diskinfo	*diskinfo, 
struct buf *bp)	
{	

	char	done = FALSE;
	struct buf *curr,
		   *prev;


        curr = diskinfo->low;     /* set current pointer to low_cyl pointer */ 
        prev = NULL;            /* set previous pointer to NULL */

        if ( curr == NULL) {
		/*
		 * we are starting with an empty list 
		 */
		diskinfo->low = bp;	/* set low ptr to this one */
		diskinfo->currbuf = bp;/* set current ptr to this one */
                bp->av_forw = NULL;  /* set this ones forward (NULL) */
                bp->av_back = NULL;  /* set this ones back (NULL) */
		return;
	}
		
	do {
		/*
		 * while not at end of list and not done. We can 
		 * use the buf's 512 blkno since the order still remains
		 * the same even if the device's block size is not 512.
		 */
                if (curr->b_blkno <= bp->b_blkno) {
			/*
			 * if this one is less or equal to the one we're trying
			 * to insert set prev to this one, and walk the
			 * curr pointer to the next one
			 */
			prev = curr;
			curr = curr->av_forw;
                } else {
			/*
			 * else it should go before this one 
			 */
                        if ( prev == NULL) {
				/*
				 * becomes new low request
				 */
				diskinfo->low = bp;
				bp->av_back = NULL;
			} else {
				prev->av_forw = bp;
				bp->av_back = prev;
			}
			bp->av_forw = curr;
			curr->av_back = bp;
			done = TRUE;
        	}
        
	} while ((curr != NULL) && (!done)) ;        

        if ( curr == NULL) {
		/*
		 * we're out of the while, so if curr is NULL, then we
		 * walked to the end of the list
		 */
		prev->av_forw = bp; /* set prev's forward ptr to this */
		bp->av_back = prev;	/* set back pointer to prev */
                bp->av_forw = curr;  /* set this ones forward to curr (NULL) */
        }

	return;
}
	
	

/************************************************************************/
/*									*/
/*	NAME:	scdisk_pending_dequeue					*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Pending Dequeue Routine	*/
/*		This routine is called with a diskinfo structure 	*/
/*		pointer.  This routine is called to remove the current  */
/*		buf from the pending queue. 				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called on either a process		*/
/*		or on an interrupt level.				*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned. Critical region protection	*/
/*		must use disable_lock, unlock_enable.			*/
/*									*/
/*	NOTES:								*/
/*		This routine is part of the bottom half of the driver.	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		buf		System I/O Request Buffer Structure     */
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Pointer to disk information structure   */	
/*									*/
/*	RETURN VALUE DESCRIPTION:	None				*/
/*									*/
/*	ERROR DESCRIPTION:		None				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:	None				*/
/*									*/
/*									*/
/************************************************************************/

void scdisk_pending_dequeue(
struct scdisk_diskinfo *diskinfo)
{				
	struct buf *bp;
	
	bp = diskinfo->currbuf;
	/*
	 * set current to next
	 */
	diskinfo->currbuf = bp->av_forw;	
	if (bp == diskinfo->low)  
		/*
		 * if this was our low buf, set low to next
		 */
		diskinfo->low = bp->av_forw;

	if (bp->av_forw != NULL)
		/*
		 * if this not end, set next in list back link
		 */
		bp->av_forw->av_back = bp->av_back;
	if (bp->av_back != NULL)
		/*
		 * if this has a previous link, set it to next
		 */
		bp->av_back->av_forw = bp->av_forw;

	if (diskinfo->currbuf == NULL)	
		/*
		 * if at end of elevator, goto to first floor
		 */
		diskinfo->currbuf = diskinfo->low;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_dump						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is the entry point for the system		*/
/*		to utilize the disk as the dump device.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any intrs	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*                        WRITE(6) Command        			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (0Ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | (MSB)                            |	*/
/*  |-----+-----------------------                             ---|	*/
/*  | 2   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 4   |                    Transfer Length                    |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 5   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        WRITE(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (2ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		scdisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- address of uio struct describing operation	*/
/*		cmd	- operation to be performed			*/
/*		arg	- address of caller argument structure		*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*		EINVAL	- invalid request				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devdump							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dump(
dev_t		devno,
struct uio	*uiop,
int		cmd, 
int		arg, 
int		chan, 
int		ext)
{
	struct  dmp_query	*dump_ptr, tmp_buf;
	struct  iovec		*iovp;
	struct  scdisk_diskinfo	*diskinfo;
	struct  sc_buf		*scbuf;
	struct  dk_cmd		cmd_ptr,*cmd_ptr2;
	int			dev, hash_key, i, lba, blk_cnt, errnoval, opri;



	/*
         *  Check for a valid minor number.
         */
	errnoval = 0x00;
	bzero((char *) &cmd_ptr, (int)sizeof(struct dk_cmd));
	dev = minor(devno);


	/*
         * Locate diskinfo structure in opened device list.
         */
	hash_key = dev & DK_HASH;
	diskinfo = scdisk_open_list[hash_key];
	while (diskinfo != NULL) {
		if (diskinfo->devno == devno)
			break;
		diskinfo = diskinfo->next_open;
	}

	/*
         *  Return an error if the disk has not been opened.
         */
	if (diskinfo == NULL) {

		return(ENXIO);
	}



	switch (cmd) {
	case DUMPINIT:
		/*
                 *  Initialize the adapter driver for a dump.  No other action
                 *  is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno,0,DUMPINIT,
				   arg,chan,ext);
		if (!errnoval) {
			diskinfo->dump_inited = TRUE;
		}
		break;
	case DUMPSTART:
		/*
                 *  Tell the adapter driver a dump is about to happen. No other
                 *  action is necessary.  This will also prevent start from
		 *  issuing any more commands
                 */
		diskinfo->state |= DK_DUMP_PENDING;
		
		if ((diskinfo->pmh.handle.mode == PM_DEVICE_SUSPEND) ||
		    (diskinfo->pmh.handle.mode == PM_DEVICE_HIBERNATION)) {
			/*
			 * If this dump is a result of hibernate
			 * then we need to make sure the
			 * dump device is powered on.
			 */
			pm_planar_control(diskinfo->pmh.handle.devno,
					  diskinfo->pm_device_id, 
					  PM_PLANAR_ON);
		}

		if (ext == PM_DUMP) {
			errnoval = devdump(diskinfo->adapter_devno,0,
					   DUMPSTART,
					   IDLUN(diskinfo->scsi_id, 
						 diskinfo->lun_id),
					   chan,ext);
		} else {
			errnoval = devdump(diskinfo->adapter_devno,0,
					   DUMPSTART,
					   arg,chan,ext);
		}
		break;
	case DUMPQUERY:
		/*
                 *  Return blocksize and maximum transfer size to the caller.
                 */
		dump_ptr = (struct dmp_query *) arg;
		errnoval = devdump(diskinfo->adapter_devno, 0, DUMPQUERY,
		    &tmp_buf, chan, ext);
		if (errnoval == 0) {
			if (tmp_buf.min_tsize > diskinfo->block_size) {
				errnoval = EINVAL;
			}
			else {
				dump_ptr->min_tsize = diskinfo->block_size;
				if (tmp_buf.max_tsize>diskinfo->max_request) {
					dump_ptr->max_tsize = 
					    diskinfo->max_request;
				}
				else {
					dump_ptr->max_tsize=tmp_buf.max_tsize;
				}
			}
		}
		break;
		
	case DUMPWRITE:
		/*
                 *  A dump is in progress.  We do not have to worry about
                 *  breaking up large commands, so just fill in an sc_buf
                 *  and issue the command via devdump for each iovec.
                 */
		iovp = uiop->uio_iov;
		lba = uiop->uio_offset / diskinfo->block_size;
		for (i = 0; i < uiop->uio_iovcnt; i++) {
			if (((int) iovp->iov_len & 0x1ff) || 
			    (!(diskinfo->state & DK_ONLINE))) {

				return(EINVAL);
			}

			blk_cnt = iovp->iov_len / diskinfo->block_size;

			scbuf = &(cmd_ptr.scbuf);
			scbuf->bufstruct.b_flags = B_WRITE;
			scbuf->bufstruct.b_dev = diskinfo->adapter_devno;
			scbuf->bufstruct.b_blkno = lba;
			scbuf->bufstruct.b_un.b_addr = iovp->iov_base;
			scbuf->bufstruct.b_bcount = iovp->iov_len;
			scbuf->bufstruct.b_error = 0x00;
			scbuf->bufstruct.b_resid = 0x00;
			scbuf->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
			scbuf->bp = NULL;
			scbuf->scsi_command.scsi_id = diskinfo->scsi_id;
			cmd_ptr.diskinfo = diskinfo;

			cmd_ptr.type = DK_BUF;
			cmd_ptr2 = &cmd_ptr;
			SCDISK_SET_CMD_LUN(cmd_ptr2,diskinfo->lun_id);
			scbuf->scsi_command.flags =(ushort)diskinfo->async_flag;
			if ((blk_cnt & 0xffffff00) || (lba & 0xffe00000)) {
				scbuf->scsi_command.scsi_length = 10;
				scbuf->scsi_command.scsi_cmd.scsi_op_code = 
				    SCSI_WRITE_EXTENDED;
				scbuf->scsi_command.scsi_cmd.scsi_bytes[0] = 
				    ((lba >> 24) & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[1] = 
				    ((lba >> 16) & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[2] = 
				    ((lba >> 8) & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[3] = 
				    (lba & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[4] = 
				    0x00;
				scbuf->scsi_command.scsi_cmd.scsi_bytes[5] = 
				    ((blk_cnt >> 8) & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[6] = 
				    (blk_cnt & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[7] = 
				    0x00;
			} else {
				scbuf->scsi_command.scsi_length = 0x06;
				scbuf->scsi_command.scsi_cmd.scsi_op_code = 
				    SCSI_WRITE;
				scbuf->scsi_command.scsi_cmd.lun |= 
				    ((lba >> 16) & 0x1f);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[0] = 
				    ((lba >> 8) & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[1] = 
				    (lba & 0xff);
				scbuf->scsi_command.scsi_cmd.scsi_bytes[2] = 
				    blk_cnt;
				scbuf->scsi_command.scsi_cmd.scsi_bytes[3] = 
				    0x00;
			}
			scbuf->timeout_value = diskinfo->rw_timeout;
			scbuf->q_tag_msg = diskinfo->q_type;
			scbuf->status_validity = 0x00;
			scbuf->scsi_status = 0x00;
			scbuf->general_card_status = 0x00;
			scbuf->flags = SC_RESUME | diskinfo->reset_delay;

			errnoval = scdisk_dump_write(diskinfo,&cmd_ptr,
						     chan,ext);
			
			if (errnoval) {

				return(errnoval);
			}
			/* 
			 * get lba for next iovec struct 
			 */

			lba += iovp->iov_len/ diskinfo->block_size;

			/* 
			 * get next iovec struct 
			 */
			uiop->uio_resid -= iovp->iov_len;
			iovp->iov_len = 0;
			iovp = (struct iovec *) (((int) iovp) + 
			    sizeof(struct iovec));
		}
		break;
	case DUMPEND:
		/*
                 *  Tell the adapter driver the dump is over.  No other action
                 *  is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno,0,DUMPEND,
				   arg,chan,ext);
		diskinfo->state &= ~DK_DUMP_PENDING;
		break;
	case DUMPTERM:
		/*
                 *  Tell the adapter driver that this device will no longer
                 *  be used as a dump device, no other action is necessary.
                 */
		errnoval = devdump(diskinfo->adapter_devno,0,DUMPTERM,
				   arg,chan,ext);
		diskinfo->dump_inited = FALSE;
		break;
	default:
		/*
                 *  Invalid dump command.
                 */
		errnoval = EINVAL;
		break;
	}




	return(errnoval);
}



/************************************************************************/
/*									*/
/*	NAME:	scdisk_dump_write					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is responsible for issue write requests	*/
/*		for DUMPWRITE and doing some minimal error recover	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any intrs	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		sc_buf		SCSI buf used to interface with SCSI	*/
/*				adapter driver.				*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*		write_cmd_ptr	Write request for dump			*/
/*									*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		Any errno values returned from devdump may be returned	*/
/*	       								*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		devdump							*/
/*									*/
/*									*/
/************************************************************************/
int
scdisk_dump_write(
struct scdisk_diskinfo *diskinfo,
struct dk_cmd  *write_cmd_ptr,
int		chan, 
int		ext)
{
	int errnoval = 0;		 /* return code of this routine */
	struct dk_cmd *dmp_cmd_ptr;	 /* command to be issued to 	*/
					 /* SCSI adapter driver		*/

	dmp_cmd_ptr = write_cmd_ptr;


#ifdef DEBUG
#ifdef SC_GOOD_PATH

	/*
	 * On the dump write path interrupts are disable at INTMAX
	 * and the dump device driver locks out all other CPU's.
	 * Thus we can access are trace table without locking here.
	 */
	scdisk_trc(diskinfo, dmpwrt,entry, (char)0, (uint)diskinfo, 
	      (uint)dmp_cmd_ptr, (uint)dmp_cmd_ptr->type, 
	      (uint)dmp_cmd_ptr->retry_count,(uint)0);
#endif	
#endif


	/*
	 * Loop until all retries and error recovery are exhausted
	 */
	
	while (TRUE) {
		


#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,dmpissue,trc, (char)0, 
			   (uint)dmp_cmd_ptr, 
			   (uint)dmp_cmd_ptr->type,
			   (uint)dmp_cmd_ptr->subtype, 
			   (uint)dmp_cmd_ptr->retry_count,
			   (uint)dmp_cmd_ptr->status);
#endif	
#endif
		

   
   		/*
		 * Issue a command to SCSI adapter driver's
		 * dump entry point.
		 */
		
		errnoval = devdump(diskinfo->adapter_devno,
				   0,DUMPWRITE,&(dmp_cmd_ptr->scbuf),
				   chan,ext);

		


#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,dmpissue,trc, (char)1, 
			   (uint)dmp_cmd_ptr, 
			   (uint)dmp_cmd_ptr->type,
			   (uint)dmp_cmd_ptr->subtype, (uint)errnoval,
			   (uint)dmp_cmd_ptr->status);
#endif	
#endif
		

		if (!errnoval) {

			if (dmp_cmd_ptr->type == DK_BUF) {

				/*
				 * If type is DK_BUF, then this must be
				 * original dump write command.
				 */
			  	if (dmp_cmd_ptr->scbuf.bufstruct.b_resid != 0 ) {
				  	/*
					 * This command completed with
					 * a resid so we must retry
					 * it.
					 */

				  	dmp_cmd_ptr = write_cmd_ptr;
					dmp_cmd_ptr->retry_count++;

				} else {
				       	/*
					 * This command completed successfully
					 * break and return
					 */
				  	break;  
				}

 
			} else if (dmp_cmd_ptr->type == DK_REQSNS) {

				/*
				 * This is a successful completion on a
				 * request sense.  Parse sense data
				 * and determine error recovery action.
				 */
				
				if (scdisk_process_dmp_sns(diskinfo,
							   &dmp_cmd_ptr,
							   write_cmd_ptr) 
				    == FALSE) {
					/*
					 * This was a recovered error
					 * so return good completion 
					 */
					break;
				}

					
				
			} else if (dmp_cmd_ptr->type == DK_RESET) {
				/*
				 * Dump will only issue one type of
				 * DK_RESET command and it is a start
				 * unit command.
				 *
				 * The start unit completed successfully.
				 * So now retry the original dump write
				 * command.
				 */
				dmp_cmd_ptr = write_cmd_ptr;

			} else {
				/* 
				 * This shouldn't happen, but just in
				 * case it does, lets retry the original
				 * dump write command.
				 */
				ASSERT(FALSE);
				dmp_cmd_ptr = write_cmd_ptr;
			}
			
		} else {
			/*
			 * An error occurred on the DUMPWRITE call
			 * to the adapter driver.  Determine type of
			 * error.
			 */
			dmp_cmd_ptr->retry_count++;
			scdisk_process_dmp_error(diskinfo,
						 &dmp_cmd_ptr,
						 write_cmd_ptr);
		}
		if (write_cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/*
			 * When we've exceeded our retries on the original
			 * command, then exit from the loop.  We should
			 * never loop forever, because the dump error
			 * recovery will eventually retry the original command
			 * and hence increment retry_count.
			 */



#ifdef DEBUG
#ifdef SC_GOOD_PATH
			scdisk_trc(diskinfo,dmpwrt,trc, (char)0, 
				   (uint)diskinfo, 
				   (uint)dmp_cmd_ptr,
				   (uint)dmp_cmd_ptr->retry_count, 
				   (uint)write_cmd_ptr->retry_count,
				   (uint)0);
#endif	
#endif


			if (!errnoval) {
			  	/*
				 * If errnoval is not set (for
				 * example due to a resid failure),
				 * then set errnoval to EIO.
				 */
			  	errnoval = EIO;
			}
			break;

		}

	}
	


#ifdef DEBUG
#ifdef SC_GOOD_PATH
	scdisk_trc(diskinfo,dmpwrt,exit, (char)0, (uint)diskinfo, 
		   (uint)dmp_cmd_ptr,(uint)dmp_cmd_ptr->type, 
		   (uint)dmp_cmd_ptr->retry_count,
		   (uint)errnoval);
#endif	
#endif


	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_dmp_error				*/
/*									*/
/*	FUNCTION:							*/
/*		When an error on a write request for dump occurs, this	*/
/*		routine determines the type of failure and initiates	*/
/*		the corresponding error recovery 			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any intrs	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine only does minimal error recovery.		*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		sc_buf		SCSI buf used to interface with SCSI	*/
/*				adapter driver.				*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*		dmp_cmd_ptr	Command that was issued to adapter	*/
/*				driver.					*/
/*		write_cmd_ptr	Write request for dump			*/
/*									*/
/*									*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*									*/
/*		None.							*/
/*									*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/*		None							*/
/*									*/
/*									*/
/************************************************************************/
void
scdisk_process_dmp_error(
struct scdisk_diskinfo *diskinfo,
struct dk_cmd  **dmp_cmd_ptr,
struct dk_cmd *write_cmd_ptr)
{
        struct sc_buf   *scbuf;		/* SCSI buf 		*/





        scbuf = &((*dmp_cmd_ptr)->scbuf);

        /*
         * Clear any flags that may be
         * set.  
         */

        scbuf->flags = 0;
	scbuf->adap_q_status = 0;



#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmperr,entry, (char)0, (uint)diskinfo, 
		   (uint)dmp_cmd_ptr,(uint)scbuf->status_validity, 
		   (uint)scbuf->scsi_status,
		   (uint)(*dmp_cmd_ptr)->type);
#endif	
#endif


	if ((scbuf->status_validity & SC_SCSI_ERROR) && 
	    (scbuf->scsi_status == SC_CHECK_CONDITION)){

                /* Process SCSI check conditions */
		
		if ((*dmp_cmd_ptr)->type == DK_REQSNS) {
                        /*
			 * A check condition on a request sense
			 * occurred. So re-issue the original dump write
			 * command.
			 */

			*dmp_cmd_ptr = write_cmd_ptr;

		} else {
			/* 
			 * If we received a check condition on
			 * a command other then request sense, then
			 * issue a request sense to extract sense
			 * data from the device.
			 */

			if ((*dmp_cmd_ptr)->type != DK_BUF) {
				/*
				 * If this is a check condition on
				 * a command that is not a request
				 * sense and not the original write,
				 * then increment the original write's
				 * retry count.  This will prevent us
				 * from looping forever on error recovery.
				 */

				write_cmd_ptr->retry_count++;
			}

			scdisk_dmp_reqsns(diskinfo,dmp_cmd_ptr);

		}

	} else {

		/*
		 * For non-check condition errors retry original
		 * command.
		 */
		
		*dmp_cmd_ptr = write_cmd_ptr;
	}


#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmperr,exit, (char)0, (uint)0, (uint)dmp_cmd_ptr,
		   (uint)(*dmp_cmd_ptr)->retry_count, (uint)write_cmd_ptr,
		   (uint)(*dmp_cmd_ptr)->type);
#endif	
#endif


	/*
	 * Clear the error from other fields in command.
	 */ 

	(*dmp_cmd_ptr)->scbuf.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
	(*dmp_cmd_ptr)->scbuf.bufstruct.b_error = 0x00;
	(*dmp_cmd_ptr)->scbuf.status_validity = 0;
	(*dmp_cmd_ptr)->scbuf.scsi_status = 0;

	return;

}



/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_dmp_sns					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine parses the request sense data from a 	*/
/* 		check condition that occurs on a dump.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine should be considered part of the bottom	*/
/*		half of the driver. It may not rely on any intrs	*/
/*		and should refrain from using any system services.	*/
/*		It can not page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- address of uio struct describing operation	*/
/*		cmd	- operation to be performed			*/
/*		arg	- address of caller argument structure		*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*		EINVAL	- invalid request				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/*		None							*/
/*									*/
/*									*/
/************************************************************************/
int
scdisk_process_dmp_sns(struct scdisk_diskinfo *diskinfo,
struct dk_cmd  **dmp_cmd_ptr,
struct dk_cmd *write_cmd_ptr)
{
        struct scdisk_req_sense_info    *sense_data; /* Pointer to sense*/
						     /* data buffer     */


	int				issue_another_cmd;
						          
	
	/*
	 * Set the default behavior to be that we will issue another
	 * command to the adapter driver after processing this
	 * sense data.
	 */

	issue_another_cmd = TRUE; 

        /* Get sense data buffer */

        sense_data = (struct scdisk_req_sense_info *) diskinfo->sense_buf;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmpsns,entry, (char)0, (uint)diskinfo, 
		   (uint)dmp_cmd_ptr,(uint)sense_data, 
		   (uint)sense_data->sense_key,
		   (uint)(*dmp_cmd_ptr)->type);
#endif	
#endif

	/*
	 * Parse sense key from sense data to determine
	 * the appropriate error recovery action.
	 */
       
        switch (sense_data->sense_key) {
	    case DK_RECOVERED_ERROR:

		/*
		 * This was a recovered error so return
		 * good completion. We don't need to issue
		 * another command to the adapter driver for this
		 * DUMPWRITE request.
		 */

		issue_another_cmd = FALSE;
		write_cmd_ptr->retry_count = DK_MAX_RETRY;
		break;
	    case DK_NOT_READY:
	    case DK_UNIT_ATTENTION:
		/*
		 * Build start unit command to be issued next, before
		 * retrying the original DUMPWRITE request.
		 */
		scdisk_dmp_start_unit(diskinfo,(uchar)DK_START,
				      dmp_cmd_ptr);
		break;

	    default:
		/* 
		 * Retry original dump command 
		 */
		*dmp_cmd_ptr = write_cmd_ptr;
	}

#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmpsns,exit, (char)0, (uint)diskinfo, 
		   (uint)issue_another_cmd,
		   (uint)sense_data, (uint)sense_data->sense_key,
		   (uint)(*dmp_cmd_ptr)->type);
#endif	
#endif
	return (issue_another_cmd);
		
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_dmp_start_unit					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a START UNIT cmd block for		*/
/*		dump error recovery.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on both interrupt level and      */
/*		process level.  It cannot page fault.			*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/
void
scdisk_dmp_start_unit(
struct scdisk_diskinfo	*diskinfo,
uchar			start_stop_flag,
struct dk_cmd  		**dmp_cmd_ptr)
{
	struct dk_cmd	*cmd_ptr;	/* Start unit command        */
	struct buf	*cmd_bp;	/* Buf in start unit command */
	struct scsi	*scsi;		/* SCSI CDB		     */


	/* Allocate a dump type cmd for this operation */

        cmd_ptr = &(diskinfo->dmp_cmd);

#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmpstart,trc, (char)0, 
		   (uint)diskinfo, (uint)diskinfo->state,
		   (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
		   (uint)cmd_ptr->status);
#endif	
#endif



        cmd_ptr->type = DK_RESET; 
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for start unit cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout;   /* 1 minute */
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;

	/*
	 * Some adapter driver's don't issue a abort for the
	 * the dump device. In this situation we must not mix tags.
	 * So issue the start unit with the same q_type. Otherwise
	 * issue it with SC_NO_Q.
	 */
	
	if (diskinfo->cmds_out) {
		/*
		 * Commands are still outstanding via the normal
		 * path. So issue this start unit with the same
		 * q_type.
		 */ 
		cmd_ptr->scbuf.q_tag_msg = diskinfo->q_type;
	} else {
		/*
		 * No commands are outstanding. So set q_tag_msg
		 * to SC_NO_Q.
		 */
		cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	}
	
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

      /*                   START STOP UNIT Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (1Bh)                |
       *|-----+---------------------------------------------------------------|
       *| 1   | Logical Unit Number   |                  Reserved     | Immed |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved            |  LoEj |  Start|
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Control                             |
       *+=====================================================================+
       */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	if (start_stop_flag == DK_START) {
		scsi->scsi_cmd.scsi_bytes[2] = 0x01;
	} else {
		scsi->scsi_cmd.scsi_bytes[2] = 0x00;

	}

	scsi->scsi_cmd.scsi_bytes[3] = 0x00;


	cmd_ptr->subtype = start_stop_flag;

	/* Update dump command pointer */
	*dmp_cmd_ptr = cmd_ptr;

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_dmp_reqsns					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a request sense command block for	*/
/*		for dump error recovery.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bzero							*/
/*									*/
/************************************************************************/

void
scdisk_dmp_reqsns(
struct scdisk_diskinfo	*diskinfo,
struct dk_cmd  **dmp_cmd_ptr)
{
	struct dk_cmd	*cmd_ptr;	/* Request sense command     */
	struct buf	*cmd_bp;	/* Buf in start unit command */
	struct scsi	*scsi;		/* SCSI CDB		     */



	/* Allocate a REQUEST type cmd for this operation */




        cmd_ptr = &(diskinfo->dmp_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmpreqsns, entry,(char)0, 
		   (uint)diskinfo, (uint)dmp_cmd_ptr,
		   (uint)(*dmp_cmd_ptr)->status, (uint)0,
		   (uint)0);
#endif	
#endif

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_REQSNS; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for request sense cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = DK_REQSNS_LEN;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                        REQUEST SENSE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (03h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |         Reserved                      |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Allocation Length                   |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_REQUEST_SENSE;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = DK_REQSNS_LEN;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	/* Clear out the sense data buffer */
	bzero(diskinfo->sense_buf, DK_REQSNS_LEN);




	/* Update dump command pointer */
	*dmp_cmd_ptr = cmd_ptr;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
	scdisk_trc(diskinfo,dmpreqsns, exit,(char)0, 
		   (uint)diskinfo, (uint)cmd_ptr,
		   (uint)0, (uint)0,(uint)0);
#endif	
#endif
	return;

}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_start_disable					*/
/*									*/
/*	FUNCTION: Calls start for pageable routines			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called at process level.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo - Address of disk information structure	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		disable_lock	unlock_enable				*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_start_disable(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr; 		/* command block pointer*/
	int			opri;


#ifdef _POWER_MP
	opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	opri = i_disable(INTTIMER);
#endif
	scdisk_start(diskinfo);
#ifdef _POWER_MP
	unlock_enable(opri,&(diskinfo->spin_lock));
#else
	i_enable(opri);
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_start						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Start IO Routine.		*/
/*		This routine will try to queue "queue_depth" commands	*/
/*		to the specified device.  It scans the command queue 	*/
/*		first.  This queue contains all error recovery commands	*/
/*		including normal reads/writes to be retried.		*/
/*									*/
/*		It next sees if there is an ioctl request pending if	*/
/*		so it will issue the request. Next the IN_PROGRESS queue*/
/*		is checked to start any ops that have already been	*/
/*		coalesced, finally the pending queue is checked, and	*/
/*		if it is not empty, scdisk_coalesce is called to	*/
/*		coalesce ops into the IN_PROGRESS queue and		*/
/*		scdisk_build_cmd is called to construct a scsi op	*/
/*		to satisfy the requests in the IN_PROGRESS queue.	*/
/*		When a request has been found and built, the adapter	*/
/*		is called via devstrategy to begin processing of the	*/
/*		operation.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called at both a process.		*/
/*		and an interrupt level.					*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo - Address of disk information structure	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		disable_lock	unlock_enable				*/
/*		devstrat						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_start(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr; 		/* command block pointer     */
	int	         opri;


#ifdef DEBUG
#ifdef SC_GOOD_PATH
	scdisk_trc(diskinfo,start, entry,(char)0, (uint)diskinfo, 
		   (uint)diskinfo->state ,
		   (uint)diskinfo->dk_cmd_q_head, 
		   (uint)diskinfo->ioctl_cmd.flags,
		   (uint)diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

        /*
	 * We will loop until we have queue dept commands queued.  If certain
	 * diskinfo->state's exist then we will only issue on command at a time
	 * or starve certain requests until this condition is cleared
	 */
	
	while (TRUE) {
		

#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,start, trc,(char)0, 
			   (uint)diskinfo->cmds_out, 
			   (uint)diskinfo->state,
			   (uint)diskinfo->timer_status,
			   (uint)diskinfo->dk_cmd_q_head,
			   (uint)diskinfo->dk_bp_queue.in_progress.head);

		scdisk_trc(diskinfo,start, trc,(char)1, 
			   (uint)diskinfo->queue_depth, 
			   (uint)diskinfo->raw_io_cmd,
			   (uint)diskinfo->q_clr,
			   (uint)diskinfo->ioctl_pending,
			   (uint)diskinfo->ioctl_cmd.flags);

#endif
#endif

		if ((diskinfo->state & DK_RECOV_MASK) ||
		    (diskinfo->cmds_out >= diskinfo->queue_depth) ||
		    (diskinfo->timer_status) ||
		    (diskinfo->q_clr)) {

			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			/*
			 * If we have request sense, reset, dump, queue recovery
			 * or reassign pending or set our timer we do not issue 
			 * any more commands until this completes.  Also if we 
			 * have reached our queue depth do not issue any thing 
			 * else.
			 */


			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}
		if (diskinfo->state & DK_RST_FAILED) { 

			/*
			 * An attempt to do the reset sequence has
			 * failed. Before we issue another command we
			 * must send a start unit and try the
			 * reset cycle over.
			 */
			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);
			if ((diskinfo->starting_close) ||
			    ((diskinfo->dk_cmd_q_head == NULL) &&
			    (!(diskinfo->ioctl_cmd.flags & DK_READY)) &&
			    (diskinfo->dk_bp_queue.in_progress.head 
				                                == NULL) &&
			    (diskinfo->currbuf == NULL)))   { 
				/*
				 * If an attempt is being made to close this
				 * disk so do not send another start unit
				 * down, but state state back to offline.
				 * If a reset failed and we have no commands
				 * pending then exit.  We do not want to
				 * issue the start unit until some other
				 * command is to be sent.
				 */

				DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 
					0, diskinfo->devno);
				return;
			}

			/*
			 *  At some point we detected a unit attention 
			 *  or some other condition that required us
			 *  to go through the reset sequence, but
			 *  the sequence failed. Therefore, before
			 *  any command can be issued, we must
			 *  try again.
			 */
			diskinfo->state = DK_ONLINE;
			scdisk_test_unit_ready(diskinfo);

			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 
					0, diskinfo->devno);
			return;

		} 	
		diskinfo->dkstat.dk_status |= IOST_DK_BUSY;
		if ((diskinfo->dk_cmd_q_head != NULL)) {
			/* 
			 * Start cmd struct on top of command queue.  This
			 * queue is for error recovery commands.
			 */

			cmd_ptr = diskinfo->dk_cmd_q_head;

			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);

			if (cmd_ptr->type & (DK_REQSNS|DK_RESET|DK_WRITEV
					     |DK_REASSIGN|DK_Q_RECOV)) {
				/*
				 * If this is a request sense, reset, 
				 * write verify, reassign or queue recovery 
				 * operation then set the state flag of this 
				 * disk accordingly.  NOTE: The cmd_ptr->type
				 * defines match the corresponding 
				 * diskinfo->state defines.
				 */
				diskinfo->state |= cmd_ptr->type;
			 }

		} else if (diskinfo->state & DK_WRITEV_PENDING) {
			/*
			 * Allow only error recovery commands to be issued if
			 * a write verify is pending
			 */
			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;

		} else if (diskinfo->pm_pending & PM_SCDISK_PENDING_SUSPEND) {
			/*
			 * If we are trying to go into a PM_DEVICE_SUSPEND
			 * or PM_DEVICE_HIBERNATION mode then we don't want
			 * to issue any non-error recovery commands.
			 */
			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
	       
	        } else if (diskinfo->ioctl_cmd.flags & DK_READY) {
			/*
			 * Issue the ioctl that is ready for this
			 * disk
			 */
			if (!diskinfo->cmds_out) {
				/*
				 * If no outstanding commands then issue
				 * ioctl.
				 */
				cmd_ptr = &diskinfo->ioctl_cmd;
				cmd_ptr->flags &= ~DK_READY;
				diskinfo->ioctl_pending = TRUE;
			}
			else {
				/*
				 * Wait for outstanding commands to complete
				 */
				DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
					diskinfo->devno,
					0, 0, 0, 0);

				DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
					diskinfo->devno);
				break;
			}
	        } else if (diskinfo->ioctl_pending ) {
			/*
			 * Allow only error recovery commands to be issued if
			 * an ioctl is pending
			 */
			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno,
				0, 0, 0, 0);

			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;	
                } else if ((diskinfo->raw_io_cmd != NULL)) {

                        /* 
			 * If raw queue has anything pending
			 * start single command structure on this queue 
			 */
                        cmd_ptr = diskinfo->raw_io_cmd;
                        diskinfo->raw_io_cmd = NULL;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
			scdisk_trc(diskinfo,start, trc,(char)2, 
				   (uint)cmd_ptr, 
				   (uint)diskinfo->raw_io_cmd,(uint)0,
				   (uint)0,(uint)0);
#endif
#endif

	

		} else if (diskinfo->dk_bp_queue.in_progress.head != NULL) {
			/* 
			 * If a request was coalesced but there were no 
			 * resources available at the time then we will arrive
			 * here.  We must issue this request(s) before
			 * we call coalesce again.
			 */

			cmd_ptr = 
				scdisk_build_cmd(diskinfo,
				    (struct buf **) 
				     &(diskinfo->dk_bp_queue.in_progress.head),
				     (char) TRUE);


		} else if (diskinfo->currbuf != NULL) {
			/* Coalesce into IN PROGRESS queue and start */
			scdisk_coalesce(diskinfo);
			cmd_ptr = 
				scdisk_build_cmd(diskinfo,(struct buf **)
		                  &(diskinfo->dk_bp_queue.in_progress.head),
				     (char) TRUE);



		} else {
			DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
				diskinfo->devno, 
				0, 0, 0, 0);
			if (!(diskinfo->cmds_out)) {
				/* 
				 * If no more commands outstanding clear 
				 * busy status for this disk
				 */
				diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;
			}

			/*
			 * Nothing on any of our queues so exit this loop
			 */
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
				diskinfo->devno);
			break;
		}


		/*
		 * Issue the command block selected above
		 */
		if (cmd_ptr == NULL) {
			/*
			 * No resources for for DK_BUF commands available
			 * so exit this loop
			 */

			break;
		}	

		DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_BSTART, 0, 
			diskinfo->devno,
			&cmd_ptr->scbuf, 
			cmd_ptr->scbuf.bufstruct.b_flags,
			cmd_ptr->scbuf.bufstruct.b_blkno,
			cmd_ptr->scbuf.bufstruct.b_bcount);
	
		if (diskinfo->restart_unit == TRUE) {
			/*
			 * If this is the first command sent
			 * afer an error occured then set SC_RESUME in the buf
			 */
			cmd_ptr->scbuf.flags |= SC_RESUME | 
				diskinfo->reset_delay;
			diskinfo->restart_unit = FALSE;
		}
		
#ifdef _POWER_MP
   		/*
		 * Prevent the iodoning of this sc_buf from
		 * being funneled.
		 */
   		cmd_ptr->scbuf.bufstruct.b_flags |= B_MPSAFE;
#endif
		cmd_ptr->scbuf.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
		cmd_ptr->scbuf.bufstruct.b_error = 0x00;
		cmd_ptr->status |= DK_ACTIVE;
		

#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,issue, trc,(char)0, (uint)cmd_ptr, 
			   (uint)cmd_ptr->type,(uint)cmd_ptr->subtype,
			   (uint)cmd_ptr->status,
			   (uint)cmd_ptr->retry_count);
#endif
#endif
		cmd_ptr->retry_count++;
		diskinfo->cmds_out++;

		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_BSTART, 0, 
			diskinfo->devno);

                /*
		 * Note: We can't keep our simple lock across calls to 
		 * devstrat, but we must remained disabled.  We can't just
		 * use a simple_unlock, because on UP this will crash.
		 * Instead we will use an unlock_enable with the prioty the
		 * same as our current priority (INTTIMER).
		 */

                /* Power Management activity flag */
                diskinfo->pmh.handle.activity = PM_ACTIVITY_OCCURRED;


#ifdef _POWER_MP
   		unlock_enable(INTTIMER,&(diskinfo->spin_lock));
#endif
		devstrat(cmd_ptr);
#ifdef _POWER_MP
   		opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	
#endif
	}  
#ifdef DEBUG
#ifdef SC_GOOD_PATH
	scdisk_trc(diskinfo,start, exit,(char)0, (uint)diskinfo, 
		   (uint)diskinfo->state ,
		   (uint)diskinfo->dk_cmd_q_head, 
		   (uint)diskinfo->cmds_out,
		   (uint)diskinfo->dk_bp_queue.in_progress.head);
#endif
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_iodone						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver IO Completion Routine.	*/
/*		This routine determines if the indicated operation	*/
/*		completed successfully or failed. If successful,	*/
/*		the appropriate routine is called to process the	*/
/*		specific type of cmd. If failed, the general failure	*/
/*		processing routine is called. Upon exiting this		*/
/*		routine, scdisk_start is called to begin processing     */
/*		of the cmd at the top of the device's cmd stack.      	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns a zero value.			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_iodone(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;
	struct buf		*bp;
	uint	      		old_pri;





	/* Get diskinfo struct */
	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#endif
	bp = &(cmd_ptr->scbuf.bufstruct);
	DDHKWD1(HKWD_DD_SCDISKDD, DD_ENTRY_IODONE, 0, diskinfo->devno);

#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,interrupt, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state ,
	      (uint)diskinfo->cmds_out, (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif
#endif
	/* 
	 * decrement cmds_out  (i.e. the number of outstanding commands)
	 */

	diskinfo->cmds_out--;
	cmd_ptr->status &= ~DK_ACTIVE;
#ifdef _POWER_MP
   		/*
		 * Iodone will not set this for MP_SAFE drivers
		 */
   		cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
#endif

	/* Test for Successful Completion of operation */
	if (cmd_ptr->type == DK_Q_RECOV) {

		if (cmd_ptr->scbuf.flags & SC_Q_RESUME) {
			/*
			 * After Q_RESUME completes we want to start
			 * issuing commands again (i.e we do not want
			 * to continue starving the device).
			 */
			diskinfo->state &=~DK_Q_RECOV_PENDING;
		}
		/*
		 * Free up Q recovery commands immediately
		 * We do not care if the had good completion or
		 * bad completion
		 */
		scdisk_free_cmd(cmd_ptr);
	}
	else if ((bp->b_flags & B_ERROR) || ((bp->b_resid != 0) && 
	    (cmd_ptr->type != DK_REQSNS) && 
	    (cmd_ptr->subtype != DK_MSENSE))) { /* Operation FAILED */
		/*
                 * Call scdisk_process_error to handle error
                 * logging and determine what type of error
                 * recovery must be done.
                 */
		scdisk_process_error(cmd_ptr);
	} else {
		/*
		 * This command completed successfully
		 * process it as such.
		 */
		scdisk_process_good(cmd_ptr);
	}

	if (!(diskinfo->cmds_out)) {
		/* 
		 * If no more commands outstanding clear busy status for this 
		 * disk
		 */
		diskinfo->dkstat.dk_status &= ~IOST_DK_BUSY;
		if (diskinfo->state & DK_Q_RECOV_PENDING) {
			/*
			 * After issuing a Q clear command we must wait for all
			 * outstanding commands to complete before issuing 
			 * anything new.  We must also set have the next issued 
			 * command go out with SC_RESUME set to allow the 
			 * adapter device driver to know we are ready to 
			 * continue.
			 */
			diskinfo->state &=~DK_Q_RECOV_PENDING;
			diskinfo->restart_unit = TRUE;

		}

                /* Do pending PM operation */
                if (diskinfo->pm_pending & PM_SCDISK_PENDING_OP) {
			/*
			 * Our PM handler is waiting on I/O to complete.
			 * So let's wake it up.
			 * NOTE: This will cause the PM handler
			 * to return PM_SUCCESS, since we're
			 * clearing the PM_SCDISK_PENDING_OP flag
			 * here.
			 */
			diskinfo->pm_pending &= ~PM_SCDISK_PENDING_OP;
			e_wakeup((int *)&(diskinfo->pm_event));
                }


#ifdef DEBUG
#ifdef SC_ERROR_PATH
		scdisk_trc(diskinfo,interrupt, trc,(char)0, 
			   (uint)diskinfo, 
			   (uint)diskinfo->state ,
			   (uint)diskinfo->cmds_out, 
			   (uint)0,(uint)0);
#endif
#endif
	}
	/* 
	 * Start all the pending operations that we can.
	 */
	scdisk_start(diskinfo);


#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,interrupt, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state ,
	      (uint)diskinfo->cmds_out, (uint)0,(uint)0);
#endif
#endif
	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_IODONE, 0, diskinfo->devno);

#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#endif


	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_good					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes successful completion of		*/
/*		a SCSI request.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_good(
struct dk_cmd	*cmd_ptr)
{
	struct buf		*bp;
	struct scdisk_diskinfo	*diskinfo;




	if (cmd_ptr->status & DK_QUEUED) {
		/*
		 * Commands with good completion are immediately dequeued 
		 * for processing
		 */
		scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	}
	bp = &(cmd_ptr->scbuf.bufstruct);

	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,good, entry,(char)0, (uint)cmd_ptr->diskinfo, 
	      (uint)cmd_ptr->diskinfo->state ,(uint)cmd_ptr->diskinfo->cmds_out,
	      (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif
#endif
	switch (cmd_ptr->type) {
	case DK_REQSNS:

		scdisk_process_sense(cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsisense, exit,(char)0, (uint)diskinfo->state, 
	      (uint)diskinfo->q_status,(uint)diskinfo->q_clr, 
	      (uint)0,(uint)diskinfo->clr_q_on_error);
#endif
#endif
		if (diskinfo->q_status & SC_DID_NOT_CLEAR_Q) {
			/*
			 * SCSI adapter driver has not cleared its
			 * queue for this device.
			 */	    
			scdisk_recover_adap_q(diskinfo);
		}
		diskinfo->q_clr = FALSE;
		break;
	case DK_RESET:
		scdisk_process_reset(cmd_ptr);
		break;
	case DK_IOCTL:
		cmd_ptr->flags &= ~DK_READY;
                cmd_ptr->intrpt = 0;
                e_wakeup(&(bp->b_event));
		diskinfo->reset_failures = 0;
		diskinfo->ioctl_pending = FALSE;
		break;
	case DK_BUF:
		scdisk_process_buf(cmd_ptr);
		break;
	case DK_WRITEV:
		scdisk_free_cmd(cmd_ptr);
		cmd_ptr = diskinfo->writev_err_cmd;
		diskinfo->state &= ~DK_WRITEV_PENDING;
		scdisk_process_good(cmd_ptr);
		return;
        case DK_REASSIGN:
		scdisk_free_cmd(cmd_ptr);
		diskinfo->state &= ~DK_REASSIGN;
		cmd_ptr = diskinfo->reassign_err_cmd;
		scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
		break;
	default:
		/* Process unknown cmd type */
		ASSERT(FALSE);
	};
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,good, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state ,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine examines the request sense data returned	*/
/*		from a device. It must determine what state the device	*/
/*		is in and what error recovery should be performed	*/
/*		on the device. Recovered and non-recovered errors	*/
/*		must be handled. Recovery may include:			*/
/*			retry operation					*/
/*			abort operation					*/
/*			issue recovery operation			*/
/*									*/
/*		When this routine exits, the device's cmd stack		*/
/*		should either be empty, or contain a cmd to be		*/
/*		started.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*   		SCSI-2 Specifies the following format for Sense Data:   */
/*									*/
/*	         Error Codes 70h and 71h Sense Data Format		*/
/* +=====-======-======-======-======-======-======-======-======+	*/
/* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/* |Byte |      |      |      |      |      |      |      |      |	*/
/* |=====+======+================================================|	*/
/* | 0   | Valid|          Error Code (70h or 71h)               |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 1   |                 Segment Number                        |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 2   |Filema|  EOM |  ILI |Reserv|     Sense Key             |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 3   | (MSB)                                                 |	*/
/* |- - -+---              Information                        ---|	*/
/* | 6   |                                                 (LSB) |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 7   |                 Additional Sense Length (n-7)         |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 8   | (MSB)                                                 |	*/
/* |- - -+---              Command-Specific Information       ---|	*/
/* | 11  |                                                 (LSB) |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 12  |                 Additional Sense Code                 |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 13  |                 Additional Sense Code Qualifier       |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 14  |                 Field Replaceable Unit Code           |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 15  |  SKSV|                                                |	*/
/* |- - -+------------     Sense-Key Specific                 ---|	*/
/* | 17  |                                                       |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 18  |                                                       |	*/
/* |- - -+---              Additional Sense Bytes             ---|	*/
/* | n   |                                                       |	*/
/* +=============================================================+	*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup	errsave					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_sense(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo		*diskinfo;
	struct sc_buf			*scbuf;
	struct buf			*bp;
	struct scdisk_stats		*stats;
	struct scdisk_req_sense_info	*sense_data;
	struct scdisk_ioctl_req_sense	*ioctl_req_sense;
	struct dk_cmd			*log_cmd_ptr;
	uchar				sense_key, sense_code;
	uchar				sense_qualifier, verify_done;
	ulong				error_blkno, recovery_level;
	int				rc;
	int				sense_code_qualifier;/* sense_code    */
							     /* combined with */
							     /*sense_qualifier*/





	/* Get diskinfo struct for this device */
	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

	/* free REQSNS cmd  */

	scdisk_free_cmd(cmd_ptr);
	diskinfo->state &= ~DK_REQSNS_PENDING;

	/* Get address of sense data buffer */
	sense_data = (struct scdisk_req_sense_info *) diskinfo->sense_buf;

	/*
	 * Save the command pointer on top of the stack for logging.
	 * If an error is to be logged, it is for the command on the top of
	 * of the stack.
	 */
	log_cmd_ptr = diskinfo->checked_cmd;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsisense, entry,(char)0, (uint)diskinfo->state, 
	      (uint)sense_data ,(uint)diskinfo->checked_cmd, (uint)0,
	      (uint)diskinfo->dk_cmd_q_head);
#endif
#endif

	/*
	 * Set cmd_ptr to address of checked_cmd
         * This is the cmd that received the Check Condition
         * it is ready for a retry attempt to be performed
         */
	cmd_ptr = diskinfo->checked_cmd;	
	/*
	 * If a Write and Verify received the check and a reset did not
	 * occur then free it and process the command the write verify was 
	 * issued for in the first place
	 */
	if ((cmd_ptr->type == DK_WRITEV) && 
	    (sense_data->sense_key != DK_UNIT_ATTENTION)) {
		diskinfo->state &= ~DK_WRITEV_PENDING;
		verify_done = TRUE;
		cmd_ptr = diskinfo->writev_err_cmd;
		scdisk_free_cmd(diskinfo->checked_cmd);
	} else {
		verify_done = FALSE;
	}
	
	ASSERT(cmd_ptr != NULL);


	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	scbuf = &(cmd_ptr->scbuf);
	bp = &(scbuf->bufstruct);
	stats = &(diskinfo->stats);
	sense_data = (struct scdisk_req_sense_info *) diskinfo->sense_buf;
	sense_key = sense_data->sense_key;
	sense_code = sense_data->add_sense_key;
	sense_qualifier = sense_data->add_sense_qualifier;

	/*
         * If this was a DKIORDSE or DKIOWRSE, transfer the request sense
         * data to the user buffer and set the "valid sense" flag.
         */
	if (cmd_ptr->type == DK_IOCTL) {
		ioctl_req_sense = &(diskinfo->ioctl_req_sense);
		if (ioctl_req_sense->count != 0) {
			rc = xmemout((caddr_t) sense_data,
			    (caddr_t) ioctl_req_sense->buffer,
			    (int) ioctl_req_sense->count,
			    &(ioctl_req_sense->xmemd));
			if (rc == XMEM_SUCC) {
				cmd_ptr->scbuf.status_validity |= 
				    SC_VALID_SENSE;
			}
		}
	}
	/* Test for recovery level on error */

	/* If the sense key is RECOVERED ERROR, HARDWARE ERROR or 
	 * MEDIUM ERROR and if the SKSV bit is one, the sense-key specific 
	 * field shall be defined as shown below:
	 *
	 *              Actual Retry Count Bytes
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|  7   |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+======+================================================|
	 * | 15  | SKSV |              Reserved                          |
	 * |-----+-------------------------------------------------------|
	 * | 16  | (MSB)                                                 |
	 * |-----+---                 Actual Retry Count              ---|
	 * | 17  |                                                 (LSB) |
	 * +=============================================================+
	 */

	recovery_level = ((sense_data->field_ptrM << 0x08) |
	    (sense_data->field_ptrL));

	if (!verify_done) {
		/* Test for LBA valid in sense data */

		if (sense_data->err_code & 0x80) {
			error_blkno = ((sense_data->sense_byte0 << 0x18) |
			    (sense_data->sense_byte1 << 0x10) |
			    (sense_data->sense_byte2 << 0x08) |
			    (sense_data->sense_byte3));

			bp->b_resid = bp->b_bcount - 
			   ((error_blkno - bp->b_blkno) * diskinfo->block_size);
			if (bp->b_resid > bp->b_bcount) 
				bp->b_resid = bp->b_bcount;
		} else {
			error_blkno = 0xffffffff;
			if (sense_data->sense_key != DK_RECOVERED_ERROR) {
				/*
				 * If this was not a recovered error then set
				 * b_resid.  The recovered errors will handle
				 * the b_resid on a case by case basis.
				 */
				
				bp->b_resid = bp->b_bcount;
			}
		}
	}


#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsisense, trc,(char)0, (uint)sense_key,
	      (uint)sense_code, 
	      (uint)sense_qualifier, (uint)recovery_level,(uint)error_blkno);
#endif
#endif


	/* Evaluate sense data for recovery procedure determination */
	switch (sense_data->sense_key) {
	case DK_NO_SENSE:
		/* Process unknown condition */
	  	if ((diskinfo->dev_type == DK_CDROM) &&
		    (sense_code == 0x00) && (sense_qualifier == 0x11)) {
		  	/*
			 * Play Audio operation in progress for CD-ROM
			 * drive.
			 */
			bp->b_error = EINPROGRESS;
			break;
		}
		scdisk_build_error(log_cmd_ptr, DK_UNKNOWN_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
		break;
	case DK_RECOVERED_ERROR:
		/*
                 * Cmd completed successfully with some recovery action
                 * Evaluate recovery level and determine if this op should
                 * be marked as ESOFT for the LVM to schedule for
                 * Possible reassignment.  The log entry for this command
                 * is always done immediately.
                 */
		if (diskinfo->dev_type == DK_CDROM) {
			/*
			 * If this is a cdrom then log all recovered
			 * errors, but return good completion.
			 */
			scdisk_build_error(log_cmd_ptr,DK_RECOVERED_ERR,
					   DK_OVERLAY_ERROR,SC_VALID_SENSE);
			scdisk_log_error(log_cmd_ptr, DK_SOFT_ERROR);
			log_cmd_ptr->error_type = 0;
			scdisk_process_good(cmd_ptr);
			return;
		}
		sense_code_qualifier = ( sense_code * 0x100 ) + sense_qualifier;
		switch ( sense_code_qualifier ) {
		      case 0x0100:
		      case 0x0200:
		      case 0x0800:
		      case 0x0900:
		      case 0x0C01:
		      case 0x1406:
		      case 0x1603:
		      case 0x1706:
		      case 0x1802:
		      case 0x1900:
		      case 0x1C00:
		      case 0x1C01:
		      case 0x1C02:
		      case 0x1F00:
		      case 0x3200:
		      case 0x3201:
		      case 0x3E00:
		      case 0x4400:
		      case 0x5300:
		     		/*
				 *  This error type should not result
				 *  in an ESOFT.
				 */
			  	scdisk_build_error(log_cmd_ptr, 
						   DK_RECOVERED_ERR,
					   DK_OVERLAY_ERROR, SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr, DK_SOFT_ERROR);
				log_cmd_ptr->error_type = 0;
				scdisk_process_good(cmd_ptr);
				return;
		      case 0x0C03:
		      case 0x1405:
		      case 0x1604:
		      case 0x1705:
		      case 0x1707:
		      case 0x1805:
				if (!(sense_data->err_code & 0x80)) {
					/*
					 * Since we did not handle
					 * this case above for
					 * recovered errors we must
					 * for this situation
					 */
					bp->b_resid = bp->b_bcount;
				}
				scdisk_build_error(log_cmd_ptr,
						   DK_RECOVERED_ERR,
						   DK_NOOVERLAY_ERROR,
						   SC_VALID_SENSE);
				bp->b_error = ESOFT;


				scdisk_log_error(log_cmd_ptr,
						 DK_SOFT_ERROR);
				cmd_ptr->retry_count = DK_MAX_RETRY;
				break;
		      case 0x0403:
			       /* 
				* Given this special case, 
				* issue a start unit 
				*/
				if (cmd_ptr->subtype != DK_TUR) {
				/*
				 * If this is not a test unit ready
				 * then log this error.  Test unit
				 * ready is the only valid way
				 * we should hit a 0x0403.
				 */
					scdisk_build_error(log_cmd_ptr, 
						   DK_RECOVERED_ERR,
						   DK_OVERLAY_ERROR, 
						   SC_VALID_SENSE);
					scdisk_log_error(log_cmd_ptr, 
							 DK_SOFT_ERROR);
					log_cmd_ptr->error_type = 0;
				}


				if (diskinfo->reset_count < DK_MAX_RESETS) {
					if (!(cmd_ptr->type & DK_RESET) ||
					    (cmd_ptr->subtype == DK_TUR)) {

						diskinfo->q_clr = TRUE;
						scdisk_start_unit(diskinfo,
							      (uchar) DK_START,
							      (uchar) FALSE);
						return;
					}
					else if (cmd_ptr->retry_count < 
						 DK_MAX_RETRY) {
						
						scdisk_start_watchdog(diskinfo,
								    (ulong)4);
						return;
					}
				}
				break;
		      case 0x1300:
		      case 0x1600:
		      case 0x1601:
		      case 0x1602:
		      case 0x1700:
		      case 0x1701:
		      case 0x1702:
		      case 0x1703:
		      case 0x1704:
		      case 0x1708:
		      case 0x1709:
		      case 0x1800:
		      case 0x1801:
		      case 0x1806:
		      case 0x1807:
		      case 0x1880:
		      case 0x1881:
		      case 0x1882:
				if (cmd_ptr->type & (DK_REASSIGN|DK_WRITEV)) {
				    scdisk_build_error(log_cmd_ptr,
				        DK_RECOVERED_ERR, DK_OVERLAY_ERROR,
				        SC_VALID_SENSE);
				    scdisk_log_error(log_cmd_ptr,DK_SOFT_ERROR);
				    log_cmd_ptr->error_type = 0;
				    scdisk_process_good(cmd_ptr);
				    return;
				}
				if (verify_done) {
				    scdisk_build_error(log_cmd_ptr,
				        DK_RECOVERED_ERR, DK_OVERLAY_ERROR,
				        SC_VALID_SENSE);
				    scdisk_log_error(log_cmd_ptr,DK_SOFT_ERROR);
				    log_cmd_ptr->error_type = 0;
				    bp->b_error = ESOFT;
				    cmd_ptr->retry_count = DK_MAX_RETRY;
				    break;
				}
				if ((!verify_done) &&
				    (diskinfo->dev_type == DK_RWOPTICAL)) {
					if (recovery_level > 
					    stats->recovery_limit) {

						/*
						 * Magneto-Optical devices
						 * erase first on a pass
						 * before doing the write
						 * with verify. So there is
						 * a strong potential to lose
						 * data by doing write with 
						 * verify here as we do for 
						 * for hard disks.  So instead
						 * will will mark it for 
						 * LVM to reassign.
						 */		
						if (!(sense_data->err_code 
						      & 0x80)) {
							 /*
							  * Since we did not
							  * handle this case 
							  * above for recovered 
							  * errors we must
							  * for this situation
							  */

							 bp->b_resid = 
								 bp->b_bcount;
						 }

						scdisk_build_error(log_cmd_ptr, 
							      DK_RECOVERED_ERR, 
							      DK_OVERLAY_ERROR,
							       SC_VALID_SENSE);
						scdisk_log_error(log_cmd_ptr,
								 DK_SOFT_ERROR);
						log_cmd_ptr->error_type = 0;
								
						bp->b_error = ESOFT;
						cmd_ptr->retry_count 
							= DK_MAX_RETRY;
						break;
					}
					/*
					 * If below recovery_limit then
 					 * this will drop through
					 * to the case 0x1000
					 */

				}
				else if ((!verify_done) &&
			    	(recovery_level > stats->recovery_limit)) {
					if (!(sense_data->err_code & 0x80)) {
					      /*
					       * Since we did not handle
					       * this case above for
					       * recovered errors we must
					       * for this situation
					       */
						bp->b_resid = bp->b_bcount;
					}
					scdisk_build_error(log_cmd_ptr, 
				    	    DK_RECOVERED_ERR, DK_OVERLAY_ERROR,
				    	    SC_VALID_SENSE);
					scdisk_log_error(log_cmd_ptr,
							 DK_SOFT_ERROR);
					log_cmd_ptr->error_type = 0;
			
					scdisk_write_verify(diskinfo, cmd_ptr, 
				    			    error_blkno);
					return;
				}
		      case 0x1000:
		      case 0x1103:
		      case 0x1200:
			    if (cmd_ptr->type & (DK_REASSIGN | DK_WRITEV)) {
				    scdisk_build_error(log_cmd_ptr,
						       DK_RECOVERED_ERR,
						       DK_OVERLAY_ERROR,
						       SC_VALID_SENSE);
				    scdisk_log_error(log_cmd_ptr, 
						     DK_SOFT_ERROR);
				    log_cmd_ptr->error_type = 0;
				    scdisk_process_good(cmd_ptr);
				    return;
			    }
			    scdisk_build_error(log_cmd_ptr, DK_RECOVERED_ERR,
			        DK_OVERLAY_ERROR, SC_VALID_SENSE);
			    /*
			     * This is put in so that we do not log an error
			     * every time we get a recovered, data error.
			     * Certain drives (like SCSI Kazuza) encounter this
			     * quite often and we put a threshold on the error
			     * reporting so as not to falsely panic readers of
			     * the error log.
			     *
			     * The verify_done was added because if the error
			     * occurred on a write verify, it needs to be
			     * logged every time.
			     */
			     if ((recovery_level >= scdisk_threshold) || 
				 (verify_done))
				scdisk_log_error(log_cmd_ptr, DK_SOFT_ERROR);
			    log_cmd_ptr->error_type = 0;
			    if (!(sense_data->err_code & 0x80) &&
							!(verify_done)) {
				/*
				 * Since we did not handle this case
				 * above for recovered errors we must
				 * for this situation.
				 */
				bp->b_resid = bp->b_bcount;
			     }
			    /*
                             * Check if an ESOFT should result here.  If
			     * there is a resid value already set (due to EOF),
			     * do not set ESOFT because bio and raw i/o always
			     * clear the ESOFT and set resid to 0.
                             */
			    if ((((bp->b_flags & B_READ) &&
				(recovery_level > stats->recovery_limit)) ||
				(!(bp->b_flags & B_READ) &&
				(recovery_level >= scdisk_threshold))) &&
				(cmd_ptr->bp->b_resid == 0)) {
				    bp->b_error = ESOFT;
				    
				    /*
				     *  This was a single or coalesced
				     *  operation, so simply set the retry
				     *  count to max to prevent retries and
				     *  call the error routine.
				     */
					cmd_ptr->retry_count = DK_MAX_RETRY;
				
			    } else {
			    /*
			     *  This recovered error did not result in an ESOFT,
			     *  so treat this as if no error occured.
			     */
				scdisk_process_good(cmd_ptr);
				return;
			    }
			    break;   
		case 0x3700:
			/*  
			 *  This error type should not result 
			 *  in an ESOFT
			 */
			scdisk_build_error(log_cmd_ptr, DK_RECOVERED_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			break;
		case 0x5D00:
			/*  
			 *  This error is logged as a permanent, recovered
			 *  error (equipment check) and allows the application
			 *  to continue without an error.  It is used in
			 *  predicting the drive failure.  No recovery
			 *  action is required.
			 */
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_OVERLAY_ERROR, SC_VALID_SENSE);
			scdisk_log_error(log_cmd_ptr, DK_HARD_ERROR);
			scdisk_process_good(cmd_ptr);
			return;                         
		default:
			switch (sense_code) {
			       case 0x03:
			       case 0x15:
				       scdisk_build_error(log_cmd_ptr, 
						          DK_RECOVERED_ERR,
							  DK_OVERLAY_ERROR, 
							  SC_VALID_SENSE);
				       scdisk_log_error(log_cmd_ptr, 
							DK_SOFT_ERROR);
				       log_cmd_ptr->error_type = 0;
				       scdisk_process_good(cmd_ptr);
				       return;

			       case 0x14:
				       /*
					* If this was a write 
					* verify, report ESOFT 
					* so the block will be 
					* relocated with the 
					* same data.
					*/
				       
				       if (verify_done) {
					       scdisk_build_error(log_cmd_ptr,
							    DK_RECOVERED_ERR,
							    DK_NOOVERLAY_ERROR,
							    SC_VALID_SENSE);
					       bp->b_error = ESOFT;
					       if (!(sense_data->err_code 
							& 0x80)) {
						       /*
							* Since we did not 
							* handle this case above
							* for recovered errors 
							* we must for this 
							* situation
							*/
						       bp->b_resid = 
							       bp->b_bcount;
					       }
					       cmd_ptr->retry_count = 
							 DK_MAX_RETRY;
					       scdisk_log_error(log_cmd_ptr, 
								DK_SOFT_ERROR);
				       }
				       else if (recovery_level >=
						scdisk_threshold) {
					       /*
						* Report a soft
						* media error to be
						*  relocated.
						*/
					       
						 scdisk_build_error(log_cmd_ptr,
							    DK_RECOVERED_ERR,
							    DK_NOOVERLAY_ERROR,
					                    SC_VALID_SENSE);
						 bp->b_error = ESOFT;
						 if (!(sense_data->err_code 
							& 0x80)) {
						       /*
							* Since we did not 
							* handle this case above
							* for recovered errors 
							* we must for this 
							* situation
							*/
						       bp->b_resid = 
							       bp->b_bcount;
					          }
						 cmd_ptr->retry_count 
							       = DK_MAX_RETRY;
				            }
					break;
			       default:
				       /*  
					*  This error type should not result 
					* in an ESOFT and is only logged if 
					* recovery_level >= scdisk_threshold
					*/
					scdisk_build_error(log_cmd_ptr, 
							      DK_RECOVERED_ERR,
							      DK_OVERLAY_ERROR, 
							      SC_VALID_SENSE);
					 if ((recovery_level >= 
					      scdisk_threshold) || 
                                               (verify_done))
						   scdisk_log_error(log_cmd_ptr,
								 DK_SOFT_ERROR);
					log_cmd_ptr->error_type = 0;
					scdisk_process_good(cmd_ptr);
					return;
			     }   
		 }      /* close of DK_RECOVERED switch statement */
		break;
	case DK_NOT_READY:
		/* Device was not ready for access */
		if ((cmd_ptr->subtype == DK_TUR) && 
		    (diskinfo->reset_count < DK_MAX_RESETS)) {
			/*
			 * Spin up disk.  We can't rely on sense codes
			 * here, since we must also support SCSI-1 disks.
			 */
			diskinfo->q_clr = TRUE;
			scdisk_start_unit(diskinfo,
					  (uchar) DK_START,
					  (uchar) FALSE);
			return;
		}
		switch (sense_code) {
		case 0x04:
			/* Given these special case, issue a start unit */

			if ((sense_qualifier != 0x02) && 
			    (diskinfo->dev_type != DK_CDROM)) {

				    /*
				     * An 0x0402 can happen as part of
				     * normal error recovery due to
				     * power management.  An 0x400
				     * can occur for cdrom1 when media
				     * is missing.
				     */
				    
				    scdisk_build_error(log_cmd_ptr, 
						       DK_HARDWARE_ERR,
						       DK_NOOVERLAY_ERROR, 
						       SC_VALID_SENSE);
			}
			bp->b_error = ENOTREADY;
			if (sense_qualifier == 04) {
				/* If this is an 0x0404 (format in
				 * progress). Setup for no retry 
				 */
				cmd_ptr->retry_count = DK_MAX_RETRY;
				break; /* format should never be going on */
			}
			if (diskinfo->reset_count < DK_MAX_RESETS) {
				if (!(cmd_ptr->type & DK_RESET)) {

					diskinfo->q_clr = TRUE;
					scdisk_start_unit(diskinfo,
							  (uchar) DK_START,
							  (uchar) FALSE);
					return;
				}
				else if (cmd_ptr->retry_count < DK_MAX_RETRY) {
					scdisk_start_watchdog(diskinfo,
							      (ulong)4);
	
					return;
				}
			}
			break;
		case 0x4C:
			/* Given these special case, issue a start unit */
			

			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
					   DK_NOOVERLAY_ERROR, 
					   SC_VALID_SENSE);
			bp->b_error = ENOTREADY;

			if (diskinfo->reset_count < DK_MAX_RESETS) {
				if (!(cmd_ptr->type & DK_RESET)) {

					diskinfo->q_clr = TRUE;
					scdisk_start_unit(diskinfo,
							  (uchar) DK_START,
							  (uchar) FALSE);
					return;
				}
				else if (cmd_ptr->retry_count < DK_MAX_RETRY) {
					scdisk_start_watchdog(diskinfo,
							      (ulong)4);
	
					return;
				}
			}

			break;
                case 0x27:

			/* Write Protected Media */
			if (diskinfo->dev_type == DK_DISK) {
				scdisk_build_error(log_cmd_ptr, 
						   DK_HARDWARE_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = ENOTREADY;
			}
			else {
				bp->b_error = EWRPROTECT;
				cmd_ptr->retry_count = DK_MAX_RETRY;
			}
                        break;
                case 0x3a:
			/* Media is missing */
			if (diskinfo->dev_type == DK_DISK) {
				scdisk_build_error(log_cmd_ptr, 
						   DK_HARDWARE_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = ENOTREADY;
			}
			else {
				bp->b_error = ENOTREADY;
				cmd_ptr->retry_count = DK_MAX_RETRY;
			}
                        break;
                case 0x30:
		case 0x31:
			/* Media not formatted or incompatible format */
			if (diskinfo->dev_type == DK_DISK) {
				scdisk_build_error(log_cmd_ptr, 
						   DK_HARDWARE_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = ENOTREADY;
			}
			else {
				bp->b_error = EFORMAT;
				cmd_ptr->retry_count = DK_MAX_RETRY;
			}
                        break;
		case 0x53:
			/*
			 * Typically indicates problems with
			 * removable media.
			 */
			if (diskinfo->dev_type == DK_DISK) {
				scdisk_build_error(log_cmd_ptr, 
						   DK_HARDWARE_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = ENOTREADY;
			       
			}  
			else if ( sense_qualifier == 0x00) {
			  	/*
				 * Media unload or eject failed
				 */
			  	bp->b_error = ENOTREADY;
			}
			else {
				/* 
				 * Sense qualifier of 02 is the
				 * only other possible value for
				 * non tape SCSI devices. Sense qualifier
				 * of 02 indicates Media Removal prevented 
				 * on eject command 
				 * This can only occur due to some one
				 * using the DK_EJECT ioctl command
				 * after some one has used the DKIOCMD
				 * interface to lock the drive.
				 * So issue an allow command an retry
				 * the failed command (eject).
				 */
				if (cmd_ptr->retry_count < DK_MAX_RETRY) {
					scdisk_prevent_allow(diskinfo,
							     (uchar) DK_ALLOW);
					return;
				}
				bp->b_error = ENOTREADY;
			}
			break;
		case 0x03:
		case 0x05:
		case 0x08:
		case 0x22:
		case 0x3E:
		case 0x40:
		case 0x44:
		default:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = ENOTREADY;
			break;
		}
		break;
	case DK_MEDIUM_ERROR:
		/* A fatal Media error has occurred */
		switch (sense_code) {
		case 0x1d:
			if (cmd_ptr->type & (DK_REASSIGN|DK_WRITEV)) {
				scdisk_build_error(log_cmd_ptr,
						   DK_MEDIA_ERR, 
						   DK_OVERLAY_ERROR,
						   SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr,DK_HARD_ERROR);
				log_cmd_ptr->error_type = 0;
				bp->b_error = EIO;
				break;
			}
			if (verify_done) {
				scdisk_build_error(log_cmd_ptr,
						   DK_MEDIA_ERR, 
						   DK_OVERLAY_ERROR,
						   SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr,DK_HARD_ERROR);
				log_cmd_ptr->error_type = 0;
				bp->b_error = EMEDIA;
				break;
			}
			if ((!verify_done) &&
			    (recovery_level > stats->recovery_limit)) {
				scdisk_build_error(log_cmd_ptr, 
						   DK_MEDIA_ERR, 
						   DK_OVERLAY_ERROR,
						   SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr,
						 DK_SOFT_ERROR);
				log_cmd_ptr->error_type = 0;
				
				scdisk_write_verify(diskinfo, cmd_ptr, 
						    error_blkno);
				return;
			}
			break;

		case 0x14:
			if (sense_qualifier == 0x00 )  {
				/* 
				 * Mark Error type as a media error for 
				 * processing 
				 */
				scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR,
					 DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
				bp->b_error = EIO;        /* no relocation */
				break; 
			}
			/* 
			 * else, don't break, continue for remaining
			 * sense_qualifiers.  
			 */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x16:
		case 0x1E:
			/*
                         *  If this was a write verify, report ESOFT so the
                         *  block will be relocated with the same data.
                         *  Otherwise, report a hard media error to be
                         *  relocated.
                         */
			if (verify_done) {
				scdisk_build_error(log_cmd_ptr, 
				    DK_RECOVERED_ERR, DK_NOOVERLAY_ERROR, 
				    SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr,DK_HARD_ERROR);
				log_cmd_ptr->error_type = 0;
				bp->b_error = ESOFT;
				cmd_ptr->retry_count = DK_MAX_RETRY;

			} else {
				scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR,
				    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
				bp->b_error = EMEDIA;
				/*
                                 *  Set up the soft_resid variable in case a
                                 *  retry of this cmd is successful.  In that
                                 *  case, scdisk_process_buf will detect the
                                 *  soft resid and report an ESOFT.
                                 */
				if (cmd_ptr->soft_resid == 0) {
					cmd_ptr->soft_resid = 
					    cmd_ptr->bp->b_bcount -
					    cmd_ptr->segment_count -
					    bp->b_bcount +
					    bp->b_resid;
				}
				/*
				 *  If this was a write command, do not retry
				 *  the operation, just relocate the block.
				 */
				if (!(bp->b_flags & B_READ)) {
					cmd_ptr->retry_count = DK_MAX_RETRY;
				}
			}
			break;
		case 0x0C:
			if ( (sense_qualifier == 03 ) ||
			    (sense_qualifier == 02)){
			/* 
			 * If this was a write verify, report ESOFT so the
			 * block will be relocated with the same data.
			 * Otherwise, report a hard media error to be
			 * relocated.
                         */
				if (verify_done) {
					scdisk_build_error(log_cmd_ptr, 
				    	DK_RECOVERED_ERR, DK_NOOVERLAY_ERROR, 
				    	SC_VALID_SENSE);
					bp->b_error = ESOFT;
					cmd_ptr->retry_count = DK_MAX_RETRY;
					scdisk_log_error(log_cmd_ptr,
							 DK_HARD_ERROR);
					log_cmd_ptr->error_type = 0;

				} else {
					scdisk_build_error(log_cmd_ptr,
							 DK_MEDIA_ERR,
				    			 DK_NOOVERLAY_ERROR,
							 SC_VALID_SENSE);
					bp->b_error = EMEDIA;
					/* 
					 * Set up the soft_resid variable
					 * in case a retry of this cmd is
					 * successful.  In that case,
					 * scdisk_process_buf will detect
					 * the soft resid and report an ESOFT.
                                 	 */
					if (cmd_ptr->soft_resid == 0) {
						cmd_ptr->soft_resid = 
					    	    cmd_ptr->bp->b_bcount -
					    	    cmd_ptr->segment_count -
					    	    bp->b_bcount +
					    	    bp->b_resid;
					}
					/* If this was a write command, do
					 * not retry the operation, just
					 * relocate the block.
					 */
					if (!(bp->b_flags & B_READ)) {
					   cmd_ptr->retry_count = DK_MAX_RETRY;
					}
				}
				break;
			}
			/*
			 * Let remaining sense qualifiers for Sense Code 0C
			 * drop through only to the next case,
			 * (the default case).
			 */

		case 0x31:
			if (diskinfo->dev_type != DK_DISK) {
				/* 
				 * if not SCSI disk then mark this 
				 * a incorrect format.
				 */
				/* 
				 * Mark Error type as a media error for 
				 * processing 
				 */
				scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = EFORMAT;
				break;
			}
			/*
			 * SCSI disks drop through to default
			 */
		case 0x01:
		case 0x02:
		case 0x09:
		case 0x15:
		case 0x19:
		case 0x1C:
		case 0x32:
		case 0x44:
		default:
			/* Mark Error type as a media error for processing */
			scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = EIO;        /* no relocation */
			break;
		}
		break;
	case DK_HARDWARE_ERROR:
		/* A fatal hardware error occurred at the device */
		switch (sense_code) {
		case 0x10:
		case 0x12:
		case 0x14:
			if ((sense_qualifier == 0x01) && (sense_code == 0x14)) {
				scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
						   DK_NOOVERLAY_ERROR, 
						   SC_VALID_SENSE);
				bp->b_error = EIO;
				break;
			}
				
			/*
                         *  If this was a write verify, report ESOFT so the
                         *  block will be relocated with the same data.
                         *  Otherwise, report a hard media error to be
                         *  relocated.
                         */
			if (verify_done) {
				scdisk_build_error(log_cmd_ptr, 
				    DK_RECOVERED_ERR, DK_NOOVERLAY_ERROR, 
				    SC_VALID_SENSE);
				scdisk_log_error(log_cmd_ptr,DK_HARD_ERROR);
				log_cmd_ptr->error_type = 0;
				bp->b_error = ESOFT;
				cmd_ptr->retry_count = DK_MAX_RETRY;

			} else {
				scdisk_build_error(log_cmd_ptr,DK_HARDWARE_ERR,
				    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
				bp->b_error = EMEDIA;
				/*
                                 *  Set up the soft_resid variable in case a
                                 *  retry of this cmd is successful.  In that
                                 *  case, scdisk_process_buf will detect the
                                 *  soft resid and report an ESOFT.
                                 */
				if (cmd_ptr->soft_resid == 0) {
					cmd_ptr->soft_resid = 
					    cmd_ptr->bp->b_bcount -
					    cmd_ptr->segment_count -
					    bp->b_bcount + bp->b_resid;
				}
				/*
				 *  If this was a write command, do not retry
				 *  the operation, just relocate the block.
				 */
				if (!(bp->b_flags & B_READ)) {
					cmd_ptr->retry_count = DK_MAX_RETRY;
				}
			}
			break;
		case 0x47:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			/*
			 * For parity errors we need to restart the whole 
			 * command.
			 */
			if (cmd_ptr->group_type == DK_COALESCED) {
				bp->b_resid = bp->b_bcount;
			}
			bp->b_error = EIO;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x06:
		case 0x08:
		case 0x09:
		case 0x15:
		case 0x1B:
		case 0x21:
		case 0x22:
		case 0x32:
		case 0x3E:
		case 0x40:
		case 0x41:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x51:
		case 0x53:
		case 0x5D:
		default:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = EIO;
			break;
		}
		break;
	case DK_ILLEGAL_REQUEST:
		/*
		 * Kludge for Maxtor 8000 microcode problem
                 * This should be pulled out when the drive no longer returns
                 * an error when already spun up!!!
		 */
		if ((cmd_ptr->subtype == DK_START) && (sense_code == 0x22)) {
			scdisk_process_good(cmd_ptr);
			return;
		}
		if (cmd_ptr->subtype == DK_START) {
			/*
			 * if this was a start unit command that received 
			 * an illegal request, assume it is some  
			 * OEM SCSI device that doesn't support start units.
			 * Complete the start unit as good and bypass to 
			 * the next command in the reset sequence.
			 */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_free_cmd(cmd_ptr);
			scdisk_reserve(diskinfo);
			return;
		}
                if (cmd_ptr->subtype == DK_READ_INFO) {
                        /*
                         * if this was a read disk info command that received 
                         * an illegal request, assume it is a  
                         * SCSI device that doesn't support this command.
                         * Complete the start unit as good and bypass to 
                         * the next command in the reset sequence.
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                        scdisk_free_cmd(cmd_ptr);
			
			/*
			 * Since this command is invalid for this
			 * device, let's prevent the driver from
			 * issuing it again until it is unconfigured
			 * and reconfigured.
			 */
			diskinfo->multi_session = FALSE;

			/*
			 * Re-issue mode sense's since the mode sense
			 * data in the sense buffer will have been 
			 * overwritten by the request sense data.
			 */
			diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
			scdisk_mode_sense(diskinfo);

                        
                        
                        return;
                }

		if ((cmd_ptr->subtype == DK_MSENSE) || 
		    (cmd_ptr->subtype == DK_SELECT)) {
			/*
			 * if this was a mode sense or select that received 
			 * an illegal request, assume it is some strange 
			 * OEM SCSI Disk that doesn't support page code 0x3f, 
			 * or some setting that we tried to apply. Complete the
			 * mode sense/or select as good and bypass to the next
			 * command in the reset sequence.
			 */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_free_cmd(cmd_ptr);
			scdisk_read_cap(diskinfo);
			return;
		}

		/* Illegal parameter in a cmd */
		switch (sense_code) {
		case 0x19:
			scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR, 
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			bp->b_error = EINVAL;
			break;
		case 0x1A:
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x39:
		case 0x3D:
		default:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
					   DK_NOOVERLAY_ERROR, 
					   SC_VALID_SENSE);

			if (!(diskinfo->wprotected)) {
				bp->b_error = EINVAL;
			}
			else {
				/* 
				 * If this is write protected media then
				 * this is most likely due to the media
				 * containing audio tracks i.e. 
				 * audio CDROM 
				 */
				bp->b_error = EIO;
			}
			break;
		}
		break;
	case DK_UNIT_ATTENTION:
		/*
                 * The device has been reset either by a SCSI Bus Reset
                 * or by a power cycle.
                 * We need to reinitialize the device through a RESET
                 * cmd cycle and then retry the operation.
                 * The watchdog timer is set for 2 secs. to allow the
                 * device to recover from the reset if this is a retry.
                 */
		if ((diskinfo->wprotected) && (sense_code == 0x28)) {

			/*
			 * Media may have changed.
			 */
			bp->b_error = ESTALE;
			break;
		}
                else if ((sense_code == 0x5a) && (sense_qualifier == 01)) {
                        /*
                         * User has tried to eject media when media
                         * removal is prevented.  Let commmand be retried.
                         * If command tag queuing then adapter's queue must
                         * also be cleared.
                         */
                        diskinfo->q_clr = TRUE;
			bp->b_error = EIO;
                        break;
                }
		else if (( sense_code == 0x42) || 
			 (sense_code == 0x26)) {
			/* log Permanent Error/Equipment Check */
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_OVERLAY_ERROR, SC_VALID_SENSE);
			scdisk_log_error(log_cmd_ptr, DK_HARD_ERROR);
		}
		if (diskinfo->reset_count < DK_MAX_RESETS) {
			if (!(cmd_ptr->type & DK_RESET) ||
			    (cmd_ptr->subtype == DK_TUR)) {

				diskinfo->q_clr = TRUE;
				scdisk_start_unit(diskinfo, (uchar) DK_START,
						  (uchar) FALSE);
				return;
			}
			else if (cmd_ptr->retry_count < DK_MAX_RETRY) {

				scdisk_start_watchdog(diskinfo,(ulong)2);
				return;
			}
		}
		break;
	case DK_DATA_PROTECT:
		/* Device is read-only */
		scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EWRPROTECT;
		break;
	case DK_BLANK_CHECK:
		/* Blank Media or Format-defined end of data indication */
                if (diskinfo->dev_type == DK_CDROM) {
                        /*
                         * This error is valid for CD-ROM so don't log it
                         * for CD-ROM. 
                         */

                        if (bp->b_resid == bp->b_bcount) {
                                /*
                                 * If the entire transfer failed
                                 * with blank check then the 1st
                                 * requested sector must have received
                                 * the blank check. Therefore
                                 * fail the entire request with an EIO.
                                 */
                                bp->b_error = EIO;
                        } 
			else {
				/*
				 * NOTE: if the entire transfer did not
				 * fail then return good completion, but
				 * with resid set (above) indicating how much
				 * succeeded/failed.
				 */

				scdisk_process_good(cmd_ptr);
				return;
			}
                        
                } else {
                        /*
                         * Blank check should never be seen for
                         * SCSI disks or read/write optical devices.
                         */

                        scdisk_build_error(log_cmd_ptr, DK_MEDIA_ERR, 
                                           DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

                        /* Mark Error in buf for processing */
                        bp->b_error = EIO;
                }

		break;
	case DK_VENDOR_UNIQUE:
		/* A Vendor unique condition has occurred */
		scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
		break;
	case DK_COPY_ABORTED:
		/* A copy or compare cmd was aborted due to an error at */
		/* either the source or destination device */
		scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
		break;
	case DK_ABORTED_COMMAND:
		/* The device aborted the cmd. May be recovered via a */
		/* for parity errors we need to restart the whole command */
		switch (sense_code) {
		case 0x00:
			/* Mark Error in buf for processing */
			bp->b_error = EIO;
			/* Setup for no retry */
			cmd_ptr->retry_count = DK_MAX_RETRY;
			break;
		case 0x47:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			if (cmd_ptr->group_type == DK_COALESCED) {
				bp->b_resid = bp->b_bcount;
			}
			bp->b_error = EIO;
			break;
		default:
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);
			/* Mark Error in buf for processing */
			bp->b_error = EIO;
		}
		break;
	case DK_MISCOMPARE:
		/*
                 *  If this was a write verify, report ESOFT so the
                 *  block will be relocated with the same data.
                 *  Otherwise, report a hard error.
                 */
		if (verify_done) {
			scdisk_build_error(log_cmd_ptr, 
			    DK_RECOVERED_ERR, DK_NOOVERLAY_ERROR, 
			    SC_VALID_SENSE);
			scdisk_log_error(log_cmd_ptr,DK_HARD_ERROR);
			log_cmd_ptr->error_type = 0;
			bp->b_error = ESOFT;
			cmd_ptr->retry_count = DK_MAX_RETRY;
		} else {
			scdisk_build_error(log_cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

			/* Mark Error in buf for processing */
			bp->b_error = EIO;
			break;
		}
	default:
		/* Process unknown sense key */
		scdisk_build_error(log_cmd_ptr, DK_UNKNOWN_ERR, 
		    DK_NOOVERLAY_ERROR, SC_VALID_SENSE);

		/* Mark Error in buf for processing */
		bp->b_error = EIO;
	}

	/*
         *  If this was a write verify operation that resulted in some error
         *  that does not indicate the block should be relocated, do not
         *  report an error since the data is good.
         */
	if (verify_done && (bp->b_error != ESOFT)) {
		bp->b_error = 0x00;
		bp->b_resid = 0x00;
		scdisk_process_good(cmd_ptr);
		return;
	}
	switch (cmd_ptr->type) {
	case DK_BUF:
		scdisk_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		scdisk_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		scdisk_process_reset_error(cmd_ptr);
		break;
	case DK_REQSNS:
		scdisk_process_reqsns_error(cmd_ptr);
		break;
	case DK_WRITEV:
        case DK_REASSIGN:
		scdisk_process_special_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}


}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_recover_adap_q					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine examines the current state of the SCSI     */
/*              adapter device driver's queue and then decides on how   */
/* 		to proceed.  In order for this routine to be called     */
/*		the SCSI adapter driver's queue for this disk must be   */
/*		halted, but notcleared.  We must decide if we want the  */
/*		adapter driver to clear this queue (flush the commands  */
/*		back to us with ENXIO) or resume this queue (let        */
/*		the SCSI adapter driver continue with it).		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine is only for the case where the 		*/
/*		SCSI adapter driver's queue for this disk is not	*/
/*		cleared following a check condition			*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	CALLED BY:							*/
/*		scdisk_process_good					*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/*		scdisk_cmd_alloc		scdisk_q_cmd		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		none							*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void scdisk_recover_adap_q(struct scdisk_diskinfo *diskinfo)
{
	struct dk_cmd *cmd_ptr;			/* command block pointer */
	


	if ((diskinfo->clr_q_on_error) || (diskinfo->q_clr)) {
		/*
		 * Adapter driver and/or adapter has not cleared its 
		 * queue for this device and we must flush the adapter
		 * driver's queue for this device.  This current situation
		 * will result from the two following conditions: 
		 *
		 *	1. The device has cleared its queue, but the adapter 
		 *	    driver has not (thus we have an inconsistency 
		 *	    between the adapter driver and the device).
		 *
		 *
		 *	2. Scdisk_process_sense has indicated that the 
		 *	   queue at the adapter driver must be cleared (i.e.
		 *	   a unit attention, etc).
		 */


		/*
		 * Issue q clear command to clear all
		 * commands on the device and adapter.
		 */
		
		cmd_ptr = scdisk_cmd_alloc(diskinfo,
					   (uchar) DK_Q_RECOV);
		ASSERT(cmd_ptr != NULL);
		cmd_ptr->scbuf.flags = SC_Q_CLR;

		
		scdisk_q_cmd(cmd_ptr,(char) DK_STACK,
			     (uchar) DK_CMD_Q);
	}
	else if (!(diskinfo->clr_q_on_error)) {
		/*
		 * Adapter has not cleared its Queue for this
		 * device and this device has not cleared its
		 * queue on this error.  Here we want to resume
		 * the adapter driver's queue.
		 */

		/*
		 * Issue Q resume command to resume all
		 * commands queued at the device and adapter
		 */
		if (diskinfo->dk_cmd_q_head == NULL) {
			/*
			 * Use reserved Q_RECOV command block
			 */
			cmd_ptr = 
				scdisk_cmd_alloc(diskinfo,
						 (uchar) DK_Q_RECOV);
			ASSERT(cmd_ptr != NULL);
			cmd_ptr->scbuf.flags = 
				SC_Q_RESUME| SC_RESUME;

			
			scdisk_q_cmd(cmd_ptr,(char) DK_STACK,
				     (uchar) DK_CMD_Q);
		}
		else {
			/*
			 * Have next command resume the
			 * the queues
			 */
			cmd_ptr = diskinfo->dk_cmd_q_head;
			cmd_ptr->scbuf.flags |= (SC_Q_RESUME|SC_RESUME);
		}
	}
	
	/*
	 * Clear any adapter queue status, since we took care of this
	 * above.
	 */
	diskinfo->q_status &= ~SC_DID_NOT_CLEAR_Q;

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine examines the return status of operations	*/
/*		that have been processed by the adapter driver and	*/
/*		have been returned in an error state.			*/
/*		This routine must examine the adapter status, the	*/
/*		SCSI status, if available, and do any logging		*/
/*		that may be required. Also must determine what error	*/
/*		recovery procedure should be followed.			*/
/*			Retry operation					*/
/*			Abort operation					*/
/*			start diagnostic operation			*/
/*									*/
/*		When this routine exits, the device's cmd stack		*/
/*		should either be empty, or contain an operation		*/
/*		to be started.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_error(
struct dk_cmd	*cmd_ptr)
{
	struct sc_buf	*scbuf;
	struct scdisk_diskinfo *diskinfo;


	/* Set up misc pointers for structures */
	scbuf = &(cmd_ptr->scbuf);
	diskinfo = cmd_ptr->diskinfo;
	/*
	 * Clear any queue recovery flag that may be
	 * set.  NOTE SC_Q_CLR can't be set with an
	 * actual command.
	 */
	scbuf->flags &= ~SC_Q_RESUME;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,error, entry,(char)0, 
	      (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr ,(uint)cmd_ptr->type, (uint)cmd_ptr->subtype,
	      (uint)scbuf->status_validity);
#endif	
#endif
	cmd_ptr->diskinfo->restart_unit = TRUE;
	cmd_ptr->status |= DK_RETRY;
	/*
	 * Since this command hand an error lets put it on the head of the cmd_Q
	 * for possible retries.
	 */

	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
	if ((cmd_ptr->type == DK_IOCTL) && (cmd_ptr->subtype == DK_IOCMD)) {
		/* No error recovery in DIAGNOSTIC mode (pass-thru command) */
		scdisk_process_diagnostic_error(cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,error, exit,(char)0, (uint)0, (uint)0 ,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif

	} else if (scbuf->status_validity & SC_ADAPTER_ERROR) {
		/* Process adapter error condition */
		scdisk_process_adapter_error(cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,error, exit,(char)1, (uint)0, (uint)0 ,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif

	} else if (scbuf->status_validity & SC_SCSI_ERROR) {
		/* Process SCSI error condition */
		scdisk_process_scsi_error(cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,error, exit,(char)2, (uint)0, (uint)0 ,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif

	}
	else {
		/*  The adapter driver returned a resid with no error. */
		scdisk_process_adapter_error(cmd_ptr);
	}

	/*
         * Please note that when this routine exits, whatever
         * is on the top of the device's cmd queue will be started.
         * Falling through this routine will cause a retry of the
         * operation that failed.
         */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,error, exit,(char)3, (uint)0, (uint)0 ,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_diagnostic_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of a SCSI request	*/
/*		during Diagnostic Mode of operation. The only requests	*/
/*		permitted during Diagnostic Mode are IOCTLs at this	*/
/*		time. The processing reflects such.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_diagnostic_error(
struct dk_cmd	*cmd_ptr)
{
	struct buf	*bp;
	struct scdisk_diskinfo *diskinfo;

	diskinfo = cmd_ptr->diskinfo;


#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,diagerr, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr ,(uint)cmd_ptr->type, (uint)0,(uint)0);
#endif	
#endif
	/*
         * The only valid operations in DIAGNOSTIC mode are ioctls
         * so we will only process completion of the ioctl operation
         * and leave error handling up to the calling process
         */

	bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_ptr->flags &= ~DK_READY;
	scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	cmd_ptr->intrpt = 0;
	e_wakeup(&(bp->b_event));
	/*
	 * If we are sending iodones back then clear 
	 * the reset_failure flag, even though this 
	 * is an error
	 */
	cmd_ptr->diskinfo->reset_failures = 0;
	cmd_ptr->diskinfo->ioctl_pending = FALSE;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,diagerr, exit,(char)0, (uint)0, (uint)0,(uint)0, (uint)0,
	      (uint)0);
#endif	
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_adapter_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes SCSI requests that have failed	*/
/*		due to an error at the adapter. This routine will log	*/
/*		the specific error in the system error log, and attempt	*/
/*		to reschedule the operation for a retry.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_adapter_error(
struct dk_cmd	*cmd_ptr)
{
	struct sc_buf	*scbuf;
	struct buf	*bp;
	int		max_retries;
	uchar		stack = FALSE;
	struct scdisk_diskinfo *diskinfo;


	/* Set up misc pointers for structures */
	scbuf = &(cmd_ptr->scbuf);
	bp = &(scbuf->bufstruct);
	diskinfo = cmd_ptr->diskinfo;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,adaperror, entry,(char)0, (uint)cmd_ptr->diskinfo->state,
	      (uint)cmd_ptr ,(uint)cmd_ptr->type, 
	      (uint)scbuf->general_card_status,(uint)scbuf->status_validity);
#endif	
#endif
	if (scbuf->status_validity & SC_ADAPTER_ERROR) {
		switch(scbuf->general_card_status) {
		case SC_HOST_IO_BUS_ERR:
		case SC_SCSI_BUS_FAULT:
		case SC_NO_DEVICE_RESPONSE:
			scdisk_build_error(cmd_ptr, DK_ADAPTER_ERR,
			    DK_NOOVERLAY_ERROR, 0);
			bp->b_error = EIO;
			break;
		case SC_CMD_TIMEOUT:
			scdisk_build_error(cmd_ptr, DK_ADAPTER_ERR,
			    DK_NOOVERLAY_ERROR, 0);
			bp->b_error = ETIMEDOUT;
			break;
		case SC_ADAPTER_HDW_FAILURE:
		case SC_ADAPTER_SFW_FAILURE:
		case SC_FUSE_OR_TERMINAL_PWR:
		case SC_SCSI_BUS_RESET:
			bp->b_error = EIO;
			break;
		default:
			/* Unknown Adapter Error condition */
			scdisk_build_error(cmd_ptr, DK_UNKNOWN_ERR,
			    DK_NOOVERLAY_ERROR, 0);
			bp->b_error = EIO;
			break;
		}
	}
	else if (bp->b_error == ENXIO) {

			
		if (cmd_ptr->type & (DK_REQSNS|DK_RESET|DK_WRITEV
					     |DK_REASSIGN)) {
			/*
			 * If this is a request sense, reset, 
			 * write verify, or reassign  
			 * operation then clear the state flag of this 
			 * disk accordingly.  NOTE: The cmd_ptr->type
			 * defines match the corresponding 
			 * diskinfo->state defines.
			 */
			stack = TRUE;
			cmd_ptr->diskinfo->state &= ~cmd_ptr->type;
		}


		/*
		 * Maximum number of retries for ENXIO is 2 times
		 * the queue_depth
		 */
		max_retries = cmd_ptr->diskinfo->queue_depth << 1; 

		if (cmd_ptr->retry_count >= max_retries) {
			scdisk_build_error(cmd_ptr, 0,
					   DK_NOOVERLAY_ERROR, 0);
			bp->b_error = EBUSY;
		}
		else {

			if (!stack) {

				/*
				 * This command needs to be place at the
				 * tail of the command q since it is not
				 * a high priority command
				 */
				scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
				scdisk_q_cmd(cmd_ptr,(char) DK_QUEUE,								      (uchar) DK_CMD_Q);
				
			}
			
			/*
			 * Retry Command.  Since it is on the 
			 * command q there is nothing  new to do here.
			 */

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,adaperror, trc,(char)0, (uint)bp->b_error, 
	      (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif		
			return;
		}
	}
	else {
		/* 
		 * No error was reported, but the b_resid value was set. 
		 */
		scdisk_build_error(cmd_ptr, DK_UNKNOWN_ERR,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EIO;
	}

	/*
	 * No matter which error case we hit, the b_resid field should
	 * be set to the full count.  If not, a coalesced command will not
	 * be handled properly (we will assume that some of the data
	 * transfered successfully, when in fact there is no way to tell).
	 */

	bp->b_resid = bp->b_bcount;

	/*
         * Now we handoff to the cmd type specific error processor
         * for preparing the cmd for a retry attempt or aborting
         * the cmd back to the requestor.
         */

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,adaperror, trc,(char)1, (uint)bp, (uint)bp->b_error ,
	      (uint)bp->b_resid, (uint)cmd_ptr->type,(uint)0);
#endif	
#endif
	switch (cmd_ptr->type) {
	case DK_BUF:
		scdisk_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		scdisk_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		scdisk_process_reset_error(cmd_ptr);
		break;
	case DK_REQSNS:
		scdisk_process_reqsns_error(cmd_ptr);
		break;
	case DK_WRITEV:
        case DK_REASSIGN:
		scdisk_process_special_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,adaperror, exit,(char)0, (uint)0, (uint)0 ,
	      (uint)0, (uint)0,
	      (uint)0);
#endif	
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_scsi_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes SCSI requests that have failed	*/
/*		due to an error on the SCSI Bus. This routine will log	*/
/*		the specific error in the system error log, and attempt	*/
/*		to reschedule the operation for a retry.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_scsi_error(
struct dk_cmd	*cmd_ptr)
{
	struct sc_buf		*scbuf;
	struct buf		*bp;
	struct scdisk_diskinfo	*diskinfo;
	uchar			scsi_status;
	int			max_retries;
	uchar			stack = FALSE;

	/* Set up misc pointers for structures */
	scbuf = &(cmd_ptr->scbuf);
	bp = &(scbuf->bufstruct);
	diskinfo = cmd_ptr->diskinfo;
	scsi_status = scbuf->scsi_status;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsierror, entry,(char)0, (uint)cmd_ptr->type, 
	      (uint)scsi_status,(uint)diskinfo->checked_cmd, (uint)0,(uint)0);
#endif	
#endif	
	switch(scsi_status) {
	case SC_GOOD_STATUS:
		bp->b_error = EIO;
		break;
	case SC_CHECK_CONDITION:
	case SC_COMMAND_TERMINATED:

		/* 
		 * The Devices is in the contingent alligiance conditions, which
		 * we must clear.  Prior to clear this condition we must 
		 * determine the the type of error and the necessary error 
		 * recovery procedure
		 */
		if (cmd_ptr->type == DK_REQSNS) {
			/* Check condition on Reqest Sns is a Fatal error */
			/* Log it as such then retry original operation */

			scdisk_build_error(cmd_ptr, DK_HARDWARE_ERR,
			    DK_NOOVERLAY_ERROR, 0);
		} else if ((cmd_ptr->type == DK_RESET) && 
			   (cmd_ptr->subtype == DK_STOP) &&
			   (!diskinfo->opened)) {
			/*
			 * If we are closed and this is a stop
			 * unit that has received the check condition,
			 * then assume it's the stop unit associated
			 * with unconfiguring.  As a result do not
			 * attempt recovery, because this may cause
			 * us to go through our reset cycle. So
			 * just assume good completion here.
			 */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_process_reset(cmd_ptr);
			return;

		} else {
			/* Do a Request Sense operation */

			
			if (cmd_ptr->type & (DK_RESET|DK_REASSIGN)) {
				/*
				 * If this is a reset, or reassign 
				 * operation then clear the state flag of this 
				 * disk accordingly.  NOTE: The cmd_ptr->type
				 * defines match the corresponding 
				 * diskinfo->state defines.
				 */
				diskinfo->state &= ~cmd_ptr->type;
			}

			scdisk_request_sense(cmd_ptr->diskinfo,cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsierror, exit,(char)0, (uint)diskinfo->state, (uint)0,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif
			return;
		}
		break;
	case SC_BUSY_STATUS:
		scdisk_build_error(cmd_ptr, DK_HARDWARE_ERR,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EBUSY;
		break;
	case SC_INTMD_GOOD:
		bp->b_error = EIO;
		break;
	case SC_RESERVATION_CONFLICT:
		scdisk_build_error(cmd_ptr, 0,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EBUSY;
		break;
	case SC_QUEUE_FULL:
		/*
		 * Device's command queue is full so lets retry this command
		 * Since it is on the head of the command 
		 * q, there is nothing  new to do here, except clear pending
		 * flags.
		 */

			
		if (cmd_ptr->type & (DK_REQSNS|DK_RESET|DK_WRITEV
					     |DK_REASSIGN)) {
			/*
			 * If this is a request sense, reset, 
			 * write verify, or reassign  
			 * operation then clear the state flag of this 
			 * disk accordingly.  NOTE: The cmd_ptr->type
			 * defines match the corresponding 
			 * diskinfo->state defines.
			 */
			stack = TRUE;
			cmd_ptr->diskinfo->state &= ~cmd_ptr->type;
		}

		/*
		 * Maximum number of retries for Queue Full is 2 times
		 * the queue_depth
		 */
		max_retries = cmd_ptr->diskinfo->queue_depth << 1; 

		if (cmd_ptr->retry_count >= max_retries) {
			scdisk_build_error(cmd_ptr, 0,
					   DK_NOOVERLAY_ERROR, 0);
			bp->b_error = EBUSY;
			break;
		}
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsierror, trc,(char)0, (uint)bp, (uint)bp->b_error ,
	      (uint)bp->b_resid, (uint)cmd_ptr->type,(uint)diskinfo->state);
#endif	
#endif	
		
		if (!stack) {
			
			/*
			 * This command needs to be place at the
			 * tail of the command q since it is not
			 * a high priority command
			 */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_q_cmd(cmd_ptr,(char) DK_QUEUE,			
					      (uchar) DK_CMD_Q);
			
		}
		return;
	default:
		/* Unknown SCSI Error condition */
		scdisk_build_error(cmd_ptr, DK_UNKNOWN_ERR,
		    DK_NOOVERLAY_ERROR, 0);
		bp->b_error = EIO;
		break;
	}

	/*
	 * No matter which error case we hit, the b_resid field should
	 * be set to the full count.  If not, a coalesced command will not
	 * be handled properly (we will assume that some of the data
	 * transfered successfully, when in fact there is no way to tell).
	 */

	bp->b_resid = bp->b_bcount;

	/*
         * Now we handoff to the cmd type specific error processor
         * for preparing the cmd for a retry attempt or aborting
         * the cmd back to the requestor.
         */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsierror, trc,(char)1, (uint)bp, (uint)bp->b_error ,
	      (uint)bp->b_resid, (uint)cmd_ptr->type,(uint)diskinfo->state);
#endif	
#endif
	switch (cmd_ptr->type) {
	case DK_BUF:
		scdisk_process_buf_error(cmd_ptr);
		break;
	case DK_IOCTL:
		scdisk_process_ioctl_error(cmd_ptr);
		break;
	case DK_RESET:
		scdisk_process_reset_error(cmd_ptr);
		/*
                 *  If the device was busy we want to timeout for 2 secs before
                 *  we retry the command (for a SCSI bus reset).  This is the 
                 *  case when the cmd_ptr is still on top of the stack, and 
		 *  the scsi status is busy.
                 */
		if ((diskinfo->retry_flag)&&(scsi_status == SC_BUSY_STATUS)) {

			scdisk_start_watchdog(diskinfo,(ulong)2);
			diskinfo->timer_status |= DK_TIMER_BUSY;
		}
		break;
	case DK_REQSNS:
		scdisk_process_reqsns_error(cmd_ptr);
		break;
	case DK_WRITEV:
        case DK_REASSIGN:
		scdisk_process_special_error(cmd_ptr);
		break;
	default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,scsierror, exit,(char)1, (uint)diskinfo->state, (uint)0,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif	

}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_buf_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine does error processing specifically for	*/
/*		requests that are related to a Buf structure. The	*/
/*		failure is evaluated, and if a retry is feasible	*/
/*		the cmd is updated to reflect the current state of	*/
/*		the request, ie. the residual, and reissued to the	*/
/*		controller.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*                                                                      */
/*	NOTES:								*/
/*i									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		iodone							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_buf_error(
struct dk_cmd	*cmd_ptr)
{
	struct sc_buf		*scbuf;
	struct scdisk_diskinfo	*diskinfo;
	struct buf		*bp;
	uint			remaining_count;



	/* Set up misc pointers for structures */
	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

	scbuf = &(cmd_ptr->scbuf);
	bp = &(scbuf->bufstruct);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,failbuf, entry,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
	/* Check if cmd is single operation */
	if (cmd_ptr->group_type == DK_SINGLE) {
		/* Operation is a Single operation */
		if (cmd_ptr->retry_count >= DK_MAX_RETRY) {
			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}

			/* Remove op from device's cmd stack */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);


			/* flush request from IN_PROGRESS queue */

			if ((cmd_ptr->error_type == 0) && 
			    (bp->b_resid == 0)) {
				/* 
				 * If this was something that we did not log 
				 * and which completely transferred then
				 * make sure that b_error = 0
				 */
				cmd_ptr->bp->b_error = 0;
				cmd_ptr->bp->b_flags &= ~B_ERROR;
			}
			else {
				cmd_ptr->bp->b_flags |= B_ERROR;
				cmd_ptr->bp->b_error = bp->b_error;
				cmd_ptr->bp->b_resid += bp->b_resid;
			}
			DDHKWD2(HKWD_DD_SCDISKDD, DD_SC_IODONE,
			    cmd_ptr->bp->b_error, cmd_ptr->bp->b_dev,
			    cmd_ptr->bp);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,sdiodone, trc,(char)2, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
			iodone(cmd_ptr->bp);
			
			/*
			 * If wa are sending iodones back then clear 
			 * the reset_failure flag, even though this 
			 * is an potential error
			 */
			diskinfo->reset_failures = 0;
			scdisk_free_cmd(cmd_ptr);
		}
	} else if (cmd_ptr->group_type == DK_COALESCED) {
		/* Operation is a Coalesced operation */
		struct buf      *bp_list;
		struct buf      *first_bad_bp;
		struct buf      *last_good_bp;
		struct buf      *current_bp;
		struct buf      *prev_bp;
		int             total_count;
		int             good_count;
		int             residual;
		int             current_retry_count;
		uchar		current_error_type;


		/*
		 * we need to try to determine the extent
		 * of the failure, indicating which parts
		 * of the coalesced operation were successful
		 * and which parts need to be retried
		 */

		/* Set up a pointer to the list of requests */
		bp_list = scbuf->bp;

		/* Determine how much of operation failed */
		residual = bp->b_resid;

		/*
		 * Determine if any part of the operation
		 * was successful, and process completion
		 * of any that were.
		 */
		first_bad_bp = NULL;
		last_good_bp = NULL;
		current_bp = bp_list;
		total_count = bp->b_bcount;
		good_count = 0;


		/*
		 *
		 *
		 *  Example of a failed transfer with 3 bufs.
		 *
		 *  B              B              
		 *  |-------|--------|--------|  = Buf addr boundaries
		 *  ^-------------------------^  = total count
		 *  ^-------^                    = good count
		 *              ^-------------^  = residual
		 *  |XXXXXXX|                    = Blocks returned as good
		 *          ^                    = first_bad_bp
		 *  ^                            = last_good_bp
		 *
		 */

		while ((total_count - (good_count + current_bp->b_bcount)) >
		    residual) {
			good_count += current_bp->b_bcount;
			last_good_bp = current_bp;
			current_bp = current_bp->av_forw;
		}
		first_bad_bp = current_bp;

		/*
		 * At this point, first_bad_bp points at the
		 * list of bp's that need to be retried, and
		 * any successful bp's are in a NULL terminated
		 * list pointed to by scbuf->bp.
		 */

		/* Check for any completed requests and process */
		if (last_good_bp != NULL) {
			current_bp = bp_list;
			do {


				/* Return to system as complete */
				current_bp->b_resid = 0;
				current_bp->b_flags &= ~B_ERROR;
				current_bp->b_error = 0;
				DDHKWD2(HKWD_DD_SCDISKDD, DD_SC_IODONE,
				    current_bp->b_error, current_bp->b_dev,
				    current_bp);
				prev_bp = current_bp;
				current_bp = current_bp->av_forw;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,sdiodone, trc,(char)3, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)bp->b_error, 
	      (uint)bp->b_resid,(uint)bp->b_bcount);
#endif	
#endif	
				iodone(prev_bp);
			
				/*
				 * If wa are sending iodones back then clear 
				 * the reset_failure flag, even though this 
				 * is an potential error
				 */
				diskinfo->reset_failures = 0;
			} while (prev_bp != last_good_bp);
		}





		/*
		 * At this point the bp list has been
		 * split into two lists.
		 * we must set scbuf.bp to point at the start of the
		 * list to be retried.
		 */
		scbuf->bp = first_bad_bp;
		/*
		 * Save off some current values for the
		 * new cmd_ptr that we will create.
		 */
		current_error_type = cmd_ptr->error_type;
		current_retry_count = cmd_ptr->retry_count;
		if (current_retry_count < DK_MAX_RETRY) {
			/*
			 * we now need to update the
			 * cmd block to reflect the
			 * remaining list for the retry
			 */
			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_free_cmd(cmd_ptr);
			cmd_ptr = 
				scdisk_build_cmd(diskinfo,
				                (struct buf **) &first_bad_bp,
						 (char) FALSE);

			/*
			 * No resources free so exit this loop
			 */
			ASSERT(cmd_ptr != NULL);

			
			
			cmd_ptr->retry_count = current_retry_count;
			cmd_ptr->error_type = current_error_type;
			scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
			/*
			 * Falling out here will allow retry of
			 * updated operation.
			 */

		} else { /* retrys exceeded */


			/* setup to determine proper residual count */
			total_count = 0;
			current_bp = scbuf->bp;
			while (current_bp != NULL) {
				total_count += current_bp->b_bcount;
				current_bp = current_bp->av_forw;
			}


			/* Log error for failure */
			if (cmd_ptr->error_type != 0) {
				scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
			}





			/* Fail first bp in the chain of bp's  */
			/* and then retry the remaining bp's   */

			first_bad_bp =  first_bad_bp->av_forw;
			/* Set error values and flag bit */


			/*
			 *
			 *
			 *  Example (cont) of a failed transfer with 3 bufs.
			 *
			 *  B              B              
			 *  |-------|--------|--------|  = Buf addr boundaries
			 *          ^-----------------^  = total_count
			 *  ^-------^                    = good count
			 *              ^-------------^  = residual
			 *  |XXXXXXX|                    = Blocks returned good
			 *          ^                    = scbuf->bp
			 *          ^-------^            = scbuf->bp->b_bcount
			 *              ^---^            = scbuf->bp->b_resid
			 *                  ^            = first_bad_bp
			 *
			 */


			scbuf->bp->b_flags |= B_ERROR;
			scbuf->bp->b_error = scbuf->bufstruct.b_error;
			scbuf->bp->b_resid = (scbuf->bp->b_bcount - 
			    (total_count - residual));
			if (scbuf->bp->b_resid == 0) {
				/*
				 * If no resid then we will set it to
				 * the entire transfer, since we know
				 * this buf has failed.
				 */
				scbuf->bp->b_resid = scbuf->bp->b_bcount;
			}

			DDHKWD2(HKWD_DD_SCDISKDD, DD_SC_IODONE,
			    scbuf->bp->b_error, scbuf->bp->b_dev,
			    scbuf->bp);

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,sdiodone, trc,(char)4, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)scbuf->bp->b_error, 
	      (uint)scbuf->bp->b_resid,(uint)scbuf->bp->b_bcount);
#endif	
#endif	

			iodone(scbuf->bp);
			
			/*
			 * If wa are sending iodones back then clear 
			 * the reset_failure flag, even though this 
			 * is an potential error
			 */
			diskinfo->reset_failures = 0;

			scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			scdisk_free_cmd(cmd_ptr);
			/* Check for IN_PROGRESS queue not empty */
			if (first_bad_bp != NULL) {
				/* Build cmd for remaining queue */
				cmd_ptr = 
					scdisk_build_cmd(diskinfo,
					    (struct buf **) &first_bad_bp,
					    (char) FALSE);
				ASSERT(cmd_ptr != NULL);

				scdisk_q_cmd(cmd_ptr,(char) DK_STACK,
					     (uchar) DK_CMD_Q);
			}
		}
	} 

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,failbuf, exit,(char)0, (uint)diskinfo->state, 
	      (uint)0,(uint)0, (uint)0,(uint)0);
#endif	
#endif	

}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_ioctl_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of an IOCTL type	*/
/*		request. The operation is rescheduled if a retry	*/
/*		is feasible, else it will be marked as failed and	*/
/*		the requesting process will be awakened.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_ioctl_error(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;


	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,ioctlerr, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr->retry_count,(uint)cmd_ptr->subtype, 
	      (uint)cmd_ptr->aborted,(uint)0);
#endif	
#endif
	if ((cmd_ptr->retry_count >= DK_MAX_RETRY) ||
	    (cmd_ptr->subtype != DK_ERP_IOCTL) ||
	    (cmd_ptr->aborted)) {

		/*
		 * If maximum number of retries is reached or
		 * if this is an ioctl that does not get error
		 * recovery or if this is an aborted command
		 * then fail it.
		 */
		/* Log error for failure */
		if (cmd_ptr->error_type != 0) {
			scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
		}

		cmd_ptr->flags &= ~DK_READY;
		scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		cmd_ptr->intrpt = 0;
		e_wakeup(&(cmd_ptr->scbuf.bufstruct.b_event));
			
		/*
		 * If we are sending iodones back then clear 
		 * the reset_failure flag, even though this 
		 * is an error
		 */

		diskinfo->reset_failures = 0;
		diskinfo->ioctl_pending = FALSE;
	}
		
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,ioctlerr, exit,(char)0, (uint)0, (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_reset_error                              */
/*									*/
/*	FUNCTION:                                                       */
/*		This routine processes the failure of a Reset cycle	*/
/*		type request. A retry is scheduled if feasible, else	*/
/*		the device is marked RST_FAILED, since it could not be	*/
/*		initialized for use.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_reset_error(
struct dk_cmd	*cmd_ptr)
{
	struct sc_buf		*scbuf;
	struct scdisk_diskinfo	*diskinfo;

	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reseterr, entry,(char)0, (uint)cmd_ptr->diskinfo->state, 
	      (uint)cmd_ptr,(uint)cmd_ptr->retry_count, 
	      (uint)cmd_ptr->diskinfo->reset_count,(uint)0);
#endif	
#endif
	diskinfo->state &= ~DK_RESET_PENDING;

	scbuf = &(cmd_ptr->scbuf);
	if ((cmd_ptr->retry_count >= DK_MAX_RETRY) || 
	    (diskinfo->reset_count >= DK_MAX_RESETS)) {
		diskinfo->retry_flag = FALSE;
		/* Log error for failure */
		if (cmd_ptr->error_type != 0) {
			scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
		}

		diskinfo->errno = scbuf->bufstruct.b_error;
		/*
		 * Note: the following line will also clear DK_RESET_PENDING
		 */
		diskinfo->state = DK_RST_FAILED;
		diskinfo->reset_failures++;
		diskinfo->reset_count = 0;
		scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		scdisk_free_cmd(cmd_ptr);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reseterr, trc,(char)0, (uint)diskinfo->state, 
	      (uint)diskinfo->dk_cmd_q_head,(uint)0, (uint)0,(uint)0);
#endif	
#endif
		/* Set to next command on stack     */
		cmd_ptr = diskinfo->dk_cmd_q_head;
		if (cmd_ptr == NULL) {
			diskinfo->disk_intrpt = 0;
			e_wakeup((int *)&(diskinfo->open_event));
			if (diskinfo->reset_failures >= 
			     DK_MAX_RESET_FAILURES) {
				/*
				 * The reset cycle has failed too many
				 * times without any bufs or ioctls getting
				 * good completion. In other words we can't
				 * execute all commands required to have a disk
				 * open for operation.  Therefore we must give
				 * up and fail everything back to prevent a
				 * hang.  This disk is basically useless.
				 */
				diskinfo->reset_failures = 0;
				scdisk_fail_disk(diskinfo);

			}
		} else if (diskinfo->reset_failures >= DK_MAX_RESET_FAILURES) {
				/*
				 * The reset cycle has failed too many
				 * times without any bufs or ioctls getting
				 * good completion. In other words we can't
				 * execute all commands required to have a disk
				 * open for operation. Therefore we must give
				 * up and fail everything back to prevent a
				 * hang.  This disk is basically useless.
				 */

			diskinfo->reset_failures = 0;	
			scdisk_fail_disk(diskinfo);

			
		} else {
			cmd_ptr->retry_count = DK_MAX_RETRY;

			/* 
			 *  Set up the errno and resid in the scbuf for
			 *  this command.  Since the reset is the command
			 *  that failed, this information has not been set
			 *  up in the original scbuf, and the error routines
			 *  will be using it.
			 */

			cmd_ptr->scbuf.bufstruct.b_error = diskinfo->errno;
			cmd_ptr->scbuf.bufstruct.b_resid = cmd_ptr->scbuf.
			    bufstruct.b_bcount;

			/* Call the cmd specific error processor */
			switch (cmd_ptr->type) {
			case DK_BUF:
				scdisk_process_buf_error(cmd_ptr);
				break;
			case DK_IOCTL:
				scdisk_process_ioctl_error(cmd_ptr);
				break;
			case DK_RESET:
				scdisk_process_reset_error(cmd_ptr);
				break;
			case DK_REQSNS:
				scdisk_process_reqsns_error(cmd_ptr);
				break;
			case DK_WRITEV:
                        case DK_REASSIGN:
				scdisk_process_special_error(cmd_ptr);
				break;
			default:
				/* Unknown Cmd type */
				ASSERT(FALSE);
			}
		}

	}
	else {
		diskinfo->retry_flag = TRUE;
		/*
		 *  If this was a SCSI_BUS_FAULT and this is the last retry,
		 *  set the SC_ASYNC flag to try asynchronous transfers.
		 *  This is to handle the case where an asynchronous device
		 *  goes bus free instead of sending a message reject back
		 *  to the initiator when it tries to negotiate for a
		 *  synchronous transfer.  The SC_ASYNC flag will be
		 *  cleared when the next command is issued.
		 */
		if ((scbuf->status_validity & SC_ADAPTER_ERROR) &&
		    (scbuf->general_card_status == SC_SCSI_BUS_FAULT) &&
		    (cmd_ptr->retry_count == (DK_MAX_RETRY - 1))) {
			scbuf->scsi_command.flags = SC_ASYNC;
		}
	}

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reseterr, exit,(char)0, (uint)0, (uint)0,(uint)0, 
	      (uint)0,(uint)0);
#endif	
#endif
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_fail_disk		                        */
/*									*/
/*	FUNCTION:                                                       */
/*		This routine is called when it is impossible to		*/
/*		complete the necessary reset cycle for valid disk       */
/*		operations.  When this situation occurs we have no      */
/*		other option but to fail all pending commands for this  */
/*		disk and give up.  At least we will not causes a hang.  */
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_fail_disk(struct scdisk_diskinfo *diskinfo)
{
        struct buf *next_bp,                     /* Next buf in chain      */
		   *tmp_bp;                      /* temporary buf pointer  */
        struct dk_cmd  *cmd_ptr;		 /* command block structure*/
	int	errnoval=EIO;


	if (diskinfo->errno) {
		errnoval = diskinfo->errno;
	}

#ifdef DEBUG
#ifdef SC_ERROR_PATH
        scdisk_trc(diskinfo,faildisk, entry,(char)0,(uint)diskinfo, 
		   (uint)diskinfo->state, 
		   (uint)0, (uint)0,(uint)0);

#endif
#endif


	/*
	 * Set disk to DK_OFFLINE so no new commands will not be allowed  
	 * until we have flushed all pending commands back.
	 */
	diskinfo->state = DK_OFFLINE;
        while (diskinfo->dk_cmd_q_head != NULL) {
                /*
                 * while there are commands on this disks
                 * command queue, fail them all.
                 */
                cmd_ptr = diskinfo->dk_cmd_q_head;
		cmd_ptr->retry_count = DK_MAX_RETRY;
		cmd_ptr->scbuf.bufstruct.b_error = errnoval;
		cmd_ptr->scbuf.bufstruct.b_resid = cmd_ptr->scbuf.
			bufstruct.b_bcount;
		
		/* Call the cmd specific error processor */
		switch (cmd_ptr->type) {
		case DK_BUF:
			scdisk_process_buf_error(cmd_ptr);
			break;
		case DK_IOCTL:
			scdisk_process_ioctl_error(cmd_ptr);
			break;
		case DK_RESET:
			cmd_ptr->scbuf.bufstruct.b_flags |= B_DONE;
			scdisk_process_reset_error(cmd_ptr);
			break;
		case DK_REQSNS:
			scdisk_process_reqsns_error(cmd_ptr);
			break;
		case DK_WRITEV:
		case DK_REASSIGN:
			scdisk_process_special_error(cmd_ptr);
			break;
		case DK_Q_RECOV:
			scdisk_free_cmd(cmd_ptr);
			break;
		default:
			/* Unknown Cmd type */
			ASSERT(FALSE);
		}
        }
        if (diskinfo->ioctl_cmd.flags & DK_READY) {
                /*
                 * If there is a pending ioctl then
		 * fail it.
                 */

                cmd_ptr = &diskinfo->ioctl_cmd;
		cmd_ptr->flags &= ~DK_READY;
		cmd_ptr->intrpt = 0;	
		cmd_ptr->scbuf.bufstruct.b_error = errnoval;
		e_wakeup(&(cmd_ptr->scbuf.bufstruct.b_event));
        }


	if (diskinfo->dk_bp_queue.in_progress.head != NULL) {
		/*
		 * Fail any bufs in the in_progress queue
		 */

		tmp_bp = diskinfo->dk_bp_queue.in_progress.head;
		while (tmp_bp != NULL) {
			tmp_bp->b_flags |= B_ERROR;
			tmp_bp->b_error = errnoval;
			tmp_bp->b_resid = tmp_bp->b_bcount;
			next_bp = tmp_bp->av_forw;


#ifdef DEBUG
#ifdef SC_ERROR_PATH

			scdisk_trc(diskinfo,sdiodone, trc,(char)0, 
				   (uint)0, 
				   (uint)0,(uint)tmp_bp->b_error, 
				   (uint)tmp_bp->b_resid,
				   (uint)tmp_bp->b_bcount);

#endif
#endif
			iodone(tmp_bp);
			tmp_bp = next_bp;
		}
		diskinfo->dk_bp_queue.in_progress.head = NULL;
		diskinfo->dk_bp_queue.in_progress.tail = NULL;
	}
        tmp_bp = diskinfo->low;
        while( tmp_bp != NULL) {
                /*
                 * while there are bufs in this disks elevator
		 * fail them all.
                 */
		tmp_bp->b_error = errnoval;
		tmp_bp->b_flags |= B_ERROR;
		tmp_bp->b_resid = tmp_bp->b_bcount;
		next_bp = tmp_bp->av_forw;
		iodone(tmp_bp);
#ifdef DEBUG
#ifdef SC_ERROR_PATH

		scdisk_trc(diskinfo,sdiodone, trc,(char)1, 
			   (uint)0, 
			   (uint)0,(uint)tmp_bp->b_error, 
			   (uint)tmp_bp->b_resid,(uint)tmp_bp->b_bcount);
#endif
#endif

		tmp_bp = next_bp;
	}

        diskinfo->currbuf = diskinfo->low = NULL;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
        scdisk_trc(diskinfo,faildisk, exit,(char)0,(uint)diskinfo, 
		   (uint)diskinfo->state, 
	       (uint)0, (uint)0,(uint)0);

#endif
#endif
	diskinfo->state = DK_RST_FAILED;
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_reqsns_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of a Request Sense	*/
/*		operation. A retry is scheduled on this operation if	*/
/*		feasible, else the next operation on the stack is	*/
/*		scheduled for a retry.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_reqsns_error(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;



	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

	/* Log error for failure */
	if (cmd_ptr->error_type != 0) {
		scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
	}
	if (cmd_ptr->scbuf.adap_q_status & SC_DID_NOT_CLEAR_Q) {
		/*
		 * Lets flush the queue
		 */
		diskinfo->q_status |= SC_DID_NOT_CLEAR_Q;
	}
	else {
		/*
		 * If the adapter driver indicates the queue is clear
		 * then override any previous plans to clear the queue.
		 */
		diskinfo->q_status &= ~SC_DID_NOT_CLEAR_Q;
	}
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reqsnserror, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)cmd_ptr,(uint)diskinfo->q_status);
#endif	
#endif
	/*
	 * Remove reqsns op from stack and allow original
	 * Operation to retry...
	 */
	scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
	scdisk_free_cmd(cmd_ptr);
	diskinfo->state &= ~DK_REQSNS_PENDING;
	/*
	 * Get command that originally received check condition
	 * and put it at the top of cmd_q
	 */
	cmd_ptr = diskinfo->checked_cmd;

	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reqsnserror, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)cmd_ptr,(uint)cmd_ptr->type);
#endif	
#endif
	/* Call the cmd specific error processor */
	switch (cmd_ptr->type) {
	      case DK_BUF:
		scdisk_process_buf_error(cmd_ptr);
		break;
	      case DK_IOCTL:
		scdisk_process_ioctl_error(cmd_ptr);
		break;
	      case DK_RESET:
		scdisk_process_reset_error(cmd_ptr);
		break;
	      case DK_REQSNS:
		scdisk_process_reqsns_error(cmd_ptr);
		break;
	      case DK_WRITEV:
	      case DK_REASSIGN:
		scdisk_process_special_error(cmd_ptr);
		break;
	      default:
		/* Unknown Cmd type */
		ASSERT(FALSE);
	}

	 if (diskinfo->q_status & SC_DID_NOT_CLEAR_Q) {

		 /*
		  * Adapter has not cleared its Queue for this
		  * device . Issue q clear command to clear all
		  * commands on the device and adapter.
		  */

		 diskinfo->q_status &= ~SC_DID_NOT_CLEAR_Q;
		 cmd_ptr = scdisk_cmd_alloc(diskinfo,(uchar)DK_Q_RECOV);
		 ASSERT(cmd_ptr != NULL);
		 cmd_ptr->scbuf.flags = SC_Q_CLR;
		 scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
	 }

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reqsnserror, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)0,(uint)0);
#endif	
#endif

}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_special_error				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine processes the failure of Special requests	*/
/*		ie, requests that have dependencies on the remaining	*/
/*		requests on the stack. There are only two types of such */
/*		requests: reassign, and write verify.  If feasible a    */
/*		retry is scheduled, else the entire cmd stack must be   */
/*		failed.							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_special_error(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;
	char			errnoval;


	/*
	 * This type of operation is special (hence it's name)
	 * If the operation fails, all other operations below
	 * it on the stack must be failed also... (dependencies
	 * you see..) So we'll retry the op until it exceeds retries
	 * but then we have to clear the cmd stack if it hasn't
	 * succeeded by that point. Such is life.
	 */

	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,specialerr, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)cmd_ptr, 
	      (uint)cmd_ptr->type,(uint)cmd_ptr->retry_count);
#endif	
#endif
	
	if (cmd_ptr->retry_count >= DK_MAX_RETRY) {
		/* Log error for failure */
		if (cmd_ptr->error_type != 0) {
			scdisk_log_error(cmd_ptr, DK_HARD_ERROR);
		}

		/*  Save errno for failing the original buf struct */
		errnoval = cmd_ptr->scbuf.bufstruct.b_error;

		/* Remove Special cmd from cmd stack */
		scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);



		/*
		 *  If this was a write verify that failed, do not report an
		 *  error since the original read command was a recovered
		 *  error and the data is good.
		 */
		if (cmd_ptr->type == DK_WRITEV) {
			diskinfo->state &= ~DK_WRITEV_PENDING;
			scdisk_free_cmd(cmd_ptr);
			cmd_ptr = diskinfo->writev_err_cmd;
			scdisk_process_good(cmd_ptr);
			return;
		}

		/* 
		 * Else this must be a reassing command. 
		 * So set next cmd on stack to look like it failed also 
		 */

		scdisk_free_cmd(cmd_ptr);
		cmd_ptr = diskinfo->reassign_err_cmd;
		diskinfo->state &= ~DK_REASSIGN_PENDING;
		scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
		cmd_ptr->retry_count = DK_MAX_RETRY;
		cmd_ptr->scbuf.bufstruct.b_error = errnoval;
		cmd_ptr->scbuf.bufstruct.b_resid = 
		    cmd_ptr->scbuf.bufstruct.b_bcount;

		/* Call the cmd specific error processor */
		switch (cmd_ptr->type) {
		case DK_BUF:
			scdisk_process_buf_error(cmd_ptr);
			break;
		case DK_IOCTL:
			scdisk_process_ioctl_error(cmd_ptr);
			break;
		case DK_RESET:
			scdisk_process_reset_error(cmd_ptr);
			break;
		case DK_REQSNS:
			scdisk_process_reqsns_error(cmd_ptr);
			break;
		case DK_WRITEV:
		case DK_REASSIGN:
			scdisk_process_special_error(cmd_ptr);
			break;
		default:
			/* Unknown Cmd type */
			ASSERT(FALSE);
		}
	}
	else {
		if (cmd_ptr->type == DK_WRITEV) {
			diskinfo->state &= ~DK_WRITEV_PENDING;
		}
		else {
			diskinfo->state &= ~DK_REASSIGN_PENDING;
		}
	}

	/* Set device flag to restart ops at adapter level */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,specialerr, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,(uint)0, (uint)0,(uint)0);
#endif	
#endif
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_buf					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine performs completion processing on		*/
/*		Buf related requests that have been received by		*/
/*		scdisk_iodone with good return status.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		iodone							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_buf(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;
	struct sc_buf		*scbuf;
	struct buf		*curbp, *nextbp;
	uint			total_count;



	/* Get diskinfo struct for device */
	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;

	diskinfo->reset_failures = 0;

	/* Set up misc pointers for structs */
	scbuf = &(cmd_ptr->scbuf);

#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,processbuf, entry,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)0, (uint)0,(uint)0);
#endif	
#endif	
	/* Check for a recovered error during processing, log any */
	if (cmd_ptr->error_type != 0) {
		scdisk_log_error(cmd_ptr, DK_SOFT_ERROR);
	}

	if (cmd_ptr->group_type == DK_SINGLE) {
		/* Cmd is single operation */
		curbp = cmd_ptr->bp;
		/*
		 * Check the soft_resid variable to see if this command was
		 * recovered with retries and needs to be relocated by
		 * reporting an ESOFT.
		 */
		if (cmd_ptr->soft_resid) {
			curbp->b_flags |= B_ERROR;
			curbp->b_error = ESOFT;
			curbp->b_resid = cmd_ptr->soft_resid;
		} else {
			curbp->b_flags &= ~B_ERROR;
			curbp->b_error = 0;
		}

		DDHKWD2(HKWD_DD_SCDISKDD, DD_SC_IODONE, curbp->b_error,
		    curbp->b_dev, curbp);
		if (curbp->b_flags & B_READ) {
			diskinfo->dkstat.dk_rblks += (curbp->b_bcount) / 
			    diskinfo->block_size;
		} else {
			diskinfo->dkstat.dk_wblks += (curbp->b_bcount) / 
			    diskinfo->block_size;
		}
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,sdiodone, trc,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)curbp->b_error, 
	      (uint)curbp->b_resid,(uint)curbp->b_bcount);
#endif	
#endif	
		iodone(curbp);


		scdisk_free_cmd(cmd_ptr);

	} else if (cmd_ptr->group_type == DK_COALESCED) {
		/*
		 * Cmd is coalesced operation
		 * Flush IN_PROGRESS queue
		 */
		curbp = scbuf->bp;
		total_count = scbuf->bufstruct.b_bcount;
		while (curbp != NULL) {
			total_count -= curbp->b_bcount;
			/*
			 *  Check the soft_resid variable to see if this is
			 *  a recovered media error that should be relocated
			 *  by reporting ESOFT.
			 */
			if (cmd_ptr->soft_resid > total_count) {
				curbp->b_flags |= B_ERROR;
				curbp->b_error = ESOFT;
				curbp->b_resid = cmd_ptr->soft_resid - 
				    total_count;
				cmd_ptr->soft_resid = 0;
			} else {
				curbp->b_flags &= ~B_ERROR;
				curbp->b_error = 0;
				curbp->b_resid = 0;
			}

			nextbp = curbp->av_forw;
			DDHKWD2(HKWD_DD_SCDISKDD, DD_SC_IODONE, curbp->b_error,
			    curbp->b_dev, curbp);
			if (curbp->b_flags & B_READ) {
				diskinfo->dkstat.dk_rblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			} else {
				diskinfo->dkstat.dk_wblks +=
				    (curbp->b_bcount) / diskinfo->block_size;
			}
			iodone(curbp);
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,sdiodone, trc,(char)1, (uint)cmd_ptr->error_type, 
	      (uint)cmd_ptr->group_type,(uint)curbp->b_error, 
	      (uint)curbp->b_resid,(uint)curbp->b_bcount);
#endif	
#endif
			curbp = nextbp;
		}

		scdisk_free_cmd(cmd_ptr);
	} 
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,processbuf, exit,(char)0, (uint)cmd_ptr->error_type, 
	      (uint)0,(uint)0, (uint)0,(uint)0);
#endif	
#endif

}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_process_reset					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine handles completion processing of		*/
/*		RESET type commands that have been received by		*/
/*		scdisk_iodone with successful completion status.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		e_wakeup						*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_process_reset(
struct dk_cmd	*cmd_ptr)
{
	struct scdisk_diskinfo	*diskinfo;
	int	minutes,seconds,frames;
	int	bcd_minutes,bcd_seconds,bcd_frames;




	/* Get diskinfo struct for device */
	diskinfo = cmd_ptr->diskinfo;

	/*
	 * Determine what cmd, if any, is next in the cycle
	 * and set it up for starting on exit
	 */
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,processreset, entry,(char)0, (uint)cmd_ptr->diskinfo, 
	      (uint)cmd_ptr->subtype,(uint)diskinfo->dk_cmd_q_head, 
	      (uint)0,(uint)0);
#endif	
#endif
	switch (cmd_ptr->subtype) {
	case DK_START:
		/* A START UNIT cmd has returned */

		scdisk_free_cmd(cmd_ptr);
		scdisk_test_unit_ready(diskinfo);
		break;
	case DK_TUR:
		/* A TEST UNIT READY cmd has returned */
		scdisk_free_cmd(cmd_ptr);
		if (diskinfo->reserve_lock) {
			/*	
			 * If reserve_lock is true then we will	
			 * issue a reserve command.
			 */
			scdisk_reserve(diskinfo);
		}
		else {
			/*	
			 * If reserve_lock is false then we will	
			 * skip reserve command and go on to 
			 * next command.
			 */

		  	if (diskinfo->prevent_eject) {
		    		/*
		     	 	 * If prevent_eject is true then we must
		     	 	 * issue a prevent media removal command next.
		     	 	 */
		    		scdisk_prevent_allow(diskinfo,
						     (uchar) DK_PREVENT);
			}
			else {
				/*	
				 * If prevent_eject is false then we will	
				 * skip prevent media removal command and 
				 * go on to mode sense command.
				 */
				diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
				scdisk_mode_sense(diskinfo);
			}
		}
		break;

	case DK_RESERVE:
		/* A RESERVE UNIT cmd has returned */

		scdisk_free_cmd(cmd_ptr);
		if (diskinfo->prevent_eject) {

			/*
			 * If prevent_eject is true then we must
			 * issue a prevent media removal command next.
			 */
			scdisk_prevent_allow(diskinfo,(uchar) DK_PREVENT);
		}
		else {
			/*	
			 * If prevent_eject is false then we will	
			 * skip prevent media removal command and go on to 
			 * mode sense command.
			 */
			diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
			scdisk_mode_sense(diskinfo);
		}
		break;
	case DK_PREVENT:
		/* A Prevent Media removal cmd has returned */

		scdisk_free_cmd(cmd_ptr);
		diskinfo->m_sense_status = DK_SENSE_CHANGEABLE;
		scdisk_mode_sense(diskinfo);


		break;
	case DK_MSENSE:
		/* A MODE SENSE cmd has returned */

		scdisk_free_cmd(cmd_ptr);

		if (diskinfo->m_sense_status == DK_SENSE_CHANGEABLE) {
			/*
			 * if this was the sense of changeable 
			 * attributes, then prepare to sense current
			 * attributes. First, copy changeable data
			 * into separate data buffer, and format the
			 * data
			 */
        		bcopy (diskinfo->sense_buf, diskinfo->ch_data, 256);
			scdisk_format_mode_data((char *) diskinfo->ch_data, 
				(struct scdisk_mode_format *)&diskinfo->ch, 
						(int) (diskinfo->ch_data[0]+1),
						(char) FALSE,
						(struct scdisk_diskinfo *)NULL);
			diskinfo->m_sense_status = DK_SENSE_CURRENT;
                       	scdisk_mode_sense(diskinfo);
		} else {
			/*
			 * this was the sense of current data, so
			 * format the data and then determine if
			 * a select is necessary
			 */
			scdisk_format_mode_data((char *) diskinfo->sense_buf, 
				  (struct scdisk_mode_format *) &diskinfo->cd,
				  (int)(diskinfo->sense_buf[0]+1),
				  (char) FALSE,
				  (struct scdisk_diskinfo *) NULL);

                        if ((diskinfo->multi_session) && 
			    (diskinfo->valid_cd_modes &  DK_M2F1_VALID) &&
			    (!(diskinfo->ioctl_chg_mode_flg) ||
			     (diskinfo->current_cd_mode == CD_MODE2_FORM1))) {
                                /*
                                 * If this drive supports multi-session CDs
				 * and has valid CD-ROM XA Data Mode 2 Form1
				 * code and we are not doing a change mode
				 * ioctl to a CD-ROM Data mode other then
				 * Mode 2 Form 1, then we need to determine 
				 * what data mode the CD is in.
                                 */
                                scdisk_read_disc_info(diskinfo);
                        }
                        else {
                                /*
                                 * If this is drive does not support
                                 * multi-session CDs then see if a 
                                 * mode select is necessary
                                 */
				diskinfo->last_ses_pvd_lba = 0;
                                if (scdisk_mode_data_compare(diskinfo)) {
                                        /*
                                         * if a mode select is necessary
                                         */
                                        scdisk_mode_select(diskinfo);
                                }
                                else {
                                        /*
                                         * bypass the mode select
                                         */
                                        scdisk_read_cap(diskinfo);
                                }
                        }

		}
		break;
        case DK_READ_INFO:

                if (diskinfo->disc_info[0] == 0x20) {
                        /*
                         * This disk is a CD-ROM XA disc that
                         * may be a photo CD.
                         */
                        diskinfo->current_cd_mode = CD_MODE2_FORM1;
                        diskinfo->current_cd_code = 
                                diskinfo->cd_mode2_form1_code;
                        diskinfo->block_size = DK_M2F1_BLKSIZE;
                        diskinfo->mult_of_blksize = 
                                diskinfo->block_size/DK_BLOCKSIZE;

			/*
			 * NOTE: The MSF format is in BCD notation
			 * and will need to be converted if non-zero.
			 */
                        bcd_minutes = diskinfo->disc_info[1];
                        bcd_seconds = diskinfo->disc_info[2];
                        bcd_frames = diskinfo->disc_info[3];
                        if ((bcd_minutes == 0) && (bcd_seconds == 0) &&
                            (bcd_frames == 0)) {
                                /*
                                 * This indicates one of the following
                                 * situations:
                                 * 1. The media is not a multi-session
                                 *    photo CD.
                                 *
                                 * 2. The media has only one session on it.
                                 *    For either of these cases, we do not need
                                 *    to re-map lba . 
                                 */
                                diskinfo->last_ses_pvd_lba = 0;
                        }
                        else {
				/*
				 * Convert BCD MSF to actual MSF
				 */

				minutes = ((0xf0 & bcd_minutes) >> 4) * 10 + 
						(0x0f & bcd_minutes);
				seconds = ((0xf0 & bcd_seconds) >> 4) * 10 + 
						(0x0f & bcd_seconds);
				frames = ((0xf0 & bcd_frames) >> 4) * 10 + 
						(0x0f & bcd_frames);

                                /* 
                                 * Convert MSF format to lba 
				 * NOTE: This conversion is only for
				 * Atlantis MM CD-ROMS with blocksize of 2K.
                                 */
                                diskinfo->last_ses_pvd_lba =
                                        (minutes * 75 * 60) +
                                        (seconds * 75) +
                                        (frames - 150) +
                                        DK_CD_PVD; 
                        }
                }
                else {

			/*
			 * This CD is not a multi-session
			 * photo CD, so do not re-map lba DK_CD_PVD
			 */
			diskinfo->last_ses_pvd_lba = 0;

			if (!(diskinfo->ioctl_chg_mode_flg)) {
				/*
				 * If we have not done any change data
				 * mode ioctls then we want to 
				 * set up this drive for Mode 1.
				 *
				 * If we have used the change data mode
				 * ioctl then we don't want
				 * to overwrite the data mode. This
				 * is because we assume the user knows
				 * what they are doing.
				 */


				diskinfo->block_size = 
					diskinfo->cfg_block_size;
				diskinfo->mult_of_blksize = 
					diskinfo->block_size/DK_BLOCKSIZE;
				diskinfo->current_cd_mode = CD_MODE1;
				diskinfo->current_cd_code = 
					diskinfo->cd_mode1_code;
			}
                }

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,processreset, trc,(char)0, (uint)diskinfo->block_size, 
	      (uint)diskinfo->mult_of_blksize,(uint)diskinfo->current_cd_mode, 
	      (uint)diskinfo->current_cd_code,(uint)diskinfo->last_ses_pvd_lba);

   scdisk_trc(diskinfo,processreset, trc,(char)1, (uint)diskinfo->disc_info[0], 
	      (uint)diskinfo->disc_info[1],(uint)diskinfo->disc_info[2], 
	      (uint)diskinfo->disc_info[3],(uint)diskinfo->last_ses_pvd_lba);
#endif	
#endif
                scdisk_free_cmd(cmd_ptr);       
                /*
                 * Determine if a mode select is necessary
                 */
                if (scdisk_mode_data_compare(diskinfo)) {
                        /*
                         * if a mode select is necessary
                         */
                        scdisk_mode_select(diskinfo);
                }
                else {
                        /*
                         * bypass the mode select
                         */
                        scdisk_read_cap(diskinfo);
                }       
                break;

	case DK_SELECT:
		/* A MODE SELECT cmd has returned */

		scdisk_free_cmd(cmd_ptr);
		scdisk_read_cap(diskinfo);
		break;
	case DK_READCAP:
		/*
		 * A READ CAPACITY cmd has returned
		 * This is the end of the RESET cycle
		 */
		/*
		 *  Check the blocksize of the drive.
		 */
		if (diskinfo->capacity.len != (int) diskinfo->block_size) {
			cmd_ptr->retry_count = DK_MAX_RETRY;
			cmd_ptr->scbuf.bufstruct.b_error = EMEDIA;
			scdisk_build_error(cmd_ptr, DK_MEDIA_ERR,
			    DK_NOOVERLAY_ERROR, 0);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,processreset, trc,(char)2, (uint)diskinfo->block_size, 
	      (uint)diskinfo->capacity.len,(uint)diskinfo->capacity.lba, 
	      (uint)cmd_ptr,(uint)0);
#endif	
#endif
			scdisk_process_reset_error(cmd_ptr);
			return;
		}
		diskinfo->reset_count = 0;

		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));

		scdisk_free_cmd(cmd_ptr);
		break;
	case DK_STOP:

		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));
		scdisk_free_cmd(cmd_ptr);
		break;
	case DK_ALLOW:
		scdisk_free_cmd(cmd_ptr);	
		if ((diskinfo->reserve_lock) &&
		    (!diskinfo->retain_reservation)) {
		  	/*
			 * A reserve was done on open and
			 * no retain reservation after close
			 * was specified.
			 */
			scdisk_release(diskinfo);
		}
		else {
			diskinfo->disk_intrpt = 0;
			e_wakeup((int *)&(diskinfo->open_event));
		}
		break;
	case DK_RELEASE:

		diskinfo->disk_intrpt = 0;
		e_wakeup((int *)&(diskinfo->open_event));
		scdisk_free_cmd(cmd_ptr);
		break;

	default:
		/* Process unknown cmd subtype */
		ASSERT(FALSE);
		break;
	}
	diskinfo->state &= ~DK_RESET_PENDING;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,processreset, exit,(char)0, (uint)diskinfo, (uint)0,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif	
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_coalesce						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine scans the PENDING queue for the		*/
/*		specified device. The goal is to minimize the number of	*/
/*		SCSI operations that must be performed to satisfy	*/
/*		the requests in the PENDING queue. Requests will	*/
/*		grouped by one of the following rules:			*/
/*									*/
/*		1 - Contiguous write operations				*/
/*		2 - Operations requiring special processing		*/
/*									*/
/*		The coalesced requests will be removed from the		*/
/*		PENDING queue and placed in the IN_PROGRESS queue	*/
/*		so that a single SCSI command may be built to		*/
/*		satisfy the requests.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_coalesce(
struct scdisk_diskinfo	*diskinfo)
{
	struct buf		*bp;
	int			total_count, next_block, rwflag;
	int			rc;
	int			blkno;

	total_count = 0;
	next_block = 0;
	rwflag = 0;

	/* Start coalescing with current buf of pending queue*/
	bp = diskinfo->currbuf;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,coales, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)bp, (uint)bp->b_bcount,(uint)bp->b_options);
#endif	
#endif
	/*
	 * Check for reassign operations
	 * Do not coalesce these operations
	 */
	if ((bp->b_options & (HWRELOC | UNSAFEREL))) {
		/* Remove bp from PENDING queue */
		scdisk_pending_dequeue(diskinfo);

		/* Place bp IN_PROGRESS queue */

		SCDISK_IN_PROG_ENQUEUE(diskinfo, bp);
		return;
	}
	/* Check for retried commands */
	if (bp->b_resid != 0) {
		/* Remove bp from PENDING queue */
		scdisk_pending_dequeue(diskinfo);

		/* Place bp IN_PROGRESS queue */

		SCDISK_IN_PROG_ENQUEUE(diskinfo, bp);
		return;
	}

	/* Remove bp from PENDING queue */
	scdisk_pending_dequeue(diskinfo);

	/* Place bp IN_PROGRESS queue */

	SCDISK_IN_PROG_ENQUEUE(diskinfo, bp);

	if (!(bp->b_options & (HWRELOC | UNSAFEREL)) &&
	    (bp->b_bcount <= diskinfo->max_coalesce) &&
	    (((int)bp->b_un.b_addr & 0x0fff) == 0) &&
	    ((bp->b_bcount & 0x0fff) == 0) &&
	    (bp->b_resid == 0)) {

		 SCDISK_GET_LBA(blkno,diskinfo,bp,rc); 
		 if (rc) {
			  /*
			   * Request does not start on device
			   * block boundary.  
                           * This should be filtered at 
			   * scdisk_strategy.
                           */
		   	   ASSERT(rc == 0);
		}
		next_block = blkno + 
		  		(bp->b_bcount /diskinfo->block_size);
		total_count += bp->b_bcount;
		rwflag = (bp->b_flags & B_READ);

		/* Check next buf in list for continued coalescing */
		bp = diskinfo->currbuf;
		if (bp != NULL) {
			SCDISK_GET_LBA(blkno,diskinfo,bp,rc); 
			if (rc) {
				/*
				 * Request does not start on device
				 * block boundary.  
				 * This should be filtered at 
				 * scdisk_strategy.
				 */
				ASSERT(rc == 0);
			}
		}
		/* Begin coalescing operations */
		while ((bp != NULL) &&
		       (rwflag == (bp->b_flags & B_READ)) &&
		       !(bp->b_options & (HWRELOC | UNSAFEREL)) &&
		       (blkno == next_block) &&
		       ((total_count + bp->b_bcount) <= 
			diskinfo->max_coalesce) &&
		       (((int)bp->b_un.b_addr & 0x0fff) == 0) &&
		       ((bp->b_bcount & 0x0fff) == 0) &&
		       (bp->b_resid == 0)) {
	
                        /*
			 * Conditions to be able to coalesce:
                         * 1) there is a next one to look at
                         * 2) the next one does not involve relocation
                         * 3) the next request is sequential with current one
                         * 4) the next one is the same direction transfer 
			 *    as this one (i.e. rwflag is read, bp is read) 
                         * 5) the next one begins on a page boundary
			 * 6) the next one will end on a page boundary
			 * 7) the next one won't exceed coalesce maximum
                         */

			/* Remove bp from PENDING queue */
			scdisk_pending_dequeue(diskinfo);
	
			/* Place bp IN_PROGRESS queue */

			SCDISK_IN_PROG_ENQUEUE(diskinfo, bp);


			next_block = blkno + 
			    (bp->b_bcount / diskinfo->block_size);
			total_count += bp->b_bcount;
			rwflag = (bp->b_flags & B_READ);

			/* Check next buf in list for continued coalescing */
			bp = diskinfo->currbuf;
			if (bp != NULL) {

				SCDISK_GET_LBA(blkno,diskinfo,bp,rc);
				if (rc) {
					/*
					 * Request does not start on device
					 * block boundary.  
					 * This should be filtered at 
					 * scdisk_strategy.
					 */
					ASSERT(rc == 0);
				}
			}

		}
	}

#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,coales, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_bp_queue.in_progress.head, (uint)0,(uint)0);
#endif	
#endif
	return;
}




/************************************************************************/
/*									*/
/*	NAME:	scdisk_build_cmd					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine scans the IN_PROGRESS queue for the	*/
/*		specified device and builds a SCSI operation to		*/
/*		satisfy the request list. The IN_PROGRESS queue		*/
/*		may contain one of three different types of cmd		*/
/*		list. The list will be either:				*/
/*		1 - a group of contiguous write operations		*/
/*		(possibly including a verify flag)			*/
/*		2 - a reassign block and write operation		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*		This routine will build the following SCSI commands:    */
/*									*/
/*                        READ(6) Command        			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (08h)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | (MSB)                            |	*/
/*  |-----+-----------------------                             ---|	*/
/*  | 2   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 4   |                    Transfer Length                    |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 5   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        READ(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (28h)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        WRITE(6) Command        			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (0Ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | (MSB)                            |	*/
/*  |-----+-----------------------                             ---|	*/
/*  | 2   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 4   |                    Transfer Length                    |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 5   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        WRITE(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (2ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                      WRITE AND VERIFY  Command			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                  Operation Code (2Eh)                 |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   | Logical Unit Number| DPO  |Reserv|Reserv|BytChk|RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+----                                              -----|	*/
/*  | 3   |                                                       |	*/
/*  |-----+----                                              -----|	*/
/*  | 4   |               Logical Block Address                   |	*/
/*  |-----+----                                              -----|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                     Reserved                          |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)               Transfer Length                   |	*/
/*  |-----+----                                              -----|	*/
/*  | 8   |                                                (LSB)  |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                     Control                           |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the address of a disk cmd		*/
/*		structure which has been built to satisfy the		*/
/*		requests in the IN_PROGRESS queue for the device.	*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

struct dk_cmd *
scdisk_build_cmd(
struct scdisk_diskinfo	*diskinfo,
struct buf **head,
char   good_path)
{
	int			blk_cnt, lba, total_count, verify;
	struct dk_cmd		*cmd_ptr;
	struct buf		*cmd_bp,*bp, *tmp_bp;
	struct scsi		*scsi;
	struct scdisk_def_list	*def_list;
	int			blkno,rc;


	if ((diskinfo->reassign_cmd.status != DK_FREE) && (good_path == TRUE))
		return(NULL);

	verify = FALSE;
        /* Start with head of IN_PROGRESS queue */
        bp = *head;


#ifdef DEBUG
#ifdef SC_GOOD_PATH

   scdisk_trc(diskinfo,buildcmd, trc,(char)0, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);
#endif	
#endif
	if (bp == NULL) {
#ifdef DEBUG
		DKprintf(("scdisk_build_cmd: NULL in_progress.head\n"));
#endif
		ASSERT(FALSE);
	}

	SCDISK_GET_LBA(blkno,diskinfo,bp,rc);
	if (rc) {
	  	/*
		 * Request does not start on device
		 * block boundary.  
		 * This should be filtered at 
		 * scdisk_strategy.
		 */
	  	ASSERT(rc == 0);
	}
	if ((bp->b_options & (HWRELOC | UNSAFEREL)) && (good_path == TRUE)) {
#ifdef DEBUG
		DKprintf(("scdisk_build_cmd building RELOC & WRITE\n"));
#endif

		if (diskinfo->cmds_out) {
			/*
			 * Must starve the queue for reassigns
			 */
			return(NULL);
		}
		/* allocate cmd for write operation */

		cmd_ptr = scdisk_cmd_alloc(diskinfo,(uchar) DK_BUF);
		if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {
                         DDHKWD3(HKWD_DD_SCDISKDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->scbuf));
		 }		
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;

		/* Initialize cmd block for WRITE op */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		cmd_bp->b_bcount = bp->b_bcount;
		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t) &(cmd_bp->b_xmemd),
		    sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*) ())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->scbuf.bp = NULL;
		cmd_ptr->scbuf.status_validity = 0x00;
		if (diskinfo->cmd_tag_q) {
			/*
			 * When this command is finally sent
			 * after the reassign.  We would like it to
			 * be used by the disk as soon as possible.
			 */
			cmd_ptr->scbuf.q_tag_msg = SC_HEAD_OF_Q;
		}
		else
			cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		cmd_ptr->group_type = DK_SINGLE;
		cmd_ptr->bp = bp;

		/* Initialize SCSI cmd for operation */
		blk_cnt = bp->b_bcount / diskinfo->block_size;
		lba = blkno;
		scsi = &(cmd_ptr->scbuf.scsi_command);
		if (bp->b_options & WRITEV) {
			/* Initialize SCSI cmd for Write with verify */
			scsi->scsi_length = 10;
			scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_AND_VERIFY;

			scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 24) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = ((lba >> 16) & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = ((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[4] = 0x00;
			scsi->scsi_cmd.scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[6] = (blk_cnt & 0xff);
			scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		} else if ((blk_cnt & 0xffffff00) || (lba & 0xffe00000)) {
			/*
			 * if the command won't fit in a 6 byte cmd use a
			 * 10 byte command instead.  
			 */
			scsi->scsi_length = 10;
			scsi->scsi_id = diskinfo->scsi_id;
			scsi->flags = (ushort) diskinfo->async_flag;
			scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_EXTENDED;

			scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 24) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = ((lba >> 16) & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = ((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[4] = 0x00;
			scsi->scsi_cmd.scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[6] = (blk_cnt & 0xff);
			scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		} else {
			/*
			 *  The command will fit in a 6 byte command block.
			 */
			scsi->scsi_length = 6;
			scsi->scsi_id = diskinfo->scsi_id;
			scsi->flags = (ushort) diskinfo->async_flag;
			scsi->scsi_cmd.scsi_op_code = SCSI_WRITE;
			scsi->scsi_cmd.lun |= ((lba >> 16) & 0x1f);
			scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = blk_cnt;
			scsi->scsi_cmd.scsi_bytes[3] = 0x00;
		}


                /*
		 *              REASSIGN BLOCKS Defect List
		 *+=====-=====-=====-=====-=====-=====-=====-=====-=====+
		 *|  Bit|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
		 *|Byte |     |     |     |     |     |     |     |     |
		 *|=====+===============================================|
		 *| 0   |                 Reserved                      |
		 *|-----+-----------------------------------------------|
		 *| 1   |                 Reserved                      |
		 *|-----+-----------------------------------------------|
		 *| 2   | (MSB)                                         |
		 *|-----+---              Defect List Length         ---|
		 *| 3   |                                         (LSB) |
		 *|=====+===============================================|
		 *|     |                 Defect Descriptor(s)          |
		 *|=====+===============================================|
		 *| 0   | (MSB)                                         |
		 *|- - -+---              Defect Logical Block Address -|
		 *| 3   |                                         (LSB) |
		 *|-----+-----------------------------------------------|
		 *|     |                       .                       |
		 *|     |                       .                       |
		 *|-----+-----------------------------------------------|
		 *| 0   | (MSB)                                         |
		 *|- - -+---              Defect Logical Block Address -|
		 *| 3   |                                         (LSB) |
		 *+=====================================================+
		 */

		/* Build a defect list for the reassignment */
		def_list = &(diskinfo->def_list);
		def_list->header[0] = 0x00;
		def_list->header[1] = 0x00;
		def_list->header[2] = 0x00;
		def_list->header[3] = 0x04;
		def_list->descriptor[0] = ((lba >> 24) & 0xff);
		def_list->descriptor[1] = ((lba >> 16) & 0xff);
		def_list->descriptor[2] = ((lba >> 8) & 0xff);
		def_list->descriptor[3] = (lba & 0xff);

		/* allocate cmd for reassign operation */
		diskinfo->reassign_err_cmd = cmd_ptr;
		cmd_ptr = &(diskinfo->reassign_cmd);
		if (cmd_ptr->status != DK_FREE) {
			ASSERT(cmd_ptr->status != DK_FREE);
		}
		cmd_ptr->status |= DK_IN_USE;
		cmd_ptr->type = DK_REASSIGN;  
		cmd_ptr->subtype = 0x00;     
		cmd_ptr->next = NULL;    
		cmd_ptr->prev = NULL; 
		cmd_ptr->aborted = FALSE;

		/* Initialize cmd block for REASSIGN op */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = (B_WRITE | B_BUSY);
		cmd_bp->b_options = 0x00;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = 0x00;
		cmd_bp->b_bcount = sizeof(struct scdisk_def_list);
		cmd_bp->b_un.b_addr = (caddr_t) def_list;
		cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
		cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->reassign_timeout;
		cmd_ptr->scbuf.status_validity = 0x00;
		if (diskinfo->cmd_tag_q)
			cmd_ptr->scbuf.q_tag_msg = SC_HEAD_OF_Q;
		else
			cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		cmd_ptr->group_type = DK_SINGLE;
		cmd_ptr->scbuf.bp = NULL;
		cmd_ptr->bp = NULL;

		/* Initialize SCSI cmd for operation */

           /*                        REASSIGN BLOCKS Command        
            *  +=====-======-======-======-======-======-======-======-======+
            *  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
            *  |Byte |      |      |      |      |      |      |      |      |
            *  |=====+=======================================================|
            *  | 0   |                Operation Code (07h)                   |
            *  |-----+-------------------------------------------------------|
            *  | 1   |Logical Unit Number |       Reserved                   |
            *  |-----+-------------------------------------------------------|
            *  | 2   |                    Reserved                           |
            *  |-----+-------------------------------------------------------|
            *  | 3   |                    Reserved                           |
            *  |-----+-------------------------------------------------------|
            *  | 4   |                    Reserved                           |
            *  |-----+-------------------------------------------------------|
            *  | 5   |                    Control                            |
            *  +=============================================================+
	    */
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_length = 6;
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		scsi->scsi_cmd.scsi_op_code = SCSI_REASSIGN_BLOCK;

		scsi->scsi_cmd.scsi_bytes[0] = 0x00;
		scsi->scsi_cmd.scsi_bytes[1] = 0x00;
		scsi->scsi_cmd.scsi_bytes[2] = 0x00;
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;

		/* Place cmd on device's cmd stack for execution */



		diskinfo->state |= DK_REASSIGN_PENDING;
		/*
		 * Clear in progress queue since we just processed it
		 */
		*head  = NULL;

		return(cmd_ptr);
        }
	if (bp->b_resid != 0) {     /* setup for end of file read/write */

		/* allocate cmd for read/write operation */
#ifdef DEBUG
#ifdef SC_GOOD_PATH
	scdisk_trc(diskinfo,buildcmd, trc,(char)1, (uint)diskinfo->state, 
		   (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);
#endif	
#endif

                cmd_ptr = scdisk_cmd_alloc(diskinfo,(uchar) DK_BUF);
	        if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {
                         DDHKWD3(HKWD_DD_SCDISKDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->scbuf));
		 }
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;

		/* Initialize cmd block for READ/WRITE op */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		
		cmd_bp->b_bcount = bp->b_bcount - bp->b_resid;
		cmd_ptr->group_type = DK_SINGLE;

		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t) &(cmd_bp->b_xmemd),
		    sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->scbuf.bp = NULL;
		cmd_ptr->scbuf.status_validity = 0x00;
		cmd_ptr->scbuf.q_tag_msg = diskinfo->q_type;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		cmd_ptr->bp = bp;

		/* Initialize SCSI cmd for operation */
		blk_cnt = cmd_bp->b_bcount / diskinfo->block_size;
		lba = blkno;
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_length = 6;
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		if (!(bp->b_flags & B_READ) && (bp->b_options & WRITEV)) {
			/* Initialize SCSI cmd for Write with verify */
			scsi->scsi_length = 10;
			scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_AND_VERIFY;

			scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 24) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = ((lba >> 16) & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = ((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[4] = 0x00;
			scsi->scsi_cmd.scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[6] = (blk_cnt & 0xff);
			scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		} else {
			if (bp->b_flags & B_READ) {
				if ((blk_cnt & 0xffffff00) || 
				    (lba & 0xffe00000) ||
				    (diskinfo->dev_type != DK_DISK)) {
					/*
					 * if the command won't fit in a 6 byte
					 * cmd use a 10 byte command instead. 
					 * Or if this is a CDROM or read/write 
					 * optical drive SCSI-2 only says
					 * that read extended is mandatory so 
					 * use the 10 byte
					 * command.
					 */
					scsi->scsi_cmd.scsi_op_code = 
					    SCSI_READ_EXTENDED;
				} else {
					scsi->scsi_cmd.scsi_op_code = 
					    SCSI_READ;
				}
			} else {
				if ((blk_cnt & 0xffffff00) || 
				    (lba & 0xffe00000)) {
					scsi->scsi_cmd.scsi_op_code = 
					    SCSI_WRITE_EXTENDED;
				} else {
					scsi->scsi_cmd.scsi_op_code = 
					    SCSI_WRITE;
				}
			}
			if ((scsi->scsi_cmd.scsi_op_code == 
			    SCSI_READ_EXTENDED) ||
			    (scsi->scsi_cmd.scsi_op_code == 
			    SCSI_WRITE_EXTENDED)) {
				scsi->scsi_length = 10;

				scsi->scsi_cmd.scsi_bytes[0] = 
				    ((lba >> 24) & 0xff);
				scsi->scsi_cmd.scsi_bytes[1] = 
				    ((lba >> 16) & 0xff);
				scsi->scsi_cmd.scsi_bytes[2] = 
				    ((lba >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[3] = 
				    (lba & 0xff);
				scsi->scsi_cmd.scsi_bytes[4] = 
				    0x00;
				scsi->scsi_cmd.scsi_bytes[5] = 
				    ((blk_cnt >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[6] = 
				    (blk_cnt & 0xff);
				scsi->scsi_cmd.scsi_bytes[7] = 
				    0x00;
			} else {
				scsi->scsi_cmd.lun |=
				    ((lba >> 16) & 0x1f);
				scsi->scsi_cmd.scsi_bytes[0] = 
				    ((lba >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[1] = (lba & 0xff);
				scsi->scsi_cmd.scsi_bytes[2] = blk_cnt;
				scsi->scsi_cmd.scsi_bytes[3] = 0x00;
			}
		}


		/*
		 * Clear in progress queue since we just processed it
		 */
		*head  = NULL;
		return(cmd_ptr);
	}
	if (bp->b_flags & B_READ) {
		/* allocate cmd for read operation */
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,buildcmd, trc,(char)2, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);

#endif	
#endif
		cmd_ptr = scdisk_cmd_alloc(diskinfo,(uchar)DK_BUF);
		if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {
			DDHKWD3(HKWD_DD_SCDISKDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->scbuf));
		}
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;

		
#ifdef DEBUG
		DKprintf(("scdisk_build_cmd building READ from \
				coalesced list\n"));
#endif
		cmd_ptr->scbuf.bp = bp;
		total_count = 0x00;
		tmp_bp = bp;
		while (tmp_bp != NULL) {
			total_count += tmp_bp->b_bcount;

			tmp_bp = tmp_bp->av_forw;
		}
		
		/* Initialize cmd block for entire READ op */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		cmd_bp->b_bcount = total_count;
		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t)
		      &(cmd_bp->b_xmemd), sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->scbuf.status_validity = 0x00;
		cmd_ptr->scbuf.q_tag_msg = diskinfo->q_type;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		if (bp->av_forw != NULL) {
			cmd_ptr->group_type = DK_COALESCED;
			cmd_ptr->scbuf.bp = bp;
			cmd_ptr->bp = NULL;
		}
		else {
			cmd_ptr->group_type = DK_SINGLE;
			cmd_ptr->scbuf.bp = NULL;
			cmd_ptr->bp = bp;
		}
		
		/* Initialize SCSI cmd for operation */
		blk_cnt = total_count / diskinfo->block_size;
		lba = blkno;
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		if ((blk_cnt & 0xffffff00) || (lba & 0xffe00000) ||
			   (diskinfo->dev_type != DK_DISK)) {
			/*
			 * if the command won't fit in a 6 byte
			 * cmd use a 10 byte command instead. 
			 * Or if this is a CDROM or read/write 
			 * optical drive SCSI-2 only says
			 * that read extended is mandatory so 
			 * use the 10 byte
			 * command.
			 */
			scsi->scsi_length = 10;
			scsi->scsi_cmd.scsi_op_code = 
				SCSI_READ_EXTENDED;

			scsi->scsi_cmd.scsi_bytes[0] = 
				((lba >> 24) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = 
				((lba >> 16) & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = 
				((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[4] = 0x00;
			scsi->scsi_cmd.scsi_bytes[5] = 
				((blk_cnt >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[6] = 
				(blk_cnt & 0xff);
			scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		} else {
			scsi->scsi_length = 6;
			scsi->scsi_cmd.scsi_op_code = SCSI_READ;
			scsi->scsi_cmd.lun |= ((lba >> 16) & 0x1f);
			scsi->scsi_cmd.scsi_bytes[0] = 
				((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = blk_cnt;
			scsi->scsi_cmd.scsi_bytes[3] = 0x00;
			
		}


		/*
		 * Clear in progress queue since we just processed it
		 */
		*head  = NULL;
		return(cmd_ptr);
	} else { /* Request is for B_WRITE */
		/* Allocate cmd for WRITE operation */

#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,buildcmd, trc,(char)3, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);
#endif	
#endif
		cmd_ptr = scdisk_cmd_alloc(diskinfo,(uchar)DK_BUF);
		if (cmd_ptr == NULL) {
			return(cmd_ptr);
		}
	        if (good_path == TRUE) {   
			DDHKWD3(HKWD_DD_SCDISKDD, DD_COALESCE, 0, 
				diskinfo->devno, cmd_ptr->bp, 
				&(cmd_ptr->scbuf));
		}
		/* increment transfer count */
		diskinfo->dkstat.dk_xfers++;
		/* determine if need to breakup */
		/* Requests are coalesced */

		cmd_ptr->scbuf.bp = bp;
		total_count = 0x00;
		tmp_bp = bp;
		while (tmp_bp != NULL) {
			total_count += tmp_bp->b_bcount;
			if (tmp_bp->b_options & WRITEV)
				verify = TRUE;
			tmp_bp = tmp_bp->av_forw;
		}
		
		/* Initialize cmd block for entire WRITE op */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = bp->b_flags;
		cmd_bp->b_options = bp->b_options;
		cmd_bp->b_work = 0x00;
		cmd_bp->b_error = 0x00;
		cmd_bp->b_blkno = blkno;
		cmd_bp->b_bcount = total_count;
		cmd_bp->b_un.b_addr = bp->b_un.b_addr;
		bcopy((caddr_t) &(bp->b_xmemd), (caddr_t)
		      &(cmd_bp->b_xmemd), sizeof(struct xmem));
		cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->rw_timeout;
		cmd_ptr->scbuf.status_validity = 0x00;
		cmd_ptr->scbuf.q_tag_msg = diskinfo->q_type;
		cmd_ptr->retry_count = 0;
		cmd_ptr->segment_count = 0x00;
		cmd_ptr->soft_resid = 0x00;
		if (bp->av_forw != NULL) {
			cmd_ptr->group_type = DK_COALESCED;
			cmd_ptr->scbuf.bp = bp;
			cmd_ptr->bp = NULL;
		}
		else {
			cmd_ptr->group_type = DK_SINGLE;
			cmd_ptr->scbuf.bp = NULL;
			cmd_ptr->bp = bp;
		}
		
		blk_cnt = total_count / diskinfo->block_size;
		lba = blkno;
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		
		/* Check for verify enabled */
		if (verify == TRUE) {
			/* Initialize SCSI cmd for Write with verify */
			scsi->scsi_length = 10;
			scsi->scsi_cmd.scsi_op_code = 
				SCSI_WRITE_AND_VERIFY;

			scsi->scsi_cmd.scsi_bytes[0] = 
				((lba >> 24) & 0xff);
			scsi->scsi_cmd.scsi_bytes[1] = 
				((lba >> 16) & 0xff);
			scsi->scsi_cmd.scsi_bytes[2] = 
				((lba >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
			scsi->scsi_cmd.scsi_bytes[4] = 0x00;
			scsi->scsi_cmd.scsi_bytes[5] = 
				((blk_cnt >> 8) & 0xff);
			scsi->scsi_cmd.scsi_bytes[6] = 
				(blk_cnt & 0xff);
			scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		} else {
			/* Normal Write */
			if ((blk_cnt & 0xffffff00) || 
			    (lba & 0xffe00000)) {
				scsi->scsi_length = 10;
				scsi->scsi_cmd.scsi_op_code = 
					SCSI_WRITE_EXTENDED;

				scsi->scsi_cmd.scsi_bytes[0] = 
					((lba >> 24) & 0xff);
				scsi->scsi_cmd.scsi_bytes[1] = 
					((lba >> 16) & 0xff);
				scsi->scsi_cmd.scsi_bytes[2] = 
					((lba >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[3] = 
					(lba & 0xff);
				scsi->scsi_cmd.scsi_bytes[4] = 0x00;
				scsi->scsi_cmd.scsi_bytes[5] = 
					((blk_cnt >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[6] = 
					(blk_cnt & 0xff);
				scsi->scsi_cmd.scsi_bytes[7] = 
					0x00;
			} else {
				scsi->scsi_length = 6;
				scsi->scsi_cmd.scsi_op_code = 
					SCSI_WRITE;
				scsi->scsi_cmd.lun |=
					((lba >> 16) & 0x1f);
				scsi->scsi_cmd.scsi_bytes[0] = 
					((lba >> 8) & 0xff);
				scsi->scsi_cmd.scsi_bytes[1] = 
					(lba & 0xff);
				scsi->scsi_cmd.scsi_bytes[2] = blk_cnt;
				scsi->scsi_cmd.scsi_bytes[3] = 0x00;
			}
		}

		/* Place cmd on device's cmd stack for execution */

	}
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,buildcmd, trc,(char)4, (uint)diskinfo->state, (uint)bp,
	      (uint)bp->b_flags, (uint)bp->b_resid,(uint)bp->b_options);

#endif	
#endif

	/*
	 * Clear in progress queue since we just processed it
	 */
	if (cmd_ptr != NULL)
		*head  = NULL;

	return(cmd_ptr);
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_write_verify					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a write verify command block and	*/
/*		places it on the device's command stack so that the	*/
/*		write verify operation will be attempted before a	*/
/*		block is relocated after a recovered error.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/************************************************************************/

void
scdisk_write_verify(
struct scdisk_diskinfo	*diskinfo,
struct dk_cmd		*prev_cmd_ptr,
ulong			blkno)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp, *prev_cmd_bp;
	struct scsi	*scsi;
	uint		buffer_offset;
	ulong		bufs_largest_blkno;
	struct buf	*prev_bp,*tmp_bp;
	int		tmp_blkno;
	int		rc;








	/* Allocate a SPECIAL type cmd for this operation */

        cmd_ptr = &(diskinfo->writev_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,writeverify, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)prev_cmd_ptr, (uint)cmd_ptr,(uint)cmd_ptr->status);
#endif	
#endif

        if ((cmd_ptr->status != DK_FREE) ||
            (diskinfo->wprotected)) {
                /*
                 *  if command not free or if this is a write protected device, 
                 *  then just retry the failed command
                 */

                return;
        }

	diskinfo->writev_err_cmd = prev_cmd_ptr;
	scdisk_d_q_cmd(prev_cmd_ptr,(uchar) DK_CMD_Q);
	cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_WRITEV;  
        cmd_ptr->subtype = 0x00;     
        cmd_ptr->next = NULL;    
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	prev_cmd_bp = &(prev_cmd_ptr->scbuf.bufstruct);

	/* Make sure blkno is valid */
	if (blkno == 0xffffffff) {
		blkno = prev_cmd_bp->b_blkno;
	}

	/* Calculate offset into user buffer */

	if (prev_cmd_ptr->group_type == DK_COALESCED) {

		/*
		 * If this is a chain of coalesced buf's, then we
		 * must find the buf in which the error(blkno) occurred
		 * in.  This is needed since we will supply a starting address
		 * and a byte offset for the adapter driver to transfer 
		 * data.
		 */
		tmp_bp = prev_cmd_ptr->scbuf.bp;
		/*
		 * NOTE: tmp_bp must not be null here, since this
		 * is a coalesced command.
		 */
		SCDISK_GET_LBA(tmp_blkno,diskinfo,tmp_bp,rc);
		if (rc) {
		  	/*
			 * This should not happen
			 * since scdisk_strategy
			 * should filter these out.
			 */
		  	ASSERT(rc==0) 
		}
		bufs_largest_blkno = tmp_blkno +
			tmp_bp->b_bcount/diskinfo->block_size - 1;

		while ((bufs_largest_blkno < blkno) && (tmp_bp != NULL)) {
			/*
			 * Loop until we find a buf with this blkno
			 */
			prev_bp = tmp_bp;
			tmp_bp = tmp_bp->av_forw;
			if (tmp_bp != NULL) {

				SCDISK_GET_LBA(tmp_blkno,diskinfo,tmp_bp,rc); 
				if (rc) {
					/*
					 * This should not happen
					 * since scdisk_strategy
					 * should filter these out.
					 */
					ASSERT(rc==0) 
				}
				bufs_largest_blkno = tmp_blkno +
				      tmp_bp->b_bcount/diskinfo->block_size - 1;
			}
			
			
		}
		if (tmp_bp == NULL) {
			/*
			 * If we did not find it in the above list, then
			 * assume the last one failed.
			 */
			tmp_bp = prev_bp;
		}

	} else {
		tmp_bp = prev_cmd_bp;
		tmp_blkno = prev_cmd_bp->b_blkno;
	}

	ASSERT(tmp_bp != NULL);

	buffer_offset = (blkno - tmp_blkno) * diskinfo->block_size;
	cmd_bp->b_dev =  prev_cmd_bp->b_dev;
	cmd_bp->b_flags = B_WRITE;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = blkno;
	cmd_bp->b_bcount = diskinfo->block_size;
	cmd_bp->b_un.b_addr = (caddr_t) ((uint)tmp_bp->b_un.b_addr + 
	    buffer_offset);
	bcopy((caddr_t) &(tmp_bp->b_xmemd), (caddr_t) &(cmd_bp->b_xmemd),
	    sizeof(struct xmem));
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = prev_cmd_ptr->scbuf.timeout_value;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	if (diskinfo->cmd_tag_q) 
		cmd_ptr->scbuf.q_tag_msg = SC_HEAD_OF_Q;
	else
		cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;

	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
							
 	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id); 
	/* Initialize SCSI cmd for operation */

     /*
      *                      WRITE AND VERIFY  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (2Eh)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   | DPO   |Reservd|Reservd| BytChk|RelAdr |
      *|-----+---------------------------------------------------------------|
      *| 2   |                                                               |
      *|-----+----                                                      -----|
      *| 3   |                                                               |
      *|-----+----                                                      -----|
      *| 4   |               Logical Block Address                           |
      *|-----+----                                                      -----|
      *| 5   |                                                               |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   | (MSB)                     Transfer Length                     |
      *|-----+----                                                      -----|
      *| 8   |                                                        (LSB)  |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 10;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_AND_VERIFY;

	scsi->scsi_cmd.scsi_bytes[0] = ((blkno >> 24) & 0xff);
	scsi->scsi_cmd.scsi_bytes[1] = ((blkno >> 16) & 0xff);
	scsi->scsi_cmd.scsi_bytes[2] = ((blkno >> 8) & 0xff);
	scsi->scsi_cmd.scsi_bytes[3] = (blkno & 0xff);
	scsi->scsi_cmd.scsi_bytes[4] = 0x00;
	scsi->scsi_cmd.scsi_bytes[5] = 0x00;
	scsi->scsi_cmd.scsi_bytes[6] = 0x01;
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;




	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,writeverify, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)0, (uint)0,(uint)0);
#endif	
#endif

	return;

}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_request_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a request sense command block and	*/
/*		places it on the device's command stack so that the	*/
/*		request sense operation will be performed before the	*/
/*		current command will be retried.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bzero							*/
/*									*/
/************************************************************************/

void
scdisk_request_sense(
struct scdisk_diskinfo	*diskinfo,
struct dk_cmd  *checked_cmd)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;
	uchar		reqsns_in_progress = FALSE;


	/* Allocate a REQUEST type cmd for this operation */




        cmd_ptr = &(diskinfo->reqsns_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reqsns, entry,(char)0, (uint)diskinfo, (uint)checked_cmd,
	      (uint)cmd_ptr->status, (uint)checked_cmd->scbuf.adap_q_status,
	      (uint)diskinfo->state);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is  
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen but if does then assume the
			 * active request sense will clear the device's queue
			 * If not then eventually commands will time out and
			 * the device's queue will be cleared via the SCSI
			 * adapter's erp (i.e. reset the bus etc.)
                         */
                        return;
                } else {
                        /*
                         * make sure command is at the head of dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
			reqsns_in_progress = TRUE;
                }
        }
	if (checked_cmd->scbuf.adap_q_status & SC_DID_NOT_CLEAR_Q) {
		diskinfo->q_status |= SC_DID_NOT_CLEAR_Q;
		checked_cmd->scbuf.adap_q_status &= ~SC_DID_NOT_CLEAR_Q;
	}
	if (!reqsns_in_progress) {
		diskinfo->checked_cmd = checked_cmd;
		scdisk_d_q_cmd(checked_cmd,(uchar) DK_CMD_Q);
	}
        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_REQSNS; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for request sense cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = DK_REQSNS_LEN;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                        REQUEST SENSE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (03h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |         Reserved                      |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Allocation Length                   |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_REQUEST_SENSE;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = DK_REQSNS_LEN;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	/* Clear out the sense data buffer */
	bzero(diskinfo->sense_buf, DK_REQSNS_LEN);

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reqsns, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->q_status,(uint)0);
#endif	
#endif

	return;

}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_start_unit_disable				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a START UNIT cmd block for routines */
/*		that have not spin locked.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on both interrupt level and      */
/*		process level.  It cannot page fault.			*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_start_unit_disable(
struct scdisk_diskinfo	*diskinfo,
uchar			start_stop_flag,
uchar			immed_flag)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	scdisk_start_unit(diskinfo,start_stop_flag,immed_flag);

	/* 
	 * reenable interrupts and unlock
	 */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_start_unit					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a START UNIT cmd block which	*/
/*		starts the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on both interrupt level and      */
/*		process level.  It cannot page fault.			*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_start_unit(
struct scdisk_diskinfo	*diskinfo,
uchar			start_stop_flag,
uchar			immed_flag)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;

	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,startunit,entry, (char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif

        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }




#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,startunit, trc,(char)1, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */

                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for start unit cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout;   /* 1 minute */
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

      /*                   START STOP UNIT Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (1Bh)                |
       *|-----+---------------------------------------------------------------|
       *| 1   | Logical Unit Number   |                  Reserved     | Immed |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved            |  LoEj |  Start|
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Control                             |
       *+=====================================================================+
       */


	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;

	if (immed_flag) {
		/*
		 * If the immediate flag is set
		 * then set it in the CDB.
		 */
		scsi->scsi_cmd.lun |= 1;
	}


	if (start_stop_flag == DK_START) {
		scsi->scsi_cmd.scsi_bytes[2] = 0x01;

                scsi->scsi_cmd.scsi_bytes[3] = 0x00;
                /* Power Management mode change */
                if (diskinfo->pmh.handle.mode != PM_DEVICE_FULL_ON) {
                        diskinfo->pmh.handle.mode = PM_DEVICE_ENABLE;
                }


	} else {
                scsi->scsi_cmd.scsi_bytes[2] = 0x00;
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	}





	cmd_ptr->subtype = start_stop_flag;

	/* Place cmd on device's operation stack */

	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	/* Begin execution of this operation */

	scdisk_start(diskinfo);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_test_unit_ready_disable				*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a TEST UNIT READY cmd block for	*/
/*		routines not disabled and spin locked.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_test_unit_ready_disable(
struct scdisk_diskinfo	*diskinfo)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	scdisk_test_unit_ready(diskinfo);
	/* 
	 * reenable interrupts and unlock
	 */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_test_unit_ready					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a TEST UNIT READY cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_test_unit_ready(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;


	/*
	 * The indicated device has been determined to have been
	 * Reset or Power Cycled at this point. We must make sure
	 * that any cmds on the device's stack are valid.
	 * ie. Remove any RESET or REQSNS type cmds that are on
	 * the stack for this device and free the cmd blocks.
	 */

	diskinfo->reset_count++;
	cmd_ptr = diskinfo->dk_cmd_q_head;

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,testunit,entry, (char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif


	while ((cmd_ptr != NULL) && (cmd_ptr->type & (DK_RESET | DK_REQSNS))) {
		scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
		if ((diskinfo->starting_close) &&
		    ((cmd_ptr->subtype == DK_RELEASE) ||
		     (cmd_ptr->subtype == DK_ALLOW))) {
			/*
			 * If we started closing the device and
			 * there is a reset command that is a release
			 * or allow media removal command, then we
			 * know that close has already allowed
			 * all pending I/O to complete.  The only way we could
			 * get here is if some kind of error (i.e. Unit 
			 * Attention) occurred on either the release or allow
			 * media removal command.  However we do not want to
			 * issue the reset cycle, because this could re-issue
			 * the reserve or prevent media removal command. It
			 * is safest instead to assume that the unit attention
			 * cleared this and wakeup the close thread. Thus
			 * letting close complete.
			 */
			diskinfo->disk_intrpt = 0;
			e_wakeup((int *)&(diskinfo->open_event));
			scdisk_free_cmd(cmd_ptr);	
			return;
		}
		scdisk_free_cmd(cmd_ptr);
		cmd_ptr = diskinfo->dk_cmd_q_head;
	}

	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,testunit, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif



        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for test unit ready cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = (void(*)())scdisk_iodone;
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

      /*                  TEST UNIT READY Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (00h)                |
       *|-----+---------------------------------------------------------------|
       *| 1   | Logical Unit Number   |                  Reserved             |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Control                             |
       *+=====================================================================+
       */


	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	cmd_ptr->subtype = DK_TUR;

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);


	if (diskinfo->pmh.handle.mode != PM_DEVICE_FULL_ON) {
		diskinfo->pmh.handle.mode = PM_DEVICE_ENABLE;
	}	

	/* Begin execution of this operation */

	scdisk_start(diskinfo);

	return;
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_reserve						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a RESERVE UNIT cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_reserve(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,reserve, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for reserve cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                RESERVE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (16h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |3rdPty | 3rd Party Device Id   |Extent |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved  Identification            |
      *|-----+---------------------------------------------------------------|
      *| 3   | (MSB)         Extent List Length                              |
      *|-----+---                                                 -----------|
      *| 4   |                                                         (LSB) |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_RESERVE_UNIT;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	cmd_ptr->subtype = DK_RESERVE;

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	return;
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_prevent_allow_disable           			*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a SCSI prevent allow media removal  */
/*		command for routines that are not disabled and spin 	*/
/*		locked.							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_prevent_allow_disable(
struct scdisk_diskinfo	*diskinfo,
uchar			prevent_allow_flag)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	scdisk_prevent_allow(diskinfo,prevent_allow_flag);
	/* 
	 * reenable interrupts and unlock
	 */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_prevent_allow             			*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a SCSI prevent allow media removal  */
/*		command.  If the prevent_allow_flag is set then it will */
/*		build a prevent media removal command.  Otherwise it    */
/*		will build an allow media removal command		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_prevent_allow(
struct scdisk_diskinfo	*diskinfo,
uchar			prevent_allow_flag)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,prevent,trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);

#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for prevent allow medium removal cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                 PREVENT ALLOW MEDIUM REMOVAL Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (1Eh)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |               Reserved                |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                    |Prevent|
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_PREVENT_ALLOW_REMOVAL;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	if (prevent_allow_flag == DK_PREVENT) {
		cmd_ptr->subtype = DK_PREVENT;	
		scsi->scsi_cmd.scsi_bytes[2] = 0x01;
	}
	else  {
		cmd_ptr->subtype = DK_ALLOW;
		scsi->scsi_cmd.scsi_bytes[2] = 0x00;
	}
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	return;
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_mode_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a MODE SENSE cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_mode_sense(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,modesense, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for mode sense cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 255; /* setup for max mode data transfer */
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                            MODE SENSE(6)  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (1Ah)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |Resvd  | DBD   |       Reserved        | 
      *|-----+---------------------------------------------------------------|
      *| 2   |      PC       |         Page Code                             |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Allocation Length                   |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_MODE_SENSE;

        /*
         * Set Page Code byte to request all supported pages....set Page
         * Control bits appropriately depending if we want changeable or
         * current mode data
         */
        scsi->scsi_cmd.scsi_bytes[0] = (0x3F |
                ((diskinfo->m_sense_status == DK_SENSE_CHANGEABLE) ?
                DK_SENSE_CHANGEABLE : DK_SENSE_CURRENT));
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = (uchar)cmd_bp->b_bcount;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	cmd_ptr->subtype = DK_MSENSE;

	/*
	 * Clear out the sense data buffer
	 */
	bzero(diskinfo->sense_buf, 256);

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
}


/************************************************************************/
/*									*/
/* NAME: scdisk_format_mode_data					*/
/*									*/
/* FUNCTION:  This function parses a buffer of mode data into a standard*/ 
/*            control structure to prepare the mode data for comparison */   
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*									*/
/*      This routine is  called at the interrupt level and 		*/
/*	cannot page fault.						*/
/*									*/
/* (DATA STRUCTURES:)  scdisk_diskinfo     Disk's information		*/
/*									*/
/* INPUTS:								*/
/*                    mode_data  - Pointer to mode data			*/
/*                    mf         - Pointer to mode format structure	*/
/*									*/
/* (RECOVERY OPERATION:)  None.						*/
/*									*/
/* RETURNS:	Void.							*/
/************************************************************************/

void scdisk_format_mode_data(
char *mode_data,
struct scdisk_mode_format *mf,
int	sense_length,
char    over_ride,
struct scdisk_diskinfo *diskinfo)
{
	char	i=0,page=0;
	short	bd_length,offset,p_length,length;

	for (i=0;i<DK_MAX_MODE_PAGES;i++)	
		/* 
		 * initialize all page indices to -1 
		 */
		mf->page_index[i] = (signed char)-1;

	mf->sense_length = sense_length;
	/* 
	 * get length of block descriptor 
	 */
	bd_length = mode_data[DK_BLK_DESC_LN_INDEX];
	if (bd_length == 8) {
		/*
		 * if == 8, we understand this
		 */
		mf->block_length = ((mode_data[9] << 16) | (mode_data[10]<< 8) |
				  mode_data[11]) & 0x00FFFFFF; 
		if (over_ride == TRUE) {
			/*
			 * The mode data string from ODM contains
			 * the valid CD-ROM Data Mode 1 density code.
			 */
			diskinfo->cd_mode1_code = mode_data[4];
		}
	} else {
		/*
		 * else we don't understand this
		 */
		mf->block_length = 0;	
	}
	/* 
	 * compute offset to first page (i.e. offset to block descriptor +
	 * length of block descriptor area )
	 */
	offset = DK_BLK_DESC_LN_INDEX+bd_length+1;   
	while (offset < mf->sense_length) {
		/*
		 * For the remainder of the sense data...
		 * Store the index into the data buffer for each page
		 */
		page = (mode_data[offset] & DK_MAX_MODE_PAGES);	
		mf->page_index[page] = offset;
		p_length = mode_data[++offset];
		if ((over_ride == TRUE) &&
		    ((page == 0x2) || (page == 0xa) || (page == 0xe))) {
			if ((page == 0x2) && (p_length >= 2) &&
			    (diskinfo->buffer_ratio > (unsigned)0)) {
				/*
				 * For page 2 if buffer ratio is nonzero then
				 * readjust it to match config methods
				 */
				mode_data[offset+1] = diskinfo->buffer_ratio;
				mode_data[offset+2] = diskinfo->buffer_ratio;
			}
			else if ((page == 0xa) && (p_length >= 2) &&
			    (diskinfo->cmd_tag_q)) {
				/*
				 * For page a if this device supports command
				 * tag queuing lets set page A and byte 3
				 * correctly for queuing including the Qer bit
				 */
				if (diskinfo->q_err_value) {
					/*
					 * Turn on the Qerr bit. Set
					 * Queue Algorithm modifier = 1
					 * and turn off DQue bit.
					 */
					mode_data[offset+2] = 0x12;
				}
				else {
					/*
					 * Do not turn on the Qerr bit. Set
					 * Queue Algorithm modifier = 1
					 * and turn off DQue bit.
					 */
					mode_data[offset+2] = 0x10;
				}

			}
			else if ((page == 0xe) && 
				 (p_length >= DK_VOL_START_PG_E)) {
			  	/*
				 * Save off config data base settings
				 * for page 0xE's volume control and
				 * output port map.
				 */
			  	if (p_length > DK_MAX_SIZE_PG_E) {
					length = DK_MAX_SIZE_PG_E;
				}
				else {
				  	length = p_length;
				}
				for (i=DK_VOL_START_PG_E;i<=length;i++) {
				   diskinfo->mode_page_e[i-DK_VOL_START_PG_E] = 
					   mode_data[offset + i];
			        }
					  

			}
		}
		for (i=0;i<=p_length;i++) {
			/*
			 * step over the data bytes for this page
			 */
			++offset;
		}
	}
	return;
}
		

/************************************************************************/
/*									*/
/* NAME: scdisk_mode_data_compare					*/
/*									*/
/* FUNCTION:  Parses a devices changable mode parameters, current mode 	*/
/* 	      parameters, and the using systems desired mode parameters */
/* 	      to determine if a mode select is necessary.		*/
/*									*/
/* NOTES:    This routine uses 4 sets of mode data.  For each of 	*/
/*           these, the driver uses an scdisk_mode_format structure.	*/
/*	     This structure contains an array (called page_index), which*/
/*	     gives the offset into a buffer of each mode data page.  A	*/
/*	     value of -1 in this array indicates the mode data page	*/
/*	     is not in the buffer.  The routine 			*/
/*	     scdisk_format_mode_data is responsible for setting up the	*/
/*	     scdisk_mode_format structure.  The 4 types of mode		*/
/*	     data used by this driver and their corresponding 		*/
/*	     structures are:						*/
/*									*/
/*   		       - Desired Mode Data (this is sent to driver	*/
/*			 via the config method.  The config method	*/
/*			 uses the ODM mode_data attribute to set this.	*/
/*			 The diskinfo->dd field is the corresponding	*/
/*			 scdisk_mode_format structure.  The buffer	*/
/*			 used for this mode data is diskinfo->mode_buf.	*/
/*			  						*/
/*									*/
/*		       - Default Mode Data (this is sent to driver	*/
/*			 via the config method.  The config method	*/
/*			 uses the ODM mode_default attribute to set 	*/
/*			 this. The diskinfo->df field is the            */
/*			 corresponding scdisk_mode_format structure.   	*/
/*			 The buffer used for this mode data is 	  	*/
/*		 	 diskinfo->df_data.  				*/
/*									*/
/*		       - Changeable Mode Data. The driver fetches 	*/
/*		         this information via a mode sense which	*/
/*			 asks for just changeable mode data.		*/
/*			 The diskinfo->ch field is the corresponding	*/
/*			 scdisk_mode_format structure. The buffer	*/
/*			 used for this mode data is diskinfo->ch_data.	*/
/*									*/
/*		       - Current Mode Data. The driver fetches 		*/
/*		         this information via a mode sense which	*/
/*			 asks for just current mode data.		*/
/*			 The diskinfo->cd field is the corresponding	*/
/*			 scdisk_mode_format structure. The buffer	*/
/*			 used for this mode data is diskinfo->sense_buf	*/
/*			 (since the current data was the last data	*/
/*			 received).					*/
/*									*/
/*									*/ 
/*  ASSUMPTION:	 This code assumes that the the page length of the	*/
/*		 changeable mode data is less then or equal to the	*/
/* 		 page length of the current mode data for each page.  	*/
/*		 If this assumption is violated then the mode data	*/
/*		 for the mode select will be wrong.			*/
/*									*/
/*									*/
/* EXECUTION ENVIRONMENT: This routine is called at the interrupt level */
/*            and can not page fault.					*/
/*									*/
/* DATA STRUCTURES: scdisk_diskinfo	- Disk information structure	*/
/*		    scdisk_mode_format	- Mode data control information	*/
/*									*/
/* (RECOVERY OPERATION:)   None.					*/
/*									*/
/* RETURNS:   0 - No mode select is necessary (or wouldn't do any good)	*/
/*	      1 - Perform mode select with altered current data		*/
/*									*/
/************************************************************************/

int scdisk_mode_data_compare(
struct scdisk_diskinfo *diskinfo)
{

	int 	select = 0;
	char	page,i,ch_index,cd_index,dd_index,df_index,made_change;
	uchar	diff,changeable,def;
	short	ch_length,dd_length,df_length,cd_length;
	char    queuing_enabled = FALSE;



	/*
	 *                      Mode Parameter Header(6)
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+=======================================================|
	 * | 0   |                Mode Data Length                       |
	 * |-----+-------------------------------------------------------|
	 * | 1   |                Medium Type                            |
	 * |-----+-------------------------------------------------------|
	 * | 2   |                Device-Specific Parameter              |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                Block Descriptor Length                |
	 * +=============================================================+
	 *
	 *
	 *                    Disk Device Specific Parameter
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |=====+======+================================================|
	 * |     |   WP |     Reserved    | DPOFUA |        Reserved     |
	 * +=============================================================+
	 * 
	 *                    CD-ROM Device Specific Parameter
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |=====+====================+=======+===================+======|
	 * |     |  Reserved          | DPOFUA|    Reserved       | EBC  |
	 * +=============================================================+
	 * 
	 *                Optical Memory Device Specific Parameter
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |=====+======+=============+======+====================+======|
	 * |     |   WP |   Reserved  |DPOFUA|     Reserved       | EBC  |
	 * +=============================================================+
	 * 
	 *
	 *
	 *			Mode Parameter Block Descriptor
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+=======================================================|
	 * | 0   |                  Density Code                         |
	 * |-----+-------------------------------------------------------|
	 * | 1   | (MSB)                                                 |
	 * |-----+---                                                 ---|
	 * | 2   |                  Number of Blocks                     |
	 * |-----+---                                                 ---|
	 * | 3   |                                                 (LSB) |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                  Reserved                             |
	 * |-----+-------------------------------------------------------|
	 * | 5   | (MSB)                                                 |
	 * |-----+---                                                 ---|
	 * | 6   |                  Block Length                         |
	 * |-----+---                                                 ---|
	 * | 7   |                                                 (LSB) |
	 * +=============================================================+
	 *
	 */

	/*
	 * First check block length, if non-zero
	 */
	if (!((diskinfo->dd.block_length == 0) || 
	      (diskinfo->cd.block_length == 0))) {
		/*
		 * if both the desired data and the current data contain
		 * a defined block length, then compare them.
		 */
		if (diskinfo->block_size != diskinfo->cd.block_length) {
			/*
			 * if they aren't the same, then set flag to do select
			 * and alter the current data block length to reflect
			 * the desired block length
			 */
			select = 1;
			diskinfo->sense_buf[9] = 
				(diskinfo->block_size >> 16) & 0x00ff;
			diskinfo->sense_buf[10] = 
				(diskinfo->block_size >> 8) & 0x00ff;
			diskinfo->sense_buf[11] = 
				diskinfo->block_size & 0x00ff;
		}
                if ((diskinfo->current_cd_code != diskinfo->sense_buf[4]) &&
		    (diskinfo->valid_cd_modes)) {
                        /*
                         * If the current sense data's density code 
                         * does not match what we want and we have 
			 * valid CD-ROM data modes from the configuration
			 * method then, change it and do a mode select.
                         */
                        select = 1;
                        diskinfo->sense_buf[4] = diskinfo->current_cd_code; 
		}
	}
        if (diskinfo->sense_buf[2] & 0x80) {
                /*
                 * Device is write protected
                 */
                diskinfo->wprotected = TRUE;
        }
	diskinfo->sense_buf[0] = 0;    /* set reserved byte 0 of header to 0 */
	diskinfo->sense_buf[2] &= 0x6f;/* clear DPOFUA and WP bits of header */


	/*
	 * Now check each changeable page
	 */

	/*
	 *                       Mode Page Format
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+======+======+=========================================|
	 * | 0   |  PS  |Reserv|       Page Code                         |
	 * |-----+-------------------------------------------------------|
	 * | 1   |                     Page Length                       |
	 * |-----+-------------------------------------------------------|
	 * | 2   |                                                       |
	 * |-----+--                   Mode Parameters                ---|
	 * | n   |                                                       |
	 * +=============================================================+
	 */

	for (page=0;page<DK_MAX_MODE_PAGES;page++) {
	    made_change = FALSE; /* flag to tell us whether we changed page*/
	    if (diskinfo->ch.page_index[page] != (signed char) -1) {
		/*
		 * if this page is supported
		 */

		/*
		 * get changeable data for this page
		 */

		ch_index = diskinfo->ch.page_index[page];
		ch_length = diskinfo->ch_data[ch_index+1];

		/*
		 * get current data for this page
		 */

		cd_index = diskinfo->cd.page_index[page];
		if (cd_index == (signed char) -1) {
			/*
			 * If the current data doesn't contain
			 * this page then continue to the next
			 * page.
			 */
			continue;
		}
		cd_length = diskinfo->sense_buf[cd_index+1];

		/*
		 * The following assertion should never happen,
		 * since it doesn't make sense for the page length
		 * of the changeable mode data to be longer then
		 * the page length of the same page in the current
		 * values mode data.
		 */

		ASSERT(cd_length>=ch_length);


		/*
		 * Let's validate whether the drive
		 * supports mode page A.  If not
		 * then it can't really support command
		 * tag queuing, since page A control queuing
		 * parameters.
		 */
                         
		/* 		
		 *                  Control mode page (0xA)
		 *+=====-=====-=====-=====-=====-=====-=====-=====-=====+
		 * |  Bit|  7  |   6 |   5 |   4 |   3 |   2 |   1 |   0 |
		 * |Byte |     |     |     |     |     |     |     |     |
		 * |=====+=====+=====+===================================|
		 * | 0   |  PS |Reser|      Page code (0Ah)              |
		 * |-----+-----------------------------------------------|
		 * | 1   |                  Page length (06h)            |
		 * |-----+-----------------------------------------------|
		 * | 2   |                  Reserved               |RLEC |
		 * |-----+-----------------------------------------+-----|
		 * | 3   |Queue algorithm modifier| Reserved | QErr| DQue|
		 * |-----+-----------------------------------+-----+-----|
		 * | 4   | EECA|            Reserved   |RAENP|UAAEN|EAENP|
		 * |-----+-----------------------------------------------|
		 * | 5   |                  Reserved                     |
		 * |-----+-----------------------------------------------|
		 * | 6   | (MSB)                                         |
		 * |-----+---     Ready AEN holdoff period            ---|
		 * | 7   |                                         (LSB) |
		 * +=====================================================+
		 */

		if ((page == 0xa) && (cd_length > 1)) {
			/* 
			 * If this device supports
			 * byte 3 (i.e cd_lenght must be 
			 * at least 2) of mode page A, 
			 * then we will consider it
			 * a valid device for potential
			 * command tag queuing.
			 */

			if (((diskinfo->sense_buf[cd_index + 3] & 0x1) == 0) &&
			   (diskinfo->cmd_tag_q)) {

				/* 
				 * If this device currently has command
				 * tag queuing turned on (i.e. the
				 * DQue bit is zero in the current mode data
				 * (cd)) and our config has told us to queue 
				 * to this device (cmd_tag_q is True), then
				 * don't disable command tag queuing in
				 * the driver. We can make this assumption, 
				 * because the device currently has queuing
				 * turned on. So one of two things must be 
				 * true:
				 *
				 * 1. The device currently has queuing
				 *    turned on. So if we not issuing
				 *    a mode select to this bit, we
				 *    can't turn it off.
				 * 
				 * 2. If we are issuing a mode select to
				 *    this bit, then the fact that cmd_tag_q
				 *    is true indicates that the routine
				 *    scdisk_format_mode_data will have
				 *    built mode select data to turn this
				 *    bit on.
				 *
				 * NOTE: the code below will check
				 * for the scenario where the device
				 * currently has queuing turned off,
				 * and we are going to turn it on.
				 */
				queuing_enabled = TRUE;
			}
		}
		/* 
		 * mask off reserved bits 6 and 7 of the page code
		 */
		diskinfo->sense_buf[cd_index] &= DK_MAX_MODE_PAGES; 
		if (diskinfo->dd.page_index[page] != (signed char) -1) {
			/*
			 * if this page is in our desired data base pages, 
			 * then we can and may potentially change this page 
			 * of data.
			 */
			dd_index = diskinfo->dd.page_index[page];
			dd_length = diskinfo->mode_buf[dd_index+1];

			if (dd_length < ch_length) {
				/*
				 * if our data base has fewer bytes for this
				 * page than the device supports, only look at
				 * those bytes.
				 */
				ch_length = dd_length;
			}
			for (i=2;i < ch_length+2; i++) {
				/*
				 * for each changeable byte of this page
				 */
                                def = 0;
                                if (diskinfo->df.page_index[page] !=
                                    (signed char) -1) {
                                    /*
                                     * if there is default data specified
                                     * for this page
                                     */
                                    df_index = diskinfo->df.page_index[page];
                                    df_length = diskinfo->df_data[df_index +1];
                                    if (i < (df_length+2))
                                          /*
                                           * if there is default data specified
                                           * for this byte
                                           */
                                          def = diskinfo->df_data[df_index+i];
                                }
				if ((page == 0xa) && (i == 3)) {
					/*
					 * Device has page A and byte 3
					 * Now lets determine if it supports
					 * the Qerr bit
					 */
					
					scdisk_q_mode(diskinfo,i,ch_index,
						      dd_index);

					/*
					 * If we're going to turn queuing
					 * on (i.e the device let's me
					 * change the DQue bit and we
					 * plan to turn it off (i.e. turn
					 * command tag queuing on), then
					 * don't disable the driver from
					 * command tag queuing.
					 * NOTE: a previous check covers
					 * the case where the drive 
					 * always has the DQue bit off and
					 * it's not changeable.
					 */
					if (((diskinfo->ch_data[ch_index + i] & 0x1) == 1) &&
					  ((diskinfo->mode_buf[dd_index + i] & 0x1) == 0)) {
						/*
						 * The DQue bit is changeable
						 * and our mode select data
						 * (mode_buf) will turn it
						 * on.
						 */
						queuing_enabled = TRUE;
					}
				}

				/*
				 * diff = a bitmask of bits we would like to
				 * change. (i.e. current_data XOR desired_data)
				 */
				diff = diskinfo->sense_buf[cd_index + i] ^ 
				       diskinfo->mode_buf[dd_index + i];
				/*
				 * changeable = a bitmask of bits we would like
				 * to change and can change. (i.e. difference
				 * between current and desired ANDed with 
				 * changeable bits)
				 */
				changeable = 
				    diff & diskinfo->ch_data[ch_index + i];
                                /*
                                 * Now, make sure we don't clobber any bits
                                 * that the data base has specified that we
                                 * should use device default parameters
                                 */
                                changeable &= ~def;
				
				if (changeable) {
				    /*
				     * means we would like to make a change
				     */
				    /*
				     * Clear and Set desired bits
				     * (i.e. current data XOR changeable)
				     */
				    diskinfo->sense_buf[cd_index +i] ^=
					changeable;
				    select = 1;
				    made_change = TRUE;
				} /* if changeable */
			} /* for each byte of this page */
		} /* if this page in data base */	
            	if (!made_change) {
			/*
			 * we didn't change anything in this
			 * page, so shift it out of the current and
			 * changeable data
			 */
			ch_index = diskinfo->ch.page_index[page];
			ch_length = diskinfo->ch_data[ch_index+1];

			/*
			 * Shift changeable mode data page out.
			 */
			for (i=ch_index;i<diskinfo->ch.sense_length;i++)
			 	diskinfo->ch_data[i] = 
			 		diskinfo->ch_data[i+ch_length+2];

			cd_index = diskinfo->cd.page_index[page];
			cd_length = diskinfo->sense_buf[cd_index+1];

			/*
			 * Shift current mode data page out.
			 */
			for (i=cd_index;i< diskinfo->cd.sense_length;i++)
			 	diskinfo->sense_buf[i] = 
			 		diskinfo->sense_buf[i+cd_length+2];

			/*
			 * Rebuild page index for modified changeable mode
			 * data (i.e. with mode page shifted out).
			 */
			scdisk_format_mode_data((char *) diskinfo->ch_data,
			    (struct scdisk_mode_format *) &diskinfo->ch,
				       (int)(diskinfo->ch.sense_length - 
					     (ch_length + 2)),
					      (char) FALSE,
					      (struct scdisk_diskinfo *)NULL);

			/*
			 * Rebuild page index for modified current mode
			 * data (i.e. with mode page shifted out).
			 */
			scdisk_format_mode_data((char *) diskinfo->sense_buf, 
			    (struct scdisk_mode_format *)&diskinfo->cd, 
			    (int)(diskinfo->cd.sense_length - (cd_length + 2)),
			    (char) FALSE,
			    (struct scdisk_diskinfo *) NULL);
		}
	    } /* if this page supported */	
	} /* for each possible page */
	if (queuing_enabled == FALSE) {
		/*
		 * If device does not support Q page, then determine
		 * we will not queue to this device.
		 */
		diskinfo->cmd_tag_q = FALSE;
		diskinfo->q_type = SC_NO_Q;
		diskinfo->clr_q_on_error = TRUE;

	}
	return(select);
}
/************************************************************************/
/*									*/
/* NAME: scdisk_q_mode							*/
/*									*/
/* FUNCTION:  Determines how to set Qerr bit if it exist and		*/
/*	      How to set the clr_q_on_error flag			*/
/*									*/
/* EXECUTION ENVIRONMENT: This routine is called at the interrupt level */
/*            and can not page fault.					*/
/*									*/
/* DATA STRUCTURES: scdisk_diskinfo	- Disk information structure	*/
/*		    scdisk_mode_format	- Mode data control information	*/
/*									*/
/* (RECOVERY OPERATION:)   None.					*/
/*									*/
/* RETURNS:  Void							*/
/*									*/
/************************************************************************/

void scdisk_q_mode(
struct scdisk_diskinfo *diskinfo,
		   char  i,
		   char ch_index,
		   char dd_index)
{
	
	
	if (diskinfo->cmd_tag_q) {
		/*
		 * This disk supports 
		 * command tag queuing. 
		 */
		if  ((diskinfo->ch_data[ch_index +i]) & 0x2) {
			/*
			 * If the Qerr bit is supported 
			 * on page A on byte 3 
			 */

			if (diskinfo->q_err_value) {
				/*
				 * if q_err_value is
				 * true then set the
				 * clr_q_on_error
				 */
				diskinfo->clr_q_on_error = TRUE;
			}
			else {
				/* 
				 * Otherwise
				 * keep Qerr bit off
				 */
				diskinfo->clr_q_on_error = FALSE;
			}
		}
	}
	else {

		/*
		 * if not command tag queuing then assume 
		 * clears queue on error, since only 
		 * one command is queued at the 
		 * device at a time
		 */
		diskinfo->clr_q_on_error = TRUE;
	}


				
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_mode_select					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a MODE SELECT cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_mode_select(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;



	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,modeselect, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */
                        return;
                } else {
                        /*

                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for mode select cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_WRITE | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = diskinfo->cd.sense_length;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->sense_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                            MODE SELECT(6)  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (15h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |  PF   |    Reserved            |  SP  | 
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Paramenter List Length              |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_MODE_SELECT;
	/*
	 * setup up lun and page format bit = complies with page format
	 */
	scsi->scsi_cmd.lun |= DK_COMPLIES_PF;
	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = (uchar)diskinfo->cd.sense_length;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	cmd_ptr->subtype = DK_SELECT;

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_read_cap						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a READ CAPACITY cmd block which	*/
/*		continues the reset cycle for the specified device.	*/
/*		The cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_read_cap(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;






	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,readcap, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */

                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;


	/* Initialize cmd block for read capacity cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x08;
	cmd_bp->b_un.b_addr = (caddr_t) &(diskinfo->capacity);
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */
     /*
      *                READ CAPACITY  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (25h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |   Reserved                    |RelAdr | 
      *|-----+---------------------------------------------------------------|
      *| 2   | (MSB)                                                         |
      *|-----+-----                                                   -------|
      *| 3   |                 Logical Block Address                         |
      *|-----+-----                                                   -------|
      *| 4   |                                                               |
      *|-----+-----                                                   -------|
      *| 5   |                                                        (LSB)  |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 8   |                           Reserved                     | PMI  |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 10;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_READ_CAPACITY;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	scsi->scsi_cmd.scsi_bytes[4] = 0x00;
	scsi->scsi_cmd.scsi_bytes[5] = 0x00;
	scsi->scsi_cmd.scsi_bytes[6] = 0x00;
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;

	cmd_ptr->subtype = DK_READCAP;

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);

	return;
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_read_disc_info                                   */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine builds the Toshiba unique READ Disc        */
/*              information SCSI command.  The cmd block is             */
/*              constructed, placed on the top                          */
/*              of the device's cmd queue and scdisk_start is called    */
/*              to initiate execution of the cmd.                       */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine is called on an interrupt level and        */
/*              process level.  It cannot page fault.                   */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*              diskinfo        Disk device specific information        */
/*                                                                      */
/*      INPUTS:                                                         */
/*              diskinfo        Disk device specific information        */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns no value.                          */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*              None.                                                   */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void
scdisk_read_disc_info(
struct scdisk_diskinfo  *diskinfo)
{
        struct dk_cmd   *cmd_ptr;
        struct buf      *cmd_bp;
        struct scsi     *scsi;




        /* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,rdiscinfo, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
              (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
              (uint)cmd_ptr->status);
#endif  
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */

                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
        cmd_ptr->aborted = FALSE;

        /* Initialize cmd block for read disc info cmd */
        cmd_bp = &(cmd_ptr->scbuf.bufstruct);
        cmd_bp->b_dev = diskinfo->adapter_devno;
        cmd_bp->b_flags = (B_READ | B_BUSY);
        cmd_bp->b_options = 0x00;
        cmd_bp->b_work = 0x00;
        cmd_bp->b_error = 0x00;
        cmd_bp->b_blkno = 0x00;
        cmd_bp->b_bcount = 0x4;
        cmd_bp->b_un.b_addr = (caddr_t)diskinfo->disc_info;
        cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
        cmd_bp->b_iodone = ((void(*)()) scdisk_iodone);
        cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
        cmd_ptr->scbuf.bp = NULL;
        cmd_ptr->scbuf.status_validity = 0x00;
        cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
        cmd_ptr->scbuf.adap_q_status = 0;
        cmd_ptr->scbuf.flags = 0;
        cmd_ptr->retry_count = 0;
        cmd_ptr->segment_count = 0x00;
        cmd_ptr->soft_resid = 0x00;
        cmd_ptr->bp = NULL;

        /* Initialize SCSI cmd for operation */
        scsi = &(cmd_ptr->scbuf.scsi_command);
        scsi->scsi_length = 10;
        scsi->scsi_id = diskinfo->scsi_id;
        scsi->flags = (ushort) diskinfo->async_flag;
        scsi->scsi_cmd.scsi_op_code = DK_SCSI_MS_DISCINFO;

     /*
      *                 READ DISC INFORMATION (Vendor Unique) Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (C7h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   |   0   |   0   |   0   |         Reserved      |      Type     |
      *|-----+---------------------------------------------------------------|
      *| 2   |                          Track Number                         |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 6   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 7   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 8   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Control                             |
      *+=====================================================================+
      */

        /*
         * Issue command with type = 11B (i.e. see if CD-ROM XA).
         */
        scsi->scsi_cmd.lun = 0x3;
        scsi->scsi_cmd.scsi_bytes[0] = 0x00;
        scsi->scsi_cmd.scsi_bytes[1] = 0x00;
        scsi->scsi_cmd.scsi_bytes[2] = 0x00;
        scsi->scsi_cmd.scsi_bytes[3] = 0x00;
        scsi->scsi_cmd.scsi_bytes[4] = 0x00;
        scsi->scsi_cmd.scsi_bytes[5] = 0x00;
        scsi->scsi_cmd.scsi_bytes[6] = 0x00;
        scsi->scsi_cmd.scsi_bytes[7] = 0x00;
        scsi->scsi_cmd.scsi_bytes[8] = 0x00;
        scsi->scsi_cmd.scsi_bytes[9] = 0x00;


        cmd_ptr->subtype = DK_READ_INFO;

        /* Place cmd on device's operation stack */
        scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);


        /* Begin execution of this operation */
        scdisk_start(diskinfo);

        return;
}



/************************************************************************/
/*									*/
/*	NAME:	scdisk_release_disable					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a RELEASE UNIT cmd block for	*/
/*		routines that are not disabled or spin locked.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level and        */
/*		process level.  It cannot page fault.		        */
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_release_disable(
struct scdisk_diskinfo	*diskinfo)
{
	int			old_pri;


	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	scdisk_release(diskinfo);
	/* 
	 * reenable interrupts and unlock
	 */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_release						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a RELEASE UNIT cmd block which	*/
/*		starts the reset cycle for the specified device.	*/
/*		cmd block is constructed, placed on the top		*/
/*		of the device's cmd stack and scdisk_start is called	*/
/*		to initiate execution of the cmd.			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level and        */
/*		process level.  It cannot page fault.		        */
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_release(
struct scdisk_diskinfo	*diskinfo)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;




	/* Allocate a RESET type cmd for this operation */

        cmd_ptr = &(diskinfo->reset_cmd);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,release, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)cmd_ptr,
	      (uint)cmd_ptr->status);
#endif	
#endif
        if (cmd_ptr->status != DK_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd_ptr->status & DK_ACTIVE) {
                        /*
                         * Shouldn't happen
                         */

                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        scdisk_d_q_cmd(cmd_ptr,(uchar) DK_CMD_Q);
                }
        }

        cmd_ptr->status |= DK_IN_USE;
        cmd_ptr->type = DK_RESET; 
        cmd_ptr->subtype = 0x00;  
        cmd_ptr->next = NULL;
        cmd_ptr->prev = NULL;    
	cmd_ptr->aborted = FALSE;

	/* Initialize cmd block for release cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = 0x00;
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = 0x00;
	cmd_bp->b_un.b_addr = (caddr_t)0x00;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)()) scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;
	SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);

	/* Initialize SCSI cmd for operation */

     /*
      *                RELEASE  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (17h)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |3rdPty | 3rd Party Device Id   |Reservd|
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_RELEASE_UNIT;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	cmd_ptr->subtype = DK_RELEASE;

	/* Place cmd on device's operation stack */
	scdisk_q_cmd(cmd_ptr,(char) DK_STACK,(uchar) DK_CMD_Q);


	/* Begin execution of this operation */
	scdisk_start(diskinfo);

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_build_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds an error record for a failed	*/
/*		operation to be placed in the system error log.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*		type - Type of error being logged.			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		bcopy							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_build_error(
struct dk_cmd	*cmd_ptr,
int		type,
int		priority,
int		valid_sense)
{
	struct scdisk_diskinfo	*diskinfo;
	struct sc_buf		*scbuf;
	struct sc_error_log_df	*erec;

	diskinfo = cmd_ptr->diskinfo;
	if ((cmd_ptr->error_type == 0) ||
	    (priority == DK_OVERLAY_ERROR)) {

		/* Store type for use by logger */
		cmd_ptr->error_type = type;

		/* Setup pointers for faster access */
		scbuf = &(cmd_ptr->scbuf);
		erec = &(diskinfo->error_rec);

		bcopy(&(scbuf->scsi_command), &(erec->scsi_command), 
		    sizeof(struct scsi));
		erec->status_validity = scbuf->status_validity;
		erec->scsi_status = scbuf->scsi_status;
		erec->general_card_status = scbuf->general_card_status;
		erec->reserved1 = 0x00;
		if (valid_sense & SC_VALID_SENSE) {
			bcopy(diskinfo->sense_buf, erec->req_sense_data, 128);
		}
		else {
			bzero(erec->req_sense_data, 128);
		}
		erec->reserved2 = diskinfo->stats.segment_count;
		erec->reserved3 = diskinfo->stats.byte_count;
	}
	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_log_error					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine places a previously recorded error record	*/
/*		in the system error log.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		cmd_ptr - Address of scbuf struct describing operation	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns no value.				*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		errsave							*/
/*									*/
/*									*/
/************************************************************************/

void
scdisk_log_error(
struct dk_cmd	*cmd_ptr,
int		sev)
{
	struct sc_error_log_df	*erec;
	struct scdisk_diskinfo	*diskinfo;

	/* Setup pointers for faster access */
	diskinfo = (struct scdisk_diskinfo *) cmd_ptr->diskinfo;
	erec = &(cmd_ptr->diskinfo->error_rec);
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,logerr, trc,(char)0, (uint)diskinfo, 
	      (uint)cmd_ptr->error_type,
	      (uint)cmd_ptr, (uint)cmd_ptr->status,(uint)sev);
#endif	
#endif
	switch(cmd_ptr->error_type) {
	case DK_MEDIA_ERR:
		if (sev == DK_HARD_ERROR)
			erec->error_id = ERRID_DISK_ERR1;
		else
			erec->error_id = ERRID_DISK_ERR4;
		break;
	case DK_HARDWARE_ERR:
		if (sev == DK_HARD_ERROR)
			erec->error_id = ERRID_DISK_ERR2;
		else
			erec->error_id = ERRID_DISK_ERR4;
		break;
	case DK_ADAPTER_ERR:
		if (sev == DK_HARD_ERROR)
			erec->error_id = ERRID_DISK_ERR3;
		else
			erec->error_id = ERRID_DISK_ERR4;
		break;
	case DK_RECOVERED_ERR:
		erec->error_id = ERRID_DISK_ERR4;
		break;
	case DK_UNKNOWN_ERR:
	default:
		if (sev == DK_HARD_ERROR)
			erec->error_id = ERRID_DISK_ERR5;
		else
			erec->error_id = ERRID_DISK_ERR4;
		break;
	}

	cmd_ptr->error_type = 0;
	errsave(erec, sizeof(struct sc_error_log_df));
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_watchdog						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is used for SCSI bus reset recovery.	*/
/*		When a UNIT ATTENTION is encountered, a 		*/
/*		timeout is invoked and this routine is called when	*/
/*		the timer pops.  Notice that the watchdog_timer field	*/
/*		in the diskinfo structure must be at the beginning so	*/
/*		the address of the diskinfo structure can be determined.*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		watchdog_timer - pointer to watchdog structure at the	*/
/*				 top of the diskinfo structure.		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None.						 	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/
void scdisk_watchdog(
struct watchdog	*watchdog_timer)
{
	struct scdisk_timer     *timer;
	struct scdisk_diskinfo	*diskinfo;
	uint	      		old_pri;




	timer = (struct scdisk_timer *) (watchdog_timer);
	diskinfo = (struct scdisk_diskinfo *)(timer->pointer);

	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif

	/*
	 *  If this timeout resulted from busy scsi status, we want to retry
	 *  the original command.  Otherwise the timeout came from a unit
	 *  attention, and the test unit ready needs to be issued to restart the
	 *  device.
	 */

#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,cmdtimer, entry,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,
	      (uint)diskinfo->timer_status);
#endif	
#endif

	if (diskinfo->timer_status & DK_TIMER_BUSY) {
		diskinfo->timer_status = 0;
		scdisk_start(diskinfo);
	}
	else {
		diskinfo->timer_status = 0;
		if ((diskinfo->dk_cmd_q_head->type == DK_Q_RECOV) ||
		    (diskinfo->dk_cmd_q_head->scbuf.flags & SC_Q_RESUME)) {
			/*
			 * We need to guarantee that any queue recovery
			 * ERP is done first.
			 */
			scdisk_start(diskinfo);
		}
		scdisk_test_unit_ready(diskinfo);
	}
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,cmdtimer, exit,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,(uint)0);
#endif	
#endif		
	
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif

	return;
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_cdt_func						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is called at dump time.  The component   	*/
/*		dump table has been allocated at config time, so this	*/
/*		routine fills in the table and returns the address.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg - 1 indicates the start of a dump			*/
/*		      2 indicates the end of a dump			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the address of the component 	*/
/*		dump table.						*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/
struct cdt *scdisk_cdt_func(
int	arg)
{
	struct scdisk_diskinfo	*diskinfo;
	struct cdt_entry	*entry_ptr;
	int			entry_count, i;

#ifdef DEBUG
	DKprintf(("Entering scdisk_cdt_func arg[0x%x]\n", 
	    arg));
#endif
	if (arg == 1) {
		/*  First dump entry is global info structure */
		entry_ptr = &scdisk_info.cdt->cdt_entry[0];
		bcopy("scinfo", entry_ptr->d_name, 7);
		entry_ptr->d_len = sizeof(struct scdisk_info);
		entry_ptr->d_ptr = (caddr_t) &scdisk_info;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Second dump entry is configured disk list */
		entry_ptr = &scdisk_info.cdt->cdt_entry[1];
		bcopy("cfglist", entry_ptr->d_name, 8);
		entry_ptr->d_len = 4 * DK_HASHSIZE;
		entry_ptr->d_ptr = (caddr_t) scdisk_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Third dump entry is opened disk list */
		entry_ptr = &scdisk_info.cdt->cdt_entry[2];
		bcopy("oplist", entry_ptr->d_name, 7);
		entry_ptr->d_len = 4 * DK_HASHSIZE;
		entry_ptr->d_ptr = (caddr_t) scdisk_open_list;
		entry_ptr->d_segval = (int) NULL;
		
		/*  Rest of entries are disk structures */
		entry_count = 2;
		for (i = 0 ; i < DK_HASHSIZE ; i++) {
			diskinfo = scdisk_list[i];
			while (diskinfo != NULL) {
				entry_ptr = &scdisk_info.cdt->cdt_entry[
				    ++entry_count];
				bcopy(diskinfo->error_rec.resource_name,
				    entry_ptr->d_name, 8);
				entry_ptr->d_len = sizeof(struct 
				    scdisk_diskinfo);
				entry_ptr->d_ptr = (caddr_t) diskinfo;
				entry_ptr->d_segval = (int) NULL;
				diskinfo = diskinfo->next;
			}
		}
	}

#ifdef DEBUG
	DKprintf(("Exiting scdisk_cdt_func\n"));
#endif
	return(scdisk_info.cdt);
}

/*
 * NAME: scdisk_q_cmd
 *
 * FUNCTION: Places a command block on the specified disk's queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)    scdisk_diskinfo  - Disk device specific information
 *		         dk_cmd           - Command block
 *
 * RETURNS:     Void.
 */

void scdisk_q_cmd(struct  dk_cmd *cmd_ptr,
		  char    queue,
		  uchar	  which_q)
{
	struct scdisk_diskinfo *diskinfo = NULL; /* disk device information */
						 /* structure		    */
        struct dk_cmd **head_ptr,
                      **tail_ptr;
	uchar	      head_is_qrecov = FALSE;


        ASSERT(cmd_ptr);

        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */


        cmd_ptr->next = NULL;/* set next to null, this will be put on end */
	cmd_ptr->prev = NULL;    

#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,qcmd, entry,(char)0, (uint)diskinfo, (uint)queue,
	      (uint)which_q, (uint)cmd_ptr->status,(uint)cmd_ptr->type);
#endif	
#endif
	if (which_q == DK_CMD_Q) {
		head_ptr = &(diskinfo->dk_cmd_q_head);
		tail_ptr = &(diskinfo->dk_cmd_q_tail);
		cmd_ptr->status |= DK_QUEUED;
	}
	else {
		ASSERT(which_q == DK_CMD_Q);
	}

        if (queue == DK_QUEUE) {
                /*
                 * Queue this command
                 */
                if (*head_ptr == NULL) {
                        /*
                         * if q is empty, set
                         * head to this one
                         */
                        *head_ptr = cmd_ptr;
		}
                else {
                        /*
                         * else q is not empty, so
                         * put on tail of q
                         */
			cmd_ptr->prev = *tail_ptr;
			if (*tail_ptr != NULL)
				(*tail_ptr)->next = cmd_ptr;
		}
                /*
                 * set new tail to this one
                 */
                *tail_ptr = cmd_ptr;
        } else {
                /*
                 * else Stack this command
                 */
                if (*head_ptr == NULL) {
                        /*
                         * if q is empty, set
                         * tail to this one also
                         */

                        *tail_ptr = cmd_ptr;
		}
                else {
                        /*
                         * else not empty, stack this command
                         * on top of the head of but not before
			 * a queue recovery command.
                         */
			if ((*head_ptr)->type == DK_Q_RECOV) {
				cmd_ptr->next = (*head_ptr)->next;
				if (cmd_ptr->next != NULL) {
					(cmd_ptr->next)->prev = cmd_ptr;
				}
				cmd_ptr->prev = *head_ptr;
				head_is_qrecov = TRUE;
				(*head_ptr)->next = cmd_ptr;
			}
			else {
				cmd_ptr->next = *head_ptr;
				(*head_ptr)->prev = cmd_ptr;
				if ((*head_ptr)->scbuf.flags & SC_Q_RESUME) {
					/*
					 * If the head element has
					 * a queue resume pending then
					 * make sure this new head element
					 * does as well.
					 */
					(*head_ptr)->scbuf.flags 
						&= ~SC_Q_RESUME;       
                                        if (cmd_ptr->type != DK_Q_RECOV) {
                                                /*
                                                 * We do not want to set
                                                 * queue resume on an
                                                 * already existing queue
                                                 * error recovery command.
						 * The main reason we
						 * do not want to do this
					         * for a queue error recovery 
						 * command is that it may be
						 * SC_Q_CLR command and we
						 * should not set SC_Q_RESUME.
                                                 */
                                                cmd_ptr->scbuf.flags
                                                        |= SC_Q_RESUME;
					}
					cmd_ptr->scbuf.flags |= SC_Q_RESUME;
				}
					
			}
		}

                /*
                 * set new head to this one
                 */
		if (!head_is_qrecov) {

			/* 
			 * If head is not a queue recovery command
			 * then point to new cmd_ptr
			 */
			*head_ptr = cmd_ptr;
		}
        }
	/*
	 * Change Commands Status as Queued
	 */

	cmd_ptr->queued = TRUE;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,qcmd, exit,(char)0, (uint)cmd_ptr, 
	      (uint)cmd_ptr->diskinfo,
	      (uint)which_q, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
#endif	
#endif
	return;
}
/*
 * NAME: scdisk_d_q_cmd_disable
 *
 * FUNCTION: Removes the specified command from the specified disk's  queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)  scdisk_diskinfo  - Disk device specific information
 *		       dk_cmd           - Command block
 *
 * RETURNS:     Void.
 */

void scdisk_d_q_cmd_disable(struct  dk_cmd *cmd_ptr,
		    uchar    which_q)
{

	uint    old_pri;			/* Old interrupt priority*/
	struct scdisk_diskinfo *diskinfo;



	if (cmd_ptr == NULL) {
		ASSERT(cmd_ptr);
		return;
	}

	diskinfo = cmd_ptr->diskinfo;
        /*
         * disable interrupts and spin lock
         */

	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 

	scdisk_d_q_cmd(cmd_ptr,which_q);

        /*
         * enable interrupts and unlock
         */

	unlock_enable(old_pri,&(diskinfo->spin_lock));

	return;
}

/*
 * NAME: scdisk_d_q_cmd
 *
 * FUNCTION: Removes the specified command from the specified disk's  queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)  scdisk_diskinfo  - Disk device specific information
 *		       dk_cmd           - Command block
 *
 * RETURNS:     Void.
 */

void scdisk_d_q_cmd(struct  dk_cmd *cmd_ptr,
		    uchar    which_q)
{
	struct scdisk_diskinfo *diskinfo = NULL; /* disk device information */
						 /* structure		    */

        struct dk_cmd **head_ptr,
                      **tail_ptr;


        ASSERT(cmd_ptr);
        diskinfo = cmd_ptr->diskinfo;             /* set disk pointer */

        if (!(cmd_ptr->queued))
		return;
        /*
         * set head pointer to prev initially. this will catch an unknown
         * cmd type when compiled without DEBUG
         */

        /*
         * Unlink this command from its list
         */
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,dqcmd, entry,(char)0, (uint)cmd_ptr, 
	      (uint)cmd_ptr->diskinfo,
	      (uint)which_q, (uint)cmd_ptr->status,(uint)cmd_ptr->type);
#endif	
#endif

	if (which_q == DK_CMD_Q) {
		head_ptr = &(diskinfo->dk_cmd_q_head);
		tail_ptr = &(diskinfo->dk_cmd_q_tail);
		cmd_ptr->status &= ~DK_QUEUED;
	} 
	else {
		ASSERT(which_q == DK_CMD_Q);
	}
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,dqcmd, trc,(char)0, (uint)cmd_ptr, (uint)*head_ptr,
	      (uint)which_q, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
#endif	
#endif

	if (cmd_ptr->prev != NULL)
		(cmd_ptr->prev)->next = cmd_ptr->next;

	if (cmd_ptr->next != NULL)
		(cmd_ptr->next)->prev = cmd_ptr->prev;

	if (*head_ptr == cmd_ptr)
		*head_ptr = cmd_ptr->next;

	if (*tail_ptr == cmd_ptr)
		*tail_ptr = cmd_ptr->prev;

	cmd_ptr->prev = NULL;
	cmd_ptr->next = NULL;


	/*
	 * Change Commands Status as UnQueued
	 */
	cmd_ptr->queued = FALSE;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,dqcmd, exit,(char)0, (uint)cmd_ptr, 
	      (uint)cmd_ptr->diskinfo,
	      (uint)*head_ptr, (uint)cmd_ptr->prev,(uint)cmd_ptr->next);
#endif	
#endif
        return;
}

/*
 * NAME: scdisk_cmd_alloc_disable
 *
 * FUNCTION: Allocates a command block if available for the routines that
 *	     have not disabled and spin locked.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   scdisk_devinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		unlock_enable	disable_lock	

 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 */
struct  dk_cmd *scdisk_cmd_alloc_disable(struct scdisk_diskinfo *diskinfo,
				 uchar cmd_type)
{
	uint    old_pri;			/* Old interrupt priority*/
	struct dk_cmd *cmd_ptr;

        /*
         * disable interrupts and spin lock
         */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	cmd_ptr = scdisk_cmd_alloc(diskinfo,cmd_type);

        /*
         * enable interrupts and unlock
         */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return(cmd_ptr);

}
/*
 * NAME: scdisk_cmd_alloc
 *
 * FUNCTION: Allocates a command block if available
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   scdisk_devinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 */
struct  dk_cmd *scdisk_cmd_alloc(struct scdisk_diskinfo *diskinfo,
				 uchar cmd_type)
{
	int     counted_cmds = 0;               /* Number of cmd_ptr's counted*/
	struct dk_cmd *pool;			/* Pointer to pool of command */
						/*  blocks		      */
	struct dk_cmd *cmd_ptr;
	int     found = FALSE;
        struct buf      *cmd_bp;
	struct scsi	*scsi;



	cmd_ptr = NULL;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,cmdalloc, trc,(char)0, (uint)diskinfo, (uint)cmd_type,
	      (uint)diskinfo->pool_index, (uint)0,(uint)counted_cmds);
#endif	
#endif
	if (cmd_type == DK_BUF) {
		pool = &diskinfo->cmd_pool[0];
		while (counted_cmds < (diskinfo->queue_depth)) {
			if (!(pool[diskinfo->pool_index].status)) {
				/*
				 * If not in use then pick it
				 */
				pool[diskinfo->pool_index].status = DK_IN_USE;
				pool[diskinfo->pool_index].type = cmd_type;
				pool[diskinfo->pool_index].aborted = FALSE;
				pool[diskinfo->pool_index].subtype = 0x00;
				pool[diskinfo->pool_index].next = NULL;
				pool[diskinfo->pool_index].prev = NULL;
				found = TRUE;
				break;
			}
			else {
				if ((diskinfo->pool_index +1) == diskinfo->queue_depth)
					diskinfo->pool_index = 0;
				else
					diskinfo->pool_index++;
			}
			counted_cmds++;

		}
		if (found) {
			cmd_ptr = &pool[diskinfo->pool_index];

		}
		else
			cmd_ptr = NULL;
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,cmdalloc, trc,(char)1, (uint)diskinfo, 
	      (uint)diskinfo->queue_depth,(uint)diskinfo->pool_index, 
	      (uint)cmd_ptr,(uint)counted_cmds);
#endif	
#endif
	}
	else if ((cmd_type == DK_IOCTL) && (!(diskinfo->ioctl_cmd.status))) {
		cmd_ptr = &diskinfo->ioctl_cmd;
		cmd_ptr->status = DK_IN_USE;
		cmd_ptr->aborted = FALSE;
		cmd_ptr->type = cmd_type;
		cmd_ptr->next = NULL;
		cmd_ptr->prev = NULL;

	}
	else if (cmd_type == DK_Q_RECOV) {
		
		cmd_ptr = &diskinfo->q_recov_cmd;
		if (!(cmd_ptr->status)) {
			cmd_ptr->status = DK_IN_USE;
			cmd_ptr->aborted = FALSE;
			cmd_ptr->type = cmd_type;
			cmd_ptr->next = NULL;
			cmd_ptr->prev = NULL;

			/* Initialize cmd block for start unit cmd */
			cmd_bp = &(cmd_ptr->scbuf.bufstruct);
			cmd_bp->b_dev = diskinfo->adapter_devno;
			cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
			cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
			cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
			scsi = &(cmd_ptr->scbuf.scsi_command);
			scsi->scsi_id = diskinfo->scsi_id;

			/*
			 * Let's guarantee that we do not
			 * hang due to some adapter driver
			 * incompatibility.  So we will
			 * set a time out value.
			 */
			cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
		}
		else 
			cmd_ptr = NULL;
	}

	if (cmd_ptr != NULL) {
		/*
		 * Set lun for cmd_ptr
		 */

		SCDISK_SET_CMD_LUN(cmd_ptr,diskinfo->lun_id);
	}
	return(cmd_ptr);
	
}
/*
 * NAME: scdisk_free_cmd_disable
 *
 * FUNCTION: Frees a Command structure for the routines that
 *	     have not disabled and spin locked.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   scdisk_devinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * EXTERNAL PROCEDURES CALLED:		
 *		unlock_enable	disable_lock	
 *
 * RETURNS:     Void.
 */
void scdisk_free_cmd_disable(
struct dk_cmd *cmd_ptr)
{
        uint    old_pri;
	struct scdisk_diskinfo *diskinfo;

	if (cmd_ptr == NULL) {
		ASSERT(cmd_ptr);
		return;
	}
	diskinfo = cmd_ptr->diskinfo;
        /*
         * disable interrupts and spin lock
         */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif

	scdisk_free_cmd(cmd_ptr);
        /*
         * enable interrupts and unlock
         */
#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif
	return;

}  
/*
 * NAME: scdisk_free_cmd
 *
 * FUNCTION: Frees a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   scdisk_devinfo  - disk info structure
 *                      dk_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void scdisk_free_cmd(
struct dk_cmd *cmd_ptr)
{
        struct  scdisk_diskinfo *diskinfo;


        if (cmd_ptr == NULL)
                /*
                 * if cmd is NULL, assume that it came from the cmd map and
                 * is already free
                 */
                return;

        /*
         * get diskinfo structure
         */

	diskinfo = cmd_ptr->diskinfo;



        ASSERT(cmd_ptr->status);
#ifdef DEBUG
#ifdef SC_GOOD_PATH
   scdisk_trc(diskinfo,cmdfree, trc,(char)0, (uint)diskinfo, (uint)0,
	      (uint)cmd_ptr->status, (uint)cmd_ptr->type,(uint)0);
#endif	
#endif
 
	if ((cmd_ptr->type == DK_RESET)
	    || (cmd_ptr->type == DK_WRITEV)){
		cmd_ptr->status = DK_FREE;
		cmd_ptr->scbuf.adap_q_status = 0;
		cmd_ptr->scbuf.flags = 0;

		return;
	}
	
        bzero(cmd_ptr, sizeof(struct dk_cmd)); /* clear entire command struc */
        cmd_ptr->diskinfo = diskinfo;          /* restore diskinfo pointer   */
	cmd_ptr->scbuf.flags = diskinfo->reset_delay;
	cmd_ptr->scbuf.bufstruct.b_event = EVENT_NULL;
	
	return;
}  /* end scdisk_free_cmd */


/*
 *
 * NAME: scdisk_sleep
 *
 * FUNCTION: Used to sleep on an event
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 *
 *
 * (DATA STRUCTURES:)  struct buf          - buf structure for I/O
 *                     
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     disable_lock                    unlock_enable
 *     iowait
 *
 * RETURNS:      Void
 */

void scdisk_sleep(
struct scdisk_diskinfo *diskinfo,
uchar   *intrpt,
uint    *event)
{
        int    old_pri;   /* old interrupt level */



#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
        if (*intrpt)
                /*
                 * if flag still set, go to sleep
                 */
		 e_sleep_thread((int *)event,&(diskinfo->spin_lock),
			       LOCK_HANDLER);
              
 
	/* 
	 * reenable interrupts and unlock
	 */

	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	old_pri = i_disable(INTTIMER);
        if (*intrpt)
                /*
                 * if flag still set, go to sleep
                 */

		 e_sleep((int *)event,EVENT_SHORT);
	/* 
	 * reenable interrupts and unlock
	 */


	i_enable(old_pri);
#endif
        return;
}


/*
 *
 * NAME: scdisk_start_watchdog
 *
 * FUNCTION: Start the disk's watchdog timer to wait before issuing
 *	     any new commands
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 *
 *
 * (DATA STRUCTURES:)  struct scdisk_diskinfo    disk's information
 *                     
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     disable_lock                    unlock_enable
 *     w_start
 *
 * RETURNS:      Void
 */

void scdisk_start_watchdog(struct scdisk_diskinfo *diskinfo,
			   ulong timeout)
{




	if (diskinfo->timer_status)  {
		/*
		 * If watchdog timer is already set for this
		 * disk then do not restart it.
		 */
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,sttimer, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->q_clr,
	      (uint)diskinfo->q_status);
#endif	
#endif
		return;
	}
#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,sttimer, trc,(char)1, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)diskinfo->q_clr,
	      (uint)diskinfo->q_status);
#endif	
#endif
	diskinfo->timer_status |= DK_TIMER_PENDING;
	diskinfo->watchdog_timer.watch.restart = timeout;
	w_start(&(diskinfo->watchdog_timer.watch));



        return;
}




/************************************************************************/
/*									*/
/*	NAME:	scdisk_pm_watchdog					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is used so our PM handler will not 	*/
/* 		sleep too long when waiting for a device to go		*/
/*		idle.							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		watchdog_timer - pointer to watchdog structure at the	*/
/*				 top of the diskinfo structure.		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None.						 	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		None.							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/
void scdisk_pm_watchdog(
struct watchdog	*watchdog_timer)
{
	struct scdisk_timer     *timer;
	struct scdisk_diskinfo	*diskinfo;
	uint	      		old_pri;




	timer = (struct scdisk_timer *) (watchdog_timer);
	diskinfo = (struct scdisk_diskinfo *)(timer->pointer);

	/* 
	 * disable my irpt level and spin lock
	 */
#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif



#ifdef DEBUG
#ifdef SC_ERROR_PATH
   scdisk_trc(diskinfo,pmh_timer, trc,(char)0, (uint)diskinfo, 
	      (uint)diskinfo->state,
	      (uint)diskinfo->dk_cmd_q_head, (uint)0,
	      (uint)diskinfo->timer_status);
#endif	
#endif
	
	/*
	 * Our PM handler is waiting on I/O to complete.
	 * So let's wake it up.  
	 * NOTE: The PM handler will return PM_ERROR
	 * in this case, since we're not clearing the
	 * SCDISK_PM_PENDING_OP flag here.
	 */
	e_wakeup((int *)&(diskinfo->pm_event));	


#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif

	return;
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_raw_iodone                                       */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine will be called whenever a command built    */
/*              for diskinfo->raw_io_cmd is iodoned. It just wakes up   */
/*              the sleeping process thread in scdisk_raw_io.           */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine is called on an interrupt level.           */
/*              It cannot page fault.                                   */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              cmd_ptr - Address of scbuf struct describing operation  */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns a zero value.                      */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*              None.                                                   */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void
scdisk_raw_iodone(
struct dk_raw_buf *raw_bp)
{

        uint    old_pri;
	struct scdisk_diskinfo *diskinfo;

	diskinfo = raw_bp->diskinfo;

#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));
	raw_bp->bp.b_flags |= B_DONE;
#else
	old_pri = i_disable(INTTIMER);
#endif

        /* 
	 * Wakeup sleeping process 
	 */
	raw_bp->diskinfo->raw_io_intrpt =  0;
        e_wakeup(&(raw_bp->bp.b_event));

#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif        

        return;
}

#ifdef DEBUG
/*
 * NAME: scdisk_trc_disable
 *
 * FUNCTION: Builds an internal Trace Entry for Debug for the routines that
 *	     have not disabled and spin locked. 
 *	     
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void scdisk_trc_disable(
struct scdisk_diskinfo *diskinfo,
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{
        uint    old_pri;

#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER,&(diskinfo->spin_lock));	 
#else
	old_pri = i_disable(INTTIMER);
#endif
	scdisk_trc(diskinfo,desc,type,count,word1,word2,word3,word4,word5);

#ifdef _POWER_MP
	unlock_enable(old_pri,&(diskinfo->spin_lock));
#else
	i_enable(old_pri);
#endif        

}
/*
 * NAME: scdisk_trc
 *
 * FUNCTION: Builds an internal Trace Entry for Debug.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void scdisk_trc(
struct scdisk_diskinfo *dp,
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{
        uint    old_pri;

	bcopy(desc,dp->dk_trace[dp->dk_trcindex].desc, SCDISK_TRC_STR_LENGTH);
        bcopy(type,dp->dk_trace[dp->dk_trcindex].type, 3);
        dp->dk_trace[dp->dk_trcindex].count = count;
        dp->dk_trace[dp->dk_trcindex].word1 = word1;
        dp->dk_trace[dp->dk_trcindex].word2 = word2;
        dp->dk_trace[dp->dk_trcindex].word3 = word3;
        dp->dk_trace[dp->dk_trcindex].word4 = word4;
        dp->dk_trace[dp->dk_trcindex].word5 = word5;
        if(++dp->dk_trcindex >= TRCLNGTH)
                dp->dk_trcindex = 0;
        bcopy(topoftrc,dp->dk_trace[dp->dk_trcindex].desc, 
	      SCDISK_TRC_STR_LENGTH);
        dp->dk_trctop = (int)&(dp->dk_trace[dp->dk_trcindex]);

}

#endif


/*
 *
 * NAME: scdisk_pm_handler
 *
 * FUNCTION: SCSI disk Power Management function.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 * NOTES:
 * 	   The device must be opened in order to issue any sc_bufs
 *	   SCSI commands to the device. If it is not open then
 *	   all SCSI commands will be failed by the adapter driver,
 *	   since it requires an SCIOSTART ioctl (which is issued by
 *	   this driver at open time).
 *
 * (DATA STRUCTURES:)  struct scdisk_diskinfo    disk's information
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * RETURNS:      int
 */

int scdisk_pm_handler(caddr_t private, int mode)
{
	struct scdisk_pm_handle *pmh = (struct scdisk_pm_handle *) private;
	struct scdisk_diskinfo *diskinfo;
	int		opri;
	int		errnoval = 0;
	int		rc = 0;
	

	/*
	 * Validate information received from PM core
	 */

	if (pmh == NULL) {
		ASSERT(FALSE);
		return (PM_ERROR);
	}

	diskinfo = pmh->diskinfo;

	if (diskinfo == NULL) {
		ASSERT(FALSE);
		return (PM_ERROR);
	}



	switch(mode){
	case PM_DEVICE_FULL_ON:


		pmh->handle.mode = PM_DEVICE_FULL_ON;

		break;
	case PM_DEVICE_ENABLE:

		/*
		 * Pin the bottom half of the driver, since
		 * we may not be open.
		 */

		errnoval = pincode((int(*)())scdisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}

		/* CD-ROM sleep mode support (unique for Super Slim CD-ROM) */
		opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));
		
		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */


#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,pmh_trc, trc,(char)1, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
#endif	
#endif
		if ((pmh->handle.mode == PM_DEVICE_SUSPEND) ||
		    (pmh->handle.mode == PM_DEVICE_HIBERNATION)) {
			/*
			 * If resuming from suspend or
			 * hibernate make sure to power
			 * on the device.
			 */
			pm_planar_control(pmh->handle.devno,
					  diskinfo->pm_device_id, 
					  PM_PLANAR_ON);

			if (diskinfo->pm_susp_bdr) {
				/*
				 * This device is buggy and requires
				 * the hardware workaround of a BDR
				 * after resumption from a SUSPEND.
				 * So we must issue a BDR to it.
				 */
				errnoval = scdisk_pm_bdr(diskinfo,&opri);
			}
			/*
			 * This is to prevent the PM core from
			 * prematurely issuing an PM_DEVICE_IDLE
			 */
			pmh->handle.activity =  PM_ACTIVITY_OCCURRED;
		}
		diskinfo->pm_pending &= ~PM_SCDISK_PENDING_SUSPEND;
		if ((diskinfo->opened) && (!diskinfo->cmds_out)) {
			/*
			 * If this device is open and
			 * there are no outstanding commands
			 * then issue test unit ready command. This
			 * will determine if the disk should be spun up.
			 * If so the normal error recovery of the driver
			 * will spin up the drive via a START UNIT command.
			 * NOTE: we don't need to wait for the
			 * completion of this command, since
			 * the SCSI adapter driver will not
			 * fail a PM_DEVICE_IDLE request if
			 * if has pending I/O.
			 */
			scdisk_test_unit_ready(diskinfo);
		} 
		pmh->handle.mode = PM_DEVICE_ENABLE;
		unlock_enable(opri,&(diskinfo->spin_lock));

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())scdisk_iodone);
			
		break;
	case PM_DEVICE_IDLE :
		/*
		 * Pin the bottom half of the driver, since
		 * we may not be open.
		 */

		errnoval = pincode((int(*)())scdisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}
		opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));

		/*
		 * Allow any halted I/O due to suspend or hibernate
		 * to resume.
		 */


#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,pmh_trc, trc,(char)2, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
#endif	
#endif
		if ((pmh->handle.mode == PM_DEVICE_SUSPEND) ||
		    (pmh->handle.mode == PM_DEVICE_HIBERNATION)) {
			/*
			 * If resuming from suspend or
			 * hibernate make sure to power
			 * on the device.
			 */
			pm_planar_control(pmh->handle.devno,
					  diskinfo->pm_device_id, 
					  PM_PLANAR_ON);

			if (diskinfo->pm_susp_bdr) {
				/*
				 * This device is buggy and requires
				 * the hardware workaround of a BDR
				 * after resumption from a SUSPEND.
				 * So we must issue a BDR to it.
				 */
				errnoval = scdisk_pm_bdr(diskinfo,&opri);
			}
		}

		diskinfo->pm_pending &= ~PM_SCDISK_PENDING_SUSPEND;


		if (!diskinfo->cmds_out) {

			if (diskinfo->opened) {
				/*
				 * If this device is open and
				 * there are no outstanding commands
				 * then issue the STOP UNIT command.
				 * NOTE: we don't need to wait for the
				 * completion of this command, since
				 * the SCSI adapter driver will not
				 * fail a PM_DEVICE_IDLE request if
				 * if has pending I/O.
				 */


				scdisk_start_unit(diskinfo, (uchar)DK_STOP,
						  (uchar) FALSE);
			} else {
				/*
				 * If the device is not open
				 * then we need to do some special
				 * stuff to spin it down. First we
				 * must unlock_enable, because fp_ioctl
				 * and fp_opendev can't be called while
				 * disable_locked. However since this device
				 * is not open,we need to guard against
				 * a possible open/unconfigure while 
				 * unlock_enabled. We will try to take the
				 * driver's global lock using LOCK_NDELAY.
				 * If some one has this lock (i.e some
				 * one is attempting an open/unconfigure), 
				 * it will fail immediately and will will exit
				 * this code with PM_ERROR.
				 *
				 * NOTE: We can't just use a LOCK_SHORT,
				 * because we could potentially get deadlocked
				 * with scdisk_open or scdisk_config (both
				 * this routines first take a lockl then
				 * do a disable (the reverse order of what
				 * we are using here)). Recall that inorder
				 * to avoid deadlocks with multiple locks,
				 * the locks must always be taken in the
				 * same order--never the reverse order.
				 */

				rc = lockl(&(scdisk_info.lock), 
					   LOCK_NDELAY);

				unlock_enable(opri,&(diskinfo->spin_lock));
				if (rc == LOCK_FAIL) {
					/*
					 * Some one else has the device
					 * lock (i.e. someone is opening
					 * or unconfiguring this device.
					 * So fail the IDLE.
					 */
					/*
					 * Unpin the bottom half of the driver
					 */
					(void *) unpincode((int(*)())
							   scdisk_iodone);
			
					return (PM_ERROR);
				}

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */
				(void) scdisk_pm_spindown(diskinfo,
							  (uchar) FALSE);


				opri = disable_lock(INTTIMER,
						    &(diskinfo->spin_lock));
				unlockl(&(scdisk_info.lock));
			}

			pmh->handle.mode = PM_DEVICE_IDLE;

		} else {

			/*
			 * There is an outstanding command. 
			 */
			unlock_enable(opri,&(diskinfo->spin_lock));
			/*
			 * Unpin the bottom half of the driver
			 */
			(void *) unpincode((int(*)())scdisk_iodone);
			
			return (PM_ERROR);
		}
		unlock_enable(opri,&(diskinfo->spin_lock));

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())scdisk_iodone);
			
		break;
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));
		diskinfo->pm_pending |= PM_SCDISK_PENDING_SUSPEND;

#ifdef DEBUG
#ifdef SC_GOOD_PATH
		scdisk_trc(diskinfo,pmh_trc, trc,(char)3, 
				   (uint)diskinfo, (uint)diskinfo->cmds_out,
				   (uint)pmh, (uint)pmh->handle.mode,
				   (uint)mode);
#endif	
#endif

		if (diskinfo->cmds_out) {
			
			/*
			 * If there is outstanding I/O, then
			 * we will wait a little while to see
			 * if it completes.
			 */

			diskinfo->pm_pending |= PM_SCDISK_PENDING_OP;
			
			/*
			 * Set timer in case we sleep too
			 * long.
			 */


			diskinfo->pm_timer.watch.restart = 4;
			w_start(&(diskinfo->pm_timer.watch));
			
			
			/*
			 * We will sleep here. Either scdisk_iodone
			 * or our watchdog timer routine will wake us
			 * up.
			 */
			e_sleep_thread((int *)&(diskinfo->pm_event),
				       &(diskinfo->spin_lock),
				       LOCK_HANDLER);
			
			/*
			 * NOTE: we must check cmds_out again, because
			 * an error recovery command could now be outstanding.
			 * This error recovery command could have be issued
			 * from the same scdisk_iodone that woke us up.
			 */
			if (diskinfo->cmds_out) {
				/*
				 * Some type of error recovery
				 * is in progress (i.e. retried command,
				 * reset seqeuence etc.).  So we will 
				 * fail this with PM_ERROR and allow
				 * this device to resume normal command
				 * processing.
				 */

				
				w_stop(&(diskinfo->pm_timer.watch));	

				diskinfo->pm_pending &= 
					~PM_SCDISK_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_SCDISK_PENDING_SUSPEND;

				unlock_enable(opri,
					      &(diskinfo->spin_lock));
				return (PM_ERROR);		

			} else if (diskinfo->pm_pending & PM_SCDISK_PENDING_OP) {
				/*
				 * If the pending idle flag
				 * is still set, then our
				 * watchdog timer has popped.
				 * So fail this with PM_ERROR.
				 */
				diskinfo->pm_pending &= 
					~PM_SCDISK_PENDING_OP;
				diskinfo->pm_pending &= 
					~PM_SCDISK_PENDING_SUSPEND;

				/*
				 * We need to restart any I/O
				 * that may have been suspended
				 * here.
				 */
				scdisk_start(diskinfo);

				unlock_enable(opri,
					      &(diskinfo->spin_lock));
				return (PM_ERROR);
				
			} else {
				/*
				 * I/O finished within our timeout
				 * value so stop the watchdog timer
				 * here.
				 */
				w_stop(&(diskinfo->pm_timer.watch));

			}
		}
		/*
		 * If we get here, then there are no outstanding commands.
		 * So set mode to the requested mode.
		 */
		
		if (mode == PM_DEVICE_SUSPEND) {
			
			if (diskinfo->opened) {
				/*
				 * Spin down disk to save power. 
				 * For a suspend request, we must
				 * wait for this command to complete,
				 * because the SCSI adapter driver
				 * will be called next by the PM core.
				 * If it has active commands, then
				 * it fails the suspend.
				 */
				diskinfo->disk_intrpt = 1;
				scdisk_start_unit(diskinfo, (uchar)DK_STOP,
						  (uchar) TRUE);
				scdisk_sleep(diskinfo,
					     &(diskinfo->disk_intrpt),
					     &(diskinfo->open_event));
			} else {
				/*
				 * If the device is not open
				 * then we need to do some special
				 * stuff to spin it down. First we
				 * must unlock_enable, because fp_ioctl
				 * and fp_opendev can't be called while
				 * disable_locked. However since this device
				 * is not open,we need to guard against
				 * a possible open/unconfigure while 
				 * unlock_enabled. We will try to take the
				 * driver's global lock using LOCK_NDELAY.
				 * If some one has this lock (i.e some
				 * one is attempting an open/unconfigure), 
				 * it will fail immediately and will will exit
				 * this code with PM_ERROR.
				 *
				 * NOTE: We can't just use a LOCK_SHORT,
				 * because we could potentially get deadlocked
				 * with scdisk_open or scdisk_config (both
				 * this routines first take a lockl then
				 * do a disable (the reverse order of what
				 * we are using here)). Recall that inorder
				 * to avoid deadlocks with multiple locks,
				 * the locks must always be taken in the
				 * same order--never the reverse order.
				 */

				rc = lockl(&(scdisk_info.lock), 
					   LOCK_NDELAY);

				unlock_enable(opri,&(diskinfo->spin_lock));
				if (rc == LOCK_FAIL) {
					/*
					 * Some one else has the device
					 * lock (i.e. someone is opening
					 * or unconfiguring this device.
					 * So fail the IDLE.
					 */

					diskinfo->pm_pending &= 
						~PM_SCDISK_PENDING_OP;

					diskinfo->pm_pending &= 
						~PM_SCDISK_PENDING_SUSPEND;


					return (PM_ERROR);
				}
				

				/*
				 * If we get here then, our lockl
				 * was successful. So we are now
				 * guarded against open/unconfigure
				 */

				(void) scdisk_pm_spindown(diskinfo,
							  (uchar)TRUE);

				opri = disable_lock(INTTIMER,
						    &(diskinfo->spin_lock));
				unlockl(&(scdisk_info.lock));
			}

			/*
			 * Try to also power off drive.
			 * This may not do anything, since
			 * it is dependent on what machine
			 * we're on and that the drive is
			 * an internal drive.
			 */
			
			pm_planar_control(pmh->handle.devno,
					  diskinfo->pm_device_id, 
					  PM_PLANAR_OFF);
		}
		
		if (pmh->handle.mode == PM_DEVICE_ENABLE) {
			pmh->handle.activity =  PM_ACTIVITY_NOT_SET;
		}
		pmh->handle.mode = mode;


		unlock_enable(opri,&(diskinfo->spin_lock));
		break;
	
	case PM_PAGE_FREEZE_NOTICE:	

		/*
		 * Pin the bottom half of the driver
		 */

		errnoval = pincode((int(*)())scdisk_iodone);
		if (errnoval) {
			return PM_ERROR;
		}
		return PM_SUCCESS;
		break;
	case PM_PAGE_UNFREEZE_NOTICE:
		

		/*
		 * Unpin the bottom half of the driver
		 */
		(void *) unpincode((int(*)())scdisk_iodone);
			
		return PM_SUCCESS;
		
	default :
		return PM_ERROR;
	}

	return PM_SUCCESS;

}

/*
 *
 * NAME: scdisk_pm_spindown
 *
 * FUNCTION: Issue stop unit command to devices that are
 *	     not opened
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned.

 * NOTE:  This routine assumes the caller is not disable_locked,
 *        and has done any necessary lockls.
 *
 * (DATA STRUCTURES:)  struct scdisk_diskinfo    disk's information
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * RETURNS:      int
 */

int scdisk_pm_spindown(struct scdisk_diskinfo *diskinfo,uchar immed_flag)
{
	int errnoval = 0;
	int idlun;

	/* 
	 * Open adapter for device and store filepointer
	 */
	errnoval = fp_opendev(diskinfo->adapter_devno, 
			      (int) FWRITE,
			      (caddr_t) NULL, 
			      (int) NULL, 
			      &(diskinfo->fp));
	
	if (errnoval == 0) {
		/* 
		 * Issue a start IOCTL to the 
		 * adapter driver 
		 */
		
		idlun = IDLUN(diskinfo->scsi_id,diskinfo->lun_id);
		
		errnoval = fp_ioctl(diskinfo->fp, 
				    (uint) SCIOSTART,
				    (caddr_t) idlun, 
				    (int) NULL);

		if (errnoval == 0) {
			
			diskinfo->errno = 0x00;
			diskinfo->disk_intrpt = 1;
			scdisk_start_unit_disable(diskinfo,
						  (uchar) DK_STOP,
						  immed_flag);
			scdisk_sleep(diskinfo,
				     &(diskinfo->disk_intrpt),
				     &(diskinfo->open_event));
			
			
			errnoval = diskinfo->errno;
			
			fp_ioctl(diskinfo->fp, (uint) SCIOSTOP,
				 (caddr_t) idlun, (int) NULL);
		}
		fp_close(diskinfo->fp);
	}
	
	return (errnoval);
}
/*
 *
 * NAME: scdisk_pm_bdr
 *
 * FUNCTION: Issue BDR (Bus Device Resets) to devices.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 * (DATA STRUCTURES:)  struct scdisk_diskinfo    disk's information
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      None
 *
 * RETURNS:      int
 */

int scdisk_pm_bdr(struct scdisk_diskinfo *diskinfo,int *opri)
{
	int   errnoval = 0;
	int   idlun;	


	idlun = IDLUN(diskinfo->scsi_id,diskinfo->lun_id);
	/*
	 * You can't call fp_opendev while disabled_locked
	 */

	unlock_enable(*opri,&(diskinfo->spin_lock));

	if (!diskinfo->opened) {
		

		/*
		 * This device is not open. So we
		 * must open the adapter driver and SCIOSTART
		 * it.
		 */
		 
		errnoval = fp_opendev(diskinfo->adapter_devno, 
				      (int) FWRITE,
				      (caddr_t) NULL, 
				      (int) NULL, 
				      &(diskinfo->fp));
		if (errnoval == 0) {
			/* 
			 * Issue a start IOCTL to the 
			 * adapter driver 
			 */
		
			errnoval = fp_ioctl(diskinfo->fp, 
					    (uint) SCIOSTART,
					    (caddr_t) idlun, 
					    (int) NULL);
		}
	}

	if (errnoval == 0) {
			
		errnoval = fp_ioctl(diskinfo->fp, 
				    (uint)SCIORESET,
				    (caddr_t)idlun, 
				    (int) NULL);
	}
	if (!diskinfo->opened) {
		/*
		 * This device is not open. So we
		 * must SCIOSTOP it and close the adapter.
		 */
		fp_ioctl(diskinfo->fp, (uint) SCIOSTOP,
			 (caddr_t) idlun, (int) NULL);
		fp_close(diskinfo->fp);


	}
	/* 
	 * Retake the spin lock
	 */
	*opri = disable_lock(INTTIMER,&(diskinfo->spin_lock));

	return(errnoval);
		 
}
