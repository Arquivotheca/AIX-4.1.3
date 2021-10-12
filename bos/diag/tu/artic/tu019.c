static char sccsid[] = "@(#)46  1.2.1.3  src/bos/diag/tu/artic/tu019.c, tu_artic, bos41J, 9508A 2/15/95 17:50:56";
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
 * FUNCTIONS: dh_diag_task, move_code_to_adapter, tu019
 *
 */

/* Stuff for findmcode in lib cfg */
#define PORTM_BASE_NAME   "708fd"
#define SP5_BASE_NAME     "718fd"
#define MP2_BASE_NAME     "f0efd"

static char filename[64];

/*****************************************************************************

Function(s) Test Unit 019 - Load Base Card Diagnostic Task Test Unit

Module Name :  tu019.c HTX

*****************************************************************************/
#include <stdio.h>
#include <cf.h>
#include <artictst.h>

/*
 * NAME: DH_BASE_CARD_DIAG_TASK
 *
 * FUNCTION: Cause the diagnostic code to be loaded and started in the adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will issue the Device Driver's task load IOCTL.
 *          It will then pass the adapter number and variables for the card
 *          page and offset to the device driver.  Once the IOCTL has
 *          completed, the device driver will return with the proper return
 *          code.
 *
 * RETURNS: A zero (0) for successful completion or a one (1) for error.
 *
 */

int dh_base_card_diag_task(fdes, tucb_ptr, basename)
   int fdes;
   TUTYPE *tucb_ptr;
   char *basename;
   {
        extern int load_diag_task();
        int rc=0;

        rc = findmcode(basename, filename, VERSIONING, NULL);
        if (!rc)
           {

                return(NOUCODE2);
           }

        rc = load_diag_task(fdes, filename);
        if (rc)
           {

                return(DRV_ERR);
           }

        return(0);
   }

/*
 * NAME: MOVE_CODE_TO_ARTIC
 *
 * FUNCTION: Load the adapter card with the diagnostic code
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will cause the diagonstic code to be loaded in
 *          the adapter card.  It selects the corect microcode based on the
 *          adapter type
 *
 * RETURNS: A zero (0) for successful completion or non-zero for error.
 *
 */

static int move_code_to_artic(fdes, tucb_ptr, basename)
   int fdes;
   TUTYPE *tucb_ptr;
   char *basename;
   {
        int rc;           /* Return Code */
        extern int dh_diag_card_reset();

        if (icareset(fdes))
           return(DRV_ERR);

        rc = dh_base_card_diag_task(fdes, tucb_ptr, basename);
        if (rc)
           {
                return(rc);
           }

        return(0);
   }

/*****************************************************************************

tu019

*****************************************************************************/

int tu019 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int type;

        if (icagetadaptype(fdes, &type))
           return(DRV_ERR);

        if (IS_PORTMASTER(type) || IS_GALE(type))
           return(move_code_to_artic(fdes, tucb_ptr, PORTM_BASE_NAME));
        else
           if (IS_SP5(type))
              return(move_code_to_artic(fdes, tucb_ptr, SP5_BASE_NAME));
           else
              return(move_code_to_artic(fdes, tucb_ptr, MP2_BASE_NAME));

   }

