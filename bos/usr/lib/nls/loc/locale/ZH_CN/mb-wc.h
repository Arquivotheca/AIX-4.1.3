/* @(#)41	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/mb-wc.h, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:43  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
typedef
	struct
	{
	    int     cmask;
	    int	    cval;
	    int     shift;
	    int	    nbytes;
	    long    lmask;
	    long    lval;
	} Tab;

static
Tab	tab[] =
{
   0x80, 0x00, 0*6, 1, 0x7F,       0,	      /* 1 byte sequence */
   0xE0, 0xC0, 1*6, 2, 0x7FF,      0x80,      /* 2 byte sequence */
   0xF0, 0xE0, 2*6, 3, 0xFFFF,     0x800,     /* 3 byte sequence */
/* 0xF8, 0xF0, 3*6, 4, 0x1FFFFF,   0x10000,      4 byte sequence 
   0xFC, 0xF8, 4*6, 5, 0x3FFFFFF,  0x200000,     5 byte sequence 
   0xFE, 0xFC, 5*6, 6, 0x7FFFFFFF, 0x4000000,    6 byte sequence */
   0,					    /* end of table */
};


