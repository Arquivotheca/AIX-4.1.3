static char sccsid[] = "@(#)91	1.8  src/bos/usr/bin/uucp/eio.c, cmduucp, bos411, 9428A410j 6/17/93 14:04:17";
/* 
 * COMPONENT_NAME: CMDUUCP eio.c
 * 
 * FUNCTIONS: MIN, ealarm, erdblk, erddata, erdmsg, eturnoff, 
 *            eturnon, ewrdata, ewrmsg 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.eio.c
	eio.c	1.4	7/29/85 16:32:54
*/

#include "uucp.h"
/*
VERSION( eio.c	1.1 - 87/06/30 - 19:02:30 );
*/

#ifdef ATTSV
#define     MIN(a,b) (((a)<(b))?(a):(b))
#endif

static jmp_buf Failbuf;

#define EBUFSIZ 1024    /* "e" protocol buffer size */

/*      This constant MUST be the same on all machines running "e"
        uucico protocol!  It must remain this value forever!
*/
#define CMSGSIZ 20      /* size of the initial file size message */


static void ealarm(int);

/*
 * error-free channel protocol
 */
static void
ealarm(int s) {
	longjmp(Failbuf, 1);
}
static void (*esig)(int);

/*
 * turn on protocol timer
 */
eturnon()
{
	esig=signal(SIGALRM, (void(*)(int))  ealarm);
	return(0);
}

eturnoff()
{
	signal(SIGALRM, esig);
	return(0);
}

/*
 * write message across link
 *	type	-> message type
 *	str	-> message body (ascii string)
 *	fn	-> link file descriptor
 * return
 *	FAIL	-> write failed
 *	0	-> write succeeded
 */
ewrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[BUFSIZ];
	int	s1, s2;

	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	s1 = strlen(bufr) + 1;
	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrmsg write failed\n", "");
		return(FAIL);
	}
	alarm(60);
	s2 = (*Write)(fn, bufr, (unsigned) s1);
	alarm(0);
	if (s1 != s2)
		return(FAIL);
	return(0);
}

/*
 * read message from link
 *	str	-> message buffer
 *	fn	-> file descriptor
 * return
 *	FAIL	-> read timed out
 *	0	-> ok message in str
 */
erdmsg(str, fn)
register char *str;
{
	register int i;
	register int len;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdmsg read failed\n", "");
		return(FAIL);
	}

	i = BUFSIZ;
	for (;;) {
		alarm(60);
		len = (*Read)(fn, str, i);
		alarm(0);
		if (len < 0) return(FAIL);
		str += len; i -= len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}

/*
 * read data from file fp1 and write
 * on link
 *	fp1	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	FAIL	->failure in link
 *	0	-> ok
 */
ewrdata(fp1, fn)
int	fn;
register FILE *fp1;
{
	register int ret;
	int len;
	long bytes;
	char bufr[EBUFSIZ];
	char text[EBUFSIZ];
	time_t	ticks;
	int	mil;
	struct stat	statbuf;
	off_t	msglen;
	char	cmsglen[CMSGSIZ];

	if (setjmp(Failbuf)) {
		DEBUG(7, "ewrdata failed\n", "");
		return(FAIL);
	}
	bytes = 0L;
	fstat(fileno(fp1), &statbuf);
	bytes = msglen = statbuf.st_size;
	(void) millitick();
	sprintf(cmsglen, "%ld", (long) msglen);
	alarm(60);
	ret = (*Write)(fn, cmsglen, sizeof(cmsglen));
	DEBUG(7, "ewrmsg write %d\n", statbuf.st_size);
	while ((len = fread(bufr, sizeof (char), BUFSIZ, fp1)) > 0) {
		alarm(60);
		ret = (*Write)(fn, bufr, (unsigned) len);
		alarm(0);
		DEBUG(7, "ewrmsg ret %d\n", ret);
		if (ret != len)
			return(FAIL);
		if ((msglen -= len) <= 0)
			break;
	}
	if (len < 0 || (len == 0 && msglen != 0)) return(FAIL);
	ticks = millitick();
        sprintf(text, "-> %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
        DEBUG(4, "%s\n", text);
        uusyslog(text);
	return(0);
}

/*
 * read data from link and
 * write into file
 *	fp2	-> file descriptor
 *	fn	-> link descriptor
 * returns:
 *	0	-> ok
 *	FAIL	-> failure on link
 */
erddata(fn, fp2)
register FILE *fp2;
{
	register int len;
	int fd2;
	long bytes;
	char text[EBUFSIZ];
	char bufr[EBUFSIZ];
	time_t	ticks;
	int	mil;
	long	msglen;
	char	cmsglen[CMSGSIZ];
        int     writefile = TRUE, ret = SUCCESS;

	bytes = 0L;
	(void) millitick();
	len = erdblk(cmsglen, sizeof(cmsglen), fn);
	if (len < 0) return(FAIL);
	sscanf(cmsglen, "%ld", &msglen);
	bytes = msglen;
	DEBUG(7, "erdblk msglen %d\n", msglen);
        if ( ((msglen-1)/512 +1) > Ulimit ) {
                ret = EFBIG;
                writefile = FALSE;
        }
        bytes = msglen;
        fd2 = fileno( fp2 );
	for (;;) {
		len = erdblk(bufr, MIN(msglen, BUFSIZ), fn);
		DEBUG(7, "erdblk ret %d\n", len);
		if (len < 0) {
			DEBUG(7, "erdblk failed\n", "");
			return(FAIL);
		}
		if ((msglen -= len) < 0) {
			DEBUG(7, "erdblk read too much\n", "");
			return(FAIL);
		}
                /* this write is to file -- use write(2), not (*Write) */
                if ( writefile == TRUE && write( fd2, bufr, len ) != len ) {
                        ret = errno;
                        DEBUG(7, "erddata: write to file failed, errno %d\n", ret);
                        writefile = FALSE;
                }
		if (msglen == 0)
			break;
	}
        if ( writefile == TRUE ) {
                ticks = millitick();
			sprintf(text, "<- %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
			DEBUG(4, "%s\n", text);
			uusyslog(text);
                return(SUCCESS);
        }
        else
                return(ret);
}

/*
 * read block from link
 * reads are timed
 *	blk	-> address of buffer
 *	len	-> size to read
 *	fn	-> link descriptor
 * returns:
 *	FAIL	-> link error timeout on link
 *	i	-> # of bytes read
 */
erdblk(blk, len,  fn)
register char *blk;
{
	register int i, ret;

	if(setjmp(Failbuf)) {
		DEBUG(7, "erdblk timeout\n", "");
		return(FAIL);
	}

	for (i = 0; i < len; i += ret) {
		alarm(60);
		DEBUG(7, "ask %d ", len - i);
		if ((ret = (*Read)(fn, blk, (unsigned) len - i)) <= 0) {
			alarm(0);
			DEBUG(7, "read failed\n", "");
			return(FAIL);
		}
		DEBUG(7, "got %d\n", ret);
		blk += ret;
		if (ret == 0)
			break;
	}
	alarm(0);
	return(i);
}
