static char sccsid[] = "@(#)har2_mfg.c  1.1 5/14/91 14:33:25";
/*
 * COMPONENT_NAME: tu_sdisk
 *
 * FUNCTIONS: mfg_certify(), random_seek()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*                                                                        */
/*   AUTHOR:       D. Dhopeshwarkar   Dept. F27  T/L:  678-9615           */
/*                                                                        */
/*   NAME:         Harrier-2 Application Test Unit (ATU) Code             */
/*                                                                        */
/*   SOURCE FILE:  har2_mfg.c                                             */
/*                                                                        */
/*   IBM CONFIDENTIAL                                                     */
/*   Copyright International Business Machines Corp. 1985, 1988           */
/*   Unpublished Work                                                     */
/*   All Rights Reserved                                                  */
/*   Licensed Material - Program Property of IBM                          */
/*                                                                        */
/*   RESTRICTED RIGHTS LEGEND                                             */
/*   Use, Duplication or Disclosure by the Government is subject to       */
/*   restrictions as set forth in paragraph (b)(3)(B) of the Rights in    */
/*   Technical Data and Computer Software clause in DAR 7-104.9(a).       */
/*                                                                        */
/*                                                                        */
/*   FUNCTION:  Harrier-2 Application Test Units.                         */
/*                                                                        */
/*              This source file contains all the code for the Harrier-2  */
/*              Application Test Units used for diagnosing the Harrier-2  */
/*              I/O subsystem. Command structures are taken from the      */
/*              diagnostic applications and passed to the Harrier-2 Device*/
/*              Driver.  The Device Driver operates in "passthrough" mode */
/*              The SCSI command is transferred to the Device Driver for  */
/*              the resident Harrier-2 adapter card.                      */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*   ROUTINES:  mfg_certify() , random_seek()                             */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:  ioctl(), memcpy(), memset(),            */
/*                                time(), getpid(),                       */
/*                                rand(), srand()                         */
/*                                                                        */
/*          in DEBUGSCSITU mode:  printf()                                */
/*                                                                        */
/*                                                                        */
/*      NOTE:  exectu() in har2_atu.c uses one level of recursion for the */
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
/*   DATA STRUCTURES:                                                     */
/*       tucb_ptr - input structure, contains details of TU to be run     */
/*       iocmd    - structure that is passed as a parameter to the        */
/*                  Device Driver via ioctl().                            */
/*   INPUTS:                                                              */
/*       fdes     - file descriptor of Harrier-2 device.                  */
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
/* (-95) SCATU_QUEQUE_FULL            - Harrier-2 Controller queue is full*/
/*                                      No more SCSI command can be       */
/*                                      accepted by Harrier-2 at this time*/
/*                                      (Resend SCSI command later)       */
/* (-96) SCATU_NONEXTENDED_SENSE      - data obtained from a request      */
/*                                      sense command was not in          */
/*                                      extended format                   */
/* (-97) SCATU_IO_BUS_ERROR          - Microchannel I/O failure           */
/* (-98) SCATU_ADAPTER_FAILURE       - failure of the Harrier-2 Adapter   */
/* (-99) SCATU_BAD_PARAMETER         - invalid input parameter            */
/**************************************************************************/

/* Include Files                                                          */
#include <diag/har2_atu.h>     /* Structure declarations used by ATU's         */
#include <sys/serdasd.h>

/* Remove comments below for debug mode                                   */
/* #define DEBUGSCSITU */

