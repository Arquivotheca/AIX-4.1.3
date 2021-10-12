/* @(#)51	1.8  src/bos/kernext/disk/badiskdd.h, sysxdisk, bos411, 9428A410j 2/11/91 13:03:36 */
/*
 * COMPONENT_NAME: (SYSXDISK)  Bus Attached Disk Device Driver
 *
 * FUNCTIONS:  Header File for Bus Attached Disk Device Driver
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/xmem.h>
#include <sys/watchdog.h>
#include <sys/timer.h>
#include <sys/intr.h>
#include <sys/dump.h>
#include <sys/iostat.h>
#include <sys/err_rec.h>
#include <sys/badisk.h>

/***************************************************************************/
/*                        GENERIC DEFINES                                  */
/***************************************************************************/ 

#define OK               0	        /* Return code for successful comp */

/***************************************************************************/
/*                        SPECIFIC DEFINES                                 */
/***************************************************************************/
#define BA_DISK_ID   0x9FDF		                     /* ID of Disk */
#define BA_NDISKS     2		                /* Number of disks allowed */
#define BA_PIO_R      1               /* flag for 16 bit register transfers*/
#define BA_PIO_W      2               /* flag for 16 bit register transfers*/

/* Register offsets */
#define BA_BSR          2                  /* Basic Status Register offset */
#define BA_BCR          2                  /* Basic Control Register offset*/
#define BA_ISR    	3                   /* Interrupt Status Reg offset */
#define BA_ATR     	3	  	      /* Attention Register offset */
#define BA_SIR     	0		    /* Status Interface Reg offset */
#define BA_CIR     	0		   /* Command Interface Reg offset */

/* POS registers */
#define BA_POS0		0x400100	        /* POS reg 0 - Card ID Low */
#define BA_POS1		0x400101	       /* POS reg 1 - Card ID High */
#define BA_POS2		0x400102       /* POS reg 2 - Enable,Addr,DMA,fair */
#define BA_POS3		0x400103           /* POS reg 3 - Burst,DMA enable */
#define BA_POS4		0x400104          /* POS reg 4 - Pacing,Bus release*/
#define BA_POS2VAL         0x01                     /* Value for POS reg 2 */
#define BA_POS3VAL         0x60                     /* Value for POS reg 3 */
#define BA_POS4VAL         0x07                     /* Value for POS reg 4 */

/* Basic Status Reg Bit Assignments */
#define BA_DMA_STAT	0x80		        /* DMA enabled or disabled */
#define BA_INT_PEND	0x40		              /* Interrupt Pending */
#define BA_CIPS		0x20		            /* Command in Progress */
#define BA_BUSY		0x10	        /* Busy, cannot accept ATR request */
#define BA_STAT_OUT	0x08		             /* Status Block ready */
#define BA_CMD_IN	0x04		                 /* Command in CIR */
#define BA_XFER		0x02		        /* Ready for Data Transfer */
#define BA_IRPT		0x01		            /* Interrupt to system */

/* Basic Control Reg Bit Assignments */
#define BA_RESET  	0x80		                 /* Hardware Reset */
#define BA_INT_ENABLE	0x01		     /* Enable Attach to interrupt */
#define BA_INT_DISABLE	0x00		    /* Disable Attach to interrupt */
#define BA_DMA_ENABLE	0x02		            /* Enable DMA requests */
#define BA_DMA_DISABLE	0x00		           /* Disable DMA requests */

/* various defines for miscellaneous situations */
#define BA_ATTACH_ERROR    0x10      /* Attach Error, internal diags failed*/
#define BA_TO1200    1200                                 /* timeout value */
#define BA_TO100     100                                  /* timeout value */
#define BA_TO10      10                                   /* timeout value */
#define BA_DUMPTO10        10                        /* dump timeout value */
#define BA_DUMPTO200       200                       /* dump timeout value */
#define BA_DUMPTO450       450                       /* dump timeout value */
#define BA_BPS             512               /* number of bytes per sector */
#define BA_DEV_MASK        0xE0      /* mask to get device number from ISR */
#define BA_IRPT_CAUSE      0x0f    /* mask for cause of interrupt from ISR */
#define BA_LBMASK         0x00ff        /* mask to get Low byte of 16 bits */
#define BA_HBMASK         0xff00       /* mask to get High byte of 16 bits */
#define BA_LWMASK         0x0000ffff    /* mask to get Low word of 32 bits */
#define BA_HWMASK         0xffff0000   /* mask to get High word of 32 bits */
#define BA_1WORD          16                     /* bit length of one word */
#define BA_1BYTE          8                      /* bit length of one byte */
#define BA_POPTIONS        0x02               /* options for parking heads */
#define BA_PFCODE          0xAA55                   /* code for pre-format */
#define BA_DELAY_REG       0xe0                 /* Hardware delay register */
#define BA_ZERO            0x00                           /* ZERO constant */

