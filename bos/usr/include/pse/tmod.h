/* @(#)67       1.2  src/bos/usr/include/pse/tmod.h, sysxpse, bos411, 9428A410j 9/24/93 09:50:29 */
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _TMOD_
#define _TMOD_

/** Copyright (c) 1990, 1991  Mentat Inc.
 ** tmod.c 2.4, last change 4/9/91
 **/

#ifndef _STROPTS_
#include <sys/stropts.h>
#endif

/** M_IOCTL ioc_cmd values for the available utility tests */
#define	TMOD_ADJMSG	_IO('G', 0x1)
#define	TMOD_BUFCALL	_IO('G', 0x2)		/** also tests unbufcall */
#define	TMOD_CANPUT	_IO('G', 0x3)		/** tests bcanput and canput */
#define	TMOD_COPY	_IO('G', 0x4)		/** tests copyb and copymsg */
#define	TMOD_DUP	_IO('G', 0x5)		/** tests dupb and dupmsg */
#define	TMOD_ESBALLOC	_IO('G', 0x6)
#define	TMOD_FLUSH	_IO('G', 0x7)		/** tests flushband and flushq */
#define	TMOD_GETADMIN	_IO('G', 0x8)
#define	TMOD_GETMID	_IO('G', 0x9)
#define	TMOD_GETQ	_IO('G', 0x10)		/** tests getq and putq */
#define	TMOD_INSQ	_IO('G', 0x11)
#define	TMOD_LINKB	_IO('G', 0x12)		/** tests linkb, unlinkb, and msgdsize */
#define	TMOD_PULLUPMSG	_IO('G', 0x13)
#define	TMOD_PUTQ	_IO('G', 0x14)
#define	TMOD_PUTBQ	_IO('G', 0x15)
#define	TMOD_QENABLE	_IO('G', 0x16)
#define	TMOD_RMVB	_IO('G', 0x17)
#define	TMOD_RMVQ	_IO('G', 0x18)
#define	TMOD_STRLOG	_IO('G', 0x19)
#define	TMOD_STRQGET	_IO('G', 0x20)
#define	TMOD_STRQSET	_IO('G', 0x21)

#endif
