/* @(#)11	1.19.1.10  src/bos/kernel/sys/POWER/sd.h, sysxdisk, bos411, 9428A410j 3/16/94 10:36:52 */
#ifndef _H_SD
#define _H_SD
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem Device Driver
 *
 * FUNCTIONS:  Header File for Serial Dasd Subsytem Device Driver
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/serdasd.h>
#include <sys/adspace.h>
#include <sys/buf.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/pin.h>
#include <sys/iostat.h>
#include <sys/scsi.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>

/*---------------------------------------------------------------------------*

                          SERIAL DASD SUBSYSTEM DEFINES

 *---------------------------------------------------------------------------*/

#ifdef	SD_3_1_BASE
	/*
	 * If this is being compiled for a 3.1 base system, then
	 * define these 3.2 specific flags 
 	 */
#define DMA_NOHIDE	0x00
#define B_SPLIT		0x00
#endif
/*
 *   Defines for downloading of one common microcode file
 */
#define SD_ADAP_OFFSET          200     /* Offset of adapter ucode in ucode  */
                                        /* file                              */
#define SD_CTRL_OFFSET          204     /* Offset of controller ucode in     */
                                        /* ucode file                        */
#define SD_HEADER_SIZE          4       /* Size of header in ucode file      */

/*
 * Hash Table sizes
 */
#define SD_ADAP_TBL_SIZE        8       /* Adapter's hash table size         */
#define SD_CTRL_TBL_SIZE        64      /* Controller's hash table size      */
#define SD_DASD_TBL_SIZE        128     /* Dasd's hash table size            */

/*
 * defines for compenent dump table
 */
#define SD_NUM_ADAP_ENTRIES     4       /* Number static entries for adapter */
#define SD_NUM_CTRL_ENTRIES     3       /* Number static entries for         */
                                        /* controller                        */
#define SD_NUM_DASD_ENTRIES     3       /* Number static entries for DASD    */

/*
 * Various Size Defines
 */

#define SD_NUM_MB               128     /* number of mailboxes for one adap  */
#define SD_NUM_CTRLS            8       /* number of controllers per adap    */
#define SD_NUM_DASD             16      /* number of dasd per controller     */
#define SD_WORD                 16      /* 16 bit word                       */
#define SD_BYTE                 8       /* 8 bits per byte                   */
#define SD_MAX_EVENTS          	128	/* Allow for 128 events per adapter  */ 
                                        /* max number of events for this adap*/
#define SD_MAX_RESTARTS         10      /* max times to restart a DASD       */
#define SD_TRY_RESET      	SD_MAX_RESTARTS - 3  /* point to try reset   */
#define SD_BPS                  512     /* Bytes per sector                  */
#define SD_MB_SIZE              32      /* number of bytes in one mailbox    */
#define SD_CMPL_REG_SIZE        4       /* number of bytes in completion reg */
#define SD_NUM_EVENT_WORDS      4       /* number of 32 bit words in freelist*/
#define SD_NUM_CMD_WORDS        4       /* number of 32 bit words in freelist*/
#define SD_NUM_MB_WORDS         4       /* number of 32 bit words in freelist*/
#define SD_BITS_PER_WORD        32      /* number of bits per long word      */
#define SD_DASD_MAX_TRANSFER    0x20000 /* maximum DASD xfer = 128K          */
#define SD_MAX_QUERIES		3	/* max query devs before reset ctrl  */
/*
 * DMA defines
 */
#define SD_DMA_INIT             MICRO_CHANNEL_DMA /* DMA init flags          */
#define SD_DMA_TYPE             0       /* DMA master/complete flags         */
/*
 * Interrupt Defines
 */
#define SD_INT_ENABLE           0x80    /* enable interrupt bit Control Reg  */
#define SD_INT_DISABLE          0x00    /* disable interrupt bit Control Reg */
#define SD_IRPT                 0x40    /* host interrupt bit Control Reg    */
/*
 * POS Register bits
 */
#define SD_ADAP_RESET_BIT       0x02    /* POS 2 adapter reset 2             */
#define SD_CARD_ENABLE          0x01    /* POS 2 Card enable bit             */
#define SD_IOSPACE_MASK         0x00fc  /* POS 2 mask for io address         */
#define SD_ILVL_MASK            0x70    /* POS 3 mask for interrupt level    */
#define SD_DMA_ENABLE           0x80    /* POS 4 DMA enable bit              */
#define SD_FAIRNESS_ENABLE      0x40    /* POS 4 Fairness enable bit         */
#define SD_STREAM_DISABLE       0x20    /* POS 4 disables streaming          */
#define SD_STREAM_ENABLE        0x00    /* POS 4 enables streaming           */
#define SD_MPXSTRM_DISABLE      0x10    /* POS 4 disables MPX streaming      */
#define SD_MPXSTRM_ENABLE       0x00    /* POS 4 enables MPX streaming       */
#define SD_MONITOR              0x01    /* POS 4 monitor SFDBKRTN            */
#define SD_PARITY_DISABLE       0x02    /* POS 4 disable parity              */
#define SD_PARITY_ENABLE        0x00    /* POS 4 enable parity               */
#define SD_DMA_MODE_3           0x0C    /* POS 4 DMA mode 3 bits             */
#define SD_ADAP_TYPE_1          0x20    /* POS 5 adapter type 1 error        */
#define SD_W_DATA_PARITY_ERR    0x08    /* POS 5 write data parity error     */
#define SD_INVALID_ACCESS       0x10    /* POS 5 invalid access              */
#define SD_INVALID_STATUS       0x04    /* POS 5 invalid status              */
/*
 * TCW Management defines
 */
#define SD_TCWSIZE              DMA_PSIZE  /* system TCW size                */
#define SD_STA_SIZE             256     /* size of a small transfer area     */
#define SD_MBA_ALLOC_SIZE       PAGESIZE /* size to allocate for MBA         */
#define SD_NUM_MBA_TCWS         (SD_MBA_ALLOC_SIZE/SD_TCWSIZE)
#define SD_STA_ALLOC_SIZE       PAGESIZE*2  /* size to allocate for STA      */
#define SD_NUM_STA_TCWS         (SD_STA_ALLOC_SIZE/SD_TCWSIZE)
#define SD_NUM_STA              (SD_STA_ALLOC_SIZE/SD_STA_SIZE)
/*
 * Various Defines
 */
#define SD_LUNDEV               0x00    /* device address --> LUN            */
#define SD_TARDEV               0x10    /* device address --> CTLR           */
#define SD_QUEUE                0x01    /* flag to queue this command        */
#define SD_STACK                0x00    /* flag to stack this command        */
#define SD_NO_MORE_TAGS         0       /* no more tags in completion reg    */
#define SD_CMPL_ERROR           0xFF    /* Error indicated in completion reg */
#define SD_ALRT_ERROR           0xFF    /* Error indicated in Alert Register */
#define SD_ALRT_NO_ERR          0       /* No error indicated in alert Reg   */
#define SD_DELAY_REG            0xE0    /*  Delay register                   */
#define SD_MIN_TAG              0x01    /* smallest valid command tag        */
#define SD_MAX_TAG              0x7F    /* largest valid command tag         */
#define SD_NOT_USED             1   	/* flag whether mailbox never used   */
#define SD_USED             	0   	/* flag whether mailbox used   	     */
#define VOLATILE             	1   	/* flag register access is volatile  */
#define NON_VOLATILE          	0   	/* flag register access not volatile */
#define SD_TRY_ANOTHER_PASS 	0   	/* rc from startcmd to try     again */
#define SD_FORCE_ANOTHER_PASS 	1   	/* rc from startcmd to force   again */
#define SD_NOT_AGAIN 		2   	/* rc from startcmd not to try again */
#define SD_FENCE_NONE           0       /* Fence mask for no hosts           */
#define SD_FENCE_ALL            0xFFFF  /* Fence mask for all possible hosts */
/*
 * Error Recover Procedure Definitions
 */
#define SD_NONE                 0xE0    /* No Error Recovery Needed          */
#define SD_RESET_A              0xE1    /* Reset Adapter                     */
#define SD_QUIESCE_A            0xE2    /* Quiesce Adapter                   */
#define SD_RESET_C              0xE3    /* Reset Controller                  */
#define SD_QUIESCE_C            0xE4    /* Quiesce Controller                */
#define SD_RESET_D              0xE5    /* Reset DASD                        */
#define SD_QUIESCE_D            0xE6    /* Quiesce DASD                      */
#define SD_DELAY_RETRY          0xE7    /* Delay and then retry              */
#define SD_RESET_C_BM           0xE8    /* Reset Controllers in bit map      */
#define SD_FAIL_C               0xE9    /* Fail This Controller              */
#define SD_SCSI_ERP             0xEA    /* Perform SCSI Error Recovery       */
#define SD_SENSE_QUIESCE_C      0xEB    /* Perform Sense and quiesce ctrl    */
/*
 * Error Type defines for Error log thresholding
 */
#define E_TYPE_A1		0	/* Adapter error type 1	             */
#define E_TYPE_A2		1	/* Adapter error type 2	             */
#define E_TYPE_A3		2	/* Adapter error type 3	             */
#define E_TYPE_A4		3	/* Adapter error type 4	             */
#define E_TYPE_M1		0	/* Microcode error type 1            */
#define E_TYPE_C1		1	/* Controller error type 1	     */
#define E_TYPE_C2		2	/* Controller error type 2	     */
#define E_TYPE_C3		3	/* Controller error type 3	     */
#define E_TYPE_D2		0	/* DASD error type 2	             */
#define E_TYPE_D4		1	/* DASD error type 4	             */
#define FORCED_LOG		0x7f 	/* Force the log, for media type errs*/
#define SD_ADAP_ETYPES		4 	/* Number of Adapter error types     */
#define SD_CTRL_ETYPES		4 	/* Number of Controller error types  */
#define SD_DASD_ETYPES		2 	/* Number of DASD error types        */
#define SD_ERR_THRESHOLD	21600 	/* Time Threshold in seconds between */
					/* log entries of same error type    */
					/* Equals 6 hours                    */