/**************************************************************************/
/*                                                                        */
/* NAME:  mfg_certify                                                     */
/*                                                                        */
/* FUNCTION: This ATU performs a read scan of all user data blocks on the */
/*           addressed hardfile.                                          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:  This routine is local to this file.            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/

int
mfg_certify(fdes, tucb_ptr)
int fdes;
HAR2_TUTYPE *tucb_ptr;       /* details of test unit to be run            */
{
   uchar read_buffer[25600]; /* used for ATU return data                  */
   HAR2_TUTYPE r_tucb;       /* used to send commands                     */
   unsigned maxblock;        /* maximum LBA on hardfile                   */
   int thisblock;       /* LBA currently seeking for                 */
   unsigned reas_block;      /* used for reassignment of LBA              */
   unsigned tmp;             /* temporary variable                        */
   int loopcnt;         /* temporary variable                        */
   FILE *temp;               /* file pointer for temp data                */
   int logical_unit;         /* logical unit of device                    */
   int count;                /* number of blocks to read per              */
                             /* Read Extended command                     */

  int rval;                  /* for ATU return code                       */
  int blocksize;             /* number of bytes per LBA                   */

  int reas_blk_flag;         /* Reassign block flag - only 1              */
                             /* block reassignment allowed                */

  int retry_count;           /* Read Extended CMD retry count             */

  int rc;                    /* Return Code                               */

  /* get the logical unit number */
  logical_unit = (tucb_ptr->scsitu.scsi_cmd_blk[1] >> 5) & 0x07;

  /* test unit ready */
  r_tucb.header.tu = SCATU_TEST_UNIT_READY;
  tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
  rval = scsitu_init( &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );
  r_tucb.header.mfg = 1;

  /* insert the logical unit */
  r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

  rval = exectu( fdes, &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );

  /* read capacity */
  r_tucb.header.tu = SCATU_READ_CAPACITY;
  tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
  rval = scsitu_init( &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );
  r_tucb.header.mfg = 1;
  r_tucb.scsitu.data_length = 8;
  r_tucb.scsitu.data_buffer = read_buffer;

  /* insert the logical unit */
  r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

  rval = exectu( fdes, &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );

  /* maxblock is the last LBA on the hardfile */
  maxblock = (unsigned) read_buffer[0] << 24 |
             (unsigned) read_buffer[1] << 16 |
             (unsigned) read_buffer[2] << 8 |
             (unsigned) read_buffer[3];

  /* blocksize is the number of bytes in the last LBA/sector */
  /* on the hardfile (typically 512 bytes/LBA)               */
  blocksize = (unsigned) read_buffer[4] << 24 |
              (unsigned) read_buffer[5] << 16 |
              (unsigned) read_buffer[6] << 8 |
              (unsigned) read_buffer[7];

  if( blocksize > 512 )
  /* must increase constant for buffer size */
    return( SCATU_BAD_PARAMETER );

  loopcnt = maxblock+1;/* Loopcnt is set to maxblock + 1 so  */
                       /* that all user data blocks from     */
                       /* LBA=0 to LBA=maxblock are read     */

  count = 50;   /* count is the number of blocks to read each*/
                /* time a Read Extended command is issued    */

  thisblock = loopcnt - count; /* starting LBA */

  /* The maximum number of blocks which can be read at one   */
  /* time is limited by SC_MAXREQUEST, which is defined in   */
  /* scsi.h                                                  */

  if (( count * 512) > SC_MAXREQUEST )
     return( SCATU_BAD_PARAMETER );

  /* Set PER bit in mode_select */

    /* First, read in current mode sense data */

  r_tucb.header.tu = SCATU_MODE_SENSE;
  rc = scsitu_init(&r_tucb);          /* Initiallize HAR2 TUTYPE structure*/
  if (rc != SCATU_GOOD)  return(rc);

     /* Logical Unit Number would normally come from the ODM        */
  r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

  memset(read_buffer, 0x0ff, 255);    /* zero out the ATU data buffer */

  r_tucb.scsitu.scsi_cmd_blk[2] = 0x3f;   /* get current mode sense */
  r_tucb.scsitu.scsi_cmd_blk[4] = 255;
  r_tucb.scsitu.data_length = 255;
  r_tucb.scsitu.data_buffer = read_buffer;

  rc = exectu(fdes, &r_tucb);
  if (rc != SCATU_GOOD)  return(rc);

    /* Second, change mode sense data and then do mode select */

  /* Clear PS bits */
  read_buffer[12] &= 0x7f;
  read_buffer[15] &= 0x7f;
  read_buffer[18] &= 0x7f;
  read_buffer[21] &= 0x7f;

  /* Mode Select(6) data setup */

/*
CURRENT Mode sense data =
       17 00 00 08 00 19 8f bb 00 00 02 00 81 01 20 87 01 05 88 01 00 80 01 00
byte # 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
*/

  /* Mode Select Header */
  read_buffer[0]  = 0x00;   /* reserved */
  read_buffer[1]  = 0x00;   /* medium type */
  read_buffer[2]  = 0x00;   /* wp and cache */
  read_buffer[3]  = 0x08;   /* block descriptor length */

  /* Block Descriptor */
  read_buffer[4]  = 0x00;   /* reserved */
  read_buffer[5]  = 0x00;   /* number of blocks (MSB)  0x00 = all */
  read_buffer[6]  = 0x00;   /* number of blocks */
  read_buffer[7]  = 0x00;   /* number of blocks (LSB) */
  read_buffer[8]  = 0x00;   /* reserved */
  read_buffer[9]  = 0x00;   /* block length (MSB) 512 bytes(0x000200) */
  read_buffer[10] = 0x02;   /* block length */
  read_buffer[11] = 0x00;   /* block length (LSB) */

  /* Page 1 starts at byte 12 */

  read_buffer[14] |= 0x04; /* enable PER bit */

  r_tucb.header.tu = SCATU_MODE_SELECT;
                                   /* Initiallize HAR2 TUTYPE structure*/
  if ((rc = scsitu_init(&r_tucb)) != SCATU_GOOD) return(rc);

     /* Logical Unit Number would normally come from the ODM        */
  r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

  r_tucb.scsitu.scsi_cmd_blk[4] = 24;    /* 24 bytes of data */
  r_tucb.scsitu.data_length = 24;
  r_tucb.scsitu.data_buffer = read_buffer; /* put data in buffer */
/*
 for (i=0;i<24;i++)
 printf("%02x ",read_buffer[i]);
 putchar('\n');
*/

  rc = exectu(fdes, &r_tucb);
  if (rc != SCATU_GOOD)  return(rc);

  /* Enable Soft Error reporting */

  r_tucb.header.tu = SCATU_ENABLE_SOFT_ERRORS;
  tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
  rval = scsitu_init( &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );
  r_tucb.header.mfg = 1;

  /* insert the logical unit */
  r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

  rval = exectu( fdes, &r_tucb );
  if( rval != SCATU_GOOD )
    return( rval );

#ifdef DEBUGSCSITU
printf("\nExecuting the Certify Unit ATU \n");
#endif

  retry_count = 0;
  reas_blk_flag = 0;
  reas_block = 0;

  /* Certify hardfile from LBA = maximum to LBA = 0 */
  while( loopcnt )  {

  /* ATU Read Extended is used to verify the user data   */
  /* on the addressed hardfile                           */

       /* verify a group of 50 blocks */
       r_tucb.header.tu = SCATU_READ_EXTENDED;
       tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
       rval = scsitu_init( &r_tucb );
       if( rval != SCATU_GOOD )
         return( rval );
       r_tucb.header.mfg = 1;

       r_tucb.scsitu.data_length = blocksize * count;
       r_tucb.scsitu.data_buffer = read_buffer;

       /* insert the logical unit */
       r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

       /* insert the block number */
       r_tucb.scsitu.scsi_cmd_blk[2] = thisblock >> 24;
       r_tucb.scsitu.scsi_cmd_blk[3] = thisblock >> 16;
       r_tucb.scsitu.scsi_cmd_blk[4] = thisblock >> 8;
       r_tucb.scsitu.scsi_cmd_blk[5] = thisblock;

       /* insert the block count */
       r_tucb.scsitu.scsi_cmd_blk[7] = 0;
       r_tucb.scsitu.scsi_cmd_blk[8] = count;

       rval = exectu( fdes, &r_tucb );

       /* if the return code is not a check condition or a */
       /* good return code the function returns the value  */

       if(( rval != 0) && ( rval != -12 ))
         return( rval );

       if(rval == -12) {  /* Check Condition Status */
         /* Issue a Request Sense for the 'Check Condition' */
         r_tucb.header.tu = SCATU_REQUEST_SENSE;
         tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
         rval = scsitu_init( &r_tucb );
         if( rval != SCATU_GOOD )
           return( rval );
         r_tucb.header.mfg = 1;
         r_tucb.scsitu.data_length = 255;
         r_tucb.scsitu.data_buffer = read_buffer;

         /* insert the logical unit */
         r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

         rval = exectu( fdes, &r_tucb );

         tucb_ptr->scsiret.sense_key = r_tucb.scsiret.sense_key;
         tucb_ptr->scsiret.sense_code = r_tucb.scsiret.sense_code;
         tucb_ptr->scsiret.host_action_code =
                                  r_tucb.scsiret.host_action_code;
         tucb_ptr->scsiret.unit_error_code =
                                  r_tucb.scsiret.unit_error_code;
         tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
         tucb_ptr->scsiret.status_validity =
                                  r_tucb.scsiret.status_validity;
         tucb_ptr->scsiret.scsi_bus_status =
                                  r_tucb.scsiret.scsi_bus_status;
         tucb_ptr->scsiret.adapter_status =
                                  r_tucb.scsiret.adapter_status;
         if( rval != SCATU_GOOD )
           return( rval );

   /* Display sense data for an error during the Read Extended command */
  #ifdef DEBUGSCSITU
  printf("\nRequest Sense returned data :");
  printf("\nSense Key  = %1x", r_tucb.scsiret.sense_key);
  printf("\nSense Code = %4x", r_tucb.scsiret.sense_code);
  printf("\nHost Action Code = %2x", r_tucb.scsiret.host_action_code);
  printf("\nUnit Error Code  = %4x", r_tucb.scsiret.unit_error_code);
  printf("\nReading blocks: LBA = %ld - %ld",thisblock,(thisblock+(count-1)));
  #endif
         /* Determine what the Read Extended failure is */
         if(r_tucb.scsiret.sense_key == 0x00)  /* sense key */
         {
            /* no sense data available */
           return(-70); /* no sense data was returned */
         }
         else if(r_tucb.scsiret.sense_key == 0x01)
         { /* recovered error */
               ++tucb_ptr->scsiret.rec_errs;
               switch( r_tucb.scsiret.sense_code )
               {
                  case 0x1700:
                  case 0x8400:
                     ++tucb_ptr->scsiret.soft_thres_errs;
                     break;
                  case 0x1801:
                  case 0x8500:
                     ++tucb_ptr->scsiret.soft_equip_chks;
                     if((read_buffer[0] & 0x80) == 0) /* valid bit */
                       return(-73); /* LBA not defined */
                     reas_block = (unsigned) read_buffer[3] << 24 |
                                  (unsigned) read_buffer[4] << 16 |
                                  (unsigned) read_buffer[5] <<  8 |
                                  (unsigned) read_buffer[6];
                     if(reas_blk_flag == 1)
                     {
                       /* check if this LBA is the same as the last */
                       if(tucb_ptr->scsiret.reas_lba == reas_block)
                         return(-72); /* same LBA in error as last */
                       return(-75); /* > 1 LBA requires reassignment */
                     }
                     tucb_ptr->scsiret.reas_lba = reas_block;
                     reas_blk_flag = 1;
                     /* Issue a Reassign Block command */
                     r_tucb.header.tu = SCATU_REASSIGN_BLOCK;
                     tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
                     rval = scsitu_init( &r_tucb );
                     if( rval != SCATU_GOOD )
                       return( rval );
                     r_tucb.header.mfg = 1;
                     r_tucb.scsitu.data_length = 8;
                     /* set up defect list */
                     /* Defect List Header */
                     read_buffer[0] = 0x00;
                     read_buffer[1] = 0x00;
                     read_buffer[2] = 0x00;
                     read_buffer[3] = 0x04;
                     /* Defect Descriptor */
                     read_buffer[4] = reas_block >> 24;
                     read_buffer[5] = reas_block >> 16;
                     read_buffer[6] = reas_block >>  8;
                     read_buffer[7] = reas_block;
                     r_tucb.scsitu.data_buffer = read_buffer;

                     /* insert the logical unit */
                     r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

                     rval = exectu( fdes, &r_tucb );
                     if( rval != SCATU_GOOD )
                       return( rval );
                      /* end case for 1801 and 8500 sense_code */
                     break;
                  case 0x5B01:
                  case 0x3200:
                  default:
                     ++tucb_ptr->scsiret.unrec_errs;
                     ++tucb_ptr->scsiret.hard_equip_chks;
                     break;
               } /* end switch for r_tucb.scsiret.sense_code */
         } /* end if 0x01 for recovered error */
         else if(r_tucb.scsiret.sense_key == 0x03)
         { /* non-recoverable error */
               ++tucb_ptr->scsiret.unrec_errs;
               switch( r_tucb.scsiret.unit_error_code ) {
                  case 0x0285: { /* case for hard data error */
                     ++tucb_ptr->scsiret.hard_data_errs;
                     if((read_buffer[0] & 0x80) == 0) /* valid bit */
                       return(-73); /* LBA not defined */
                     reas_block = (unsigned) read_buffer[3] << 24 |
                                  (unsigned) read_buffer[4] << 16 |
                                  (unsigned) read_buffer[5] <<  8 |
                                  (unsigned) read_buffer[6];
                     if(reas_blk_flag == 1) {
                       /* check if this LBA is the same as the last */
                       if(tucb_ptr->scsiret.reas_lba == reas_block)
                         return(-72); /* same LBA in error as last */
                       return(-75); /* > 1 LBA requires reassignment */
                     } /* endif */
                     tucb_ptr->scsiret.reas_lba = reas_block;
                     reas_blk_flag = 1;
                     /* Issue a Reassign Block command */
                     r_tucb.header.tu = SCATU_REASSIGN_BLOCK;
                     tucb_ptr->scsiret.atu_num = r_tucb.header.tu;
                     rval = scsitu_init( &r_tucb );
                     if( rval != SCATU_GOOD )
                       return( rval );
                     r_tucb.header.mfg = 1;
                     r_tucb.scsitu.data_length = 8;
                     /* set up defect list */
                     /* Defect List Header */
                     read_buffer[0] = 0x00;
                     read_buffer[1] = 0x00;
                     read_buffer[2] = 0x00;
                     read_buffer[3] = 0x04;
                     /* Defect Descriptor */
                     read_buffer[4] = reas_block >> 24;
                     read_buffer[5] = reas_block >> 16;
                     read_buffer[6] = reas_block >>  8;
                     read_buffer[7] = reas_block;
                     r_tucb.scsitu.data_buffer = read_buffer;

                     /* insert the logical unit */
                     r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

                     rval = exectu( fdes, &r_tucb );
                     if( rval != SCATU_GOOD )
                       return( rval );
                     } /* end case 0x0285 for hard data error */
                     break;

                  default:
                     /* hard equipment checks */
                     ++tucb_ptr->scsiret.unrec_errs;
                     ++tucb_ptr->scsiret.hard_equip_chks;
                     ++retry_count;
                     if( retry_count > 2 )
                       return(-71); /* hard equip retry failure */
                      /* end default for hard equipment checks */
                     break;
                  } /* end switch for non-recoverable unit_error_code */
         } /* end if for 0x03 non-recoverable error */
         else if(r_tucb.scsiret.sense_key == 0x04)
         { /* non-recoverable hardware error */
               ++tucb_ptr->scsiret.unrec_errs;
               ++tucb_ptr->scsiret.hard_equip_chks;
               rc = 0;
               switch (r_tucb.scsiret.unit_error_code & 0x0f)
               {
                  case 0x00:
                  case 0x02:
                  case 0x03: /* retry recovery action */
                     ++retry_count;
                     if( retry_count > 2 )
                       rc = -71; /* hard equip failure after retry */
                     break;
                  case 0x01:
                  case 0x04:
                     rc = -74; /* permanent hardware failure */
                     break;
                  default:
                     rc = -73; /* unexpected error */
                     break;
               } /* endswitch */
               if (rc < 0) return(rc);
         } /* end if 0x04 non-recoverable hardware error */
         else    /* unexpected error */
         {
               return(-73);
         } /* end if for Sense Key */
         } /* end if for rval = -12 Check Condition Status */

       else {  /* rval == 0 Read Extended command successful */

  /* This debug block writes the returned data into a temp file buffer  */
  #ifdef DEBUGSCSITU
  if (thisblock == ((maxblock+1) - 50)) {
     temp = fopen("block", "w+");  /* open file 'current dir'.block */
     fprintf(temp, "CERTIFY UNIT DEBUG RESULTS:\n");
     fprintf(temp, "\nMaxblock LBA = %ld", maxblock);
     fprintf(temp, "\nThisblock LBA = %ld", thisblock);
     fprintf(temp, "\nReturn code = %d\n", rval);
     fprintf(temp, "\nRead Extended data = :\n");
     rval = 0;
     while( rval < 25600 ) {
       fprintf(temp, "%02.2x", read_buffer[rval++]);
       if ( !(rval % 27))
          fprintf(temp, "\n");
       } /* endwhile */
     fclose(temp);
  } /* endif */
  #endif

         retry_count = 0;
         loopcnt -= count;
         /* number of user data blocks read successfully so far */
         tucb_ptr->scsiret.blks_read = (maxblock + 1) - loopcnt;
         if(( thisblock - count) <= 0) {
           thisblock = 0;
           count = loopcnt;
           if(count == 0)
             break;
         } /* end if */
         else
           thisblock -= count;
       } /* end of else for rval == 0 */
  } /* end of while( loopcnt ) */
  return(SCATU_GOOD); /* Certify Unit completed successfully */
} /* end of certify() */

