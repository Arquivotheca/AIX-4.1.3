static char sccsid[] = "@(#)94	1.1  src/bos/usr/ccs/lib/libc/NLstrcspn.c, libcnls, bos411, 9428A410j 2/26/91 17:40:10";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrcspn
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <string.h>
#include <ctype.h>

/*
 *  Return the number of array elements (bytes or wchar_ts) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on wchar_ts).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  wchar_ts.
 */

/*
 * NAME: NLstrcspn
 *
 * FUNCTION: Return the number of array elements (bytes or wchar_ts) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on wchar_ts).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  wchar_ts.
 *
 * RETURN VALUE DESCRIPTION: see FUNCTION
 */
int NLstrcspn(char *string, char *charset)
{
	return ( strcspn(string, charset) );
}
