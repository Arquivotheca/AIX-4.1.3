/* @(#)catuser.h	1.3 6/14/91 14:47:22 */
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catuser.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#ifndef _H_CATUSER
#define _H_CATUSER
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* include file for use with the 'psca' special file			   */
/* For more detailed information, see the AIX Technical Reference	   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#include <sys/termio.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>			/* For DD_CAT */
#include <sys/comio.h>				/* For CIO_* */
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ioctl constants							   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#define CAT			('c'<<8)
#define CAT_RW_SRAM		(CAT|01)	/* Read/Write Shared RAM */
#define CAT_RUN_DIAG		(CAT|02)	/* not used */
 
#define CAT_DNLD		(CAT|04)	/* Download microcode */
#define CAT_POS_ACC		(CAT|05)	/* Access pos registers */
#define CAT_SET_ADAP		(CAT|06)	/* Set adapter parameters */
#define CAT_RESET_SUB		(CAT|11)	/* Reset subchannel */
#define CAT_SET_SUB		(CAT|12)	/* set subchannel number */
 
#define CAT_CULOADED		(CAT|19)	/* cu table loaded */
#define CAT_CU_NOTLOADED 	(CAT|20)	/* cu table not loaded */
#define CAT_NOPCA_BUF		(CAT|21)	/* no pca buffer available */
#define CAT_RESET_ALL		(CAT|22)	/* reset all subchannels */
#define CAT_CU_LOAD		(CAT|24)	/* load a CU table */
#define CAT_ADAP_INFO		(CAT|25)	/* get adapter information */
#define CAT_GET_VPD		(CAT|26)	/* get VPD data */
#define CAT_RESET_ADAP		(CAT|27)	/* reset adapter hardware */
#define DEBUG_SW		(CAT|28)	/* turns on/off debug prints */
 
#define RES_CLAW_SUBCHAN        (CAT| 33) 
#define CAT_DEBUG		(CAT|0277)	/* Set debug level(arg is int)*/
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * */
/*  Definition of the CATDD extension structure  */
/* * * * * * * * * * * * * * * * * * * * * * * * */
#define CAT_SEND_ATTN	0x10		/* send attention interrupt to S/370 */
 
typedef struct cat_write_ext {
	cio_write_ext_t	cio_ext;	/* common IO extension */
	int attn_int;			/* CATDD send attention int. field */
	uchar use_ccw;			/* 0  : no pass-thru of ccw during xmit 
					   ~0 : pass-thru... */
	uchar ccw;			/* ccw to pass-thru if use_ccw!=0 */
} cat_write_ext_t;
 
typedef struct cat_read_ext {
	cio_read_ext_t cio_ext;		/* common IO extension */
	uchar ccw;			/* ccw used by host */
} cat_read_ext_t;
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*	Declarations of structures used with ca ioctls */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define CAT_MAX_SC	256
#define CAT_MAX_VPD_LEN 256
#define	CAT_VPD_VALID	1
#define	CAT_VPD_INVALID	2
 
/*
** Vital Product Data structure.
*/
typedef struct cat_vital_product_data {
	ulong status;			/* VPD_STATUS value */
	ulong length;			/* number of bytes in vpd[] */
	uchar vpd[CAT_MAX_VPD_LEN];	/* VPD data */
} cat_vpd_t;
 
