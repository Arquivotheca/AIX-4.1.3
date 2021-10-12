static char sccsid[] = "@(#)20	1.8  src/bos/usr/ccs/lib/libIN/DLlock.c, libIN, bos411, 9428A410j 6/10/91 10:15:28";
/*
 * LIBIN: DLlock
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Allocate a device (used in combination with DLfree).
 *
 * PARAMETERS:
 *           name of device
 *
 * RETURN VALUE DESCRIPTION: 
 *           FALSE                 ... device already locked
 *           TRUE                  ... device has been allocated
 */

#include <IN/standard.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <sys/dir.h>
static int ulockf();
static int checkLock();

#define LOCKDIR "/etc/locks/"
#define LOCKLEN (MAXNAMLEN+12)
#define SIZEOFPID 10

DLlock( devname )
 char *devname;
{       char lockname[ LOCKLEN ];
	extern char *CScat(), *CSsname();

	CScat(lockname, LOCKDIR, CSsname( devname ), (char *)NULL );
	if ( ulockf(lockname) == 0 )
		return TRUE;
	else
		return FALSE;
}

/*
 * Create a lock file (file) using the same protocol as BNU UUCP.
 * If one already exists, send a signal 0 to the process--if
 * it fails, then unlink it and make a new one.
 *
 * input:
 *	file - name of the lock file
 *
 * return:
 *	0	-> success
 *	FALSE	-> failure
 */
static int
ulockf(file)
char *file;
{
	int fd;
	static	char pid[SIZEOFPID+2] = { '\0' }; /* +2 for '\n' and NULL */


	if (pid[0] == '\0') 
		(void) sprintf(pid, "%*d\n", SIZEOFPID, getpid());

	fd=open(file,O_RDWR | O_CREAT | O_EXCL,0444);
	if(fd < 0) {
		if (errno != EEXIST)
			return(-1);
		if (checkLock(file) == -1)
			return(-1);
		else 
			fd=creat(file,(mode_t)0444);
		    }
	(void) write(fd, pid, (unsigned)(SIZEOFPID+1));	/* +1 for '\n' */
	(void) close(fd);
	return(0);
}

/*
 * check to see if the lock file exists and is still active
 * - use kill(pid,0) - (this only works on ATTSV and some hacked
 * BSD systems at this time)
 * return:
 *	0	-> success (lock file removed - no longer active
 *	FALSE	-> lock file still active
 */
static int
checkLock(file)
register char *file;
{
	register int ret;
	int lpid = -1;
	char alpid[SIZEOFPID+2];	/* +2 for '\n' and NULL */
	int fd;

	fd = open(file, O_RDONLY);
	if (fd == -1) {
	    if (errno == ENOENT)  /* file does not exist -- OK */
		return(0);
	    goto unlk;
	}
	ret = read(fd, (char *) alpid, (unsigned)(SIZEOFPID+1)); /* +1 for '\n' */
	(void) close(fd);
	if (ret != (SIZEOFPID+1)) 
	    goto unlk;
	lpid = atoi(alpid);
	if ((ret=kill(lpid, 0)) == 0 || errno == EPERM) 
	    return(-1);
unlk:
	if (unlink(file) != 0) {
		return(-1);
	}
	return(0);
}
