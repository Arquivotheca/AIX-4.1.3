/* @(#)46       1.8.4.6  src/bos/kernel/sys/POWER/scdiskdd.h, sysxdisk, bos41J, 9521A_all 5/23/95 17:46:39 */
#ifndef _H_SCDISKDD
#define _H_SCDISKDD
/*
 * COMPONENT_NAME: (SYSXDISK) SCSI Disk Device Driver Include File
 *
 * FUNCTIONS:  NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* scdiskdd.h header dependencies                                       */
/************************************************************************/

#include <sys/types.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/scsi.h>
#include <sys/watchdog.h>
#include <sys/iostat.h>
#include <sys/dump.h>
#include <sys/scdisk.h>
#include <sys/pm.h>

#ifdef DEBUG
#define DKprintf(args) if (scdisk_debug) printf args
#endif


/************************************************************************/
/* Device driver internal trace table information.                      */
/************************************************************************/
/*
 * Debug Trace length
 */
#ifdef DEBUG
#define TRACE_LABEL_SIZE        9       /* Size of trace table label	     */

#define TRACE_TABLE_PTR ">TRACEP<"
#define TRCLNGTH                1000    /* Length of of each Disk Trace      */
					/* Buffer.			     */
#define TRCLNGTH_PTR            10      /* Number of Disk Trace table        */
                                        /* buffers that we point to.         */
#endif

/*
 * Debug Trace Table Structure
 */
#ifdef DEBUG
struct scdisk_trace {
#define SCDISK_TRC_STR_LENGTH       8
        char    desc[SCDISK_TRC_STR_LENGTH];/* ASCII descrip of this entry   */
        char    type[3];                    /* ASCII desrip of entry or exit */
        char    count;                      /* which of several entries      */
        uint    word1;                      /*  meaning depends on the desc  */
        uint    word2;                      /*  meaning depends on the desc  */
        uint    word3;                      /*  meaning depends on the desc  */
        uint    word4;                      /*  meaning depends on the desc  */
        uint    word5;                      /*  meaning depends on the desc  */
};
#endif

/************************************************************************/
/* X3.131 SCSI Standard Sense Key Values 				*/
/************************************************************************/
#define DK_NO_SENSE		0x00
#define DK_RECOVERED_ERROR	0x01
#define DK_NOT_READY		0x02
#define DK_MEDIUM_ERROR		0x03
#define DK_HARDWARE_ERROR	0x04
#define DK_ILLEGAL_REQUEST	0x05
#define DK_UNIT_ATTENTION	0x06
#define DK_DATA_PROTECT		0x07
#define DK_BLANK_CHECK		0x08
#define DK_VENDOR_UNIQUE	0x09
#define DK_COPY_ABORTED		0x0A
#define DK_ABORTED_COMMAND	0x0B
#define DK_EQUAL_CMD		0x0C
#define DK_VOLUME_OVERFLOW	0x0D
#define DK_MISCOMPARE		0x0E

/************************************************************************/
/* Block Sizes.								*/
/************************************************************************/
#define DK_BLOCKSIZE	512		/* Disk blocksize 		*/
#define DK_M2F1_BLKSIZE 2048            /* The block size for CD-ROM    */
                                        /* XA data mode 2 form 1.       */
#define DK_M2F2_BLKSIZE 2336            /* The block size for CD-ROM    */
                                        /* XA data mode 2 form 2.       */
#define DK_CDDA_BLKSIZE 2352            /* The block size for CD-DA     */


/************************************************************************/
/* Multi-Session CD-ROM Defines						*/
/************************************************************************/


#define DK_CD_PVD       	0x10    /* The LBA where the Physical   */
                                        /* Volume Descriptor (PVD) is   */
					/* located on CDs with using    */   
           				/* the ISO 9660 standard        */
#define DK_SCSI_MS_DISCINFO     0xc7    /* The SCSI opcode of the       */
                                        /* Toshiba unique read disc     */
                                        /* info command                 */


/************************************************************************/
/* Misc. Defines 							*/
/************************************************************************/

#define DK_MAX_RETRY    5               /* maximum error retry count(+1)*/
#define DK_MAX_RESETS	3		/* maximum number of reset      */
					/* cycles on an error           */
#define DK_MAX_RESET_FAILURES 2		/* maximum number of reset cycle*/
					/* failures before we give up   */
#define DK_FMT_DEF_SIZE	4		/* size of format defect list.  */
					/* This size must be >= 4       */

#define DK_HASH		0x0F
#define DK_HASHSIZE	0x10
#define DK_REQSNS_LEN	0xFF
#define DK_ERROR	-1
#define DK_CMD_Q        0x1             /* Use the disks pending Queue  */
#define DK_QUEUE	0x1		/* Place cmd_ptr on end of Q    */
#define DK_STACK	0x2             /* Place cmd_ptr on front of Q  */
#define DK_PAGESIZE     PAGESIZE        /* Page size in bytes		*/

/* Queue types */
#define DK_PENDING	0x01
#define DK_INPROG	0x02

/* SCSI Disk Error types */
#define DK_MEDIA_ERR		0x01
#define DK_HARDWARE_ERR		0x02
#define DK_ADAPTER_ERR		0x03
#define DK_RECOVERED_ERR	0x04
#define DK_UNKNOWN_ERR		0x05

/* SCSI Disk Error severities */
#define DK_HARD_ERROR		0x01
#define DK_SOFT_ERROR		0x02

/* SCSI Disk Error severities */
#define DK_OVERLAY_ERROR	0x01
#define DK_NOOVERLAY_ERROR	0x02

/* SCSI Disk Mode Sense/Select defines */
#define DK_MAX_MODE_PAGES	0x3f	/* Maximum number of mode page  */
#define DK_BLK_DESC_LN_INDEX	3	/* Index to block descriptor    */
					/* length 			*/
#define DK_COMPLIES_PF		0x10	/* Indicates mode pages comply  */
					/* to Page Format               */

#define DK_MAX_SIZE_PG_E	0xe	/* The largest possible size of	*/
					/* Mode Page E from Parameter 	*/
					/* length.			*/
#define DK_VOL_START_PG_E	0x7	/* Starting offset of volume	*/
					/* from paramter length.	*/
					/* control info in Mode Page E	*/
#define DK_PAUSE		0x1	/* SCSI Pause command		*/
#define DK_RESUME		0x2	/* SCSI Resume command		*/


#define	DK_LG_IOCTL_LEN		65535   /* Size of large buffer of ioctl*/
#define DK_TOC_LEN		12	/* Size of buffer for read TOC  */
					/* data.			*/
#define DK_TOC_MSF		0x2     /* MSF bit setting of Read TOC  */
					/* SCSI command.		*/

#define DK_MSENSE_E_LEN		28      /* Size of buffer for Mode Sense*/
					/* of page E (Volume data)	*/

