static char sccsid[] = "@(#)26	1.13  src/bos/usr/bin/logname/logname.c, cmdsauth, bos411, 9428A410j 2/28/94 16:45:17";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include "logname_msg.h"

#define MSGSTR(num,str) catgets(catd,MS_LOGNAME,num,str)

main(int argc, char *argv[])
{
	char *name,*getlogin();
	nl_catd catd;

	setlocale (LC_ALL, "");
	catd = catopen (MF_LOGNAME, NL_CAT_LOCALE);

	if ((name = getlogin()) == NULL)
	{
		fprintf (stderr, MSGSTR(M_LNFAIL, "%s: getlogin failed\n"),
			basename (argv[0]));
		return (1);
	}

	puts(name);

	return (0);

}
