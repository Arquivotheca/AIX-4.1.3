static char sccsid[] = "@(#) 98 1.1 src/bos/usr/lpp/bosinst/BosMenus/parse.c, bosinst, bos411, 9428A410j 93/07/01 12:10:11";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: parse
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME: parse
 *
 * FUNCTION: parse multi line text strings into a menu strucure
 *
 * EXECUTION ENVIRONMENT:
 *	Called by BosMenus preformat routines to place multi line
 *	messages into a menu.
 * Assumptions: 
 *	1. buf is statically declared somewhere so the menus will
 *              work.
 *	2. limit is initiallit at least 1.
 *
 * NOTES:
 * 	 Recursively parse a multiline string, replacing
 *       newlines with nulls.  Each new line is stored in the
 *       menu Text.
 *      
 * DATA STRUCTURES:
 *	menu structure update
 *	text string passed in is updated
 *
 * RETURNS: None.
 */

#include <string.h>
#include "Menus.h"

void parse(
struct Menu *menu,		/* menu to work with                  */
int start,			/* index into menu text to place line */
char *buf,			/* text buffer to search              */
int limit)			/* max number of menu lines to use    */
{
    char *ptr;

    menu->Text[start] = buf;
    ptr = strchr(buf, '\n');
    if (ptr)
    {
	*ptr++ = '\0';
	if (--limit)
	    parse(menu, start+1, ptr, limit-1);
    }
}
