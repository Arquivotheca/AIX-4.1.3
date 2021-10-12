static char sccsid[] = "@(#)44  1.25.2.12  src/bos/kernel/proc/acct.c, sysproc, bos411, 9428A410j 6/15/94 17:11:32";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: acct
 *		acctexit
 *		compress
 *		compress_int
 *
 *   ORIGINS: 27, 3
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/acct.h>
#include <sys/fp_io.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/lockf.h>
#include <sys/vnode.h>
#include <sys/syspest.h>
#include <sys/priv.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/var.h>
#include <sys/errids.h>
#include <sys/wait.h>


#define AHZ 64
/*
 * Declarations
 */

extern uid_t getuidx(int);
extern gid_t getgidx(int);

long compress(register long t, long ut);

#ifndef	_LONG_LONG

/*
 * Simulate 64 bit division.  Not required if (long long) is a
 * supported data type.
 */

#define DIVL64(ll,d) (ll[0] == 0 ? ll[1] / d:_divl64(ll,d))
static unsigned long _divl64 (unsigned long * ll, unsigned long d);
#endif

/* Accounting turned off error */
static struct err_rec0 
	progacct_log = {ERRID_ACCT_OFF,"SYSPROCACCT"};

/*
 * Perform process accounting functions.
 */

/*
 * NAME: 	acct()	(system call entry point)
 *
 * FUNCTION:	Used to open or close the accounting log file. It
 *		shares the function copen with the open and creat
 *		system calls.  It also shares the fp_close system call
 *		with close.
 *
 * PARAMETERS:	fname. Fname is either a character pointer to the name
 *		of the accounting file to open, or a NULL pointer.  If
 *		fname is a NULL, then the accounting file is closed.
 *
 * RETURN VALUE:	Zero is returned for successful operation or
 *			if an error occurs the error is returned.
 */
static struct file *acctp = (struct file *) NULL;
lock_t acct_lock = LOCK_AVAIL;

BUGVDEF(bugacct, 0);

int
acct(char *fname )
{
	int waslocked;
	int rc;			/* return code from fp_open()	*/
	static int svcnumE = 0;
	static int svcnumD = 0;

	if (privcheck(SYS_OPER) == EPERM) {
		u.u_error = EPERM;
		return(-1);
	}

	waslocked = lockl(&acct_lock, LOCK_SHORT);

	if (fname == NULL) {
		if(audit_flag && audit_svcstart("ACCT_Disable", &svcnumD, 0)){
			audit_svcfinis();
		}
		if (acctp != NULL) {
			/*
			 * Close file and return system open file table
			 * element previously held by acctp
			 */
			(void)fp_close(acctp);
			acctp = NULL;
		}
	} else {
		if(audit_flag && audit_svcstart("ACCT_Enable", &svcnumE, 0)){
			if(fname){
	                        char *ptr;
	                        int len;
	
	                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                        		if(waslocked != LOCK_NEST){
                                		unlockl(&acct_lock);
					}
	                                u.u_error = ENOMEM;
                        		return(-1);
	                        }
	                        if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
                        		if(waslocked != LOCK_NEST){
                                		unlockl(&acct_lock);
					}
	                                u.u_error = EFAULT;
                        		return(-1);
	                        }
                        	audit_svcbcopy(ptr, len);
                        	free(ptr);
			}
			audit_svcfinis();
		}
		if (acctp != NULL) {
			if (waslocked != LOCK_NEST)
				unlockl(&acct_lock);
			u.u_error = EBUSY;
			return(-1);
		}
		if (!(rc = fp_open(fname, O_RDWR|O_APPEND, 0, 0,
				FP_USR, &acctp))) {
			if((acctp->f_vnode)->v_vntype != VREG) {
				(void)fp_close(acctp);
				acctp = NULL;
				if (waslocked != LOCK_NEST)
					unlockl(&acct_lock);
				u.u_error = EACCES;
				return(-1);
			} 
		}
		else
		{
			acctp = NULL;
			if (waslocked != LOCK_NEST)
				unlockl(&acct_lock);
			u.u_error = rc;
			return(-1);
		}
	}

	if (waslocked != LOCK_NEST)
		unlockl(&acct_lock);
	return(0);
}

/*
 * On exit, write a record on the accounting file.
 * This routine is ONLY called by exit().  In order to use
 * the vn_rdwr() vnode operation we must have our buffer in user
 * address space. I don't think this guy will mind if we borrow
 * his stack.
 */