/* Interrupt Definitions */
#define BA_COMP_SUCC      0x01	            /* Command Complete w/ Success */
#define BA_COMP_SUCC_ECC  0x03	           /* Command Complete w/ Succ,ECC */
#define BA_COMP_SUCC_RET  0x05         /* Command Complete w/ Succ,Retries */
#define BA_COMP_S_E_R     0x07	       /* Command Complete w/ Succ,ECC,ret */
#define BA_COMP_WARN      0x08	  	    /* Command Complete w/ Warning */
#define BA_COMP_FAIL      0x0C		    /* Command Complete w/ Failure */
#define BA_FORMAT_PCOMP   0x06		      /* Format Partially Complete */
#define BA_ABORT_COMP     0x09		                 /* Abort Complete */
#define BA_RESET_COMP     0x0A		                 /* Reset Complete */
#define BA_DTR	          0x0B		            /* Data Transfer Ready */
#define BA_DMA_ERROR      0x0D		                      /* DMA Error */
#define BA_COMMAND_ERROR  0x0E		            /* Command Block Error */
#define BA_ATTEN_ERROR    0x0F		                /* Attention Error */

/* Attention Request Codes */
#define BA_ACCEPT_CMD   0x01		   /* Request Attach to accept CB  */
#define BA_EOI		0x02	     /* End of Interrupt - System Complete */
#define BA_ABORT_CMD	0x03	       /* Tells attach to gracefully abort */
#define BA_SOFT_RESET  	0x04		     /* Tells attach to soft reset */

#define BA_HARD_RESET   0x05                            /* Hard Reset flag */

/* Commands */
#define BA_READ_DATA	0x0100	 	              /* Read Data command */
#define BA_READ_VERIFY	0x0300		               /* Read with Verify */
#define BA_WRITE_DATA	0x0200		                     /* Write Data */
#define BA_WRITE_VERIFY	0x0400		              /* Write With Verify */
#define BA_SEEK_CMD    	0x0500		                           /* Seek */
#define BA_PARK_HEADS	0x0600		       /* Park Heads in Safe Place */
#define BA_PREFORMAT	0x1700		                 /* Format Prepare */
#define BA_FORMAT_CMD	0x1600		                 /* Perform Format */
#define BA_DIAG_TEST	0x1200		            /* Run Diagnostic Test */
#define BA_GET_DIAG_STAT 0x1400		    /* Get Diagnostic Status Block */
#define BA_BUFFW_CMD  	0x1000		        /* Write attachment buffer */
#define BA_BUFFR_CMD  	0x1100		         /* Read Attachment Buffer */
#define BA_GET_MFG	0x1500		                 /* Get MFG header */
#define BA_GET_CC_STAT	0x0700	      /* Get Command Complete Status Block */
#define BA_GET_DEV_STAT	0x0800		              /* Get Device Status */
#define BA_GET_DEV_CONFIG 0x0900	       /* Get Device Configuration */
#define BA_GET_POS_INFO	0x0A00		   /* Get POS register information */
#define BA_TRANS_RBA	0x0B00	       /* Translate Relative Block Address */

/* Device Numbers for CMD and STAT blocks */
#define BA_DISK		0x0000		         /* Device select for disk */
#define BA_ATTACH	0xE000  	   /* Device select for attachment */
#define BA_OPTIONS      0x06		                    /* Option bits */
#define BA_TWO_WRDS	0x00		        /* Length of command block */
#define BA_FOUR_WRDS	0x40		        /* Length of command Block */
#define BA_FILL_WORD    0x0000             /* Fill word for command Blocks */

