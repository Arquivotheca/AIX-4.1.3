/* @(#)07       1.1  src/bos/usr/lib/nls/loc/uconv/UCSTBL.h, cmdiconv, bos411, 9428A410j 11/2/93 10:31:21
 *
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _ICONV_UCSTBL_H
#define _ICONV_UCSTBL_H

#include <sys/types.h>
#include <uconv.h>
#include "uc_convP.h"

typedef struct endian_rec {
	int		source;
	int		target;
} endian_t;

typedef struct subchar_rec {
	uchar_t		val[STEM_MAX+4];
	int		len;
} subchar_t;

typedef struct _LC_ucs_iconv_rec {
	struct _UconvObject	object;
	endian_t		endian;
} _LC_ucs_iconv_t;

#define	ENDIAN_SYSTEM	0
#define	ENDIAN_BIG	0xfeff
#define	ENDIAN_LITTLE	0xfffe

#endif /*_ICONV_UCSTBL_H*/
