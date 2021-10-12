static char sccsid[] = { "@(#)63        1.1  src/bos/diag/tu/iop/ioptu_05.c, tu_iop, bos411, 9428A410j  12/15/93  13:45:18" };
/*
 * FUNCTIONS: ioptu_05()
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  TU 05 restores the led values if saved in TU04.        */
/*  TU04 must have been executed before this TU or rc = 20 */
/*  will be returned.                                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"


int ioptu_05( fildes, first_time, save_area, tucb_ptr)
int fildes;
int *first_time;
char *save_area;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */

        if (*first_time){
                err = 20 ;
                return (err) ;
        } /* endif */

        /* put orig value back */
        data[0] = *save_area;
        data[1] = *(save_area+1);
        set_nvram(fildes, LED, data, MV_BYTE, TWO, tucb_ptr);

        *first_time=1;          /* reset to new */
        return (err);                 /* end TU 5 */
}   /* ioptu_05 */
