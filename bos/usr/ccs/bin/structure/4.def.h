/* @(#)85	1.2  src/bos/usr/ccs/bin/structure/4.def.h, cmdprog, bos411, 9428A410j 6/15/90 22:55:50 */
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26; 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define YESTAB	TRUE
#define NOTAB	FALSE
#define TABOVER(n)	tabover(n,outfd)
#define OUTSTR(x)		fprintf(outfd,"%s",x)
#define OUTNUM(x)		fprintf(outfd,"%d",x)


extern LOGICAL *brace;
#define YESBRACE(v,i)	{ if (DEFINED(LCHILD(v,i))) brace[LCHILD(v,i)] = TRUE; }
#define NOBRACE(v,i)	{ if (DEFINED(LCHILD(v,i))) brace[LCHILD(v,i)] = FALSE; }
#define HASBRACE(v,i)	 ((DEFINED(LCHILD(v,i))) ? brace[LCHILD(v,i)] : TRUE)
