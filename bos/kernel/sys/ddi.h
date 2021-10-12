/* @(#)33     1.3  src/bos/kernel/sys/ddi.h, sysxpse, bos411, 9428A410j 3/11/94 08:10:51 */
/*
 *   COMPONENT_NAME: SYSXPSE ddi.h
 *
 *   ORIGINS: 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1991  Mentat Inc.
 */

#ifndef _DDI_
#define _DDI_

#undef datamsg
extern	int	datamsg(   int type   );

#undef enableok
extern	void	enableok(   queue_t * q   );

#undef noenable
extern	void	noenable(   queue_t * q   );

#undef OTHERQ
extern	queue_t	* OTHERQ(   queue_t * q   );

#ifdef puthere
#undef puthere
extern	void	puthere(   queue_t * q, mblk_t * mp   );
#endif

#ifdef putnext
#undef putnext
extern	void	putnext(   queue_t * q, mblk_t * mp   );
#endif

#undef RD
extern	queue_t	* RD(   queue_t * q   );

#undef WR
extern	queue_t	* WR(   queue_t * q   );

#endif
