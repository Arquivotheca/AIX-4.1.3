/* @(#)39	1.1  src/bos/diag/tu/siosl/kbddev/kbd_tu.h, tu_siosl, bos411, 9428A410j 3/6/92 16:32:50 */

/*
 * COMPONENT_NAME: (tu_siosl) header file for keyboard device diagnostic
 *			test unit.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<nl_types.h>

#ifndef _dkbd
#define _dkbd

#define CANCEL_KEY_ENTERED	7
#define EXIT_KEY_ENTERED	8

struct	tucb_d
{
	nl_catd	catd;
	long	ad_mode;
	int	kbtype;		/* 0=101, 1=102, 2=106 */
};

#endif 
