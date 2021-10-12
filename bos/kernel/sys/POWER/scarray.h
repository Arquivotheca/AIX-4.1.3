/* @(#)59       1.2  src/bos/kernel/sys/POWER/scarray.h, sysxarray, bos41J, 9511A_all 3/11/95 15:09:14 */
#ifndef _H_SCARRAY
#define _H_SCARRAY
/*
 * COMPONENT_NAME: SYSXARRAY 
 *
 * FUNCTIONS:  NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* scarray.h header dependencies                                         */
/************************************************************************/

#include <sys/types.h>
#include <sys/scsi.h>

/* SCSI Disk Ioctls */
#define DKIORDSE	0x01	/* Read and return sense data on error  */
#define DKIOWRSE	0x02	/* Write and return sense data on error */
#define DKIOCMD		0x03	/* Issue a user-defined SCSI command    */
#define DKIOCMD_RS      0x34	/* Issue user-defined SCSI command with */
				/* automatic request sense.		*/
#define DKIOCMD_PTRS    0x35	/* Issue user-defined SCSI command with */
				/* automatic request sense for pass	*/
				/* through.				*/
#define DKDRVGROUP      0x36	/* Issue command to set a drive group   */
#define DKIOCMD_RS2     0x37    /* Issue user-defined SCSI command with */
                                /* automatic request sense.             */
#define DKIOCMD_PTRS2   0x38    /* Issue user-defined SCSI command with */
                                /* automatic request sense for pass     */
                                /* through.                             */

/* Router specific Ioctls */
#define RT_GET_ROUTER_INFO			0x100
#define RT_GET_EXTENDED_ROUTER_INFO		0x101
#define RT_GET_VERSION				0x102
#define RT_GET_ADAPTER_LIST			0x103
#define RT_ADD_PATH				0x104
#define RT_GET_ROUTER_STATE			0x105
#define RT_SET_ROUTER_STATE			0x106
#define RT_FAIL_ADAPTER				0x107
#define RT_FAIL_CONTROLLER			0x108
#define RT_RESTORE_ADAPTER			0x109
#define RT_RESTORE_CONTROLLER			0x10a
#define RT_PREVENT_SWITCHING			0x10b
#define RT_ALLOW_SWITCHING			0x10c
#define RT_START_MONITOR			0x10d



#define ERRID_DISK_ARRAY_ERR1 0x6f3ebf52 /* Physical volume media error */
#define ERRID_DISK_ARRAY_ERR2 0xf19115c9 /* Physical volume hardware error */
#define ERRID_DISK_ARRAY_ERR3 0x52506ecd /* Adapt-detected physical vol fail */
#define ERRID_DISK_ARRAY_ERR4 0x0ae077c2 /* Physical volume recovered error */
#define ERRID_DISK_ARRAY_ERR5 0xf104f339 /* Unknown physical volume failure */
#define ERRID_DISK_ARRAY_ERR6 0x66ab702e /* Subsystem component failure */
#define ERRID_DISK_ARRAY_ERR7 0xa2087390 /* Passive ctrl healthcheck error */
#define ERRID_DISK_ARRAY_ERR8 0x117a9f88 /* Array active controller switch */
#define ERRID_DISK_ARRAY_ERR9 0xb8d1d759 /* Array controller switch failure */
#define ERRID_DISK_ARRAY_ERR10 0x912cc9e6 /* Array configuration changed */
#define ERRID_DISK_ARRAY_ERR11 0xa5900f34 /* Array drive type failure */
#define ERRID_DISK_ARRAY_ERR12 0x13675dbe /* Array polled status failure */
#define ERRID_DISK_ARRAY_ERR13 0x61dea57f /* Array ICON chip failure */
#define ERRID_DISK_ARRAY_ERR14 0xd65128ef /* Array drive failure */




/* New errno values */
#define EDRV                    160
#define EICON                   161

/* handy stuff for interpreting SCSI data */
struct scsi_short {
    unsigned char hi;
    unsigned char lo;
};
struct scsi_long {
    unsigned char hi;
    unsigned char mhi;
    unsigned char mlo;
    unsigned char lo;
};

#define FROM_SCSI_SHORT(x)      (((x).hi << 8) + ((x).lo))
#define FROM_SCSI_LONG(x)      (((x).hi << 24) + ((x).mhi << 16) + ((x).mlo << 8) + ((x).lo))

