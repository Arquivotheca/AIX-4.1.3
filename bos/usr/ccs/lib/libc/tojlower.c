static char sccsid[] = "@(#)37	1.1  src/bos/usr/ccs/lib/libc/tojlower.c, libcnls, bos411, 9428A410j 2/26/91 17:43:14";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: tojlower
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
/*	tojlower (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	tojlower returns the Shift-JIS lowercase equivalent of a      */
/*	Shift-JIS uppercase letter.  The input should be the	      */
/*	Shift-JIS form of an uppercase ASCII letter.		      */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character is not the Shift-JIS form of an	      */
/*	uppercase ASCII letter, the function returns the input value. */
/*								      */
/**********************************************************************/


#include <ctype.h>
#include <NLctype.h>

int tojlower (int c)
{
	return( towlower(c) );
}
