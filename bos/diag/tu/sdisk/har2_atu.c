static char sccsid[] = "@(#)65  1.4.2.2  src/bos/diag/tu/sdisk/har2_atu.c, tu_sdisk, bos411, 9428A410j 11/8/93 10:25:32";
/*
 *   COMPONENT_NAME: TU_SDISK
 *
 *   FUNCTIONS: break_fence
 *		do_command
 *		exectu
 *		scsitu_init
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
/**************************************************************************/
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*   NAME:         9333 Application Test Unit (ATU) Code             */
/*                                                                        */
/*   SOURCE FILE:  har2_atu.c                                             */
/*                                                                        */
/*   FUNCTION:  9333 Application Test Units.                         */
/*                                                                        */
/*              This source file contains all the code for the 9333  */
/*              Application Test Units used for diagnosing the 9333  */
/*              I/O subsystem. Command structures are taken from the      */
/*              diagnostic applications and passed to the 9333 Device*/
/*              Driver.  The Device Driver operates in "passthrough" mode */
/*              The SCSI command is transferred to the Device Driver for  */
/*              the resident 9333 adapter card.                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*   ROUTINES:  exectu(), scsitu_init()                                   */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  ioctl(), memcpy(), memset(),            */
/*                                time(), getpid(),                       */
/*                                rand(), srand()                         */
/*                                                                        */
/*          in DEBUGSCSITU mode:  printf()                                */
/*                                                                        */
/*                                                                        */
/*   LOCAL PROCEDURES CALLED: do_command()                                */
/*      NOTE:  exectu() uses one level of recursion for the               */
/*             RANDOM SEEK TEST & MFG CERTIFY UNIT ATU's                  */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*   EXECUTION ENVIRONMENT:                                               */
/*                                                                        */
/*       This routine is called as a subroutine of a diagnostic           */
/*       application.  The file associated with fdes must have been       */
/*       opened in SC_DIAGNOSTIC mode with an openx() call.               */
/*                                                                        */
/*       This routine will accept commands to perform specific test       */
/*       units on 9333 devices.  Supported test units are:           */
/*                                                                        */
/*       #0x01    -  TEST UNIT READY                                      */
/*       #0x02    -  REQUEST SENSE                                        */
/*       #0x03    -  RESERVE UNIT                                         */
/*       #0x04    -  MODE SELECT                                          */
/*       #0x05    -  SEND DIAGNOSTIC                                      */
/*       #0x06    -  RELEASE UNIT                                         */
/*       #0x07    -  MODE SENSE                                           */
/*       #0x08    -  INQUIRY                                              */
/*       #0x09    -  RECEIVE DIAGNOSTIC RESULTS                           */
/*       #0x0B    -  START / STOP UNIT                                    */
/*       #0x10    -  RANDOM SEEK TEST                                     */
/*       #0x17    -  FORMAT UNIT                                          */
/*       #0x18    -  REASSIGN BLOCK                                       */
/*       #0x19    -  MFG CERTIFY UNIT                                     */
/*       #0x70    -  READ CAPACITY                                        */
/*       #0x71    -  READ EXTENDED                                        */
/*       #0x72    -  ENABLE SOFT ERRORS                                   */
/*       #0x73    -  DISABLE SOFT ERRORS                                  */
/*                                                                        */
/*   DATA STRUCTURES:                                                     */
/*       tucb_ptr - input structure, contains details of TU to be run     */
/*       iocmd    - structure that is passed as a parameter to the        */
/*                  Device Driver via ioctl().                            */
/*   INPUTS:                                                              */
/*       fdes     - file descriptor of 9333 device.                  */
/*       tucb_ptr - structure with details of test unit to be run.        */
/*                                                                        */
/*                  NOTE:  tucb_ptr->scsitu.ioctl_pass_param MUST be      */
/*                         set by diagnostic application.                 */
/*                                                                        */
/*                                                                        */
/*   RETURN VALUES & DESCRIPTION:                                         */
/*                                                                        */
/* ( 00) SCATU_GOOD                   - command completed successfully    */
/* (-10) SCATU_TIMEOUT                - Adapter command timed out         */
/* (-11) SCATU_RESERVATION_CONFLICT   - device is reserved for another    */
/*                                      initiator                         */
/* (-12) SCATU_CHECK_CONDITION        - device is indicating a            */
/*                                      check condition status            */
/* (-13) SCATU_COMMAND_ERROR          - the command completed with errors */
/*                                      this includes a 'busy' status     */
/* (-14) SCATU_BAD_REQUEST_SENSE_DATA - request sense data obtained during*/
/*                                      the RANDOM SEEK TEST test unit    */
/*                                      indicated a condition other than  */
/*                                      recoverable or nonrecoverable     */
/*                                      data error                        */
/* (-70)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19'. No sense data was return-*/
/*                                      ed for a 'Check Condition Status' */
/*                                      reported after a Read Extended    */
/*                                      command.                          */
/* (-71)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19' only. Hard equipment      */
/*                                      failure persist even after retry  */
/*                                      attempts.                         */
/* (-72)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19' only. Same LBA in error   */
/*                                      after block reassignment and Read */
/*                                      Extended retry.                   */
/* (-73)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19' only. Unexpected error    */
/*                                      detected.                         */
/* (-74)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19' only. Permanent hardware  */
/*                                      error.                            */
/* (-75)                              - Used by the MFG Certify Unit ATU  */
/*                                      # x'19' only. More than one LBA   */
/*                                      requires reassignment.            */
/* (-95) SCATU_QUEQUE_FULL            - 9333 Controller queue is full*/
/*                                      No more SCSI command can be       */
/*                                      accepted by 9333 at this time*/
/*                                      (Resend SCSI command later)       */
/* (-96) SCATU_NONEXTENDED_SENSE      - data obtained from a request      */
/*                                      sense command was not in          */
/*                                      extended format                   */
/* (-97) SCATU_IO_BUS_ERROR          - Microchannel I/O failure           */
/* (-98) SCATU_ADAPTER_FAILURE       - failure of the 9333 Adapter   */
/* (-99) SCATU_BAD_PARAMETER         - invalid input parameter            */
/**************************************************************************/

