/* @(#)54     1.7  src/bos/usr/include/pse/led.h, sysxpse, bos411, 9428A410j 3/23/93 16:34:55 */
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: OK_32PTR
 *		RES_ACQ
 *		RES_ACQ_INLINE
 *		RES_INIT
 *		RES_REL
 *		RES_REL_INLINE
 *		RES_WAS_CRITICAL
 *		RET_ADDR
 *		SQ_EMPTY
 *		SQ_NEXT
 *		SQ_PREV
 *		U8
 *		csq_acquire
 *		csq_release
 *		mps_copyin
 *		mps_copyout
 *		spl7
 *		sq_lateral
 *		
 *
 *   ORIGINS: 27,63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/** Copyright (c) 1990  Mentat Inc.
 ** led.h 1.3, last change 4/9/91
 **/

#ifndef _LED_
#define _LED_

#define AIX
#define MPS

#ifndef	BIG_ENDIAN
#define	BIG_ENDIAN 1 /* explicit value for test in courier.h */
#endif	/* BIG_ENDIAN */

#define	SYS5

#define	SVR4_STYLE

#define USE_STDARG

/* After a server forks, should the child or the parent go back to listen
 * for new requests ?  If this is set, the parent does the work and the child
 * listens.  This assumes that ignoring SIGCLD will allow the parent to
 * ignore the child and not need to do any waits or other cleanup.
 */
#define	PARENT_WORKS_AFTER_FORK

/* Maximum buffer size that should be placed on the stack (local variables) */
#define	MAX_STACK_BUF	512

#define	U8(x)		((unsigned char)(x))
#define	LONG_SIGN_BIT	(0x80000000L)

/* Convert milliseconds to clock ticks and vice versa.  Obviously dependent
** on the system clock rate 
#define	MS_TO_TICKS(ms)		((ms) >> 3)
#define	TICKS_TO_MS(ticks)	((ticks) << 3)
*/

#ifndef HZ
#define	HZ		100
#endif

typedef short		i16;
typedef int		i32;
typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef unsigned int	u32;

typedef	unsigned char	* DP;
typedef	char		* IDP;
typedef	struct msgb	* MBLKP;
typedef	struct msgb	** MBLKPP;
typedef int		* ERRP;
typedef	char		* USERP;
#ifdef reg
#undef reg
#endif

#include <sys/types.h>
#define reg register

/* Used only for debugging to find the caller of a function, not required */
#ifndef	RET_ADDR
#define	RET_ADDR(addr_of_first_arg)	(((pfi_t *)addr_of_first_arg)[-1])
#endif

#define	OK_32PTR(addr)	true

#define	noshare

#define	NATIVE_POLL

#ifdef KERNEL

#ifndef _KERNEL
#define _KERNEL
#endif

#ifdef	staticf
#undef	staticf
#endif
#define	staticf	/**/

/* Define printf to be mi_printf in aixsth.c */
#define	printf	mi_printf

#ifndef EINTR
#include <sys/errno.h>
#endif

#include <sys/limits.h>
#include <sys/param.h>

#define	RES_INIT(res)	{ int _res_savpri = spl7(); (res)->res_acqcnt = 0; (res)->res_critical = spl7(); splx(_res_savpri);}
#define	RES_ACQ(res)	{ int _res_savpri = spl7(); if ((res)->res_acqcnt++ == 0) (res)->res_savpri=_res_savpri; }
#define	RES_REL(res)	{if (--(res)->res_acqcnt ==0) splx((res)->res_savpri);}
#define	RES_WAS_CRITICAL(res)	((res)->res_savpri == (res)->res_critical)
#define	RES_ACQ_INLINE(res)	RES_ACQ(res)
#define	RES_REL_INLINE(res)	RES_REL(res)

typedef	struct res_s {
	int	res_acqcnt;
	int	res_savpri;
	int	res_critical;
} RES;

#ifndef	INTOFFL0
#include <sys/intr.h>
#endif

#define	SPLDECL		int	_savflags;
#define	SPLSTR		(_savflags = i_disable(INTMAX))
#define	SPLX		i_enable(_savflags)
/* #define splx(mask)	splx(mask)	/* i_enable( (mask) ) */
#define spl7()		splhi()		/* i_disable( INTMAX ) */

#define	globaldef
#define	globalref	extern

/* define for poll() */
#define	int_fd	fd

#define	STH_XTRA						\
	struct {						\
		OSR	sthou_osr;				\
		char	sthou_extra[sizeof(struct strfdinsert)];\
	} sth_sthou;
#define	sth_osr	sth_sthou.sthou_osr

/*
 * psuedo-u for open/close processing.
 */
typedef struct psuedo_u_s {
	/* read/write fields */
	int	u_valid;
	char	u_error;
	pid_t	*u_ttyp;
	/* read only fields */
	cred_t	*u_cred;
	struct proc *u_procp;
} psuedo_u_t;

#define	OSR_XTRA	\
	u32	osr_sleeper;	\
	psuedo_u_t osr_u;		/* only valid when u_valid is set */
#define	OPEN_XTRA
#define	RD_XTRA
#define	IOCTL_XTRA	\
	MBLKP	ioc_copy_mp;
#define	osr_ioctl_copy_mp	osr_osru.ioctl_osr.ioc_copy_mp
#define	OIA_XTRA

#define	F_OSR_IN_USE	0x2000
#define	F_OSR_COPYIN	0x4000
#define	F_OSR_COPYOUT	0x8000

extern	SQH	st_runq;

/* SQ flags */
#define	SQ_INUSE	1
#define	SQ_HOLD		2

struct	q_xtra {
	SQH	qx_sqh;
	SQ	qx_runq_sq;
};
#define	q_sqh		q_osx->qx_sqh
#define	q_runq_sq	q_osx->qx_runq_sq

extern	SQP	remqhi(   SQHP sqh   );

#define	SQ_EMPTY(sq)		(((SQP)(sq))->sq_next == (SQP)(sq))
#define	SQ_NEXT(sq)		(((SQP)(sq))->sq_next)
#define	SQ_PREV(sq)		(((SQP)(sq))->sq_prev)
#define	SQ_INSQT		insqti
#define	SQ_REMQH		remqhi
#define	SQH_INIT		sqh_init
#define	csq_lateral		sq_lateral
#define	sq_lateral(sqh,sq)	(*(sq)->sq_entry)((sq)->sq_arg0, (sq)->sq_arg1)
#define	csq_acquire(sqh,sq)	(true)
#define	csq_release(sqh)
#define	mps_sq_get(sq)		(((sq)->sq_flags & SQ_INUSE) ? false : (((sq)->sq_flags |= SQ_INUSE), true))

#define	mps_copyin(osr, usrc, kdest, cnt)	(copyin(usrc, kdest, cnt) ? EFAULT : 0)
#define	mps_copyout(osr, ksrc, udest, cnt)	(copyout(ksrc, udest, cnt) ? EFAULT: 0)

extern	void	mps_osr_loop();
extern	int	mps_osr_restart();
#define	STH_OSR_LOOP		mps_osr_loop
#define	STH_OSR_RESTART		mps_osr_restart

#endif	/*KERNEL*/

#endif	/*_LED_*/
