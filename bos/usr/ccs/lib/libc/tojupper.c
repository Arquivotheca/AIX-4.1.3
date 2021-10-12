static char sccsid[] = "@(#)38	1.1  src/bos/usr/ccs/lib/libc/tojupper.c, libcnls, bos411, 9428A410j 2/26/91 17:43:18";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: tojupper
 *
 * ORIGINS: 10
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
/**********************************************************************/
/*								      */
/* SYNOPSIS							      */
/*	int							      */
/*	tojupper (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	tojupper returns the Shift-JIS uppercase equivalent of a      */
/*	Shift-JIS lowercase letter.  The input should be the	      */
/*	Shift-JIS form of a lowercase ASCII letter.		      */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character is not the Shift-JIS form of a	      */
/*	lowercase ASCII letter, the function returns the input value. */
/*								      */
/**********************************************************************/
#include <ctype.h>
#include <NLctype.h>

int tojupper (int c)
{
	return ( towupper(c) );
}
