static char sccsid[] = "@(#)54 1.1 src/bos/usr/lpp/bosinst/cdeject/cdeject.c, bosinst, bos41J, 9514A_all 95/04/05 09:10:40";
/*
 * COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
 *
 * FUNCTIONS: bosinst
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Function: This function issues an ioctl to the cdrom device driver of the
 * cdrom device specified as an arguement to eject the cd from the drive.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/scdisk.h>

int main(int argc, char **argv)
{
	int fd;
	int rc;

	if (argc < 2) {
		return (EINVAL);
	}


	if ((fd = open(argv[1],O_RDONLY)) < 0 ) {
		return(errno);
	}

	rc = ioctl(fd,DKEJECT,NULL);

	close(fd);
	return (rc);
}
