/* @(#)99	1.1  src/bos/kernext/xns/spidp.h, sysxxns, bos411, 9428A410j 7/24/93 14:02:49 */
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: SI
 *		
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      Base:   spidp.h 7.4 (Berkeley) 6/28/90
 */

/*
 * Definitions for NS(tm) Internet Datagram Protocol
 * containing a Sequenced Packet Protocol packet.
 */
struct spidp {
	struct idp	si_i;
	struct sphdr 	si_s;
};
struct spidp_q {
	struct spidp_q	*si_next;
	struct spidp_q	*si_prev;
};
#define SI(x)	((struct spidp *)x)
#define si_sum	si_i.idp_sum
#define si_len	si_i.idp_len
#define si_tc	si_i.idp_tc
#define si_pt	si_i.idp_pt
#define si_dna	si_i.idp_dna
#define si_sna	si_i.idp_sna
#define si_sport	si_i.idp_sna.x_port
#define si_cc	si_s.sp_cc
#define si_dt	si_s.sp_dt
#define si_sid	si_s.sp_sid
#define si_did	si_s.sp_did
#define si_seq	si_s.sp_seq
#define si_ack	si_s.sp_ack
#define si_alo	si_s.sp_alo
