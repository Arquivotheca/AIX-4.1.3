static char sccsid[] = "@(#)31	1.1  src/bos/usr/ccs/lib/libc/jistouj.c, libcnls, bos411, 9428A410j 2/26/91 17:42:51";

/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: jistouj
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
/*  NAME								*/
/*	jistouj								*/
/*									*/
/*  SYNOPSIS								*/
/*	unsigned char *							*/
/*	jistouj(s1, s2)							*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Jis to UNIX-jis string conversion routine. The input string    */
/*	s2, containing double-byte jis characters, is converted to a 	*/
/*	string of double-byte UNIX-jis characters, s1. Returns s1.	*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/

unsigned char *
jistouj(s1, s2)
unsigned char *s1, *s2;
{
	unsigned char *s0 = s1;

	while (*s2)
		*s1++ = *s2++ | 0x80;		/* set MSB */
	return	s0;
}
