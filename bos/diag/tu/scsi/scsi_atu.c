static char sccsid[] = "@(#)00  1.8  src/bos/diag/tu/scsi/scsi_atu.c, tu_scsi, bos41J, 9520A_a 5/11/95 16:34:13";
/*
 * COMPONENT_NAME: tu_scsi
 *
 * FUNCTIONS: exectu, do_command, scsitu_init
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
/*   FILE NAME: scsi_atu.c                                                  */
/*   FUNCTION:  SCSI Application Test Units.                              */
/*                                                                        */
/*              This source file contains all the code for the SCSI       */
/*              Application Test Units used for diagnostics of SCSI       */
/*              devices.  Command structures are taken from the           */
/*              diagnostic applications and passed to the device          */
/*              driver.  The device driver operates in "passthrough"      */
/*              mode.  The SCSI command is transferred to the             */
/*              device driver for the resident SCSI adapter card.         */
/*                                                                        */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  ioctl(), memcpy(), memset(),            */
/*                                time(), getpid(),                       */
/*                                rand(), srand()                         */
/*        in DEBUGSCSITU mode:  printf()                                  */



#ifndef _C_SCSI_ATU
#define _C_SCSI_ATU

/* #define DEBUGSCSITU */

/* INCLUDED FILES */

#include <sys/types.h>
#define ulong_t unsigned long
#include <stdio.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#include <sys/buf.h>
#include <sys/devinfo.h>
#include <sys/scsi.h>
/* #include <sys/scdisk.h> */

#include <diag/scsi_atu.h>

/* END OF INCLUDED FILES  */

#ifdef DEBUGSCSITU
 extern int errno;
 extern char *sys_errlist[]; /* system defined err msgs, indexed by errno    */
 extern int sys_nerr;        /* max value for errno                          */
#endif

/************************************************************************/
/* Function Declarations                                                */
/************************************************************************/

/* externally visible */
int             exectu();
int             scsitu_init();

/* local */
static int      do_command();




/************************************************************************/
/* Initialize values                                                    */
/************************************************************************/


/*
 * NAME: exectu
 *
 * FUNCTION:  Execute a specific SCSI Test Unit.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called as a subroutine of a diagnostic
 *      application.  The file associated with fdes must have been
 *      opened in SC_DIAGNOSTIC mode with an openx() call.
 *
 * NOTES:  This routine will accept commands to perform specific test
 *         units on SCSI devices.  Supported test units are:
 *
 *         #0x01      -  TEST UNIT READY
 *         #0x02      -  REQUEST SENSE
 *         #0x03      -  RESERVE UNIT
 *         #0x04      -  MODE SELECT
 *         #0x05      -  SEND DIAGNOSTIC
 *         #0x06      -  RELEASE UNIT
 *         #0x07      -  MODE SENSE
 *         #0x08      -  INQUIRY
 *         #0x09      -  RECEIVE DIAGNOSTIC RESULTS
 *         #0x0A      -  WRITE
 *         #0x0B      -  START / STOP UNIT
 *         #0x0C      -  PLAY AUDIO
 *         #0x0D      -  AUDIO TRACK SEARCH
 *         #0x0E      -  PLAY AUDIO TRACK INDEX
 *         #0x10      -  Random seek test
 *         #0x11      -  FORMAT UNIT
 *         #0x12      -  REASSIGN BLOCKS
 *         #0x13      -  WRITE AND VERIFY
 *         #0x14      -  LOAD / UNLOAD UNIT
 *         #0x15      -  READ CAPACITY
 *         #0x16      -  READ EXTENDED
 *         #0x17      -  WRITE EXTENDED
 *         #0x18  24  -  PREVENT/ALLOW MEDIA REMOVAL
 *
 *         #0xFF 255  -  USER DEFINED - (passthrough mode)
 *
 * DATA STRUCTURES:
 *      tucb_ptr - input structure, contains details of TU to be run
 *      iocmd    - structure that is passed as a parameter to the
 *                 device driver via ioctl().
 * INPUTS:
 *      fdes     - file descriptor of SCSI device.
 *      tucb_ptr - structure with details of test unit to be run.
 *                 NOTE:  tucb_ptr->scsitu.ioctl_pass_param MUST be
 *                        set by diagnostic application.
 *
 *
 * RETURNS:
 * RETURN VALUE DESCRIPTION:
 *      SCATU_GOOD                   - command completed successfully
 *      SCATU_TIMEOUT                - command timed out
 *      SCATU_RESERVATION_CONFLICT   - device is reserved for another
 *                                     initiator
 *      SCATU_CHECK_CONDITION        - device is indicating a
 *                                     check condition status
 *      SCATU_COMMAND_ERROR          - the command completed with errors
 *                                     this includes a 'busy' status
 *      SCATU_BAD_REQUEST_SENSE_DATA - request sense data obtained during
 *                                     the RANDOM SEEK TEST test unit
 *                                     indicated a condition other than
 *                                     recoverable or nonrecoverable
 *                                     data error
 *      SCATU_NONEXTENDED_SENSE      - data obtained from a request
 *                                     sense command was not in
 *                                     extended format
 *      SCATU_IO_BUS_ERROR           - Microchannel I/O failure
 *      SCATU_ADAPTER_FAILURE        - failure of the SCSI adapter
 *      SCATU_BAD_PARAMETER          - invalid input parameter
 *
 * EXTERNAL PROCEDURES CALLED:  memcpy(), rand(), srand(),
 *      in DEBUGSCSITU mode: printf()
 *
 * LOCAL PROCEDURES CALLED:  do_command()
 *      NOTE:  exectu() uses one level of recursion for the
 *             RANDOM SEEK TEST test unit
 */

