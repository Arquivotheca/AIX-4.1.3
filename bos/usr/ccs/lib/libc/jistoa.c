static char sccsid[] = "@(#)29	1.3  src/bos/usr/ccs/lib/libc/jistoa.c, libcnls, bos411, 9428A410j 8/20/92 08:45:13";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: jistoa
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
/*	jistoa (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	jistoa returns the ASCII equivalent of a Shift-JIS character. */
/*								      */
/* DIAGNOSTICS							      */
/*	If the input character does not have an ASCII equivalent,     */
/*	the function returns the input value.			      */
/*								      */
/**********************************************************************/

#include <stdlib.h>	/* MB_CUR_MAX */
#include <sys/limits.h>
#include <ctype.h>

extern int _jistoa_932(int c);
extern int _jistoa_eucjp(int c);

/**********
 conversion table.
**********/
static unsigned char __tbl[84] = {
/*  Character         ,    .         :    ;    ?    !                         */
/*  IBM-932          43   44   45   46   47   48   49   4a   4b   4c   4d   4e*/
/*  IBM-eucJP        a4   a5   a6   a7   a8   a9   aa   ab   ac   ad   ae   af*/
                    0x2c,0x2e,0x00,0x3a,0x3b,0x3f,0x21,0x00,0x00,0x00,0x00,0x00,

/* ^        _                                                           -   / */
/*4f  50   51   52   53   54   55   56   57   58   59   5a   5b   5c   5d   5e*/
/*b0  b1   b2   b3   b4   b5   b6   b7   b8   b9   ba   bb   bc   bd   be   bf*/
0x5e,0x00,0x5f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2d,0x2f,

/* \   ~         |              '              "    (    )              [   ] */
/*5f  60   61   62   63   64   65   66   67   68   69   6a   6b   6c   6d   6e*/
/*c0  c1   c2   c3   c4   c5   c6   c7   c8   c9   ca   cb   cc   cd   ce   cf*/
0x5c,0x7e,0x00,0x7c,0x00,0x00,0x27,0x00,0x00,0x22,0x28,0x29,0x00,0x00,0x5b,0x5d,

/* {   }    <    >                                            +               */
/*6f  70   71   72   73   74   75   76   77   78   79   7a   7b   7c   7d   7e*/
/*d0  d1   d2   d3   d4   d5   d6   d7   d8   d9   da   db   dc   dd   de   df*/
0x7b,0x7d,0x3c,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2b,0x00,0x00,0x00,

/*     =                                                      `               */
/*80  81   82   83   84   85   86   87   88   89   8a   8b   8c   8d   8e   8f*/
/*e0  e1   e2   e3   e4   e5   e6   e7   e8   e9   ea   eb   ec   ed   ee   ef*/
0x00,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,

/* $             %    #    &    *    @ */
/*90  91   92   93   94   95   96   97 */
/*f0  f1   f2   f3   f4   f5   f6   f7 */
0x24,0x00,0x00,0x25,0x23,0x26,0x2a,0x40 };

union conv_t {
	int c;
	char s[2+MB_LEN_MAX];
};

int jistoa (register c)
{
	if (MB_CUR_MAX == 1)
		return(c);
	else if (MB_CUR_MAX == 2) 
		return (_jistoa_932(c));
	else if (MB_CUR_MAX == 3)
		return (_jistoa_eucjp(c));

	return(c);
}

int _jistoa_eucjp(int c)
{
	union conv_t conv; 
	int rc;

	/**********
	  convert it to process file code
	  if invalid or more than 2 , return c
	**********/
	conv.c = 0;
	if (((rc = wctomb(&conv.s[2], (wchar_t)c)) == -1) || (rc > 2))
		return(c);
	
	/**********
	  out of ascii conversion range
	**********/
	if (conv.c <0xa1a4 || conv.c>0xa3fa)
		return(c);

	/**********
	  is this value in the table
	**********/
	if (conv.c <=0xa1f7)
		conv.c = __tbl[conv.c-0xa1a4];

	/***********
	 '0' - '9' 
	***********/
	else if (conv.c >=0xa3b0 && conv.c <= 0xa3b9)
		conv.c -= 0xa380;

	/**********
	  'A' - 'Z'
	**********/
	else if (conv.c >=0xa3c1 && conv.c <= 0xa3da)
		conv.c -= 0xa380;

	/**********
	  'a' - 'z'
	**********/
	else if (conv.c >=0xa3e1)
		conv.c -= 0xa380;
	
	/**********
	  no ascii sub
	**********/
	else
		return(c);

	/**********
	  if the entry in the table was 0, return c
	**********/
	if (conv.c == 0)
		return(c);

	/**********
	  in ascii, process code == file code, just return conv.c
	**********/
	return(conv.c);
}

int _jistoa_932(int c)
{
	union conv_t conv; 
	int rc;

	/**********
	  convert it to process file code
	  if invalid or more than 2 , return c
	**********/
	conv.c = 0;
	if (((rc = wctomb(&conv.s[2], (wchar_t)c)) == -1) || (rc > 2))
		return(c);
	
	/**********
	  outside the range of conversion
	**********/
	if (conv.c <0x8143 || conv.c>0x829a)
		return(c);

	/**********
	  is this in the table
	**********/
	if (conv.c <=0x817F)
		conv.c = __tbl[conv.c-0x8143];
	/**********
	  Since 7F is skipped, subtract 1 from anything above it
	**********/
	else if (conv.c <=0x8197)
		conv.c = __tbl[conv.c-0x8143-1];

	/**********
	  '0' - '9'
	**********/
	else if (conv.c >=0x824f && conv.c <= 0x8258)
		conv.c -= 0x821f;

	/**********
	  'A' - 'Z'
	**********/
	else if (conv.c >=0x8260 && conv.c <= 0x8279)
		conv.c -= 0x821f;

	/**********
	  'a' - 'z'
	**********/
	else if (conv.c >=0x8281)
		conv.c -= 0x8220;
	
	/**********
	  no ascii sub
	**********/
	else
		return(c);

	/**********
	  if the entry in the table was 0, return c
	**********/
	if (conv.c == 0)
		return(c);

	/**********
	  since the process code for ascii == file code, return conv.c
	**********/
	return(conv.c);
}
