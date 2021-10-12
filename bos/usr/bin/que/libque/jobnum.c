static char sccsid[] = "@(#)64	1.17  src/bos/usr/bin/que/libque/jobnum.c, cmdque, bos411, 9428A410j 1/29/93 12:14:14";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: plusone, getjobnum, incjobnum
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	This module contains routines that deal with the job numbering
	mechanism and the job number file
*/
#include <stdio.h>
#include <IN/standard.h>
#include <sys/param.h>
#include <IN/backend.h>
#include <sys/utsname.h>
#include "common.h"
#include <errno.h>
#include "enq.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/id.h>
#include <security.h>
#include <ctype.h>

#include "libq_msg.h"


#define NUMFILE "/var/spool/lpd/stat/numfile"
#define NUMLEN 10

extern int qdaemon;	/*set in qmain or qstatus.c enq.c. Global Control 
				Flag. sorry.*/
long plusone(j)
long j;
{
	j++;
	if (j>MAXORDER) j=1;
	if (JOBNUM(j) == 0) j++;
	return(j);
}

long getjobnum(jfile)
int jfile;
{
	int numread;
	long jobnum=0;
	char buf[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

	if (lseek(jfile,(off_t)SEEK_SET,0)<0)
		syserr((int)EXITFATAL,GETMESG(MSGSEEK,"Lseek failure in %s."),NUMFILE);
	if (NUMLEN != (numread=read(jfile,buf,(unsigned)NUMLEN)))
		syserr((int)EXITFATAL,GETMESG(MSGREAD,"Read failure in %s."),NUMFILE);
	sscanf(buf,"%d",&jobnum);
#ifdef DEBUG
	if (getenv("GJOBNUM"))
		sysraw("read %d from %s\n",jobnum,NUMFILE);
#endif

	return(jobnum);
}

/* 	Open the jobnum file and increment it by one. 
	Create it if it is nonexistent and place 001 in it. 
	Return the number string to the caller.

if not yet open
	open as old
	if doesn't exist
		create it
		jobnum=0
	else
		read jobnum
else
	read jobnum
add 1 to jobnum and modulo maxjob
rewind
write jobnum to file
return newjobnum as a string
*/

incjobnum(rval)
char            *rval;
{
	static int 	jfile;
	long 		jobnum;
	gid_t	effgid;		/* to use for chown() */
	struct flock flock_struct;

	/* settings to lock the file with */
	flock_struct.l_type = F_WRLCK;
	flock_struct.l_whence = 0;
	flock_struct.l_start = 0;
	flock_struct.l_len = 0;

	if ((jfile=open(NUMFILE,O_RDWR))<0)
		if (errno==ENOENT)	/*file not found*/
		{
			errno=0;
			if ((jfile=open(NUMFILE,O_RDWR|O_CREAT,0660))<0)
				syserr((int)EXITFATAL,GETMESG(MSGCREA,"Failure to create %s."),NUMFILE);
			else
			{
				if (fcntl(jfile, F_SETLKW, &flock_struct) == -1)
					syserr((int)EXITFILE,GETMESG(MSGRERR,"Read error %d on %s."),errno,NUMFILE); 
				jobnum=0;
			}
		}
		else	/* couldn't open for some other reason*/
			syserr((int)EXITFATAL,GETMESG(MSGOPEN,"Failure to open %s."),NUMFILE);
	else	/* file already exists and is now open*/
	{
		if (fcntl(jfile, F_SETLKW, &flock_struct) == -1)
			syserr((int)EXITFILE,GETMESG(MSGRERR,"Read error %d on %s."),errno,NUMFILE); 

		jobnum=getjobnum(jfile);
	}

	/* get the effective gid to chown() */
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
	{
		syserr((int)EXITFATAL,GETMESG(MSGEGID,"Cannot get effective gid. Errno = %d"),errno);
	}
		
	/* set ownership to administrative user (root) and
	 * effective gid (printq)
	 */
	if (fchown(jfile, OWNER_ADMDATA, effgid) == -1)
	{
		syserr((int)EXITFATAL,GETMESG(MSGCHWN,"Cannot chown %s. Errno = %d"),NUMFILE,errno);

	}

	/* make sure permission is set to 660 always 	*/
	if (acl_fset(jfile,R_ACC|W_ACC,R_ACC|W_ACC,NO_ACC) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGPERM,"Cannot set permissions on %s. Errno = %d"),NUMFILE,errno);

	jobnum= plusone(jobnum);
	sprintf(rval,"%09d\n",jobnum);

	if (lseek(jfile,(off_t)SEEK_SET,0)<0)
		syserr((int)EXITFATAL,GETMESG(MSGSEEK,"Lseek failure in %s."),NUMFILE);

	write(jfile,rval,(unsigned)NUMLEN);

	close(jfile);

	return(OK);
}


