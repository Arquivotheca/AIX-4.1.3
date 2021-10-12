/* @(#)10	1.2  src/bos/usr/lib/nls/loc/iconv/misc/Universal_UCS_Conv.h, cmdiconv, bos411, 9428A410j 11/5/93 03:37:36
 *
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		none
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

#ifndef _Universal_UCS_Conv_h
#define _Universal_UCS_Conv_h

#include <sys/types.h>

typedef struct _LC_UnivUCSConv_iconv_rec {

	_LC_core_iconv_t	core;

	iconv_t		cd_from_ucs;
	iconv_t		cd_to_ucs;
	uchar_t		*ucs_buf_top;	/* Interchange buffer top     */
	int		ucs_buf_size;	/* Interchange buffer size    */
	uchar_t		*ucs_buf;	/* Interchange buffer         */
	size_t		ucs_left;	/* # of bytes left in ucs_buf */

} _LC_UnivUCSConv_iconv_t;

#define DEF_UCS_BUFSIZE		4096	/* Default size for ucs_buf   */

#endif /*!_Universal_UCS_Conv_h*/