#define SD_LOG_LIMIT		3 	/* Allow three logs in UEC shift tbl */
/*
 * Dump Defines
 */
#define SD_DUMP_RETRIES         04      /* max retries for dump              */
#define SD_MAX_DMP_LOOPS        4000000 /* 4 million microseconds delay cnt  */
/*
 * Retry count defines
 */
#define SD_MAX_POS_RETRIES      04      /* max retries for reading pos regs  */
#define SD_MAX_IPL_RETRIES      10      /* max retries waiting for IPL diags */
#define SD_MAX_RESET_RETRIES    3       /* max retries for resetting the adap*/
/*
 * Mode sense/select defines
 */
#define SD_MAX_MODE_PAGES	0x3f
#define SD_BLK_DESC_LN_INDEX	3

/*
 *    Event notification command code for send_diagnostics
 */ 

#define SD_EVENT_NOTIFICATION 0x85

/*
 * Mailbox OP codes
 */
#define SD_SEND_SCSI_OP         0x80    /* send scsi command mailbox op      */
#define SD_DOWNLOAD_MCODE       0x01    /* download adapter microcode        */
#define SD_QUERY_DEV_OP         0x02    /* query device mailbox op           */
#define SD_ABORT_SCSI_OP        0x03    /* abort scsi command mailbox op     */
#define SD_RSTQSC_OP            0x04    /* reset/quiesce mailbox op          */
#define SD_SETPARMS_OP          0x05    /* set adapter parms op              */
#define SD_TRACE_SNAP_OP        0x06    /* adapter trace snapshot op         */
#define SD_QUERY_TRACE_OP       0x07    /* adapter query trace op            */
#define SD_INQUIRY_OP           0x08    /* adapter inquiry                   */
/*
 * Status Definitions
 */
#define SD_SUSPEND              0x01    /* flag to suspend operation         */
#define SD_LOCKED               0x02    /* flag to lock operation            */
#define SD_DOWNLOAD_PENDING     0x04    /* pending download (for adaps/crtls)*/
#define SD_REASSIGN_PENDING     0x04    /* pending reassign (for DASD only)  */
#define SD_QUIESCE_PENDING      0x08    /* pending quiesce                   */
#define SD_RESET_PENDING        0x10    /* pending reset                     */
#define SD_VERIFY_PENDING       0x20    /* pending device verification       */
#define SD_REQ_SNS_PENDING	0x40    /* request sense pending             */
#define SD_DELAY		0x80    /* flag to delay further operation   */


/*
 * Adapter POS register addressing definitions
 */
#define SD_POS0                 0x400000 /* RO, card id low        0x77      */
#define SD_POS1                 0x400001 /* RO, card id high       0x8d      */
#define SD_POS2                 0x400002 /* RW, card arb and enable          */
#define SD_POS3                 0x400003 /* RW, data when 6&7 non-zero       */
#define SD_POS4                 0x400004 /* RW, card int and nibble ena      */
#define SD_POS5                 0x400005 /* RW, card status, unused          */
#define SD_POS6                 0x400006 /* RW, card addr ext low            */
#define SD_POS7                 0x400007 /* RW, card addr ext high           */
/*
 * Adapter bus memory addressing definitions
 */
#define SD_CMPL                 0x50    /* RO,  Completion Register (4 bytes)*/
#define SD_BCKP                 0x54    /* RO,  Backup Register (4 bytes)    */
#define SD_ALRT                 0x58    /* RO,  Alert Register (4 bytes)     */
#define SD_MBPT                 0x68    /* RW,  Mail Box Pointer Register (4)*/
#define SD_LTAG                 0x6C    /* RW,  Last Tag Register  (1 byte)  */
#define SD_CTAG                 0x6E    /* RO,  Current Tag Register (1 byte)*/
#define SD_CTRL                 0x70    /* RW,  Adapter Control Register (1) */
/*
 * HAC definitions
 */
#define SD_HAC_LOG_SENSE        0x40    /* Bit specifying log of sense data  */
#define SD_HAC_MASK             0x0F    /* Mask to get recovery action       */
#define SD_NO_RECOV_HAC         0x00    /* No error recovery action required */
#define SD_NO_RETRY_HAC         0x01    /* Retry Not recommended             */
#define SD_RETRY_1_HAC          0x02    /* Retry failing operation once      */
#define SD_RETRY_10_HAC         0x03    /* Retry failing operation ten times */
#define SD_RESET_CTRL_HAC       0x04    /* Reset controller and retry op     */
#define SD_QUIESCE_CTRL_HAC     0x05    /* Quiesce controller and retry op   */
#define SD_RESET_DASD_HAC       0x06    /* Reset DASD and retry failing op   */
#define SD_QUIESCE_DASD_HAC     0x07    /* Quiesce DASD and retry failing op */
#define SD_VERIFY_CTRL_HAC      0x08    /* Verify the controller             */
#define SD_ESOFT_ERR_HAC        0x09    /* Recommend reassignment (ESOFT)    */
#define SD_EMEDIA_ERR_HAC       0x0A    /* Request reassignment (EMEDIA)     */
#define SD_A_RESET_DASD_HAC     0x0B    /* Absolute reset of DASD and retry  */
#define SD_VERIFY_DASD_HAC      0x0C    /* Verify the DASD                   */
#define SD_REASSIGN_FAIL        0x0D    /* Reassign failed, treat like 0x01  */
#define SD_ASYNC_EVNT_HAC       0x0E    /* Async Event HAC. Retry once.      */
#define SD_NO_VALID_SENSE       0x0F    /* None of the sense data is valid   */
/*
 * Ready Async Messages
 */
#define SD_DEVICE_READY         0x00    /* A device has come ready           */
#define SD_TYPE1_NO_IML         0x01    /* Controller Type 1 Err without IML */
#define SD_TYPE1_WITH_IML       0x02    /* Controller Type 1 Error with IML  */
/*
 * PIO operation defines
 */
#define GETC                    0x00    /* read a character                  */
#define PUTC                    0x01    /* write a character                 */
#define GETS                    0x02    /* read a short                      */
#define PUTS                    0x03    /* write a short                     */
#define GETL                    0x04    /* read a long                       */
#define PUTL                    0x05    /* write a long                      */
/*
 *  Serial Dasd minor number masks.
 */
#define SD_DAEMON_MASK          0x1000  /* Bit set for daemon entry point    */
#define SD_CTRL_MASK            0x2000  /* minor mask for controllers        */
#define SD_DASD_MASK            0x4000  /* minor mask for dasd               */
/*
 * Debug Trace length
 */
#ifdef DEBUG
#define TRCLNGTH                300    /* Length of adapter trace table     */
#define TRCLNGTH_PTR            10     /* Number of adap trace tables       */
				       /* that we point to.		    */
#endif

/*---------------------------------------------------------------------------*

                          SERIAL DASD SUBSYSTEM MACROS

 *---------------------------------------------------------------------------*/

/*
 * Create the QC SCSIEXT byte of the send scsi command mailbox
 */
#define SD_QC_SCSI(qc,scsi_ext)  (((qc) << 6) | (scsi_ext))

/*
 * calculates the bus dma address the adapter uses
 * when using the reserved TCW range which was passed in the ddi area.
 */
#define SD_DMA_ADDR(start_addr, tcw_num) \
                ((start_addr) + ((tcw_num) * SD_TCWSIZE))

/*
 * returns the lun id from a device address.
 */
#define SD_LUN(x)          ((x) & 0x0F)

/*
 * returns the device address from the target id and lun
 * parameters are ( controller, drive, C=1/D=0)
 */
#define SD_LUNTAR(s,l,i)   (((s<<5)&0xE0) | (i) | (l))

/*
 * generate the TAR number from the device address
 */
#define SD_TARGET(x) ((x & 0xE0) >> 5)

/*
 * generate the mailbox address, start is the start
 * of the allocated mailbox TCW space, tag is the
 * mailbox tag (1-127) being addressed.
 */
#define SD_MB_ADDR(start,tag)      (start + ((tag) * SD_MB_SIZE))

/*
 * Non-Critial Path PIO macros
 */
#define SD_PUTC(a, c) sd_pio(ap, (void *)(a), (char) PUTC, (long)(c))
#define SD_GETC(a, c) sd_pio(ap, (void *)(a), (char) GETC, (long)(c))
#define SD_PUTS(a, s) sd_pio(ap, (void *)(a), (char) PUTS, (long)(s))
#define SD_GETS(a, s) sd_pio(ap, (void *)(a), (char) GETS, (long)(s))
#define SD_PUTL(a, l) sd_pio(ap, (void *)(a), (char) PUTL, (long)(l))
#define SD_GETL(a, l) sd_pio(ap, (void *)(a), (char) GETL, (long)(l))

/*
 * Macro for setting a tag as active ( 0 = active )
 */
#define SD_GETTAG(word,tag)         word &= (~(1<<(31-(tag%32))))

/*
 * Macro for clearing a tag as free  ( 1 = free )
 */
#define SD_FREETAG(word,tag)       word |= ( 1<<( 31-(tag%32)))


/*---------------------------------------------------------------------------*

                          SERIAL DASD SUBSYSTEM EXTERNS

 *---------------------------------------------------------------------------*/

