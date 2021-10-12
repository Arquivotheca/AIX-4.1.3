/* @(#)71	1.8  src/bos/kernel/sys/POWER/ltypes.h, incstd, bos411, 9428A410j 6/15/90 17:48:13 */
#ifndef _H_LTYPES
#define _H_LTYPES
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ltypes.h	5.2 87/01/09 18:23:51 */
/*
 * quick set of typedefs -- this version is for the R2 platform
 */

typedef int int32;
typedef char int8;
typedef short int16;
typedef unsigned long uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;

#define hipart(X) (((uint32 *)&(X))[0])
#define lopart(X) (((uint32 *)&(X))[1])

#ifndef StoQDNaN
#define StoQDNaN(X)	hipart((X)) |= BIT19
#endif  /* StoQDNaN */

#ifndef StoQFNaN
#define StoQFNaN(X)	hipart((X)) |= BIT22
#endif  /* StoQFNaN */

#ifndef _QNaN
#define _QNaN
static uint32 FQNaN[] = {0x7fc00000};
static uint32 DQNaN[] = {0x7ff80000, 0x00000000};
#define _QNaN
#endif  /* _QNaN */

#define NORETBIT        0x08

#define _DOUBLE(x) (*(DOUBLE *)&(x))

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif	/* MIN */
#ifndef MAX
#define MAX(X, Y) ((X) < (Y) ? (Y) : (X))
#endif	/* MAX */

#endif  /* _H_LTYPES */
