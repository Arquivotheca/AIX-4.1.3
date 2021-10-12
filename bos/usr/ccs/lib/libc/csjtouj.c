static char sccsid[] = "@(#)08	1.2  src/bos/usr/ccs/lib/libc/csjtouj.c, libcnls, bos411, 9428A410j 6/8/91 15:59:06";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: csjtouj
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
/*	csjtouj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Shift-jis to UNIX-jis conversion routine. Converts the double-	*/
/*	byte shift-jis character pointed to by s2 to a double-byte 	*/
/*	UNIX-jis character pointed to by s1. Returns s1.		*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/

unsigned char *csjtouj(unsigned char *s1, unsigned char *s2)
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
		s1[0] = (((c1 - 0x81) << 1) + 0x21 + p) | 0x80;
	else
		s1[0] = (((c1 - 0xe0) << 1) + 0x5f + p) | 0x80;
	s1[1] = c2 | 0x80;
	return s1;
}
