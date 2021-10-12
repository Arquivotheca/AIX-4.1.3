static char sccsid[] = "@(#)59	1.2  src/bos/usr/bin/tcbck/tcbattr.c, cmdsadm, bos411, 9428A410j 4/24/91 16:25:34";

/*
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker.
 *
 * FUNCTIONS: get_nextrec, get_record
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <stdio.h>
#include "tcbattr.h"

/*
 * NAME: get_nextrec
 *
 * FUNCTION: Locate the beginning of the next stanza in a buffer
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *
 *	get_nextrec is called with a pointer to some location within
 *	the "current" stanza, normally the first character in the
 *	stanza.
 *
 * RETURNS: Pointer to next stanza or a pointer to the end of the buffer
 *	if no stanzas remain.
 */  

char *
get_nextrec (cp)	/* find the start of the next record */
char	*cp;
{
	/*
	 * The start of a record is an alpha-numeric after a newline
	 * or the beginning of a filename [ test for a '/' character ]
	 * Skip until a newline is seen, then skip any comment lines
	 * which may follow.
	 */

	while (*cp) {
 		if (cp = (char *)strchr (cp, '\n'))
			cp++;
		else
			break;

		if (*cp == '*' || *cp == '\n')
			continue;

		if (*cp != ' ' && *cp != '\t')
			break;
	}
	return (cp);
}

/*
 * NAME: get_record
 *
 * FUNCTION: Finds beginning and end of a stanza in a memory region.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Zero on success, non-zero on failure.
 */

int 
get_record (name, buf, begin, end)
char	*name;		/* Name of stanza to locate */
char	*buf;		/* Memory region to search */
char	**begin;	/* Pointer to pointer to beginning of stanza */
char	**end;		/* Pointer to pointer to end of stanza */
{
	int	len;	/* Length of name */

	len = strlen (name);

	/*
	 * Skip any leading whitespace in the buffer
	 */

	while (*buf && isspace (*buf))
		buf++;

	/*
	 * While there is still data in the buffer to be processed,
	 * see if we are currently pointing at the desired stanza.
	 * If not, skip this stanza and go find the next one, returning
	 * an error when no stanzas remain.
	 */

	while (*buf) {
		if (strncmp (name, buf, len) == 0 && buf[len] == ':')
			break;

		buf = get_nextrec (buf);
	}
	if (! *buf)
		return ENOENT;

	/* 
	 * begin points to the first character in the stanza and
	 * end points to the first character _past_ the stanza.
	 */

	*begin = buf;
	*end = get_nextrec (buf);

	return 0;
}

