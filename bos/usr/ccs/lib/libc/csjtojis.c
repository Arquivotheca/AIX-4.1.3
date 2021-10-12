static char sccsid[] = "@(#)07	1.2  src/bos/usr/ccs/lib/libc/csjtojis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:04";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: csjtojis
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
/*	csjtojis(s1, s2)						*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Shift-jis to jis conversion routine. Converts the double-byte	*/
/*	shift-jis character pointed to by s2 to a double-byte jis 	*/
/*	character pointed to by s1. Returns s1.				*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/

unsigned char *csjtojis(unsigned char *s1, unsigned char *s2)
{
	int c1, c2, p;

	c1 = s2[0];
	c2 = s2[1];
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
		s1[0] = ((c1 - 0x81) << 1) + 0x21 + p;
	else
		s1[0] = ((c1 - 0xe0) << 1) + 0x5f + p;
	s1[1] = c2;
	return s1;
}
