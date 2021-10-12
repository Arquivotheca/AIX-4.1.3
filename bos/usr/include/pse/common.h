/* @(#)48	1.3  src/bos/usr/include/pse/common.h, sysxpse, bos411, 9428A410j 7/16/91 15:50:33 */
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** common.h 2.2, last change 3/18/91
 **/

#ifndef	_COMMON_
#define	_COMMON_

#define	NODEV	NODEVICE

#define A_CNT(arr)	(sizeof(arr)/sizeof(arr[0]))
#define	A_END(arr)	(&arr[A_CNT(arr)])
#define	A_LAST(arr)	(&arr[A_CNT(arr)-1])
#define ERR		(-1)
#define fallthru
#define	false		((boolean)0)
#define getarg(ac,av)	(optind < ac ? av[optind++] : nilp(char))
#ifndef	MAX
#define MAX(x1,x2)	((x1) >= (x2) ? (x1) : (x2))
#endif
#ifndef	MIN
#define MIN(x1,x2)	((x1) <= (x2) ? (x1) : (x2))
#endif

/* The MAX_XXX and MIN_XXX defines assume a two's complement architecture.
** They should be overriden in <led.h> if this assumption is incorrect.
*/
#define	MAX_INT		((int)(MAX_UINT >> 1))
#define	MAX_LONG	((long)(MAX_ULONG >> 1))
#define	MAX_SHORT	((short)(MAX_USHORT >> 1))
#define	MAX_UINT	((unsigned int)~0)
#define	MAX_ULONG	((unsigned long)~0)
#define	MAX_USHORT	((unsigned short)~0)
#define	MIN_INT		(~MAX_INT)
#define	MIN_LONG	(~MAX_LONG)
#define	MIN_SHORT	(~MAX_SHORT)

#define	newa(t,cnt)	((t *)calloc(cnt, sizeof(t)))
#define	nilp(t)		((t *)0)
#define	nil(t)		((t)0)
#define	noop
#define reg		register
#define	staticf		static
#define	true		((boolean)1)

typedef	int	boolean;
typedef	int	(*pfi_t)();
typedef	void	(*pfv_t)();
#ifdef XENIX
typedef	int	(far *farpfi_t)();
#endif
typedef	pfi_t	(*pfpfi_t)();

#define	BE32_EQL(a,b)	(((u8 *)a)[0] == ((u8 *)b)[0]  && ((u8 *)a)[1] == ((u8 *)b)[1]  && ((u8 *)a)[2] == ((u8 *)b)[2]  && ((u8 *)a)[3] == ((u8 *)b)[3])
#define	BE16_EQL(a,b)	(((u8 *)a)[0] == ((u8 *)b)[0]  && ((u8 *)a)[1] == ((u8 *)b)[1])
#define	BE16_TO_U16(a)	((((u16)((u8 *)a)[0] << (u16)8) | ((u16)((u8 *)a)[1] & 0xFF)) & (u16)0xFFFF)
#define	BE32_TO_U32(a)	((((u32)((u8 *)a)[0] & 0xFF) << (u32)24) | (((u32)((u8 *)a)[1] & 0xFF) << (u32)16) | (((u32)((u8 *)a)[2] & 0xFF) << (u32)8)  | ((u32)((u8 *)a)[3] & 0xFF))
#define	U16_TO_BE16(u,a) ((((u8 *)a)[0] = (u8)((u) >> 8)), (((u8 *)a)[1] = (u8)(u)))
#define	U32_TO_BE32(u,a) ((((u8 *)a)[0] = (u8)((u) >> 24)), (((u8 *)a)[1] = (u8)((u) >> 16)), (((u8 *)a)[2] = (u8)((u) >> 8)),(((u8 *)a)[3] = (u8)(u)))

/* Local Environment Definition, this may and should override the
** the default definitions above where the local environment differs.
*/
#include <pse/led.h>

#ifdef	BIG_ENDIAN

#ifndef	ABE32_TO_U32
#define	ABE32_TO_U32(p)		(*((u32 *)p))
#endif

#ifndef	ABE16_TO_U16
#define	ABE16_TO_U16(p)		(*((u16 *)p))
#endif

#ifndef	U16_TO_ABE16
#define	U16_TO_ABE16(u,p)	(*((u16 *)p) = (u))
#endif

#ifndef	U32_TO_ABE16
#define	U32_TO_ABE16(u,p)	U16_TO_ABE16(u,p)
#endif

#ifndef	UA32_TO_U32
#define	UA32_TO_U32(p,u)	((u) = (((u32)((u8 *)p)[0] << 24) | ((u32)((u8 *)p)[1] << 16) | ((u32)((u8 *)p)[2] << 8) | (u32)((u8 *)p)[3]))
#endif

#ifndef	U32_TO_ABE32
#define	U32_TO_ABE32(u,p)	(*((u32 *)p) = (u))
#endif

#else

#ifndef	ABE16_TO_U16
#define	ABE16_TO_U16(p)		BE16_TO_U16(p)
#endif

#ifndef	ABE32_TO_U32
#define	ABE32_TO_U32(p)		BE32_TO_U32(p)
#endif

#ifndef	U16_TO_ABE16
#define	U16_TO_ABE16(u,p)	U16_TO_BE16(u,p)
#endif

#ifndef	U32_TO_ABE16
#define	U32_TO_ABE16(u,p)	U16_TO_ABE16(u,p)
#endif

#ifndef	U32_TO_ABE32
#define	U32_TO_ABE32(u,p)	U32_TO_BE32(u,p)
#endif

#ifndef	UA32_TO_U32
#define	UA32_TO_U32(p,u)	((u) = (((u32)((u8 *)p)[3] << 24) | ((u32)((u8 *)p)[2] << 16) | ((u32)((u8 *)p)[1] << 8) | (u32)((u8 *)p)[0]))
#endif

#endif

#ifdef	KERNEL

/* Extra MPS mblk type */
#define	M_MI		64
/* Subfields for M_MI messages */
#define	M_MI_READ_RESET	1
#define	M_MI_READ_SEEK	2
#define	M_MI_READ_END	4

#ifndef EINVAL
#include <sys/errno.h>
#endif

#ifdef MPS
#define	mi_adjmsg	adjmsg
#endif

#ifndef	CANPUTNEXT
#define	CANPUTNEXT(q)	canput((q)->q_next)
#endif

#endif /* KERNEL */

#ifndef	GOOD_EXIT_STATUS
#define	GOOD_EXIT_STATUS	0
#endif

#ifndef	BAD_EXIT_STATUS
#define	BAD_EXIT_STATUS		1
#endif

#ifndef	is_ok_exit_status
#define	is_ok_exit_status(status)	(status == GOOD_EXIT_STATUS)
#endif

#endif