void
acctexit(int st)
{
	register struct file *fp;
	register struct vnode *vp;
	struct uio   uio_struct;
	struct iovec iovector;
	struct acct buf;
	lock_t kernlocked;
	lock_t acctlocked;
	struct timeval t;
	long etime_ticks;
	long etime_sec = 0;
	long etime_usec = 0;
	register int i;

	/* serialize with fp_close in acct() */
	acctlocked = lockl(&acct_lock, LOCK_SHORT);

	if ((fp=acctp) == NULL) {
		if (acctlocked != LOCK_NEST)
		    unlockl(&acct_lock);
		return;
	}
	/*
	 * Called at exit only
	 */
	bcopy(U.U_comm,buf.ac_comm,sizeof(buf.ac_comm));
	buf.ac_btime = U.U_start;
	buf.ac_utime = compress((long)U.U_utime, (long)U.U_ru.ru_utime.tv_usec);
	buf.ac_stime = compress((long)U.U_stime, (long)U.U_ru.ru_stime.tv_usec);
	etime_ticks = lbolt - U.U_ticks;
	if (etime_ticks) {
		etime_sec = etime_ticks / HZ;
		etime_usec = (etime_ticks % HZ) * NS_PER_TICK;
	}
	buf.ac_etime = compress(etime_sec, etime_usec);
	buf.ac_io = compress_int(U.U_ioch); 
	buf.ac_rw = compress(U.U_ior+U.U_iow, 0); 
	buf.ac_uid = getuidx(ID_REAL);
	buf.ac_gid = getgidx(ID_REAL);
	buf.ac_tty = U.U_ttyp ? U.U_ttyd : NODEVICE;
	buf.ac_stat = WIFEXITED(st) ? WEXITSTATUS(st):WTERMSIG(st);
	buf.ac_flag = U.U_acflag;
	if (WIFSIGNALED(st)) {
		buf.ac_flag |= AXSIG;
		if (st & 0x80)
			buf.ac_flag |= ACORE;
	}
	u.u_error = 0;
	/* max file size, so all acct info can be written out */
	U.U_limit =  RLIM_INFINITY; 

	t = U.U_ru.ru_stime;
	timevaladd(&t, &U.U_ru.ru_utime);

	/* calculate the mean size by dividing memory integral by ticks */
	if (i = t.tv_sec * HZ + t.tv_usec / (NS_PER_SEC / HZ))
#ifdef	_LONG_LONG
		buf.ac_mem = compress_int((long) (U.U_irss / i));
#else
		buf.ac_mem = compress_int((long) DIVL64 (U.U_irss, i));
#endif
	else
		buf.ac_mem = 0;

	/* set up the uio structure */
	iovector.iov_base = (caddr_t)&buf;
	iovector.iov_len = sizeof (struct acct);

	uio_struct.uio_iov = &iovector;
	uio_struct. uio_iovcnt = 1;
	uio_struct.uio_offset = 0;
	uio_struct.uio_segflg = UIO_SYSSPACE;
	uio_struct.uio_resid = sizeof (struct acct);
	vp = acctp->f_vnode;

	if (VNOP_RDWR(vp, UIO_WRITE, fp->f_flag, &uio_struct, NULL,
			fp->f_vinfo, NULL, fp->f_cred)) {
		/* Filesystem that holds the pacct file is probably full */
		struct stat statbuf;
		register int oldsize, newsize;

		if (! fp_fstat(acctp, &statbuf, STATSIZE, FP_SYS)) {
			oldsize = statbuf.st_size;
			newsize = oldsize - (oldsize % sizeof(struct acct));
			if ( oldsize != newsize) 
				VNOP_FTRUNC(vp, fp->f_flag, newsize,
						fp->f_vinfo, fp->f_cred);
		}
		if (acctlocked != LOCK_NEST)
			unlockl(&acct_lock);

		/* Turn off accounting */
		acct((char *)NULL);
		errsave(&progacct_log, sizeof(progacct_log));
		return;
	}

	if (acctlocked != LOCK_NEST)
	    unlockl(&acct_lock);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
long
compress(register long t, long ut)
{
	register ulong exp = 0, round = 0;

	t = t * AHZ; /*  compiler will convert only this format to a shift */
	if (ut)	{
		ut /= 1000;	/* convert from nanoseconds to microseconds */
		t += ut / (1000000/AHZ);
	}
	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return ((exp<<13) + t);
}

long
compress_int(register long t)
{
	register ulong exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return ((exp<<13) + t);
}

#ifndef	_LONG_LONG
/*
 * Name: _divl64
 *
 * Function: Perform 64 bit dividend by 32 bit divisor division
 *
 * Note: This function is only called for extremely CPU consumptive
 *	 processes.  It has not been written to be real fast and
 *	 since it is only used for memory accounting needn't give
 *	 accuracy better than 13 significant bits.  When the
 *	 build environment compilers support (long long) it can
 *	 be removed.
 */

static unsigned long
_divl64 (unsigned long * ll, unsigned long div)
{
	while (div != 0 && ll[0] != 0) {
		div >>= 1;
		ll[1] >>= 1;
		ll[1] |= (ll[0] & 01) << 31;
		ll[0] >>= 1;
	}
	if (div != 0)
		return ll[1] / div;
	else
		return INT_MAX;
}
#endif
