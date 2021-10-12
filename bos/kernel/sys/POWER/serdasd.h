/* @(#)13	1.9  src/bos/kernel/sys/POWER/serdasd.h, sysxdisk, bos411, 9428A410j 11/6/93 10:00:45 */
#ifndef _H_SERDASD
#define _H_SERDASD
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem
 *
 * FUNCTIONS:  Header File for Serial Dasd Subsytem
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/scsi.h>
#include <sys/scdisk.h>
#include <sys/ddconc.h>


/***************************************************************************/
/*                      OPENX EXTENDED PARAMETERS                          */
/***************************************************************************/ 

/* These are in addition to those defined in scsi.h */
#define SD_NO_RESERVE	SC_NO_RESERVE        /* Don't reserve DASD on open */
#define SD_DAEMON	SC_RESV_05	     /* Daemon open                */


/***************************************************************************/
/*                             IOCTLS                                      */
/***************************************************************************/ 

#define SD_SCSICMD		DKIOCMD	  /* SCSI Pass Thru Command        */
#define SD_RESET		0x71      /* Reset                         */
#define SD_ADAP_DOWNLOAD	0x72      /* Adapter Microcode Download    */
#define SD_ADAP_TRACE_SNAPSHOT	0x73	  /* Adapter Trace Snapshot        */ 
#define SD_ADAP_QUERY_TRACE     0x74      /* Adapter Query Trace           */
#define SD_ADAP_INQUIRY       	0x75      /* Adapter Inquiry               */
#define SD_SET_ADAP_PARMS	0x76      /* Set Adapter Parameters        */
#define SD_QUERY_DEVICE		0x77      /* Query Device                  */
#define SD_READ_ADAP_ID		0x78	  /* Read Adapter POS ID           */
#define SD_GET_EVENT		0x79      /* Get first Asynch event        */
#define SD_DAEMON_ERROR         0x7a      /* For daemon to errlog events   */
#define SD_MBOX			0x7B      /* Pass Thru Mailbox             */
#define SD_SET_FENCE		0x7C      /* Set fence for a DASD          */
#define SD_CLEAR_FENCE		0x7D      /* Clear fence for a DASD        */


/***************************************************************************/
/*                       MISCELLANEOUS DEFINES                             */
/***************************************************************************/ 





/* Additional SCSI commands */
#define SD_WRITE_SAME		0x41		/* Write Same SCSI Command */
#define SD_FENCE_OP_CODE        0xD0            /* Pseudo scsi fence cmd   */

/* Fence command modifiers  */

#define SD_FENCE_MASK_SWAP      0x01      /* Mask and swap Fence Command   */
#define SD_FENCE_COMPARE_SWAP   0x02      /* Compare and swap Fence Command*/

/* Support Operations for the SCSI_SEND_DIAGNOSTIC command */
#define SD_WRITE_VPD		0x80	  /* Write DASD Product VPD        */
#define SD_RESET_DASD		0x81	  /* Reset DASD                    */
#define SD_SET_TRACE_SNAPSHOT	0x82	  /* Set trace snapshot parameters */
#define SD_PREPARE_TRACE_STATUS 0x83	  /* Prepare Trace Status          */
#define SD_PREPARE_TRACE_DUMP	0x84	  /* Prepare Trace Dump            */
#define SD_SELF_TEST_BIT	0x04	  /* Self Test bit of cmd byte 1   */

/* Adapter POS ID */
#define SD_ADAP_ID		0x788F	  /* POS ID of adapter             */

/* Device type identifiers */
#define	SD_ADAPTER		0x00
#define SD_CONTROLLER		0x01
#define SD_DASD			0x02

/* Adapter VPD size */
#define SD_ADAP_VPD_SIZE	255

/*
 *  Adapter query trace length
 */
#define SD_ADAP_DUMP_LENGTH     0x1200

/***************************************************************************/
/*                       ADAPTER STATUS BYTE CODES                         */
/***************************************************************************/ 

