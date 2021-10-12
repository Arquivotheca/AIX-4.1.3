/* @(#)85	1.5  src/bos/usr/include/unctrl.h, libcgen, bos411, 9428A410j 6/16/90 00:15:38 */
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
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
 * unctrl.h
 *
*/
#ifndef _H_UNCTRL
#define _H_UNCTRL

#ifndef unctrl
extern char	*_unctrl[];

# define	unctrl(ch)	(_unctrl[(unsigned) ch])
#endif

#endif /* _H_UNCTRL */
