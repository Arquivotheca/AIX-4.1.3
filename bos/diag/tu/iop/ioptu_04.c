static char sccsid[] = { "@(#)62        1.1  src/bos/diag/tu/iop/ioptu_04.c, tu_iop, bos411, 9428A410j  12/15/93  13:45:14" };
/*
 * FUNCTIONS: ioptu_04()
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
        /*  The first time here, TU 04 reads and saves values in     */
        /*  NVRAM 300-301                                            */
        /*  It then writes 666 to leds and returns.                  */
        /*  Subsequent times here it writes 999 to leds and returns. */
        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"
#include "address.h"

int ioptu_04( fildes, first_time, save_area, tucb_ptr)
int fildes;
int *first_time;
char *save_area;
TUTYPE *tucb_ptr;
{
        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */

        /* If first time thru, read and save values in  */
        /* NVRAM 300-301 */
        if (*first_time){
                /* read and save leds */
                get_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);
                *(save_area) = data[0];
                *(save_area+1) = data[1];

                /* write 666 to leds */
                data[0] = data[2] = 0x66 ;
                data[1] = data[3] = 0x66 ;
                set_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);
        } /* endif */

        if (! *first_time){
                /* write 999 to leds */
                data[0] = data[2] = 0x99 ;
                data[1] = data[3] = 0x99 ;
                set_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);
        } /* endif */

        /* Set first_time to false */
        if (*first_time) {
                *first_time=0;
        }  /* endif */

        return(err);
}       /* ioptu_04() */