#define SD_SCSI_STATUS		0x00    /* Controller SCSI Status          */
#define SD_CTRL_STATUS		0x01    /* Special Controller Status       */
#define SD_CMD_TO_ACTIVE_TAG	0x02	/* Command to an active tag        */
#define SD_DNLD_ADAP_BUSY	0x03    /* Download with adapter not quiet */
#define SD_ILLEGAL_ADAP_CMD	0x04	/* Illegal Adapter Command         */
#define SD_CTRL_NOT_RESPONDING	0x05	/* Controller not responding       */
#define SD_UNIT_POWERED_ON	0x06	/* Unit Powered On                 */
#define SD_TIME_OUT		0x07	/* Time out (only for query device */
#define SD_OPEN_LINK		0x08    /* open link / ctrl powered down   */
#define SD_RESET_PURGE_TAG	0x09    /* Tag purged following reset      */
#define SD_UNEXP_SCSI_STATUS	0x0A	/* unexpected SCSI status          */
#define SD_UNEXP_CTRL_STATUS	0x0B	/* unexpected special status       */
#define SD_DATA_DMA_FAIL	0x0C	/* Data DMA Failed                 */
#define SD_MB_DMA_FAIL		0x0D	/* Mailbox DMA failed              */
#define SD_INVALID_MB_PTR	0x0E	/* Invalid Mailbox Pointer         */
#define SD_DMA_COUNT_ERROR	0x0F	/* DMA Count Error                 */
#define SD_BAD_CTRL_ADDRESS	0x10	/* out of range controller address */
#define SD_QUIESCE_TIME_OUT	0x11	/* quiesce time out                */
#define SD_RECOV_LINK_ERROR	0x12    /* recoverable link error          */
#define SD_DNLD_MC_CHKSUM_FAIL	0x13	/* Download microcode checksum fail*/
#define SD_MSG_TO_INACTIVE_TAG	0x14	/* Message to an inactive tag      */
#define SD_MSG_WRONG_LINK	0x15	/* Message on wrong link           */
#define SD_INVALID_LINK_MSG	0x16	/* Invalid link message            */
#define SD_LINK_ERROR		0x17	/* Unrecoverable link error        */
#define SD_INVALID_ADAP_PARMS	0x18	/* Invalid adapter parameters      */
#define SD_INVALID_TAG		0x19	/* Invalid Tag                     */
#define SD_DMA_DISABLED		0x1D	/* DMA disabled, retrying          */
#define SD_ASYNCHRONOUS_EVENT	0x1F	/* Asynchronous event occured      */
#define SD_WBUFF_STARTED	0x20	/* Write Buffer Started            */
#define SD_MB_TERM_INTERN_RST	0x21	/* MB terminated by internal reset */
#define SD_ADAP_HDW_ERROR	0x22	/* Adapter Hardware Error          */
#define SD_TRACE_DATA_DMA_ERR	0x24	/* Trace data DMA error            */
#define SD_SYSTEM_DMA_HUNG	0x25	/* system DMA hung                 */
#define SD_TRACE_SUPERCEDED	0x26    /* snapshot conditions superceded  */
#define SD_PREV_TRC_DUMP_BUSY	0x27	/* Previous Trace Dump Busy        */

#define SD_DD_PURGED_TAG	0x01   /* Device driver failed command    */

/***************************************************************************/
/*                       CONTROLLER STATUS BYTE CODES                      */
/***************************************************************************/ 

#define	SD_ABORTED_CMD		0x81	 /* aborted command                */
#define SD_INVALID_QC		0x82	 /* invalid queue control          */
#define SD_PURG_OS		0x83	 /* purged after outstanding sense */
#define SD_PURG_EPOW		0x84     /* purged after EPOW              */
#define SD_WBUFF_INPROG		0x87	 /* Write Buffer in Progress       */
#define SD_INVALID_MSG		0x88	 /* invalid message                */
#define SD_INVALID_MB		0x89	 /* Invalid Mailbox                */
#define SD_NO_VALID_DELAY	0x8A	 /* No Valid Delay                 */


/***************************************************************************/
/*                       SCSI STATUS BYTE CODES                            */
/***************************************************************************/ 

#define SD_GOOD		SC_GOOD_STATUS	  /* target completed successfully */
#define SD_CHECK	SC_CHECK_CONDITION /* target is reporting an error,*/
				       /* exception, or abnormal condition */
#define SD_RES_CONFLICT	SC_RESERVATION_CONFLICT	/* LUN is reserved by other*/
					        /* initiator */
#define SD_QUEUE_FULL	0x28   		/* Not an error, but controller's  */
				                          /* queue is full */
#define SD_FENCED_OUT   0x3E             /* fenced out                     */

/***************************************************************************/
/*                        STRUCTURES                                       */
/***************************************************************************/

/*
 * Adapter Error Log Structure
 */
