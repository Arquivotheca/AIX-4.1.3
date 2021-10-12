/* @(#)50	1.9  src/bos/kernel/sys/sysmacros.h, sysios, bos411, 9428A410j 7/26/93 16:01:27 */
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystems
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SYSMACROS
#define _H_SYSMACROS

#include <sys/param.h>

/*
 * Some macros for units conversion
 */

/* Core clicks to segments and vice versa */
#if NCPS == 1
#define ctos(x) (x)
#define stoc(x) (x)
#else
#define ctos(x) (((x)+(NCPS-1))/NCPS)
#define	stoc(x) ((x)*NCPS)
#endif

/* Core clicks to disk blocks */
#if NCPD == 1
#define ctod(x) (x)
#else
#define ctod(x) (((x)+(NCPD-1))/NCPD)
#endif
#define bltoc(x) ((x)*NCPD)

/* inumber to disk address */
#ifdef INOSHIFT
#define itod(x) (daddr_t)(((unsigned)(x)+(2*INOPB-1))>>INOSHIFT)
#else
#define itod(x) (daddr_t)(((unsigned)(x)+(2*INOPB-1))/INOPB)
#endif

/* inumber to disk offset */
#ifdef INOSHIFT
#define itoo(x) (int)(((unsigned)(x)+(2*INOPB-1))&(INOPB-1))
#else
#define itoo(x) (int)(((unsigned)(x)+(2*INOPB-1))%INOPB)
#endif

/* clicks to bytes */
#ifdef BPCSHIFT
#define	ctob(x)	((x)<<BPCSHIFT)
#else
#define	ctob(x)	((x)*NBPC)
#endif

/* bytes to clicks */
#ifdef BPCSHIFT
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))>>BPCSHIFT)
#define	btoct(x)	((unsigned)(x)>>BPCSHIFT)
#else
#define	btoc(x)	(((unsigned)(x)+(NBPC-1))/NBPC)
#define	btoct(x)	((unsigned)(x)/NBPC)
#endif

/*
 * Conversion from bytes to reporting block units, and from stat() block
 * units to reporting block units (and back).  Reporting block units are
 * used as the user interface unit for setting and getting file sizes.
 * These macros assume that the number of bytes represented in blocks
 * never exceeds the maximum file offset; i.e. LONG_MAX.  The macros also
 * assume that a positive value is being converted.
 */
#define	bytes2rblocks(x)	(((unsigned long)(x)+(1024-1))/1024)
#define	rblocks2bytes(x)	((unsigned long)(x)*1024)
#define	stblocks2rblocks(x)	(((unsigned long)(x)*UBSIZE+(1024-1))/1024)
#define	rblocks2stblocks(x)	(((unsigned long)(x)*1024+(UBSIZE-1))/UBSIZE)

/* The major(), minor(), and makedev() macros also exist in types.h for
   BSD compliance.  Any changes to these macros must also be made in
   types.h.
*/

/* major part of a device */
#define major(__x)        (int)((unsigned)(__x)>>16)
#define	bmajor(x)	(int)(((unsigned)x>>16) & 0x7FFF)
#define	brdev(x)	(x&0x7fffffff)

/* minor part of a device */
#define minor(__x)        (int)((__x)&0xFFFF)

/* make a device number */
#define	makedev(__x,__y)	(dev_t)(((__x)<<16) | (__y))

#endif/* _H_SYSMACROS */
