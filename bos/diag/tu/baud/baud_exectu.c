static char sccsid[] = "@(#)74	1.2  src/bos/diag/tu/baud/baud_exectu.c, tu_baud, bos411, 9439B411a 9/29/94 10:56:38";
/*
 * COMPONENT_NAME: tu_baud
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: exectu.c
 *
 * STATUS: Release 1, EVT Version 1
 *
 * DEPENDENCIES:
 *   1. The BAUD "diagnostics" device driver must be installed.
 *
 * RESTRICTIONS: None
 *
 ****************************************************************************/


#include <stdio.h>
#include <fcntl.h>
#include "baud_exectu.h"     /* THIS FILE'S HEADER */

int dd_FileDes = -1;           /* EXTERNAL VARIABLES */
struct TestResults *Results;   /* EXTERNAL VARIABLES */
unsigned int CommandStatus;    /* EXTERNAL VARIABLES */


/*****************************************************************************
 * NAME: exectu
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: This function executes under a process
 *
 * NOTES:
 *   1.
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES: None
 *
 * RETURNS:
 *   1.     successful ==> one of the successful return codes from the TUs.
 *   2. NOT successful:
 *     ILLEGAL_TU = 0x00003100 ==> Illegal Test Unit.
 *     one of the NOT successful return codes from the TUs.
 */
/*
 * DEPENDENCIES: None
 *
 * RESTRICTIONS: None
 *
 * INPUT: None
 *
 * OUTPUT: None
 ****************************************************************************/

char *pos;    /* direct ptr to pos regs */
char *ioa;     /* direct ptr to io regs */


int exectu( char *devname, TU_PTR_TYPE tup)
{
int
    ErrCode,        /* ERROR CODE PART OF RETURN CODE */
    i,               /* GENERAL PURPOSE FOR LOOP COUNTER */
    LoopCount,       /* Contains number of times the TU is to be executed  */
    RetCode,        /* RETURN CODE FROM TUs with TU number */
    TU_Number;       /* Contains the TU number to be executed */
    extern   int   errno;
/*--------------------------------------------------------------------------*/

   CommandStatus = 0xE9;      /* NO INTERRUPTS, OPERATING STATE */
   LoopCount = tup->header.loop;
   TU_Number = tup->header.tu;

   if((dd_FileDes == -1) && (TU_Number != TU_OPEN)) return(ILLEGAL_TU);

   for ( i = 0; i < LoopCount; i++ )  {
     switch(TU_Number)
        {
        case TU_OPEN:  /* Initialize Audio card for diagnostics */
          {
          extern int  errno;

          /* Configure the device */
          /* Open the device driver for the adapter */
          if ((dd_FileDes = open( devname, O_RDWR)) < 0){
            printf("\nCANNOT OPEN DEVICE\n");
            RetCode = errno;
            } /* endif */
          else
            {
            dd_FileDes = dd_FileDes;
       /*   printf("FileDes = %x\n", dd_FileDes);      */
            ioctl(dd_FileDes, GET_PTR_TO_IOCC, &pos);
            ioctl(dd_FileDes, GET_PTR_TO_REGS, &ioa);
       /*   printf("REG ADDR = %x\n", pos);
            printf("IOC ADDR = %x\n", ioa);            */
            i = LoopCount;     /* do only once */
            RetCode = 0;
            };
          break;
          };
        case TU_VPD_CHECK:  /* VPD check */
          {
          adapter_reset();
          RetCode = vpd_check(&tup->secondary_ptr[0]);
          break;
          };
        case TU_MCI_CHIP:  /* MCI check */
          {
          adapter_reset();
          RetCode = mci_timer_test(&tup->secondary_ptr[0]);
          break;
          };

        case TU_CODEC_TEST:  /* CODEC register test */
          {
          adapter_reset();
          wait_audio_ready();
          RetCode = codec_register_test(&tup->secondary_ptr[0]);
          break;
          };

        case TU_ADAPTER_RESET:  /* Codec setup test */
          {
          adapter_reset();
          wait_audio_ready();
          RetCode = 0;
          break;
          };

        case TU_DIGITAL_LOOP:  /* Digital loopback test */
          {
          adapter_reset();
          RetCode = digital_loop(&tup->secondary_ptr[0]);
          RetCode = read_control_regs();
          break;
          };

        case TU_REGISTER_CONTROL: /* Manual register control */
          {
          RetCode = manual();
          break;
          };

        case TU_RECORD_PLAY:  /* Record/playback test */
          {
          adapter_reset();
          RetCode = record_playback(&tup->secondary_ptr[0]);
          break;
          };

        case TU_SIMUL_REC_PLAY:  /* Simul Record/playback test */
          {
          adapter_reset();
          RetCode = record_playback_sim(&tup->secondary_ptr[0]);
          break;
          };

        case TU_HTX_REC_PLAY:  /* Simul Record/playback test */
          {
          adapter_reset();
          RetCode = record_playback_htx(&tup->secondary_ptr[0]);
          break;
          };

        case TU_CLOSE:  /* Close audio adapter for diagnostics */
          {
      /*  adapter_reset();    */
          sleep(3);
          ioctl(dd_FileDes, REL_PTR_TO_IOCC, &pos);
          ioctl(dd_FileDes, REL_PTR_TO_REGS, &ioa);
          close(dd_FileDes);
          RetCode = 0;
          break;
          };



        default:      /* ILLEGAL TST UNIT    */
          RetCode = ILLEGAL_TU;
        }; /* switch */


/*****************************************************************************
** IF A TU RETURNS AN ERROR, THEN BREAK OUT AND RETURN.
*****************************************************************************/

     ErrCode = RetCode & 0x0000ffff;      /* EXTRACT ERROR CODE */
     if ( ErrCode )
           break;  /* OUT OF for LOOP */
     };/* for */


   return( RetCode );
} /* exectu */