#define DK_CD_VOL_MASK		(CD_VOLUME_ALL | \
				CD_VOLUME_CHNLS | \
				CD_ALL_AUDIO_MUTE) /* Mask to determine */
					     /* volume type.		*/
#define DK_SUBQ_FMT		0x00	/* Sub-channel data format code	*/
					/* for read sub-Q.		*/
#define DK_SUBQ			0x1	/* Sub-Q bit setting for read	*/
					/* sub-Q channel.		*/
#define DK_SUBQ_LEN		48	/* Length of Read Sub-Q channel	*/
					/* SCSI command.		*/
#define DK_SUBQ_MSF		0x2     /* MSF bit setting of Read Sub-Q*/
					/* channel data.		*/
#define DK_AUDIO_STATUS_INVALID	0x00	/* Audio status byte not 	*/
					/* supported or not valid.	*/
#define DK_AUDIO_PLAY_IN_PROGRESS 0x11  /* Audio play operation in 	*/
					/* progress.			*/
#define DK_AUDIO_PLAY_PAUSED	0x12	/* Audio play operation paused.	*/
#define DK_PLAY_COMPLETED	0x13	/* Audio play operation 	*/
					/* successfully completed.	*/
#define DK_PLAY_STOP_ERROR	0x14	/* Audio play operation stopped	*/
					/* due to error.		*/
#define DK_NO_AUDIO_STATUS 	0x15	/* No current audio status to	*/
					/* return.			*/
/************************************************************************/
/* Initialization information on individual disks                       */
/************************************************************************/
struct disk_ddi  {
	char	resource_name[8];	/* resource name logged with  	*/
					/* each error log entry.        */
	dev_t	adapter_devno;		/* SCSI adapter device driver   */
					/* major/minor num 		*/
	uchar	scsi_id;		/* SCSI ID for disk             */
	uchar	lun_id;			/* SCSI LUN for disk            */
	uchar	safe_relocate;		/* Flag to indicate whether or  */
					/* not hardware re-assign of    */
					/* bad blocks is supported by   */
					/* the device, and if supported,*/
					/* whether or not it is safe in */
					/* case of power disruption.    */
#define DK_NO_RELOCATION	0       /* Relocation not supported     */
#define DK_SAFE_RELOCATION	1       /* Relocation is safe           */
#define DK_UNSAFE_RELOCATION	2       /* Rel. supported but not safe  */
	uchar	async_flag;		/* Flag to force a device to    */
					/* run async.  Possible values  */
					/* are SC_ASYNC or NULL.  For   */
					/* most devices, this should be */
					/* NULL (even if the device is  */
					/* async, it should still work).*/
					/* The purpose of this flag is  */
					/* to force a sync device to    */
					/* run async.                   */
	uchar	extended_rw;		/* This flag is set to TRUE if  */
					/* the SCSI extended read and   */
					/* write commands are supported */
					/* by the device.		*/
	uchar	reset_delay;		/* This flag is set to 		*/
                                        /* SC_DELAY_CMD if a delay is   */
					/* required after the device is */
					/* reset and before a command   */
					/* is sent.  The flag is set to */
					/* NULL if the delay is not     */
					/* needed.			*/
	uchar	q_type;			/* The queuing type of the disk */
					/* These are defined in scsi.h  */
					/* for resvd8 of sc_buf		*/
	uchar   q_err_value;	       	/* TRUE if Qerr bit should be   */
					/* set.	Otherwise FALSE.	*/
	uchar   clr_q_on_error;	       	/* TRUE if device clears its    */
					/* queue on error.  Otherwise   */
					/* FALSE.			*/
	uchar   buffer_ratio;           /* read/write data buffer ratio */
					/* of SCSI disks to be filled in*/
					/* for page 2 mode select data  */
	uchar   cmd_tag_q;		/* if device supports command   */
					/* tag queuing	this is TRUE	*/
					/* otherwise it is FALSE	*/
        uchar   reserve_lock;        	/* Flag to lock target from     */
                                        /* other initiators. 		*/
					/* False indicates no reserve   */
					/* will be issued on a normal   */
					/* open.    			*/ 
    	uchar   load_eject_alt; 	/* This is for the early IBM 	*/
					/* CDROM's which did not support*/
					/* the SCSI-2 load eject command*/
    	uchar   pm_susp_bdr; 		/* This is for a Teac CD-ROM    */
					/* which does not resume from a */
					/* PM Suspend. As a result a BDR*/
					/* must be issued to this device*/
					/* to wake it up.		*/
	uchar   dev_type;	        /* Type of device:              */
#define DK_DISK		0x01		/* SCSI disk			*/
#define DK_CDROM	0x02		/* SCSI CDROM			*/
#define DK_RWOPTICAL    0x03		/* SCSI READ/WRITE OPTICAL      */
	uchar    prevent_eject;  	/* Prevent target's media from  */
					/* being ejected while open.    */
        uchar   play_audio;             /* TRUE if SCSI-2 optional      */
                                        /* play audio command set is    */
                                        /* supported by the device.     */
        uchar   cd_mode2_form1_code;    /* The mode select density code */
                                        /* for CD-ROM data mode2 form1. */
        uchar   cd_mode2_form2_code;    /* The mode select density code */
                                        /* for CD-ROM data mode 2 form2.*/
        uchar   cd_da_code;             /* The mode select density code */
                                        /* for CD-DA data mode.         */
        uchar   multi_session;          /* TRUE if the CD-ROM drive     */
                                        /* supports the Toshiba unique  */
                                        /* Read Disc Info SCSI command. */
	uchar	valid_cd_modes;		/* Indicates which CD-ROM Data	*/
					/* modes are valid for this 	*/
					/* device. CD-ROM data mode 1	*/
					/* is always valid, since we	*/
					/* extract it from the mode_data*/
					/* attribute string.  As a 	*/
					/* result we do not set or even */
					/* have a flag for it, since we	*/
					/* assume it is always true.	*/
#define DK_M2F1_VALID	0x1		/* The cd_mode2_form1_code field*/
					/* is valid.			*/
#define DK_M2F2_VALID	0x2		/* The cd_mode2_form2_code field*/
					/* is valid.			*/
#define DK_CDDA_VALID	0x4		/* The cd_da_code field is valid*/

	ushort 	rw_timeout;		/* Time-out period for reads and*/
					/* writes.			*/
#define DK_TIMEOUT	30		/* Default time-out period(secs)*/
#define DK_RWOPT_TIME	120		/* R/W optical time out (secs)  */
	ushort 	fmt_timeout;		/* Format time-out period (secs)*/
#define DK_FMT_TIMEOUT	      1800      /* 30 minute time-out period    */
	ushort 	start_timeout;		/* Start Unit time-out period   */
					/* (secs).			*/
#define DK_START_TIMEOUT	60      /* 60 second time-out period    */
	ushort 	reassign_timeout;	/* reassign time-out period     */
#define DK_REASSIGN_TIMEOUT    120      /* 120 second time-out period   */
	ushort  queue_depth;		/* maximum number of commands   */
					/* that can be queued up to the */
					/* SCSI adpater device driver   */
					/* for a given device.		*/

