static char sccsid[] = "@(#)35  1.7  src/bos/diag/tu/sun/sun_exectu.c, tu_sunrise, bos411, 9437A411a 8/29/94 15:48:37";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: exectu
 *              log_error
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
Module Name :  sun_exectu.c
*****************************************************************************/
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"      /* SUNRISE TU header files */

/* Constants used in this file */

unsigned long exectu(char *device_name, TU_TYPE *tucb_ptr );
unsigned long log_error(ERROR_DETAILS error);


/* GLOBAL variables which are accessed in other files */

   /* Global pointer to the Test Unit Control Block */
   TU_TYPE *sun_tucb;

/* GLOBAL variables private to this file */
#ifndef BUD
   static unsigned errors_logged;
#else
   unsigned errors_logged;
#endif

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
 *        device_name - Logical device name
 *        tucb_ptr - pointer to the test unit control block
 * OUTPUT:
 *
 */

unsigned long exectu(char *device_name, TU_TYPE *tucb_ptr )
{
   unsigned long ret_code = 0;


   /* initialize variables */

   sun_tucb = tucb_ptr; /* global TUCB pointer */
   errors_logged = 0;  /* static global variable used in this file */

   DEBUG_0("\nBEGINNING OF exectu()");

   /* If loop count requested is zero, return indicating no execution of TU */
   if (!tucb_ptr->loop) {
     tucb_ptr->error_log_count = errors_logged;
     return(LOOP_COUNT_WAS_ZERO);
   } /* endif */

   /* Execute Requested Test Unit */

   switch (tucb_ptr->tu) {

   case POS_CHK_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling POS CHECK TEST");
         ret_code = pos_chk();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case BASE_VPD_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling BASE VPD TEST");
         ret_code = test_base_vpd();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case MIAMIREG_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling MIAMI REGISTERS TEST");
         ret_code = TestMiamiRegs();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case PIXELREG_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling PIXEL REGISTERS TEST");
         ret_code = Test2070Regs();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case VIDSETUP_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling VIDEO SETUP TEST");
         ret_code = vidsetup(NTSC_NOINPUT);
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case FRAMEBUFFER_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling FRAME BUFFER PIO TEST");
         ret_code = Test2070MemIO();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case DMA_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling FRAME BUFFER DMA TEST");
         ret_code = FrameBufferDMATest();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case SCALER_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling SCALER TEST");
         ret_code = tscaler();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case CODEC_VPD_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling CODEC VPD TEST");
         ret_code = test_codec_vpd();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case CODECREG_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling CODEC REGISTERS TEST");
         ret_code = Test560Regs();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case FIELDMEM_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling FIELD MEMORY TEST");
         ret_code = TestFieldMem();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case COMPRESS_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling COMPRESS TEST");
         ret_code = CompTest();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case DECOMPRESS_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling DECOMPRESS TEST");
         ret_code = DeCompTest();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case EMC_DMA_TEST:
      while (tucb_ptr->loop && !ret_code) {
         DEBUG_0("\r\nCalling EMC_DMA TEST");
         ret_code = EMC_DMATest();
         tucb_ptr->loop--;
      } /* endwhile */
      break;

   case TU_OPEN:
      ret_code = hw_init(device_name);
      /* if ret_code indicates CODEC card not exist, then set the flag */
      if (ret_code == NO_CODEC) {
        tucb_ptr->codec_present = 0;
        ret_code = OK;  /* Reset the ret_code */
      } else {
        tucb_ptr->codec_present = 1;
      } /* endif */
      break;

   case TU_CLOSE:
      ret_code = hw_clean(device_name);
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

  if (errors_logged < sun_tucb->error_log_count) {
    sun_tucb->error_log[errors_logged] = error;
    errors_logged++;
  } /* endif */

  if (errors_logged == sun_tucb->error_log_count) {
    ret_code = error.error_code;
  } /* endif */

   /* Reset the Sunrise card after card failed */
   /* May not need this in the future ??? */
   if (ret_code=pio_mcwrite (0x5, 0x80, 1)) {
       return(ret_code);
   }
   usleep(100);  /* required at least 50us delay */
   if (ret_code=pio_mcwrite (0x5, 0x0, 1)) {
       return(ret_code);
   }

  return(ret_code);

} /* end log_error() */

