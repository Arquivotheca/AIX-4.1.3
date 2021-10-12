static char sccsid[] = "@(#)42	1.9  src/bos/usr/bin/csh/time.c, cmdcsh, bos411, 9438C411a 9/23/94 17:02:49";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: settimes dotime ptimes
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include "sh.h"

#define MICRO	1000000L
#define HEC	100L
#define HS 	(long)(MICRO/HEC)
#define MICRO_to_TENTHS       (long)(MICRO/10) /* mili-Secs to 10ths-of-Sec */


void
settimes(void)
{
	struct rusage ruch;

	(void)gettimeofday(&time0, (struct timezone *)0);
	(void) getrusage(RUSAGE_SELF, &ru0);
	(void) getrusage(RUSAGE_CHILDREN, &ruch);
	ruadd(&ru0, &ruch);
}

/*
 * dotime is only called if it is truly a builtin function and not a
 * prefix to another command
 */
void
dotime(void)
{
	struct timeval timedol;
	struct rusage ru1, ruch;

	(void) getrusage(RUSAGE_SELF, &ru1);
	(void) getrusage(RUSAGE_CHILDREN, &ruch);
	ruadd(&ru1, &ruch);
	(void)gettimeofday(&timedol,(struct timezone *)0);
	prusage(&ru0, &ru1, &timedol, &time0);
}

void
ruadd(register struct rusage *ru, register struct rusage *ru2)
{
	register long *lp, *lp2;
	register int cnt;

	tvadd(&ru->ru_utime, &ru2->ru_utime);
	tvadd(&ru->ru_stime, &ru2->ru_stime);
	if (ru2->ru_maxrss > ru->ru_maxrss)
		ru->ru_maxrss = ru2->ru_maxrss;
	cnt = &ru->ru_last - &ru->ru_first + 1;
	lp = &ru->ru_first; lp2 = &ru2->ru_first;
	do
		*lp++ += *lp2++;
	while (--cnt > 0);
}

void
prusage(register struct rusage *r0,
	register struct rusage *r1,
	struct timeval *e,
	struct timeval *b)
{
	register time_t t =
	    (r1->ru_utime.tv_sec-r0->ru_utime.tv_sec)*HEC+
	    (r1->ru_utime.tv_usec-r0->ru_utime.tv_usec)/HS+
	    (r1->ru_stime.tv_sec-r0->ru_stime.tv_sec)*HEC+
	    (r1->ru_stime.tv_usec-r0->ru_stime.tv_usec)/HS;
	register uchar_t *cp;
	register int i;
	register struct varent *vp = adrof("time");
	time_t ms = 
	    (e->tv_sec-b->tv_sec)*HEC + (e->tv_usec-b->tv_usec)/HS;

	cp = (uchar_t *)"%Uu %Ss %E %P %X+%Dk %I+%Oio %Fpf+%Ww";
	if (vp && vp->vec[0] && vp->vec[1])
		cp = vp->vec[1];
	for (; *cp; cp++) {
		if (*cp != '%') {
			display_char(*cp);
		}
		else if (cp[1]) switch(*++cp) {

		case 'U':
			pdeltat(&r1->ru_utime, &r0->ru_utime);
			break;

		case 'S':
			pdeltat(&r1->ru_stime, &r0->ru_stime);
			break;

		case 'E':
			psecs((long)(ms / HEC));
			break;

		case 'P':
			printf("%d%%", (int) (t*HEC / ((ms ? ms : 1))));
			break;

		case 'W':
			i = r1->ru_nswap - r0->ru_nswap;
			printf("%d", i);
			break;

		case 'X':
			printf("%d", 
				t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
			break;

		case 'D':
			printf("%d", t == 0 ? 0 :
			    (r1->ru_idrss+r1->
				ru_isrss-(r0->ru_idrss+r0->ru_isrss))/t);
			break;

		case 'K':
			printf("%d", t == 0 ? 0 :
			((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
			    (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
			break;

		case 'M':
			printf("%d", r1->ru_maxrss);
			break;

		case 'F':
			printf("%d", r1->ru_majflt-r0->ru_majflt);
			break;

		case 'R':
			printf("%d", r1->ru_minflt-r0->ru_minflt);
			break;

		case 'I':
			printf("%d", r1->ru_inblock-r0->ru_inblock);
			break;

		case 'O':
			printf("%d", r1->ru_oublock-r0->ru_oublock);
			break;
		}
	}
	printf("\n");
}

void
pdeltat(struct timeval *t1, struct timeval *t0)
{
	struct timeval td;

	tvsub(&td, t1, t0);
	printf("%d.%01d", td.tv_sec, (int)(td.tv_usec / MICRO_to_TENTHS) );
}

void
tvadd(struct timeval *tsum, struct timeval *t0)
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > MICRO)
		tsum->tv_sec++, tsum->tv_usec -= MICRO;
}

void
tvsub(struct timeval *tdiff, struct timeval *t1, struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += MICRO;
}
