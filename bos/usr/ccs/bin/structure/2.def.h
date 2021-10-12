/* @(#)69	1.2  src/bos/usr/ccs/bin/structure/2.def.h, cmdprog, bos411, 9428A410j 6/15/90 22:54:52 */
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

extern int accessnum;		/* number of nodes accessible from START */
extern VERT *after;		/* node numbers associated with after numbers of depth first search */
extern int *ntobef;		/* before numbers associated with nodes */
extern int *ntoaft;		/* after numbers associated with nodes */
