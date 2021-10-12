/* @(#)11	1.4  src/bos/usr/lib/methods/common/msg_hdr.h, cmdnet, bos411, 9428A410j 4/10/91 16:58:35 */
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define MSGSTR Msg
#include <locale.h>
#include <nl_types.h>
extern *malloc();
#define Msg(n,s) catgets(catd,MSG_SET,n,s)

