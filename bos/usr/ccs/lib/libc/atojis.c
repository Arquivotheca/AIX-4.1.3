static char sccsid[] = "@(#)04	1.2  src/bos/usr/ccs/lib/libc/atojis.c, libcnls, bos411, 9428A410j 6/8/91 15:58:57";
/*
 *  COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: atojis
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

/**********************************************************************/
/*								      */
/* SYNOPSIS							      */
/*	int							      */
/*	atojis (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	atojis returns the Shift-JIS/EUC equivalent of an ASCII char. */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character does not have a Shift-JIS equivalent,  */
/*	the function returns the input value.			      */
/*								      */
/**********************************************************************/

#include <stdlib.h>	/* MB_CUR_MAX */
#include <ctype.h>

extern int _atojis_932(int c);
extern int _atojis_eucjp(int c);

union conv_t {
	int c;
	char s[4];
};

int atojis(register c)
{
	if (MB_CUR_MAX == 1)
		return(c);
	else if (MB_CUR_MAX == 2) 
		return (_atojis_932(c));
	else if (MB_CUR_MAX == 3)
		return (_atojis_eucjp(c));

	return(c);
}

int _atojis_eucjp(int c)
{
	wchar_t wc;

	unsigned short tbl[32] = {
		0xa1aa, 0xa1c9, 0xa1f4, 0xa1f0, /*    !  "  #  $    */
		0xa1f3, 0xa1f5, 0xa1c6, 0xa1ca, /*    %  &  '  (    */
		0xa1cb, 0xa1f6, 0xa1dc, 0xa1a4, /*    )  *  +  ,    */
		0xa1be, 0xa1a5, 0xa1bf,         /*    -  .  /       */
		0xa1a7, 0xa1a8, 0xa1d2, 0xa1e1, /*    :  ;  <  =    */
		0xa1d3, 0xa1a9, 0xa1f7,         /*    >  ?  @       */
		0xa1ce, 0xa1c0, 0xa1cf, 0xa1b0, /*    [  \  ]  ^    */
		0xa1b2, 0xa1ec,                 /*    _  `          */
		0xa1d0, 0xa1c3, 0xa1d1, 0xa1c1  /*    {  |  }  ~    */
	};

	union conv_t conv; 
	
	/**********
	  outside the range of conversion
	**********/
	if (c <0x21 || c>0xfe)
		return(c);

	/**********
	  table look up
	**********/
	if (c <=0x2f)
		conv.c = tbl[c-0x21];

	/**********
	  '0' - '9'
	**********/
	else if (c <= 0x39)
		conv.c = c + 0xa380;

	/**********
	  table look up
	**********/
	else if (c<=0x40)
		conv.c = tbl[c-0x2B];

	/**********
	  'A' - 'Z'
	**********/
	else if (c <= 0x5a)
		conv.c = c + 0xa380;

	/**********
	  table look up
	**********/
	else if (c<=0x60)
		conv.c = tbl[c-0x45];

	/**********
	  'a' - 'z'
	**********/
	else if (c <= 0x7a)
		conv.c = c + 0xa380;

	/**********
	  table look up
	**********/
	else 
		conv.c = tbl[c-0x5F];

	/**********
	  convert the character to process code and return
	**********/
	if (mbtowc(&wc, &conv.s[2], 2) == -1)
		return(c);

	return(wc);
}
		
int _atojis_932(int c)
{
	wchar_t wc;

	unsigned short tbl[32] = {
		0x8149, 0x8168, 0x8194, 0x8190, /*    !  "  #  $    */
		0x8193, 0x8195, 0x8165, 0x8169, /*    %  &  '  (    */
		0x816a, 0x8196, 0x817b, 0x8143, /*    )  *  +  ,    */
		0x815d, 0x8144, 0x815e,         /*    -  .  /       */
		0x8146, 0x8147, 0x8171, 0x8181, /*    :  ;  <  =    */
		0x8172, 0x8148, 0x8197,         /*    >  ?  @       */
		0x816d, 0x815f, 0x816e, 0x814f, /*    [  \  ]  ^    */
		0x8151, 0x818c,                 /*    _  `          */
		0x816f, 0x8162, 0x8170, 0x8160  /*    {  |  }  ~    */
	};

	union conv_t conv;
	
	/**********
	  is this outsides the bounds of conversion
	**********/
	if (c <0x21 || c>0xfe)
		return(c);

	/**********
	  table look up
	**********/
	if (c <=0x2f)
		conv.c = tbl[c-0x21];

	/**********
	  '0' - '9'
	**********/
	else if (c <= 0x39)
		conv.c = c + 0x821f;

	/**********
	  table look up
	**********/
	else if (c<=0x40)
		conv.c = tbl[c-0x2B];

	/**********
	  'A' - 'Z'
	**********/
	else if (c <= 0x5a)
		conv.c = c + 0x821f;

	/**********
	  table look up
	**********/
	else if (c<=0x60)
		conv.c = tbl[c-0x45];

	/**********
	  'a' - 'z'
	**********/
	else if (c <= 0x7a)
		conv.c = c + 0x8220;

	/**********
	  table look up
	**********/
	else 
		conv.c = tbl[c-0x5F];

	/**********
	  convert the character to process code and return
	**********/
	if (mbtowc(&wc, &conv.s[2], 2) == -1)
		return(c);

	return(wc);
}
		
