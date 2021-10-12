static char sccsid[] = "@(#)43	1.5  src/bos/usr/ccs/lib/libPW/giveup.c, libPW, bos411, 9428A410j 6/16/90 00:56:15";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: giveup
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

# include "signal.h"

/*
	Chdir to "/" if argument is 0.
	Set IOT signal to system default.
	Call abort(III).
	(Shouldn't produce a core when called with a 0 argument.)
*/


giveup(dump)
{
	if (!dump)
		chdir("/");
	signal(SIGIOT,SIG_DFL);
	abort();
}
