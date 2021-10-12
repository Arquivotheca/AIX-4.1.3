/* @(#)96	1.4.1.1  src/bos/usr/include/diag/har2_atu.h, dasdisk, bos411, 9428A410j 8/10/93 09:09:47 */
/*
 *   COMPONENT_NAME: TU_SDISK
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**********************************************************************/
/*                                                                    */
/* Source file name = har2_atu.h                                      */
/*                                                                    */
/* This C-Language program is an include file for the 9333 ATU   */
/* source code (har2_atu.c).                                          */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/* include files for 9333 ATU's                                  */
#include <stdio.h>
#include <nl_types.h>
#include <memory.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>


/* defines for test unit numbers                                      */
#define SCATU_TEST_UNIT_READY             (0x01)
#define SCATU_REQUEST_SENSE               (0x02)
#define SCATU_RESERVE_UNIT                (0x03)
#define SCATU_MODE_SELECT                 (0x04)
#define SCATU_SEND_DIAGNOSTIC             (0x05)
#define SCATU_RELEASE_UNIT                (0x06)
#define SCATU_MODE_SENSE                  (0x07)
#define SCATU_INQUIRY                     (0x08)
#define SCATU_RECEIVE_DIAGNOSTIC_RESULTS  (0x09)
#define SCATU_START_STOP_UNIT             (0x0b)
#define SCATU_RANDOM_SEEK_TEST            (0x10)
#define SCATU_FORMAT_UNIT                 (0x17)
#define SCATU_REASSIGN_BLOCK              (0x18)
#define SCATU_MFG_CERTIFY_UNIT            (0x19)
#define SCATU_READ_CAPACITY               (0x70)
#define SCATU_READ_EXTENDED               (0x71)
#define SCATU_ENABLE_SOFT_ERRORS          (0x72)
#define SCATU_DISABLE_SOFT_ERRORS         (0x73)
#define SCATU_WRITE_EXTENDED              (0x74)
#define SCATU_BREAK_FENCE                 (0x0c)


/* return codes from HAR2 ATUs                                        */
#define SCATU_GOOD                           (0)
#define SCATU_TIMEOUT                      (-10)
#define SCATU_RESERVATION_CONFLICT         (-11)
#define SCATU_CHECK_CONDITION              (-12)
#define SCATU_COMMAND_ERROR                (-13)
#define SCATU_BUSY                         SCATU_COMMAND_ERROR
#define SCATU_BAD_REQUEST_SENSE_DATA       (-14)
#define SCATU_ODM_FAIL                     (-94)
#define SCATU_QUEUE_FULL                   (-95)
#define SCATU_NONEXTENDED_SENSE            (-96)
#define SCATU_IO_BUS_ERROR                 (-97)
#define SCATU_ADAPTER_FAILURE              (-98)
#define SCATU_BAD_PARAMETER                (-99)

#define uchar unsigned char

struct tucb_t {
        long tu;             /* test unit number                      */
        long mfg;            /* 0 = normal,  1 = manufacturing        */
        long loop;           /* # of times to loop on timeouts, 0=1   */
        long r1;             /* reserved for future use               */
        long r2;             /* reserved for future use               */
};

struct sctu_x {
        uchar resvd5;        /* target address for the passthru command */
        long data_length;    /* number of bytes to transfer           */
        uchar *data_buffer;  /* pointer to data buffer for cmd */
        int cmd_timeout;     /* max length to wait for command compl  */
        int command_length;  /* number of bytes in the command        */
        uchar scsi_cmd_blk[12]; /* the scsi command itself    */
        char flags;          /* flags described in SCSI device driver */
                             /* 'no disconnect', 'no negotiations',   */
                             /* 'read', and 'write'                   */
        int seed_value;      /* seed value for random number generator*/
        int ioctl_pass_param;/* parameter for passthrough ioctl       */
};

struct sctu_r {
        int sense_key;       /* sense_key from request sense          */
        int sense_code;      /* sense_code from request sense         */
        int host_action_code;/* host_action_code from request sense   */
        int unit_error_code; /* unit_error_code from request sense    */
        int rec_errs;        /* # of recovered errors during test     */
        int soft_thres_errs; /* # of soft data threshold exceeded     */
        int soft_equip_chks; /* # of soft equipment checks            */
        int unrec_errs;      /* # of unrecovered errors during test   */
        int hard_data_errs;  /* # of hard data errors during test     */
        int hard_equip_chks; /* # of hard equipment checks            */
        unsigned reas_lba;   /* Reassigned LBA                        */
        unsigned blks_read;  /* # of blocks certified during test     */
        int atu_num;         /* ATU number executed during test       */
        int status_validity; /* 0 = no valid status                   */
                             /* 1 = valid SCSI bus status             */
                             /* 2 = valid adapter status              */
        int scsi_bus_status; /* status byte from device (if valid)    */
        int adapter_status;  /* status from adapter (if valid)        */
        uchar   alert_register_adap; /* Byte 2 of alert register      */
        uchar   alert_register_cont; /* Byte 3 of alert register      */
};

#define HAR2_TUTYPE struct _sctus

struct _sctus {
        struct tucb_t header;/* which tu do I run?                    */
        struct sctu_x scsitu;/* input values                          */
        struct sctu_r scsiret; /* returned values                     */
};
