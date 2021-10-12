/* @(#)72	1.4  src/bos/usr/bin/xsend/xmail.h, cmdmailx, bos411, 9428A410j 4/17/91 15:33:09 */
/* 
 * COMPONENT_NAME: CMDMAILX xmail.h
 * 
 * FUNCTIONS: nin, nout 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *	xmail.h	4.1	83/03/30
 */

#include <stdio.h>
#include <mp.h>
#include <locale.h>
#include <nl_types.h>
extern MINT *x, *b, *one, *c64, *t45, *z, *q, *r, *two, *t15;
extern char buf[256];
#ifdef debug
#define nin(x, y) m_in(x, 8, y)
#define nout(x, y) m_out(x, 8, y)
#endif
