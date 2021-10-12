static char sccsid[] = "@(#)70	1.2  src/bos/diag/tu/iop/ioptu_13.c, tu_iop, bos411, 9428A410j 4/14/94 10:37:50";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_13()
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
/*  TU 13 sets up TOD for Jan 1, 1989 8 AM . This TU may be  */
/*  run if TU 12 sent back a return code of TOD_AT_POR_STATE.*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"

int ioptu_13( fildes, tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        long i;
        static char timedata[10] = {
                0,0,0,8,1,1,89,1,0,1
        };                                  /* TOD data                     */
        char data[4] ;                      /* returned data  */

        data[0] = 0x7f ;  /* MAIN page 0 reg bank 1 clear ints  */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        data[0] = 0x10 ;           /* int pf operation */
        /* write TOD real time mode reg */
        set_tod(fildes, REALTIME, data, MV_BYTE, tucb_ptr);

        for ( i = 1 ; i < 4 ; i++ ){

                data[0] = 0x00 ;           /* clear data */
                /* write TOD bank 1 regs */
                set_tod(fildes, REALTIME+i, data, MV_BYTE, tucb_ptr);

        } /* endfor */

        for ( i = 0 ; i < 4 ; i++ ){

                data[0] = 0x00 ;           /* clear data */
                /* write TOD bank 0 regs */
                set_tod(fildes, MAIN+i, data, MV_BYTE, tucb_ptr);

        } /* endfor */

        /* route all ints to mfo ex alarm */

        data[0] = 0x3b ;
        /* write TOD int routing reg */
        set_tod(fildes, IRQROUTE, data, MV_BYTE, tucb_ptr);

        for ( i = 0 ; i < 17 ; i++ ){

                data[0] = 0x00 ;         /* clear data        */
                /* write TOD page 0 ram */
                set_tod(fildes, TIMER0L+i, data, MV_BYTE, tucb_ptr);

        } /* endfor */

        data[0] = PG1REG0;           /* MAIN points to page 1 reg0   */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        for ( i = 1 ; i < 0x20 ; i++ ){
                data[0] = 0 ;            /* clear data       */
                /* write TOD page 1 ram */
                set_tod(fildes, MAIN+i, data, MV_BYTE, tucb_ptr);
        } /* endfor */

        data[0] = PG0REG1 ;           /* page 0 , bank 1  */
        /* write TOD main status */
        set_tod(fildes, MAIN, data, MV_BYTE, tucb_ptr);

        for ( i = 0 ; i < 10 ; i++ ){

                data[0] = timedata[i] ;   /* real time data */
                /* write TOD page 1 ram */
                set_tod(fildes, CSECCOUNT+i, data, MV_BYTE, tucb_ptr);

        } /* endfor */

        /* start tod, int en, 24 hour mode, leap 1 yr ago  */

        data[0] = 0x19 ;

        /* write TOD real time mode reg */
        set_tod(fildes, REALTIME, data, MV_BYTE, tucb_ptr);

        return(err);

} /* ioptu_13.c */
