/* @(#)20       1.6  src/bos/usr/include/iconvTable.h, libiconv, bos411, 9428A410j 8/26/93 05:36:46
 *
 *   COMPONENT_NAME: CMDICONV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_ICONVTABLE_H
#define	_ICONVTABLE_H

/*
 *	This header file defines the convertion table structures, used by
 *	iconv function,  and also genxlt function that creates a compiled
 *	version of conversion table file.
 */

#define	ICONV_REL1_MAGIC	's'
#define	ICONV_REL2_MAGIC	'2'

typedef struct _IconvTable	{
	uchar_t		magic;
	uchar_t		inval_handle;
	uchar_t		inval_char;
	uchar_t		sub_handle;
	uchar_t		sub_char;
	uchar_t		sub_mark;
	uchar_t		dummy[26];
	uchar_t		data[256];
} IconvTable;

typedef struct _iconvTable_rec	{
	_LC_core_iconv_t	core;
	IconvTable		table;
} iconvTable_t;

#endif	/* _ICONVTABLE_H */
