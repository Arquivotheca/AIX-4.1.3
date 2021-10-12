static char sccsid[] = "@(#)35	1.2  src/bos/usr/ccs/lib/libc/tojhira.c, libcnls, bos411, 9428A410j 6/8/91 15:59:34";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: to_jhira, _tojhira
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/************************************************************************/
/*									*/
/*  SYNOPSIS								*/
/*	int								*/
/*	tojhira(c)							*/
/*	register c;							*/
/*									*/
/*	int								*/
/*	_tojhira(c)							*/
/*	int c;								*/
/*									*/
/*  DESCRIPTION								*/
/*	Returns the hiragana equivalent of a katakana character. The	*/
/*	unchecked form, _tojhira, does not check for a valid katakana	*/
/*	character. 							*/
/*									*/
/*  DIAGNOSTICS								*/
/*	For the checked form tojhira, the input value will be returned	*/
/*	unchanged if the conversion cannot be made.			*/
/*									*/
/************************************************************************/
#include <ctype.h>
#include <sys/limits.h>
#include <stdlib.h>

int tojhira(int c)	/* checked form */
{
	int _tojhira(int c);

	/* check for valid katakana with hiragana equivalent */
	if (isjkata(c))
		return _tojhira(c);
	else
		return c;
}

int _tojhira(int c)	/* unchecked form */
{
	unsigned char str[MB_LEN_MAX];
	wchar_t  wc;


	/**********
	  return c if an invalid process code
	**********/
	if (wctomb(str, (wchar_t)c) == -1)
		return(c);

	/*********
	  EUC
	**********/
	if ((MB_CUR_MAX == 3) && (str[0] == 0xa5))
		str[0] = 0xa4;
	
	/********
	  IBM-932
	**********/
	else if ((MB_CUR_MAX == 2) && (str[0] == 0x83)) {
		if (str[1] > 0x7f)
			str[1]--;
		str[0] = 0x82;
		str[1] += 0x5f;
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
