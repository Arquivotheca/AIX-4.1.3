/* @(#)32	1.4  src/bos/usr/lib/nls/loc/iconv/fold/fold.h, cmdiconv, bos411, 9428A410j 8/19/91 21:45:25
 *
 * COMPONENT_NAME: (CMDICONV)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct _LC_fold_iconv_rec {
	_LC_core_iconv_t	core;
	iconv_t	curcd;
	iconv_t	gl;
	iconv_t gr;
	iconv_t	defgl;
	iconv_t defgr;
	iconv_t	*cds;
	EscTblTbl	*ett;
	int	ncds;
} _LC_fold_iconv_t;
