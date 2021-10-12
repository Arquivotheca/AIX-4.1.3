static char sccsid[] = "@(#)03	1.4  src/bos/usr/bin/uucp/imsg.c, cmduucp, bos411, 9428A410j 6/17/93 14:36:50";
/* 
 * COMPONENT_NAME: CMDUUCP imsg.c
 * 
 * FUNCTIONS: imsg, omsg 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.imsg.c
	imsg.c	1.1	7/29/85 16:33:11
*/
#include "uucp.h"
/* VERSION( imsg.c	5.2 -  -  ); */

#define MSYNC	'\020'
/* maximum likely message - make sure you don't get run away input */
#define MAXIMSG	256

/*
 * read message routine used before a
 * protocol is agreed upon.
 *	msg	-> address of input buffer
 *	fn	-> input file descriptor 
 * returns:
 *	EOF	-> no more messages
 *	0	-> message returned
 */
imsg(msg, fn)
register char *msg;
register int fn;
{
	register char c;
	short fndsync;
	char *bmsg;

	fndsync = 0;
	bmsg = msg;
	CDEBUG(7, "imsg %s>", "");
	while (read(fn, msg, sizeof(char)) == sizeof(char)) {
		/* *msg &= 0177;*/
		c = *msg;
		CDEBUG(7, "%s", c < 040 ? "^" : "");
		CDEBUG(7, "%c", c < 040 ? c | 0100 : c);
		if (c == MSYNC) { /* look for sync character */
			msg = bmsg;
			fndsync = 1;
			continue;
		}
		if (!fndsync)
			continue;

		if (c == '\0' || c == '\n') {
			*msg = '\0';
			return(0);
		}
		else
			msg++;

		if (msg - bmsg > MAXIMSG)	/* unlikely */
			return(FAIL);
	}
	/* have not found sync or end of message */
	*msg = '\0';
	return(EOF);
}

/*
 * initial write message routine -
 * used before a protocol is agreed upon.
 *	type	-> message type
 *	msg	-> message body address
 *	fn	-> file descriptor
 * return: 
 *	Must always return 0 - wmesg (WMESG) looks for zero
 */
omsg(type, msg, fn)
register char *msg;
register char type;
int fn;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "%c%c%s", MSYNC, type, msg);
	(*Write)(fn, buf, strlen(buf) + 1);
	return(0);
}
