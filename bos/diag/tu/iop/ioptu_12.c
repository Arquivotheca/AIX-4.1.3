static char sccsid[] = "@(#)69	1.2  src/bos/diag/tu/iop/ioptu_12.c, tu_iop, bos411, 9428A410j 4/14/94 10:37:23";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_12()
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
/*  TU 12 tests for TOD at POR state. If in that state, send */
/*  error = TOD_AT_POR_STATE to diag app.                    */
/*  Then test for TOD running. If not, send                  */
/*  error = TOD_NOT_RUNNING to diag app.                     */
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

int ioptu_12( fildes, tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */
        static char save[16] ;              /* save data area               */

        /* Save off the registers used by this test unit.              */
        /*      The registers are referenced by indexes to a global    */
        /*      array called tod_reg_flags[].  The actual registers    */
        /*      are saved in an array called tod_reg_array[].  But     */
        /*      this array should only be accessed via save_tod_regs() */
        /*      and restore_tod_regs().                                */

        /*      This test unit saves:                                  */
        /*              MAIN     4000C0                                */
        /*              PERFLAG  4000C3       REG_SELECT = 0           */
        /*              REALTIME 4000C1       REG_SELECT = 1           */

        clear_tod_flags();              /* Clear TOD flags */
        tod_reg_flags[F_MAIN] = 1;      /* Set TOD flags */
        tod_reg_flags[F_PERFLAG] = 1;
        tod_reg_flags[F_REALTIME] = 1;
        tod_reg_flags[F_IRQROUTE] = 1;
        save_tod_regs(fildes, tucb_ptr);       /* Save TOD regs. */

        /* End of save reg code.   Registers are restored
           after the switch statement in exectu().          */

        data[0] = PG0REG0;       /* main point at reg bank 0  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* read TOD periodic flag reg */
        get_tod(fildes, PERFLAG, data, MV_BYTE, tucb_ptr);

        if ((data[0] & 0x40) == 0){
        } else {
                err = TOD_AT_POR_STATE ;
                return(err);
        } /* endif */

        data[0] = PG0REG1;         /* MAIN point at reg bank 1  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* read TOD real time mode reg */
        get_tod(fildes, REALTIME, data, MV_BYTE, tucb_ptr);

        if ((data[0] & 0x08) == 0){
                err = TOD_AT_POR_STATE ;
                return(err);
        } /* endif */

        data[0] = PG0REG0;         /* MAIN point at reg bank 0  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* write bb to the Interrupt Routing Register */
        data[0] = 0xbb ;           /* data to write  */
        set_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        /* force real time clock save  */

        /* write 3b to the TOD int routing reg */
        data[0] = 0x3b ;
        set_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        /*  read TOD and save */
        get_tod(fildes, SECSAVE, data, MV_BYTE, tucb_ptr);
        save[2] = data[0];
        get_tod(fildes, MINSAVE, data, MV_BYTE, tucb_ptr);
        save[3] = data[0];

        sleep(1);                  /* delay */

        data[0] = PG0REG0;         /* MAIN point at reg bank 0 */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        /* write bb to Interrupt Routing Register */
        data[0] = 0xbb ;           /* data to write  */
        set_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        /* force real time clock save  */
        /* write 3b to Interrupt Routing Register */
        data[0] = 0x3b ;
        set_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        /* read TOD and save */
        get_tod(fildes, SECSAVE, data, MV_BYTE, tucb_ptr);
        get_tod(fildes, MINSAVE, data+1, MV_BYTE, tucb_ptr);

        if ( data[0] == save[2] && data[1] == save[3] ){
                err = TOD_NOT_RUNNING ;
                return(err);
        } /* endif */

        return(err);
} /* ioptu_12 *.c */