extern  lock_t  sd_global_lock;   /* global lock used by open,ioctl,close  */


/*
 * hash table of pointers to dasd info structures
 */
extern  struct  sd_dasd_info    *sd_dasd_table[SD_DASD_TBL_SIZE];
/*
 * hash table of pointers to controller info structures
 */
extern  struct  sd_ctrl_info    *sd_ctrl_table[SD_CTRL_TBL_SIZE];
/*
 * hash table of pointers to adapter info structs
 */
extern  struct   sd_adap_info   *sd_adap_table[SD_ADAP_TBL_SIZE];
/*
 * Pointer to first adapter in Start Chain
 */
extern  struct  sd_adap_info    *aphead;
/*
 * Debug Strings
 */
#ifdef DEBUG
extern char     *strategy;
extern char     *insertq;
extern char     *dequeue;
extern char     *start;
extern char     *coales;
extern char     *relocate;
extern char     *startcmd;
extern char     *interrupt;
extern char     *complete;
extern char     *error;
extern char     *sdiodone;
extern char     *inadapchain;
extern char     *indasdchain;
extern char     *outdasdchain;
extern char     *outadapchain;
extern char     *lasttag;
extern char     *alertnotag;
extern char     *bufretry;
extern char     *failbuf;
extern char     *scsierror;
extern char     *processbuf;
extern char     *processreset;
extern char     *waitreset;
extern char     *qcmd;
extern char     *dqcmd;
extern char     *cmdalloc;
extern char     *cmdfree;
extern char     *cmdfail;
extern char     *mballoc;
extern char     *mbfree;
extern char     *cmdtimer;
extern char     *ioctltimer;
extern char     *flushadap;
extern char     *failadap;
extern char     *faildasd;
extern char     *reset_quiesce;
extern char     *haltadap;
extern char     *verify;
extern char     *async;
extern char     *piorecov;
extern char     *activity;
extern char     *qrytimer;
extern char     *qrydev;
extern char     *startunit;
extern char     *testunit;
extern char     *reqsns;
extern char     *reserve;
extern char     *release;
extern char     *fence;
extern char     *modesense;
extern char     *modeselect;
extern char     *readcap;
extern char     *inquiry;
extern char     *setparms;
extern char     *scsisense;
extern char     *notcws;
extern char     *tcwalloc;
extern char     *sddelay;
extern char     *delay_tm;
extern char     *entry;
extern char     *exit;
extern char     *message;
extern char     *trc;
#endif


/*---------------------------------------------------------------------------*

                  SERIAL DASD SUBSYSTEM ADAPTER STRUCTURES

 *---------------------------------------------------------------------------*/

/*
 * Debug Trace Table Structure
 */
#ifdef DEBUG

struct sd_trace {
#define SD_TRC_STR_LENGTH       8
        char    desc[SD_TRC_STR_LENGTH];    /* ASCII descrip of this entry   */
        char    type[3];                    /* ASCII desrip of entry or exit */
        char    count;                      /* which of several entries      */
        uint    word1;                      /*  meaning depends on the desc  */
        uint    word2;                      /*  meaning depends on the desc  */
        uint    word3;                      /*  meaning depends on the desc  */
        uint    word4;                      /*  meaning depends on the desc  */
        uint    word5;                      /*  meaning depends on the desc  */
};
#endif


/*
 * Watchdog Structure
 */
struct sd_watchdog {
        struct watchdog watch;          /* system watchdog structure         */
        void            *pointer;       /* pointer to info structure         */
};

/*
 * Asynchronous Event Structure
 */
struct sd_event {
        struct sd_event *next_event;    /* pointer to next event for link    */
        uchar           event;          /* describes the event :             */

#define SD_CONFIG               0x00    /* Config starting at this controller*/
#define SD_DLMC                 0x01    /* Download microcode to this ctrl   */
#define SD_ADAPCFG              0x02    /* Config this adapter               */
#define SD_ADAPDLMC             0x04    /* Download microcode to this adap   */

        uchar           tarlun;         /* Target and lun address of the     */
                                        /* device posting an event           */
};

/*
 * Daemon's Error Logging Structure
 */
struct sd_daemon_errlog {
	uchar           cmd_type;        /* type of command                  */
			   		 /* 0x01   addressed to adapter      */
			   		 /* 0x02    addressed to controller  */
			   		 /* 0x04    addressed to dasd        */
	uchar           log_func;        /* which log function to call       */
#define SD_DAEMON_ADAP_LOG    0x01       /* call sd_log_adap_err             */
#define SD_DAEMON_CTRL_LOG    0x02       /* call sd_log_ctrl_err             */
#define SD_DAEMON_DASD_LOG    0x04       /* call sd_log_dasd_err             */
#define SD_DAEMON_CMD_LOG     0x08       /* call sd_log__error               */
	uint		elog_sys_dma_rc;/* system DMA return code...for log  */
        uchar           status_validity;/* 0 = no valid status               */
                                        /* 1 = valid adapter status          */
                                        /* 2 = valid controller status       */
                                        /* 3 = valid adapter and ctrler      */
					/* 4 = valid scsi status only        */

        uchar           scsi_status;    /* SCSI Status (if valid)            */
        uchar           adapter_status; /* Adapter Status (if valid)         */
        uchar           controller_status; /* controller status (if valid)   */
	uchar           driver_status;  /* Device driver status              */
        ushort          uec;          	/* Unit Error Code                   */
        uchar           tarlun;         /* address of device                 */
        uchar           elog_validity;	/* bitmap flags for valid log data   */
};

/*
 * Define device structure for adapter
 */
struct sd_adap_dds {
        uchar           dev_type;       /* device type :                     */
                                        /* 0 = Adap, 1 = Ctrl, 2 = DASD      */
        ulong           bus_id;         /* adapter I/O bus id                */
        ushort          bus_type;       /* adapter I/O bus type              */
        uchar           slot;           /* I/O slot number                   */
        uint            base_addr;      /* adapters base address             */
        char            resource_name[16]; /* resource name of this adapter  */
        int             dma_lvl;        /* dma level                         */
        int             intr_lvl;       /* interrupt level                   */
        int             intr_priority;  /* interrupt priority                */
        ulong           tcw_start_addr; /* start addr of bus tcw space       */
        int             tcw_length;     /* length in bytes of tcw area       */
};

/*
 * Hardware Determined Mailbox Structure
 */
struct sd_mbox {
        struct sd_mbox  *nextmb;        /* pointer to next mb, bytes 0..3    */
        uchar           op_code;        /* adapter op code, byte 4           */
        uchar           tag;            /* mailbox tag, byte 5               */
        union {
                uchar   contents;       /* generic name                      */
                uchar   qc_scsiext;     /* queue control, scsi extension     */
                uchar   abort_tag;      /* tag of mailbox to abort           */
                uchar   reset_type;     /* type of reset for reset cmd       */

#define SD_QUIESCE_ADAP_MB      0x06    /* Quiesce adapter reset type        */
#define SD_RESET_CTRL_MB        0x05    /* Reset controller reset type       */
#define SD_QUIESCE_CTRL_MB      0x04    /* Quiesce controller reset type     */
#define SD_RESET_DASD_MB        0x03    /* Reset dasd reset type             */
#define SD_QUIESCE_DASD_MB      0x02    /* Quiesce dasd reset type           */

                uchar   length;         /* parm length(trace,adapt parms)    */
        } mb6;                          /* mailbox byte 6                    */
        union {
                uchar   contents;       /* generic name                      */
                uchar   nul_address;    /* address for adapter operations    */
                uchar   dev_address;    /* LUN-TAR address of device         */
        } mb7;                          /* mailbox byte 7                    */
        union {
                struct {
                        uint          dma_addr;  /* DMA address, bytes 8..11 */
                        uint          dma_length;/* DMA length, bytes 12..15 */
                        struct sc_cmd scsi_cmd;  /* bytes 16 .. 27           */
                } fields;
                struct {
                        uint          parm[5];   /* adap parms, bytes 8..27  */
                } ad_parm;
                struct {
                        uchar        byte8;
                        uchar        byte9;
                        uchar        byte10;
                        uchar        byte11;
                        uchar        byte12;
                        uchar        byte13;
                        uchar        byte14;
                        uchar        byte15;
                        uchar        byte16;
                        uchar        byte17;
                        uchar        byte18;
                        uchar        byte19;
                        uchar        byte20;
                        uchar        byte21;
                        uchar        byte22;
                        uchar        byte23;
                        uchar        byte24;
                        uchar        byte25;
                        uchar        byte26;
                        uchar        byte27;
                } data;                        /* data bytes 16 - 27         */
                struct {
                        uint        dma_addr;  /* DMA address bytes 8..11    */
                        uint        dma_length;/* DMA length, bytes 12..15   */
                        uint        ev1_4;     /* events 1-4 bytes 16..19    */
                        uint        ev5_7;     /* events 5-7 bytes 20-23     */
                        uchar       byte24;
                        uchar       byte25;
                        uchar       byte26;
                        uchar       byte27;
                } ad_snpsht;
        } mb8;                                /* mailbox bytes 8 .. 27       */
        union {
                struct {
                        uchar       byte28;   /* mailbox byte 28             */
                        uchar       byte29;   /* mailbox byte 29             */
                        uchar       byte30;   /* mailbox byte 30             */
                        uchar       byte31;   /* mailbox byte 31             */
                } data;
                struct {
                        uint        parm5;    /* adapter parm 6 bytes 28..31 */
                } ad_parm;
        } mb28;                               /* mailbox bytes  28..31       */
};


/*
 * Command "Jacket" Structure
 */
