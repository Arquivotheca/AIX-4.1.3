/* @(#)63	1.8  src/bos/usr/include/IN/PFdefs.h, libIN, bos411, 9428A410j 6/16/90 00:17:30 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

struct  pwinfo
{       FILE *_ufile;
	char *_ufilename;
	int  _unargs;
	char **_uargs;
	int  _usbuf;
	char *_ubuf;
};
