static char sccsid[] = "@(#)49	1.9  src/bos/usr/ccs/lib/libc/DStemp.c, libcio, bos411, 9435C411a 9/1/94 16:51:10";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: __ito6, __pidnid, __lucky
 *
 * ORIGINS: 3,27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _THREAD_SAFE
#include <stdlib.h>
#include <lib_lock.h>
extern lib_lock_functions_t	_libc_lock_funcs;
#endif
#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/times.h>

/*
 * These routines support the Distributed Services modifications
 * to tmpnam(3), tempnam(3), and mktemp(3).
 */

/*
 * __ito6 --	Converts integers to strings of characters in a 6-bit
 *		character set (in which all characters are printable).
 *
 *		Called by __pidnid() and __lucky().
 */
char *
__ito6(num, len)
register unsigned long num;
int len;	/* length of string desired */
{
#define	ABC "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789__"
	register int i;
	static char buf[7];

	buf[6] = '\0';
	for (i = 5; i >= (6 - len); i--)
	{
		buf[i] = ABC[num & 0x3f];
		num >>= 6;
	}
	return(&buf[i+1]);
}

/*
 * __pidnid --	Encodes the process' pid, nid, and an incrementing
 *		selector as a 10-character, null-terminated string of
 *		printable characters.
 *
 *		Called by tmpnam(3) and tempnam(3).
 */
char *
__pidnid()
{
	struct xutsname x;
	static char buf[11];
	static unsigned int selector = 0;

	/*
	 * The characters in the encoded string are designated
	 * as follows:
	 *	pid (16 bits) =>	3 6-bit chars (18 bits)
	 *	nid (low 24 bits) =>	4 6-bit chars (24 bits)
	 *	selector (arbitrary) =>	3 6-bit chars (18 bits)
	 */
#ifdef _THREAD_SAFE
	strcpy(buf, __ito6((int)lib_thread_id(_libc_lock_funcs), 3));
#else /* _THREAD_SAFE */
	strcpy(buf, __ito6(getpid(), 3));
#endif /* _THREAD_SAFE */
	unamex(&x);
	strcat(buf, __ito6(x.nid & 0xffffff, 4));
	strcat(buf, __ito6(selector++, 3));
	buf[10] = '\0';
	selector &= 0x3fff;
	return(buf);
}

/*
 * __lucky --	Encodes the process' pid, a pseudo-random number,
 *		and an incrementing selector as a 6-character, null-
 *		terminated string of printable characters.
 *
 *		Called by mktemp(3).
 */
char *
__lucky()
{
#ifdef _THREAD_SAFE
	DRAND48D dp;
#endif
	unsigned long random;
	unsigned short seed[3];
	unsigned long mung;
	struct tms tms;
	static char buf[7];
	static unsigned int selector = 0;

	/*
	 * 36 bits are used to create a 6-character suffix, according
	 * to the definitions below.
	 */
#define PIDBITS		16	/* from user's pid */
#define	SELBITS		6	/* from static selector */
#define	RANDBITS	14	/* from pseudo-random number */

#define	FRONTBITS	30
#define	BACKBITS	(36 - FRONTBITS)
#define	PIDSHIFT	(FRONTBITS - PIDBITS)
#define	SELSHIFT	(PIDSHIFT - SELBITS)
#define	RANDSHIFT	(RANDBITS - SELSHIFT)
#define	SELMASK		((1 << SELBITS) - 1)
#define RANDMASK	((1 << RANDBITS) - 1)
#define	BACKMASK	((1 << BACKBITS) - 1)

	/*
	 * Seed the random number generator with this process' user
	 * and system times, and use the top of the result for the
	 * random bit field.
	 */
	times(&tms);
	seed[0] = tms.tms_utime >> 16;
	seed[1] = tms.tms_utime;
	seed[2] = tms.tms_stime;
#ifdef _THREAD_SAFE
	jrand48_r(seed, &dp, &random);
#else
	random = jrand48(seed);
#endif
	random = (random >> (32 - RANDBITS)) & RANDMASK;

	/*
	 * Since we can't call __ito6() with a 36-bit number, we
	 * call it twice, once with FRONTBITS bits and once with 
	 * BACKBITS.
	 */
#ifdef _THREAD_SAFE
	mung = (unsigned long)lib_thread_id(_libc_lock_funcs) <<PIDSHIFT;
#else /* _THREAD_SAFE */
	mung = getpid() << PIDSHIFT;
#endif /* _THREAD_SAFE */
	mung |= (selector++ & SELMASK) << SELSHIFT;
	mung |= random >> RANDSHIFT;
	strcpy(buf, __ito6(mung, FRONTBITS / 6));
	strcat(buf, __ito6(random & BACKMASK, BACKBITS / 6));

	buf[6] = '\0';
	selector &= SELMASK;

	return(buf);
}
