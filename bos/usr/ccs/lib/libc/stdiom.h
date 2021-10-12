/* @(#)11	1.8  src/bos/usr/ccs/lib/libc/stdiom.h, libcio, bos411, 9428A410j 2/26/91 13:41:26 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* The following macros improve performance of the stdio by reducing the
	number of calls to _bufsync and _wrtchk.  _BUFSYNC has the same
	effect as _bufsync, and _WRTCHK has the same effect as _wrtchk,
	but often these functions have no effect, and in those cases the
	macros avoid the expense of calling the functions.  */

extern int __forkid;

#define _BUFSYNC(iop)	if (_bufend(iop) - iop->_ptr <   \
				( iop->_cnt < 0 ? 0 : iop->_cnt ) )  \
					_bufsync(iop)
#define _WRTCHK(iop)	((((iop->_flag & (_IOWRT | _IOEOF)) != _IOWRT) \
				|| (iop->_base == NULL)  \
				|| (iop->_ptr == iop->_base && iop->_cnt == 0 \
					&& !(iop->_flag & (_IONBF | _IOLBF))) \
				|| ((iop->__stdioid != __forkid) \
					&& (iop->_flag & _IORW))) \
			? _wrtchk(iop) : 0 )

#define _FORKCMP(iop)	if( (iop->__stdioid != __forkid)  )	\
			    {    			\
				if(iop->_ptr != iop->_base ||    \
				  ( !(iop->_flag & _IOWRT) && iop->_cnt > 0)) \
				   iop->_flag |= _IONONSTD;		\
				iop->__stdioid = __forkid ;		\
			    }
