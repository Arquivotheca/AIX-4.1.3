static char sccsid[] = "@(#)32	1.2  src/bos/usr/ccs/lib/libc/kutentojis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:28";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: kutentojis
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
/************************************************************************/
/*									*/
/*  SYNOPSIS								*/
/*	int								*/
/*	kutentojis(c)							*/
/*	int c;								*/
/*									*/
/*  DESCRIPTION								*/
/*	Convert a kuten code to corresponding jis code. Kuten code 	*/
/*	range is 0 < code < 121.					*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Returns 0 for invalid	*/
/*	kuten codes.							*/
/*									*/
/************************************************************************/
int kutentojis(int c)
{
	int	ku;			/* number of 'ku' */

	if (0 < (ku = c / 100) && ku < 121 && 0 < (c %= 100) && c < 95)
		c += ((ku + 0x20) << 8) + 0x20;
	else				/* invalid kuten code */
		c = 0;
	return c;
}
