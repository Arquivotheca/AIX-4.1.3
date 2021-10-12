/* @(#)58	1.3  src/bos/kernel/sys/vlimit.h, libcsys, bos411, 9428A410j 7/12/90 15:11:58 */
#ifndef _H_VLIMITS
#define _H_VLIMITS
/*
 * COMPONENT_NAME: (LIBCSYS) Standard C Library System Functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* LIM_NORAISE is not emulated */
#define	LIM_NORAISE	0	/* if <> 0, can't raise limits */
#define	LIM_CPU		1	/* max secs cpu time */
#define	LIM_FSIZE	2	/* max size of file created */
#define	LIM_DATA	3	/* max growth of data space */
#define	LIM_STACK	4	/* max growth of stack */
#define	LIM_CORE	5	/* max size of ``core'' file */
#define	LIM_MAXRSS	6	/* max desired data+stack core usage */

#define	NLIMITS		6

#define INFINITY        0x7fffffff

#endif /* _H_VLIMIT */

