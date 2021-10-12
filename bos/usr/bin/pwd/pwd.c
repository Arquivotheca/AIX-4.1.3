static char sccsid [] = "@(#)68	1.10  src/bos/usr/bin/pwd/pwd.c, cmdfiles, bos412, 9446C 11/14/94 16:49:24";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: pwd
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * Print working (current) directory
 */
#include <unistd.h>
#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <locale.h>

main()
{
	char pathname[PATH_MAX + 1];

	(void) setlocale (LC_ALL,"");  /* In case perror needs it */

	if(getcwd(pathname,sizeof(pathname)) == 0) {
		perror("pwd");
		exit(1);
	}

	puts(pathname);
	exit(0);
}