/************************************************************************/
/* Mode Select page information				                */
/************************************************************************/

/* header returned from a 10-byte MODE SENSE */
struct scsi_10byte_mode_header {
    struct scsi_short   data_length;
    unsigned char       medium_type;
    unsigned char       device_specific;
    unsigned char       reserved[2];
    struct scsi_short   block_desc_length;
};

/* Page 0x2a */
struct array_physical_page {
    unsigned char       page_code;
    unsigned char       page_length;
    unsigned char	drive_status[16][15];
};

/* Page 0x2b */
struct logical_array_page {
    unsigned char       page_code;
    unsigned char       page_length;
    unsigned char       lun_status;
    unsigned char       raid_level;
    unsigned char       lun_type;
    struct scsi_long    lun_blocksize;
    struct scsi_short   drive_sector_size;
    struct scsi_long    num_of_blocks;
    struct scsi_long    avail_capacity;
    struct scsi_long    segment_size;
    unsigned char       segment0_size;
    struct scsi_short   lun_flags;
    struct scsi_long    recon_block_completed;
    unsigned char       recon_frequency;
    struct scsi_short   recon_amount;
    unsigned char       reserved[3];
    unsigned char       disk_bit_map[32];
    unsigned char       config_table[64];
};

/* Page 0x2c */
struct redundant_controller_page {
    unsigned char       page_code;
    unsigned char       page_length;
    unsigned char       primary_id[16];
    unsigned char       alternate_id[16];
    unsigned char       alt_present1;
    unsigned char       primary_mode;
    unsigned char       alt_present2;
    unsigned char       alternate_mode;
    unsigned char	quiescence_timeout;
    unsigned char	rdac_options;	
    unsigned char       lun_table[64];
    unsigned char       reserved2[2];
};

/************************************************************************/
/* Information on routers                                               */
/************************************************************************/
struct path_desc {
        dev_t           adapter_devno;  /* devno of the SCSI adapter    */
                                        /* device on this path.         */
        dev_t           ctrl_devno;     /* devno of the controller      */
                                        /* device on this path.         */
        uchar           ctrl_scsi_id;   /* SCSI id of the controller on */
                                        /* this path.                   */
        char            ctrl_name[8];   /* Name of the controller on    */
                                        /* this path.                   */
};

struct router_ioctl_state {
        uchar           state;          /* State desired: Dual Active,  */
                                        /* Active/Passive, etc.         */
#define DUAL_ACTIVE             0x00
#define ACTIVE_PASSIVE          0x01
#define ACTIVE_RESET            0x02
#define PASSIVE_ACTIVE          0x10
#define RESET_ACTIVE            0x20

#define CTRL0_MASK              0xf0
#define CTRL1_MASK              0x0f
#define CTRL_IN_RESET           0x2
#define CTRL_PASSIVE            0x1
#define CTRL_ACTIVE             0x0

        uint            hlthchk_freq;   /* Intervals between health-    */
                                        /* checks, in seconds           */
        uint            aen_freq;       /* Intervals between aen        */
                                        /* polling, in seconds          */
        uint            balance_freq;   /* Intervals between load       */
                                        /* balancing, in seconds        */
        uint            lun_ownership;  /* Bitmap of LUN ownership      */
                                        /* desired. Bit 0 is for LUN 0, */
                                        /* etc. A zero means the LUN is */
                                        /* assigned to the even SCSI id */
                                        /* controller.                  */
        uchar           load_balancing; /* TRUE if router will do dyna- */
                                        /* mic load balancing.          */
        struct path_desc ctrl_path[2];  /* Path for each controller in  */
                                        /* this router.                 */
};