/* Include Files                                                          */
#include <diag/har2_atu.h>     /* Structure declarations used by ATU's         */
#include <sys/serdasd.h>

/* Remove comments below for debug mode                                   */
/* #define DEBUGSCSITU */

/* Function Declarations                                                  */
/* externally visible  */
int exectu();
int scsitu_init();
int do_command();

int
exectu(fdes, tucb_ptr)
int fdes;                /* file descriptor of device      */
HAR2_TUTYPE *tucb_ptr;   /* details of test unit to be run */
{
        struct sd_iocmd iocmd;  /* this goes to the device driver head    */
        int command_stat;       /* temporarily store status of command    */
        int rc;                 /* function return code                   */

#ifdef DEBUGSCSITU
        printf("In 9333 exectu\n");
#endif
        memset( &iocmd, '\0', sizeof( iocmd ) );

        iocmd.data_length = tucb_ptr->scsitu.data_length;
        iocmd.buffer = tucb_ptr->scsitu.data_buffer;
        iocmd.timeout_value = tucb_ptr->scsitu.cmd_timeout;
        iocmd.resvd5=tucb_ptr->scsitu.resvd5;
        iocmd.flags = tucb_ptr->scsitu.flags;
        iocmd.command_length = tucb_ptr->scsitu.command_length;

        if( tucb_ptr->scsitu.command_length > 12 )
          return( SCATU_BAD_PARAMETER );  /* command too long */

        /* copy the command over */
        memcpy( iocmd.scsi_cdb, tucb_ptr->scsitu.scsi_cmd_blk,
                            tucb_ptr->scsitu.command_length );
        iocmd.command_length = tucb_ptr->scsitu.command_length;


#ifdef DEBUGSCSITU
        printf("initial data_length = %d  ",iocmd.data_length);
        printf("initial timeout_value = %d\n",iocmd.timeout_value);
        printf("initial resvd5 = %02x\n",iocmd.resvd5);
        printf("initial status_validity = %d\n",iocmd.status_validity);
        printf("initial scsi_bus_status = %d\n",iocmd.scsi_bus_status);
        printf("initial adapter_status = %d\n",iocmd.adapter_status);
        printf("initial flags = %d\n",iocmd.flags);
        printf("initial command_length = %d\n",iocmd.command_length);
        printf("initial command = " );
        {
             int itemp;
             itemp = 0;
             while( itemp < iocmd.command_length ) {
               printf(" %02.2x",
                      tucb_ptr->scsitu.scsi_cmd_blk[itemp++] );
             } /* end while */
        }
        printf("copied command = " );
        {
             int itemp;
             itemp = 0;
             while( itemp < iocmd.command_length ) {
               printf(" %02.2x", iocmd.scsi_cdb[itemp++] );
             } /* end while */
        }
        printf("\n" );
#endif

        switch( tucb_ptr->header.tu )
        {
          case SCATU_TEST_UNIT_READY:     /* TEST UNIT READY */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_TEST_UNIT_READY )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_REQUEST_SENSE:       /* REQUEST SENSE */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length > 255 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_REQUEST_SENSE )
                  {rc = SCATU_BAD_PARAMETER;break;}
                command_stat = do_command( fdes, tucb_ptr, &iocmd );
                if( command_stat == SCATU_GOOD ) {
                  if( (iocmd.buffer[0] & 0x7e)== 0x70 ) {
                    /* extended sense */
                    tucb_ptr->scsiret.sense_key =
                           iocmd.buffer[2] & 0x0f;
                    tucb_ptr->scsiret.sense_code =
                          (uchar) iocmd.buffer[13] | (unsigned ) (
                                  (uchar) iocmd.buffer[12] ) << 8;
                    tucb_ptr->scsiret.host_action_code = iocmd.buffer[18];
                    tucb_ptr->scsiret.unit_error_code =
                          (uchar) iocmd.buffer[21] | (unsigned ) (
                                  (uchar) iocmd.buffer[20] ) << 8;
                  }
                  else
                  {
                    /* nonextended sense error class */
                    tucb_ptr->scsiret.sense_key =
                           iocmd.buffer[0] & 0x70 >> 4;
                    /* error code */
                    tucb_ptr->scsiret.sense_code =
                           iocmd.buffer[0] & 0x0f;
                    rc = SCATU_NONEXTENDED_SENSE;
                    break;
                  }
                } /* end if */
                rc = command_stat;
                break;

          case SCATU_RESERVE_UNIT:        /* RESERVE UNIT */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_RESERVE_UNIT )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_MODE_SELECT:         /* MODE SELECT */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length < iocmd.scsi_cdb[4] )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_MODE_SELECT )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_SEND_DIAGNOSTIC:     /* SEND DIAGNOSTIC */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length < ( iocmd.scsi_cdb[4] +
                          ( (unsigned) iocmd.scsi_cdb[3] << 8 ) ) )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_SEND_DIAGNOSTIC )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_RELEASE_UNIT:        /* RELEASE UNIT */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_RELEASE_UNIT )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_MODE_SENSE:          /* MODE SENSE */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length < iocmd.scsi_cdb[4] )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_MODE_SENSE )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_INQUIRY:             /* INQUIRY */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length < iocmd.scsi_cdb[4] )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_INQUIRY )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_RECEIVE_DIAGNOSTIC_RESULTS: /* RECEIVE DIAG RESULTS */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length < ( iocmd.scsi_cdb[4] +
                          ( (unsigned) iocmd.scsi_cdb[3] << 8 ) ) )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] !=
                                  SCSI_RECEIVE_DIAGNOSTIC_RESULTS )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_START_STOP_UNIT:     /* START / STOP UNIT */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_START_STOP_UNIT )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

