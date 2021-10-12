static char sccsid[] = "@(#)58	1.9  src/bos/usr/ccs/lib/libc/ttylock.c, libctty, bos411, 9428A410j 11/8/93 13:53:24";

/*
 * COMPONENT_NAME: (libctty) terminal control routines
 *
 * FUNCTIONS:  ttylock, tlockf, ttylocked, putlock, mklock, ttyunlock,
 *	clrlock, ttytouchlock, ttywait
 *
 * ORIGINS: 3, 27
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

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *  Routines modified from Basic Networking Utilities (BNU) 1.0
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

#define ASCIILOCKS
#define	SIZEOFPID	10		/* maximum number of digits in a pid */

#define MAXLOCKS 10	/* maximum number of lock files */
char *lckarray[MAXLOCKS];
int numlocks = 0;

#define SAME 0
#define EQUALS(a,b)	((a) && (b) && (strcmp((a),(b))==SAME))

int ttylockDebug;
static mklock();
static tlockf();

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(l, f, s) if (ttylockDebug >= l) fprintf(stderr, f, s)

#define DEVICE_LOCKPRE		"/etc/locks/"
#define LOCKPRE			"/etc/locks/LCK."
#define LTMP			tmp

#define MAXNAMESIZE	80
#define MAXBASENAME 	22 
char *Bnptr;            /* used when BASENAME macro is expanded */
#define BASENAME(str, c) ((Bnptr = strrchr((str), c)) ? (Bnptr + 1) : (str))

static void unmux(char *s, char *t);

/*
 * lock device.
 *
 * return:
 *	0	-> success
 *	-1	-> failure
 */
ttylock(device)
char *device;
{
	char lname[MAXNAMESIZE];
	char basedevice[MAXBASENAME];

	unmux(device, basedevice);
	(void) sprintf(lname, "%s.%s", LOCKPRE, basedevice);
	BASENAME(lname, '/')[MAXBASENAME] = '\0';
	return(tlockf(basedevice, lname) < 0 ? -1 : 0);
}

/*
 *
 * lock filename.
 *
 * Return:-
 *	 0 on successful completion. 
 * 	-1 if "device" is locked by another process.
 *
 */
