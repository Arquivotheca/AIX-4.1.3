/* @(#)00	1.1  src/bos/kernext/xns/spp_debug.h, sysxxns, bos411, 9428A410j 7/24/93 14:02:52 */
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: 
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
 *      Base:   spp_debug.h     7.4 (Berkeley) 6/28/90
 */

struct	spp_debug {
	u_long	sd_time;
	short	sd_act;
	short	sd_ostate;
	caddr_t	sd_cb;
	short	sd_req;
	struct	spidp sd_si;
	struct	sppcb sd_sp;
};

#define	SA_INPUT 	0
#define	SA_OUTPUT	1
#define	SA_USER		2
#define	SA_RESPOND	3
#define	SA_DROP		4

#ifndef CONST
#define CONST
#endif

#ifdef SANAMES
CONST	char	*sanames[] =
    { "input", "output", "user", "respond", "drop" };
#endif

#define	SPP_NDEBUG 100
extern	struct	spp_debug spp_debug[SPP_NDEBUG];
extern	int	spp_debx;
