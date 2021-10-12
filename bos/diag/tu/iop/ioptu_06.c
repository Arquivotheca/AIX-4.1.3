static char sccsid[] = { "@(#)64        1.2  src/bos/diag/tu/iop/ioptu_06.c, tu_iop, bos411, 9431A411a  7/8/94  09:21:35" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_06()
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
/*  TU 06 reads the EC level register and returns its value  */
/*  in the return code.                                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include "sys/mdio.h"

#include "address.h"

int ioptu_06( fildes, tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{

        int err = 0 ;                       /* return code    */
        char data[4] ;                      /* returned data  */

	if (is_pegasus())
		return(0);	/* not implemented on Pegasus */

        /* read EC register   */
        get_iocc(fildes, ECLEVEL, data, MV_BYTE, tucb_ptr);
        err = data[0] ;               /* return ec reg value */

        return(err);
}                 /* end TU 6 */