	int	recovery_limit;		/* Max allowable recovery level */
					/* before a device recovered    */
					/* error is reported to be      */
					/* re-assigned. 		*/
	uint	segment_size;		/* # bytes in sample rate       */
	uint	segment_cnt;		/* # of segments read from this */
					/* device 			*/
	uint	byte_count;		/* # bytes read beyond the      */
					/* number of segments		*/
	uint	max_coalesce;		/* Max number of bytes to be 	*/
					/* coalesced.			*/
	uint    max_request;		/* maximum request size for     */
					/* a SCSI disk			*/
        uint    block_size;             /* The device's current block   */
                                        /* size.                        */
	int	mode_data_length;	/* mode select data length      */
	char	mode_data[256];		/* mode select data buffer      */
	int	mode_default_length;	/* mode select default data lng */
	char	mode_default_data[256];	/* mode select default data     */

        int     pm_dev_itime;           /* PM device idle time          */
        int     pm_dev_stime;           /* PM device standby time       */
        int     pm_dev_att;             /* PM device attribute          */
        int     pm_device_id;           /* PM device id for device      */
};

/************************************************************************/
/* Macro definitions							*/
/************************************************************************/

#define SCDISK_IN_PROG_ENQUEUE(diskinfo, bp)				\
{									\
	if (diskinfo->dk_bp_queue.in_progress.head == NULL) {		\
		diskinfo->dk_bp_queue.in_progress.head = bp;		\
		diskinfo->dk_bp_queue.in_progress.tail = bp;		\
		bp->av_forw = NULL;					\
	} else {							\
		diskinfo->dk_bp_queue.in_progress.tail->av_forw = bp;	\
		bp->av_forw = NULL;					\
		diskinfo->dk_bp_queue.in_progress.tail = bp;		\
	}								\
}

#define SCDISK_IN_PROG_DEQUEUE(diskinfo, bp)				\
{									\
       if (diskinfo->dk_bp_queue.in_progress.head != NULL) {		\
	       diskinfo->dk_bp_queue.in_progress.head = bp->av_forw;	\
	       if (diskinfo->dk_bp_queue.in_progress.head == NULL) {	\
		       diskinfo->dk_bp_queue.in_progress.tail = NULL;   \
		       }						\
       }                                                                \
}

/* 
 * This macro sets the lun for the corresponding cmd_ptr (struct
 * dk_cmd. NOTE: It assumes that scsi_cmd.lun has not yet been
 * set (i.e. it zeroes out the lower 5 bits.).
 */
#define SCDISK_SET_CMD_LUN(cmd_ptr,lun_id) 				\
{									\
									\
	cmd_ptr->scbuf.lun = (lun_id) & 0xff;				\
									\
	if ((lun_id) > 7 ) {						\
		/*							\
		 * If lun is greater than 7 then zero out		\
		 * the 3 bits in the SCSI CDB used for lun in 		\
		 * SCSI-1 & SCSI 2.					\
		 */							\
		cmd_ptr->scbuf.scsi_command.scsi_cmd.lun = 0x0;		\
									\
	}								\
	else {								\
		/*							\
		 * If lun is less then or equal to 7 then set		\
		 * the 3 bits in the SCSI CDB used for lun in 		\
		 * SCSI-1 & SCSI 2 to the lun value as well.		\
		 */							\
									\
		cmd_ptr->scbuf.scsi_command.scsi_cmd.lun = 		\
			((lun_id) & 0xff) << 5;				\
									\
	}								\
}
		
/*
 * If the block size does not equal DK_BLOCKSIZE then convert the
 * bp->blkno to the correct block number for the device's block size.
 * If diskinfo->block_size is not a multiple of DK_BLOCKSIZE then
 * set rc = -1;  If this is a multi-session CD then re-map all
 * DK_CD_PVD lba requests.
 */
#define SCDISK_GET_LBA(blkno,diskinfo,bp,rc)                            \
{                                                                       \
                                                                        \
       rc = 0;                                                          \
       if (diskinfo->block_size == DK_BLOCKSIZE) {                      \
	       /*							\
		* No conversion is requird here since			\
		* the buf structs b_blkno is in terms			\
		* of 512 byte blocks (DK_BLOCKSIZE).			\
		*/							\
               blkno = bp->b_blkno;                                     \
       }                                                                \
       else if (diskinfo->block_size % DK_BLOCKSIZE) {                  \
	       /*							\
		* If the device's block size is not a multiple		\
		* of 512 fail this request. In this case 		\
		* diskinfo->mult_of_blksize = 0.			\
		*/							\
									\
               rc = -1;                                     		\
       }                                                                \
       else {                                                           \
               if (bp->b_blkno % diskinfo->mult_of_blksize) {           \
		      /*						\
		       * Request must start on a device 		\
		       * block boundary. Otherwise fail it.		\
		       */						\
                      rc = -1;                                          \
               }                                                        \
               else {                                                   \
		      /*						\
		       * Request starts on a device 			\
		       * block boundary. So convert it from		\
		       * 512 bytes/per block block number to 		\
		       * the correct block number for the device.	\
		       */						\
									\
                      blkno = bp->b_blkno/diskinfo->mult_of_blksize;	\
									\
		      if ((blkno == DK_CD_PVD) &&			\
                         (diskinfo->last_ses_pvd_lba) &&		\
			  (diskinfo->block_size == DK_M2F1_BLKSIZE)) {  \
				/*					\
				 * If this is mult-session photo CD	\
				 * and the request is to lba 16		\
				 * (pvd) and we are running in M2F1	\
				 * (the only valid mode for photo CD),	\
				 * then remap it to the			\
				 * last session's pvd.			\
				 */					\
                                blkno = diskinfo->last_ses_pvd_lba;     \
                      }                                                 \
               }                                                        \
       }                                                                \
                                              				\
}


/************************************************************************/
/* Bufs used for raw I/O                                                */
/************************************************************************/
struct dk_raw_buf {
       struct buf              bp;      /* Raw buf to be processed.     */
       struct scdisk_diskinfo *diskinfo;/* Raw Buf's diskinfo structure.*/
};                                                                     



/************************************************************************/
/* Device capacity data block 						*/
/************************************************************************/
struct scdisk_capacity {
	int	lba;	/* last logical block address */
	int	len;	/* block length in bytes */
};

/************************************************************************/
/* Device statistics and thresholds structure                           */
/************************************************************************/
struct scdisk_stats {
	int	recovery_limit;		/* Max Allowable recovery level */
	uint	segment_size;		/* # bytes in sample rate       */
	uint	segment_count;		/* # segments processed         */
	uint	byte_count;		/* # bytes current segment      */
};

