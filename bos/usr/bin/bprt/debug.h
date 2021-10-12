/* @(#)20	1.1  src/bos/usr/bin/bprt/debug.h, libbidi, bos411, 9428A410j 8/27/93 09:56:55 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: TRACE
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
#define TRACE(fname,format, ch) {FILE * stream; \
                          if (debug) { stream = fopen (fname, "a+"); \
                          fprintf (stream, format, ch);\
                          fclose (stream);}}

