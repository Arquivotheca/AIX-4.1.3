static char sccsid[] = "@(#)24        1.11.1.3  src/bos/usr/ccs/lib/librts/trcstart.c, librts, bos41B, 412_41B_sync 11/29/94 10:25:58";

/*
 * COMPONENT_NAME: CMDTRACE/LIBRTS   system trace facility
 *
 * FUNCTIONS: trcstart, trcstop, trcon, trcoff
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/trcctl.h>

static int ctlopen(int);		/* prototyping for lint */

extern char *malloc();

int
trcstop(chan)
{
	int fd;
	int rv;

	if((fd = ctlopen(chan)) < 0)
		return(-1);
	rv = ioctl(fd,TRCOFF,0);
	if ((rv = ioctl(fd,TRCSTOP,0)) < 0) {
		close(fd);
		return(-1);
	}
	rv = ioctl(fd,TRCSYNC,0);
	close(fd);
	return(rv);
}

int
trcon(chan)
{
	int fd;
	int rv;

	if((fd = ctlopen(chan)) < 0)
		return(-1);
	rv = ioctl(fd,TRCON,0);
	close(fd);
	return(rv);
}

int
trcoff(chan)
{
	int fd;
	int rv;

	if((fd = ctlopen(chan)) < 0)
		return(-1);
	rv = ioctl(fd,TRCOFF,0);
	close(fd);
	return(rv);
}

static ctlopen(int channel)
{
	char filename[32];
	char ext[32];

	if(channel == 0)
		sprintf(ext,"");
	else
		sprintf(ext,"%d",channel);
	sprintf(filename,"/dev/systrctl%s",ext);
	return(open(filename,0,0));
}

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

int
trcstart(flags)
char *flags;
{
	int pid;
	char buf[128];
	char *cp;
	char *cmd;
	int rv;
	struct {
		short fill;
		char  excode;
		char  sigcode;
	} status;

	if(access("/usr/bin/trace",01) < 0)
		return(-1);
	pid = kfork();
	if(pid < 0)
		return(-1);
	if(pid > 0) {
		for(;;) {
			rv = waitpid(pid,&status,NULL);
			if(rv < 0) {
				if(errno == EINTR)
					continue;
				return(-1);
			}
			if(rv == 0)
				return(-1);
			if(rv == pid) {
				if(status.sigcode)
					return(-1);
				if((int)status.excode < 0)
					return(-1);
				return(status.excode);	/* trace does an exit(channel) */
			}
		}
	}
	/*
	 * look for presence if 'trace' at the start of flags.
	 */
	VCPY(flags,buf);
	if((cp = strtok(buf," \t\n")) == 0)
		cp = "";
	if(strcmp(cp,"trace") != 0 && strcmp(cp,"/usr/bin/trace") != 0
		&& strcmp(cp,"/bin/trace") != 0 ) {
		cmd = malloc(strlen(flags) + 32);
		sprintf(cmd,"%s %s","/usr/bin/trace",flags);
	} else {
		cmd = flags;
	}
	fcntl(0,F_CLOSEM,0);		/* close all the file descriptors */
	open("/dev/null",O_RDWR);  /* redirect all stdin,stdout,stderr to /dev/null */
	dup(0);
	dup(0);
	execl("/usr/bin/sh","sh","-c",cmd,0);
	_exit(255);
}

