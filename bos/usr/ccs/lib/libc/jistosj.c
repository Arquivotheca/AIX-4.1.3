static char sccsid[] = "@(#)30	1.2  src/bos/usr/ccs/lib/libc/jistosj.c, libcnls, bos411, 9428A410j 6/8/91 15:59:26";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: jistosj 
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
/*	unsigned char *							*/
/*	jistosj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Jis to shift-jis string conversion routine. The input string    */
/*	s2, containing double-byte jis characters, is converted to a 	*/
/*	string of double-byte shift-jis characters, s1. Returns s1.	*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/
unsigned char *jistosj(unsigned char *s1, unsigned char *s2)
{
	int c1, c2;
	unsigned char *s0 = s1;

	while (c1 = *s2++) {
		c1--;
		c2 = *s2++;
		if (c1 < 0x5e)
			*s1++ = (c1 >> 1) + 0x71;
		else
			*s1++ = (c1 >> 1) + 0xb1;
		if (c1 & 1)
			*s1++ = c2 + 0x7e;
		else if (c2 <= 0x5f)
			*s1++ = c2 + 0x1f;
		else
			*s1++ = c2 + 0x20;
	}
	return s0;
}
