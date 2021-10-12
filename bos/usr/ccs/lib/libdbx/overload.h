/* @(#)66	1.1.1.1  src/bos/usr/ccs/lib/libdbx/overload.h, libdbx, bos411, 9428A410j 7/13/92 10:42:16 */
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _NO_PROTO
extern Node resolveOverload(Node, unsigned long);
extern Node resolveTemplate(Symbol, unsigned long);
extern Node resolveFns(Name, unsigned long);
extern int insertInArgArray(Symbol);
extern void deleteFromArgArray(int);
#else
extern Node resolveOverload();
extern Node resolveTemplate();
extern Node resolveFns();
extern int insertInArgArray();
extern void deleteFromArgArray();
#endif
