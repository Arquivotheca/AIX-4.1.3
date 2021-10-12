static char sccsid[] = "@(#)18	1.1  src/bos/usr/lpp/kls/dictutil/hugetcmp.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:55";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hugetcmp.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hugetcmp.c
 *
 *  Description:  get cursor move value.
 *
 *  Functions:    hugetcmp()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include "hut.h"        /* Dictionary Utility Define File.   */

int hugetcmp(getcode)
int     getcode;                /* check character code         */
{
	int   curmp = 1;


	if (getcode >= 0xa1 && getcode <= 0xfe)
	{
	/* getcode is DBCS    */
		curmp = 2;
	};

  	return( curmp );
};
