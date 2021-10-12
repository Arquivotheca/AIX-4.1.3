/* @(#)24	1.4  src/bos/usr/lib/nls/loc/iconv/host/host.h, cmdiconv, bos411, 9428A410j 8/19/91 21:45:30
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
 

#define SO (unsigned char) 0x0E      /* Shift Out, host code to turn DBCS on */
#define SI (unsigned char) 0x0F      /* Shift In, host code to turn DBCS off */

typedef struct _LC_host_iconv_rec {
	_LC_core_iconv_t	core;
	iconv_t		curcd;
	iconv_t		sb_cd;
	iconv_t		db_cd;
	unsigned char	*cntl;
} _LC_host_iconv_t;
