/* @(#)33	1.10  src/bos/kernel/sys/POWER/m_param.h, sysproc, bos411, 9428A410j 4/7/94 15:43:02 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: LOADWCS
 *		STORWCS
 *
 *   ORIGINS: 26, 3, 9, 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_M_PARAM
#define _H_M_PARAM

#define NSRS  16
#define NGPRS 32
#define NFPRS 32

/* routines for loading and storing words in faultable critical sections */
#define	LOADWCS(x)	(* (volatile int *) (x))
#define STORWCS(x,y)	(*(y) = (x))

#define	HZ	100		/* ticks per second of the clock	*/
#define	hz	HZ		/* Berkeley uses lower case hz		*/
#define CLKTICK 20408		/* microseconds in a clock tick (49 HZ)	*/

#define MAXSEG	(64*1024)	/* max seg size (in clicks)		*/
/* default max data seg size is linear fn of avail pg space at boot time*/
/*
 * Virtual memory related constants, all in bytes, on page boundries
 */
#define	MAXTSIZ		(256*256*4096)		/* max text size */
#define	DFLDSIZ		(128*256*4096)		/* initial data size limit */
#define	MAXDSIZ		(256*256*4096)		/* max data size */
#define	DFLSSIZ		( 16*256*4096)		/* initial stack size limit */
#define	MAXSSIZ		(256*256*4096)		/* max stack size */

#endif /*_H_M_PARAM*/
