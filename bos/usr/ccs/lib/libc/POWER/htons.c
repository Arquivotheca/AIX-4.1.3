#ifndef lint
static char sccsid[] = "@(#)38	1.3  src/bos/usr/ccs/lib/libc/POWER/htons.c, libcnet, bos411, 9428A410j 4/25/91 13:39:23";
#endif
/*
 * COMPONENT_NAME: LIBCNET htons.c
 *
 * FUNCTIONS:  htons
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 /*
 * htons.c - host to net short for IBM RT PC 032 CPU
 */

short
htons( s )
short s;
{
	return( s );
}