int
exectu(fdes, tucb_ptr)
        int             fdes;           /* file descriptor of device      */
        SCSI_TUTYPE    *tucb_ptr;       /* details of test unit to be run */
{
        struct sc_iocmd iocmd;  /* this goes to the device driver head    */
        int             command_stat;   /* temporarily store status of
                                         * command    */
        int             loopcount;      /* times to retry */
        int             rand(); /* generates a random number 0 < x < 2^31 */
        void            srand();/* initialized the random numbers         */
        int             logical_unit;   /* logcial unit of device */
        int    i;       /* work variable */

#ifdef DEBUGSCSITU
        printf("\n>> in exectu <<\n");
#endif

        memset(&iocmd,0,sizeof(iocmd));     /* clear all of iocmd area */

        /* number of times to repeat a command */
        /* zero means one */
        loopcount = tucb_ptr->header.loop;
        if (loopcount == 0)
                loopcount = 1;

        iocmd.data_length = tucb_ptr->scsitu.data_length;
        iocmd.buffer = tucb_ptr->scsitu.data_buffer;
        if (iocmd.buffer == NULL)
                iocmd.data_length = 0;
        iocmd.timeout_value = tucb_ptr->scsitu.cmd_timeout;
        iocmd.status_validity = 0;
        iocmd.scsi_bus_status = 0;
        iocmd.adapter_status = 0;
        iocmd.flags = tucb_ptr->scsitu.flags;
        iocmd.command_length = tucb_ptr->scsitu.command_length;
        iocmd.lun = tucb_ptr->scsitu.lun;

        tucb_ptr->scsiret.sense_key = 0;
        tucb_ptr->scsiret.sense_code = 0;
        tucb_ptr->scsiret.rec_errs = 0;
        tucb_ptr->scsiret.unrec_errs = 0;
        tucb_ptr->scsiret.status_validity = 0;
        tucb_ptr->scsiret.adapter_status = 0;
        tucb_ptr->scsiret.scsi_bus_status = 0;

        if (tucb_ptr->scsitu.command_length > 12) {
                return (SCATU_BAD_PARAMETER);   /* command too long */
        }

        /* copy the command over */
        memcpy(iocmd.scsi_cdb, tucb_ptr->scsitu.scsi_cmd_blk,
               tucb_ptr->scsitu.command_length);
        iocmd.command_length = tucb_ptr->scsitu.command_length;

        /* get the logical unit */
        logical_unit = (tucb_ptr->scsitu.scsi_cmd_blk[1] >> 5) & 0x07;

#ifdef DEBUGSCSITU
        printf("ioctl iocmd initial values---------------------------------\n");
        printf("  data_length = %d   ", iocmd.data_length);
         printf("timeout_value = %d   ", iocmd.timeout_value);
          printf("status_validity = %d   ", iocmd.status_validity);
           printf("flags = %d\n", iocmd.flags);

        printf("scsi_bus_status = %d   ", iocmd.scsi_bus_status);
         printf("adapter_status = %d   ", iocmd.adapter_status);
          printf("command length = %d\n", iocmd.command_length);
        printf("initial command = ");
        {
                int             itemp;
                itemp = 0;
                while (itemp < iocmd.command_length) {
                        printf(" %02.2x",
                               tucb_ptr->scsitu.scsi_cmd_blk[itemp++]);
                }
        }
        printf("\n copied command = ");
        {
                int             itemp;
                itemp = 0;
                while (itemp < iocmd.command_length) {
                        printf(" %02.2x", iocmd.scsi_cdb[itemp++]);
                }
        }
        printf("\n");
#endif

        switch (tucb_ptr->header.tu) {

        case SCATU_LOAD_UNLOAD_UNIT:     /* LOAD - UNLOAD UNIT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_UNLOAD)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_TEST_UNIT_READY:     /* TEST UNIT READY */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_TEST_UNIT_READY)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_REQUEST_SENSE:       /* REQUEST SENSE */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length > 255)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_REQUEST_SENSE)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }

                if (command_stat == SCATU_GOOD) {
                                tucb_ptr->scsiret.sense_key =
                                        iocmd.buffer[2] & 0x0f;
                                tucb_ptr->scsiret.sense_code =
                                        (uchar) iocmd.buffer[13] |
                                        (unsigned) (
                                             (uchar) iocmd.buffer[12]) << 8;
                }
                return (command_stat);
                break;

        case SCATU_RESERVE_UNIT:        /* RESERVE UNIT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_RESERVE_UNIT)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_MODE_SELECT:/* MODE SELECT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length < iocmd.scsi_cdb[4])
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_MODE_SELECT)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_SEND_DIAGNOSTIC:     /* SEND DIAGNOSTIC */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length < (iocmd.scsi_cdb[4] +
                                       ((unsigned) iocmd.scsi_cdb[3] << 8)))
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_SEND_DIAGNOSTIC)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_REASSIGN_BLOCKS:     /* REASSIGN BLOCKS */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_REASSIGN_BLOCK)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_RELEASE_UNIT:        /* RELEASE UNIT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_RELEASE_UNIT)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_MODE_SENSE:  /* MODE SENSE */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length < iocmd.scsi_cdb[4])
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_MODE_SENSE)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_INQUIRY:     /* INQUIRY */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length < iocmd.scsi_cdb[4])
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_INQUIRY)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_RECEIVE_DIAGNOSTIC_RESULTS:
                /* RECEIVE DIAGNOSTIC RESULTS */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length < (iocmd.scsi_cdb[4] +
                                       ((unsigned) iocmd.scsi_cdb[3] << 8)))
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] !=
                    SCSI_RECEIVE_DIAGNOSTIC_RESULTS)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_RANDOM_SEEK_TEST:{
                                /* random seek test */

                        uchar           read_buffer[4096]; /* for read */
                        SCSI_TUTYPE     r_tucb; /* used to send commands */
                        unsigned        maxblock;       /* max block on disk */
                        unsigned        thisblock;      /* block currently
                                                         * seeking for  */
                        unsigned        expand_rand;    /* how much to multiply
                                                         * value from rand()
                                                         * by to cover all
                                                         * blocks on device */
                        int             rval;           /* for return value */
                        int             blocksize;      /* size of blocks on
                                                         * the disk   */

                        /* we need a place for request sense */
                        if (tucb_ptr->scsitu.data_length < 255)
                                return (SCATU_BAD_PARAMETER);

                        /* set the random number seed */
                        srand(tucb_ptr->scsitu.seed_value);

                        /* test unit ready */

                        r_tucb.header.tu = SCATU_TEST_UNIT_READY;
                        r_tucb.header.mfg = tucb_ptr->header.mfg;

                        rval = scsitu_init(&r_tucb);
                        if (rval != SCATU_GOOD)
                                return (rval);
                        r_tucb.scsitu.flags = tucb_ptr->scsitu.flags;
                        r_tucb.scsitu.ioctl_pass_param =
                                tucb_ptr->scsitu.ioctl_pass_param;
                        r_tucb.scsitu.data_length = 0;
                        r_tucb.scsitu.data_buffer = 0;
                        r_tucb.scsitu.cmd_timeout =
                                tucb_ptr->scsitu.cmd_timeout;

                        /* insert the logical unit */
                        r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

                        rval = exectu(fdes, &r_tucb);
                        if (rval != SCATU_GOOD)
                                return (rval);

                        /* read capacity */

                        r_tucb.header.tu = SCATU_READ_CAPACITY;
                        r_tucb.header.mfg = tucb_ptr->header.mfg;

                        rval = scsitu_init(&r_tucb);
                        if (rval != SCATU_GOOD)
                                return (rval);
                        r_tucb.scsitu.flags = tucb_ptr->scsitu.flags;
                        r_tucb.scsitu.flags &= ~B_WRITE;
                        r_tucb.scsitu.flags |= B_READ;
                        r_tucb.scsitu.ioctl_pass_param =
                                tucb_ptr->scsitu.ioctl_pass_param;
                        r_tucb.scsitu.data_length = 8;
                        r_tucb.scsitu.data_buffer = read_buffer;
                        r_tucb.scsitu.cmd_timeout =
                                tucb_ptr->scsitu.cmd_timeout;

                        /* insert the logical unit */
                        r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

                        rval = exectu(fdes, &r_tucb);
                        if (rval != SCATU_GOOD)
                                return (rval);

                        maxblock = (unsigned) read_buffer[0] << 24 |
                                (unsigned) read_buffer[1] << 16 |
                                (unsigned) read_buffer[2] << 8 |
                                (unsigned) read_buffer[3];

                        blocksize = (unsigned) read_buffer[4] << 24 |
                                (unsigned) read_buffer[5] << 16 |
                                (unsigned) read_buffer[6] << 8 |
                                (unsigned) read_buffer[7];

                        if (blocksize > 4096) {
                                /* must increase constant for buffer size */
                                return (SCATU_BAD_PARAMETER);
                        }
                        /* calculate value to multiply the random numbers by */
                        /* to get a value that can be anywhere on the disk */
                        if (maxblock < 32767)
                                expand_rand = 1;
                        else
                                expand_rand = maxblock / 32767 + 1;

                        while (loopcount--) {
                                /* get a random block */
                                do {
                                        thisblock = (unsigned) rand()
                                                * expand_rand;
                                } while (thisblock > maxblock);


                                /* verify that block */

                                r_tucb.header.tu = SCATU_READ_EXTENDED;
                                r_tucb.header.mfg = tucb_ptr->header.mfg;

                                rval = scsitu_init(&r_tucb);
                                if (rval != SCATU_GOOD)
                                        return (rval);
                                r_tucb.scsitu.flags = tucb_ptr->scsitu.flags;
                                r_tucb.scsitu.flags &= ~B_WRITE;
                                r_tucb.scsitu.flags |= B_READ;
                                r_tucb.scsitu.ioctl_pass_param =
                                        tucb_ptr->scsitu.ioctl_pass_param;
                                r_tucb.scsitu.data_length = blocksize;
                                r_tucb.scsitu.data_buffer = read_buffer;
                                r_tucb.scsitu.cmd_timeout =
                                        tucb_ptr->scsitu.cmd_timeout;

                                /* insert the logical unit */
                                r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit
                                        << 5;

                                /* insert the block number */
                                r_tucb.scsitu.scsi_cmd_blk[2] = thisblock >> 24;
                                r_tucb.scsitu.scsi_cmd_blk[3] = thisblock >> 16;
                                r_tucb.scsitu.scsi_cmd_blk[4] = thisblock >> 8;
                                r_tucb.scsitu.scsi_cmd_blk[5] = thisblock;

                                /* insert the block count */
                                r_tucb.scsitu.scsi_cmd_blk[7] = 0;
                                r_tucb.scsitu.scsi_cmd_blk[8] = 1;

                                rval = exectu(fdes, &r_tucb);
                                /* ignore rval */

                                /* request sense */

                                r_tucb.header.tu = SCATU_REQUEST_SENSE;
                                r_tucb.header.mfg = tucb_ptr->header.mfg;

                                rval = scsitu_init(&r_tucb);
                                if (rval != SCATU_GOOD)
                                        return (rval);
                                r_tucb.scsitu.flags = tucb_ptr->scsitu.flags;
                                r_tucb.scsitu.flags &= ~B_WRITE;
                                r_tucb.scsitu.flags |= B_READ;
                                r_tucb.scsitu.ioctl_pass_param =
                                        tucb_ptr->scsitu.ioctl_pass_param;
                                r_tucb.scsitu.data_length = 255;
                                r_tucb.scsitu.data_buffer =
                                        tucb_ptr->scsitu.data_buffer;
                                r_tucb.scsitu.cmd_timeout =
                                        tucb_ptr->scsitu.cmd_timeout;

                                /* insert the logical unit */
                                r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit
                                        << 5;

                                rval = exectu(fdes, &r_tucb);
                                if (rval != SCATU_GOOD)
                                        return (rval);
                                if (r_tucb.scsiret.sense_key == 1) {
                                        /* recovered error */
                                        ++tucb_ptr->scsiret.rec_errs;
                                } else if (r_tucb.scsiret.sense_key == 3) {
                                        /* medium error */
                                        ++tucb_ptr->scsiret.unrec_errs;
                                } else if (r_tucb.scsiret.sense_key == 4) {
                                        /* hardware error */
                                        ++tucb_ptr->scsiret.unrec_errs;
                                }
                        }       /* bottom of while( loopcount-- ) */

                        return (rval);
                }
                break;

        case SCATU_WRITE:       /* WRITE */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_WRITE)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_START_STOP_UNIT:     /* START / STOP UNIT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_START_STOP_UNIT)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_PLAY_AUDIO:  /* PLAY AUDIO */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != 0x00C1)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_PLAY_AUDIO_TRACK_INDEX:      /* PLAY AUDIO */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != 0x0048)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_AUDIO_TRACK_SEARCH:  /* AUDIO TRACK SEARCH */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != 0x00c0)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_READ_CAPACITY:       /* READ CAPACITY */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.data_length != 8)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_READ_CAPACITY)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_READ_EXTENDED:       /* READ EXTENDED */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_READ_EXTENDED)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case  SCATU_WRITE_EXTENDED:       /* WRITE EXTENDED */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_WRITE_EXTENDED)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

        case SCATU_FORMAT_UNIT:/* FORMAT UNIT */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);

                /* if FmtData is 0 then there must be no data */
                if (!(iocmd.scsi_cdb[1] & 0x10) && iocmd.data_length > 0)
                        return (SCATU_BAD_PARAMETER);

                /* if FmtData is 1 then there must be data */
                if (iocmd.scsi_cdb[1] & 0x10) {
                        int             def_bytes;
                        if (iocmd.data_length < 4)
                                return (SCATU_BAD_PARAMETER);
                        def_bytes = (unsigned) iocmd.buffer[4] |
                                ((unsigned) iocmd.buffer[3]) << 8;
                        if (def_bytes + 4 != iocmd.data_length)
                                return (SCATU_BAD_PARAMETER);
                }
                if (iocmd.scsi_cdb[0] != SCSI_FORMAT_UNIT)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

 case SCATU_WRITE_AND_VERIFY:  /* WRITE AND VERIFY */

                if (iocmd.command_length != 10)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_WRITE_AND_VERIFY)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

 case SCATU_PREVENT_ALLOW_REMOVAL:     /* PREVENT/ALLOW MEDIA REMOVAL */

                if (iocmd.command_length != 6)
                        return (SCATU_BAD_PARAMETER);
                if (iocmd.scsi_cdb[0] != SCSI_PREVENT_ALLOW_REMOVAL)
                        return (SCATU_BAD_PARAMETER);

                while (loopcount--) {
                        command_stat = do_command(fdes, tucb_ptr, &iocmd);
                        if (command_stat != SCATU_TIMEOUT)
                                break;
                }
                return (command_stat);
                break;

 case SCATU_USER_DEFINED:     /* USER DEFINED ------*/

                while (loopcount--) {
                 command_stat = do_command(fdes, tucb_ptr, &iocmd);
                 if (command_stat != SCATU_TIMEOUT)
                  break;
                }
                return (command_stat);
                break;

        default:
                /* unknown tu number */
                return (SCATU_BAD_PARAMETER);

        }                       /* end of switch on tu number */

        return (SCATU_BAD_PARAMETER);
}



