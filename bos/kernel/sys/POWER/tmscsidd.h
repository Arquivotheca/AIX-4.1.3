/* @(#)12	1.6  src/bos/kernel/sys/POWER/tmscsidd.h, sysxtm, bos411, 9428A410j 10/18/93 13:58:21 */
#ifndef _H_TMSCSIDD
#define _H_TMSCSIDD
/*
 * COMPONENT_NAME: (SYSXTM) IBM SCSI Target Mode Header File
 *
 * FUNCTIONS: NONE
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
/************************************************************************/
/*                                                                      */
/*  SOURCE FILE:        tmscsidd.h                                      */
/*                                                                      */
/*  NAME:       SCSI target mode driver include paramaters.             */
/*                                                                      */
/*  NOTE:       This header file contains the definition of the         */
/*              structures which are passed from the tm driver          */
/*              to the SCSI adapter driver.                             */
/*                                                                      */
/************************************************************************/

#include <sys/scsi.h>
#include <sys/tmscsi.h>


/************************************************************************/
/* Misc. Defines */
/************************************************************************/
#define	K		   1024
#define	TCWRANGE	   DMA_PSIZE
#define TM_HASH_SIZE       16		/* size of has table */
#define TM_HASH_NUMBER     0xf		/* one less than hash size */
#define TM_WAKEUP          0x01
#define TM_NOWAKEUP        0x02
#define TM_ADAPTER_ERROR   0x01
#define TM_SCSI_ERROR      0x02
#define	MAX_SENSE_LEN	   256
#define	TM_DEFAULT_DELAY   2
#define	TM_DEFAULT_TIMEOUT 10
#define	TM_RS_TIMEOUT	   30
/* for debug purposes only */
/* #define TMIOBRKPT          0xbbb */

/********** structure for retry_delay timer ****************************/
struct tm_timer {
	struct	watchdog dog;
	struct	tm_dev_def *tdp;
};

/************************************************************************/
/* Initialization information                                           */
/************************************************************************/
struct tm_ddi	{
        dev_t           adapter_devno;		/* adapter major/minor  */
        uchar           scsi_id;                /* SCSI ID for tm/im    */
        uchar           lun_id;                 /* SCSI LUN for im dev  */
	char		resource_name[16];	/* name, for err logging*/
	uint		buf_size;		/* size,in bytes, for   */
						/*  each receive buffer */
	uint		num_bufs;		/* num bufs to allocate */
        int             int_prior;              /* interrupt priority   */
                                                /* from parent adapter  */
	uint		resvd1;			/* reserved, must be 0  */
};

/************************************************************************/
/* Driver Operation descriptor block (includes sc_buf)                  */
/************************************************************************/
struct tm_cmd {
        struct sc_buf	scb;
        struct tm_dev_def *tdp;
        uchar		retry_flag;
        uchar		retry_count;
        uchar		type;
#define TM_REQSENSE_CMD    0x01
#define TM_OTHER_CMD       0x02
	uchar		resvd1;
	uchar		old_status_validity;
	uchar		old_scsi_status;
	uchar		old_general_card_status;
	uchar		old_b_error;
};

/************************************************************************/
/* Information on individual devices                                    */
/************************************************************************/
struct tm_dev_def {
	struct	tm_timer tm_wdog;
	uint		num_reads;	/* num of reads to this dev */
        dev_t           devno;		/* devno of this device     */
	uint		max_xfer;	/* max transfer size	    */
        uchar		opened;		/* This device is open	    */
        uchar           cmd_state;      /* device's current state   */
#define	TM_IDLE		0x00
#define	TM_READ_ACTIVE	0x01
#define	TM_WRT_ACTIVE	0x02
#define	TM_RS_ACTIVE	0x04
#define	TM_PASS_THRU	0x08
#define TM_INTERRUPTED	0x10
#define TM_CMD_DONE	0x40
#define TM_CMD_ERROR	0x80

        uchar           retry_pending;  /* this device has a retry      */
                                        /* currently pending            */

	uint		count_non_open_bufs;	/* count bufs thrown on */
					/*   the floor while not opened */
	ushort		timeout_value;	/* timeout value		*/
	uchar		timeout_type;	/* scaled/fixed timeout value	*/
	uchar		retry_delay;	/* delay between retrys(0-255)	*/
	uchar		event_flag;	/* what events selected by user	*/
	uchar		async_flag;	/* sync or async initiator cmds */
	int		async_events;	/* reported by adapter		*/
	int		recv_event;	/* event word for recv bufs	*/
        uint		lock_word;	/* device lock word 		*/
	int		num_free_bufs;	/* no. of bufs on the free list */
	int		num_to_free;	/* num to be piled up before free */
	int		num_bufs_qued;	/* num bufs qued after last wakeup */
	int		num_to_wakeup;	/* num to be qued before wakeup */
	int		previous_error;	/* save earlier error status    */
#define	TM_SFW_ERR	0x01		/* driver detected logic error  */
#define	TM_SYS_ERR	0x02		/* kernel system call failure   */
	void		(*buf_free)();	/* adapter's buf free function	*/
        struct file	*fp;		/* file pointer for scsi DD     */
        struct tm_ddi	ddi;		/* ddi info for with device	*/
	struct tm_buf	*head_free;	/* head of free buffer list 	*/
	struct tm_buf	*tail_free;	/* tail of free buffer list	*/
	struct tm_buf	*head_rdbufs;	/* read bufs qued for this dev	*/
	struct tm_buf	*tail_rdbufs;	/* tail of read bufs list	*/
	struct tm_get_stat iostatus;	/* status of previous command 	*/
        struct tm_cmd	cmd_pb;		/* scsi command parameter block */
        struct tm_cmd	rscmd_pb;	/* request sense cmd parameter block */
	struct sc_error_def error_rec;  /* error logging structure      */
        struct tm_dev_def *next;	/* pointer to next in hash chain*/
        char		rs_buf[MAX_SENSE_LEN];	/* sense data area */
        Simple_lock tm_mp_lock;         /* MP lock per tm device        */
};

#endif /* _H_TMSCSIDD */