/*
** Structure definition for arg field of CAT_RW_SRAM ioctl op.
** Contains user buffer address, PSCA shared RAM offset, length
** and direction of transfer.  For now, the dma_mode field is
** not used.
*/
struct cat_rw_sram {
	uint status;			/* return code from xfer operation */
	uchar rd_wrt;			/* direction of transfer */
#define CAT_CHNL_RD		0	/* write to the adapter memory */
#define CAT_CHNL_RD_NO_FAIL	2	/* write to the adapter memory */
#define CAT_CHNL_WR		1	/* read from the adapter memory */
#define CAT_CHNL_WR_NO_FAIL	3	/* read from the adapter memory */
	uchar dma_mode;			/* unused, reserved */
#define	CAT_USE_DMA		1
#define CAT_USE_PIO		0
	ulong sram_offset;		/* offset into the adapter shared RAM */
	uchar *str_ptr;			/* user buffer address */
	ulong num_bytes;		/* length of transfer */
};
 
 
/*
** Structure definition for arg field of CAT_CU_LOAD ioctl op
** Used to download a Control Unit specific Command Decode Table
** to the adapter.
*/
struct cat_cu_load {
	uint status;		/* return code from download operation */
	uchar cu_type;		/* control unit type associated w/ this table */
	uchar overwrite;	/* if TRUE overwrite existing table */
	uchar *cu_addr;		/* user buffer address */
	uint length;		/* length of CU table */
};
 
 
/*
** Structure definition for arg field of CAT_DNLD ioctl op
** Used to download microcode to the adapter.
*/
struct cat_dnld	{
	uint status;		/* return code from download operation */
	uchar *mcode_ptr;	/* addr of user buffer with microcode */
	uint mcode_len;		/* microcode length */
};
 
 
/*
** Structure definition for arg field of CAT_POS_ACC ioctl op.
** Used to set/get the adapter POS registers.
*/
struct cat_pos_acc {
	uint status;		/* returned code from driver */
	ushort opcode;		/* CHNL_READ or CHNL_WRITE */
	uchar pos_reg;		/* register number, 0-7 */
	uchar pos_val;		/* POS value returned/to set */
};
 
 
/*
** Structure definition for arg field of CAT_SET_ADAP ioctl op
*/
struct cat_set_adap {
	uint status;			/* return code from driver */
	ushort num_sub;			/* Number of subchannels to authorize */
	struct cat_adapcfg *adap_param;	/* pointer to (struct cat_adapcfg *) */
};
 
 
/*
** Subchannel description. One for each subchannel authorized.
*/
struct cat_subchid {
	uchar subchan;			/* Subchannel address */
	uchar cutype[3];		/* CU tbl # (basic, extended, claw) */
};
 
/*
** Adapter Configuration structure.
** Used to configure the adapter for operation.
*/
typedef struct cat_adapcfg {
	uchar speed;			/* Channel speed to use */
#define CAT_DCLOCK	0
#define CAT_SPD_1_9	1
#define CAT_SPD_2_7	2
#define CAT_SPD_3_4	3
#define CAT_SPD_4_5	4
	uchar reserve[3];		/* Reserved for future use */
	ushort xmitsz;			/* Transmit buffer size */
	ushort recvsz;			/* Receive buffer size */
	ushort xmitno;			/* Number of transmit buffers */
	ushort recvno;			/* Number of receive buffers */
	ushort flags;			/* Flags */
	ushort nosubs;			/* Number of subchannels */
	struct cat_subchid subid[CAT_MAX_SC];	/* Subchannel descriptions */
} cat_adapcfg_t;
 
/*
** Claw, since we need  to validate the subchannels if not validated 
*/
 
typedef struct claw_sess_blk {
  int linkid;
  char WS_appl[8];
  char H_appl[8];
  char adap[8];
  char H_name[8];
};
 
/*
** Structure definition for arg field of CIO_START ioctl op.
** Subchannel configuration parameters, used to configure a
** subchannel for operation.
**
** NOTE: We should probably declare a SUBPARMS type for the
** subset, spec_mode, startde fields since these fields are
** eventually put into this data type by the driver.
*/
#define CAT_NETID_LENGTH	1
struct cat_set_sub {
	struct session_blk sb;		/* from comio.h */
	uchar subset;			/* # of subchannels in subchan group */
	uchar set_default;		/* if TRUE ignore following 3 fields */
	uchar specmode;			/* special mode indicator */
#define CAT_8232_MOD	0x00		/* 8232 mode (default) */
#define CAT_CLAW_MOD	0x01		/* CLAW mode */
#define CAT_CHNL_MOD	0x02		/* Asynchronous chan-to-chan mode */
#define CAT_3215_MOD	0x04		/* Printer/Keyboard 3215 mode */
#define CAT_ATTN_MOD	0x40		/* Give ATTN with all data to 370 */
#define CAT_FLUSHX_MOD	0x80		/* Flush xmit on reset, wait for ack */
	uchar shrtbusy;			/* short busy status */
#define CAT_SBALONE	0		/* short busy alone */
#define CAT_SB_SM	1		/* short busy & status modifier */
#define CAT_SB_CUE	2		/* short busy and control unit end */
	uchar startde;			/* start with unsolicited device end */
#define CAT_STARTDE	1		/* start with unsolicited device end */
	uchar scflags;			/* Reserved for future use */
#define NEW_SC_DEF	1		/* new definition included */
#define SET_DEFAULT	2		/* reset subch's in group to default */
struct claw_sess_blk  claw_blk;         /* uses with specmode = CAT_CLAW_MOD */
					/* "shrtbusy", "startde", and        */
					/* "scflags" fields will be ignor.   */
	uchar reserved;			/* Reserved for future use */
};
 
