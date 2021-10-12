/* @(#)62	1.5  src/bos/usr/include/netiso/tp_seq.h, sockinc, bos411, 9428A410j 3/5/94 12:41:48 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* 
 * ARGO TP
 *
 * $Header: tp_seq.h,v 5.1 88/10/12 12:20:59 root Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_seq.h,v $
 *	(#)tp_seq.h	7.4 (Berkeley) 1/16/90 *
 *
 * These macros perform sequence number arithmetic modulo (2**7 or 2**31).
 * The relevant fields in the tpcb are:
 *  	tp_seqmask : the mask of bits that define the sequence space.
 *  	tp_seqbit  : 1 + tp_seqmask
 *  	tp_seqhalf : tp_seqbit / 2 or half the sequence space (rounded up)
 * Not exactly fast, but at least it's maintainable.
 */

#ifndef __TP_SEQ__
#define __TP_SEQ__

#define SEQ(tpcb,x) \
	((x) & (tpcb)->tp_seqmask)

#define SEQ_GT(tpcb, seq, operand ) \
( ((int)((seq)-(operand)) > 0)\
? ((int)((seq)-(operand)) < (int)(tpcb)->tp_seqhalf)\
: !(-((int)(seq)-(operand)) < (int)(tpcb)->tp_seqhalf))

#define SEQ_GEQ(tpcb, seq, operand ) \
( ((int)((seq)-(operand)) >= 0)\
? ((int)((seq)-(operand)) < (int)(tpcb)->tp_seqhalf)\
: !((-((int)(seq)-(operand))) < (int)(tpcb)->tp_seqhalf))

#define SEQ_LEQ(tpcb, seq, operand ) \
( ((int)((seq)-(operand)) <= 0)\
? ((-(int)((seq)-(operand))) < (int)(tpcb)->tp_seqhalf)\
: !(((int)(seq)-(operand)) < (int)(tpcb)->tp_seqhalf))

#define SEQ_LT(tpcb, seq, operand ) \
( ((int)((seq)-(operand)) < 0)\
? ((-(int)((seq)-(operand))) < (int)(tpcb)->tp_seqhalf)\
: !(((int)(seq)-(operand)) < (int)(tpcb)->tp_seqhalf))
	
#define SEQ_MIN(tpcb, a, b) ( SEQ_GT(tpcb, a, b) ? b : a)

#define SEQ_MAX(tpcb, a, b) ( SEQ_GT(tpcb, a, b) ? a : b)

#define SEQ_INC(tpcb, Seq) ((++Seq), ((Seq) &= (tpcb)->tp_seqmask))

#define SEQ_DEC(tpcb, Seq)\
	((Seq) = (((Seq)+(unsigned)((int)(tpcb)->tp_seqbit - 1))&(tpcb)->tp_seqmask))

/* (amt) had better be less than the seq bit ! */

#define SEQ_SUB(tpcb, Seq, amt)\
	(((Seq) + (unsigned)((int)(tpcb)->tp_seqbit - amt)) & (tpcb)->tp_seqmask)
#define SEQ_ADD(tpcb, Seq, amt) (((Seq) + (unsigned)amt) & (tpcb)->tp_seqmask)


#define IN_RWINDOW(tpcb, seq, lwe, uwe)\
	( SEQ_GEQ(tpcb, seq, lwe) && SEQ_LT(tpcb, seq, uwe) )

#define IN_SWINDOW(tpcb, seq, lwe, uwe)\
	( SEQ_GT(tpcb, seq, lwe) && SEQ_LEQ(tpcb, seq, uwe) )

#endif /* __TP_SEQ__ */
