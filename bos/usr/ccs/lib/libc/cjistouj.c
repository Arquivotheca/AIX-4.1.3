static char sccsid[] = "@(#)06	1.2  src/bos/usr/ccs/lib/libc/cjistouj.c, libcnls, bos411, 9428A410j 6/8/91 15:59:02";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: cjistouj
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
/*	cjistouj(s1, s2)						*/
/*	unsigned char *s1, *s2;						*/
/*									*/
/*  DESCRIPTION								*/
/*	Jis to UNIX-jis conversion routine. Converts the double-byte	*/
/*	jis character pointed to by s2 to a double-byte UNIX-jis 	*/
/*	character pointed to by s1. Returns s1.				*/
/*									*/
/*  DIAGNOSTICS								*/
/*	No check for valid input is performed. Invalid input will 	*/
/*	result in undefined output.					*/
/*									*/
/************************************************************************/

#include <stdlib.h>	/* MB_CUR_MAX */

unsigned char *cjistouj(unsigned char *s1, unsigned char *s2)
{
	s1[0] = s2[0] | 0x80;			/* set MSB */
	s1[1] = s2[1] | 0x80;			/* set MSB */
	return	s1;
}