/************* Error Logging and Error Recovery Defines ********************/
/* Error Recovery Procedure Definitions */ 
#define BA_NONE            0x00             /* No error recovery procedure */
#define BA_LR1             0x01                 /* Log Error, Retry 1 time */
#define BA_LR4             0x02                /* Log Error, Retry 4 times */
#define BA_LR2B            0x03   /* Log Error, Retry 2 times, Map Bad RBA_*/
#define BA_LAR3            0x04     /* Log Error, Abort, and Retry 3 times */
#define BA_LOG		   0x05  /* Log Status Block data in Sys Error Log */
#define BA_LHR1            0x09     /* Log Error, Hard Reset, retry 1 time */
#define BA_SILENT_RETRY    0x0A                   /* Retry without logging */
#define BA_MORE_INFO       0x0B         /* Need more info to process error */
/* Error Type Definitions */
#define BA_SE		0x01                            /* Soft Read Error */
#define BA_HE           0x02                            /* Hard Read Error */
#define BA_SC           0x03                       /* Soft Equipment Chech */
#define BA_HC		0x04                       /* Hard Equipment Check */
#define BA_AE		0x05                    /* Device Attachment Error */
#define BA_SK           0x08                                 /* Seek Error */
/* Device Status Codes */
#define BA_WRITE_FAULT_EM 	0x04       /* Write Fault Emulated  - This */
                                        /* represents bit 10 of the device */
                                        /* status word, not hex code 04    */
/* Device Error Codes */
#define BA_SEEK_FAULT	      	0x01                         /* Seek Fault */
#define BA_INTERFACE_FAULT	0x02                    /* Interface Fault */
#define BA_BLK_NOT_FOUND_ID	0x03             /* Block not found ( ID ) */
#define BA_BLK_NOT_FOUND_AM     0x04             /* Block not found ( AM ) */
#define BA_DATA_ECC_ERROR	0x05       /* Data ECC Error (Hard Error ) */
#define BA_ID_CRC_ERROR        	0x06                       /* ID CRC Error */
#define BA_RBA_OUTOF_RANGE     	0x07               /* RBA_ is out of range */
#define BA_SELECT_ERR		0x0B                    /* Selection Error */
#define BA_WRITE_FAULT		0x0D                        /* Write Fault */
#define BA_READ_FAULT 		0x0E                         /* Read Fault */
#define BA_NO_INDEX_SECPULSE	0x0F           /* No index or sector pulse */
#define BA_DEVICE_NOT_READY    	0x10                   /* Device not ready */
#define BA_SEEK_ERROR           0x11                         /* Seek Error */
#define BA_NO_DATA_AM		0x14                   /* No Data AM Found */
#define BA_NOIDAM_OR_IDECCERR   0x15   /* No Data AM or ID ECC Error found */
#define BA_NO_ID_FOUND 		0x18               /* No ID found on track */
/* Command Error Codes */
#define BA_INVALID_PARM		0x01   /* Invalid Parameter in Command Blk */
#define BA_CMD_NOT_SUPPORTED	0x03              /* Command Not Supported */
#define BA_CMD_ABORTED		0x04        /* Command Aborted (by System) */
#define BA_CMD_REJECTED		0x06                   /* Command Rejected */
#define BA_FORMAT_REJECTED	0x07  /* Format Rejected (Sequence Error ) */
#define BA_FORMAT_ERROR_DE      0x0A      /* Format Error (Diagnose error) */
#define BA_FORMAT_ERROR_HCS	0x0D       /* Format Error (Host Checksum) */
#define BA_FORMAT_WARNING_PTO 	0x0F /*Format Warning (Push Table Overflow)*/
#define BA_FORMAT_WARNING_15P	0x10   /* Format Warning ( > 15 Pushes/cyl */
#define BA_FORMAT_WARNING_VEF   0x12     /*Format Warn, Verify Errors found*/
#define BA_INVALID_DEV		0x13         /* Invalid Device for Command */
/* Retry Defines */
#define BA_LR1_MAX_RETRIES 	0x01                               /* Once */
#define BA_LR2_MAX_RETRIES      0x02                              /* Twice */
#define BA_LR3_MAX_RETRIES      0x03                             /* Thrice */
#define BA_LR4_MAX_RETRIES      0x04                              /* frice */


/***************************************************************************/
/*                               STRUCTURES                                */
/***************************************************************************/

struct ba_trace {                                        /* trace structure */
	char desc[10];       /* ASCII description of what this entry is for */
	uint ptr;         /* some address whose meaning depends on the desc */
	};

struct ba_cmd_blk {                    /* info for 16 bit CIR/SIR transfers */
       char wc;                     /* word count - number to write or read */
       char current;                            /* current word of transfer */
       char reg;                 /* flag to specify which register - SIR/CIR*/
       ushort w[10];                                      /* array of words */
       void (*store)();                   /* routine to store incoming data */
       };

