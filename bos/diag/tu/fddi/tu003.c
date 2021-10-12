static char sccsid[] = "@(#)54  1.4  src/bos/diag/tu/fddi/tu003.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:35";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: On_Line_Diag
 *              tu003
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************

Function(s) Test Unit 003 - On Line Diagnostic Mode.

Module Name :  tu003.c
SCCS ID     :  1.11

Current Date:  5/23/90, 11:17:53
Newest Delta:  1/22/90, 13:26:38

This test unit will place the adapter in the On line Diagnostic Mode. This
is accomplished by writing the hexidecimal value 0x0D00 to the Host Command
Register (HCR).

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif


/*****************************************************************************

On_Line_Diag
   This routine places the adapter in the On Line Diagnostic mode by
   writing the command 0x0D00 to the Host Command Register (HCR).

*****************************************************************************/

int On_Line_Diag(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;
        ushort          hsr;
        unsigned long   status;
        fddi_hcr_cmd_t  diag;
        struct htx_data *htx_sp;
        int i;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Set up On Line Diagnostic command and write this to the HCR
         * via ioctl command.
         */

        diag.hcr_val = ON_LINE_DIAG_MODE;
        diag.l_cpb = 0;

        status = 0;
        rc = ioctl(fdes, FDDI_HCR_CMD, &diag);
        status = diag.status;

        if (rc || diag.status || diag.cpb[FDDI_CPB_SIZE-1])
           {
#ifdef debugg
                detrace(0,"On Line Diag failed.   rc =  %x\n", rc);
                detrace(0,"Returned status = %x\n", status);
                detrace(0,"Returned status in cpb = %x\n",diag.cpb[FDDI_CPB_SIZE-1]);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, DIAG_MODE_ERR));
           }
        else
          if (htx_sp != NULL)
             (htx_sp->good_others)++;

        hsr = diag.hsr_val;
        if (!(hsr & CCI_BIT))
          {
#ifdef debugg
            detrace(0,"Command Completion Interrupt bit not set. HSR = %x\n",
                       hsr);
#endif
            if (htx_sp != NULL)
               (htx_sp->bad_others)++;
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, DIAG_INT_ERR));
          }
#ifdef debugg
        detrace(0,"Adapter in Diagnostic Mode.\n");
#endif

        return(0);
   }


/*****************************************************************************

tu003

*****************************************************************************/

int tu003 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int     rc;

        rc = On_Line_Diag(fdes, tucb_ptr);
        return(rc);
   }
