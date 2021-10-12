static char sccsid[] = "@(#)51	1.1  src/bos/usr/bin/errlg/rd_chkstop/main.c, cmderrlg, bos411, 9428A410j 4/4/94 16:58:07";
/*
 *   COMPONENT_NAME: cmderrlg
 *
 *   FUNCTIONS: checkstop
 *		main
 *		nvclose
 *		nvopen
 *		nvread
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <sys/mdio.h>


#define CHKSIZE      (9 * 1024)
#define OFF_CHKPTR   0x030C

static int Nvfd = -1;
char Chkfile[20] = "checkstop.logout";

/*
 * NAME:     main
 */

main(argc,argv)
char *argv[];
{
	checkstop();
	exit(0);
}

/*
 * NAME:     checkstop
 * FUNCTION: read out the checkstop info from nvram and copy to
 *           a local file, checkstop.logout.
 * INPUTS:   none  (read from /dev/nvram)
 * RETURNS:  none
 */

static
checkstop()
{
	char checkstop_count;
	int  checkstop_ptr;
	char checkstop_data[CHKSIZE];
	int  fd;

	if(nvopen() < 0)  {
		printf("Unable to open NVRAM to obtain checkstop logout data.\n");
	
	}
	else if(nvread(OFF_CHKPTR,&checkstop_ptr,4) < 0)  {	/* read check. location */
		printf("Unable to read address of checkstop logout data from NVRAM.\n");
		if(nvclose() < 0)
			printf ("Unable to close NVRAM.\n");
	}
	else if(nvread(checkstop_ptr,checkstop_data,CHKSIZE) < 0) {	/* read data */
		printf("Unable to read checkstop logout data from NVRAM.\n");
		if(nvclose() < 0)
			printf ("Unable to close NVRAM.\n");
	}
	
	else {
		nvclose();
		if((fd = open(Chkfile,O_WRONLY|O_TRUNC|O_CREAT,0644)) < 0) {
			perror(Chkfile);
		}
		else if(write(fd,checkstop_data,CHKSIZE) != CHKSIZE) {	/* write data */
			perror(Chkfile);
			close(fd);
		}
		else 
			close(fd);
	}
}


/*
 * NAME:     nvopen
 * FUNCTION: Open /dev/nvram.
 * INPUTS:   none 
 * RETURNS:  1 - success
 *           -1 - failure
 */

static int
nvopen()
{

	if(Nvfd >= 0)
		return(1);
	if((Nvfd = open("/dev/nvram",O_RDWR)) < 0) { 	/* open /dev/nvram */
		perror("/dev/nvram");	
		return(-1);
	}
	return(1);
}

/*
 * NAME:     nvclose
 * FUNCTION: Close /dev/nvram.
 * INPUTS:   none 
 * RETURNS:   1 - success
 *			 -1 - failure
 */

static int
nvclose()
{
	int	rc=1;

	if(Nvfd >= 0 && close(Nvfd) < 0) {
		perror("/dev/nvram");
		rc = -1;
	}
	Nvfd = -1;
	return(rc);
}

/*
 * NAME:     nvread
 * FUNCTION: read from /dev/nvram.
 * INPUTS:   buf, count, offset 
 * RETURNS:  -1 - failure
 *            0 - success
 */

static
nvread(offset,buffer,count)
{
	MACH_DD_IO mddrec;

	if(Nvfd < 0)
		return(-1);
	mddrec.md_addr = offset;
	mddrec.md_data = (char *) buffer;
	mddrec.md_size = count;
	mddrec.md_incr = MV_BYTE;
	if(ioctl(Nvfd,MIONVGET,&mddrec) < 0) {
		perror("MIONVGET");
		return(-1);
	}
	return(0);
}