struct sd_cmd {
        struct sd_cmd   *nextcmd;       /* next cmd, if linked               */
        struct sd_dasd_info *dp;        /* pointer to dasd_info for this cmd */
        struct sd_adap_info *ap;        /* pointer to adap_info for this cmd */
        struct sd_ctrl_info *cp;        /* pointer to ctrl_info for this cmd */
        struct buf      *bp;            /* pointer to first buf for this cmd */
        struct xmem     *xmem_buf;      /* cross memory descriptor           */
        void            *mb_dma_addr;   /* dma_address of this mailbox       */
        struct  sd_mbox *mb;            /* pointer to real 32-byte mailbox   */
        char            *b_addr;        /* address of data buffer            */
        uint            b_length;       /* total length of data transfer     */
	uint		first_bp_resid; /* The resid of the first bp 	     */
        uint            rba;            /* target RBA of this command        */
        uint            b_error;        /* error for this command            */
        int             dma_flags;      /* dma flags for this transfer       */
        int             timeout;        /* timeout value for this command    */
        uint            last_rba;       /* last rba for this command         */
        int             tcws_start;     /* starting TCW                      */
	uint		tcw_mask_first;	/* mask for first word of tcw alloced*/
	uint		tcw_mask_last;	/* mask for last word of tcw alloced */
	uint		elog_sys_dma_rc;/* system DMA return code...for log  */
        char            tcw_first_word;	/* first TCW word used               */
        char            tcw_words; 	/* number of TCW words used          */
        signed char 	sta_index;      /* allocated STA, -1 if none         */
        uchar           type;           /* type of command                   */

#define SD_ADAP_CMD             0x01    /* addressed to adapter              */
#define SD_CTRL_CMD             0x02    /* addressed to controller           */
#define SD_DASD_CMD             0x04    /* addressed to dasd                 */

        uchar           tag;            /* this command's tag                */
        uchar           status;         /* status of this command            */

#define SD_FREE                 0x00    /* free to be used                   */
#define SD_ACTIVE               0x01    /* currently in use                  */
#define SD_RETRY                0x02    /* complete, need retry              */
#define SD_QUEUED               0x04    /* command on queue                  */
#define SD_TIMEDOUT             0x08    /* command timed out                 */
#define SD_LOG_ERROR   		0x10    /* Flag to log error immediately     */
#define SD_RECOV_ERROR		0x20    /* Flag That error was recovered     */
#define SD_DAEMON_PRI 		0x40    /* Flag for daemon priority command  */

        uchar           cmd_info;       /* imformation about this command    */

#define SD_NORMAL_PATH          0x00    /* normal path command               */
#define SD_IOCTL                0x01    /* IOCTL command                     */
#define SD_REFRESH              0x02    /* Refresh command (volume status    */
                                        /*                  change)          */
#define SD_LOCK                 0x03    /* Lock via reserve                  */
#define SD_UNLOCK               0x04    /* Unlock via release                */
#define SD_TEST                 0x05    /* Test device via test unit ready   */
#define SD_FENCE_POS_CHECK      0xC0    /* Fence host position check         */
#define SD_FENCE                0xC1    /* reserved fence command            */
#define SD_REQSNS               0xC2    /* reserved request sense command    */
#define SD_RST_QSC              0xC3    /* reserved Reset/Quiesce command    */
#define SD_START_UNIT           0xC4    /* reserved start Unit command       */
#define SD_TEST_UNIT_READY      0xC5    /* reserved test unit ready command  */
#define SD_QRYDEV               0xC6    /* reserved query device command     */
#define SD_SPECIAL              0x07    /* reserved for special cmds         */
#define SD_RESERVE              0xC8    /* reserved Reserve command          */
#define SD_MODE_SENSE           0xC9    /* reserved mode sense command       */
#define SD_MODE_SELECT          0xCA    /* reserved mode select command      */
#define SD_READ_CAPACITY        0xCB    /* reserved read capacity command    */
#define SD_STOP_UNIT            0x0C    /* reserved stop unit command        */
#define SD_RELEASE              0xCD    /* reserved release command          */
#define SD_INQUIRY              0xCE    /* reserved inquiry command          */
#define SD_REASSIGN             0x4F    /* reserved reassign block command   */

#define SD_OK_DURING_VERIFY     0x80    /* mask for cmd_info if command can  */
                                        /* be let through during a verify    */
#define SD_OK_DURING_REASSIGN   0x40    /* mask for cmd_info if command can  */
                                        /* be let through during a reassign  */


        signed char    	retry_count;    /* number of retries on this command */
        signed char   	retries;        /* number of retries to allow        */
        uchar           status_validity;/* 0 = no valid status               */
                                        /* 1 = valid adapter status          */
                                        /* 2 = valid controller status       */
                                        /* 3 = valid adapter and ctrler      */
#define SD_VALID_SCSI_STATUS    4       /* 4 = valid scsi status only        */

        uchar           scsi_status;    /* SCSI Status (if valid)            */
        uchar           adapter_status; /* Adapter Status (if valid)         */
        uchar           controller_status; /* controller status (if valid)   */
	uchar           driver_status;  /* Device driver status              */
        uchar           erp;            /* Error recovery proc, if necessary */
        ushort          uec;          	/* Unit Error Code                   */
        uchar           dev_address;    /* address of device                 */
        uchar           last_rba_valid; /* flag whether last rba is valid    */
        uchar           reloc;          /* flag whether relocation is needed */
        uchar           elog_validity;	/* bitmap flags for valid log data   */
        uchar           alert_tag;	/* mailbox tag in alert for log data */
#define	SD_REQ_SNS_LENGTH	32	/* length of request sense data      */
        uchar           req_sns_bytes[SD_REQ_SNS_LENGTH];
					/* Request sense bytes for this cmd  */
        struct sd_mbox  mbox_copy;      /* local copy of mail box            */
	struct conc_cmd *conc_cmd_ptr;  /* pointer to conc_cmd struct        */
	
};

/*
 * Interrupt Structure
 */
struct sd_intr {
        struct intr             intr_st; /* system interrupt structure */
        struct sd_adap_info     *ap;     /* pointer to adapter info structure*/
};

/*
 * Small Transfer Area Management Structure
 */
struct sta_str {
        uint            in_use;        /* TRUE if this area in use           */
        char            *stap;         /* pointer to this xfer area          */
};

/*
 * Error Logging Statistics Structure
 */
struct	sd_logbook {
	ulong		last_log;	/* Timestamp of last log entry       */
#define	SD_NUM_UEC_HISTORY	8 
	ushort		uecs[SD_NUM_UEC_HISTORY]; /* Last UEC's logged	     */
};

/*
 * Adapter Info Structure
 */
struct sd_adap_info {
        struct sd_adap_info *hash_next;/*pointer to next adapter for hashing */
        dev_t           devno;         /* this adapters devno                */
        uint            ioctl_event;   /* ioctl event word                   */
        uint            ioctl_timeout; /* 1 if timed out, else 0             */
        lock_t          dev_lock;      /* locking word to serialize ioctl's  */
        struct sd_watchdog ioctl_timer;/* to control ioctl timeouts          */
        struct sd_watchdog cmd_timer;  /*to control adapter command timeouts */
        uchar           diag_mode;     /* flag if diagnostic mode            */
        uchar           status;        /* status of adapter                  */
        uchar           ioctl_intrpt;  /* ioctl flag used before sleep       */
        struct sd_adap_dds dds;        /* this adapters dds                  */

        /*
         * note the preceding fields must remain in
         * this order to stay consistent with sd_ctrl_info
         * and sd_dasd_info
         */