#ifndef DIAG
          case SCATU_RANDOM_SEEK_TEST:    /* random seek test */
                rc = random_seek( fdes, tucb_ptr );
                break;
#endif

          case SCATU_BREAK_FENCE:    /* random seek test */
                rc = break_fence( fdes, tucb_ptr );
                break;

          case SCATU_FORMAT_UNIT:         /* FORMAT UNIT */
                if(!((iocmd.command_length == 6)||(iocmd.command_length == 10)))
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_FORMAT_UNIT )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_REASSIGN_BLOCK:      /* REASSIGN BLOCK */
                if( iocmd.command_length != 6 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length != 8 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_REASSIGN_BLOCK )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

#ifndef DIAG
          case SCATU_MFG_CERTIFY_UNIT:    /* MFG Certify Unit */
                rc = mfg_certify( fdes, tucb_ptr );
                break;
#endif

          case SCATU_READ_CAPACITY:       /* READ CAPACITY */
                if( iocmd.command_length != 10 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.data_length != 8 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_READ_CAPACITY )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_READ_EXTENDED:       /* READ EXTENDED */
                if( iocmd.command_length != 10 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_READ_EXTENDED )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_WRITE_EXTENDED:       /* WRITE EXTENDED */
                if( iocmd.command_length != 10 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != SCSI_WRITE_EXTENDED )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_ENABLE_SOFT_ERRORS:            /* ENABLE SOFT ERRORS */
                if( iocmd.command_length != 10 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != 0x3f )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          case SCATU_DISABLE_SOFT_ERRORS:           /* DISABLE SOFT ERRORS */
                if( iocmd.command_length != 10 )
                  {rc = SCATU_BAD_PARAMETER;break;}
                if( iocmd.scsi_cdb[0] != 0x3f )
                  {rc = SCATU_BAD_PARAMETER;break;}
                rc = do_command( fdes, tucb_ptr, &iocmd );
                break;

          default:
                /* unknown tu number */
                rc = SCATU_BAD_PARAMETER;
                break;

        } /* end of switch on tu number */
        return (rc);

}

