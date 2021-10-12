static char sccsid[] = "@(#)23	1.2  src/bos/usr/ccs/lib/libc/oncmsg.c, libcrpc, bos411, 9428A410j 1/19/94 09:52:55";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: oncmsg
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>
#include <limits.h>
#include <libc_msg.h>

char *
oncmsg( set_no, msg_no, defmsg)
long set_no;
long msg_no;
char *defmsg;

{
	nl_catd   catd;    	/* file descriptor for message catalog */
	char 	  *msg; 	/* message string returned from catgets */
	static char buffer[NL_TEXTMAX];

	/* open the message catalog */
	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
	/* get a the requested message */
	msg = catgets(catd, set_no, msg_no, defmsg);
	/* close the catalog */
	strcpy(buffer,msg);
	catclose(catd);
	return(buffer);
}
