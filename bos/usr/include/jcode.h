/* @(#)21	1.4  src/bos/usr/include/jcode.h, libcnls, bos411, 9428A410j 6/16/90 00:10:47 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Library Support 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   jcode.h: definitions for jis/sj/uj conversion routines
 */

extern unsigned char
		*cjistosj(),	/* character conversion */
		*cjistouj(),
		*csjtojis(),
		*csjtouj(),
		*cujtojis(),
		*cujtosj(),

		*jistosj(),	/* string conversion */
		*jistouj(),
		*sjtojis(),
		*sjtouj(),
		*ujtojis(),
		*ujtosj();