/************************************************************************/
/* sc_iocmd_rs structure for DKIOCMD_RS                                 */
/************************************************************************/
struct sc_iocmd_rs {
        uint   data_length;          /* Bytes of data to be transfered */
        char   *buffer;              /* Pointer to transfer data buffer */
        uint   timeout_value;        /* In seconds */
        uchar  status_validity;      /* 0 = no valid status */
        /* 1 = valid SCSI bus status only       */
        /* 2 = valid adapter status only        */
        uchar  scsi_bus_status;      /* SCSI bus status (if valid) */
        uchar  adapter_status;       /* Adapter status (if valid), refer to
                                        sc_buf definition */
        uchar  resvd1;               /* reserved, should be set to zero */
        uchar  resvd2;               /* reserved, should be set to zero */
        uchar  adap_q_status;        /* used to return back whether or  */
                                     /* or not the SCSI adapter driver  */
                                     /* and SCSI adapter cleared their  */
                                     /* queue for this device or not.   */
                                     /* A value of zero implies that    */
                                     /* the device's queue at the       */
                                     /* adapter is cleared.             */
                                     /* A value of SC_DID_NOT_CLEAR_Q,  */
                                     /* defined in sc_buf, implies that */
                                     /* the device's queue at the       */
                                     /* adapter has not be cleared.     */
        uchar  q_tag_msg;            /* Used to pass down Queue Tag     */
                                     /* message fields of SC_NO_Q,      */
                                     /* SC_SIMPLE, SC_HEAD_OF_Q,        */
                                     /* SC_ORDERED_Q defined above in   */
                                     /* sc_buf's definition.            */
        uchar  flags;                /* SC_NODISC, SC_ASYNC, B_READ, B_WRITE */

        uchar  resvd5;               /* Used to tell the SCSI adapter   */
                                     /* driver, and SCSI adapter whether*/
                                     /* or not it should clear or resume*/
                                     /* its queue. This is done via the */
                                     /* defines SC_Q_CLR, SC_Q_RESUME   */
                                     /* defined above in sc_buf.        */
#ifdef _THREADS
	uchar  lun;		     /* This is the target's lun.  If   */
				     /* the  target's lun is greater    */
                                     /* than 7, this field must be used */
                                     /* and the 3 lun bits (used a lun  */
                                     /* in SCSI-1) in the scsi_command  */
                                     /* will be ignored.                */
#else
        uchar  resvd6;               /* reserved, should be set to zero */
#endif
        uchar  resvd7;               /* reserved, should be set to zero */


        uchar  command_length;       /* Length of SCSI command block */
        uchar  scsi_cdb[12];         /* SCSI command descriptor block */
        uchar   req_sense_length;    /* Length of request sense data  */
				     /*	buffer in bytes 	      */
        char    *request_sense_ptr;  /* Pointer to request sense buffer */
};


/************************************************************************/
/* sc_iocmd_ptrs structure for DKIOCMD_PTRS                             */
/************************************************************************/
struct sc_iocmd_ptrs {
        uint   data_length;          /* Bytes of data to be transfered */
        char   *buffer;              /* Pointer to transfer data buffer */
        uint   timeout_value;        /* In seconds */
        uchar  status_validity;      /* 0 = no valid status */
        /* 1 = valid SCSI bus status only       */
        /* 2 = valid adapter status only        */
        uchar  scsi_bus_status;      /* SCSI bus status (if valid) */
        uchar  adapter_status;       /* Adapter status (if valid), refer to
                                        sc_buf definition */
        uchar  resvd1;               /* reserved, should be set to zero */
        uchar  resvd2;               /* reserved, should be set to zero */
        uchar  adap_q_status;        /* used to return back whether or  */
                                     /* or not the SCSI adapter driver  */
                                     /* and SCSI adapter cleared their  */
                                     /* queue for this device or not.   */
                                     /* A value of zero implies that    */
                                     /* the device's queue at the       */
                                     /* adapter is cleared.             */
                                     /* A value of SC_DID_NOT_CLEAR_Q,  */
                                     /* defined in sc_buf, implies that */
                                     /* the device's queue at the       */
                                     /* adapter has not be cleared.     */
        uchar  q_tag_msg;            /* Used to pass down Queue Tag     */
                                     /* message fields of SC_NO_Q,      */
                                     /* SC_SIMPLE, SC_HEAD_OF_Q,        */
                                     /* SC_ORDERED_Q defined above in   */
                                     /* sc_buf's definition.            */
        uchar  flags;                /* SC_NODISC, SC_ASYNC, B_READ, B_WRITE */
        uchar  resvd5;               /* Used to tell the SCSI adapter   */
                                     /* driver, and SCSI adapter whether*/
                                     /* or not it should clear or resume*/
                                     /* its queue. This is done via the */
                                     /* defines SC_Q_CLR, SC_Q_RESUME   */
                                     /* defined above in sc_buf.        */
#ifdef _THREADS
	uchar  lun;		     /* This is the target's lun.  If   */
				     /* the  target's lun is greater    */
                                     /* than 7, this field must be used */
                                     /* and the 3 lun bits (used a lun  */
                                     /* in SCSI-1) in the scsi_command  */
                                     /* will be ignored.                */
#else
        uchar  resvd6;               /* reserved, should be set to zero */
#endif
        uchar  resvd7;               /* reserved, should be set to zero */