/************************************************************************/
/* Disk driver operation descriptor block                               */
/************************************************************************/
struct dk_cmd {
	struct sc_buf		scbuf;		/* buf for adapter use  */
	struct buf		*bp;		/* buf list for cmd     */
	uchar			type;		/* cmd type             */
#define DK_BUF		0x01			/* used by strategy     */
#define DK_IOCTL	0x02			/* used by ioctl        */
#define DK_REQSNS	0x04			/* used to request sense*/
#define DK_RESET	0x08			/* used during reset    */
#define DK_WRITEV	0x20			/* used for write verify*/
#define DK_Q_RECOV      0x40                    /*used for queue recovery*/
#define DK_REASSIGN	0x80			/* used for reassign op  */

	uchar			subtype;	/* cmd subtype  	*/

#define DK_TUR		0x01		    	/* test unit ready	*/
#define DK_START	0x02			/* start unit		*/
#define DK_STOP		0x03			/* stop unit		*/
#define DK_RESERVE	0x04			/* reserve		*/
#define DK_RELEASE	0x05			/* release		*/
#define DK_MSENSE	0x06			/* mode sense		*/
#define DK_SELECT	0x07			/* mode select		*/
#define DK_READCAP	0x08			/* read capacity	*/
#define DK_RDWR_SENSE	0x09			/* R/W with sense ioctl */
#define DK_IOCMD	0x0a			/* Diagnostic ioctl	*/
#define DK_PREVENT	0x0b			/* Prevent ejection     */
#define DK_ALLOW	0x0c			/* Allow  ejection      */
#define DK_ERP_IOCTL	0x0d			/* Ioctls in which the  */
						/* driver will perform  */
						/* error recovery       */
#define DK_READ_INFO    0xe                     /* Determine if CD is   */
                                                /* multi-session and,   */
                                                /* if so, it will       */
                                                /* return the starting  */
                                                /* address of the last  */
                                                /* session.             */
	uchar			group_type;
#define DK_SINGLE	0x01
#define DK_COALESCED	0x02

	uchar			flags;
#define DK_READY	0x01

	uchar			status;        /* State of command block */
#define DK_FREE		0x00
#define DK_IN_USE	0x01
#define DK_ACTIVE       0x02
#define DK_QUEUED	0x04
#define DK_RETRY	0x08	

	uchar			queued;       /* TRUE if queued         */
	ushort			retry_count;
	uchar			intrpt;        /* used to wait for      */
					       /* resources             */
	uchar			resvd1;
	uchar			resvd2;
	uchar			aborted;       /* TRUE if command is    */
					       /* being aborted		*/
	uchar		        error_type;    /* Type of error         */
					       /* (for logging)		*/
	uint			segment_count;
	uint			soft_resid;
	struct scdisk_diskinfo	*diskinfo;
	struct dk_cmd		*next;        
	struct dk_cmd		*prev;
};




/************************************************************************/
/* Disk bp queue control structs                                        */
/************************************************************************/

struct dk_queue_control {
	struct buf	*head;
	struct buf	*tail;
};

struct dk_bp_queue {
	struct dk_queue_control	pending;
	struct dk_queue_control	in_progress;
};

/************************************************************************/
/* Information for entire driver                                        */
/************************************************************************/
struct scdisk_info {
	int		config_cnt;	/* Configuration count          */
	int		open_cnt;	/* Number devices open		*/
	lock_t		lock;		/* Lock Word                    */
	struct cdt	*cdt;		/* Component dump table ptr	*/
};

/************************************************************************/
/* Defect list for reassignment operations                              */
/************************************************************************/
struct scdisk_def_list {
	uchar   header[4];
	uchar   descriptor[4];
};

/************************************************************************/
/* Structure used for DKIORDSE and DKIOWRSE ioctl's.                    */
/************************************************************************/
struct scdisk_ioctl_req_sense {
	struct xmem	xmemd;		/* xmemd for request sense data */
	char		*buffer;	/* user address of r.s. buf     */
	int		count;		/* length of user r.s. buf      */
};

/************************************************************************/
/* Mode Data Format Control Structure					*/
/************************************************************************/
struct scdisk_mode_format {
        signed char page_index[DK_MAX_MODE_PAGES]; /* offset to page    */
        ushort  sense_length;                      /* length of sense   */
        uint    block_length;                      /* device block lgth */
};
struct scdisk_timer {
	struct watchdog watch;			  /* system watchdog timer*/
	void   *pointer;			  /* diskinfo pointer     */
};
struct scdisk_pm_handle {
	struct 	pm_handle	handle;
	struct	scdisk_diskinfo *diskinfo;
};


/************************************************************************/
/* Information on individual disks                                      */
/************************************************************************/
struct scdisk_diskinfo {
	struct scdisk_diskinfo  *next;	        /* Ptr next disk's info */
	struct scdisk_diskinfo  *next_open;     /* Ptr to next open disk*/
	dev_t			devno;          /* Device major+minor   */
						/* numbers		*/

        /*
         * NOTE: the preceding fields must remain in
         * this order to stay consistent with hardware's
	 * thresholding disablement scheme.  In other words they
	 * currently traverse scdisk_open_list of diskinfo structures
	 * via the next pointer looking for a devno match.
         */

	dev_t			adapter_devno;  /* Adapter maj+min      */
	struct scdisk_timer     watchdog_timer;	/* For reset recovery   */
	uchar			scsi_id;        /* SCSI id for device   */
	uchar			lun_id;         /* SCSI lun for device  */
	ushort			reset_count;    /* Number of reset tries*/
	struct dk_cmd           *dk_cmd_q_head; /* head of dk_cmd queue */
	struct dk_cmd           *dk_cmd_q_tail; /* tail of dk_cmd queue */
	struct dk_cmd           ioctl_cmd;      /* reserved ioctl cmd */

	struct dk_cmd	        *cmd_pool;      /* Pool of buf & ioctl  */
						/* cmds                 */
	int                     pool_index;     /* index into above pool*/

	uint			open_event;	/* open/close event word*/
	struct dk_cmd		*checked_cmd;   /* cmd with check cond. */
	struct dk_cmd		*writev_err_cmd;/* cmd for which write  */
						/* verify was issued    */
	struct dk_cmd           *reassign_err_cmd;/* cmd which reassign */
						  /* is being done for  */
	struct dk_cmd	        reset_cmd;      /* reserved reset cmd   */
	struct dk_cmd	        reqsns_cmd;     /* reserved reqsns cmd  */
	struct dk_cmd	        writev_cmd;     /* reserved write and   */
						/* verify  cmd         */
	struct dk_cmd	        q_recov_cmd;    /* reserved command for*/
						/* Q clear and Q resume */
	struct dk_cmd		reassign_cmd;	/* reserved reassing cmd*/
	struct dk_cmd		dmp_cmd;	/* reserved command for */
						/* dump error recovery  */
	struct dk_bp_queue	dk_bp_queue;    /* bp queue control     */

