static char sccsid[] = "@(#)65	1.1  src/bos/diag/tu/iop/ioptu_20.c, tu_iop, bos41J, 9513A_all 3/9/95 09:00:00";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: ioptu_20
 *
 *   ORIGINS: 83,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *   Copyright (C) Bull S.A. 1995
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  TU 20 reads and checks the cabinet power status tables   */
/*  it is only valid for Pegasus models                      */
/*  the cabinet # is passed to the TU using the r1 field     */
/*  of the TUTYPE header.                                    */
/*  Return value is:                                         */
/*   0  No trouble found                                     */
/*   1  Cabinet is Powered off                               */
/*  -1  Other errors: errno is set                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define PS_TABLE_LEN	18

#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>

#include "tu_type.h"

int ioptu_20(fildes ,tucb_ptr)
int fildes;
TUTYPE *tucb_ptr;
{
	char ps_table[PS_TABLE_LEN];
        int cab_num;
	int rc;
	unsigned long len;	/* length of returned datas from mdd */
	MACH_DD_IO mddrec;

	/* getting cabinet number input parameter */
	cab_num = tucb_ptr->header.r1;

	/* parameters control */
	if (!is_pegasus() || cab_num < 0 || cab_num > 7) {
		errno = EINVAL;
		return(-1);
	}

	/* initializations */
	errno = 0;
	len = 0;

	/* reading cabinet Power Status Table using the */
	/* MPOWERGET machine device driver ioctl call */
	mddrec.md_size = PS_TABLE_LEN;
	mddrec.md_incr = MV_BYTE;
	mddrec.md_data = ps_table;
	mddrec.md_cbnum = cab_num;
	mddrec.md_length = &len;

	rc = ioctl(fildes, MPOWERGET, &mddrec);

	if (rc < 0)	/* errno is set by mdd */
		return(-1);

	/* sanity check of returned datas */
	if(len != PS_TABLE_LEN ||
	   (ps_table[1] & 7) != cab_num) {
		errno = EIO;
		return(-1);
	}

	/* checking power-on status bit */
	if((ps_table[0] & 0x40) != 0x40)
		return(1);

	/* no errors at this point */
        return(0);
}                 /* end TU 20 */
