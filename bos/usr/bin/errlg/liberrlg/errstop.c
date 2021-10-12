static char sccsid[] = "@(#)07	1.1  src/bos/usr/bin/errlg/liberrlg/errstop.c, cmderrlg, bos411, 9428A410j 3/2/93 09:24:02";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: errstop
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


#include <stdio.h>
#include <errno.h>
#include <sys/erec.h>
#include <errlg.h>

/*
 * NAME:     errstop
 * FUNCTION: terminate an errdemon 
 * INPUTS:  NONE
 * RETURNS: NONE
 *
 */

void
errstop()
{
	int fd;
	

	if((fd = open("/dev/errorctl",0)) < 0) {
		perror("open(/dev/errorctl,0)");
		exit(1);
	}

	if(ioctl(fd,ERRIOC_STOP,0) < 0 && errno != ENXIO) {	/* anything but */
		perror("ioctl(fd,/dev/errorctl)");				/* no errdemon */
		exit(1);
	}
	close(fd);
	return;
}

