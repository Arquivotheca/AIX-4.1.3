/* @(#)48	1.2.1.4  src/bos/usr/ccs/lib/libc/patlocal.h, libcpat, bos411, 9428A410j 4/6/94 21:54:41 */
#ifndef __H_PATLOCAL
#define __H_PATLOCAL
/*
 * COMPONENT_NAME: (LIBCPAT) Internal Regular Expression
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/localedef.h>


/************************************************************************/
/*									*/
/* Function prototypes							*/
/*									*/
/************************************************************************/

extern	int	_mbce_lower(char *, size_t, char *);

extern	wchar_t	_mbucoll(_LC_collate_objhdl_t, char *, char **);

	wchar_t *__wccollwgt();
	wchar_t __wcprimcollwgt();
	wchar_t __wcuniqcollwgt();
	wchar_t *__mbuniqcdwgt();

/************************************************************************/
/*									*/
/* Macros to determine collation weights				*/
/*									*/
/************************************************************************/

#define MAX_PC		__OBJ_DATA(hdl)->co_wc_max /* max process code	*/

#define MIN_PC		__OBJ_DATA(hdl)->co_wc_min /* min process code	*/

#define MAX_UCOLL	__OBJ_DATA(hdl)->co_col_max /* max unique weight	*/

#define MIN_UCOLL	__OBJ_DATA(hdl)->co_col_min  /* min unique weight	*/

#define	MAX_NORDS	__OBJ_DATA(hdl)->co_nord /* # collation weights	*/

#define CLASS_SIZE	32			/* [: :] max length	*/

#define FAST_LOCALE	(!(__OBJ_DATA(hdl)->co_subs || /* fast path locale? */ \
			   (__OBJ_DATA(hdl)->co_special & _LOC_HAS_MCCE))) 


#define __wccollwgt(wc)		/* address of weight table */		\
	( __OBJ_DATA(hdl)->co_coltbl[wc].ct_wgt.n )

/* IMPORTANT usage note about the  __wccollwgt() macro:         */
/* if (*(pwgt=__wccollwgt(wc))== _SUB_STRING)                   */
/*    caller must use _getcolval(...) to get collating weights! */

#define __wcprimcollwgt(wc)	/* primary weight for process code */	\
	( *__wccollwgt(wc) )

/* IMPORTANT usage note about the  __wcprimcollwgt() macro:     */
/* if (__wcprimcollwgt(wc))== _SUB_STRING)                      */
/*    caller must use _getcolval(...) for order zero to         */
/*    get the primary collating weight!                         */

#define __wcuniqcollwgt(wc)	/* unique collating weight */		\
	( __wccollwgt(wc)[_UCW_ORDER] )

/* usage note about the  __wcuniqcollwgt() macro:                            */
/*    unlike __wcprimcollwgt() this __wcuniqcollwgt can be used at any time. */
/*    HOWEVER, it will not handle multi characte collating elements,         */
/*    for that you must use _mbucoll()                                       */

#endif /* __H_PATLOCAL */