        struct sd_mbox  *MB;            /* pointer to array of mailboxes     */
        void            *base_MB_dma_addr; /* base dma address of Mailboxes  */
        struct sd_cmd   *errhead;       /* head of error queue               */
        struct sd_cmd   *errtail;       /* tail of error queue               */
        struct sd_cmd   *ioctlhead;     /* head of ioctl queue               */
        struct sd_cmd   *ioctltail;     /* tail of ioctl queue               */
        struct sd_cmd   *ctrl_errhead;  /* head of controller error queue    */
        struct sd_cmd   *ctrl_errtail;  /* tail of controller error queue    */
        struct sd_cmd   *ctrl_ioctlhead;/* head of controller ioctl queue    */
        struct sd_cmd   *ctrl_ioctltail;/* tail of controller ioctl queue    */
        struct sd_dasd_info *dphead;    /* head of dasd start chain for adap */
        struct sd_dasd_info *nextdp;    /* next dasd in start chain process  */
        struct sd_dasd_info *starting_dp;/* Starting mark for round robin    */
        uint            *tcw_free_list; /* pointer to tcw management table   */
        struct sd_mbox  *curr_mb;       /* pointer to current MB to be used  */
        struct sd_event *event_head;    /* Pointer to head of event list     */
        struct sd_event *event_tail;    /* Pointer to tail of event list     */
        struct trb      *reset_timer;   /* Timer to handle adapter resets    */
        struct trb      *halt_timer;    /* Timer to handle adapter quiesce   */
        struct trb      *delay_timer;   /* Timer to handle delayed operation */
        uint            max_transfer;   /* max transfer (bytes) for this adap*/
        int             daemon_pid;     /* this adapter's daemons PID        */
        int             dma_channel;    /* dma channel id                    */
        uint            free_cmd_list[SD_NUM_CMD_WORDS]; /* free cmd list    */
        uint            mb_alloc_list[SD_NUM_MB_WORDS]; /* alloced mb list   */
        uint            mb_free_list[SD_NUM_MB_WORDS];   /* free mb list     */
        uint            free_event_list[SD_NUM_EVENT_WORDS];/*free event list*/
        uint            asynch_event;   /* asynchronous event word           */
        uint            adap_event;     /* adapter event word                */
        uint            resources;      /* adapter resource event word       */
        uint            open_no1_event; /* event word to let open continue   */
        int             dump_ilevel;    /* old interrupt level before dump   */
        ushort          num_tcws;       /* number of reserved tcws           */
        ushort          sta_tcw_start;  /* starting tcw for STA              */
        ushort          mb_tcw_start;   /* starting tcw for MBA              */
        char            mb_word;  	/* next mailbox word to look at      */
        char            tcw_word;  	/* next tcw word to look at          */
        char            shift;  	/* number to shift tcw word to search*/
        char            num_tcw_words;  /* number of tcw words               */
        uchar           resource_intrpt;/* flag used before sleep on resource*/
        uchar           asynch_intrpt;  /* flag used before sleep on asynch  */
        uchar           open_no1_intrpt;/* flag used before sleep on 1st open*/
        uchar           opened;         /* flag whether this adapter open    */
        uchar           fs_open;        /* flag if file system open on adap  */
        uchar           internal_open;  /* flag if internal open on adap     */
        uchar           ever_open;      /* flag if this adap was ever open   */
        uchar           daemon_open;    /* flag if this adaps daemon open    */
        uchar           inited_ctrls;   /* number of inited ctrls on adap    */
        uchar           open_ctrls;     /* number of opened ctrls on adap    */
        uchar           cmds_out;       /* number of cmds outstanding on adap*/
        uchar           curr_tag;       /* current tag of next MB to be used */
        uchar           next_tag;       /* tag of next MB to be used         */
        uchar           last_tag;       /* tag value written to last tag     */
        char            dumpdev;        /* flag whether dump device          */
        uchar           IPL_tmr_cnt;    /* IPL timeout counter               */
        char            reset_result;   /* results of reset                  */
        char            reset_count;    /* count of reset attempts           */
        char            adap_result;    /* results of adapter command        */
        uchar           pos2;           /* data for pos2                     */
        uchar           pos3;           /* data for pos3                     */
        uchar           pos4;           /* data for pos4                     */
        uchar           adap_resources; /* flag for available resources      */
        struct sd_cmd   *cmd_map[SD_NUM_MB]; /* pointers to all ACTIVE tags  */
        struct sd_ctrl_info *ctrllist[SD_NUM_CTRLS];  /* adap's ctrler list  */
        struct sd_cmd   cmds[SD_NUM_MB];/* command structures                */
        struct sd_cmd   quiesce;        /* reserved command structure resets */
        struct sd_cmd   special;        /* reserved command structure for    */
        struct sta_str  STA[SD_NUM_STA];/* STA management table              */
        struct sd_event sd_event[SD_MAX_EVENTS]; /* pool of event structures */
        struct sd_adap_error_df elog;   /* adapter error logging data        */
	struct sd_logbook logbook[SD_ADAP_ETYPES]; /* log book for adapter   */
        struct sd_intr  sdi;            /* interrupt structure               */
        struct xmem     xmem_buf;       /* local xmem descriptor structure   */
	int             unconfiguring ; /* True if unconfiguring             */
	Simple_lock	spin_lock;	/* Adapter spin lock		     */

#ifdef DEBUG
	int		ap_trctop;	/* Top of Dasd Trace 		     */
	struct sd_trace *ap_trace;       /* Dasd Trace Buffer	             */
	int		ap_trcindex;	/* Dasd Trace Index                  */
#endif
};

/*---------------------------------------------------------------------------*

                  SERIAL DASD SUBSYSTEM CONTROLLER STRUCTURES

 *---------------------------------------------------------------------------*/

/*
 * Controller device define structure
 */
struct sd_ctrl_dds {
        uchar           dev_type;       /* device type :                     */
                                        /* 0 = adap, 1 = ctrl, 2 = DASD      */
        char            resource_name[16];/* this controller's resource name */
        dev_t           adapter_devno;  /* this ctrl's adapter maj/min number*/
        uchar           target_id;      /* this controllers target ID (0..7) */
	uchar           fence_enabled;  /* this controller supports fencing  */
	uchar           conc_enabled;  /* this controller supports fencing  */
    };

/*
 * Controller info structure
 */
struct sd_ctrl_info {
        struct sd_ctrl_info *hash_next; /* pointer to next controller        */
        dev_t           devno;          /* this controllers devno            */
        uint            ioctl_event;    /* ioctl event word                  */
        uint            ioctl_timeout;  /* 1 if timed out else 0             */
        lock_t          dev_lock;       /* lock for controller               */
        struct sd_watchdog ioctl_timer; /* to control ioctl time outs        */
        struct sd_watchdog cmd_timer;   /* to control ctrler command timeouts*/
        uchar           diag_mode;      /* flag if diagnostic mode           */
        uchar           status;         /* status of this controller         */
        uchar           ioctl_intrpt;   /* ioctl flag used before sleep      */
        struct sd_ctrl_dds dds;         /* this controllers dds              */

        /*
         * note the preceding fields must remain in
         * this order to stay consistent with sd_adap_info
         * and sd_dasd_info
         */

        struct sd_adap_info *ap;        /* pointer to adapter info structure */
        struct trb      *delay_timer;   /* Timer to handle delayed operation */
        uchar           opened;         /* flag wheter opened                */
        uchar           fs_open;        /* flag if file system open on ctrl  */
        uchar           internal_open;  /* flag if internal open on ctrl     */
        uchar           inited_dasd;    /* count of inited dasd on this ctrl */
        uchar           open_dasd;      /* count of opened dasd on this ctrl */
        uchar           cmds_out;       /* number of cmds outstanding on ctrl*/
        uchar           cmds_qed;       /* number of cmds queued on dasd     */
        uchar           reset_count;  	/* number of resets                  */
        uchar           sense_buf[256]; /* buffer for inquiries,reqsns, etc  */
        struct sd_cmd   reset;          /* reserved cmd struct for resets    */
        struct sd_cmd   quiesce;        /* reserved cmd struct for quiesce   */
        struct sd_cmd   reqsns;         /* reserved cmd struct for req sense */
        struct xmem     xmem_buf;       /* local xmem descriptor structrue   */
        struct sc_error_log_df elog;    /* error logging data                */
	struct sd_logbook logbook[SD_CTRL_ETYPES]; /* log book for controller*/
        struct sd_dasd_info *dasdlist[SD_NUM_DASD]; /* ctrler's dasd list    */

};


/*---------------------------------------------------------------------------*

                  SERIAL DASD SUBSYSTEM DASD STRUCTURES

 *---------------------------------------------------------------------------*/


/*
 * Capacity Structure
 */
struct sd_capacity {
        int             lba;            /* last logical block address        */
        int             len;            /* block length in bytes             */
};

/*
 *  Send diagnostics paramter list for event notification structure.
 */
struct sd_diag_event 
{
    uchar byte[6];
};


/*
 * Defect List
 */
struct sd_def_list {
        int             header;         /* header for defect list            */
        int             lba;            /* logical block address of bad block*/
};

/*
 * DASD device define structure
 */
struct sd_dasd_dds {
        uchar           dev_type;       /* device type :                     */
                                        /* 0 = adap, 1 = ctrl, 2 = DASD      */
        char            resource_name[16]; /* this dasd's resource name      */
        dev_t           controller_devno;  /* this dasd's controller maj/min */
        uchar           lun_id;         /* this dasd's LUN ID ( 0 .. 15 )    */
        uchar           safe_relocate;  /* flag if reassign support, and safe*/
#define DK_NO_RELOCATION        0       /* Relocation not supported          */
#define DK_SAFE_RELOCATION      1       /* Relocation is safe                */
#define DK_UNSAFE_RELOCATION    2       /* Rel. supported but not safe       */
        uchar           extended_rw;    /* flag if SCSI extended rw supported*/
        uint            segment_size;   /* number of bytes in sample rate    */
        uint            segment_cnt;    /* number of segments read           */
        uint            byte_count;     /* number of bytes into current seg  */
        uint            max_coalesce;   /* max number of bytes to coalesce   */
        int             queue_depth;    /* number of cmds to queue to dasd   */
        int             mode_data_length;/* mode select data length          */
        int             mode_default_length;/* mode default data length      */
        char            mode_data[256]; /* mode data buffer                  */
        char            mode_default_data[256]; /* mode default data buffer  */
};

/*
 * Internal Debug Trace Table Structure for DASD
 */
#ifdef DEBUG
struct dp_trace {
        char    desc[SD_TRC_STR_LENGTH];    /* ASCII descrip of this entry   */
        char    type[3];                    /* ASCII desrip of entry or exit */
        char    count;                      /* which of several entries      */
        uint    word1;                      /*  meaning depends on the desc  */
};
#endif

/*
 * Mode Data Format Control Structure
 */
struct sd_mode_format {
        signed char page_index[SD_MAX_MODE_PAGES]; /* offset to page in buff */
        ushort  sense_length;			   /* total length of sense  */
        uint    block_length;			   /* device block length    */
};


/*
 * DASD info Structure
 */
struct sd_dasd_info {
        struct sd_dasd_info *hash_next; /* pointer to next dasd info for     */
        dev_t           devno;          /* this dasd's devno                 */
        uint            ioctl_event;    /* ioctl event word                  */
        uint            ioctl_timeout;  /* 1 if timeout else 0               */
        lock_t          dev_lock;       /* lock for dasd                     */
        struct sd_watchdog ioctl_timer; /* to control ioctl time outs        */
        struct sd_watchdog cmd_timer;   /* to control dasd command timeouts  */
        uchar           diag_mode;      /* flag if diagnostic mode           */
        uchar           status;         /* status of this dasd               */
        uchar           ioctl_intrpt;   /* ioctl flag used before sleep      */
        struct sd_dasd_dds dds;         /* this dasd's dds                   */

