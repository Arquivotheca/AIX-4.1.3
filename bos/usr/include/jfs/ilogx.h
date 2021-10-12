/* @(#)26	1.1  src/bos/usr/include/jfs/ilogx.h, syspfs, bos41J, 145887.bos 3/3/95 09:30:36 */

#ifndef _H_JFS_ILOGX
#define _H_JFS_ILOGX
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <vmm/vmlock.h>

/*	log inode extended information
 */
struct ilogx {
	Simple_lock	_logxlock;	/* log extension lock 		*/
	uint		_logflag;	/* flags			*/
	int		_logppong;	/* alternating log page writes  */

	struct {
		struct tblock	*head;	/* FIFO commit queue header	*/
		struct tblock	*tail;
	} _logcq;

	int		_logcsn;	/* commit sequence number 	*/
	int		_logcrtc;	/* GC_READY transaction count 	*/
	struct tblock  *_loglcrt;	/* latest GC_READY transaction 	*/

	/* End Of Page Marker (EOPM) control 
	 */
	int		_logeopm;	/* next EOPM in eopm queue    */
	int		_logeopmc;	/* EOPM count in commit_queue */
	struct tblock	_logeopmq[2];	/* EOPM tblock queue	      */
};

#define i_logxlock	i_logx->_logxlock
#define i_logflag	i_logx->_logflag
#define i_logppong	i_logx->_logppong
#define i_logcq		i_logx->_logcq
#define i_logcsn	i_logx->_logcsn
#define i_logcqtc	i_logx->_logcqtc
#define i_logcrtc	i_logx->_logcrtc
#define i_loglcrt	i_logx->_loglcrt
#define i_logeopm	i_logx->_logeopm
#define i_logeopmc	i_logx->_logeopmc
#define i_logeopmq	i_logx->_logeopmq


/* i_logxflag 
 */
#define LOGX_GCPAGEOUT	0x00000001   /* pageout is in progress */

#define LOGX_LOCK(ip)            simple_lock(&((ip)->i_logxlock))
#define LOGX_UNLOCK(ip)          simple_unlock(&((ip)->i_logxlock))

#endif /* ILOGX */
