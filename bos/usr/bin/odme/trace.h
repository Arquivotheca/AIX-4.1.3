/* @(#)77	1.4  src/bos/usr/bin/odme/trace.h, cmdodm, bos411, 9428A410j 6/15/90 22:36:20 */

/*
 * COMPONENT_NAME: (ODME) TRACE.H - if tracing is enabled.
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*--------- private trace macro ----------*/

extern int trace;
extern int logger;

#ifdef DEBUG
#define TRC(a,b,c,d,e,f,g) if(trace)  trcprt(a,b,c,d,e,f,g);
#else
#define TRC(a,b,c,d,e,f,g)
#endif

#ifdef DEBUG
#define LOG(a,b); if(logger || trace) logprt(a,b);
#else
#define LOG(a,b)
#endif
