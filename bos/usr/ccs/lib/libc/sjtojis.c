static char sccsid[] = "@(#)33	1.2  src/bos/usr/ccs/lib/libc/sjtojis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:30";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: sjtojis
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
/*	sjtojis(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Shift-jis to jis string conversion routine. The input string   	*/
/*	s2, containing double-byte shift-jis characters, is converted 	*/
/*	to a string of double-byte jis characters, s1. Returns s1.	*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/
unsigned char *sjtojis(unsigned char *s1, unsigned char *s2)
{
	int c1, c2, p;
	unsigned char *s0 = s1;

	while (c1 = *s2++) {
		c2 = *s2++;
		p = 0;
		if (c2 < 0x7f)
			c2 -= 0x1f;
		else if (c2 < 0x9f)
			c2 -= 0x20;
		else {
			c2 -= 0x7e;
			p++;
		}
		if (c1 < 0xa0)
			*s1++ = ((c1 - 0x81) << 1) + 0x21 + p;
		else
			*s1++ = ((c1 - 0xe0) << 1) + 0x5f + p;
		*s1++ = c2;
	}
	return s0;
}
