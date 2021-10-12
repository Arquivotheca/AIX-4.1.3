static char sccsid[] = "@(#)36	1.3  src/bos/usr/ccs/lib/libc/tojkata.c, libcnls, bos411, 9428A410j 6/9/91 17:14:39";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: to_jkata, _tojkata
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
/*	tojkata(c)							*/
/*	register c;							*/
/*									*/
/*	int								*/
/*	_tojkata(c)							*/
/*	int c;								*/
/*									*/
/*  DESCRIPTION								*/
/*	Returns the katakana equivalent of a katagana character. The	*/
/*	unchecked form, _tojkata, does not check for a valid katagana	*/
/*	character. 							*/
/*									*/
/*  DIAGNOSTICS								*/
/*	For the checked form tojkata, the input value will be returned	*/
/*	unchanged if the conversion cannot be made.			*/
/*									*/
/************************************************************************/
#include <ctype.h>
#include <sys/limits.h>
#include <stdlib.h>

int tojkata(int c)	/* checked form */
{
	int _tojkata(int c);

	/* check for valid kata */
	if (isjhira(c))
		return _tojkata(c);
	else
		return c;
}

int _tojkata(int c)	/* unchecked form */
{
	unsigned char str[MB_LEN_MAX];
	wchar_t  wc;

	/**********
	  return c if an invalid process code
	**********/
	if (wctomb((char *)str, (wchar_t)c) == -1)
		return(c);

	/*********
	  EUC
	**********/
	if ((MB_CUR_MAX == 3) && (str[0] == 0xa4))
		str[0] = 0xa5;
	
	/********
	  IBM-932
	**********/
	else if ((MB_CUR_MAX == 2) && (str[0] == 0x82)) {
		if (str[1] >= 0x7f)
			str[1]++;
		str[0] = 0x83;
		str[1] -= 0x5f;
	}
	/**********
	  not a recognized codeset
	**********/
	else
		return(c);


	/**********
	  convert the characters back to process code
	**********/
	if (mbtowc(&wc, str, MB_CUR_MAX) == -1)
		return(c);

	return((int)wc);
}
