static char sccsid[] = { "@(#)59        1.1  src/bos/diag/tu/iop/ioptu_01.c, tu_iop, bos411, 9428A410j  12/15/93  13:40:27" };
/*
 * FUNCTIONS: ioptu_01()
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* * * * * * * *  * * * * * * * * * * * * * * * * * * * * *  */
/*  TU 01 first reads and saves the values in NVRAM addr 300 */
/*   and 301. It then writes values to NVRAM 300 thru 303,   */
/*   reads them back and  tests them for correct values.     */
/*  At the end of the test, it restores the original values. */
/*  If a miscompare occurs, the values in NVRAM ( and in the */
/*    leds) is unknown....                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"

int ioptu_01( fildes, tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */
        static char save[16] ;              /* save data area               */

        /* Save the original contents of the leds */
        get_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);
        save[0] = data[0];
        save[1] = data[1];

        /* Write 123 to leds */
        data[0] = data[2] = 0x12 ;
        data[1] = data[3] = 0x34 ;
        set_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);

        /* Clear out variable and read what was put out on leds */
        data[0] = data[1] = 0;
        get_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);
        if(data[0] != data[2] || data[1] != data[3]) {
                err=2;
                return(err);
        } /* end if */

        /* write 5678 to locations LED+2 */
        data[0] =data[2] = 0x56;
        data[1] = data[3] = 0x78;
        set_nvram(fildes, LED+2, data, MV_BYTE, TWO, tucb_ptr);

        /* Clear out variable and read what was put out on leds */
        data[0] = data[1] = 0;
        get_nvram(fildes, LED+2, data, MV_BYTE, TWO, tucb_ptr);
        if(data[0] != data[2] || data[1] != data[3]) {
                err=2;
                return(err);
        } /* end if */

        /* put original led values back */
        data[0] = save[0];
        data[1] = save[1];
        set_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);

        return(err);

}

