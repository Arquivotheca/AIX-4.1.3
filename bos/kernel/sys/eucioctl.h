/* @(#)50 1.1 src/bos/kernel/sys/eucioctl.h, sysxtty, bos411, 9428A410j 10/15/93 11:57:42 */
/*
 * COMPONENT_NAME: SYSXTTY
 *
 * FUNCTIONS :
 *
 * ORIGINS: 40, 71, 83
 *
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_EUCIOCTL
#define _H_EUCIOCTL

/*
 * EUC ioctl's
 */
#ifndef IOC_VOID
#define	IOC_VOID	0x20000000
#endif
#define _IO_EUC(x,y)	(IOC_VOID|(x<<8)|y)

#define EUC_WSET	_IO_EUC('J', 129)	/* set code widths */
#define EUC_WGET	_IO_EUC('J', 130)	/* get code widths */
#define EUC_IXLON	_IO_EUC('J', 131)	/* input conversion on */
#define EUC_IXLOFF	_IO_EUC('J', 132)	/* input conversion off */
#define EUC_OXLON	_IO_EUC('J', 133)	/* output conversion on */
#define EUC_OXLOFF	_IO_EUC('J', 134)	/* output conversion off */
#define EUC_MSAVE	_IO_EUC('J', 135)	/* save mode, go to ASCII mode */
#define EUC_MREST	_IO_EUC('J', 136)	/* restore mode */

struct eucioc {
	unsigned char eucw[4];	/* memory width of code sets */
	unsigned char scrw[4];	/* screen width of code sets */
};
typedef struct eucioc	eucioc_t;

#endif /* _H_EUCIOCTL */