/*
** Structure passed as argument to CAT_RUN_DIAG ioctl operation.
** Identifies diagnostic test number and test loop count.
*/
struct cat_run_diag {
	int test_num;		/* Diagnostic test number */
	int times;		/* Test loop count */
	int status;		/* status returned by driver */
};
 
/*
** Structure passed as an argument to CAT_ADAP_INFO ioctl operation.
** The driver uploads the information from the adapter status area.
*/
struct cat_adap_info {
	uint status;			/* operational status, see pscadefs.h */
	ushort chanspd;			/* interface speed, see cat_adapcfg_t */
	ushort sbchact;			/* num. of active subchannels */
	ushort xbuflen;			/* length of adapter transmit buffer */
	ushort rbuflen;			/* length of adapter receive buffer */
	ushort xbufno;			/* num. of adapter transmit buffers */
	ushort	rbufno;			/* num. of adapter receive buffers */
	uchar	sub_status[CAT_MAX_SC];	/* status of each subchannel */
#define CAT_WSUSP_STAT	0x01		/* subchannel is 370 write suspended */
#define CAT_VTAM_STAT	0x02		/* subchannel is in VTAM header mode */
#define CAT_GRP_STAT	0x20		/* subchannel is part of a group */
#define CAT_ONLN_STAT	0x40		/* subchannel is on-line */
#define CAT_AUTH_STAT	0x80		/* subchannel is authorized */
	uchar opmode[CAT_MAX_SC];	/* operating mode of each channel */
	/*
	** See cat_set_sub special mode flags
	** for a definition of channel mode bits.
	*/
	uchar cutype[CAT_MAX_SC];	/* control unit type per subchannel */
};
 
/*
** Device Dependant Structure for the config function.
** See the PSCA Hardware Functional Spec for a description
** of the POS registers.
*/
struct cadds {
	/* BUS INFORMATION */
	int bus_id;	/* I/O bus ID (to pass to IOCC_ATT and BUSMEM_ATT) */
	int bus_type;	/* Bus type (BUS_MICRO_CHANNEL) for intr.h struct */
 
	/* ADAPTER INFORMATION */
	int slot_num;		/* microchannel slot card found in */
	ulong addr_bits;	/* POS register bits for base bus I/O address */
	int int_bits;		/* bus interrupt level */
	int intr_priority;	/* System interrupt priority */
	int dma_lvl;		/* to set arbitration level */
 
	/* DEVICE SPECIFIC INFORMATION */
	char resource_name[16];		/* Device logical name */
	ushort card_id;			/* Card ID returned by POS registers */
	int burst_bits;			/* Burst release */
	int parity_opts;		/* Sync err, data parity, addr parity*/
	int fair;			/* Fairness */
	int dma_enable;			/* enable DMA */
	int io_dma;			/* set I/O DMA */
	int rdto;			/* receive data transfer offset */
	char clawset[256];              /* define claw subchannel */
	cat_adapcfg_t config_params;	/* configuration parameters */
};
 
/*
** Device-specific statistics
** Returned in driver statistics structure.
*/
typedef struct cat_stats {
	ulong sta_que_overflow;		/* status lost, full status que */
	ulong rec_que_overflow;		/* receive packet lost, full recv que */
	ulong total_intr;		/* total interrupts */
	ulong intr_not_handled;		/* interrupts not handled, */
										/* ucode not functional */
	ulong recv_intr_cnt;		/* number of receive interrupts */
	ulong xmit_intr_cnt;		/* number of transmit interrupts */
	ulong rec_no_mbuf;		/* no mbuf available */
	ulong xmit_sent;		/* transmit commands send to PCA */
	ulong xmit_dma_completes;	/* transmit dma transfers completed */
	ulong recv_dma_completes;	/* receive dma transfers completed */
} cat_stats_t;
 
/*
** Driver statistics structure.
** Returned by CIO_QUERY ioctl operation
*/
typedef struct {
	cio_stats_t cc;			/* General COMIO statistics */
	cat_stats_t ds;			/* Device specific statistics */
} cat_query_t;
 
/*
** Possible values of status.  All 0's indicate success.
*/
#define DNLD_SUCC		0
#define READ_DONE		0
#define WRITE_DONE		0
#define DNLD_TIMEOUT		1
#define POST_TIME_OUT		2
#define POST_FAIL		3
#define DNLD_FAIL		4
#define NOT_DIAG_MODE		8
#define INVALID_CMD		10
#define BAD_RANGE		20
 
#endif	  /* _H_CATUSER */