/*
 * NAME:  do_command
 *
 * FUNCTION:  execute the ioctl and check return codes.
 *
 * EXECUTION ENVIRONMENT:  This routine is local to this file.
 *
 * EXTERNAL PROCEDURES CALLED:  ioctl()
 *
 * RETURNS: NONE
 */


static int
do_command(fdes, tucb_ptr, iocmd_ptr)
        int             fdes;           /* file descriptor of device    */
        SCSI_TUTYPE    *tucb_ptr;       /* input parameters             */
        struct sc_iocmd *iocmd_ptr;     /* structure for ioctl()        */
{
        int             devret;         /* return value from ioctl()    */
#ifdef DEBUGSCSITU
        char ermsg[100];    /* for display of system error messages */
#endif

        devret = ioctl(fdes, tucb_ptr->scsitu.ioctl_pass_param, iocmd_ptr);
        /* Check for a device failing the wide negotiation. */
        if ((devret == -1) && (iocmd_ptr->status_validity & 2) &&
              (iocmd_ptr->adapter_status & SC_SCSI_BUS_FAULT)) {
         /* Retry the command again. */
          iocmd_ptr->status_validity = 0;
          iocmd_ptr->scsi_bus_status = 0;
          iocmd_ptr->adapter_status = 0;
          devret = ioctl(fdes,tucb_ptr->scsitu.ioctl_pass_param, iocmd_ptr);
        }

#ifdef DEBUGSCSITU
        printf("***** ioctl() return value = %d : ", devret);
        if (devret == -1) {
         strcpy(ermsg," System Error Msg: ");
         if (errno <= sys_nerr)
          (void) strncat(ermsg,sys_errlist[errno], sizeof(ermsg) - strlen(ermsg) );
         printf("%s",ermsg);
        }
        printf("\n");

