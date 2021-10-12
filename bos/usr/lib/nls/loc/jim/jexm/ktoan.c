static char sccsid[] = "@(#)94	1.2  src/bos/usr/lib/nls/loc/jim/jexm/ktoan.c, libKJI, bos411, 9428A410j 6/6/91 11:21:24";

/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.3		11/11/88
 *	ktoan()		Katakana to AlphaNumeric conversion.
 */

static	char	ktoantbl[] = {
	0xa0, '>',  '{',  '}',  '<',  '?',  '~',  '#',		/* 0xa0 - 0xa7	*/
	'E',  '$',  '%',  '&',  '\'', '(',  ')',  'Z',		/* 0xa8 - 0xaf	*/
	'\\', '3',  'e',  '4',  '5',  '6',  't',  'g',		/* 0xb0 - 0xb7	*/
	'h',  ':',  'b',  'x',  'd',  'r',  'p',  'c',		/* 0xb8 - 0xbf	*/
	'q',  'a',  'z',  'w',  's',  'u',  'i',  '1',		/* 0xc0 - 0xc7	*/
	',',  'k',  'f',  'v',  '2',  '^',  '-',  'j',		/* 0xc8 - 0xcf	*/
	'n',  ']',  '/',  'm',  '7',  '8',  '9',  'o',		/* 0xd0 - 0xd7	*/
	'l',  '.',  ';',  '\\', '0',  'y',  '@',  '[',		/* 0xd8 - 0xdf	*/
};

char	ktoan(kana)
char	kana;
{
	if(kana < 0xa0 || 0xdf < kana)
		return	kana;
	return ktoantbl[kana - 0xa0];
}
