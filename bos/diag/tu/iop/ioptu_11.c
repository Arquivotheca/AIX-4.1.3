static char sccsid[] = "@(#)68	1.3  src/bos/diag/tu/iop/ioptu_11.c, tu_iop, bos411, 9428A410j 4/14/94 10:37:01";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_11()
 *
 *   ORIGINS: 27, 83
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  TU 11 tests for battery low signal from TOD.           */
/*  If low, sends error = 12 to diag app.                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"
#include "ioptu.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern uchar tod_reg_flags[];

int ioptu_11( fildes, tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */


/* Save off the registers used by this test unit.              */
/*      The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */

/*      This test unit saves:                                  */
/*              MAIN     4000C0                                */
/*              IRQ1     4000C4      REG_SELECT = 1            */

        clear_tod_flags();              /* Clear TOD flags */
        tod_reg_flags[F_MAIN] = 1;      /* Set TOD flags */
        tod_reg_flags[F_IRQ1] = 1;
        tod_reg_flags[F_IRQROUTE] = 1;
        save_tod_regs(fildes, tucb_ptr);       /* Save TOD regs. */

        /* End of save reg code.   Registers are restored
           after the switch statement in exectu().          */

        data[0] = PG0REG1;         /* MAIN point at reg bank 1  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* enable power fail int by setting bit D7=1 */
        data[0] = 0x80 ;           /* enable power fail int. */
        set_tod(fildes, IRQ1, data, MV_BYTE, tucb_ptr);

        data[0] = PG0REG0;           /* MAIN point at reg bank 0  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* get the contents of the Interrupt Routing Register */
        get_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        if (( data[0] & 0x40 ) == 0 ){
        } else {
                err = BATTERY_LOW ;
                return(err) ;
        } /* endif */

        return(err);

}  /* ioptu_11.c */
