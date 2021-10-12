static char sccsid[] = "@(#)96  1.3  src/bos/diag/tu/artic/st_task.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:24";
/* COMPONENT_NAME:  
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
 *
 * FUNCTIONS: start_task, sptsk_retcode
 *
 */
#include <stdio.h>
#include <artictst.h>
typedef unsigned char byte;

/*
 * NAME: START_TASK
 *
 * FUNCTION: Start the task on the adapter card
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will start the individual tasks on the adapter and
 *          wait for their completition.  It will also check to see if and
 *          error conditions have occured while the task was running
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int start_task (fdes, tucb_ptr, command_type, error_code)
   int fdes;
   TUTYPE *tucb_ptr;
   int command_type; /* Command Number to be executed */
   byte *error_code; /* Error code returned           */
   {
        unsigned short offset;  /* Offset within the 64 byte array */
        int rc;                 /* Return Code */
        short shrc;             /* Return code for Device Driver calls  */
        unsigned long timeout;  /* delay time required before interrupt quits */
        unsigned char primstat;  /* Tasks primary status byte */
        byte tmp;

        extern int outrm();
        extern int dh_diag_mem_write();
        extern int dh_diag_mem_read();
        extern int sptsk_ret_code();

        /* Clear the error byte */
        if ((rc=outrm(fdes,tucb_ptr,TASK_ERROR_STATUS,0x00,REG_ERR11)) != 0)
           {
                 return(rc);
           }

        /* INITIATE THE COMMAND */
        if (shrc = icaintreg(fdes))
           {
                 return(DRV_ERR);
            }

        /* Set up the Command Number */
        if (shrc = icaissuecmd(fdes, (unsigned char) command_type, 0,
                               FIVE_SECONDS, NULL))
        {
                 return(DRV_ERR);
        }

        if (shrc = icaintwait(fdes, TWO_MINUTES))
           {
              icaintdereg(fdes);
              return(TUTCTO);
           }

        /* This is Interrupt Status Test portion of the START_TASK Procedure */
        /* Check if an error occured, put result in error_code */
        /* Interrupt Status */
        *error_code = 0;
        if ((shrc = icagetprimstat(fdes, &primstat)) != 0)
           {
              icaintdereg(fdes);
              return(DRV_ERR);
            }

         rc = 0;

         if (primstat != TASK_RUNNING)
           {
              if (tucb_ptr->header.tu != SP5DSP_TEST)  /* Need special handling */
                {                                      /* for SP 5 DSP test     */
                 if ((rc = dh_diag_mem_read(fdes,tucb_ptr,0,
                      (unsigned short)TASK_ERROR_STATUS, error_code)) != 0)
                    {
                      return(DDINPR7);
                    }

                 if ((rc = dh_diag_mem_read(fdes,tucb_ptr,0,
                      (unsigned short)0x430, &tmp)) != 0)
                    {
                      return(DDINPR7);
                    }

                }
                else
                   rc = sptsk_retcode(fdes, tucb_ptr);
           }

        icaintdereg(fdes);
        return(rc);
   }

/*
 * NAME: Get Sandpiper EIB test status
 *
 * FUNCTION: Get error code from Sandpiper V message area
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will return any errors that were caused as a result
 *          of the execution of tu071. The error code is put by the task        .
 *          at a fixed location in memory in ascii format. This function reads
 *          the status and converts is to decimal.
 *
 * RETURNS: the status read from the device.
 *
 */
int sptsk_retcode(int fdes, TUTYPE *tucb_ptr)
{
   int i, j;
   unsigned char buf[4];
   int rc = 0;

   buf[0] = 0;   /* Zero out most significant nibble */

   if (icareadmem(fdes, 3, 0x0, SP5_STAT_OFFSET + 1, 0xFF, &buf[1]))
   {
       return(DRV_ERR);
   }

   for (j = 0, i = 3; i >= 1 ; i--, j+=4)  /* Convert from ascii */
   {
       if ((buf[i] >= '0') && (buf[i] <= '9'))
          buf[i] -= '0';
       else
          if ((buf[i] >= 'A') && (buf[i] <= 'F'))
             buf[i] += (0x0A - 'A');
          else
             return(UNKNOWN_SP5_ERR);

       if (j)
          rc |= (buf[i] << j);
       else
          rc = buf[i];
   } /* endfor */

   return(SP5_ERROR_BASE | rc);
}
