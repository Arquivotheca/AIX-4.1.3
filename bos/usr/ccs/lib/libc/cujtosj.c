static char sccsid[] = "@(#)10	1.2  src/bos/usr/ccs/lib/libc/cujtosj.c, libcnls, bos411, 9428A410j 6/8/91 15:59:10";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: cujtosj
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
/*	cujtosj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	UNIX-jis to shift-jis conversion routine. Converts the double-	*/
/*	byte UNIX-jis character pointed to by s2 to a double-byte 	*/
/*	shift-jis character pointed to by s1. Returns s1.		*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/
unsigned char * cujtosj(unsigned char *s1, unsigned char *s2)
{
	int c1, c2;

	c1 = (s2[0] - 1) & 0x7f;		/* clear MSB */
	c2 = s2[1] & 0x7f;			/* clear MSB */
	if (c1 < 0x5e)
		s1[0] = (c1 >> 1) + 0x71;
	else
		s1[0] = (c1 >> 1) + 0xb1;
	if (c1 & 1)
		s1[1] = c2 + 0x7e;
	else if (c2 <= 0x5f)
		s1[1] = c2 + 0x1f;
	else
		s1[1] = c2 + 0x20;
	return s1;
}