	uchar		mode;   /* Current op mode of device            */
#define DK_NORMAL	0x01    /* Normal IO mode for device            */
#define DK_DIAG		0x02    /* Diagnostic mode for device           */
#define DK_SINGLE_MD	0x04    /* Format mode for device		*/
	uchar		state;  /* Current state of device              */
#define DK_OFFLINE	0x00    /* Device is uninitialized and offline  */
#define DK_ONLINE	0x01    /* Device is initialized and online     */
#define DK_RST_FAILED	0x02    /* Device is uninitialized and offline  */
#define DK_REQSNS_PENDING   DK_REQSNS  /* Device has request sense      */
				       /* pending			*/
#define DK_RESET_PENDING    DK_RESET   /* Device has reset pending      */
#define DK_DUMP_PENDING     0x10       /* Device has dump pending       */
#define DK_WRITEV_PENDING   DK_WRITEV  /* Device has write verify 	*/
				       /* pending			*/
#define DK_Q_RECOV_PENDING  DK_Q_RECOV /* Device has q recovery pending */
#define DK_REASSIGN_PENDING DK_REASSIGN/* Device has reassign pending   */

#define DK_RECOV_MASK   (DK_REQSNS_PENDING | DK_RESET_PENDING  \
			| DK_REASSIGN_PENDING | DK_Q_RECOV_PENDING \
	                |DK_DUMP_PENDING)
				/* Mask of DK_REQSNS_PENDING	        */
				/*      DK_RESET_PENDING  		*/
				/*	DK_DUMP_PENDING			*/
				/*	DK_Q_RECOV_PENDING		*/
				/*	DK_REASSIGN_PENDING 		*/
	uchar		disk_intrpt;     /* used to wait for resources  */
	uchar		raw_io_intrpt;	/* used for raw I/O event word  */
	uchar		ioctl_chg_mode_flg; /* Used to indicate that the*/
					/* DK_CD_MODE ioctl has been	*/
					/* performed.			*/
        uchar           m_sense_status; /* flag if chgable or curr sns  */
#define DK_SENSE_CURRENT        0x00    /* the sense reflects current   */
#define DK_SENSE_CHANGEABLE     0x40    /* the sense reflects changeable*/
	uchar		opened;         /* device has been opened       */

	uchar		cmd_pending;    /* cmd pending in strategy      */
	uchar		errno;          /* saved error on open path     */
	uchar		retain_reservation; /* Do not release device    */
	uchar		q_type;		/* Queuing type using for normal*/
					/* operation			*/
	uchar		q_err_value;    /* Queue error recovery flag    */
	uchar 		clr_q_on_error; /* Whether or not disk clears   */
					/* its queue on error		*/   
	uchar           buffer_ratio;   /* Read/Write buffer ration     */
	uchar 		cmd_tag_q;      /* If device supports command   */
					/* tag queuing			*/
	uchar		q_status;	/* Used to determine if the     */
					/* adapter's queue was cleared  */
	uchar		q_clr;		/* TRUE if Clear Q command is   */
					/* needed			*/	
	uchar		timer_status;	/* If a watchdog timer is set   */
					/* as well as why it is set	*/
#define DK_TIMER_PENDING   0x1		/* Timer is set			*/
#define DK_TIMER_BUSY      0x2		/* Timer was set due to SCSI    */
					/* Busy status			*/
	
	uchar		restart_unit;   /* Issue SC_RESUME to adapter   */
	uchar		retry_flag;     /* Set if a retry is to be done */
	uchar		safe_relocate;  /* Is relocation safe?          */
	uchar		async_flag;     /* SC_ASYNC or NULL for sync    */

	uchar		dump_inited;    /* Dump has been inited for dev */
	uchar		extended_rw;    /* is extended r/w supported?   */
	uchar		reset_delay;    /* is delay needed after reset? */
	uchar		starting_close; /* TRUE if a close is being     */
					/* attempted for this disk.	*/
	uchar		reset_failures; /* Number of times the reset    */
					/* cycle has failed without a   */
					/* a good buf or ioctl 		*/
					/* completion.			*/
	uchar		wprotected;	/* True if the device is media  */
					/* write protected.  False 	*/
					/* otherwise.			*/
        uchar           reserve_lock;   /* Flag to lock target from     */
                                        /* other initiators. 		*/
					/* False indicates no reserve   */
					/* will be issued on a normal   */
					/* open.    			*/  
	uchar           prevent_eject;  /* Prevent target's media from  */
					/* being ejected while open.    */ 
  	uchar		cfg_prevent_ej; /* prevent eject configurable   */
					/* attribute's value.		*/
  	uchar		cfg_reserve_lck;/* reserve lock configurable    */
					/* attribute's value.		*/
    	uchar   	load_eject_alt; /* This is for the early IBM 	*/
					/* CDROM's which did not support*/
					/* the SCSI-2 load eject command*/
    	uchar           pm_susp_bdr; 	/* This is for a Teac CD-ROM    */
					/* which does not resume from a */
					/* PM Suspend. As a result a BDR*/
					/* must be issued to this device*/
					/* to wake it up.		*/
	uchar           dev_type;	/* Type of device:              */
	uchar		ioctl_pending;  /* Ioctl operation pending.	*/
        uchar           play_audio;     /* TRUE if SCSI-2 optional      */
                                        /* play audio command set is    */
                                        /* supported by the device.     */
	uchar		overide_pg_e;	/* TRUE if a play audio ioctl	*/
					/* has been requested to change	*/
					/* the volume or audio channel	*/
					/* mapping, since opened.	*/
	uchar 		cd_mode1_code;  /* The mode select density      */
                                        /* code for CD-ROM data mode 1  */
        uchar           cd_mode2_form1_code;/* The mode select density  */
                                        /* code for CD-ROM data mode 2  */
                                        /* form 1.                      */
        uchar           cd_mode2_form2_code;/* The mode select density  */
                                        /* code for CD-ROM data mode 2  */
                                        /* form 2.                      */
        uchar           cd_da_code;     /* The mode select density code */
                                        /* for CD-DA data mode.         */
        uchar           current_cd_code;/* The current CD density code  */
        uchar           current_cd_mode;/* The Current CD-ROM Data mode */
        uchar           multi_session;  /* TRUE if the CD-ROM drive     */
                                        /* supports the Toshiba unique  */
                                        /* Read Disc Info SCSI command. */
	uchar		valid_cd_modes;	/* Indicates which CD-ROM Data	*/
					/* modes are valid for this 	*/
					/* device. CD-ROM data mode 1	*/
					/* is always valid, since we	*/
					/* extract it from the mode_data*/
					/* attribute string.  As a 	*/
					/* result we do not set or even */
					/* have a flag for it, since we	*/
					/* assume it is always true.	*/
        uchar           mult_of_blksize;/* The multiple needed to go    */
                                        /* from DK_BLOCKSIZE to the     */
                                        /* diskinfo->block_size         */
	uchar		play_audio_started; /* A play audio command was */
					/* issued without a subsequent  */
					/* stop unit.  Thus the play	*/
					/* operation may still be in 	*/
					/* progress.			*/
	ushort 		rw_timeout;	/* Time-out period for reads and*/
					/* writes.			*/
	ushort 		fmt_timeout;	/* Format time-out period (secs)*/
	ushort 		start_timeout;	/* Start Unit time-out period   */
					/* (secs).			*/
	ushort 		reassign_timeout;/* reassign time-out period    */
	ushort          queue_depth;	/* maximum number of commands   */
					/* that can be queued up to a   */
					/* device			*/
	ushort	        cmds_out;       /* Number of command issued for */
					/* this device			*/
        struct dk_cmd   *raw_io_cmd;	/* Pointer to a raw I/O command	*/
					/* that either are not on block	*/
					/* boundaries or do not have	*/
					/* transfer lengths that are 	*/
					/* multiples of the block size.	*/
	struct buf 	*currbuf;	/* current buf of elevator      */
	struct buf 	*low;		/* low buf of elevator          */
					/* that can be queued up to a   */
					/* device			*/
        uint            block_size;     /* The device's current block   */
                                        /* size.                        */
        uint            cfg_block_size; /* The block size specified by  */
                                        /* the configuration method.    */

        uint            last_ses_pvd_lba; /* The location of the PVD    */
                                        /* (Physical Volume Descriptor) */
                                        /* on the last session of a     */
                                        /* multi-session CD.            */

	uint		max_request;    /* Maximum request to scsi dd   */
	uint		max_coalesce;	/* Maximum size to coalesce	*/
   
	lock_t		lock;           /* Lock Word                    */
	struct file	*fp;            /* filepointer for dev--- calls */

	struct sc_error_log_df error_rec; /* Error record for device    */

	struct scdisk_stats stats;      /* I/O & Soft error statistics  */
	int		mode_data_length; /* Length of useable mode data*/
        uchar           disc_info[4];    /* Disc information data buffer*/
	uchar		mode_buf[256];  /* Mode data buffer             */
	uchar		sense_buf[256]; /* Request Sense data buffer    */
	uchar		ch_data[256];	/* buffer of chngeable mode data*/
	uchar		df_data[256];	/* buffer of default mode data  */
        uchar def_list_header[DK_FMT_DEF_SIZE]; /* buffer for format	*/
					/* defect list header.		*/
	uchar ioctl_buf[DK_REQSNS_LEN]; /* Ioctl data buffer		*/

	uchar mode_page_e[DK_MAX_SIZE_PG_E-DK_VOL_START_PG_E+1]; 
					/* Volume control settings from	*/
					/* Mode Page E in ODM.		*/ 
	struct scdisk_mode_format dd;   /* Mode control for desired data*/
        struct scdisk_mode_format df;   /* Mode control for default data*/
        struct scdisk_mode_format ch;   /* Mode control for chngeable   */
        struct scdisk_mode_format cd;   /* Mode control for current     */
	struct scdisk_ioctl_req_sense ioctl_req_sense;/* Req Sense info */
	struct scdisk_capacity capacity; /* Disk capacity information   */
	struct scdisk_def_list def_list;
	struct dkstat	dkstat;		/* Disk iostat information      */
