static char sccsid[] = "@(#)02	1.1  src/bos/diag/boot/eject.c, diagboot, bos41B, bai4 1/18/95 09:46:06";
/*
 * COMPONENT_NAME: DIAGBOOT
 *
 * FUNCTIONS:    Eject the cdrom disc from the drive
 *
 * RETURN CODE: 1 Not successfull
 * 		0 Successfull
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
 */
#include <stdio.h>
#include <sys/scdisk.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <locale.h>

main (argc, argv, envp)
int argc;
char **argv;
char **envp;
{
        int     fdes;
	char	devname[64];
	int	rc;


	setlocale(LC_ALL, "");
	if(argc < 1)
		exit(0);

	strcpy(devname, argv[1]);
	if((fdes=open(devname,0))<0)
                exit(1);

	rc=ioctl(fdes, DKEJECT, NULL );
	close(fdes);
	exit(rc != 0 ? 1 : 0);
}

