static char sccsid[] = "@(#)63	1.13  src/bos/usr/bin/dosdir/devio.c, cmdpcdos, bos411, 9428A410j 6/10/94 14:37:33";
/*
 * COMPONENT_NAME: LIBDOS  routines to read dos floppies
 *
 * FUNCTIONS: _devio 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define _NO_PROTO
#include <stdio.h>
#include "errno.h"
#include "fcntl.h"
#include "sys/signal.h"
#include <sys/stat.h>
#ifdef aiws
#include "sys/fd.h"
#endif
#include "pcdos.h"

#include <nl_types.h>
#include "dosdir_msg.h" 
#define MSGSTR(N,S) catgets(catd,MS_DOSDIR,N,S) 
extern nl_catd catd;

/*
 *          _devio - handle DOS abort/retry/ignore semantics.
 *
 * This needs to be enveloped around every system call that might
 * spin a floppy in libdos.
 *
 * Note 1: sync() and fsync() don't spin the disk.
 *
 * Note 2: calls to access UNIX files on a mounted disk need not return
 *         EIO or ENXIO since I/O happens asynchronously in the kernel.
 */

extern int ioctl();
extern int doserrno;
extern char quiet;

#define NUMERR	6
static int errnolist[NUMERR] = {
	ENOTREADY, EBUSY, EIO, EWRPROTECT, ENXIO, EFORMAT
};

_devio(op,arg1,arg2,arg3,arg4)
register int (*op)(),arg1,arg2,arg3,arg4;
{
	return _devio1(NULL,0,op,arg1,arg2,arg3,arg4);
}


_devio1(disk, seekp, op,arg1,arg2,arg3,arg4)
DCB *disk;
long seekp;
register int (*op)(),arg1,arg2,arg3,arg4;
{
	register int rc;
	int ch = -1, i;
	char message[28];
	char *opname;
	char *abort_resp, *retry_resp, *ignor_resp;

retry:
	if (op)		/* op == 0 if dmount() opened device O_RDONLY */
	{
		rc = (*op)(arg1,arg2,arg3,arg4);
		if (rc >= 0 ) return rc;
	}
	else
		errno = EWRPROTECT;

	for(i=0; i<NUMERR; i++)
		if (errno == errnolist[i])
			break;
	if (i == NUMERR) {
		doserrno = errno;
		return rc;
	}

	if      (op == open)   opname = "open";
	else if (op == read)   opname = "read";
	else if (op == write)  opname = "write";
	else if (op == ioctl)  opname = "ioctl";
	else if (op == fstat)  opname = "fstat";
	else if (op == close)  opname = "close";
	else if (op == link)   opname = "link";
	else if (op == unlink) opname = "unlink";
	else if (op == 0)      opname = "write";
	else                   opname = "syscall";

	fprintf(stderr, "%s: %s: ", opname, devicename);
	perror();

query:
	if (quiet) {
		if (op != open && op != close)
			close(arg1);
		_runmount();
		return(-1);
	}
	fprintf(stderr,MSGSTR(ABORT, "Abort, Retry, Ignore?"));
	abort_resp = MSGSTR(A_RESP, "Aa");
	retry_resp = MSGSTR(R_RESP, "Rr");
	ignor_resp = MSGSTR(I_RESP, "Ii");
	if ((ch = _pause(" ")) == -1) return -1;  /* can't open tty */
	switch (ch) {
		case '\n':
			fprintf(stderr,"\n");
			break;
		case '\003':
		case '\177':
			fprintf(stderr,"^C\n");
			ch = abort_resp[0];
			break;
		default:
			fprintf(stderr,"%c\n", ch);
	}
								/* ABORT */
	if (strchr(abort_resp, ch)) {
		if (op != open && op != close)
			close(arg1);
		_runmount();
		kill( getpid(), SIGINT );
		return(-1);
	}
								/* RETRY */
	if (strchr(retry_resp, ch)) {
#ifdef aiws
		if (op != open && op != ioctl && op != 0)
			ioctl(arg1, FDIOCRESET, 0 );
#endif
		if (disk && errno == EWRPROTECT && op == 0) {
			int tmpfd;

			op = write;
			close(arg1);
			tmpfd = open(disk->dev_name, O_RDWR|020);
			if (tmpfd < 0) {
				op = 0;
				goto retry;
			}
			lseek(disk->fd, seekp, 0);
			disk->fd = tmpfd;
			disk->protect = (int)write;
		}

		goto retry;
	}

	if (!strchr(ignor_resp, ch))	/* -not ignore- */
		goto query;

								/* IGNORE */
	/* ignore option depends on routine called */
	if (op == open)
		return open("/dev/null", arg2, arg3 );
	if (op == read || op == write || op == 0) {
		/* to ignore, seek past requested block */
		lseek(arg1,arg3,1);
		return arg3;
	}
	return 0;
}
