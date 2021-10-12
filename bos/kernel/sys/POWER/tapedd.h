/* @(#)20 1.26.2.6 src/bos/kernel/sys/POWER/tapedd.h, sysxtape, bos41J 3/29/95 17:20:42 */
#ifndef _H_TAPEDD
#define  _H_TAPEDD
#include <sys/pm.h>
/*
 * COMPONENT_NAME: (INCSYS) SCSI Tape Device Driver Include File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/************************************************************************/
/*									*/
/*  NOTE:	This header file contains the definition of the		*/
/*              structures which are passed from the tape device driver */
/*              to the SCSI adapter driver.  Also contained are         */
/*              structures passed by the application for ioctl          */
/*              execution.                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* SCSI Status Block Values                                             */
/************************************************************************/
#define TAPE_GOOD_CMPLTN     0x00
#define TAPE_CHK_CONDITION   0x02
#define TAPE_COND_GOOD       0x04
#define TAPE_BUSY            0x08
#define TAPE_IMMED_GOOD      0x10
#define TAPE_IMMED_COND_GOOD 0x14
#define TAPE_RES_CONFLICT    0x18
#define TAPE_CMD_TIMEDOUT    0xff          /* no scsi status available */

/************************************************************************/
/* X3.131 SCSI Standard Sense Key Values                                */
/************************************************************************/
#define TAPE_NO_SENSE                0x00
#define TAPE_RECOVERED_ERROR         0x01
#define TAPE_NOT_READY               0x02
#define TAPE_MEDIUM_ERROR            0x03
#define TAPE_HARDWARE_ERROR          0x04
#define TAPE_ILLEGAL_REQUEST         0x05
#define TAPE_UNIT_ATTENTION          0x06
#define TAPE_DATA_PROTECT            0x07
#define TAPE_BLANK_CHECK             0x08
#define TAPE_VENDOR_UNIQUE           0x09
#define TAPE_COPY_ABORTED            0x0A
#define TAPE_ABORTED_COMMAND         0x0B
#define TAPE_EQUAL_CMD               0x0C
#define TAPE_VOLUME_OVERFLOW         0x0D
#define TAPE_MISCOMPARE              0x0E

/************************************************************************/
/* Misc. Defines                                                        */
/************************************************************************/
#define TAPE_MAX_RETRY       4          /* maximum error retry count */
#define TAPE_MAXREQUEST      262144     /* maximum transfer allowed  */
#define DEVNO_OVERLAY        0x1FFF     /* overlay for clear dev flag*/
#define TAPE_NOREW_ON_CLOSE  0x01       /* bit check for no rewind   */
#define TAPE_RETEN_ON_OPEN   0x02       /* bit check for retention   */
#define TAPE_DENSITY2        0x04       /* chk for density 2 setting */
#define TAPE_REVERSE         0x01       /* tape motion subroutine flg*/
#define TAPE_FORWARD         0x02       /* tape motion subroutine flg*/
#define TAPE_RECORD          0x01       /* tape motion subroutine flg*/
#define TAPE_FILEMARK        0x02       /* tape motion subroutine flg*/
#define TAPE_WAKEUP          0x01       /* tape iodone subroutine flg*/
#define TAPE_NOWAKEUP        0x02       /* tape iodone subroutine flg*/
#define TAPE_ADAPTER_ERROR   0x01       /* flag used by error proc.  */
#define TAPE_SCSI_ERROR      0x02       /* flag used by error proc.  */
#define TAPE_NOBUFF          0x01       /* device buffers not used   */
#define TAPE_BUFFERED        0x02       /* use device buffers for wrt*/

/************************************************************************/
/* Structure used for power management support                          */
/************************************************************************/
struct tape_pm_handle {
	struct pm_handle        pmh;      /* power management structure */
	struct tape_device_df   *device_ptr;
	int    pm_device_id;              /* PM device id               */
};

/************************************************************************/
/* Structure used for watchdog timer support                            */
/************************************************************************/
struct tape_watchdog {
	struct watchdog         watch_timer; /* watchdog timer structure */
	struct tape_device_df   *device_ptr;
};
/************************************************************************/
/* Error information thresholds for tapes                               */
/************************************************************************/
struct tape_error_df { /* error count fields (see below for error type) */
	uint            write_recovered_error;
	uint            read_recovered_error;
	uint            medium_error;
	uint            hardware_error;
	uint            aborted_cmd_error;
};