/**************************************************************************/
/*                                                                        */
/* NAME:  random_seek                                                     */
/*                                                                        */
/* FUNCTION: This ATU performs 2000 random reads of user data blocks on   */
/*           the addressed hardfile.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:  This routine is local to this file.            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: rand() , srand()                           */
/*                                                                        */
/**************************************************************************/

int
random_seek(fdes, tucb_ptr)
int fdes;
HAR2_TUTYPE *tucb_ptr;       /* details of test unit to be run            */
{
   uchar read_buffer[4096];  /* used for read command                     */
   HAR2_TUTYPE r_tucb;       /* used to send commands                     */
   unsigned maxblock;        /* max block on disk                         */
   unsigned thisblock;       /* block currently seeking for               */
   unsigned expand_rand;     /* how much to multiply value                */
                             /* from rand() by to cover all               */
                             /* blocks on device                          */
   int logical_unit;         /* logical unit number of device             */
   int rval;                 /* temporary for return value                */
   int loopcount;            /* number of random seeks                    */
   int blocksize;            /* size of blocks on the disk                */
   int rand();               /* generates a random number 0 < x < 2^31    */
   void srand();             /* initializes the random numbers            */

   /* get the logical unit number */
   logical_unit = (tucb_ptr->scsitu.scsi_cmd_blk[1] >> 5) & 0x07;

   /* number of random seeks to perform */
   loopcount = tucb_ptr->header.loop;

   /* set the random number seed */
   srand( tucb_ptr->scsitu.seed_value );

   /* test unit ready */
   r_tucb.header.tu = SCATU_TEST_UNIT_READY;
   rval = scsitu_init( &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );
   r_tucb.header.mfg = 1;

   /* insert the logical unit */
   r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;


   rval = exectu( fdes, &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );

   /* read capacity */
   r_tucb.header.tu = SCATU_READ_CAPACITY;
   rval = scsitu_init( &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );
   r_tucb.header.mfg = 1;
   r_tucb.scsitu.data_length = 8;
   r_tucb.scsitu.data_buffer = read_buffer;

   /* insert the logical unit */
   r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

   rval = exectu( fdes, &r_tucb );
   if( rval != SCATU_GOOD )
     return( rval );

   maxblock = (unsigned) read_buffer[0] << 24 |
              (unsigned) read_buffer[1] << 16 |
              (unsigned) read_buffer[2] << 8 |
              (unsigned) read_buffer[3];

   blocksize = (unsigned) read_buffer[4] << 24 |
               (unsigned) read_buffer[5] << 16 |
               (unsigned) read_buffer[6] << 8 |
               (unsigned) read_buffer[7];

   if( blocksize > 4096 )
   /* must increase constant for buffer size */
     return( SCATU_BAD_PARAMETER );

   /* calculate value to multiply the random numbers by    */
   /* to get a value that can be anywhere on the disk      */
   if( maxblock < 32767 )
     expand_rand = 1;
   else
     expand_rand = maxblock / 32767 + 1;

   while( loopcount-- ) {
       /* get a random block */
       do {
            thisblock = (unsigned) rand() * expand_rand;
       }while( thisblock > maxblock );


       /* verify that block */
       r_tucb.header.tu = SCATU_READ_EXTENDED;
       rval = scsitu_init( &r_tucb );
       if( rval != SCATU_GOOD )
         return( rval );
       r_tucb.header.mfg = 1;
       r_tucb.scsitu.flags &= ~B_WRITE;
       r_tucb.scsitu.flags |= B_READ;
       r_tucb.scsitu.data_length = blocksize;
       r_tucb.scsitu.data_buffer = read_buffer;

       /* insert the logical unit */
       r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

       /* insert the block number */
       r_tucb.scsitu.scsi_cmd_blk[2] = thisblock >> 24;
       r_tucb.scsitu.scsi_cmd_blk[3] = thisblock >> 16;
       r_tucb.scsitu.scsi_cmd_blk[4] = thisblock >> 8;
       r_tucb.scsitu.scsi_cmd_blk[5] = thisblock;

       /* insert the block count */
       r_tucb.scsitu.scsi_cmd_blk[7] = 0;
       r_tucb.scsitu.scsi_cmd_blk[8] = 1;

       rval = exectu( fdes, &r_tucb );  /* ignore rval */

       /* request sense */
       r_tucb.header.tu = SCATU_REQUEST_SENSE;
       rval = scsitu_init( &r_tucb );
       if( rval != SCATU_GOOD )
         return( rval );
       r_tucb.header.mfg = 1;
       r_tucb.scsitu.flags &= ~B_WRITE;
       r_tucb.scsitu.flags |= B_READ;
       r_tucb.scsitu.data_length = 255;
       r_tucb.scsitu.data_buffer = read_buffer;

       /* insert the logical unit */
       r_tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;

       rval = exectu( fdes, &r_tucb );
       if( rval != SCATU_GOOD )
         return( rval );
       if( r_tucb.scsiret.sense_key == 1 )
         /* recovered error */
         ++tucb_ptr->scsiret.rec_errs;
       else if( r_tucb.scsiret.sense_key == 3 )
              /* medium error */
              ++tucb_ptr->scsiret.unrec_errs;
       else if( r_tucb.scsiret.sense_key == 4 )
              /* hardware error */
              ++tucb_ptr->scsiret.unrec_errs;
   } /* bottom of while( loopcount-- ) */
return( rval );
} /* end rand_seek */