static
tlockf(device, file)
register char *device, *file;
{
#ifdef	ASCIILOCKS
	static	char pid[SIZEOFPID+2] = { '\0' }; /* +2 for '\n' and NULL */
#else
	static int pid = -1;
#endif

	static char tempfile[MAXNAMESIZE];

#ifdef	ASCIILOCKS
	if (pid[0] == '\0') {
		(void) sprintf(pid, "%*d\n", SIZEOFPID, getpid());
#else
	if (pid < 0) {
		pid = getpid();
#endif
		(void) sprintf(tempfile, "%s/LTMP.%d", DEVICE_LOCKPRE, getpid());
	}

	if (mklock(pid, tempfile, file) == -1) {
		(void) unlink(tempfile);
		if (ttylocked(device))
			return(-1);
		else {
		    if (mklock(pid, tempfile, file)) {
			(void) unlink(tempfile);
			DEBUG(4,"ttylock failed in mklock()\n","");
			return(-1);
		    }
		}
	}

	putlock(file);
	return(0);
}

/*
 * check to see if the lock file exists and is still active
 *
 * return:
 *	0	-> success (lock file removed - no longer active)
 *	-1	-> lock file still active
 */
ttylocked(device)
register char *device;
{
	register int ret;
	int lpid = -1;
#ifdef	ASCIILOCKS
	char alpid[SIZEOFPID+2];	/* +2 for '\n' and NULL */
#endif
	int fd;
	extern int errno;
	char file[MAXNAMESIZE];
	char basedevice[MAXBASENAME];

	unmux(device, basedevice);
	sprintf (file, "%s.%s", LOCKPRE, device);
	fd = open(file, O_RDONLY);
	DEBUG(4, "ttylock file %s\n", file);
	if (fd == -1) {
	    if (errno == ENOENT)  /* file does not exist -- OK */
		return(0);
	    DEBUG(4,"Lock File--can't read (errno %d) --remove it!\n", errno);
	    goto unlk;
	}
#ifdef	ASCIILOCKS
	ret = read(fd, (char *) alpid, SIZEOFPID+1); /* +1 for '\n' */
	(void) close(fd);
	if (ret != (SIZEOFPID+1)) {
#else
	ret = read(fd, (char *) &lpid, sizeof(int));
	(void) close(fd);
	if (ret != sizeof(int)) {
#endif

	    DEBUG(4, "Lock File--bad format--remove it!\n", "");
	    goto unlk;
	}
#ifdef	ASCIILOCKS
	lpid = atoi(alpid);
#endif
	if ((ret=kill(lpid, 0)) == 0 || errno == EPERM) {
	    DEBUG(4, "Lock File--process still active--not removed\n","");
	    return(-1);
	}
	else { /* process no longer active */
	    DEBUG(4, "kill pid (%d), ", lpid);
	    DEBUG(4, "returned %d", ret);
	    DEBUG(4, "--ok to remove lock file (%s)\n", file);
	}
unlk:
	
	if (unlink(file) != 0) {
		DEBUG(4,"ttylock failed in unlink()\n","");
		return(-1);
	}
	return(0);
}


/*
 * put name in list of lock files
 * return:
 *	none
 */
putlock(name)
char *name;
{
	register int i;
	char *p;

	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			break;
	}
	if (i >= MAXLOCKS) {
		DEBUG (4, "TOO MANY LOCKS - EXITING", "");
		clrlock(NULL);
		exit(-1);
	}
	if (i >= numlocks)
		i = numlocks++;
	p = (char *) calloc((unsigned) strlen(name) + 1, sizeof (char));
	if (p == NULL) {
		DEBUG (4, "CAN NOT ALLOCATE FOR %s\n", name);
		clrlock(NULL);
		exit (-1);
	}
	(void) strcpy(p, name);
	lckarray[i] = p;
	return(0);
}

/*
 * makes a lock on behalf of pid.
 * input:
 *	pid - process id
 *	tempfile - name of a temporary in the same file system
 *	name - lock file name (full path name)
 * return:
 *	-1 - failed
 *	0  - lock made successfully
 */
static
mklock(pid,tempfile,name)
#ifdef	ASCIILOCKS
char *pid;
#else
int pid;
#endif 
char *tempfile, *name;
{	
	register int fd;
	register int rc;
	char	cb[100];

	fd=creat(tempfile, (mode_t)0444);
	if(fd < 0){
		(void) sprintf(cb, "%s %s %d",tempfile, name, errno);
		DEBUG (4, "ULOCKC %s\n", cb);
		if((errno == EMFILE) || (errno == ENFILE))
			(void) unlink(tempfile);
		return(-1);
	}
#ifdef	ASCIILOCKS
	if (write(fd, pid, SIZEOFPID+1) != (SIZEOFPID+1)) {
		/* error writing to lock file */
		(void)close(fd);
		(void)unlink(tempfile);
		return(-1);
	}
#else
	if (write(fd, (char *) &pid, sizeof(int)) != sizeof(int)) {
		/* error writing to lock file */
		(void)close(fd);
		(void)unlink(tempfile);
		return(-1);
	}
#endif
	(void) chmod(tempfile,0444);
	/* (void) chown(tempfile, UUCPUID, UUCPGID); */
	(void) close(fd);
	if(link(tempfile,name)<0){
		DEBUG(4, "errno: %d ", errno);
		DEBUG(4, "link(%s, ", tempfile);
		DEBUG(4, "%s)\n", name);
		if(unlink(tempfile)< 0){
			(void) sprintf(cb, "ULK err %s %d", tempfile,  errno);
			DEBUG (4, "ULOCKLNK %s\n", cb);
		}
		return(-1);
	}
	if(unlink(tempfile)<0){
		(void) sprintf(cb, "%s %d",tempfile,errno);
		DEBUG (4, "ULOCKF %s\n", cb);
	}
	return(0);
}


/*
 * remove a lock file
 * return:
 *  none
 */
ttyunlock(device)
char *device;
{
	char ln[MAXNAMESIZE];
	char basedevice[MAXBASENAME];

	unmux(device, basedevice);
	(void) sprintf(ln, "%s.%s", LOCKPRE, basedevice);
	BASENAME(ln, '/')[MAXBASENAME] = '\0';
	clrlock(ln);
	return;
}


/*
 * remove lock file associated with name
 * return:
 *	none
 */
clrlock(name)
register char *name;
{
	register int i;

	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			continue;
		if (name == NULL || EQUALS(name, lckarray[i])) {
			(void) unlink(lckarray[i]);
			(void) free(lckarray[i]);
			lckarray[i] = NULL;
		}
	}
	return;
}




/*
 * update access and modify times for lock files
 * return:
 *	none
 */
void
ttytouchlock()
{
	register int i;

	struct {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			continue;
		utime(lckarray[i], &ut);
	}
	return;
}

/*
 * Wait for device lock to go away
 *
 * return:
 *	none
 */
ttywait(device)
char *device;
{
	    for (;;) {	/* busy wait for LCK..line to go away */
		/* Defect 10047 - This sleep needs to be short or
		 * getty will wait too long raising DTR after a dialout.
		 * This can inhibit the use of callback modems for instance.
		 */
		sleep(5);
		if (ttylocked(device) == 0) /* LCK..line gone */
		    break;
	    }
}

/*
 * Copies string s to string t, starting after leading "/dev/" and
 * converting any '/' to '.'
 *
 * return:
 *	none
 */	
static void 
unmux(char *s, char *t)
{
    struct stat unstat;
    char undev[] = "/dev/";
    register int unlen = strlen(undev);
    register char *dev;

    /* were we given full path of a device ? */
    if (!strncmp(s, undev, unlen)) {

	/* copy starting after "/dev/" */
	strcpy(t, s + unlen);
	dev = s;
    } else {
	char tname[MAXBASENAME];

	strcpy(t, s);
	strcpy(tname, undev);
	strcat(tname, s);
	dev = tname;
    }
    if (!stat(dev, &unstat) && S_ISCHR(unstat.st_mode)) {
	/* ability to check st_type against VMPC is broken */
	struct qry_devsw qdevsw;

	qdevsw.devno = unstat.st_rdev;
	sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw));
	if ((qdevsw.status & DSW_MPX) && unstat.st_size != -1)
	    /* now have a channel of an mpx device */
	    if (dev = strrchr(t, '/')) 
		*dev = '\0';
    }
}