/**************************************************************************/
/*                                                                        */
/* NAME:  do_command                                                      */
/*                                                                        */
/* FUNCTION:  execute the ioctl and check return codes.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:  This routine is local to this file.            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  ioctl()                                   */
/*                                                                        */
/**************************************************************************/


int
do_command( fdes, tucb_ptr, iocmd_ptr )
int fdes;                       /* file descriptor of device    */
HAR2_TUTYPE *tucb_ptr;          /* input parameters             */
struct sd_iocmd *iocmd_ptr;     /* structure for ioctl()        */
{
        int devret;     /* return value from ioctl()    */
        int rc;         /* return value from do_command */

        devret = ioctl( fdes, tucb_ptr->scsitu.ioctl_pass_param, iocmd_ptr );

#ifdef DEBUGSCSITU
        printf("ioctl() return value = %d\n",devret );
        printf("returned data_length = %d\n",iocmd_ptr->data_length);
        printf("returned timeout_value = %d\n",iocmd_ptr->timeout_value);
        printf("returned status_validity = %d\n",iocmd_ptr->status_validity);
        printf("returned scsi_bus_status = %d\n",iocmd_ptr->scsi_bus_status);
        printf("returned adapter_status = %d\n",iocmd_ptr->adapter_status);
        printf("returned flags = %d\n",iocmd_ptr->flags);
        printf("returned alert_register byte 2 = %d\n",iocmd_ptr->resvd2);
        printf("returned alert_register byte 3 = %d\n",iocmd_ptr->resvd1);
        printf("returned command_length = %d\n",iocmd_ptr->command_length);
        printf("returned command = " );
        {
             int itemp;
             itemp = 0;
             while( itemp < iocmd_ptr->command_length ) {
                  printf(" %02.2x", iocmd_ptr->scsi_cdb[itemp++] );
             }
        }
        printf("\n" );
#endif

        if(devret == -1 && errno == ETIMEDOUT )
          return( SCATU_TIMEOUT );
        if(devret == -1 && errno == EIO )
          return( SCATU_ADAPTER_FAILURE );

        if(devret == -1) return (-1);  /* error info is in errno */

        if(devret == 0 && iocmd_ptr->status_validity == 0)
          return( SCATU_GOOD );

        /* pass status values back to DA */
        tucb_ptr->scsiret.status_validity = iocmd_ptr->status_validity;
        tucb_ptr->scsiret.adapter_status  = iocmd_ptr->adapter_status;
        tucb_ptr->scsiret.scsi_bus_status = iocmd_ptr->scsi_bus_status;
        tucb_ptr->scsiret.alert_register_adap = iocmd_ptr->resvd2;
        tucb_ptr->scsiret.alert_register_cont = iocmd_ptr->resvd1;

