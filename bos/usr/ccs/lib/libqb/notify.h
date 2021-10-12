/* @(#)64       1.6  src/bos/usr/ccs/lib/libqb/notify.h, libqb, bos411, 9428A410j 7/12/91 10:50:27 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
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

#define QMSGFORK "0781-264  Unable to fork a new process."
#define QMSGEXEC "0781-265  Unable to send message to remote user."
#define QMSGMHDR "Message from qdaemon:\n"

/* message getting changes */
#define MSGSTR(num,str) getmesg(num,str) 
