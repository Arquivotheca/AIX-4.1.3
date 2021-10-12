/* @(#)29       1.4  src/bos/usr/ccs/bin/sno/sno.h, cmdlang, bos411, 9428A410j 3/9/94 12:26:47 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_SNO
#define _H_SNO

#include <stdio.h>
#include <nl_types.h>
#ifdef NLS
#include <NLchar.h>
#endif /* NLS */

#ifdef MSG
#include	"sno_msg.h"
#define		MSGSTR(Num, Str) catgets(catd, MS_SNO, Num, Str)
nl_catd catd;
#else
#define		MSGSTR(Num, Str) Str
#endif

struct  node {
        struct node *p1;
        struct node *p2;
        char typ;
#ifdef NLS
        NLchar ch;
#else /* NLS */
        char ch;
#endif /* NLS */
};

extern int     freesize;
extern struct  node *lookf;
extern struct  node *looks;
extern struct  node *lookend;
extern struct  node *lookstart;
extern struct  node *lookdef;
extern struct  node *lookret;
extern struct  node *lookfret;
extern int     cfail;
extern int     rfail;
extern struct  node *freelist, *freespace;
extern struct  node *namelist;
extern int     lc;
extern struct  node *schar;

struct node *strst1(), *salloc(), *look(), *copy(), *sgetc(),
	*binstr(), *pop(), *doop(), *push(), *search(),
	*add(), *sub(), *mult(), *div(), *cat(),
	*compile(), *execute(), *eval(), *syspit();
#endif /* _H_SNO */