#ifdef _POWER_MP
	Simple_lock	spin_lock;	/* Disk Thread-Interrupt Spin	*/
#endif					/* Lock.			*/
#ifdef DEBUG
	char trace_label[TRACE_LABEL_SIZE];/* Label in memory so        */
				      /* trace table is easy to find	*/	
        struct scdisk_trace *dk_trace;/* Disk Trace Buffer              */
        int             dk_trctop;    /* Pointer in Disk Trace buffer   */
				      /* where next entry will go.	*/
        int             dk_trcindex;  /* Index into Disk Trace Buffer   */
#endif
        struct scdisk_pm_handle pmh;    /* power management pm_handle   */
					/* for device.			*/
        uchar           pm_pending;     /* PM pending operation         */
#define PM_SCDISK_PENDING_OP          	0x01
#define PM_SCDISK_PENDING_SUSPEND	0x02
        uchar           pm_reserve[2];  /* reserved area                */
        int             pm_device_id;   /* PM device id for CD-ROM      */
	uint		pm_event;	/* PM event word.		*/
	struct scdisk_timer pm_timer;	/* For reset recovery   */
};

/************************************************************************/
/* Request Sense Data Block                                             */
/************************************************************************/
struct scdisk_req_sense_info  {
	uchar	err_code;	/* error class and code   */
	uchar	rsvd0;
	uchar	sense_key;
	uchar	sense_byte0;
	uchar	sense_byte1;
	uchar	sense_byte2;
	uchar	sense_byte3;
	uchar	add_sense_length;
	uchar	add_sense_byte0;
	uchar	add_sense_byte1;
	uchar	add_sense_byte2;
	uchar	add_sense_byte3;
	uchar	add_sense_key;
	uchar	add_sense_qualifier;
	uchar	fru;
	uchar	flag_byte;
	uchar	field_ptrM;
	uchar	field_ptrL;
	uchar	rsvd2;
	uchar	rsvd3;
	uchar	unit_ref_code1;
	uchar	unit_ref_code2;
	uchar	rsvd4;
	uchar	rsvd5;
	uchar	error_rec0;
	uchar	error_rec1;
	uchar	error_rec2;
	uchar	error_rec3;
	uchar	error_rec4;
	uchar	error_rec5;
	uchar	rsvd6;
	uchar	rsvd7;
};

#ifndef _NO_PROTO 
/************************************************************************/
/* scdisk functions                                                     */
/************************************************************************/

int	scdisk_config(dev_t devno, int op, struct uio *uiop);
int	scdisk_open(dev_t devno, int rwflag, int chan, int ext);
int	scdisk_close(dev_t devno, int chan, int ext);
int	scdisk_read(dev_t devno, struct uio *uiop, int chan, int ext);
int	scdisk_write(dev_t devno, struct uio *uiop, int chan, int ext);
int	scdisk_rdwr(dev_t devno,struct uio *uiop,int chan, 
		    int ext,int rw_flag);

int	scdisk_mincnt(struct buf *bp, void *minparms);
int	scdisk_raw(struct uio *uiop,int ext,int rw_flag,
		   struct scdisk_diskinfo  *diskinfo);

#ifdef _LONG_LONG
int	scdisk_raw_buffer(struct iovec *iovp,struct xmem *xmemp,
			  struct uio *uiop,offset_t offset,
			  int rw_flag,
			  struct scdisk_diskinfo  *diskinfo,
			  int ext);
