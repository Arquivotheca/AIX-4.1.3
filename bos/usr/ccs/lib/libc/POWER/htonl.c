#ifndef lint
static char sccsid[] = "@(#)37	1.3  src/bos/usr/ccs/lib/libc/POWER/htonl.c, libcnet, bos411, 9428A410j 4/25/91 13:39:08";
#endif
/*
 * COMPONENT_NAME: LIBCNET htonl.c
 *
 * FUNCTIONS:  htonl
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
 * htonl.c - host to net long for IBM RT PC 032 CPU
 */

long
htonl( l )
long l;
{
	return( l );
}
