/* @(#)64	1.1  src/bos/usr/include/pse/q.h, sysxpse, bos411, 9428A410j 5/7/91 14:31:02 */
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

#ifndef _Q_
#define _Q_

/** Copyright (c) 1990  Mentat Inc.
 ** q.c 2.1, last change 11/14/90
 **/

typedef struct q_s {
	struct q_s	* q_next;
	struct q_s	* q_prev;
} Q;

#define	Q_empty(q)	(!(q)  ||  (q)->q_next == (q))
#define	Q_init(q)	((q)->q_prev = (q), (q)->q_next = (q))
#define	Q_nxt(t,q)	((t *)(((Q *)q)->q_next))
#define	Q_prv(t,q)	((t *)(((Q *)q)->q_prev))
#define	Q_new(t)	((t *)q_init((Q *)newa(t, 1)))
#define	Q_enew(t)	((t *)q_init((Q *)enewa(t, 1)))

extern	Q *	q_init(   Q * qp   );

extern	Q *	q_i_t(   Q * qhp, Q * qp   );

extern	Q *	q_out(   Q * qp   );

#endif