/************************************************************************/
/* Initialization information on individual tapes                       */
/************************************************************************/
struct tape_ddi_df  {
	uchar           resource_name[8];  /* name for this device    */
	dev_t           adapter_devno;  /* adapter major/minor number */
	uchar           scsi_id;                /* SCSI Id for tape   */
	uchar           lun_id;                 /* SCSI LUN for tape  */
	uchar           mode;                   /* mode for tape      */
	uchar           dev_type;               /* device type        */
#define TAPE_8MM            0x01
#define TAPE_9TRACK         0x02
#define TAPE_QUARTER        0x03
#define TAPE_OTHER          0x04
#define TAPE_8MM5GB         0x05
#define TAPE_QIC525         0x06
#define TAPE_QIC1200        0x07
#define TAPE_4MM2GB         0x08
#define TAPE_3490E          0x0a
#define TAPE_4MM4GB         0x0b
						/* filemark with var. */
	uchar           ecc_flag;               /* reserved           */
	uchar           extend_filemarks;       /* use long or short  */
	uchar           retention_flag;         /* retention on reset */
	uchar           res_sup;                /* res/rel cmds ok    */
	uchar           density_set1;           /* setting for density*/
	uchar           density_set2;           /* setting for density*/
	uchar           reserved;               /* reserved           */
	uchar           compression_flag;       /* turn compression on*/
	uchar           autoloader_flag;        /* turn autoloader on */
	uint            var_blocksize;          /* variable blocksize */
	uint            blocksize;              /* blocksize for tape */
	uint            min_read_error;         /* min. count for log.*/
						/* recovered errors   */
	uint            min_write_error;        /* min. count for log.*/
						/* recovered errors   */
	uint            read_ratio;             /* for rec. errors    */
	uint            write_ratio;            /* for rec. errors    */
	uint            mode_data_length;       /* length of mode data*/
	uint		delay;			/* retry delay (ost)  */
	uint		readwrite;		/* timeout value (ost)*/
	uchar           mode_select_data[256];  /* mode select data   */
	struct		tape_pm_handle tape_pm_ptr; /* for tape power */ 
						/* management handler */
};

/************************************************************************/
/* Driver Operation descriptor block (includes sc_buf)                  */
/************************************************************************/
struct tape_cmd {
	struct sc_buf           scbuf;
	uchar                   flags;         /* operation flags        */
#define TAPECMD_BUSY      0x01
	uchar                   retry_flag;    /* retry this command?    */
	uchar                   retry_count;   /* number of retrys to do */
	uchar                   tape_position;
#define TAPE_FILEMARK_ERROR  0x01              /* filemark encounterd    */
#define TAPE_ENDOFTAPE_ERROR 0x02              /* EOT encountered        */
#define TAPE_BEGOFTAPE_ERROR 0x04              /* BOT encountered        */
#define TAPE_OVERFLOW_ERROR  0x08              /* buffer write failed    */
#define TAPE_ILI_ERROR       0x10              /* illegal length error   */
#define TAPE_ENDOFDATA_ERROR 0x20              /* end of data on read    */
	uchar                   type;
#define TAPE_RESET_CMD       0x01              /* device/bus reset occur */
#define TAPE_REQSENSE_CMD    0x02              /* req. sense info avail. */
#define TAPE_DIAG_CMD        0x03              /* diag cmd in progress   */
#define TAPE_OTHER_CMD       0x04              /* general cmd in progress*/
#define TAPE_SPECIAL_CMD     0x05              /* special cmd - no retry */
	uchar                   last_state;     /* Last state for error. */
	struct tape_device_df   *device_ptr;
};

/************************************************************************/
/* Request Sense Data Block                                             */
/************************************************************************/
struct req_sense_info  {            /* defined by device specification */
       uchar          err_code;
       uchar          rsvd0;
       uchar          sense_key;
       uchar          sense_byte0;
       uchar          sense_byte1;
       uchar          sense_byte2;
       uchar          sense_byte3;
       uchar          add_sense_length;
       uchar          add_sense_byte0;
       uchar          add_sense_byte1;
       uchar          add_sense_byte2;
       uchar          add_sense_byte3;
       uchar          add_sense_key;
       uchar          extended_byte1;
       uchar          extended_byte2;
       uchar          extended_byte3;
       uchar          extended_byte4;
       uchar          extended_byte5;
       uchar          extended_byte6;
       uchar          extended_byte7;
       uchar          extended_byte8;
       uchar          extended_byte9;
       uchar          extended_byte10;
       uchar          extended_byte11;
       uchar          extended_byte12;
       uchar          extended_byte13;
       uchar          extended_byte14;
       uchar          extended_byte15;
       uchar          extended_byte16;
       uchar          rsvd8[228];
};

/************************************************************************/
/* Information on individual tapes                                      */
/************************************************************************/
struct tape_device_df {
	dev_t           devno;  /* What device number           */
	uchar           mode;   /* Current op mode of device    */
#define TAPE_DIAGNOSTIC 0x01    /* Diagnostic mode for device   */
#define TAPE_NORMAL     0x02    /* Normal mode for device       */

