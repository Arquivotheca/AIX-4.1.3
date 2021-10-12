/* @(#)60       1.3  src/bos/usr/include/pse/nd.h, sysxpse, bos411, 9428A410j 8/30/93 03:35:52 */
/*
 * COMPONENT_NAME: LIBCPSE
 * 
 * ORIGINS: 27 63 71 83 
 * 
 */
/*
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/** Copyright (c) 1988  Mentat Inc.
 ** nd.c 1.5, last change 1/2/90
 **/

#ifndef _ND_H
#define _ND_H

#define	ND_BASE		('N' << 8)	/** base */
#define	ND_GET		(ND_BASE + 0)	/** Get a value */
#define	ND_SET		(ND_BASE + 1)	/** Set a value */

#ifdef _KERNEL
extern	void	nd_free(caddr_t * nd_pparam);
extern	int	nd_getset(queue_t * q, caddr_t nd_param, mblk_t * mp);
extern	int	nd_get_default(queue_t * q, mblk_t * mp, caddr_t data);
extern	int	nd_get_long(queue_t * q, mblk_t * mp, ulong * lp);
extern	int	nd_load(caddr_t * nd_pparam, char * name, int (*get_pfi)(), int (*set_pfi)(), caddr_t data);
extern	int	nd_set_long(queue_t * q, mblk_t * mp, char * value, ulong * lp);
#endif  /* _KERNEL */
#endif  /* _ND_H */