        printf("ioctl iocmd result values----------------------------------\n");
        printf("  data_length = %d   ", iocmd_ptr->data_length);
         printf("timeout_value = %d   ", iocmd_ptr->timeout_value);
          printf("status_validity = %d   ", iocmd_ptr->status_validity);
           printf("flags = %d\n", iocmd_ptr->flags);

        printf("scsi_bus_status = %d    ", iocmd_ptr->scsi_bus_status);
         printf("adapter_status = %d   ", iocmd_ptr->adapter_status);
          printf("command_length = %d\n", iocmd_ptr->command_length);
        printf("command = ");
        {
                int             itemp;
                itemp = 0;
                while (itemp < iocmd_ptr->command_length) {
                        printf(" %02.2x", iocmd_ptr->scsi_cdb[itemp++]);
                }
        }
        printf("\n");
#endif

        /* pass status values back to DA */
        tucb_ptr->scsiret.status_validity = iocmd_ptr->status_validity;
        tucb_ptr->scsiret.adapter_status = iocmd_ptr->adapter_status;
        tucb_ptr->scsiret.scsi_bus_status = iocmd_ptr->scsi_bus_status;

        if (devret != -1) {
                return (SCATU_GOOD);
        }
        if (errno == ETIMEDOUT)
                return (SCATU_TIMEOUT);

