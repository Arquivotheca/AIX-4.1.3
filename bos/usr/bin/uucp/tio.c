static char sccsid[] = "@(#)20	1.11  src/bos/usr/bin/uucp/tio.c, cmduucp, bos411, 9428A410j 6/24/91 16:13:01";
/* 
 * COMPONENT_NAME: UUCP tio.c
 * 
 * FUNCTIONS: min, trdblk, trddata, trdmsg, twrblk, twrdata, twrmsg 
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

/*
"tio.c	4.6 (Berkeley) 1/24/86";
*/

#include <signal.h>
#include "uucp.h"
#include <setjmp.h>
#include <sys/stat.h>
#include <netinet/in.h>

extern nl_catd catd;
extern int pkfail(int);
extern int maxmsg;		/* vary the wait time between packets */

#define TPACKSIZE	512
#define TBUFSIZE	1024
#define min(a,b)	(((a)<(b))?(a):(b))

/*
 *	htonl is a function that converts a long from host
 *		order to network order
 *	ntohl is a function that converts a long from network
 *		order to host order
 *
 *	network order is 		0 1 2 3 (bytes in a long)
 *	host order on a vax is		3 2 1 0
 *	host order on a pdp11 is	1 0 3 2
 *	host order on a 68000 is	0 1 2 3
 *	most other machines are		0 1 2 3
 */

struct tbuf {
	long t_nbytes;
	char t_data[TBUFSIZE];
};

extern jmp_buf Failbuf;

twrmsg(type, str, fn)
char type;
register char *str;
{
	char bufr[TBUFSIZE];
	register char *s;
	int len, i;
	int send_len;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	alarm(maxmsg);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	/* Next, len is forced to be an integer multiple of TPACKSIZE */
	if ((i = len % TPACKSIZE)) {
		send_len = len + TPACKSIZE - i;	/* Round up. */
		while (len < send_len)		/* Pad out the unused */
			bufr[len++] = '\0';	/* buffer with nulls. */
	}
	twrblk(bufr, len, fn);
	alarm(0);
	return SUCCESS;
}

trdmsg(str, fn)
register char *str;
{
	int len, cnt = 0;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	alarm(maxmsg);
	for (;;) {
		len = read(fn, str, TPACKSIZE);
		if (len == 0)
			continue;
		if (len < 0) {
			alarm(0);
			return FAIL;
		}
		str += len;
		cnt += len;
		if (*(str - 1) == '\0' && (cnt % TPACKSIZE) == 0)
			break;
	}
	alarm(0);
	return SUCCESS;
}

twrdata(fp1, fn)
FILE *fp1;
{
	struct tbuf bufr;
	register int len;
	int ret, mil;
#ifdef AIX
	time_t ticks;
#else
	struct timeb t1, t2;
#endif
	long bytes;
	char text[TBUFSIZE];

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
	bytes = 0L;
#ifdef AIX
	(void) millitick();
#else !AIX
#ifdef USG
	time(&t1.time);
	t1.millitm = 0;
#else !USG
	ftime(&t1);
#endif !USG
#endif AIX
	while ((len = read(fileno(fp1), bufr.t_data, TBUFSIZE)) > 0) {
		bytes += len;
#if defined(vax) || defined(pdp11) || defined(ns32000) || defined(AIX)
		bufr.t_nbytes = htonl((long)len);
#else !vax and !pdp11 and !ns32000 and !AIX
		bufr.t_nbytes = len;
#endif !vax and !pdp11 and !ns32000 and !AIX
		DEBUG(8,"twrdata sending %d bytes\n",len);
		len += sizeof(long);
		alarm(maxmsg);
		ret = twrblk((char *)&bufr, len, fn);
		alarm(0);
		if (ret != len)
			return FAIL;
		if (len != TBUFSIZE+sizeof(long))
			break;
	}
	bufr.t_nbytes = 0;
	len = sizeof(long);
	alarm(maxmsg);
	ret = twrblk((char *)&bufr, len, fn);
	alarm(0);
	if (ret != len)
		return FAIL;
#ifdef AIX
	ticks = millitick();
	sprintf(text, "-> %ld / %ld.%.3d secs", bytes, ticks / 1000, ticks % 1000);
#else !AIX
#ifdef USG
	time(&t2.time);
	t2.millitm = 0;
#else !USG
	ftime(&t2);
#endif !USG
	Now = t2;
	t2.time -= t1.time;
	mil = t2.millitm - t1.millitm;
	if (mil < 0) {
		--t2.time;
		mil += 1000;
	}
	sprintf(text, MSGSTR(MSG_TIO2, "sent data %ld bytes %ld.%02d secs"),
				bytes, (long)t2.time, mil/10);
	sysacct(bytes, t2.time);
#endif AIX
	DEBUG(1, "%s\n", text);
	uusyslog(text);
	return SUCCESS;
}

