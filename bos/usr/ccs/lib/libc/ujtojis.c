static char sccsid[] = "@(#)40	1.2  src/bos/usr/ccs/lib/libc/ujtojis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:40";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library Conversion Functions
 *
 * FUNCTIONS: utojis
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
/*	ujtojis(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	UNIX-jis to jis string conversion routine. The input string    	*/
/*	s2, containing double-byte UNIX-jis characters, is converted 	*/
/*	to a string of double-byte jis characters, s1. Returns s1.	*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/
unsigned char *ujtojis(unsigned char *s1, unsigned char *s2)
{
	unsigned char *s0 = s1;

	while (*s2)
		*s1++ = *s2++ & 0x7f;		/* clear MSB */
	return s0;
}