struct ba_error_log_df {             /* Bus Attached disk error log record */
       uint error_type;                                      /* error type */
       char resource_name[ERR_NAMESIZE];                  /* resource name */
       uint bucket_count;                 /* which bucket error occured in */
       uint byte_count;    /* byte count within bucket when error occurred */
       struct badisk_cstat_blk sense_data; /* sense data result from error */
       };

struct ba_watchdog {                        /* personal watchdog structure */
	struct watchdog watch;	                /* watchdog time structure */
	ulong     ba_pointer;                 /* pointer to my info struct */
	};


struct ba_info {
	/* 
	 * interrupt structure, THIS MUST BE THE FIRST ITEM 
	 * IN THIS STRUCTURE IN ORDER FOR THE ALGORITHM TO WORK 
	 */
	struct 	intr intr_st;	
        struct  ba_watchdog mywdog;               /* regular watchdog timer */
        struct  ba_watchdog idle;               /* watchdog for device idle */
	dev_t 	devno;			        /* This disks device number */
	struct buf *head;		    /* Pointer to head of command Q */
	struct buf *tail;		    /* Pointer to tail of command Q */
	struct xmem *mp;		         /* Cross memory descriptor */
	char 	*target_buff;	    /* Buffer address used in data transfer */
	int	chained;	 	 /* Number of buf structs coalesced */
	int	dma_flags;	 	 /* DMA flags for d_slave transfers */
	int	dma_chn_id;		                  /* DMA Channel Id */
	int 	byte_count;	 	  /* Current # of bytes to transfer */
	int 	rba;    	        /* Relative Block Address for trans */
	int 	resid;    	              /* Residual Value after error */
	int 	idle_seek_rba;                  /* target RBA for idle seek */
	int 	bytes_sent;		  /* Current # of bytes transferred */
	int	target_xfer;	  	    /* Number of blocks to transfer */
        int     format_count;           /* count of cylinders during format */
	int	dump_ilevel;             /* old interrupt level before dump */
        uint    pio_address;                         /* address of PIO xfer */
        uint    seg_base;       /* base address after reserving segment reg */
        uint    iocc_seg;    /* address after reserving segment reg for iocc*/
	ushort 	lcmd_dev;	                 /* last cmd target device  */
        ushort  lint_dev;                               /* last irpt device */
        ushort  last_cmd;                            /* last command issued */
        ushort  pio_data;                              /* data for PIO xfer */
	char  	idle_seek_inprog;	           /* idle seek in progress */
	char  	idle_seek_cmpl;	                      /* idle seek complete */
	char  	retry_count;	           /* retry count for io operations */
        char    erp;                            /* error recovery procedure */
        char    etype;                                     /* type of error */
        char    state;                             /* state of pio transfer */
	char 	opened;		    	             /* flag if disk opened */
	char 	buf_complete;	        /* Transfer of buf request complete */
	char	dma_active;		              /* flag DMA initiated */
	char 	rwop;    	               /* flag read/write operation */
	char 	atre;                               /* flag attention error */
	char 	w_waitcc;                   /* flag watchdog waiting for cc */
	char 	l_waitcc;                    /* flag logging waiting for cc */
	char 	command_cmpl;	                /* flag if command complete */
	char 	diagnos_mode;                 /* flag if in diagnostic mode */
        char    reloc;                           /* flag relocate bad block */
        char    dumpdev;                        /* flag whether dump device */
        char    epow_pend;                     /* flag whether EPOW pending */
        char    pio_op;                         /* programmed I/O operation */
        char    pio_size;                 /* size of PIO xfer, byte or word */
        struct ba_cmd_blk blk;                   /* command block structure */
        struct ba_error_log_df elog;                  /* error logging data */
 	struct ba_dds dds;	                 /* configuration structure */
	struct badisk_spec ba_cfig;	                  /* disk specifics */
 	struct badisk_cstat_blk  ba_ccs; /*cmd complete status block struct */
 	struct badisk_dstat_blk  ba_dsb; /*diagnostic status block structure*/
 	struct badisk_pos_regs  ba_pos;           /* POS register structure */
        struct trb *piowtrb;        /* timer structure for 16 bit PIO xfers */
        struct trb *irpttrb;  /* timer structure for i_level interrupt poll */
        struct trb *cmdtrb;           /* timer structure for command issues */
        struct dkstat dkstat;                /* IOSTAT statistics structure */
	};