        /* is SCSI adapter status valid */
        if (iocmd_ptr->status_validity & 2) {
                if (iocmd_ptr->adapter_status & SC_HOST_IO_BUS_ERR)
                        return (SCATU_IO_BUS_ERROR);
                if (iocmd_ptr->adapter_status & SC_SCSI_BUS_FAULT)
                        return (SCATU_COMMAND_ERROR);
                if (iocmd_ptr->adapter_status & SC_CMD_TIMEOUT)
                        return (SCATU_TIMEOUT);
                if (iocmd_ptr->adapter_status & SC_NO_DEVICE_RESPONSE)
                        return (SCATU_TIMEOUT);
                if (iocmd_ptr->adapter_status & SC_ADAPTER_HDW_FAILURE)
                        return (SCATU_ADAPTER_FAILURE);
                if (iocmd_ptr->adapter_status & SC_ADAPTER_SFW_FAILURE)
                        return (SCATU_ADAPTER_FAILURE);
                if (iocmd_ptr->adapter_status & SC_FUSE_OR_TERMINAL_PWR)
                        return (SCATU_ADAPTER_FAILURE);
        }
        /* is scsi bus status valid */
        if (iocmd_ptr->status_validity & 1) {
                /* keep only the informative bits */
                iocmd_ptr->scsi_bus_status &= SCSI_STATUS_MASK;
                if (iocmd_ptr->scsi_bus_status == SC_GOOD_STATUS)
                        return (SCATU_GOOD);
                if (iocmd_ptr->scsi_bus_status & SC_CHECK_CONDITION)
                        return (SCATU_CHECK_CONDITION);
                if (iocmd_ptr->scsi_bus_status & SC_RESERVATION_CONFLICT)
                        return (SCATU_RESERVATION_CONFLICT);
                /* catch everything else */
                if (iocmd_ptr->scsi_bus_status)
                        return (SCATU_COMMAND_ERROR);
        }
        return (SCATU_COMMAND_ERROR);
}




