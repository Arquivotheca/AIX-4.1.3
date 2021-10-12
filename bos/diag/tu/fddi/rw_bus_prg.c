static char sccsid[] = "@(#)45  1.3  src/bos/diag/tu/fddi/rw_bus_prg.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:12";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: rw_bus_prg
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

Function(s) rw_bus_prg - Read the NP Bus Program Store area of memory

Module Name :  rw_bus_prg.c
SCCS ID     :  1.20

Current Date:  5/23/90, 11:17:55
Newest Delta:  3/26/90, 16:25:54


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

rw_bus_prg

*****************************************************************************/
int rw_bus_prg (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;
        int             k;
        unsigned char   buf0[BUFF64K_SIZE];
        unsigned long   status;
        fddi_mem_acc_t  turam;

        /*
         * Read all the NP Bus Program Store area and preseve it in buf0.
         */

        turam.opcode = FDDI_RD_MEM_NP_BUS_PROGRAM;
        turam.ram_offset = 0;
        turam.num_transfer = 1;
        turam.buffer_1 = buf0;
        turam.buff_len1 = BUFF64K_SIZE;

        status = 0;
        rc = ioctl(fdes, FDDI_MEM_ACC, &turam);
        status = turam.status;

        if (rc)
           {
#ifdef debugg
                detrace(0, "Read of NP Bus Program Store failed. ");
                detrace(0, "rc = %x\n", rc);
#endif
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                         BUSPRG2_RD_ERR));
           }

#ifdef debugg
        detrace(0,"Read of NP Bus Program Store area.\n");
        detrace(0,"buf0:\n");
        for (k = 0; k < SHOW_BYTES; k++)
           if ((buf0[k] < 0x20) || (buf0[k] > 0x7E))
              detrace(0," %02x", buf0[k]);
           else
              detrace(0," %c", buf0[k]);
        detrace(0,"\n");
#endif

        return(rc);
    }