int     scdisk_io_buffer(struct iovec *iovp,struct xmem  *xmemp,  
			 struct uio *uiop, offset_t offset,  
                         int  rw_flag, 
			 struct scdisk_diskinfo  *diskinfo,
			 int size,             
			 int  start_flag);
int	scdisk_raw_io(struct iovec *iovp,struct uio *uiop,
		      offset_t  offset,  int rw_flag,
		      struct scdisk_diskinfo *diskinfo, 
		      int size);
#else
int	scdisk_raw_buffer(struct iovec *iovp,struct xmem *xmemp,
			  struct uio *uiop,off_t offset,
			  int rw_flag,
			  struct scdisk_diskinfo  *diskinfo,
			  int ext);

int     scdisk_io_buffer(struct iovec *iovp,struct xmem  *xmemp,  
			 struct uio *uiop, off_t offset,  
                         int  rw_flag, 
			 struct scdisk_diskinfo  *diskinfo,
			 int size,             
			 int  start_flag);
int	scdisk_raw_io(struct iovec *iovp,struct uio *uiop,
		      off_t  offset,  int rw_flag,
		      struct scdisk_diskinfo *diskinfo, 
		      int size);
#endif
struct dk_cmd *scdisk_build_raw_cmd(struct scdisk_diskinfo *diskinfo,
				    int rw_flag, int  block_number,
				    int size, char *buffer,
				    struct dk_raw_buf *raw_bp, int aspace);
int	scdisk_ioctl(dev_t devno, int op, int arg, int flag, int chan,int ext);
int     scdisk_rdwrse(int op,int arg, int devflag, 
		      struct scdisk_diskinfo	*diskinfo);
int	scdisk_dkiocmd(int arg, int devflag, 
		      struct scdisk_diskinfo	*diskinfo);
int	scdisk_dk_amr_pmr(int op,struct scdisk_diskinfo	*diskinfo);
int	scdisk_dkeject(struct scdisk_diskinfo	*diskinfo);
int	scdisk_dkformat(int arg, int devflag, 
			struct scdisk_diskinfo *diskinfo);
int	scdisk_dkaudio(int arg, int devflag,
		       struct scdisk_diskinfo  *diskinfo);
int	scdisk_dk_cd_mode(int arg,int devflg,
			   struct scdisk_diskinfo  *diskinfo);
int     scdisk_alloc(struct scdisk_diskinfo *diskinfo);
void    scdisk_init_cmds(struct scdisk_diskinfo *diskinfo);
void    scdisk_release_allow(struct scdisk_diskinfo *diskinfo);
int	scdisk_ioctl_mode_sense(struct scdisk_diskinfo  *diskinfo,
				uchar  pc,uchar page_code,
				int allocation_length);
int 	scdisk_audio_msf(struct scdisk_diskinfo  *diskinfo,
			 int start_msf,int end_msf);
int	scdisk_audio_trk_indx(struct scdisk_diskinfo  *diskinfo,
			      uchar  start_trk,uchar  start_indx,
			      uchar  end_trk,uchar end_indx);
int	scdisk_pause_resume(struct scdisk_diskinfo  *diskinfo,
			    uchar pause_resume_flag);
int	scdisk_read_subchnl(struct scdisk_diskinfo  *diskinfo,
			    uchar sub_chnl_fmt,uchar  trk_num,
			    uchar subq,uchar msf,
			    int allocation_length);
int     scdisk_read_toc(struct scdisk_diskinfo  *diskinfo,uchar start_trk,
			uchar msf,int allocation_length);
int	scdisk_pin_buffer(caddr_t  buffer_addr,int *length,short segflag,
			  register int    blocksize);

int	scdisk_strategy(struct buf *bp);
int	scdisk_dump(dev_t devno, struct uio *uiop, int cmd, int arg, int chan,
	int ext);
int	scdisk_dump_write(struct scdisk_diskinfo *diskinfo,
			  struct dk_cmd  *write_cmd_ptr,
			  int chan, int ext);
void    scdisk_process_dmp_error(struct scdisk_diskinfo *diskinfo,
				 struct dk_cmd  **dmp_cmd_ptr,
				 struct dk_cmd *write_cmd_ptr);
int	scdisk_process_dmp_sns(struct scdisk_diskinfo *diskinfo,
			       struct dk_cmd  **dmp_cmd_ptr,
			       struct dk_cmd *write_cmd_ptr);
void	scdisk_dmp_start_unit(struct scdisk_diskinfo	*diskinfo,
			      uchar start_stop_flag,
			      struct dk_cmd **dmp_cmd_ptr);
void	scdisk_dmp_reqsns(struct scdisk_diskinfo *diskinfo,
			  struct dk_cmd  **dmp_cmd_ptr);

void	scdisk_pending_enqueue(struct scdisk_diskinfo *diskinfo, 
	struct buf *bp);
void	scdisk_pending_dequeue(struct scdisk_diskinfo *diskinfo);
void	scdisk_start_disable(struct scdisk_diskinfo *diskinfo);
void	scdisk_start(struct scdisk_diskinfo *diskinfo);
void	scdisk_iodone(struct dk_cmd *cmd_ptr);
void	scdisk_process_good(struct dk_cmd *cmd_ptr);
void	scdisk_process_sense(struct dk_cmd *cmd_ptr);
void scdisk_recover_adap_q(struct scdisk_diskinfo *diskinfo);
void	scdisk_process_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_diagnostic_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_adapter_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_scsi_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_buf_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_ioctl_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_reset_error(struct dk_cmd *cmd_ptr);
void    scdisk_fail_disk(struct scdisk_diskinfo *diskinfo);
void	scdisk_process_reqsns_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_special_error(struct dk_cmd *cmd_ptr);
void	scdisk_process_buf(struct dk_cmd *cmd_ptr);
void	scdisk_process_reset(struct dk_cmd *cmd_ptr);
void	scdisk_coalesce(struct scdisk_diskinfo *diskinfo);
struct dk_cmd	*scdisk_build_cmd(struct scdisk_diskinfo *diskinfo, 
				  struct buf **head, char good_path);
void	scdisk_write_verify(struct scdisk_diskinfo *diskinfo, 
	struct dk_cmd *prev_cmd_ptr, ulong blkno);
void	scdisk_request_sense(struct scdisk_diskinfo *diskinfo,
        struct dk_cmd *cmd_ptr);
void	scdisk_start_unit(struct scdisk_diskinfo *diskinfo, 
			  uchar start_stop_flag,
			  uchar immed_flag);
void	scdisk_start_unit_disable(struct scdisk_diskinfo *diskinfo, 
				  uchar start_stop_flag,
				  uchar immed_flag);
void	scdisk_test_unit_ready_disable(struct scdisk_diskinfo *diskinfo);
void	scdisk_test_unit_ready(struct scdisk_diskinfo *diskinfo);

void	scdisk_prevent_allow_disable(struct scdisk_diskinfo *diskinfo, uchar
				     prevent_allow_flag);
