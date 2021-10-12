static char sccsid[] = "@(#)24  1.1  src/bos/usr/lpp/kls/dictutil/humsg.c, cmdkr, bos411, 9428A410j 5/25/92 14:47:06";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		humsg.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       humsg.c
 *
 *  Description:  display message.
 *
 *  Functions:    humsg()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include "hut.h"
#include <memory.h>

extern int     hudisply();

int humsg( udcb , line , col , msgid )

UDCB    *udcb;          /* pointer to UDCB.                             */
short   line;           /* start line.                                  */
short   col;            /* start column.                                */
short   msgid;          /* message ID.                                  */

{
	char    blkmsg[U_EMSG];
	int     wrc;
	short   data_length = U_MAXCOL;
	char	*msg_ptr ;

	/* display error message.                                       */
	memset(blkmsg,0x20,(int)(data_length));
	blkmsg[U_MAXCOL-1] = NULL;

	msg_ptr = catgets(udcb->msg_catd, 1, msgid, "dummy");
	if (strcmp(msg_ptr, "dummy") !=0)
		strncpy(blkmsg, msg_ptr, data_length);
	wrc =  hudisply(udcb,line,col,blkmsg,data_length);

	fflush(stdout);
	return;
}
