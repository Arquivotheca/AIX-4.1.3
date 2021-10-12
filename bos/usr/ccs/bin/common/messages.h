/* @(#)48       1.5  src/bos/usr/ccs/bin/common/messages.h, cmdprog, bos411, 9428A410j 6/3/91 12:01:25 */
/*
 * COMPONENT_NAME: (CMDPROG) messages.h
 *
 * FUNCTIONS: MESSAGE
 *
 * ORIGINS: 27 03 09 32 00
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT INTERNATIONAL BUSINESS MACHINES CORP. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 *  @OSF_COPYRIGHT@
 */
/* AIWS C compiler */

#ifndef UERROR
#    define     UERROR  uerror
#endif

#ifndef WERROR
#    define     WERROR  werror
#endif

#ifndef WARNING
#    define     WARNING warning
#endif

#ifndef MESSAGE
#define         MESSAGE(x) catgets(catd, MS_CTOOLS, x+1, msgtext[ x ])
#endif

extern char     *msgtext[ ];

#define ALWAYS  1