        /* is SCSI adapter status valid? */
        if((iocmd_ptr->status_validity & 2) || (iocmd_ptr->status_validity & 4))
        {
          if( iocmd_ptr->adapter_status & SC_HOST_IO_BUS_ERR )
            rc = ( SCATU_IO_BUS_ERROR );
          else if( iocmd_ptr->adapter_status & SC_SCSI_BUS_FAULT )
            rc = ( SCATU_IO_BUS_ERROR );
          else if( iocmd_ptr->adapter_status & SC_CMD_TIMEOUT )
            rc = ( SCATU_TIMEOUT );
          else
            rc = ( SCATU_ADAPTER_FAILURE );
        }

        /* is scsi bus status valid? */
        else if( iocmd_ptr->status_validity & 1 )
        {
          /* keep only the informative bits */
          iocmd_ptr->scsi_bus_status &= SCSI_STATUS_MASK;
          if( iocmd_ptr->scsi_bus_status == SC_GOOD_STATUS )
            rc = ( SCATU_GOOD );
          else if( iocmd_ptr->scsi_bus_status & SC_CHECK_CONDITION )
            rc = ( SCATU_CHECK_CONDITION );
          else if( iocmd_ptr->scsi_bus_status & SC_RESERVATION_CONFLICT )
            rc = ( SCATU_RESERVATION_CONFLICT );
          else if( iocmd_ptr->scsi_bus_status &  SD_FENCED_OUT )
            rc = ( SCATU_RESERVATION_CONFLICT );
          else if( iocmd_ptr->scsi_bus_status & SD_QUEUE_FULL)
            rc = ( SCATU_QUEUE_FULL );
          /* catch everything else */
          else if( iocmd_ptr->scsi_bus_status )
            rc = ( SCATU_COMMAND_ERROR );
        }

        /* some unexpected error */
        else rc = ( SCATU_COMMAND_ERROR );

        return (rc);
}                       /* end of do_command() */
/**************************************************************************/
/*                                                                        */
/* NAME:  scsitu_init                                                     */
/*                                                                        */
/* FUNCTION:  Initialize control structures for execution of a TU.        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is called as a subroutine by the Random Seek ATU     */
/*                                                                        */
/* NOTES:  The scsitu_init() subroutine, which is used by har2_exer.c,    */
/*         should be used by the Diagnostic Application as well.          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      tucb_ptr - contains details of TU to be run                       */
/*                                                                        */
/* INPUTS:  tucb_ptr - structure to be initialized.                       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      SCATU_GOOD                   - structure initialized successfully */
/*      SCATU_BAD_PARAMETER          - invalid input parameter            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  memset(), time(), getpid()                */
/*      in DEBUGSCSITU mode: printf()                                     */
/*                                                                        */
/**************************************************************************/

