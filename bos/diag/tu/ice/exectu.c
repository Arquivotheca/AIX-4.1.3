static char sccsid[] = "@(#)75	1.1  src/htx/usr/lpp/htx/lib/hga/exectu.c, tu_hga, htx410 6/2/94 11:37:00";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: exectu
 *		log_error
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include<stdio.h>
#include "hga_tu.h"      /* S3 TU header files */
#include "colors.h"

/* Error codes returned from this module */
#define LOOP_COUNT_WAS_ZERO     0xA1
#define INVALID_TU_NUMBER       0xFF

/* Constants used in this file */

/* Function Prototypes */

unsigned long exectu( char *logical_devname, TU_TYPE *tucb_ptr );
unsigned long log_error(ERROR_DETAILS error);


/* GLOBAL variables which are accessed in other files */

   /* Global pointer to the Test Unit Control Block */
   TU_TYPE *hga_tucb;

   /* GLOBAL variables private to this file */
   static unsigned errors_logged;

/*
 * NAME: exectu
 *
 * FUNCTION: This function is the external interface for the execution of
 *           the test units
 *
 * EXECUTION ENVIRONMENT: Process only.
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns zero if successful, else returns error code.
 */
/*
 * INPUT:
 *        logical_devname - Logical name for device to be tested (e.g. hga0)
 *        tucb_ptr - pointer to the test unit control block
 * OUTPUT:
 *
 */

unsigned long exectu( char *logical_devname, TU_TYPE *tucb_ptr )
{
   unsigned long ret_code = 0;

   /* initialize variables */

   hga_tucb = tucb_ptr; /* global TUCB pointer */
   errors_logged = 0;  /* static global variable used in this file */

   /* If loop count requested is zero, return indicating no execution of TU */
   if (!tucb_ptr->loop) {
     tucb_ptr->error_log_count = errors_logged;
     return(LOOP_COUNT_WAS_ZERO);
   } /* endif */

   /* Execute Requested Test Unit */

   switch (tucb_ptr->tu) {
   case RAMDAC_BASIC_TEST:
      while (tucb_ptr->loop && !ret_code) {
         ret_code = s3_test_bt485();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case VRAM_TEST:
      while (tucb_ptr->loop && !ret_code && !errors_logged) {
         ret_code = s3vram_test();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case GM_TEST:
      while (tucb_ptr->loop && !ret_code) {
         ret_code = s3_GMTest();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case LINES_DRAW_TEST:
      while (tucb_ptr->loop && !ret_code) {
         ret_code = lines_test();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case WINDOWING_TEST:
      while (tucb_ptr->loop && !ret_code) {
         ret_code = draw_rec();
         tucb_ptr->loop--;
      } /* endwhile */
      break;


   case TU_OPEN:
      ret_code = open_normal_dd(logical_devname);
      break;

   case TU_CLOSE:
	sleep(2);
      ret_code = close_normal_dd();
      break;

   case RED_SCREEN:
	s3_clear_screen(CLR_RED);
   	break;
   case GREEN_SCREEN:
   	s3_clear_screen(CLR_GREEN);
	break;
   case BLUE_SCREEN:
	s3_clear_screen(CLR_BLUE);
	break;
   case WHITE_SCREEN:
   	s3_clear_screen(CLR_WHITE);
	break;
   case BLACK_SCREEN:
   	s3_clear_screen(CLR_BLACK);
	break;

   default:

      ret_code = INVALID_TU_NUMBER;       /* Invalid TU number passed! */

   } /* endswitch */

   /* update actual number logged */
   tucb_ptr->error_log_count = errors_logged;

   /* detect the case where errors were logged but returning zero! */
   if (errors_logged && !ret_code) {
     ret_code = tucb_ptr->error_log[0].error_code;
   } /* endif */

   return(ret_code);

} /* end exectu() */


/*
 * NAME: log_error
 *
 * FUNCTION: Logs an error details structure in the error log.
 *
 * This function is to be called by any routine which wishes to
 * log an error. If the return value is zero, then the calling routine
 * may continue. However, if the return value is non-zero
 * it indicates that the error log has just become full after processing
 * the current request, or is full and therefore the current request
 * cannot be processed. If a non-zero value is returned, then the calling
 * routine should discontinue further testing and return. The returned
 * non-zero value will be equal to the last return code that was passed
 * in to be logged.
 *
 * EXECUTION ENVIRONMENT: Process only.
 *
 * NOTES: None
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES:
 *          error_log - member of the TUCB structure accessed globally.
 *          error_log_count - member of the TUCB structure accessed globally.
 *          errors_logged - global variable
 *
 * RETURNS: zero if error log still has room, non-zero if log is full.
 */
/*
 * INPUT:
 *        error - ERROR_DETAILS structure containing error details to be logged
 * OUTPUT:
 *        if errors_logged is less than error_log_count, copies the input
 *        error details structure to error_log[errors_logged] and increments
 *        errors_logged.
 */
unsigned long log_error(ERROR_DETAILS error)
{
  unsigned long ret_code = 0;

  if (errors_logged < hga_tucb->error_log_count) {
    hga_tucb->error_log[errors_logged] = error;
    errors_logged++;
  } /* endif */

  if (errors_logged == hga_tucb->error_log_count) {
    ret_code = error.error_code;
  } /* endif */

  return(ret_code);

} /* end log_error() */

