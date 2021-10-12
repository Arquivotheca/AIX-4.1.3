/* @(#)28       1.6  src/bos/usr/include/diag/scsi_atu.h, tu_scsi, bos411, 9428A410j 4/29/94 14:58:07 */
/*
 * COMPONENT_NAME: tu_scsi
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*   FILE NAME: scsi_atu.h                                                */
/*   FUNCTION:  SCSI Application Test Units header file.                  */
/*                                                                        */
/*              This header file is used to define the SCSI Application   */
/*              Test Unit interface values and structures.                */

#ifndef _H_SCSI_ATU
#define _H_SCSI_ATU


/* defines for test unit numbers */
#define SCATU_TEST_UNIT_READY   (0x01)
#define SCATU_REQUEST_SENSE     (0x02)
#define SCATU_RESERVE_UNIT      (0x03)
#define SCATU_MODE_SELECT       (0x04)
#define SCATU_SEND_DIAGNOSTIC   (0x05)
#define SCATU_RELEASE_UNIT      (0x06)
#define SCATU_MODE_SENSE        (0x07)
#define SCATU_INQUIRY           (0x08)
#define SCATU_RECEIVE_DIAGNOSTIC_RESULTS (0x09)
#define SCATU_WRITE             (0x0a)
#define SCATU_START_STOP_UNIT   (0x0b)
#define SCATU_PLAY_AUDIO        (0x0c)
#define SCATU_AUDIO_TRACK_SEARCH        (0x0d)
#define SCATU_PLAY_AUDIO_TRACK_INDEX    (0x0e)
#define SCATU_RANDOM_SEEK_TEST  (0x10)
#define SCATU_FORMAT_UNIT       (0x11)
#define SCATU_REASSIGN_BLOCKS   (0x12)
#define SCATU_WRITE_AND_VERIFY  (0x13)
#define SCATU_LOAD_UNLOAD_UNIT  (0x14)
#define SCATU_READ_CAPACITY     (0x15)
#define SCATU_READ_EXTENDED     (0x16)
#define SCATU_WRITE_EXTENDED    (0x17)
#define SCATU_PREVENT_ALLOW_REMOVAL     (0x18)
#define SCATU_USER_DEFINED      (0xFF)     /* User Defined - Passthrough   */

/* return codes from SCSI ATUs */
#define SCATU_GOOD                      (0)
#define SCATU_TIMEOUT                   (-10)
#define SCATU_RESERVATION_CONFLICT      (-11)
#define SCATU_CHECK_CONDITION           (-12)
#define SCATU_COMMAND_ERROR             (-13)
#define SCATU_BUSY                      SCATU_COMMAND_ERROR
#define SCATU_BAD_REQUEST_SENSE_DATA    (-14)
#define SCATU_NONEXTENDED_SENSE         (-96)
#define SCATU_IO_BUS_ERROR              (-97)
#define SCATU_ADAPTER_FAILURE           (-98)
#define SCATU_BAD_PARAMETER             (-99)


/* include file for scsi test units */
struct tucb_t {         /* tucb_t should come from atu.h ???            */
        long tu;        /* test unit number                             */
        long mfg;       /* 0 = normal, 1 = manufacturing                */
        long loop;      /* Number of times to loop on timeouts, 0 = 1   */
        long r1;        /* reserved for future use                      */
        long r2;        /* reserved for future use                      */
};

struct sctu_x {
        long data_length;       /* number of bytes to transfer               */
        unsigned char *data_buffer; /* pointer to data buffer for command    */
        int cmd_timeout;        /* max length to wait for command complete   */
        int command_length;     /* number of bytes in the command            */
        unsigned char scsi_cmd_blk[12]; /* the scsi command itself           */
        char flags;             /* flags described in SCSI device driver doc */
                                /* 'no disconnect', 'no negotiations',       */
                                /* 'read', and 'write'                       */
        int seed_value;         /* seed value for random number generator    */
        int ioctl_pass_param;   /* parameter for passthrouth ioctl           */
      uchar lun;                /* target's lun                              */
};

struct sctu_r {
        int sense_key;          /* sense_key from request sense              */
        int sense_code;         /* sense_code from request sense             */
        int rec_errs;           /* number of recovered errors during test    */
        int unrec_errs;         /* number of unrecovered errors during test  */
        int status_validity;    /* 0 = no valid status                       */
                                /* 1 = valid SCSI bus status                 */
                                /* 2 = valid adapter status                  */
        int scsi_bus_status;    /* status byte from device (if valid)        */
        int adapter_status;     /* status from adapter (if valid)            */
};

#define SCSI_TUTYPE struct _sctus

struct _sctus {
        struct tucb_t header;   /* which tu do I run?   */
        struct sctu_x scsitu;   /* input values         */
        struct sctu_r scsiret;  /* returned values      */
};


#endif /* _H_SCSI_ATU */
