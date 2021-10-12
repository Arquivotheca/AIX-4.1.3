static char sccsid[] = "@(#)34	1.1  src/bos/diag/tu/sol/tu003.c, tu_sol, bos411, 9428A410j 4/23/91 11:43:39";
/*
 * COMPONENT_NAME:  SOL Test Unit
 *
 * FUNCTIONS: tu_003   (Optics SERDES wrap test (Activate, SCR, OLS, RHR, CRC))
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include <stdio.h>
#include "soltst.h"

int
tu_003(fdes, tucb_ptr)
int fdes;
TUTYPE *tucb_ptr;
{

  int                     result;
  struct sol_diag_test    diag_struct;

  /* check for card presence    */

  result = ioctl(fdes,SOL_CARD_PRESENT,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == ENOCONNECT) {
                return(SOL_NO_OPTICAL_CARD_PLUGGED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

  bzero(&diag_struct,sizeof(diag_struct));


  diag_struct.diag_mode = SOL_1_BIT_WRAP;

  /* perform lock_to_xtal                       */


  result = ioctl(fdes,SOL_LOCK_TO_XTAL,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                return(SOL_LOCK_TO_XTAL_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

/******************  SOL_ACTIVATE  **********************/


  result = ioctl(fdes,SOL_ACTIVATE,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                tucb_ptr -> diag.sla_error = diag_struct.result;
                return(SOL_ACTIVATE_MODE_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

/******************  SOL_SCR **********************/

  result = ioctl(fdes,SOL_SCR,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                tucb_ptr -> diag.sla_error = diag_struct.result;
                return(SOL_SCR_MODE_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */


/******************  SOL_OLS **********************/

  result = ioctl(fdes,SOL_OLS,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                tucb_ptr -> diag.sla_error = diag_struct.result;
                return(SOL_OLS_MODE_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

/******************  SOL_RHR **********************/

  result = ioctl(fdes,SOL_RHR,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                tucb_ptr -> diag.sla_error = diag_struct.result;
                return(SOL_RHR_TEST_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

/******************  SOL_CRC **********************/

  result = ioctl(fdes,SOL_RHR,&diag_struct);
  if (result != 0) {                      /* if error */
          if (errno == EIO) {
                tucb_ptr -> diag.sla_error = diag_struct.result;
                return(SOL_CRC_TEST_FAILED);
          } else {
               tucb_ptr -> diag.sla_error = errno;
               return(SOL_PROGRAMMING_ERROR);
          } /* endif */

  }  /* endif */

  return(SOL_SUCCESS);

}
