/* @(#)15       1.4  src/bos/usr/include/ccsid.h, libiconv, bos411, 9428A410j 12/7/93 13:49:08
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
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _CCSID_H
#define _CCSID_H

#include <sys/types.h>
#include <sys/limits.h>
#include <iconv.h>


typedef	struct		__ccsid_data_rec {
	uchar_t		cs[_POSIX_NAME_MAX + 1];
	CCSID		ccsid;
} ccsid_data_rec, *ccsid_data_t;

typedef	struct		__ccsid_hdr_rec {
	int		version;
	uchar_t		reserve[20];
	uint_t		to_tbl_cnt;
	uint_t		from_tbl_cnt;
} ccsid_hdr_rec, *ccsid_hdr_t;

#endif	/*!_CCSID_H*/