/*
 * NAME:  scsitu_init
 *
 * FUNCTION:  Initialize control structures for execution of a TU.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called as a subroutine of a diagnostic
 *      application.
 *
 * NOTES:  Supported test unit list is the same as that for exectu()
 *
 * DATA STRUCTURES:
 *      tucb_ptr - contains details of TU to be run
 *
 * INPUTS:  tucb_ptr - structure to be initialized.
 *
 * RETURNS:
 * RETURN VALUE DESCRIPTION:
 *      SCATU_GOOD                   - structure initialized successfully
 *      SCATU_BAD_PARAMETER          - invalid input parameter
 *
 * EXTERNAL PROCEDURES CALLED:  memset(), time(), getpid()
 *      in DEBUGSCSITU mode: printf()
 */


int
scsitu_init(tucb_ptr)
        SCSI_TUTYPE    *tucb_ptr;       /* structure to be initialized */
{
        int             ret;            /* value to be returned */

#ifdef DEBUGSCSITU
        printf("\n>> in scsitu_init <<\n");
#endif

        if (tucb_ptr->header.mfg == 1)
                tucb_ptr->header.loop = 1;
        else
                tucb_ptr->header.loop = 2;
        tucb_ptr->header.r1 = 0;
        tucb_ptr->header.r2 = 0;

        tucb_ptr->scsitu.command_length = 6;
        memset(tucb_ptr->scsitu.scsi_cmd_blk, 0, 12);
        tucb_ptr->scsitu.flags = 0;     /* CORRECT FOR EACH DEVICE */
        tucb_ptr->scsitu.seed_value = 0;
        tucb_ptr->scsitu.ioctl_pass_param = 0;
        tucb_ptr->scsitu.lun = 0;

        /*-- clear data buffer and length areas --*/
        tucb_ptr->scsitu.data_buffer = NULL;
        tucb_ptr->scsitu.data_length = 0;

        tucb_ptr->scsiret.sense_key = 0;
        tucb_ptr->scsiret.sense_code = 0;
        tucb_ptr->scsiret.rec_errs = 0;
        tucb_ptr->scsiret.unrec_errs = 0;
        tucb_ptr->scsiret.status_validity = 0;
        tucb_ptr->scsiret.scsi_bus_status = 0;
        tucb_ptr->scsiret.adapter_status = 0;
        ret = SCATU_GOOD;

        switch (tucb_ptr->header.tu) {
        case SCATU_LOAD_UNLOAD_UNIT:     /* LOAD - UNLOAD UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_UNLOAD;
                break;

        case SCATU_TEST_UNIT_READY:     /* TEST UNIT READY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_TEST_UNIT_READY;
                break;

        case SCATU_REQUEST_SENSE:       /* REQUEST SENSE */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_REQUEST_SENSE;
                tucb_ptr->scsitu.scsi_cmd_blk[4] = 255;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_RESERVE_UNIT:        /* RESERVE UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_RESERVE_UNIT;
                break;

        case SCATU_MODE_SELECT:/* MODE SELECT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_MODE_SELECT;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

        case SCATU_SEND_DIAGNOSTIC:     /* SEND DIAGNOSTIC */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_SEND_DIAGNOSTIC;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

        case SCATU_REASSIGN_BLOCKS:     /* REASSIGN BLOCKS */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_REASSIGN_BLOCK;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

        case SCATU_RELEASE_UNIT:        /* RELEASE UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_RELEASE_UNIT;
                break;

        case SCATU_MODE_SENSE:  /* MODE SENSE */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_MODE_SENSE;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_INQUIRY:     /* INQUIRY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_INQUIRY;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_RECEIVE_DIAGNOSTIC_RESULTS:
                /* RECEIVE DIAGNOSTIC RESULTS */
                tucb_ptr->scsitu.scsi_cmd_blk[0] =
                        SCSI_RECEIVE_DIAGNOSTIC_RESULTS;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_RANDOM_SEEK_TEST:    /* random seek test */
                /* how about 2000 seeks */
                tucb_ptr->header.loop = 2000;
                /* let's get a good random seed */
                tucb_ptr->scsitu.seed_value = (int) time(NULL) +
                        (int) getpid();
                /* no actual command */
                tucb_ptr->scsitu.command_length = 0;
                break;

        case SCATU_WRITE:       /* WRITE */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_WRITE;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

        case SCATU_START_STOP_UNIT:     /* START / STOP UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_START_STOP_UNIT;
                tucb_ptr->scsitu.scsi_cmd_blk[4] = 0x01;        /* START */
                break;

        case SCATU_PLAY_AUDIO:  /* PLAY AUDIO */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = 0x00C1;
                tucb_ptr->scsitu.command_length = 10;
                break;

        case SCATU_PLAY_AUDIO_TRACK_INDEX:      /* PLAY AUDIO */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = 0x0048;
                tucb_ptr->scsitu.command_length = 10;
                break;

        case SCATU_AUDIO_TRACK_SEARCH:  /* AUDIO TRACK SEARCH */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = 0x00C0;
                tucb_ptr->scsitu.command_length = 10;
                break;

        case SCATU_READ_CAPACITY:       /* READ_CAPACITY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_READ_CAPACITY;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_READ_EXTENDED:       /* READ EXTENDED */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_READ_EXTENDED;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

        case SCATU_WRITE_EXTENDED:       /* WRITE EXTENDED */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_WRITE_EXTENDED;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

        case SCATU_FORMAT_UNIT:/* FORMAT UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_FORMAT_UNIT;
                /* use manufactur's list only */
                tucb_ptr->scsitu.scsi_cmd_blk[1] = 0x08;
                /* tucb_ptr->scsitu.flags |= B_WRITE; */
                break;

 case SCATU_WRITE_AND_VERIFY:  /* WRITE AND VERIFY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_WRITE_AND_VERIFY;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

 case SCATU_PREVENT_ALLOW_REMOVAL:     /* PREVENT/ALLOW MEDIA REMOVAL (R/W OPTICAL DISK) */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_PREVENT_ALLOW_REMOVAL;
                tucb_ptr->scsitu.command_length = 6;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

 case SCATU_USER_DEFINED:     /* USER DEFINED -----*/
                break;        /* leave all fields initialized -*/

        default:
                ret = SCATU_BAD_PARAMETER;

        }                       /* end of switch on tu number */

        return (ret);
}                               /* end of scsitu_init() */

/* end of scsi_atu.c */

#endif                          /* _C_SCSI_ATU */
