static char sccsid[] = "@(#)61	1.2  src/bos/diag/tu/iop/ioptu_02.c, tu_iop, bos411, 9428A410j 4/14/94 10:36:13";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_02()
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

/* * * * * * * * * * * * * * * * * * * * * ** * * * * * * *  */
/*  TU 02 first reads and saves the values in TOD locations  */
/*    00DE & 00DF.                                           */
/*  It then writes values to those locations,  reads them    */
/*  back and tests them for correct values.                  */
/*  It then restores original values.                        */
/*  If miscompare occurs, values in TOD RAM may be different */
/*  from original .                                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *   */


#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"
#include "tu_type.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];

int ioptu_02( fildes, tucb_ptr)
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
        /*              RAMREG   4000DE                                */
        /*              RAMTEAT  4000DF                                */

        clear_tod_flags();              /* Clear TOD flags */
        tod_reg_flags[F_MAIN] = 1;      /* Set TOD flags */
        tod_reg_flags[F_RAMREG] = 1;
        tod_reg_flags[F_RAMTEAT] = 1;
        save_tod_regs(fildes, tucb_ptr);       /* Save TOD regs. */
        /* End of save reg code.  Will be restored in calling program. */

        /* write 55aa to TOD ram */
        data[0] = data[2] = 0x55 ;
        data[1] = data[3] = 0xaa ;
        set_tod(fildes, RAMREG, data, MV_BYTE, tucb_ptr);
        set_tod(fildes, RAMTEAT, data+1, MV_BYTE, tucb_ptr);

        /* clear out the data */
        data[0] = data[1] = 0;

        /* read back and compare */
        get_tod(fildes, RAMREG, data, MV_BYTE, tucb_ptr);
        get_tod(fildes, RAMTEAT, data+1, MV_BYTE, tucb_ptr);
        if ( data[0] != data[2] || data[1] != data[3] ){
                err = 3 ;
                return(err) ;
        } /* endif */

        /* write bbdd TOD ram */
        data[0] = data[2] = 0xbb ;
        data[1] = data[3] = 0xdd ;
        set_tod(fildes, RAMREG, data, MV_BYTE, tucb_ptr);
        set_tod(fildes, RAMTEAT, data+1, MV_BYTE, tucb_ptr);

        /* clear out data */
        data[0] = data[1] = 0;

        /* read back and compare */
        get_tod(fildes, RAMREG, data, MV_BYTE, tucb_ptr);
        get_tod(fildes, RAMTEAT, data+1, MV_BYTE, tucb_ptr);
        if ( data[0] != data[2] || data[1] != data[3] ){
                err = 3 ;
                return(err) ;
        } /* endif */
        return(err);
}   /* ioptu_02() */