	uchar           cmd_state;      /* Current device state */
#define TAPE_TUR          0x01          /* test unit ready in progress */
#define TAPE_LOAD         0x02          /* load command in progress */
#define TAPE_RESERVE      0x03          /* reserve in progress */
#define TAPE_RELEASE      0x04          /* release in progress */
#define TAPE_SENSE        0x05          /* mode sense in progress */
#define TAPE_SELECT       0x06          /* mode select in progress */
#define TAPE_IO           0x07          /* read/write in progress */
#define TAPE_IOCTL        0x08          /* ioctl call in progress */
#define TAPE_REQ_SENSE    0x09          /* request sense in progress */
#define TAPE_GENERAL      0x0A          /* other command in progress */
#define TAPE_SIGNALRCV    0x0B          /* signal rcvd to halt command */

	uchar              flags;       /* flags for tape operation     */
#define TAPE_FLAG_OVERLAY        0x04   /* overlay to clear on reverse  */
#define TAPE_DUMP_DEVICE         0x01   /* tape is inited as a dump dev */
#define TAPE_OPENING_DEVICE      0x02   /* tape is being opened         */
#define TAPE_READ_ONLY           0x04   /* tape is read only            */
#define TAPE_OFFLINE             0x08   /* tape has failed, no more cmds*/
#define TAPE_FILEMARK_DETECT     0x10   /* a filemark has been detected */
#define TAPE_ENDOFTAPE_DETECT    0x20   /* end of tape encountered      */
#define TAPE_REQSENSE_AVAIL      0x40   /* alternate buffer command out */
	uchar              flags2;      /* flags for tape operation     */
#define TAPE_LOAD_REQUIRED       0x01   /* unit attention rec. on open  */
#define TAPE_RESET_PARAMS        0x02   /* a reset of ecc and blocksize */
#define TAPE_LOADER_READY        0x04   /* tape is available in 3490e 	*/
#define TAPE_SHORTREAD_OK        0x08   /* var-block read < blk len ok  */
#define TAPE_NOSOFT_CHK          0x10   /* var-block read < blk len ok  */

	uchar              operation;   /* what action tape did last    */
#define TAPE_READ_ACTIVE         0x01   /* tape is currently reading    */
#define TAPE_WRITE_ACTIVE        0x02   /* tape is currently writing    */

	uchar              opened;      /* This tape is open            */
	uchar              powered_off; /* This tape is powered off     */
	uchar              open_mode;   /* mode this tape was opened for*/
	uchar              last_state;  /* Last state for error.        */
	uchar              async_flag;  /* do/don't use asynch data xfer*/
	uchar              sense_flag;  /* used for mode sense process  */
#define TAPE_SENSE_A             0x00   /* other scsi tape mode sense   */
#define TAPE_SENSE_B             0x01   /* other scsi tape mode sense   */
#define TAPE_SENSE_C             0x03   /* other scsi tape mode sense   */
#define TAPE_SENSE_D             0x04   /* other scsi tape mode sense   */
	uchar              mode_buf[255];      /* mode select info.     */
	uchar              save_ecc;    /* save area for configured ecc */
	uchar              tape_previously_open; /* special open flag   */
	uchar              filemark_save;  /* when filemarks are hit    */
	uchar              tape_page_supsen;  /* page format require    */
	uchar              tape_page_supsel;  /* page format require    */
	uchar              bytes_requested;   /* bytes for mode sense   */
	uchar              selection_set;     /* flag for mode params   */
	int                save_blocksize; /* save area for config. blk */
	int                max_xfer_size;/* maximun xfer size allowed   */
	uint               write_xfer_count;  /* count of bytes written */
	uint               write_block_count; /* count of blocks written*/
	uint               write_resid;       /* count of 1K blocks written*/
	uint               read_xfer_count;   /* count of bytes read    */
	uint               read_block_count;  /* count of blocks read   */
	uint               read_resid;        /* count of 1K blocks read*/
	uint               stackhead;
	int                initial_tape_blocks; /* count of blocks avail-*/
						/* able at open time     */
	struct file             *fp;    /* file pointer for scsi DD     */
	struct tape_cmd         *stack_ptr[4];
	struct req_sense_info   req_sense_buf;
	struct tape_ddi_df      tape_ddi;   /* ddi info for with device  */
	struct tape_error_df    tape_error; /* error threshold counters. */
	struct tape_watchdog    tape_watchdog_ptr;  /* for watchdog timer*/
	struct tape_cmd    cmd_1_ptr;         /* scsi command info       */
	struct tape_cmd    cmd_2_ptr;         /* scsi command info       */
	struct tape_cmd    reset_cmd_ptr;     /* reset command info      */
	struct tape_cmd    reqsense_cmd_ptr;  /* request sense cmd info. */
	struct tape_cmd    *cmd_outstanding;  /* waiting command         */
	uint            lock_word;            /* device lock word        */
#ifdef _POWER_MP
	Simple_lock 	intr_lock;            /* interrupt-thread lock   */
#endif /* _POWER_MP */
};
#endif  /* _H_TAPEDD */


