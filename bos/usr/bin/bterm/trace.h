/* @(#)93	1.1  src/bos/usr/bin/bterm/trace.h, libbidi, bos411, 9428A410j 8/26/93 13:35:32 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: TRACE
 *		TRACE2
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _TRACE_H
#define _TRACE_H 1

extern int debug;

#ifdef DEBUG

#define	TRACE(f)	{ if (debug) { debug_setlocation(__FILE__,__LINE__); debug_print f ; } }
#define	TRACE2(f)	{ if (debug) { debug_print f ; } }

#else DEBUG

#define TRACE(f)
#define	TRACE2(f)

#endif DEBUG

#endif _TRACE_H

