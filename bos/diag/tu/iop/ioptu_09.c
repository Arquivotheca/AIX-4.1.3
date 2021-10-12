static char sccsid[] = { "@(#)66        1.2  src/bos/diag/tu/iop/ioptu_09.c, tu_iop, bos411, 9431A411a  7/8/94  09:22:01" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_09()
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
/*  TU 09 reads the second byte of power status register and */
/*   returns its value in the return code.                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"

int ioptu_09( fildes ,tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */


        /* read and return power status+1 */

	get_pwr_stat(fildes, POWERSTAT+1, data, tucb_ptr);

        err = data[0] ;
        return(err);
}                 /* end TU 9 */
