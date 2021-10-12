static char sccsid[] = "@(#)66	1.1  src/bos/usr/bin/errlg/errstop/main.c, cmderrlg, bos411, 9428A410j 3/3/93 08:39:49";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main, errstop
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Command line interface for errstop.
 *
 * NAME:     main
 * FUNCTION: Issue ERRIOC_STOP ioctl to terminate the errdemon.
 *
 * INPUTS:   NONE
 * RETURNS:  NONE
 */

#include <errno.h>
#include <sys/erec.h>


main()
{
	errstop();		/* stop the errdemon via ioctl */
	exit(0);
}

