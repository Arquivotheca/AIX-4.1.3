/* @(#)28	1.3  src/bos/usr/ccs/bin/ld/bind/util.h, cmdld, bos411, 9428A410j 1/28/94 11:34:19 */
#ifndef Binder_UTIL
#define Binder_UTIL
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
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

/* String routines */
extern char	*save_dotted_string(char *, int);
extern char	*save_string(char *, int);

/* Case conversion routines--strings are converted in place. */
extern void	lower(char *);
extern void	upper(char *);

/* Escape sequence handling routine */
extern char *	unescape_pathname(char *, int, char *);

/* heap routines */
extern CSECT	*deletemin(CSECT_HEAP);
extern ITEM	*find_item(ITEM	*, int);
extern void	makeheap(CSECT_HEAP);

#endif /* Binder_UTIL */