trddata(fn, fp2)
FILE *fp2;
{
	register int len, nread;
	char bufr[TBUFSIZE];
#ifdef AIX
	time_t ticks;
#else
	struct timeb t1, t2;
#endif
	int mil;
	long bytes, Nbytes;

	if(setjmp(Failbuf))
		return FAIL;
	signal(SIGALRM, (void(*)(int)) pkfail);
#ifdef AIX
	(void) millitick();
#else !AIX
#ifdef USG
	time(&t1.time);
	t1.millitm = 0;
#else !USG
	ftime(&t1);
#endif !USG
#endif AIX
	bytes = 0L;
	for (;;) {
		alarm(maxmsg);
		len = trdblk((char *)&Nbytes,sizeof Nbytes,fn);
		alarm(0);
		if (len != sizeof Nbytes)
			return FAIL;
#if defined(vax) || defined(pdp11) || defined(ns32000) || defined(AIX)
		Nbytes = ntohl(Nbytes);
#endif vax or pdp11 or ns32000 or AIX
		DEBUG(8,"trddata expecting %ld bytes\n", Nbytes);
		nread = Nbytes;
		if (nread == 0)
			break;
		alarm(maxmsg);
		len = trdblk(bufr, nread, fn);
		alarm(0);
		if (len < 0) {
			return FAIL;
		}
		bytes += len;
		DEBUG(11, "trddata got %ld\n",bytes);
		if (write(fileno(fp2), bufr, len) != len) {
			alarm(0);
			return FAIL;
		}
	}
#ifdef AIX
	ticks = millitick();
	sprintf(bufr, "<- %ld / %ld.%.3d secs"
		, bytes, ticks / 1000, ticks % 1000);
#else !AIX
#ifdef USG
	time(&t2.time);
	t2.millitm = 0;
#else !USG
	ftime(&t2);
#endif !USG
	Now = t2;
	t2.time -= t1.time;
	mil = t2.millitm - t1.millitm;
	if (mil < 0) {
		--t2.time;
		mil += 1000;
	}
	sprintf(bufr, MSGSTR(MSG_TIO6, 
		"received data %ld bytes %ld.%02d secs"), bytes, 
		(long)t2.time, mil/10);
	sysacct(bytes, t2.time - t1.time);
#endif AIX
	DEBUG(1, "%s\n", bufr);
	uusyslog(bufr);
	return SUCCESS;
}

#if !defined(BSD4_2) && !defined(USG)
#define	TC	1024
static	int tc = TC;
#endif !BSD4_2 && !USG

trdblk(blk, len,  fn)
register int len;
char *blk;
{
	register int i, ret;

#if !defined(BSD4_2) && !defined(USG)
	/* call ultouch occasionally */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
#endif !BSD4_2 && !USG
	for (i = 0; i < len; i += ret) {
		ret = read(fn, blk, len - i);
		if (ret < 0)
			return FAIL;
		blk += ret;
		if (ret == 0)
			return i;
	}
	return i;
}


twrblk(blk, len, fn)
register char *blk;
{
#if !defined(BSD4_2) && !defined(USG)
	/* call ultouch occasionally */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
#endif !BSD4_2 && !USG
	return write(fn, blk, len);
}