#ifndef _NO_PROTO
int ba_config(dev_t devno,int op,struct uio *uiop);
int ba_open(dev_t devno,int rwflag,int chan,int ext);
int ba_close(dev_t devno,int chan,int ext);
int ba_read(dev_t devno,struct uio *uiop,int chan,int ext);
int ba_write(dev_t devno,struct uio *uiop,int chan,int ext);
int ba_strategy(struct buf *bp);
int ba_ioctl(dev_t devno,int op,int arga,ulong flag);
int ba_epow(struct intr *is);
int ba_intr(struct intr *is);
int ba_dump(dev_t devno,struct uio *uiop,int cmd,char *arg,int chan,int ext);
struct cdt *ba_dump_func(int arg);
void ba_get_specs(struct ba_info *ba);
void ba_get_ccs(struct ba_info *ba);
void ba_get_dsb(struct ba_info *ba);
void ba_get_pos(struct ba_info *ba);
int ba_issue_cmd(struct ba_info *ba,int op,ushort device);
int ba_mincnt(struct buf *bp,void *minparms);
int ba_pos_setup(struct ba_info *ba);
void ba_pos_clean();
void ba_log_error(struct ba_info *ba);
void ba_process_error(struct ba_info *ba);
int ba_sendeoi(caddr_t parms);
int ba_eoirecov(caddr_t parms,int op,struct pio_except *infop);
int ba_piodelay(caddr_t parms);
int ba_piodrcv(caddr_t parms,int op,struct pio_except *infop);
int ba_busystat(caddr_t parms);
int ba_bstatrecov(caddr_t parms,int op,struct pio_except *infop);
int ba_piofunc(caddr_t parms);
int ba_piorecov(caddr_t parms,int op,struct pio_except *infop);
int ba_start(struct ba_info *ba);
void ba_start_nextio(struct ba_info *ba);
int ba_dma_cleanup(struct ba_info *ba);
void ba_piowxfer(struct trb *t);
void ba_waitirpt(struct trb *t);
void ba_cmdxfer(struct trb *t);
void ba_idle(struct watchdog *wp);
void ba_watch(struct watchdog *wp);
#else
int ba_config();
int ba_open();
int ba_close();
int ba_read();
int ba_write();
int ba_strategy();
int ba_ioctl();
int ba_epow();
int ba_intr();
int ba_dump();
struct cdt *ba_dump_func();
void ba_get_specs();
void ba_get_ccs();
void ba_get_dsb();
void ba_get_pos();
int ba_issue_cmd();
int ba_mincnt();
int ba_pos_setup();
void ba_pos_clean();
void ba_log_error();
void ba_process_error();
int ba_sendeoi();
int ba_eoirecov();
int ba_piodelay();
int ba_piodrcv();
int ba_busystat();
int ba_bstatrecov();
int ba_piofunc();
int ba_piorecov();
int ba_start();
void ba_start_nextio();
int ba_dma_cleanup();
void ba_piowxfer();
void ba_waitirpt();
void ba_cmdxfer();
void ba_idle();
void ba_watch();
#endif 

/*
 * Macro to swap bytes of a word
 */
#define BA_SWAPB(x) ((x>>BA_1BYTE)&BA_LBMASK)|((x<<BA_1BYTE)&BA_HBMASK)

/*
 * Macro for debug print statements
 */

#ifdef DEBUG
#define BADBUG(args) if(badebug) printf args 
#endif

/*
 * Macro for Internal Trace
 */

#define BADTRC(string,address)  ba_trace[ba_trcindex].desc[0] = string[0]; \
			        ba_trace[ba_trcindex].desc[1] = string[1]; \
			        ba_trace[ba_trcindex].desc[2] = string[2]; \
			        ba_trace[ba_trcindex].desc[3] = string[3]; \
			        ba_trace[ba_trcindex].desc[4] = string[4]; \
			        ba_trace[ba_trcindex].desc[5] = string[5]; \
			        ba_trace[ba_trcindex].desc[6] = string[6]; \
			        ba_trace[ba_trcindex].desc[7] = string[7]; \
			        ba_trace[ba_trcindex].desc[8] = string[8]; \
			        ba_trace[ba_trcindex].desc[9] = string[9]; \
			        ba_trace[ba_trcindex].ptr = address; \
		                if(++ba_trcindex >= TRCLNGTH) ba_trcindex = 0;\
		                ba_trctop = \
				   (struct ba_trace *)&ba_trace[ba_trcindex]
