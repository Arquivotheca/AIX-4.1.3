static char sccsid[] = "@(#)76  1.17  src/bos/usr/ccs/lib/libc/NLgetamsg.c, libcmsg, bos411, 9428A410j 11/10/93 15:25:38";
/*
 * COMPONENT_NAME: (LIBCMSG) LIBC Message Catalog Functions
 *
 * FUNCTIONS: NLgetamsg
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>
#include "catio.h"

/*
 * NAME: NLgetamsg
 *                                                                    
 * FUNCTION: Open a message catalog, get message text, close the message catalog.
 *
 * ARGUMENTS:
 *	catname		- name of message catalog to be opened
 *	setno		- message catalog set number
 *	msgno		- message number within setno
 *	def		- default message text
 *
 * NOTES: This function is obselete and is being maintained for backward
 *      compatibility with AIX v3.1.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	NLgetamsg executes under a process.
 *
 * RETURNS: Pointer to the message text found in the message catalog or
 *	on any error, a pointer to the default message text.
 */  

char *NLgetamsg(char *catname, int setno, int msgno, char* def) 
{
	nl_catd catd;		/*---- catd for the catalog ----*/

	catd = catopen(catname, NL_CAT_LOCALE);
	return (catgets(catd, setno, msgno, def));
}