        /*
         * note the preceding fields must remain in
         * this order to stay consistent with sd_ctrl_info
         * and sd_adap_info
         */

#ifdef DEBUG
	int		dp_trctop;	/* Top of Dasd Trace 		     */
	struct dp_trace dp_trace[100];	/* Dasd Trace Buffer		     */
	int		dp_trcindex;	/* Dasd Trace Index                  */
#endif
        struct trb      *delay_timer;   /* Timer to handle delayed operation */
        struct sd_adap_info *ap;        /* pointer to this dasd's adapter    */
        struct sd_ctrl_info *cp;        /* pointer to this dasd's controller */
        struct sd_dasd_info *nextdp;    /* pointer to next dasd in chain     */
        struct sd_cmd   *errhead;       /* head of error queue               */
        struct sd_cmd   *errtail;       /* tail of error queue               */
        struct sd_cmd   *ioctlhead;     /* head of ioctl queue               */
        struct sd_cmd   *ioctltail;     /* tail of ioctl queue               */
        struct sd_cmd   *checked_cmd;   /* Pointer to checked command        */
        struct sd_cmd   *reassign_write;/* Pointer to reassigned write       */
        struct buf      *currbuf;       /* pointer to current buf on elevator*/
        struct buf      *prev_buff;     /* last buf scanned by activity timer*/
        struct buf      *low_cyl;       /* pointer to buf on lowest cylinder */
        uint            dasd_event;     /* open event word                   */
        uint            serial_num1;    /* high 4 bytes of serial number     */
        uint            serial_num2;    /* low 4 bytes of serial number      */
        uint            max_transfer;   /* Maximum size of transfer for dasd */
        uchar           opened;         /* flag whether opened               */
        uchar           cmds_out;       /* number of cmds outstanding on dasd*/
        uchar           start_chain;    /* flag whether in start chain       */
        uchar           no_reserve;     /* flag whether no reserve           */
        uchar           saved_no_reserve;/* saved no_reserve flag            */
        uchar           retain_reservation; /* flag whether to retain reserv */
        uchar           restart_count;  /* number of restarts                */
        uchar           reset_count;  	/* number of resets                  */
        uchar           query_count;  	/* count of query device attempts    */
        uchar           direction;      /* direction of elevator             */

#define SD_ELEVATOR_UP          0x00    /* elevator going up,
                                           (down = ~SD_ELEVATOR_UP)          */

        uchar           interrupted;    /* Flag whether interrupt received   */
        uchar           buf_started;    /* Flag whether new buf started      */
        uchar           old_tag;        /* previous tag used by dump         */
        uchar           reqsns_tag;     /* request sense tag used by dump    */
        char            dasd_result;    /* results of dasd startup sequence  */
        uchar           sick;           /* flag that device is unhealthy     */
        uchar           forced_open;    /* flag for forced open              */
        uchar           seq_not_done;   /* flag startup sequence not done    */
        uchar           serial_num_valid;/* flag whether serial num valid    */
        uchar           m_sense_status; /* flag if changeable or current sns */
#define	SD_SENSE_CURRENT	0x00	/* the sense data reflects current   */
#define	SD_SENSE_CHANGEABLE	0x40	/* the sense data reflects changeable*/
        uchar           sense_buf[256]; /* buffer for inquiries,reqsns, etc  */
        uchar           ch_data[256];   /* buffer of changeable mode data    */
	struct sd_mode_format dd;	/* Mode control data for desired data*/
	struct sd_mode_format df;	/* Mode control data for default data*/
	struct sd_mode_format ch;	/* Mode control for changeable data  */
	struct sd_mode_format cd;	/* Mode control for current data     */
        struct sd_cmd   reqsns;         /* reserved cmd struct for req sns   */
        struct sd_cmd   reassign;       /* reserved cmd struct for reassign  */
        struct sd_cmd   special;        /* reserved cmd struct for special   */
        struct sd_cmd   restart;        /* reserved cmd struct for startup   */
        struct sd_cmd   reset;          /* reserved cmd struct for resets    */
        struct sd_cmd   abs_reset;      /* reserved cmd struct for resets    */
        struct sd_cmd   quiesce;        /* reserved cmd struct for quiesce   */
        struct sd_cmd   qrydev;         /* reserved cmd struct for query dev */
        struct sd_watchdog query_timer;    /* watchdog timer for query device*/
        struct sd_capacity disk_capacity;  /* disk capacity information      */
        struct sd_capacity cyl_capacity;   /* cylinder capacity info         */
        struct sd_def_list def_list;       /* defect list for reassigns      */
        struct sc_error_log_df elog;       /* error logging data             */
	struct sd_logbook logbook[SD_DASD_ETYPES]; /* log book for dasd      */
        struct dkstat   dkstat;            /* I/O statistics for this disk   */
        struct xmem     xmem_buf;          /* local xmem descriptor structure*/
	uchar           conc_registered;   /* Indicates a kernel extension   */
	                                   /* has registerd for concurrent   */
					   /* mode                           */
	int (*conc_intr_addr)(struct conc_cmd*,uchar,uchar,dev_t);
	                                   /* Place to store registered      */
					   /* kernel extension's concurent   */
					   /* entry address.                  */
	ushort          fence_host_position; /* Position of this host in the */
	                                     /* fence register               */
	uchar           fence_data_valid;  /* TRUE if fence_host and         */
	                                   /* fence_data are valid, FALSE    */
	                                   /* otherwise.                     */
	ushort          fence_mask;        /* Fence mask for mask_swap fence */
	                                   /* command.                       */
	ushort          fence_data;        /* Fence data for mask_swap fence */
	                                   /* command.                       */
	struct sd_cmd   concurrent;        /* concurrent mode command jacket */
	struct sd_diag_event diag_event;   /* Space for a volume status      */
	                                   /* change parameter block         */
	struct conc_cmd *conc_cmd_list;    /* list of conc commands waiting  */
	int             unregistering;     /* unregister in progress         */
};


/*
 * Function prototypes
 */
#ifndef _NO_PROTO
/*
 *         SDCONFIG.C
 */
int sd_config(dev_t devno, int op, struct uio *uiop);
int sd_adap_config(dev_t devno, int op, struct uio *ui);
int sd_ctrl_config(dev_t devno, int op, struct uio *ui);
int sd_dasd_config(dev_t devno, int op, struct uio *ui);

/*
 *         SDOPEN.C
 */
int sd_open(dev_t devno, int rwflag, int chan, int ext);
int sd_adap_open(struct sd_adap_info *ap, int minorno, int rwflag, int chan, 
	int ext);
int sd_ctrl_open(struct sd_ctrl_info *cp, int rwflag, int chan, int ext);
int sd_dasd_open(struct sd_dasd_info *dp, int rwflag, int chan, int ext);
/*
 *         SDCLOSE.C
 */
int sd_close(dev_t devno,  int chan, int ext);
int sd_adap_close(struct sd_adap_info *ap, int minorno,  int chan, int ext);
int sd_ctrl_close(struct sd_ctrl_info *cp, int chan, int ext);
int sd_dasd_close(struct sd_dasd_info *dp,  int chan, int ext);
/*
 *         SDRDWR.C
 */
int sd_read(dev_t devno, struct uio *uiop, int chan, int ext);
int sd_write(dev_t devno, struct uio *uiop, int chan, int ext);
int sd_mincnt(struct buf *bp, void *minparms);
/*
 *         SDUTILT.C
 */
struct sd_adap_info *sd_alloc_adap(dev_t devno, uint tcw_size);
void sd_free_adap(struct sd_adap_info *ap);
struct sd_ctrl_info *sd_alloc_ctrl(dev_t devno);
void sd_free_ctrl(struct sd_ctrl_info *cp);
struct sd_dasd_info *sd_alloc_dasd(dev_t devno, int queue_depth);
void sd_free_dasd(struct sd_dasd_info *dp);
int sd_setup_adap(struct sd_adap_info *ap);
int sd_get_vpd(struct sd_adap_info *ap, uchar *vpd, int start, int count);
/*
 *         SDINTR.C
 */
int sd_intr(struct intr *is);
int sd_parse_err(struct sd_adap_info *ap, uint base);
void sd_parse_ready_async(struct sd_adap_info *ap, char *alert_reg, 
	uchar *processed, uchar *erp, ushort *uec);
void sd_alert_notag(struct sd_adap_info *ap, uchar erp, uchar address);
void sd_prepare_buf_retry(struct sd_cmd *cmd);
void sd_fail_buf_cmd(struct sd_cmd *cmd, char fail_dasd);
void sd_process_complete(struct sd_cmd *cmd, char skip);
void sd_process_error(struct sd_adap_info *ap, struct sd_cmd *cmd, char skip, 
	char queue);
void sd_process_scsi_error(struct sd_adap_info *ap, struct sd_cmd *cmd);
void sd_process_sense(struct sd_dasd_info *dp, struct sd_ctrl_info *cp,
        char type);
void sd_process_buf(struct sd_cmd *cmd);
void sd_process_reset(struct sd_cmd *cmd);
/*
 *         SDSTART.C
 */
void sd_start_disable(struct sd_adap_info *ap);
void sd_start(struct sd_adap_info *ap);
int sd_coalesce(struct sd_dasd_info *dp, char *rm_dp_from_list,
        char *cmdsbuilt, struct buf *curr);
int sd_relocate(struct sd_dasd_info *dp, struct buf *curr, struct sd_cmd *cmd);
int sd_start_cmd(struct sd_cmd *cmd);
/*
 *         SDUTILB.C
 */
