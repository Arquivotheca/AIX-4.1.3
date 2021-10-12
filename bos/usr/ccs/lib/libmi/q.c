static char sccsid[] = "@(#)90	1.1  src/bos/usr/ccs/lib/libmi/q.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:30";
/*
 *   COMPONENT_NAME: CMDPSE
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
 ** q.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>

typedef struct q_s {	/**/
	struct q_s	* q_next;
	struct q_s	* q_prev;
} Q;

extern	Q	* q_init(   Q * qp   );

#define	Q_empty(q)	(!(q)  ||  (q)->q_next == (q))		/**/
#define	Q_init(q)	((q)->q_prev = (q), (q)->q_next = (q))	/**/
#define	Q_nxt(t,q)	((t *)(((Q *)q)->q_next))		/**/
#define	Q_prv(t,q)	((t *)(((Q *)q)->q_prev))		/**/
#define	Q_new(t)	((t *)q_init((Q *)newa(t, 1)))		/**/
#define	Q_enew(t)	((t *)q_init((Q *)enewa(t, 1)))		/**/

Q *
q_init (qp)
	Q	* qp;
{
	Q_init(qp);
	return qp;
}

Q *
q_i_t (qhp, qp)
	Q	* qhp;
	Q	* qp;
{
	qhp->q_prev->q_next = qp;
	qp->q_next = qhp;
	qp->q_prev = qhp->q_prev;
	qhp->q_prev = qp;
	return qp;
}

Q *
q_out (qp)
	Q	* qp;
{
	qp->q_next->q_prev = qp->q_prev;
	qp->q_prev->q_next = qp->q_next;
	Q_init(qp);
	return qp;
}