int scsitu_init( tucb_ptr )
HAR2_TUTYPE *tucb_ptr;  /* structure to be initialized */
{
        int ret = SCATU_GOOD;        /* value to be returned */

#ifdef DEBUGSCSITU
        printf("in scsitu_init\n");
#endif

        tucb_ptr->header.mfg = 0;  /* 0 = normal , 1 = manufacturing */
        tucb_ptr->header.loop = 1;
        tucb_ptr->header.r1 = 0;
        tucb_ptr->header.r2 = 0;

        tucb_ptr->scsitu.data_length = 0;
        tucb_ptr->scsitu.data_buffer = 0;
        tucb_ptr->scsitu.cmd_timeout = 30;
        tucb_ptr->scsitu.command_length = 6;
        memset( tucb_ptr->scsitu.scsi_cmd_blk, 0, 12 );
        tucb_ptr->scsitu.flags = 0;
        tucb_ptr->scsitu.seed_value = 0;
        tucb_ptr->scsitu.ioctl_pass_param = 0x03; /* DD pass-thru cmd */

        tucb_ptr->scsiret.sense_key = 0;
        tucb_ptr->scsiret.sense_code = 0;
        tucb_ptr->scsiret.host_action_code = 0;
        tucb_ptr->scsiret.unit_error_code = 0;
        tucb_ptr->scsiret.rec_errs = 0;
        tucb_ptr->scsiret.soft_thres_errs = 0;
        tucb_ptr->scsiret.soft_equip_chks = 0;
        tucb_ptr->scsiret.unrec_errs = 0;
        tucb_ptr->scsiret.hard_data_errs = 0;
        tucb_ptr->scsiret.hard_equip_chks = 0;
        tucb_ptr->scsiret.reas_lba = 999999999;
        tucb_ptr->scsiret.blks_read = 0;
        tucb_ptr->scsiret.atu_num = 0;
        tucb_ptr->scsiret.status_validity = 0;
        tucb_ptr->scsiret.scsi_bus_status = 0;
        tucb_ptr->scsiret.adapter_status = 0;

        switch( tucb_ptr->header.tu ) {
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

          case SCATU_MODE_SELECT:         /* MODE SELECT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_MODE_SELECT;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

          case SCATU_SEND_DIAGNOSTIC:     /* SEND DIAGNOSTIC */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_SEND_DIAGNOSTIC;
                tucb_ptr->scsitu.flags |= B_WRITE;
                tucb_ptr->scsitu.cmd_timeout = 60;
                break;

          case SCATU_RELEASE_UNIT:        /* RELEASE UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_RELEASE_UNIT;
                break;

          case SCATU_MODE_SENSE:          /* MODE SENSE */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_MODE_SENSE;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_INQUIRY:             /* INQUIRY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_INQUIRY;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_RECEIVE_DIAGNOSTIC_RESULTS: /* RECEIVE DIAG RESULTS */
                tucb_ptr->scsitu.scsi_cmd_blk[0] =
                            SCSI_RECEIVE_DIAGNOSTIC_RESULTS;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_START_STOP_UNIT:             /* START / STOP UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_START_STOP_UNIT;
                tucb_ptr->scsitu.scsi_cmd_blk[4] = 0x01;  /* START */
                tucb_ptr->scsitu.cmd_timeout = 60;
                break;

          case SCATU_RANDOM_SEEK_TEST:    /* Random Seek Test */
                /* how about 2000 seeks */
                tucb_ptr->header.loop = 2000;
                /* let's get a good random seed */
                tucb_ptr->scsitu.seed_value = (int) time( NULL ) +
                    (int) getpid();
                /* no actual command */
                tucb_ptr->scsitu.command_length = 0;
                break;

          case SCATU_FORMAT_UNIT:         /* FORMAT UNIT */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_FORMAT_UNIT;
                tucb_ptr->scsitu.cmd_timeout = 3600; /* 60 minutes */
                break;

          case SCATU_REASSIGN_BLOCK:      /* REASSIGN BLOCK */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_REASSIGN_BLOCK;
                tucb_ptr->scsitu.flags |= B_WRITE;
                tucb_ptr->scsitu.cmd_timeout = 120; /* 2 minutes */
                break;

          case SCATU_MFG_CERTIFY_UNIT:    /* Certify Unit */
                /* no actual command */
                tucb_ptr->scsitu.command_length = 0;
                break;

          case SCATU_BREAK_FENCE:    /* Break Fence */
                /* no actual command */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SD_FENCE_OP_CODE;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_READ_CAPACITY:       /* READ_CAPACITY */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_READ_CAPACITY;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_READ_EXTENDED:               /* READ EXTENDED */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_READ_EXTENDED;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags &= ~B_WRITE;
                tucb_ptr->scsitu.flags |= B_READ;
                break;

          case SCATU_WRITE_EXTENDED:               /* WRITE EXTENDED */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = SCSI_WRITE_EXTENDED;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

          case SCATU_ENABLE_SOFT_ERRORS:           /* ENABLE SOFT ERRORS */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = 0x3f;
                tucb_ptr->scsitu.scsi_cmd_blk[1] = 0x00;
                tucb_ptr->scsitu.scsi_cmd_blk[2] = 0x29;
                tucb_ptr->scsitu.scsi_cmd_blk[3] = 0x34;
                tucb_ptr->scsitu.scsi_cmd_blk[4] = 0x68;
                tucb_ptr->scsitu.scsi_cmd_blk[5] = 0x42;
                tucb_ptr->scsitu.scsi_cmd_blk[6] = 0x71;
                tucb_ptr->scsitu.scsi_cmd_blk[7] = 0x58;
                tucb_ptr->scsitu.scsi_cmd_blk[8] = 0x10;
                tucb_ptr->scsitu.scsi_cmd_blk[9] = 0x00;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

          case SCATU_DISABLE_SOFT_ERRORS:           /* DISABLE SOFT ERRORS */
                tucb_ptr->scsitu.scsi_cmd_blk[0] = 0x3f;
                tucb_ptr->scsitu.scsi_cmd_blk[1] = 0x00;
                tucb_ptr->scsitu.scsi_cmd_blk[2] = 0x29;
                tucb_ptr->scsitu.scsi_cmd_blk[3] = 0x34;
                tucb_ptr->scsitu.scsi_cmd_blk[4] = 0x68;
                tucb_ptr->scsitu.scsi_cmd_blk[5] = 0x42;
                tucb_ptr->scsitu.scsi_cmd_blk[6] = 0x71;
                tucb_ptr->scsitu.scsi_cmd_blk[7] = 0x58;
                tucb_ptr->scsitu.scsi_cmd_blk[8] = 0x00;
                tucb_ptr->scsitu.scsi_cmd_blk[9] = 0x00;
                tucb_ptr->scsitu.command_length = 10;
                tucb_ptr->scsitu.flags |= B_WRITE;
                break;

          default:
                ret = SCATU_BAD_PARAMETER;

        } /* end of switch on tu number */
return( ret );
} /* end of scsitu_init() */
/**************************************************************************/
/*                                                                        */
/* NAME:  Break Fence                                                     */
/*                                                                        */
/* FUNCTION: This ATU will break a fence erected by another host system	  */
/*           against this host. The ATU will issue two fence commands     */
/*           The first will be used to determine the fence position of    */
/*           for this host. The second will force the fence off for this  */
/*           host.                                                        */
/*                                                                        */
/* EXECUTION ENVIRONMENT:  This routine is local to this file.            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: exectu()                                   */
/*                                                                        */
/**************************************************************************/