void *sd_hash(dev_t devno);
int sd_read_POS(struct sd_adap_info *ap, uint offset);
int sd_write_POS(struct sd_adap_info *ap, uint offset, uchar data);
int sd_reload_pos(struct sd_adap_info *ap);
void sd_wait_reset(struct trb *t);
void sd_wait_reset_disable(struct trb *t);
int sd_restart_adap(struct sd_adap_info *ap);
void sd_q_cmd_disable(struct sd_cmd *cmd, char  queue);
void sd_q_cmd(struct sd_cmd *cmd, char  queue);
void sd_d_q_cmd_disable(struct sd_cmd *cmd);
void sd_d_q_cmd(struct sd_cmd *cmd);
struct sd_event *sd_event_alloc(struct sd_adap_info *ap);
struct sd_cmd *sd_cmd_alloc_disable(struct sd_adap_info *ap);
struct sd_cmd *sd_cmd_alloc(struct sd_adap_info *ap);
void sd_free_cmd_disable(struct sd_cmd *cmd);
void sd_free_cmd(struct sd_cmd *cmd);
void sd_fail_cmd(struct sd_cmd *cmd, char fail_dasd);
void sd_process_conc_cmd(struct sd_cmd *cmd);
int sd_MB_alloc(struct sd_cmd *cmd);
void sd_free_MB(struct sd_cmd *cmd,char status);
int sd_TCW_alloc(struct sd_cmd *cmd);
int sd_TCW_realloc(struct sd_cmd *cmd);
void sd_TCW_dealloc(struct sd_cmd *cmd);
int sd_STA_alloc(struct sd_cmd *cmd);
void sd_STA_dealloc(struct sd_cmd *cmd);
int sd_set_adap_parms_disable(struct sd_adap_info *ap, char from_open);
int sd_set_adap_parms(struct sd_adap_info *ap, char from_open);
void sd_cmd_timer(struct watchdog *w);
int sd_dma_cleanup(struct sd_cmd *cmd,uchar dma_err);
void sd_request_sense(struct sd_dasd_info *dp, struct sd_ctrl_info *cp,
        char type);
int sd_start_unit_disable(struct sd_dasd_info *dp, char start_flag);
int sd_start_unit(struct sd_dasd_info *dp, char start_flag);
void sd_test_unit_ready(struct sd_dasd_info *dp, uchar cinfo);
void sd_reserve(struct sd_dasd_info *dp, uchar cinfo);
void sd_fence(struct sd_dasd_info *dp,uchar type);
void sd_mode_sense(struct sd_dasd_info *dp);
void sd_mode_select(struct sd_dasd_info *dp);
void sd_format_mode_data(char *mode_data,struct sd_mode_format *mf, 
	int sense_length);
int sd_mode_data_compare(struct sd_dasd_info *dp);
void sd_inquiry(struct sd_dasd_info *dp);
void sd_read_cap_disable(struct sd_dasd_info *dp, char just_cylinder);
void sd_read_cap(struct sd_dasd_info *dp, char just_cylinder);
void sd_release_disable(struct sd_dasd_info *dp, uchar cinfo);
void sd_release(struct sd_dasd_info *dp, uchar cinfo);
void sd_send_msg(struct sd_dasd_info *dp, uchar message_code);
void sd_reset_quiesce_disable(struct sd_adap_info *ap, uchar op, uchar type);
void sd_reset_quiesce(struct sd_adap_info *ap, uchar op, uchar type);
void sd_flush_adap(struct sd_adap_info *ap);
void sd_flush_ctrl(struct sd_ctrl_info *cp);
void sd_fail_adap_disable(struct sd_adap_info *ap);
void sd_fail_adap(struct sd_adap_info *ap);
void sd_fail_dasd(struct sd_dasd_info *dp);
void sd_halt_adap_disable(struct trb *t);
void sd_halt_adap(struct trb *t);
void sd_verify_disable(struct sd_adap_info *ap, char type);
void sd_verify(struct sd_adap_info *ap, char type);
int  sd_check_map(struct sd_adap_info *ap, struct sd_ctrl_info *cp,
                  struct sd_dasd_info *dp, char type);
void sd_async_event(struct sd_adap_info *ap, uchar address, uchar event,int signal);
void sd_sleep(struct sd_adap_info *ap,uchar *intrpt, uint *event);
void sd_ioctl_timer(struct watchdog *w);
void sd_add_chain(struct sd_dasd_info *dp);
void sd_del_chain(struct sd_dasd_info *dp, char force);
void sd_build_cmd(struct sd_cmd *cmd, uint flags, uint dma_addr);
int  sd_pio_recov(struct sd_adap_info *ap, int exception, uchar op,
        void *ioaddr, long parm, uchar vol);
int  sd_pio(struct sd_adap_info *ap, void *ioaddr, char op, long data);
int  sd_epow(struct intr *is);
void sd_trc_disable(struct sd_adap_info *ap, char *desc, char *type,
		    char count, uint word1, uint word2,
		    uint word3, uint word4, uint word5);
void sd_trc(struct sd_adap_info *ap,char *desc, char *type,char count, 
	    uint word1, uint word2,
	    uint word3, uint word4, uint word5);
void sd_dptrc_disable(struct sd_dasd_info *dp,char *desc, char *type,
	char count, uint word1);
void sd_dptrc(struct sd_dasd_info *dp,char *desc, char *type,char count, 
	uint word1); 
struct cdt *sd_adap_cdt_func(int arg);
struct cdt *sd_ctrl_cdt_func(int arg);
struct cdt *sd_dasd_cdt_func(int arg);
void sd_walk_event(struct sd_adap_info *ap);
void sd_log_error(struct sd_cmd *cmd);
void sd_get_scsi_length(struct sc_error_log_df *log,uchar scsi_op_code);
void sd_log_dasd_err(struct sd_dasd_info *dp, ushort uec);
void sd_log_ctrl_err(struct sd_ctrl_info *cp, ushort uec);
void sd_log_adap_err(struct sd_adap_info *ap, ushort uec, uchar address);
int sd_log_limit(struct sd_logbook *lb, uchar etype, ushort uec);
void sd_shift_uecs(struct sd_logbook *lb, uchar etype, ushort uec);
void sd_delay(struct sd_adap_info *ap, char type, uint seconds);
void sd_delay_timer(struct trb *t);
int sd_concurrent(struct conc_cmd *ccptr);
void sd_q_conc_cmd (struct conc_cmd *ccptr, struct sd_dasd_info *dp);
struct conc_cmd *sd_d_q_conc_cmd (struct sd_dasd_info *dp);
void sd_build_conc_cmd (struct sd_dasd_info *dp, struct conc_cmd *ccptr);
void sd_return_conc_cmd (struct sd_dasd_info *dp, struct conc_cmd *ccptr);
/*
 *        SDSTRATEGY.C
 */
int  sd_strategy(struct buf *bp);
void sd_insert_q(struct sd_dasd_info *dp, struct buf *bp);
void sd_dequeue(struct sd_dasd_info *dp);
/*
 *        SDACTIVITY.C
 */
void sd_activity(struct watchdog *w);
int sd_act_fill(struct sd_adap_info *ap, int j);
void sd_query_timer(struct watchdog *w);
void sd_reset_check(struct trb *t);
/*
 *        SDDUMP.C
 */
int sd_dump(dev_t devno,struct uio *u, int cmd, int *arg,int chan, int ext);
int sd_dumpstrt(uint iocc,uint seg,struct sd_dasd_info *dp);
int sd_dmp_quiesce(struct sd_cmd *cmd, struct sd_dasd_info *dp, uint seg_base,
        uint iocc_seg);
int sd_dumpwrt(uint iocc,uint seg,struct sd_dasd_info *dp, struct uio *uiop);
int sd_dump_dev(struct sd_dasd_info *dp, uint seg,uint iocc,struct iovec *iovp,
        int lba);
int sd_dump_read(uchar *tag,struct sd_dasd_info *dp, uint seg_base,
        uint iocc_seg,int *loop,struct iovec *iovp);
void sd_dump_complete(struct sd_cmd *cmd,struct sd_adap_info *ap,
        struct iovec *iovp,struct sd_dasd_info *dp);
int sd_dump_reqsns(struct sd_dasd_info *dp,uint seg);
int sd_dump_retry(uchar tag,struct sd_dasd_info *d, uint seg,struct iovec *iovp);
void sd_dumpend(struct sd_dasd_info *dp);
/*
 *        SDIOCTL.C
 */
int sd_ioctl(dev_t devno, int op,int arg,ulong devflag,int chan,int ext);
int sd_adap_ioctl(struct sd_adap_info *ap,int op,dev_t devno,
	struct sd_iocmd *a,struct sd_ioctl_parms
        *b,struct devinfo *c,struct sd_event *event,
	struct sd_daemon_errlog *d_errlog,ulong devflag,
        int chan,int ext,uint time_out,int daemon_pid);
int sd_adapter_info(struct devinfo *arg,ulong devflag,int chan,int ext,
        struct sd_adap_info *ap,int op);
int sd_adap_dwnld(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,ulong devflag,
        int chan,int ext,struct sd_adap_info *ap,int op);
int sd_adap_tr_snpsht(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,ulong
        devflag,int chan,int ext,struct sd_adap_info *ap,int op);
int sd_adap_qry_trc(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,
        ulong devflag,int chan,int ext,struct sd_adap_info *ap, int op);
int sd_adap_inqry(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,ulong devflag,
        int chan,int ext,struct sd_adap_info *ap,int op);
int sd_adap_set_parms(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,
        ulong devflag,int chan,int ext,struct sd_adap_info *ap, int op);
