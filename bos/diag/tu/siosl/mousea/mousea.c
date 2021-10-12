static char sccsid[] = "@(#)52  1.1.1.4  src/bos/diag/tu/siosl/mousea/mousea.c, tu_siosl, bos411, 9428A410j 12/17/93 10:43:48";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: exectu
 *              mexectu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <errno.h> 
#include <ctype.h> 
#include <time.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "tu_type.h"

#define TRUE 1
#define FALSE 0

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*         								   */
/*       The fd file descriptor is for the mouse device driver             */
/*       The file descriptor for the machine device driver is passed in    */
/*       the TUCB structure.						   */
/*                                                                         */
/***************************************************************************/

#ifdef DIAGNOSTICS
int mexectu(long fd, struct TUCB *tucb_ptr)  /* Compiled for mouse diagnostics */
#else
int exectu(long fd, struct TUCB *tucb_ptr)  /* Compiled for everyone else */
#endif
   {
     short unsigned int i,j;  /* Loop Index */
           int rc = SUCCESS;  /* return code */
              char msg[80];  /* Character buffer for htx message strings */
     static int firsttime = 1;

    /* The first time through, we will enable mouse adapter. */

     if (firsttime)
     {
        rc = setup_mouse(tucb_ptr->mach_fd);
        if (rc != SUCCESS) {
           return(rc);
        }

        firsttime = 0;
     }

   /* Get machine type and mse/kbd/tab polyswitch status check mask */

    rc = get_machine_model_tum(tucb_ptr->mach_fd);

   /* Initialize variables for system register addresses with appropriate
      addresses according to machine type */
   if (rc == SUCCESS) {

       if (mach_info.machine == POWER) {
         comp_reset_reg = COMP_RESET_REG_POWER;
       }
       else {
         comp_reset_reg = COMP_RESET_REG_RSC;
       }

     if (tucb_ptr->header.loop == 0)
	tucb_ptr->header.loop =1;

     for (i=0; (i<tucb_ptr->header.loop) && (rc == SUCCESS); i++)
     {
       switch(tucb_ptr->header.tu)

        {  case   10: PRINT("Mouse adapter internal test - TU 10\n");
	              rc = tu10m(tucb_ptr->mach_fd,tucb_ptr);
                      break;
           case   15: PRINT("Native POS register test - TU 15\n");
	              rc = tu15m(tucb_ptr->mach_fd,tucb_ptr);
                      break;
           case   20: PRINT("Fuse test - TU 20\n");
	              rc = tu20m(tucb_ptr->mach_fd,tucb_ptr);
                      break;
	   case   25: PRINT("Put mouse adapter in Non-BLOCK mode - TU 25\n");
	              rc = tu25m(tucb_ptr->mach_fd,tucb_ptr);
                      break;
           case   30: PRINT("Mouse adapter diag wrap test - TU 30\n");
	              rc = tu30m(tucb_ptr->mach_fd,tucb_ptr);
                      break; 
           default : rc = WRONG_TU_NUMBER;
		     return(rc);
        }  /* end case */

     }  /* i for loop */
  } /* if rc = SUCCESS */
   return(rc);   /* indicate there was no error */
 }  /* End function */