int
break_fence(fdes, tucb_ptr)
int fdes;
HAR2_TUTYPE *tucb_ptr;       /* details of test unit to be run            */
{
   uchar read_buffer[4096];  /* used for read command                     */
   HAR2_TUTYPE r_tucb;       /* used to send commands                     */
   int logical_unit;         /* logical unit number of device             */
   int rval;                 /* temporary for return value                */

   /* get the logical unit number */
   logical_unit = (tucb_ptr->scsitu.scsi_cmd_blk[1] >> 5) & 0x07;

   /* Fence command */
   r_tucb.header.tu = SCATU_BREAK_FENCE;
   rval = scsitu_init( &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );

   /* insert the logical unit */
   r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;
   /* Force Fence */
   r_tucb.scsitu.scsi_cmd_blk[1] |= 0x00;
   /* mask and swap */
   r_tucb.scsitu.scsi_cmd_blk[1] |= SD_FENCE_MASK_SWAP;

   r_tucb.scsitu.data_length = 4;
   r_tucb.scsitu.data_buffer = read_buffer;

   rval = exectu( fdes, &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );
    
   /* Fence command */
   r_tucb.header.tu = SCATU_BREAK_FENCE;
   rval = scsitu_init( &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );

   /* insert the logical unit */
   r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;
   /* Force Fence */
   r_tucb.scsitu.scsi_cmd_blk[1] |= 0x10;
   /* mask and swap */
   r_tucb.scsitu.scsi_cmd_blk[1] |= SD_FENCE_MASK_SWAP;

   /* Fence Mask = old Fence Position Indicator */
   r_tucb.scsitu.scsi_cmd_blk[2] |= read_buffer[0];
   r_tucb.scsitu.scsi_cmd_blk[3] |= read_buffer[1];

   r_tucb.scsitu.data_length = 4;
   r_tucb.scsitu.data_buffer = read_buffer;

   rval = exectu( fdes, &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );
    

} /* end rand_seek */