void	scdisk_prevent_allow(struct scdisk_diskinfo *diskinfo, uchar
			     prevent_allow_flag);
void	scdisk_reserve(struct scdisk_diskinfo *diskinfo);
void	scdisk_mode_sense(struct scdisk_diskinfo *diskinfo);
void 	scdisk_format_mode_data(char *mode_data,struct scdisk_mode_format *mf,
        int sense_length,char over_ride,struct scdisk_diskinfo *diskinfo);
int	scdisk_mode_data_compare(struct scdisk_diskinfo *diskinfo);
void    scdisk_q_mode(struct scdisk_diskinfo *diskinfo,char  i,char ch_index,
		     char dd_index);
void	scdisk_mode_select(struct scdisk_diskinfo *diskinfo);
void	scdisk_read_cap(struct scdisk_diskinfo *diskinfo);
void    scdisk_read_disc_info(struct scdisk_diskinfo    *diskinfo);
void	scdisk_release_disable(struct scdisk_diskinfo *diskinfo);
void	scdisk_release(struct scdisk_diskinfo *diskinfo);
void	scdisk_build_error(struct dk_cmd *cmd_ptr, int type, int priority, 
	int valid_sense);
void	scdisk_log_error(struct dk_cmd *cmd_ptr, int sev);
void	scdisk_watchdog(struct watchdog *watchdog_timer);
void	scdisk_pm_watchdog(struct watchdog *watchdog_timer);
struct cdt 	*scdisk_cdt_func(int arg);
void scdisk_q_cmd(struct  dk_cmd *cmd_ptr,char queue,uchar  which_q);
void scdisk_d_q_cmd_disable(struct dk_cmd *cmd_ptr,uchar which_q);
void scdisk_d_q_cmd(struct dk_cmd *cmd_ptr,uchar which_q);
struct  dk_cmd *scdisk_cmd_alloc_disable(struct scdisk_diskinfo *diskinfo,
					 uchar cmd_type);
struct  dk_cmd *scdisk_cmd_alloc(struct scdisk_diskinfo *diskinfo,
				 uchar cmd_type);
void scdisk_free_cmd_disable(struct dk_cmd *cmd_ptr);
void scdisk_free_cmd(struct dk_cmd *cmd_ptr);
void scdisk_sleep(struct scdisk_diskinfo *diskinfo,uchar *intrpt,uint  *event);
void scdisk_start_watchdog(struct scdisk_diskinfo *diskinfo,ulong timeout);
void scdisk_raw_iodone(struct dk_raw_buf *raw_bp);
void scdisk_trc_disable(struct scdisk_diskinfo *diskinfo, char *desc,
			char *type,char count,uint word1,uint word2,
			uint word3,uint word4,uint word5);
void scdisk_trc(struct scdisk_diskinfo *diskinfo,char *desc,char *type,
		char count,uint word1,uint word2,
		uint word3,uint word4,uint word5);
int     scdisk_pm_handler(caddr_t private, int mode);
int	scdisk_pm_bdr(struct scdisk_diskinfo *diskinfo,int *opri);
int	scdisk_pm_spindown(struct scdisk_diskinfo *diskinfo,
			   uchar immed_flag);

#else

/************************************************************************/
/* scdisk functions                                                     */
/************************************************************************/

int	scdisk_config();
int	scdisk_open();
int	scdisk_close();
int	scdisk_read();
int	scdisk_write();
int	scdisk_rdwr();
int	scdisk_mincnt();
int	scdisk_raw();
int	scdisk_raw_buffer();
int     scdisk_io_buffer();
int	scdisk_raw_io();
struct dk_cmd *scdisk_build_raw_cmd();
int	scdisk_ioctl();
int     scdisk_rdwrse();
int	scdisk_dkiocmd();
int	scdisk_dk_amr_pmr();
int	scdisk_dkeject();
int	scdisk_dkformat();
int	scdisk_dkaudio();
int	scdisk_dk_cd_mode();
int     scdisk_alloc();
void    scdisk_init_cmds();
void    scdisk_release_allow();
int	scdisk_ioctl_mode_sense();
int 	scdisk_audio_msf();
int	scdisk_audio_trk_indx();
int	scdisk_pause_resume();
int	scdisk_read_subchnl();
int     scdisk_read_toc();
int	scdisk_pin_buffer();


int	scdisk_strategy();
int	scdisk_dump();
int	scdisk_dump_write();
void    scdisk_process_dmp_error();
int	scdisk_process_dmp_sns();
void	scdisk_dmp_start_unit();
void	scdisk_dmp_reqsns();
void	scdisk_pending_enqueue();
void	scdisk_pending_dequeue();
void	scdisk_start_disable();
void	scdisk_start();
void	scdisk_iodone();
void	scdisk_process_good();
void	scdisk_process_sense();
void 	scdisk_recover_adap_q();
void	scdisk_process_error();
void	scdisk_process_diagnostic_error();
void	scdisk_process_adapter_error();
void	scdisk_process_scsi_error();
void	scdisk_process_buf_error();
void	scdisk_process_ioctl_error();
void	scdisk_process_reset_error();
void    scdisk_fail_disk();
void	scdisk_process_reqsns_error();
void	scdisk_process_special_error();
void	scdisk_process_buf();
void	scdisk_process_reset();
void	scdisk_coalesce();
struct dk_cmd	*scdisk_build_cmd();
void	scdisk_write_verify();
void	scdisk_request_sense();
void	scdisk_start_unit_disable();
void	scdisk_start_unit();
void	scdisk_test_unit_ready_disable();
void	scdisk_test_unit_ready();
void	scdisk_prevent_allow_disable();
void	scdisk_prevent_allow();
void	scdisk_reserve();
void	scdisk_mode_sense();
void 	scdisk_format_mode_data();
int 	scdisksd_mode_data_compare();
void    scdisk_q_mode();
void	scdisk_mode_select();
void	scdisk_read_cap();
void    scdisk_read_disc_info();
void	scdisk_release_disable();
void	scdisk_release();
void	scdisk_build_error();
void	scdisk_log_error();
void	scdisk_watchdog();
void	scdisk_pm_watchdog();
struct cdt	*scdisk_cdt_func();
void scdisk_q_cmd();
void scdisk_d_q_cmd();
void scdisk_d_q_cmd_disable();
struct  dk_cmd *scdisk_cmd_alloc_disable();
struct  dk_cmd *scdisk_cmd_alloc();
void scdisk_free_cmd_disable();
void scdisk_free_cmd();

void scdisk_sleep();
void scdisk_start_watchdog();
void scdisk_raw_iodone(struct buf *bp);
void scdisk_trc_disable();
void scdisk_trc();

int     scdisk_pm_handler();
int	scdisk_pm_bdr();
int	scdisk_pm_spindown();

#endif /* _NO_PROTO */
#endif /* _H_SCDISKDD */