        uchar  command_length;       /* Length of SCSI command block */
        uchar  scsi_cdb[12];         /* SCSI command descriptor block */
        uchar   req_sense_length;    /* Length of request sense data buffer
                                           in bytes */
        char    *request_sense_ptr;  /* Pointer to request sense buffer */
	uchar	passthru_status;	/* Status of this command	*/
#define NO_ERR			0x00
#define ERR_ON_PASS_MODE	0x01
#define ERR_ON_CMD		0x02
	uchar	passthru_direction;	/* Direction bits for the Set	*/
					/* Pass Thru Mode command. 	*/
#define NO_DATA_TRANSFER	0x01
#define INITIATOR_TO_TARGET	0x02
#define TARGET_TO_INITIATOR	0x03
	uchar	dest_channel;		/* The channel number to select	*/
					/* the drive			*/
	uchar	dest_scsi_id;		/* The SCSI id to select the	*/
					/* drive.			*/
};

/************************************************************************/
/* Initialization information on individual arrays                       */
/************************************************************************/
struct array_ddi  {
	char	resource_name[8];	/* resource name logged with  	*/
					/* each error log entry.        */
	dev_t	adapter_devno;		/* SCSI adapter device driver   */
					/* major/minor num 		*/
	uchar	scsi_id;		/* SCSI ID for array             */
	uchar	lun_id;			/* SCSI LUN for array            */
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
	uchar	q_type;			/* The queuing type of the array */
					/* These are defined in scsi.h  */
					/* for resvd8 of sc_buf		*/
	uchar   q_err_value;	       	/* TRUE if Qerr bit should be   */
					/* set.	Otherwise FALSE.	*/
	uchar   clr_q_on_error;	       	/* TRUE if device clears its    */
					/* queue on error.  Otherwise   */
					/* FALSE.			*/
	uchar   buffer_ratio;           /* read/write data buffer ratio */
					/* of SCSI arrays to be filled in*/
					/* for page 2 mode select data  */
	uchar   cmd_tag_q;		/* if device supports command   */
					/* tag queuing	this is TRUE	*/
					/* otherwise it is FALSE	*/

	uchar	 reserve_lock;		/* If true, open sequence will 	*/
					/* include a reserve cmd. Else	*/
					/* no reserve issued to device  */

	ushort   queue_depth;		/* maximum number of commands   */
					/* that can be queued up to the */
					/* SCSI adpater device driver   */
					/* for a given device.		*/
	int	recovery_limit;		/* Max allowable recovery level */
					/* before a device recovered    */
					/* error is reported to be      */
					/* re-assigned. 		*/

	uchar	dev_type;		/* Is this devno a controller, 	*/
					/* an array, or a router.	*/

#define ARRAY_DEVICE		0x00
#define CONTROLLER_DEVICE	0x01
#define ROUTER_DEVICE		0x02

	uchar	ctrl_type;		/* Which type of controller 	*/

#define INF_CONTROLLER		0x00
#define ADP_CONTROLLER		0x01

	ushort	rw_timeout;		/* Timeout values in secs for	*/
					/* read and write commands.	*/
	ushort	reassign_timeout;	/* Timeout values in secs for	*/
					/* re-assign commands.		*/

	uint	segment_size;		/* # bytes in sample rate       */
	uint	segment_cnt;		/* # of segments read from this */
					/* device 			*/
	uint	byte_count;		/* # bytes read beyond the      */
					/* number of segments		*/
	uint	max_coalesce;		/* Max number of bytes to be 	*/
					/* coalesced.			*/
	uint    max_request;		/* maximum request size for     */
					/* a SCSI array			*/
	int	mode_data_length;	/* mode select data length      */
	char	mode_data[256];		/* mode select data buffer      */
	int	mode_default_length;	/* mode select default data lng */
	char	mode_default_data[256];	/* mode select default data     */
};
#endif /* _H_SCARRAY */