struct sd_adap_error_df {
        uint    error_id;                       /* error type id             */
        char    resource_name[ERR_NAMESIZE];    /* dds resource name         */
        ushort  unit_err_code;                  /* UEC                       */
        uchar   validity;                       /* validity byte             */

#define SD_POS5_VALID           0x01            /* POS5 data valid           */
#define SD_ALERT_VALID          0x02            /* Alert contents valid      */
#define SD_DMA_RC_VALID         0x04            /* System DMA rc valid       */
#define SD_CHANNEL_STATUS_VALID 0x08            /* Channel Status valid      */

        uchar   pos_reg_5;                      /* contents of POS reg 5     */
        ulong   alert_reg;                      /* contents of Alert Reg     */
        uint    sys_dma_rc;                     /* return code from sys dma  */
        uint    chan_status_reg;                /* Channel status register   */
};

/*
 * Serial DASD Subsytem specific IOCTL Parameter structure
 */
struct sd_ioctl_parms {
        uint                data_length;    /* length of data for transfer */
        char                *buffer;        /* pointer to data buffer      */
        uint                time_out;       /* time out value for this cmd */
	                                    /* in seconds                  */
        uchar               reset_type;     /* Full Reset or Quiesce       */
#define SD_RESET_OP		0x00
#define SD_QUIESCE_OP		0x01
#define SD_DASD_RESET		0x02

        uchar               status_validity;            
#define SD_NO_STATUS		0x00        /* successful completion      */
#define SD_VALID_ADAP_STATUS   	0x01	    /* Adapter Status is valid    */
#define SD_VALID_CTRL_STATUS  	0x02	    /* Controller Status is valid */
#define SD_DRIVER_STATUS        0x04        /* Device Status only         */
        uchar               adapter_status; /* Adapter Status byte        */
        uchar               controller_status;  /* Controller Status byte */
        uint                resvd1;         /* reserved, various uses     */
        uint                resvd2;         /* reserved, various uses     */
        uint                resvd3;         /* reserved, various uses     */
        uint                resvd4;         /* reserved, various uses     */
        uint                resvd5;         /* reserved, various uses     */
        uint                resvd6;         /* reserved, various uses     */
        uint                resvd7;         /* reserved, various uses     */
};

/*
 * Structure to address data returned by fence command.
 */

struct sd_fence_info
{
    ushort fence_posn;                      /* fence position indicator    */
    ushort fence_oldvalue;                  /* old value of fence register */
};

/*
 * Serial DASD Subsytem SCSI like Pass-through Ioctl Structure
 */
struct sd_iocmd {
        uint   data_length;          /* Bytes of data to be transfered */
        char   *buffer;              /* Pointer to transfer data buffer */
        uint   timeout_value;        /* In seconds */
        uchar  status_validity;      /* 0 = no valid status */
	/* 1 = valid SCSI bus status only       */
	/* 2 = valid adapter status only        */
/* Addition to status_validity definitions for sc_iocmd in scsi.h */
#define SD_VALID_ALERTREG	0x04 /* Valid Alert Register Contents   */

        uchar  scsi_bus_status;      /* SCSI bus status (if valid) See  */
				     /* SCSI Status Byte Code defines   */
				     /* above.				*/
        uchar  adapter_status;       /* Adapter status (if valid), refer*/ 
				     /* to sc_buf definition 		*/
	uchar  resvd1;               /* Returned as the controller 	*/
				     /* status byte. See Controller	*/
				     /* Status Byte Codes defines above.*/
	uchar  resvd2;               /* Returned as the adapter 	*/
				     /* status byte. See Adapter	*/
				     /* Status Byte Codes defines above.*/
	uchar  resvd3;               /* reserved for future expansion 	*/
	uchar  resvd4;               /* reserved for future expansion 	*/     
        uchar  flags;                /* B_READ, B_WRITE 		*/
	uchar  resvd5;		     /* Specifies the address of the    */
				     /* device.				*/
	uchar  resvd6;               /* Queue Control 			*/
#define SD_Q_NONE	0x00	     /* Unqueued command      		*/
#define SD_Q_INVALID	0x40	     /* Invalid        			*/
#define SD_Q_ORDERED	0x80	     /* Ordered Command        		*/
#define SD_Q_UNORDERED	0xC0	     /* Unordered Command      		*/
	uchar  resvd7;               /* SCSI extension 			*/
#define SD_NO_EXT	0x00	     /* No extension            	*/
#define SD_SPLIT_WRITE	0x20	     /* Split Write Enabled     	*/
#define SD_SPLIT_READ	0x10	     /* Split Read Enabled      	*/

        uchar  command_length;       /* Length of SCSI command block */
        uchar  scsi_cdb[12];         /* SCSI command descriptor block */
};
#endif /* _H_SERDASD */
