/* @(#)97       1.1  src/bos/kernel/include/pse/str_debug.h, sysxpse, bos411, 9428A410j 8/27/93 09:16:32 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

#ifndef	_STR_DEBUG_H
#define	_STR_DEBUG_H

/*
 *	Debug areas
 */

#define DB_SCHED	(1L << 0)
#define DB_LINK    	(1L << 1)
#define DB_OPEN		(1L << 2)
#define DB_CLOSE	(1L << 3)
#define DB_READ		(1L << 4)
#define DB_WRITE	(1L << 5)
#define DB_IOCTL	(1L << 6)
#define DB_PUTPMSG	(1L << 7)
#define DB_GETPMSG	(1L << 8)
#define DB_PUTMSG	(1L << 9)
#define DB_GETMSG	(1L << 10)
#define DB_PUSHPOP	(1L << 11)
#define DB_ALLOC   	(1L << 12)
#define DB_CANPUT	(1L << 13)
#define DB_ECHO		(1L << 14)
#define DB_DBG_TLI	(1L << 15)
#define DB_SIG		(1L << 16)
#define DB_MP		(1L << 17)
#define DB_SYNC		(1L << 18)
#define	DB_CTTY		(1L << 19)
#define DB_FUNC		(1L << 20)
#define	DB_CONF		(1L << 21)
#define	DB_WELD		(1L << 22)
#define	DB_TIMER	(1L << 23)
#define	DB_UTIL		(1L << 24)

#define DB_LAST		(1L << 31)	/* must be last in sequence! */

/*
 *	The following
 *		- provides the external declarations (  DEBUG + _NO_PROTO )
 *	OR	- provides the right prototypes ( DEBUG + ANSI C )
 *	OR	- makes all debug calls disappear from the code ( NO DEBUG )
 *
 *	Some debug facilities are only appropriate for certain versions,
 *	and thus may have further configuration controls.
 */

/*
 *	printf facility - works in every version
 */

#if	STREAMS_DEBUG

#define STR_DEBUG(stmt)	stmt

extern	void	DB_init();
extern	void	DB0(int, char *);
extern	void	DB1(int, char *, caddr_t);
extern	void	DB2(int, char *, caddr_t, caddr_t);
extern	void	DB3(int, char *, caddr_t, caddr_t, caddr_t);
extern	void	DB4(int, char *, caddr_t, caddr_t, caddr_t, caddr_t);
extern	void	DB5(int, char *, caddr_t, caddr_t, caddr_t, caddr_t, caddr_t);
extern	void	DB6(int, char *, caddr_t, caddr_t, caddr_t, caddr_t, caddr_t, caddr_t);
extern	void	DB_show_stream(caddr_t);

#else	/* !STREAMS_DEBUG */

#define STR_DEBUG(stmt)	/* null */

#define	DB_init()
#define DB0(area, fmt)
#define DB1(area, fmt, a)
#define DB2(area, fmt, a, b)
#define DB3(area, fmt, a, b, c)
#define DB4(area, fmt, a, b, c, d)
#define DB5(area, fmt, a, b, c, d, e)
#define DB6(area, fmt, a, b, c, d, e, f)
#define	DB_show_stream(sth)

#endif	/* !STREAMS_DEBUG */

/*
 * Function call monitoring
 */

#if	STREAMS_DEBUG
extern	void	STREAMS_ENTER_FUNC(int (*)(), int, int, int, int, int, int);
extern	void	STREAMS_LEAVE_FUNC(int (*)(), int);
extern	void	REPORT_FUNC();
#define	ENTER_FUNC(func, a, b, c, d, e, f) \
	STREAMS_ENTER_FUNC((int(*)())(func),(int)(a),(int)(b),(int)(c),(int)(d),(int)(e),(int)(f))
#define	LEAVE_FUNC(func, retval) \
	STREAMS_LEAVE_FUNC((int(*)())(func),(int)(retval))
#else
#define	ENTER_FUNC(func, a, b, c, d, e, f)
#define	LEAVE_FUNC(func, retval)
#define	REPORT_FUNC()
#endif

/*
 * STREAMS_CHECK package - works only for STREAMS_DEBUG
 */

#if	STREAMS_DEBUG && defined(DB_CHECK_LOCK)
#define	STREAMS_CHECK	1
#else
#define	STREAMS_CHECK	0
#endif

#if	STREAMS_CHECK
extern	void	DB_isopen(caddr_t);
extern	void	DB_isclosed(caddr_t);
extern	void	DB_check_streams(char *);
#else
#define	DB_isopen(sth)
#define	DB_isclosed(sth)
#define DB_check_streams(caller)
#endif
#endif /* _STR_DEBUG_H */