int sd_adap_read_id(struct sd_ioctl_parms *arga,ulong devflag,
        int chan,int ext,struct sd_adap_info *ap,int op);
int sd_adap_mbox(struct sd_cmd *cmd,struct sd_ioctl_parms *arga,ulong devflag,
        int chan,int ext,struct sd_adap_info *ap,int op);
int sd_get_asynch(struct sd_event *event,ulong devflag,int chan,int ext,
	struct sd_adap_info *ap,int op);
int sd_daemon_log(struct sd_daemon_errlog *d_errlog,ulong devflag,int chan,
        int ext,struct sd_adap_info *ap,struct sd_cmd *cmd,int op);
int sd_ctrl_ioctl(struct sd_ctrl_info *cp,int op,dev_t devno,
	struct sd_iocmd *a,struct sd_ioctl_parms
        *b,struct devinfo *c,ulong devflag, int chan,int ext, uint time_out);
int sd_ctlr_info(struct devinfo *arga,ulong devflag,int chan,int ext,
        struct sd_ctrl_info *cp,int op);
int sd_dasd_ioctl(struct sd_dasd_info *dp,int op,dev_t devno,
	struct sd_iocmd *a,struct sd_ioctl_parms
        *b, struct dd_conc_register *conc_register,
	struct devinfo *c,ulong devflag, int chan,int ext,uint time_out);
int sd_dasds_info(struct devinfo *arg,ulong devflag,int chan,int ext,
        struct sd_dasd_info *dp,int op);
int sd_dasd_qry_dev(struct sd_ioctl_parms *arg,ulong devflag,int chan,int ext,
        struct sd_dasd_info *dp,struct sd_cmd *cmd,int op);
struct sd_cmd *sd_get_struct(struct sd_adap_info *ap);
int sd_ioctl_reset(struct sd_ioctl_parms *arga,ulong devflag,int chan,int ext,
        struct sd_adap_info *ap,struct sd_cmd *cmd,int op,int devtp,int signal,
		   int *async_flg,struct sd_adap_info **device,uchar *address,
		   uchar *event,int ctrl_dwnld);
int sd_ioctl_scsicmd(struct sd_iocmd *arga,ulong devflag,int chan,int ext,
        struct sd_adap_info *ap,struct sd_cmd *cmd,int op,int dev,int *dwnld,
	int *reset_flg);
int sd_ioctl_finish(int op,struct sd_iocmd *iocmd,
        struct sd_ioctl_parms *ioctl_parms, struct sd_cmd *cmd,
        struct sd_adap_info *ap,struct sd_dasd_info *dp,int num,uint time_out,
        int dwnld,int reset_flg,int async_flg,struct sd_adap_info *device,
		    uchar address,uchar event,ulong devflag);
int sd_ioctl_wait(struct sd_cmd *cmd,struct sd_adap_info *ap,
		  struct sd_dasd_info *dp,int flag,uint time_out,int async_flg,
		  struct sd_adap_info *device,uchar address,uchar event,
		  int chk_type);
int sd_copyin( char * arga, char * argb, int length,ulong devflag);
int sd_copyout(char * arga, char * argb, int length,ulong devflag);
int sd_prepare_dma(struct sd_cmd *cmd,uint length,int min_length,char *buff,
        ulong devflag);
void sd_ioctl_status(struct sd_cmd *cmd,struct sd_iocmd *arg);
int sd_ioctl_download(struct sd_cmd *cmd,struct sd_adap_info *ap,
                      struct sd_dasd_info *dp,int num,int time_out,
		      int reset_flg);
void sd_ioctl_verify_dasd(int ctrl_cmd, struct sd_adap_info *ap);
void sd_ioctl_free(int op,ulong devflag,struct sd_iocmd *iocmd,
		   struct sd_ioctl_parms *ioctl_parms,struct sd_cmd *cmd);
/*
 *       SDGFREE.S
 */
int sd_getfree(int word);

#else

/*
 *         SDCONFIG.C
 */
int sd_config();
int sd_adap_config();
int sd_ctrl_config();
int sd_dasd_config();


/*
 *         SDOPEN.C
 */
int sd_open();
int sd_adap_open();
int sd_ctrl_open();
int sd_dasd_open();
/*
 *         SDCLOSE.C
 */
int sd_close();
int sd_adap_close();
int sd_ctrl_close();
int sd_dasd_close();
/*
 *         SDRDWR.C
 */
int sd_read();
int sd_write();
int sd_mincnt();
/*
 *         SDUTILT.C
 */
struct sd_adap_info *sd_alloc_adap();
void sd_free_adap();
struct sd_ctrl_info *sd_alloc_ctrl();
void sd_free_ctrl();
struct sd_dasd_info *sd_alloc_dasd();
void sd_free_dasd();
int sd_setup_adap();
int sd_get_vpd();
/*
 *         SDINTR.C
 */
int sd_intr();
void sd_parse_err();
int sd_parse_ready_async();
void sd_alert_notag();
void sd_prepare_buf_retry();
void sd_fail_buf_cmd();
void sd_process_complete();
void sd_process_error();
void sd_process_scsi_error();
void sd_process_sense();
void sd_process_buf();
void sd_process_reset();
/*
 *         SDSTART.C
 */
void sd_start_disable();
void sd_start();
int sd_coalesce();
int sd_relocate();
int sd_start_cmd();
/*
 *         SDUTILB.C
 */
void *sd_hash();
int sd_read_POS();
int sd_write_POS();
int sd_reload_pos();
void sd_wait_reset();
void sd_wait_reset_disable();
int sd_restart_adap();
void sd_q_cmd_disable();
void sd_q_cmd();
void sd_d_q_cmd_disable()
void sd_d_q_cmd();
struct sd_event *sd_event_alloc();
struct sd_cmd *sd_cmd_alloc_disable();
struct sd_cmd *sd_cmd_alloc();
void sd_free_cmd_disable();
void sd_free_cmd();
void sd_fail_cmd();
void sd_process_conc_cmd();
int sd_MB_alloc();
void sd_free_MB();
int sd_TCW_alloc();
int sd_TCW_realloc();
void sd_TCW_dealloc();
int sd_STA_alloc();
void sd_STA_dealloc();
int sd_set_adap_parms();
void sd_cmd_timer();
int sd_dma_cleanup();
void sd_request_sense();
int sd_start_unit_disable();
int sd_start_unit();
void sd_test_unit_ready();
void sd_reserve();
void sd_fence();
void sd_mode_sense();
void sd_mode_select();
void sd_format_mode_data();
int sd_mode_data_compare();
void sd_inquiry();
void sd_read_cap_disable();
void sd_read_cap();
void sd_release_disable();
void sd_release();
void sd_send_msg();
void sd_reset_quiesce_disable();
void sd_reset_quiesce();
void sd_flush_adap();
void sd_flush_ctrl();
void sd_fail_adap_disable();
void sd_fail_adap();
void sd_fail_dasd();
void sd_halt_adap_disable();
void sd_halt_adap();
void sd_verify_disable();
void sd_verify();
int  sd_check_map();
void sd_async_event();
void sd_sleep();
void sd_ioctl_timer();
void sd_add_chain();
void sd_del_chain();
void sd_build_cmd();
int  sd_pio_recov();
int  sd_pio();
int  sd_epow();
void sd_trc_disable();
void sd_trc();
void sd_dptrc_disable();
void sd_dptrc(); 
struct cdt *sd_adap_cdt_func();
struct cdt *sd_ctrl_cdt_func();
struct cdt *sd_dasd_cdt_func();
void sd_walk_event();
void sd_log_error();
void sd_get_scsi_length();
void sd_log_dasd_err();
void sd_log_ctrl_err();
void sd_log_adap_err();
int sd_log_limit();
void sd_shift_uecs();
void sd_delay();
void sd_delay_timer();
int sd_concurrent();
void sd_q_conc_cmd ();
struct conc_cmd *sd_q_conc_cmd ();
void sd_build_conc_cmd ();
void sd_return_conc_cmd ();
/*
 *        SDSTRATEGY.C
 */
int  sd_strategy();
void sd_insert_q();
void sd_dequeue();

/*
 *         SDACTIVITY.C
 */
void sd_activity();
int sd_act_fill();
void sd_exec_daemon();
void sd_query_timer();
void sd_reset_check();
/*
 *         SDDUMP.C
 */
int sd_dump();
int sd_dumpstrt();
int sd_dmp_quiesce();
int sd_dumpwrt();
int sd_dump_dev();
int sd_dump_read();
void sd_dump_complete();
int sd_dump_reqsns();
int sd_dump_retry();
void sd_dumpend();
/*
 *         SDIOCTL.C
 */
int sd_ioctl();
int sd_adap_ioctl();
int sd_adapter_info();
int sd_adap_dwnld();
int sd_adap_tr_snpsht();
int sd_adap_qry_trc();
int sd_adap_inqry();
int sd_adap_set_parms();
int sd_adap_read_id();
int sd_adap_mbox();
int sd_get_asynch();
int sd_daemon_errlog();
int sd_ctrl_ioctl();
int sd_ctlr_info();
int sd_dasd_ioctl();
int sd_dasds_info();
int sd_dasd_qry_dev();
struct sd_cmd *sd_get_struct();
int sd_ioctl_reset();
int sd_ioctl_scsicmd();
int sd_ioctl_finish();
int sd_ioctl_wait();
int sd_copyin();
int sd_copyout();
int sd_prepare_dma();
void sd_ioctl_status();
int sd_ioctl_download();
void sd_ioctl_verify_dasd();
void sd_ioctl_free();

/*
 *       SDGFREE.S
 */
int sdgfree();

#endif
#endif /* _H_SD */
