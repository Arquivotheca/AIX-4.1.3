static char sccsid[] = "@(#)86	1.6  src/bos/usr/ccs/lib/libPW/logname.c, libPW, bos411, 9428A410j 6/16/90 00:56:27";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: logname
 *
 * ORIGINS: 3 27
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
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

/*
 * FUNCTION: Return caller's login name
 *
 * RETURN VALUE DESCRIPTIONS:
 *		char * to login name if successful
 *		NULL on ERROR
 */

char *
logname()
{
	extern char *getenv();

	return(getenv("LOGNAME"));
}
