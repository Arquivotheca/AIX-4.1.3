/* @(#)17	1.2  src/bos/usr/include/hcs.h, libiconv, bos411, 9428A410j 6/11/91 00:19:16
 *
 * COMPONENT_NAME: (LIBICONV)
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

typedef struct _HostConv	HostConv;
typedef struct _HostConv	{
	char	*local;
	char	*host;
	char	*sbcs;
	char	*dbcs;
	unsigned char	*fcntl;
	unsigned char	*tcntl;
};

extern HostConv	_iconv_host[];
