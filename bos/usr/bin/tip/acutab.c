static char sccsid[] = "@(#)32	1.4  src/bos/usr/bin/tip/acutab.c, cmdtip, bos411, 9428A410j 4/10/91 09:06:09";
/* 
 * COMPONENT_NAME: UUCP acutab.c
 * 
 * FUNCTIONS: MSGSTR 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "acutab.c	5.2 (Berkeley) 4/3/86"; */

#include "tip.h"
#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

extern int df02_dialer(), df03_dialer(), df_disconnect(), df_abort(),
	   biz31f_dialer(), biz31_disconnect(), biz31_abort(),
	   biz31w_dialer(),
	   biz22f_dialer(), biz22_disconnect(), biz22_abort(),
	   biz22w_dialer(),
	   ven_dialer(), ven_disconnect(), ven_abort(),
	   hay_dialer(), hay_disconnect(), hay_abort(),
	   cour_dialer(), cour_disconnect(), cour_abort(),
	   v3451_dialer(), v3451_disconnect(), v3451_abort(),
	   v831_dialer(), v831_disconnect(), v831_abort(),
	   dn_dialer(), dn_disconnect(), dn_abort();

acu_t acutable[] = {
#if BIZ1031
	"biz31f", biz31f_dialer, biz31_disconnect,	biz31_abort,
	"biz31w", biz31w_dialer, biz31_disconnect,	biz31_abort,
#endif
#if BIZ1022
	"biz22f", biz22f_dialer, biz22_disconnect,	biz22_abort,
	"biz22w", biz22w_dialer, biz22_disconnect,	biz22_abort,
#endif
#if DF02
	"df02",	df02_dialer,	df_disconnect,		df_abort,
#endif
#if DF03
	"df03",	df03_dialer,	df_disconnect,		df_abort,
#endif
#if DN11
	"dn11",	dn_dialer,	dn_disconnect,		dn_abort,
#endif
#ifdef VENTEL
	"ventel",ven_dialer,	ven_disconnect,		ven_abort,
#endif
#ifdef HAYES
	"hayes",hay_dialer,	hay_disconnect,		hay_abort,
#endif
#ifdef COURIER
	"courier",cour_dialer,	cour_disconnect,	cour_abort,
#endif
#ifdef V3451
#ifndef V831
	"vadic",v3451_dialer,	v3451_disconnect,	v3451_abort,
#endif
	"v3451",v3451_dialer,	v3451_disconnect,	v3451_abort,
#endif
#ifdef V831
#ifndef V3451
	"vadic",v831_dialer,	v831_disconnect,	v831_abort,
#endif
	"v831",v831_dialer,	v831_disconnect,	v831_abort,
#endif
	0,	0,		0,			0
};

