static char sccsid[] = "@(#)57	1.4  src/bos/usr/ccs/bin/gprof/hertz.c, cmdstat, bos411, 9428A410j 4/25/91 17:47:40";
/*
* COMPONENT_NAME: (CMDSTAT) hertz
*
* FUNCTIONS: hertz
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Copyright (c) 1980 Regents of the University of California.
* All rights reserved.  The Berkeley software License Agreement
* specifies the terms and conditions for redistribution.
*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *  
 *  #ifndef lint
 *  static char sccsid[] = "hertz.c	5.1 (Berkeley) 6/4/85";
 *  #endif not lint
 */


#include <sys/time.h>
#include <sys/m_param.h>

    /*
     *	discover the tick frequency of the machine
     *	if something goes wrong, we return 0, an impossible hertz.
     */
#define	HZ_WRONG	0

hertz()
{
	struct itimerval tim;
	return (HZ);

/* This value is returning 1000 instead of 100, the
 * value in HZ is fine for this anyways...

	tim.it_interval.tv_sec = 0;
	tim.it_interval.tv_usec = 1;
	tim.it_value.tv_sec = 0;
	tim.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tim, 0);
	setitimer(ITIMER_REAL, 0, &tim);
	if (tim.it_interval.tv_usec < 2)
		return(HZ_WRONG);
	return (1000000 / tim.it_interval.tv_usec);
*/
}
