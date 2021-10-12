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

/*
 *	_LC_ucs_lower_iconv_t
 */
typedef struct _LC_ucs_lower_iconv_rec	{
	_LC_core_iconv_t	core;
	uchar_t			mask;        /* 
					      * 0 = default GL/GR, 
					      * 0x80 = toggle 
					      */
} _LC_ucs_lower_iconv_t;
